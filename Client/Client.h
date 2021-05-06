#ifndef TCP_SERVER_FOR_DBMS_CLIENT_H
#define TCP_SERVER_FOR_DBMS_CLIENT_H

#include "../DataChunks/DataChunks.h"
#include <fcntl.h>
#include <map>
#include "../DbConnector/DbConnector.h"

class DbConnector;
enum Phase {
    start = 0,
    parseRequest,
    sendRequest,
    recieveResponse,
    sendResponse,
    closing
};

class Client {
private:
    DataChunks *    _request;
    DataChunks *    _response;
    int             _phase;
    int             _socket;
    int             _logFd;
    int             _numOfRequest;
    std::string     _userName;
    std::string     _dbName;
    std::string     _protocol;
    std::string     _packetLength;
    size_t          _sendBytes;

    DbConnector     *_dbConnector;
public:
    std::map<unsigned char, std::string> protoTypes;
    explicit Client(int & sock, int & logFd);
    ~Client();

    DataChunks	* getResponse()     const;
    int			  getPhase()        const;
    int           getSocket()       const;
    int           getDbSocket()     const;
    size_t        getSendBytes()    const;

    void          clear();
    void          setPhase(int phase);
    void          setSendBytes(size_t n);
    void          parseStartupData();
    void          addRequest(unsigned char *data, size_t size);
    void          makeLog();
    void          sendRequestToDb();
    void          getResponseFromDb();
    void          fillTypesMap();
};


#endif
