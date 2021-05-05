#include "DbConnecter.h"

DbConnecter::DbConnecter() {
    if ((_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        std::cout << RED << "HERE1" << DEFAULT << std::endl; //here
    _servAddr.sin_family = PF_INET;
    _servAddr.sin_port = htons(5432);

    if (inet_pton(AF_INET, "127.0.0.1", &_servAddr.sin_addr) <= 0)
        std::cout << RED << "HERE2" << DEFAULT << std::endl;//here
    if (connect(_fd, (struct sockaddr *)&_servAddr, sizeof(_servAddr)) < 0) {
        std::cout << RED << "Connection failed" << DEFAULT << std::endl;
        exit(1);
    }
}

DbConnecter::~DbConnecter() = default;

void DbConnecter::sendResponseToBd(DataChunks *request) {
    unsigned char *req = request->toPointer();
    int sendRet = send(_fd, req, request->getDataSize(), 0);
    if (sendRet <= 0) {
        std::cout << YELLOW << "send on db error" << DEFAULT << std::endl;
    }
    free(req);
}

DataChunks * DbConnecter::getResponseFromDb() {
    unsigned char buf[10000];

    auto *response = new DataChunks();

    bzero(buf, 1024);
    int ret = read(_fd, buf, 10000);
    if (ret < 0) {
        std::cout << RED << "Db returned -1" << DEFAULT << std::endl;
        return response;
    }
    response->addData(buf, ret);
    return response;
}

int DbConnecter::getFd() { return _fd; }