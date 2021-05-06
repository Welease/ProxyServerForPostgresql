#include "DbConnector.h"

DbConnector::DbConnector() {
    if ((_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        ;//todo exc
    }
    _servAddr.sin_family = PF_INET;
    _servAddr.sin_port = htons(3306);

    if (inet_pton(AF_INET, "127.0.0.1", &_servAddr.sin_addr) <= 0) {
        ; //todo exc
    }
    if (connect(_socket, (struct sockaddr *)&_servAddr, sizeof(_servAddr)) < 0) {
        ; //todo exc
    }
}

DbConnector::~DbConnector() = default;

void DbConnector::sendResponseToBd(DataChunks *request) {
    unsigned char *req = request->toPointer();
    ssize_t sendRet = send(_socket, req, request->getDataSize(), 0);
    if (sendRet <= 0) {
        ;//todo exc
    }
    free(req);
}

DataChunks * DbConnector::getResponseFromDb() {
    unsigned char buf[10000];

    auto *response = new DataChunks();

    bzero(buf, 1024);
    ssize_t ret = read(_socket, buf, 10000);
    if (ret < 0)
        return response;
    response->addData(buf, ret);
    return response;
}

int DbConnector::getSocket() { return _socket; }