#ifndef FILEDESCRIPTOR_HPP
#define FILEDESCRIPTOR_HPP

class FileDescriptor
{
	public:

	const int fd;

	FileDescriptor(int fd);
	~FileDescriptor();
};

#endif /* FILEDESCRIPTOR_HPP */