#ifndef METAUTILS_HPP
#define METAUTILS_HPP

#include <string>
#include <cstdint>
#include <vector>

namespace Torrent {
struct Metadata
{
    struct FileEntry
    {
        std::string path;
        uint64_t size = 0;
    };

    std::string announce;
    std::string name;

    uint64_t pieceLength = 0;
    uint64_t totalSize   = 0;

    std::vector<std::string> pieceHashes;
    std::vector<FileEntry> files;
    std::string infoHash;
};

namespace Utils {

std::string urlEncode(const std::string& str);
Metadata fillMetadata(const std::string& torrentFilePath);
std::string extractRawInfoSection(const std::string& data);
std::string computeInfoHash(const std::string& rawInfoSection);
size_t skipElement(const std::string& data, size_t pos);

}  // namespace Utils
}  // namespace Torrent
#endif  // METAUTILS_HPP
