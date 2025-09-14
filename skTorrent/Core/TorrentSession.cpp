#include "TorrentSession.hpp"
#include "RequestBuilder.hpp"
#include <random>
#include <iostream>

#include <Logger.hpp>

namespace Torrent::Core {

TorrentSession::TorrentSession(const std::string& peerId, const std::string& filePath)
    : m_peerId(peerId)
    , m_filePath(filePath)
{
    LOG_INFO(TorrentSession, "Creating torrent session", LOG_MD(FilePath, m_filePath));
}

void TorrentSession::start()
{
    auto path = m_filePath;
    m_thread  = std::jthread(
        [this, path](std::stop_token)
        {
            try
            {
                prepareSession();
            }
            catch (const std::exception& e)
            {
                LOG_CRITICAL(TorrentSession, "Failed to prepare session", LOG_MD(FilePath, path), LOG_MD(Error, e.what()));
            }
        });
    LOG_INFO(TorrentSession, "Session created", LOG_MD(FilePath, m_filePath));
}

void TorrentSession::prepareSession()
{
    m_meta = Utils::fillMetadata(m_filePath);
    getAnnounceRequest();
}

std::string TorrentSession::getAnnounceRequest()
{
    RequestBuilder builder;
    builder.setUrl(m_meta.announce);

    builder.addParameter("info_hash", Utils::urlEncode(m_meta.infoHash));
    builder.addParameter("peer_id", Utils::urlEncode(m_peerId));
    builder.addParameter("port", "6'881");
    builder.addParameter("downloaded", "0");
    builder.addParameter("uploaded", "0");
    builder.addParameter("left", std::to_string(m_meta.totalSize));
    builder.addParameter("event", "started");

    auto request = builder.build();

    return request;
}
}  // namespace Torrent::Core
