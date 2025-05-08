#pragma once
#include "stdint.h"
#include "sys/uio.h"
#include <string>
#include <vector>
#include <atomic>
namespace GcRpc{
    constexpr uint16_t MAX_BUFFER_SIZE = 1024;

    /**
     * @brief Buffer is designed using a ring buffer area same as Netty-Buffer. Copy and move constructor are all provided.
     * @warning Buffer itself is an inner-class and not thread-safe. Direct use of it is not recommanded.
     * @param buf a vector of char, its size is origined as MAX_BUFFER_SIZE.
     * @param readIndex a pointer to the beginning position of reading. readIndex should be less than MAX_BUFFER_SIZE
     * @param writeIndex a pointer to the beginning position of writing, writeIndex can be MAX_BUFFER_SIZE but equals to 0.
     * @attention I design a null position to handle the corner condition of ring buffer, so the actual buffer size is one 
     * less than the MAX_BUFFER_SIZE. If setting MAX_BUFFER_SIZE=1024, only 1023 Bytes can be written in.
     */
    class Buffer{
        std::vector<char> buf;
        uint16_t readIndex;
        uint16_t writeIndex;  
    public:
        Buffer();
        ~Buffer() = default;
        Buffer(const Buffer & other); //deepcopy
        Buffer(Buffer && other);

        Buffer & operator=(const Buffer & other);

        Buffer & operator=(Buffer && other);

        /**
         * @brief Read from the buffer and get a std::string
         */
        std::string read(uint16_t read_size);

        /**
         * @brief The size of bytes readable in the buffer.
         */
        uint16_t readableBytes();

        /**
         * @brief The sizeof bytes that can be written in.
         */
        uint16_t writableBytes();

        /**
         * @brief return a pointer used for ssize_t read(int fd, void *buf, size_t count);
         * @warning this function won't support ring vector.
         */
        char* writeBegin();

        /**
         * @brief return a iovec array with size 2 for ssize_t readv(int filedes, const struct iovec *iov, int iovcnt);
         */
        const struct iovec * readvBegin();

        /**
         * @brief return a iovec array with size 2 for ssize_t writev(int filedes, const struct iovec *iov, int iovcnt);
         */
        const struct iovec * writevBegin();

        /**
         * @brief append a string after the writeIndex.
         */
        uint16_t append(std::string & str);

        /**
         * @overload a overload of uint16_t append(std::string);
         */
        uint16_t append(const char * str, uint16_t size);

        /**
         * @brief reset the buffer and pointers;
         */
        void clear();

        /**
         * @brief Modify the writeIndex. Because this class doesn't know the bytes that actually written,
         * classes using this Buffer should use this function to make sure it work.
         */
        void hasWritten(uint16_t size);

        /**
         * @brief Modify the readIndex. Because this class doesn't know the bytes that actually read,
         * classes using this Buffer should use this function to make sure it work.
         */
        void hasRead(uint16_t size);

        /**
         * @brief Return all buffers that be receive from the internet and saved in buffer.
         */
        const std::string retrieveAllBufferAsString();

    private:
        /**
         * @brief return the begin pointer of inner buffer.
         */
        char * begin();
    };

}