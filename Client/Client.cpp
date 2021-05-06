#include <cmath>
#include "Client.h"

Client::Client(int & sock, int & dbPort, std::string & dbIp, int & logFd) {
    _socket = sock;
    _dbPort = dbPort;
    _dbIp = dbIp;
    _logFd = logFd;
    _request = new DataChunks();
    _response = new DataChunks();
    _phase = start;
    _sendBytes = 0;
    fcntl(_socket, F_SETFL, O_NONBLOCK);
    _dbConnector = new DbConnector(_dbPort, _dbIp);
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

void Client::addRequest(unsigned char *data, size_t size) {
    _request->addData(data, size);
    _phase = sendRequest;
    _numOfRequest++;
    makeLog();
}

int Client::getIntFromHex(unsigned char *hex) {
    return (int(hex[0]) * int(std::pow(16, 4)) + int(hex[1]) * int(std::pow(16, 3)) +
            int(hex[2]) * int(std::pow(16, 2)) + int(hex[3]));
}

void Client::writeInfo(const char *header, size_t len1, const char *message, size_t len) const {
    write(_logFd, header, len1);
    for (size_t i = 0; i < len; ++i)
        if (message[i] != '\0') write(_logFd, &message[i], 1);
    write(_logFd, "\n", 1);
}

void Client::makeLog() {
    char		timeBuff[100];

    time_t time_ = time(nullptr);
    unsigned char *tmp = _request->toPointer();
    struct tm * local = localtime(&time_);
    size_t timeLen = strftime(timeBuff, 80, "%F %X ", local);
    _packetLength = std::to_string(getIntFromHex(_numOfRequest == 1 ? tmp : (tmp + 1)));
    auto key = messageTypes.find(tmp[0]);
    if (key != messageTypes.end())
        _protocol = key->second;
    if (_numOfRequest == 1) {
        _protocol = messageTypes.find('-')->second;
    }
    writeInfo("DATE: ", 6, timeBuff, timeLen);
    writeInfo("PAYLOAD LENGTH: ", 16, _packetLength.c_str(), _packetLength.length());
    writeInfo("MESSAGE TYPE: ", 14, _protocol.c_str(), _protocol.length());
    for (size_t i = _numOfRequest == 1 ? 8 : 5; i < _request->getDataSize() - 1; ++i) {
        write(_logFd, &tmp[i], 1);
        if (tmp[i] == ';' && i != _request->getDataSize() - 1) write(_logFd, "\n", 1);
    }
    write(_logFd, "\n------------------------------------------------\n", 50);
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
    messageTypes.insert(std::pair<unsigned char, std::string>('H', "Flush"));
    messageTypes.insert(std::pair<unsigned char, std::string>('P', "Parse"));
    messageTypes.insert(std::pair<unsigned char, std::string>('D', "Describe"));
    messageTypes.insert(std::pair<unsigned char, std::string>('F', "FunctionCall"));
    messageTypes.insert(std::pair<unsigned char, std::string>('Q', "Query"));
    messageTypes.insert(std::pair<unsigned char, std::string>('-', "StartupMessage"));
    messageTypes.insert(std::pair<unsigned char, std::string>('p', "PasswordMessage"));
    messageTypes.insert(std::pair<unsigned char, std::string>('S', "Sync"));
    messageTypes.insert(std::pair<unsigned char, std::string>('X', "Terminate"));
}
