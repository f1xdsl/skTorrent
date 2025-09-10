#include <Utils/MetaUtils.hpp>

#include <gtest/gtest.h>
#include <openssl/sha.h>
#include <openssl/crypto.h>
#include <filesystem>
#include <fstream>

TEST(TorrentMetaExtractInfoTest, ExtractSingleFileInfo)
{
    // простой торрент: словарь { "announce": "...", "info": { "name": "test", "piece length": 16384 } }
    std::string torrent = "d8:announce13:http://tracker4:infod4:name4:test12:piece lengthi16384eee";

    std::string info = Torrent::Utils::extractRawInfoSection(torrent);

    EXPECT_TRUE(info.starts_with("d"));
    EXPECT_TRUE(info.ends_with("e"));
    EXPECT_NE(info.find("4:name4:test"), std::string::npos);
    EXPECT_NE(info.find("12:piece lengthi16384e"), std::string::npos);
}

TEST(TorrentMetaExtractInfoTest, ExtractMultiFileInfo)
{
    std::string torrent =
        "d4:infod5:filesld6:lengthi12345e4:pathl8:file1.txteee"
        "4:name10:multi_test12:piece lengthi32768eee";

    std::string info = Torrent::Utils::extractRawInfoSection(torrent);

    EXPECT_TRUE(info.starts_with("d"));
    EXPECT_TRUE(info.ends_with("e"));
    EXPECT_NE(info.find("5:files"), std::string::npos);
    EXPECT_NE(info.find("4:name10:multi_test"), std::string::npos);
    EXPECT_NE(info.find("12:piece lengthi32768e"), std::string::npos);
}

TEST(TorrentMetaExtractInfoTest, ExtractNestedDictAndList)
{
    // словарь в списке внутри info
    std::string torrent =
        "d4:infod4:listld6:lengthi10e4:pathl8:file1.txteee"
        "3:str5:helloe"
        "e";

    std::string info = Torrent::Utils::extractRawInfoSection(torrent);

    EXPECT_TRUE(info.starts_with("d"));
    EXPECT_TRUE(info.ends_with("e"));
    EXPECT_NE(info.find("4:listl"), std::string::npos);
    EXPECT_NE(info.find("3:str5:hello"), std::string::npos);
}

TEST(TorrentMetaExtractInfoTest, ThrowsIfNoInfo)
{
    std::string torrent = "d4:name4:teste";
    EXPECT_THROW(Torrent::Utils::extractRawInfoSection(torrent), std::runtime_error);
}

TEST(TorrentMetaExtractInfoTest, ThrowsIfInfoNotDict)
{
    std::string torrent = "d4:info4:spam e";
    EXPECT_THROW(Torrent::Utils::extractRawInfoSection(torrent), std::runtime_error);
}

TEST(TorrentMetaInfoHashTest, ComputeInfoHashMatchesOpenSSL)
{
    std::string rawInfo = "d4:name4:test12:piece lengthi16384ee";

    std::string infoHash = Torrent::Utils::computeInfoHash(rawInfo);
    ASSERT_EQ(infoHash.size(), SHA_DIGEST_LENGTH);

    unsigned char expected[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(rawInfo.data()), rawInfo.size(), expected);

    EXPECT_EQ(0, memcmp(infoHash.data(), expected, SHA_DIGEST_LENGTH));
}

TEST(TorrentMetaInfoHashTest, ComputeInfoHashIsDeterministic)
{
    std::string rawInfo = "d4:name5:file1e";

    auto h1 = Torrent::Utils::computeInfoHash(rawInfo);
    auto h2 = Torrent::Utils::computeInfoHash(rawInfo);

    EXPECT_EQ(h1, h2);
}

static std::string encStr(const std::string& s)
{
    return std::to_string(s.size()) + ":" + s;
}

static std::string writeTempTorrent(const std::string& data)
{
    auto dir  = std::filesystem::temp_directory_path();
    auto path = dir / ("sk_torrent_test_" + std::to_string(std::rand()) + ".torrent");
    std::ofstream ofs(path, std::ios::binary);
    ofs.write(data.data(), static_cast<std::streamsize>(data.size()));
    ofs.close();
    return path.string();
}

TEST(TorrentMetaFillTest, SingleFileTorrent)
{
    std::string announce = "http://tracker";
    std::string pieces   = std::string(20, 'A') + std::string(20, 'B');

    std::string info =
        "d"
        "6:length"
        "i"
        "12345"
        "e"
        "4:name" +
        encStr("test") +
        "12:piece length"
        "i"
        "16384"
        "e"
        "6:pieces" +
        std::to_string(pieces.size()) + ":" + pieces + "e";

    std::string torrent =
        "d"
        "8:announce" +
        encStr(announce) + "4:info" + info + "e";

    auto path = writeTempTorrent(torrent);
    auto md   = Torrent::Utils::fillMetadata(path);

    EXPECT_EQ(md.announce, announce);
    EXPECT_EQ(md.name, "test");
    EXPECT_EQ(md.pieceLength, 16'384);
    ASSERT_EQ(md.pieceHashes.size(), 2u);
    EXPECT_EQ(md.pieceHashes[0].size(), 20u);
    EXPECT_EQ(md.pieceHashes[1].size(), 20u);
    EXPECT_EQ(md.pieceHashes[0], std::string(20, 'A'));
    EXPECT_EQ(md.pieceHashes[1], std::string(20, 'B'));
    EXPECT_EQ(md.totalSize, 12'345);

    unsigned char expected[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(info.data()), info.size(), expected);
    std::string expectedHash(reinterpret_cast<char*>(expected), SHA_DIGEST_LENGTH);
    EXPECT_EQ(md.infoHash, expectedHash);
}

TEST(TorrentMetaFillTest, MultiFileTorrent)
{
    std::string announce = "http://tracker2";
    std::string pieces   = std::string(20, 'Z');

    std::string file1 =
        "d"
        "6:length"
        "i"
        "100"
        "e"
        "4:path"
        "l" +
        encStr("dir") + encStr("file1.txt") +
        "e"
        "e";

    std::string file2 =
        "d"
        "6:length"
        "i"
        "200"
        "e"
        "4:path"
        "l" +
        encStr("file2.bin") +
        "e"
        "e";

    std::string info =
        "d"
        "5:files"
        "l" +
        file1 + file2 +
        "e"
        "4:name" +
        encStr("multi") +
        "12:piece length"
        "i"
        "32768"
        "e"
        "6:pieces" +
        std::to_string(pieces.size()) + ":" + pieces + "e";

    std::string torrent =
        "d"
        "8:announce" +
        encStr(announce) + "4:info" + info + "e";

    auto path = writeTempTorrent(torrent);
    auto md   = Torrent::Utils::fillMetadata(path);

    EXPECT_EQ(md.announce, announce);
    EXPECT_EQ(md.name, "multi");
    EXPECT_EQ(md.pieceLength, 32'768);
    ASSERT_EQ(md.pieceHashes.size(), 1u);
    EXPECT_EQ(md.pieceHashes[0], pieces);
    EXPECT_EQ(md.totalSize, 300);
    ASSERT_GE(md.files.size(), 2u);

    unsigned char expected[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(info.data()), info.size(), expected);
    std::string expectedHash(reinterpret_cast<char*>(expected), SHA_DIGEST_LENGTH);
    EXPECT_EQ(md.infoHash, expectedHash);
}

TEST(TorrentMetaFillTest, MissingFileThrows)
{
    EXPECT_THROW(Torrent::Utils::fillMetadata("/this/path/does/not/exist.torrent"), std::runtime_error);
}

TEST(TorrentMetaFillTest, InvalidBencodeThrows)
{
    std::string bad = "xyz";
    auto path       = writeTempTorrent(bad);
    EXPECT_THROW(Torrent::Utils::fillMetadata(path), std::runtime_error);
}
