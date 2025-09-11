#include "TorrentSession.hpp"
#include "RequestBuilder.hpp"
#include <random>
#include <iostream>

Torrent::TorrentSession::TorrentSession(const std::string& filePath)
    : m_filePath(filePath)
{
    m_meta = Utils::fillMetadata(m_filePath);
}

std::string generatePeerId()
{
    std::string peerId = "-SK0001-";  // префикс клиента (SK = SkeBorrent)
    std::mt19937 rng{std::random_device{}()};
    for (int i = peerId.size(); i < 20; i++)
    {
        peerId.push_back('A' + (rng() % 26));
    }
    return peerId;
}

std::string Torrent::TorrentSession::getAnnounceRequest()
{
    RequestBuilder builder;
    builder.setUrl(m_meta.announce);

    std::string peerId   = generatePeerId();
    auto encodedInfoHash = Utils::urlEncode(m_meta.infoHash);
    auto encodedPeerId   = Utils::urlEncode(peerId);
    builder.addParameter("info_hash", encodedInfoHash);
    builder.addParameter("peer_id", encodedPeerId);
    builder.addParameter("port", "6'881");
    builder.addParameter("downloaded", "0");
    builder.addParameter("uploaded", "0");
    builder.addParameter("left", std::to_string(m_meta.totalSize));
    builder.addParameter("event", "started");

    auto request = builder.build();  // Le magic

    return request;
}
