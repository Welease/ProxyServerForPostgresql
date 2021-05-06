#include <cmath>
#include "Client.h"

Client::Client(int &sock, int & logFd) {
    _socket = sock;
    _logFd = logFd;
    _request = new DataChunks();
    _response = new DataChunks();
    _phase = start;
    _sendBytes = 0;
    fcntl(_socket, F_SETFL, O_NONBLOCK);
    _dbConnector = new DbConnector();
    _numOfRequest = 0;
    fillTypesMap();
}

void Client:: clear() {
    delete _request;
    delete _response;
    _phase = start;
    _sendBytes = 0;

    _request = new DataChunks();
    _response = new DataChunks();
}

Client::~Client() {
    delete _response;
    delete _request;
    _phase = start;
    _sendBytes = 0;
}

void Client::parseStartupData() {
    const char * tmp = reinterpret_cast<const char *>(_request->toPointer());

    size_t endOfUserName = _request->findDataFragment(reinterpret_cast<const unsigned char *>(" - "), 3);
    if (endOfUserName != NOT_FOUND) {
        _userName = std::string(tmp + 8, endOfUserName - 8);
        std::cout << RED << _userName << std::endl;
        _dbName = std::string(tmp + endOfUserName + 6, _request->getDataSize() - endOfUserName - 5);
        std::cout << BLUE << _dbName << DEFAULT << std::endl;
    }
    free((void *) tmp);
}

void Client::addRequest(unsigned char *data, size_t size) {
    std::cout << data[0] << std::endl;
    _request->addData(data, size);
    _phase = sendRequest;
    _numOfRequest++;
    if (_numOfRequest == 1)
        parseStartupData();
    else makeLog();
}

int Client::getIntFromHex(unsigned char *hex) {
    std::cout << int(hex[0]) << " " << int(hex[1]) << " " << int(hex[2]) << " " << int(hex[3]) << std::endl;
    return (int(hex[0]) * int(std::pow(16, 4)) + int(hex[1]) * int(std::pow(16, 3)) +
            int(hex[2]) * int(std::pow(16, 2)) + int(hex[3]));
}

void Client::writeInfo(const char *header, size_t len1, const char *message, size_t len) {
    write(_logFd, header, len1);
    for (size_t i = 0; i < len; ++i)
        if (message[i] != '\0') write(_logFd, &message[i], 1);
    write(_logFd, "\n", 1);
}

void Client::makeLog() {
    char		timeBuff[100];

    time_t time_ = time(nullptr);
    struct tm * local = localtime(&time_);
    size_t timeLen = strftime(timeBuff, 80, "%F %X ", local);
    unsigned char *tmp = _request->toPointer();
    _packetLength = std::to_string(getIntFromHex(tmp + 1));
    std::cout << "PACK length: " << _packetLength << std::endl;
    auto key = messageTypes.find(tmp[4]);
    if (key != messageTypes.end())
        _protocol = key->second;
    writeInfo("FROM: ", 6, _userName.c_str(), _userName.length());
    writeInfo("DATABASE NAME: ", 15, _dbName.c_str(), _dbName.length());
    writeInfo("DATE: ", 6, timeBuff, timeLen);
    writeInfo("PAYLOAD LENGTH: ", 16, _packetLength.c_str(), _packetLength.length());
    writeInfo("MESSAGE TYPE: ", 14, _protocol.c_str(), _protocol.length());
    for (size_t i = 5; i < _request->getDataSize() - 1; ++i) {
        write(_logFd, &tmp[i], 1);
        if (tmp[i] == ';' && i != _request->getDataSize() - 1) write(_logFd, "\n", 1);
    }
    write(_logFd, "--------------------------------------------------", 50);
    free(tmp);
}

void Client::sendRequestToDb() {
    _dbConnector->sendResponseToBd(_request);
    _phase = recieveResponse;
}

void Client::getResponseFromDb() {
    _response = _dbConnector->getResponseFromDb();
    _phase = sendResponse;
}


DataChunks * Client::getResponse() const { return _response; }

int Client::getPhase() const { return _phase; }

int Client::getSocket() const { return _socket; }

size_t Client::getSendBytes() const { return _sendBytes; }

void Client::setPhase(int phase) { _phase = phase; }

void Client::setSendBytes(size_t n) { _sendBytes = n; }

int Client::getDbSocket() const { return _dbConnector->getSocket(); }

void Client::fillTypesMap() {
    messageTypes.insert(std::pair<unsigned char, std::string>('B', "Bind"));
    messageTypes.insert(std::pair<unsigned char, std::string>('C', "Close"));
    messageTypes.insert(std::pair<unsigned char, std::string>('d', "CopyData"));
    messageTypes.insert(std::pair<unsigned char, std::string>('c', "CopyDone"));
    messageTypes.insert(std::pair<unsigned char, std::string>('f', "CopyFail"));
    messageTypes.insert(std::pair<unsigned char, std::string>('E', "Execute"));
    messageTypes.insert(std::pair<unsigned char, std::string> ('H', "Flush"));
    messageTypes.insert(std::pair<unsigned char, std::string>('P', "Parse"));
    messageTypes.insert(std::pair<unsigned char, std::string>('D', "Describe"));
    messageTypes.insert(std::pair<unsigned char, std::string>('F', "FunctionCall"));
    messageTypes.insert(std::pair<unsigned char, std::string>('Q', "Query"));
    messageTypes.insert(std::pair<unsigned char, std::string>('-', "StartupMessage"));
    messageTypes.insert(std::pair<unsigned char, std::string>('p', "PasswordMessage"));
    messageTypes.insert(std::pair<unsigned char, std::string>('S', "Sync"));
    messageTypes.insert(std::pair<unsigned char, std::string>('X', "Terminate"));

}
