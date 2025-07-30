#include "Buffer.h"

#include <bits/types/struct_iovec.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>

const char Buffer::kCRLF[] = "\r\n";

ssize_t Buffer::readFd(int fd, int* saveErrno)
{
    char extrabuf[65536] = {0};  //栈上临时缓冲区，64k
    struct iovec vec[2];  //readv参数，表示一块内存区域
    const size_t writable = writableBytes();  //buffer可写大小

    //第一块内存使用buffer的可写区块
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;

    //第二块内存使用临时缓冲区
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    //比较buffer和extrabuf的大小，来决定使用一块还是两块内存
    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    //读取数据
    const ssize_t n = ::readv(fd, vec, iovcnt);

    //根据读取的数据量调整结果
    if (n < 0)  //报错
    {
        *saveErrno = errno;
    }
    else if (n <= writable)  //未使用extrabuf
    {
        writerIndex_ += n;
    }
    else  //扩容buffer，将剩余数据写入
    {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }
    return n;
}

ssize_t Buffer::writeFd(int fd, int* saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0)
    {
        *saveErrno = errno;
    }
    return n;
}