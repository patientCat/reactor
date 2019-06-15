#pragma once

#include <vector>


class Buffer
{
public:
    Buffer(int initSize = 1024);
    int recv(char*addr, int len);
    int addPacketLength(int len);
private:
    int start_;
    int toRead_;
    int toWrite_;
    int end_;
    std::vector<char> data_;
};

