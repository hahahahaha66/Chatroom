#include "FileUtil.h"
#include "Logging.h"
#include <cstdio>

FileUtil::FileUtil(const std::string &fileName) : writtenBytes_(0) {
    if (fileName == "stdout") {
        fp_ = stdout;
    } else {
        fp_ = ::fopen(fileName.c_str(), "ae"); // 获取文件描述符，追加模式
    }

    if (!fp_) {
        fprintf(stderr, "Failed to open file: %s\n", fileName.c_str());
        exit(1);
    }

    ::setbuffer(fp_, buffer_, sizeof(buffer_)); // 设置文件流的用户缓冲区
}

FileUtil::~FileUtil() {
    if (fp_ != stdout) {
        ::fclose(fp_); // 关闭文件描述符
    }
}

void FileUtil::append(const char *data, size_t len) {
    size_t written = 0;
    printf("%s", data);

    while (written != len) {
        size_t remain = len - written;
        size_t n = write(data + written, remain);
        if (n != remain) {
            int err = ferror(fp_);
            if (err) {
                fprintf(stderr, "FileUtil::append() failed %s\n",
                        getErrnoMsg(err));
            }
        }
        written += n;
    }
    writtenBytes_ += written;
}

void FileUtil::flush() {
    ::fflush(fp_); // 强制刷新
}

size_t FileUtil::write(const char *data, size_t len) {

    return ::fwrite_unlocked(data, 1, len, fp_); // 线程不安全，但效率更高
}
