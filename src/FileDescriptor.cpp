#include "FileDescriptor.hpp"
#include <stdexcept>
#include <unistd.h>

FileDescriptor::FileDescriptor(int fd) : fd(fd)
{
	if (fd < 0)
		throw std::runtime_error("not a valid fd");
}

FileDescriptor::~FileDescriptor()
{
	close(fd);
}