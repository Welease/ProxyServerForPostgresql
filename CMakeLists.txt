cmake_minimum_required(VERSION 3.17)
project(TCP_server_for_DBMS)

set(CMAKE_CXX_STANDARD 17)

add_executable(TCP_server_for_DBMS main.cpp ProxyServer/ProxyServer.cpp ProxyServer/ProxyServer.h Client/Client.cpp Client/Client.h DataChunks/DataChunks.cpp DataChunks/DataChunks.h DBconnecter/DbConnecter.cpp DBconnecter/DbConnecter.h)