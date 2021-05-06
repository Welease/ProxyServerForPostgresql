#include "ProxyServer.h"

int working = 1;

ProxyServer::ProxyServer(std::string & ip, int & port, int & fd) : _ip(ip),
                                                                   _port(port),
                                                                   _socket(-1),
                                                                   _flag(true),
                                                                   _fdLog(fd){
}

ProxyServer::~ProxyServer() = default;

void ProxyServer::initialization() {
    _socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    _addrLen = (sizeof(_sockAddr));
    _sockAddr.sin_family = PF_INET;
    _sockAddr.sin_addr.s_addr = inet_addr(_ip.c_str());
    _sockAddr.sin_port = htons(_port);

    if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &_flag, sizeof(int)) ||
        bind(_socket, (const struct sockaddr *) &_sockAddr, _addrLen) ||
        fcntl(_socket, F_SETFL, O_NONBLOCK) ||
        listen(_socket, 2048)) {
        throw initErrorException();
    }
}

void ProxyServer::fdsPreparing(fd_set & writeSet, fd_set & readSet, int & maxFd) {
    FD_ZERO(&writeSet);
    FD_ZERO(&readSet);
    FD_SET(_socket, &readSet);
    maxFd = _socket;
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
    if (FD_ISSET(_socket, &readSet)) {
        fd = accept(_socket, 0, 0);
        if (fd > 0) {
            clients.push_back(new Client(fd, _fdLog));
            std::cout << YELLOW << "New client successfully connected" << DEFAULT << std::endl;
        }
    }
}

void ProxyServer::sendingResponse(Client *&client) {
    unsigned char *  response;
    ssize_t ret;

    response = client->getResponse()->toPointer();
    ret = write(client->getSocket(), response + client->getSendBytes(), client->getResponse()->getDataSize() - client->getSendBytes());
    if (ret <= 0) {
        client->setPhase(closing);
        return;
    }
    client->setSendBytes(client->getSendBytes() + ret);
    free(response);
    if (client->getSendBytes() == client->getResponse()->getDataSize()) {
        std::cout << GREEN << "Response successfully send" << DEFAULT << std::endl;
        client->clear();
    }
}

void ProxyServer::requestProcessing(std::vector<Client *> &clients, fd_set &readSet, fd_set &writeSet) {
    unsigned char    buf[BUF_SIZE];
    ssize_t ret;

    for (auto & client : clients) {
        if (FD_ISSET(client->getSocket(), &readSet)) {
            ret = recv(client->getSocket(), buf, BUF_SIZE, 0);
            std::cout << BLUE << "Got new request, start to processing..." << DEFAULT << std::endl;
            if (ret <= 0) client->setPhase(closing);
            else client->addRequest(buf, ret);
        }
        if (client->getPhase() == sendRequest && FD_ISSET(client->getDbSocket(), &writeSet))
            client->sendRequestToDb();
        else if (client->getPhase() == recieveResponse && FD_ISSET(client->getDbSocket(), &readSet))
            client->getResponseFromDb();
        else if (client->getPhase() == sendResponse && FD_ISSET(client->getSocket(), &writeSet))
            sendingResponse(client);
    }
}

void ProxyServer::closingConnections(std::vector<Client *> &clients) {
    for (auto it = clients.begin(); it != clients.end();) {
        if ((*it)->getPhase() == closing) {
            std::cout << RED << "Connection with client successfully closed" << DEFAULT << std::endl;
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

    std::cout << GREEN << "ProxyServer started working" << DEFAULT << std::endl;
    signal(SIGPIPE, SIG_IGN);

    while (working) {
        fdsPreparing(writeSet, readSet, maxFd);

        settingToFdSets(clients, maxFd, writeSet, readSet);

        if (select(maxFd + 1, &readSet, &writeSet, 0, 0) == -1) {
            //todo exc
        }

        addNewClients(readSet, clients);

        requestProcessing(clients, readSet, writeSet);

        closingConnections(clients);
    }
}


const char * ProxyServer::initErrorException::what() const noexcept{
    return ("Initial error");
}

