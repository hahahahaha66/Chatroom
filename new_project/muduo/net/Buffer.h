#ifndef BUFFER_H
#define BUFFER_H

#include <cstddef>
#include <vector>
#include <string>
#include <algorithm>

class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize),
          readerIndex_(kCheapPrepend),
          writerIndex_(kCheapPrepend)
    {
    }

    //可读大小
    size_t readableBytes() const { return writerIndex_ - readerIndex_; }
    //可写大小
    size_t writableBytes() const { return buffer_.size() - writerIndex_; }
    //头部预留大小
    size_t prependableBytes() const { return readerIndex_; }

    //返回可读区块的头指针
    const char* peek() const
    {
        return begin() + readerIndex_;
    }
    
    void retrieveUntil(const char* end)
    {
        retrieve(end - peek());
    }

    //移动可读区块的指针
    void retrieve(size_t len)
    {
        //只读了缓冲区的部分长度
        if (len < readableBytes())
        {
            readerIndex_ += len;
        }
        else  //读完缓冲区的全部内容
        {
            retrieveAll();
        }
    }

    void retrieveAll()
    {
        //重置读写指针
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;    
    }

    //获取可读区块的string但不移动可读区块的指针位置
    std::string GetBufferAllAsString()
    {
        size_t len = readableBytes();
        std::string result(peek(), len);
        return result;
    }

    //retrieveAsString的外层包装
    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());
    }

    //获取可读区块的string但移动可读区块的指针位置
    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    //比较可写区块与len的大小
    void ensureWritableBytes(size_t len)
    {
        if (writableBytes() < len)
        {
            makeSpace(len);
        }
    }

    //append的外层包装
    void append(const std::string& str)
    {
        append(str.data(), str.size());
    }

    //在可写区块后追加字符串，并移动可写区块的指针
    void append(const char* data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        writerIndex_ += len;
    }

    //用于找到一行的结束标志，按行解析受到的数据
    const char* findCRLF() const 
    {
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    //返回可写区块的指针
    char* beginWrite()
    {
        return begin() + writerIndex_;
    }

    //beginWrite的const版本
    const char* beginWrite() const
    {
        return begin() + writerIndex_;
    }

    //从Fd上读取数据
    ssize_t readFd(int fd, int* saveErrno);
    //想Fd写入数据
    ssize_t writeFd(int fd, int* saveErrno);

private:
    //返回缓冲区开始的位置
    char* begin()
    {
        return &(*buffer_.begin());
    }

    const char* begin() const
    {
        return &(*buffer_.begin());
    }

    void makeSpace(int len)
    {
        //如果剩余空间不足
        if (writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writerIndex_ + len);
        }
        else //如果可写加上头部空间足够
        {
            size_t readable = readableBytes();
            //将已写入的空间向前移动
            std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

    std::vector<char> buffer_;  //缓冲区
    size_t readerIndex_;  //可读指针
    size_t writerIndex_;  //可写指针
    static const char kCRLF[];  //表示一行结束的结束符
};

#endif