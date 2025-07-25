#include "FileClient.h"

int main()
{
    EventLoop loop;
    std::string serverip = "127.0.0.1";
    uint16_t port = 9090;

    // FileUploader uploader(&loop, serverip, port, "ubuntu-24.04.1-desktop-amd64.iso", 
    //                           "/home/hahaha/work/Chatroom/new_project/FTP/upload/ubuntu-24.04.1-desktop-amd64.iso",
    //                           1000, 1000, "Group");
    // uploader.Start();

    FileDownloader downloader(&loop, serverip, port, "ubuntu-24.04.1-desktop-amd64.iso",
                              "/home/hahaha/work/Chatroom/new_project/FTP/download");
    downloader.Start();

    loop.loop();


}