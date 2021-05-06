#ifndef TCP_SERVER_FOR_DBMS_PROXYSERVER_H
#define TCP_SERVER_FOR_DBMS_PROXYSERVER_H

#include <iostream>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <vector>
#include <list>
#include "../Client/Client.h"

# define DEFAULT "\e[39m\e[0m"
# define GREEN "\e[92m"
# define RED  "\e[31m"
# define YELLOW "\e[1;33m"
# define BLUE "\e[1;34m"
# define BUF_SIZE 1000000
class Client;

class ProxyServer {

private:
    std::string     _ip;
    std::string     _dbIp;
    int             _port;
    int             _dbPort;
    sockaddr_in     _sockAddr;
    socklen_t       _addrLen;
    int             _socket;
    int             _flag;
    int             _fdLog;


public:
    ProxyServer(std::string & ip, std::string & dbIp,  int & port, int & dbPort, int & fd);
    ~ProxyServer();

    void initialization();
    void startMainLoop();

    class initErrorException : public std::exception {
    public :
        virtual const char * what() const throw();
    };

    class proxyServerRunningException : public std::exception {
    public :
        virtual const char * what() const throw();
    };

private:
    void        fdsPreparing(fd_set & writeSet, fd_set & readSet, int & maxFd);
    void        settingToFdSets(std::vector<Client *> & clients, int & maxFd, fd_set & writeSet, fd_set & readSet);
    void        addNewClients(fd_set & readSet, std::vector<Client *> & clients);
    void        requestProcessing(std::vector<Client *> & clients, fd_set & readSet, fd_set & writeSet);
    void        sendingResponse(Client * & client);
    void        closingConnections(std::vector<Client *> & clients);

};

#endif
