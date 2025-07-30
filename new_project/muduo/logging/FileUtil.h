#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include <stdio.h>
#include <string>

//封装底层的I/O操作，相当于对write的封装
class FileUtil
{
public:
    explicit FileUtil(const std::string& fileName);
    ~FileUtil();

    void append(const char* data, size_t len);  //写入数据

    void flush();  //刷新缓冲区

    off_t writtenBytes() const { return writtenBytes_; }

private:
    size_t write(const char* data, size_t len);

    FILE* fp_;
    char buffer_[64*1024];  //写操作缓冲区
    //off_t是专门为文件偏移量/写入大小设计的类型，安全，专业，兼容，可扩展，适合大文件写入
    off_t writtenBytes_;
};

#endif