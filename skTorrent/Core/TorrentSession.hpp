#ifndef TORRENTSESSION_HPP
#define TORRENTSESSION_HPP

#include <Utils/MetaUtils.hpp>

namespace Torrent::Core {

class TorrentSession
{
public:
    TorrentSession() = default;
    void start();
    void stop();
    void status();

private:
    Metadata m_meta;
    std::string m_filePath;
};
}  // namespace Torrent::Core
#endif  // TORRENTSESSION_HPP
