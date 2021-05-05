#include "DataChunks.h"

DataChunks::DataChunks() : _data(new std::list< std::pair<size_t, unsigned char *> >()), _dataSize(0) { }

DataChunks::~DataChunks() {
    clear();
    delete _data;
}

void DataChunks::addData(unsigned char *data, size_t size){
    auto *tmp = (unsigned char *)malloc(size);
    memmove(tmp, data, size);
    _data->push_back(std::pair<size_t, unsigned char *>(size, tmp));
    _dataSize += size;
}

unsigned char *DataChunks::toPointer() {
    size_t i = 0;
    auto *ret = (unsigned char *)malloc(_dataSize);
    for (auto & it : *_data){
        memmove(ret + i, it.second, it.first);
        i += it.first;
    }
    return ret;
}

void DataChunks::clear() {
    for (auto & i : *_data)
        free(i.second);
    _data->clear();
    _dataSize = 0;
}

size_t DataChunks::findDataFragment(unsigned char const *toFind, size_t len){
    size_t ret;
    unsigned char *data = toPointer();
    auto *tmp = (unsigned char *)memmem(data, _dataSize, toFind, len);
    if (!tmp) {
        free(data);
        return NOT_FOUND;
    }
    ret = tmp - data;
    free(data);
    return ret;
}

DataChunks & DataChunks::operator=(const DataChunks &bytes) {
    clear();
    delete _data;
    _data = bytes._data;
    _dataSize = bytes._dataSize;
    return *this;
}
