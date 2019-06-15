//c++ program
//design by >^.^<
//@date    2019-06-13 04:45:57
#include "Buffer.hh"
#include <cassert>

Buffer::Buffer(int initSize)
{
    assert(initSize >= 1024);
    end_ = initSize;
    data_.resize(initSize);
    start_ = toRead_ = toWrite_ = 4;
}
