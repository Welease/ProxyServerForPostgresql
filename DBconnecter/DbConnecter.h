#ifndef TCP_SERVER_FOR_DBMS_DBCONNECTER_H
#define TCP_SERVER_FOR_DBMS_DBCONNECTER_H

#include "../ProxyServer/ProxyServer.h"

class DbConnecter {
private:
    sockaddr_in   _servAddr;
    int           _fd;

public:
    DbConnecter();
    ~DbConnecter();

    DataChunks* getResponseFromDb();
    void        sendResponseToBd(DataChunks * request);
    int getFd();
};


#endif
