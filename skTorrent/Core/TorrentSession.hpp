#ifndef TORRENTSESSION_HPP
#define TORRENTSESSION_HPP

#include <Utils/MetaUtils.hpp>
#include <thread>

namespace Torrent::Core {

class TorrentSession
{
public:
    explicit TorrentSession(const std::string& peerId, const std::string& filePath);
    std::string getAnnounceRequest();

    void prepareSession();
    void start();
    void stop();
    void status();

private:
    Metadata m_meta;
    std::jthread m_thread;
    std::string m_filePath;
    std::string m_peerId;
};
}  // namespace Torrent::Core
#endif  // TORRENTSESSION_HPP
