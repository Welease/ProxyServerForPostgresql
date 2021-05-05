#ifndef TCP_SERVER_FOR_DBMS_DBCONNECTOR_H
#define TCP_SERVER_FOR_DBMS_DBCONNECTOR_H

#include "../ProxyServer/ProxyServer.h"

class DbConnector {
private:
    sockaddr_in   _servAddr;
    int           _socket;

public:
    DbConnector();
    ~DbConnector();

    DataChunks* getResponseFromDb();
    void        sendResponseToBd(DataChunks * request);
    int         getSocket();
};


#endif
