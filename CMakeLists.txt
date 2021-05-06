cmake_minimum_required(VERSION 3.17)
project(proxy_server_for_DBMS)

set(CMAKE_CXX_STANDARD 17)

add_executable(proxy
                main.cpp
                ProxyServer/ProxyServer.cpp
                ProxyServer/ProxyServer.h
                Client/Client.cpp
                Client/Client.h
                DataChunks/DataChunks.cpp
                DataChunks/DataChunks.h
                DbConnector/DbConnector.cpp
                DbConnector/DbConnector.h)