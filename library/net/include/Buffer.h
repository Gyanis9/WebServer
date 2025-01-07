//
// Created by guo on 25-1-6.
//

#ifndef WEBSERVER_BUFFER_H
#define WEBSERVER_BUFFER_H

#include <string>
#include <vector>
#include <cassert>
#include <algorithm>
#include "../../base/include/copyable.h"


class Buffer : public copyable {
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

public:
    explicit Buffer(size_t initialSize = kInitialSize) :
            m_buffer(kCheapPrepend + initialSize),
            m_readIndex(kCheapPrepend), m_writeIndex(kCheapPrepend) {}

    size_t readableBytes() const { return m_writeIndex - m_readIndex; }

    size_t writeableBytes() const { return m_buffer.size() - m_writeIndex; }

    size_t prependableBytes() const { return m_readIndex; }

    void rretrieve(size_t len) {
        if (len < readableBytes()) {
            m_readIndex += len;
        } else {
            retrieveAll();
        }

    }

    void retrieveAll() {
        m_readIndex = kCheapPrepend;
        m_writeIndex = kCheapPrepend;
    }

    std::string retrieveAllAsString() {
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len) {
        assert(len <= readableBytes());
        std::string result(peek(), len);
        rretrieve(len);
        return result;
    }

    void ensureWriteableBytes(size_t len) {
        if (writeableBytes() < len) {
            makeSpace(len);
        }
    }

    void append(const char *data, size_t len) {
        ensureWriteableBytes(len);
        std::copy(data, data + len, beginWrite());
        m_writeIndex += len;
    }

    char *beginWrite() {
        return begin() + m_writeIndex;
    }

    const char *beginWrite() const {
        return begin() + m_writeIndex;
    }

    ssize_t readFd(int fd, int *savedErrno);

    ssize_t writeFd(int fd, int *savedErrno);

private:
    char *begin() { return &*m_buffer.begin(); }

    const char *begin() const { return &*m_buffer.begin(); }

    const char *peek() const { return begin() + m_readIndex; }

    void makeSpace(size_t len) {
        if (writeableBytes() + prependableBytes() < len + kCheapPrepend) {
            m_buffer.resize(m_writeIndex + len);
        } else {
            size_t readable = readableBytes();
            std::copy(begin() + m_readIndex, begin() + m_writeIndex, begin() + kCheapPrepend);
            m_readIndex = kCheapPrepend;
            m_writeIndex = m_readIndex + readable;
        }
    }

private:
    std::vector<char> m_buffer;
    size_t m_readIndex;
    size_t m_writeIndex;
};


#endif //WEBSERVER_BUFFER_H
