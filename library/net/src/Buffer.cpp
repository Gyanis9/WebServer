//
// Created by guo on 25-1-6.
//

#include "../include/Buffer.h"


#include <cerrno>
#include <sys/uio.h>
#include <csignal>


ssize_t Buffer::readFd(int fd, int *savedErrno) {
    char extrabuf[65536] = {0};
    struct iovec vec[2];
    const size_t writable = writeableBytes();
    vec[0].iov_base = begin() + m_writeIndex;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0) {
        *savedErrno = errno;
    } else if (n <= writable) {
        m_writeIndex += n;
    } else {
        m_writeIndex = m_buffer.size();
        append(extrabuf, n - writable);
    }
    return n;
}

ssize_t Buffer::writeFd(int fd, int *savedErrno) {
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0) {
        *savedErrno = errno;
    }
    return n;
}