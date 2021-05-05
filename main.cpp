#include "ProxyServer/ProxyServer.h"

int main() {
    std::string ip = "127.0.0.1";
    int port = 8000;
    int fdLog = open("/home/tanzilya/TCP_server_for_DBMS/file", O_CREAT | O_RDWR | 0666);
    auto *proxyServer = new ProxyServer(ip, port, fdLog);
    proxyServer->initialization();
    proxyServer->startMainLoop();
}
