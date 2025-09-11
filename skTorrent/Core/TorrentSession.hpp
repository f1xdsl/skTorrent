#ifndef TORRENTSESSION_HPP
#define TORRENTSESSION_HPP

#include <Utils/MetaUtils.hpp>

namespace Torrent {

class TorrentSession
{
public:
    explicit TorrentSession(const std::string& filePath);
    std::string getAnnounceRequest();

    void start();
    void stop();
    void status();

private:
    Metadata m_meta;
    std::string m_filePath;
};
}  // namespace Torrent
#endif  // TORRENTSESSION_HPP
