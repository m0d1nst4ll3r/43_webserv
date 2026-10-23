#ifndef FILEDESCRIPTOR_HPP
#define FILEDESCRIPTOR_HPP

class FileDescriptor
{
    public:

    FileDescriptor(int fd);
    ~FileDescriptor();

    int fd() const;

    private:

    const int _fd;
};

#endif /* FILEDESCRIPTOR_HPP */