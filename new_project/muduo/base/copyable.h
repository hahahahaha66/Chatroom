#ifndef COPYABLE_H
#define COPYABLE_H

// 为了和nocopyable区分开，允许一些轻量级类进行拷贝，不用担心资源和开销
class copyable {
  protected:
    copyable() = default;
    ~copyable() = default;
};

#endif