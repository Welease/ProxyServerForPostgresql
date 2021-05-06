#include "DbConnector.h"

DbConnector::DbConnector(int & port, std::string & host) {
    if ((_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        throw ProxyServer::initErrorException();
    }
    _servAddr.sin_family = PF_INET;
    _servAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &_servAddr.sin_addr) <= 0) {
        throw ProxyServer::initErrorException();
    }
    if (connect(_socket, (struct sockaddr *)&_servAddr, sizeof(_servAddr)) < 0) {
        throw ProxyServer::initErrorException();
    }
}

DbConnector::~DbConnector() = default;

void DbConnector::sendResponseToBd(DataChunks *request) const {
    unsigned char *req = request->toPointer();
    ssize_t sendRet = send(_socket, req, request->getDataSize(), 0);
    if (sendRet <= 0) {
        throw ProxyServer::proxyServerRunningException();
    }
    free(req);
}

DataChunks * DbConnector::getResponseFromDb() const {
    unsigned char buf[BUF_SIZE];

    auto *response = new DataChunks();

    bzero(buf, 1024);
    ssize_t ret = read(_socket, buf, BUF_SIZE);
    if (ret < 0)
        return response;
    response->addData(buf, ret);
    return response;
}

int DbConnector::getSocket() const { return _socket; }