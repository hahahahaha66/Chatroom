#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

//作为基类，设置子类的禁止拷贝函数，并禁用赋值操作符

class noncopyable
{
public:
    //delete，显示调用删除，设为public，确保子类成功被删除
    noncopyable(const noncopyable &) = delete;
    noncopyable &operator=(const noncopyable &) = delete;

protected:
    //构造和析构设为protected，防止类外创建和删除
    noncopyable() = default;
    ~noncopyable() = default;
};

#endif