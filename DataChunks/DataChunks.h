#ifndef TCP_SERVER_FOR_DBMS_DATACHUNKS_H
#define TCP_SERVER_FOR_DBMS_DATACHUNKS_H

#include <list>
#include <iostream>
#include <string.h>
#define  NOT_FOUND (size_t)-1

class DataChunks {
private:
    std::list< std::pair<size_t, unsigned char *> >*_data;
    size_t _dataSize;

public:
    DataChunks();
    ~DataChunks();

    void                 addData(unsigned char *data, size_t size);
    size_t               findDataFragment(unsigned char const *toFind, size_t len);
    size_t               getDataSize() const {  return _dataSize; }
    unsigned char *      toPointer();
    void                 clear();

    DataChunks & operator=(DataChunks const & bytes);
};


#endif
