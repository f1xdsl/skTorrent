#include <iostream>
#include <Core/TorrentSession.hpp>

int main()
{
    std::string filePath = "/home/f1xdsl/dev/skeborrent/build/cms14.torrent";
    Torrent::TorrentSession torrent(filePath);
    auto request = torrent.getAnnounceRequest();
    std::cout << request << std::endl;
    return 0;
}
