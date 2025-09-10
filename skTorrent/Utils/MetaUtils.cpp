#include "MetaUtils.hpp"
#include "BencodeParser.hpp"
#include <stdexcept>
#include <openssl/sha.h>
#include <openssl/crypto.h>
#include <fstream>

namespace Torrent::Utils {
size_t skipElement(const std::string& data, size_t pos)
{
    size_t retpos = pos;

    auto skipString = [&](std::string data, size_t pos)
    {
        std::string len;
        while (data[pos] != ':')
        {
            len += data[pos];
            ++pos;
        }
        pos += std::stoul(len);
        return pos;
    };

    if (std::isdigit(static_cast<unsigned char>(data[retpos])))
    {
        retpos = skipString(data, retpos);
    }
    else
    {
        switch (data[retpos])
        {
            case 'i':
            {
                while (data[retpos] != 'e')
                {
                    ++retpos;
                }
                break;
            }
            case 'l':
            {
                ++retpos;
                while (data[retpos] != 'e')
                {
                    retpos = skipElement(data, retpos);
                }
                break;
            }
            case 'd':
            {
                ++retpos;
                while (data[retpos] != 'e')
                {
                    retpos = skipString(data, retpos);
                    ++retpos;
                    retpos = skipElement(data, retpos);
                }
                break;
            }
            default: break;
        }
    }
    ++retpos;
    return retpos;
}

std::string extractRawInfoSection(const std::string& data)
{
    size_t keyPos = data.find("4:info");
    if (keyPos == std::string::npos)
    {
        throw std::runtime_error("Failed to extract info section: not found");
    }

    size_t infoStart = keyPos + 6;
    if (infoStart >= data.size() || data[infoStart] != 'd')
    {
        throw std::runtime_error("Failed to extract info section: invalid start");
    }

    size_t infoEnd = skipElement(data, infoStart);
    return data.substr(infoStart, infoEnd - infoStart);
}

std::string computeInfoHash(const std::string& rawInfoSection)
{
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(rawInfoSection.data()), rawInfoSection.size(), hash);

    return std::string(reinterpret_cast<char*>(hash), SHA_DIGEST_LENGTH);
}

Metadata fillMetadata(const std::string& torrentFilePath)
{
    std::ifstream ifs(torrentFilePath, std::ios::binary);
    if (!ifs)
    {
        throw std::runtime_error("Failed to open torrent file");
    }
    std::string data((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

    Metadata meta;

    meta.infoHash = computeInfoHash(extractRawInfoSection(data));

    Bencode::Parser parser(data);
    Bencode::Value value;
    try
    {
        value = parser.parse();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Failed to parse torrent file: " + std::string(e.what()));
    }
    catch (...)
    {
        throw std::runtime_error("Failed to parse torrent file");
    }

    auto dict = value.asDict();

    if (dict.contains("announce"))
    {
        meta.announce = dict["announce"].asStr();
    }

    auto info = dict["info"].asDict();
    if (info.contains("name"))
    {
        meta.name = info["name"].asStr();
    }

    if (info.contains("piece length"))
    {
        meta.pieceLength = info["piece length"].asInt();
    }

    if (info.contains("pieces"))
    {
        std::string raw = info["pieces"].asStr();
        for (size_t i = 0; i < raw.size(); i += 20)
        {
            meta.pieceHashes.push_back(raw.substr(i, 20));
        }
    }

    if (info.contains("length"))
    {
        meta.totalSize = info["length"].asInt();
        meta.files.push_back({meta.name, meta.totalSize});
    }
    else if (info.contains("files"))
    {
        auto filesList = info["files"].asList();

        for (const auto& file : filesList)
        {
            auto fileDict = file.asDict();
            auto len      = fileDict["length"].asInt();
            auto pathList = fileDict["path"].asList();
            std::string path;
            for (const auto& pathPart : pathList)
            {
                path += pathPart.asStr() + "/";
            }

            if (!path.empty())
            {
                path.pop_back();
            }

            meta.files.push_back({path, len});
            meta.totalSize += len;
        }
    }
    return meta;
}
}  // namespace Torrent::Utils
