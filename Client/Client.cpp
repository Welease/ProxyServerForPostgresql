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
    _request->addData(data, size);
    _phase = sendRequest;
    _numOfRequest++;
    if (_numOfRequest == 1)
        parseStartupData();
    else if (_numOfRequest > 2) makeLog();
}

void Client::makeLog() {
    struct tm*	local;
    time_t		time_;
    char		timeBuff[100];
    size_t		timeLen;

    time_ = time(nullptr);
    local = localtime(&time_);
    timeLen = strftime(timeBuff, 80, "%F %X ", local);
    unsigned char *tmp = _request->toPointer();
    _packetLength = std::to_string(int(int(tmp[0]) | int(tmp[1]) << 8 | int(tmp[2]) << 16));
    _protocol = protoTypes.find(tmp[4])->second;
    write(_logFd, "FROM: ", 6);
    for (size_t i = 0; i < _userName.length(); ++i) {
        if (_userName[i] != '\0') write(_logFd, &_userName[i], 1);
    }
    write(_logFd, "\n", 1);
    write(_logFd, "DATABASE NAME: ", 15);
    for (size_t i = 0; i < _dbName.length(); ++i) {
        if (_dbName[i] != '\0') write(_logFd, &_dbName[i], 1);
    }
    write(_logFd, "\n", 1);
    write(_logFd, "DATE: ", 6);
    write(_logFd, timeBuff, timeLen);
    write(_logFd, "\n", 1);
    write(_logFd, "PAYLOAD LENGTH: ", 16);
    write(_logFd, _packetLength.c_str(), _packetLength.length());
    write(_logFd, "\n", 1);
    write(_logFd, "SEQUENCE ID: ", 13);
    write(_logFd, _protocol.c_str(), _protocol.length());
    write(_logFd, "\n", 1);
    for (size_t i = 5; i < _request->getDataSize() - 1; ++i) {
        write(_logFd, &tmp[i], 1);
        if (tmp[i] == ';' && i != _request->getDataSize() - 1) write(_logFd, "\n", 1);
    }
    write(_logFd, "\n\n\n", 3);
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

    protoTypes.insert(std::pair<unsigned char, std::string>('Q', "COM_QUERY"));
    protoTypes.insert(std::pair<unsigned char, std::string>(3, "COM_QUERY"));
    protoTypes.insert(std::pair<unsigned char, std::string>(0, "COM_SLEEP"));
    protoTypes.insert(std::pair<unsigned char, std::string>(1, "COM_QUIT"));
    protoTypes.insert(std::pair<unsigned char, std::string>(5, "COM_CREATE_DB"));
    protoTypes.insert(std::pair<unsigned char, std::string>(6, "COM_DROP_DB"));
    protoTypes.insert(std::pair<unsigned char, std::string>(9, "COM_STATISTICS"));
}
