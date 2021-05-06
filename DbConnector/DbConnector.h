#ifndef TCP_SERVER_FOR_DBMS_DBCONNECTOR_H
#define TCP_SERVER_FOR_DBMS_DBCONNECTOR_H

#include "../ProxyServer/ProxyServer.h"

class DbConnector {
private:
    sockaddr_in   _servAddr;
    int           _socket;

public:
    DbConnector(int & port, std::string & host);
    ~DbConnector();

    DataChunks* getResponseFromDb() const;
    void        sendResponseToBd(DataChunks * request) const;
    int         getSocket() const;
};


#endif
