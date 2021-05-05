#include "ProxyServer.h"

int working = 1;

ProxyServer::ProxyServer(std::string & ip, int & port, int & fd) : _ip(ip),
                                                        _port(port),
                                                        _fd(-1),
                                                        _flag(true),
                                                        _fdLog(fd){
}

ProxyServer::~ProxyServer() = default;

void ProxyServer::initialization() {
    _fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    _address_len = (sizeof(_socket_addr));
    _socket_addr.sin_family = PF_INET;
    _socket_addr.sin_addr.s_addr = inet_addr(_ip.c_str());
    _socket_addr.sin_port = htons(_port);

    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &_flag, sizeof(int)) ||
        bind(_fd, (const struct sockaddr *) &_socket_addr, _address_len) ||
        fcntl(_fd, F_SETFL, O_NONBLOCK) ||
        listen(_fd, 2048)) {
        std::cout << RED << "ERROR in initialization" << DEFAULT << std::endl;
        exit(1);
    }
}

void ProxyServer::fdsPreparing(fd_set & writeSet, fd_set & readSet, int & maxFd) {
    FD_ZERO(&writeSet);
    FD_ZERO(&readSet);
    FD_SET(_fd, &readSet);
    maxFd = _fd;
}

void ProxyServer::settingToFdSets(std::vector<Client *> &clients, int &maxFd, fd_set &writeSet, fd_set &readSet) {
    for (auto & it : clients ) {
        if (it->getPhase() <= parseRequest) {
            FD_SET(it->getSocket(), &readSet);
            maxFd = (it->getSocket() > maxFd) ? it->getDbSocket() : maxFd;
        }
        else if (it->getPhase() == recieveResponse) {
            maxFd = (it->getDbSocket() > maxFd) ? it->getDbSocket() : maxFd;
            FD_SET(it->getDbSocket(), &readSet);
        }
        else if (it->getPhase() == sendResponse) {
            maxFd = (it->getSocket() > maxFd) ? it->getDbSocket() : maxFd;
            FD_SET(it->getSocket(), &writeSet);
        }
        else if (it->getPhase() == sendRequest) {
            maxFd = (it->getDbSocket() > maxFd) ? it->getDbSocket() : maxFd;
            FD_SET(it->getDbSocket(), &writeSet);
        }
    }
}

void ProxyServer::addNewClients(fd_set &readSet, std::vector<Client *> &clients) {
    int fd;
    if (FD_ISSET(_fd, &readSet)) {
        fd = accept(_fd, 0, 0);
        if (fd > 0) {
            clients.push_back(new Client(fd, _fdLog));
            std::cout << YELLOW << "Client successfully connected!" << DEFAULT << std::endl;
        }
    }
}

void ProxyServer::sendingResponse(Client *&client) {
    unsigned char *  tmp;
    ssize_t ret;

    tmp = client->getResponse()->toPointer();
    ret = write(client->getSocket(), tmp + client->getSendBytes(), client->getResponse()->getDataSize() - client->getSendBytes());
    if (ret <= 0) {
        std::cout << RED << "Error in write" << DEFAULT << std::endl;
        client->setPhase(closing);
        return;
    }
    client->setSendBytes(client->getSendBytes() + ret);
    free(tmp);
    if (client->getSendBytes() == client->getResponse()->getDataSize()) {
        std::cout << GREEN << "Response send" << DEFAULT << std::endl;
        client->clear();
    }
}

void ProxyServer::requestProcessing(std::vector<Client *> &clients, fd_set &readSet, fd_set &writeSet) {
    unsigned char    buf[BUFSIZE];
    ssize_t ret;

    for (auto & it : clients) {
        if (FD_ISSET(it->getSocket(), &readSet)) {
            ret = recv(it->getSocket(), buf, BUFSIZE, 0);
            std::cout << RED << ret << DEFAULT << std::endl;
            if (ret <= 0) it->setPhase(closing);
            else {
                it->addRequest(buf, ret);
                for (int i = 0; i < ret; ++i)
                    std::cout << buf[i];
                std::cout << std::endl;
            }
        }
        if (it->getPhase() == sendRequest && FD_ISSET(it->getDbSocket(), &writeSet))
            it->sendRequestToDb();
        else if (it->getPhase() == recieveResponse && FD_ISSET(it->getDbSocket(), &readSet))
            it->getResponseFromDb();
        else if (it->getPhase() == sendResponse && FD_ISSET(it->getSocket(), &writeSet))
            sendingResponse(it);
    }
}

void ProxyServer::closingConnections(std::vector<Client *> &clients) {
    for (auto it = clients.begin(); it != clients.end();) {
        if ((*it)->getPhase() == closing) {
            std::cout << RED << "Connection with client with fd " << (*it)->getSocket() << " successfully closed" << DEFAULT << std::endl;
            close((*it)->getSocket());
            (*it)->clear();
            delete *it;
            it = clients.erase(it);
        }
        else ++it;
    }
}

void ProxyServer::startMainLoop() {
    std::vector<Client *> clients;
    int                 maxFd;
    fd_set              writeSet;
    fd_set              readSet;

    std::cout << GREEN << "ProxyServer is started" << DEFAULT << std::endl;
    signal(SIGPIPE, SIG_IGN);

    while (working) {
        fdsPreparing(writeSet, readSet, maxFd);

        settingToFdSets(clients, maxFd, writeSet, readSet);

        if (select(maxFd + 1, &readSet, &writeSet, 0, 0) == -1) {
            std::cout << RED << "Select error" << DEFAULT << std::endl;
            exit(2);
        }

        addNewClients(readSet, clients);

        requestProcessing(clients, readSet, writeSet);

        closingConnections(clients);
    }
}


const char * ProxyServer::initErrorException::what() const noexcept{
    return ("Initial error");
}

