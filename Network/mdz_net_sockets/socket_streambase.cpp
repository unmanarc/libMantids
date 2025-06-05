#include "socket_streambase.h"

#ifndef _WIN32
#include <sys/socket.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#include "socket_tcp.h"

#endif
#include <string.h>
#include <unistd.h>

using namespace Mantids;
using namespace Mantids::Network::Sockets;

Socket_StreamBase::Socket_StreamBase()
{
    //printf("Creating streamsocket %p\n", this); fflush(stdout);
}

Socket_StreamBase::~Socket_StreamBase()
{
  //  printf("Removing streamsocket %p\n", this); fflush(stdout);
}

void Socket_StreamBase::writeEOF(bool)
{
    shutdownSocket(SHUT_RDWR);
}

bool Socket_StreamBase::streamTo(Memory::Streams::StreamableObject *out, Memory::Streams::StreamableObject::Status &wrsStat)
{
    char data[8192];
    Memory::Streams::StreamableObject::Status cur;
    for (;;)
    {
        ssize_t r = partialRead(data,sizeof(data));
        switch (r)
        {
        case -1: // ERR.
            out->writeEOF(false);
            return false;
        case 0: // EOF.
            out->writeEOF(true);
            return true;
        default:
            if (!(cur=out->writeFullStream(data,r,wrsStat)).succeed || cur.finish)
            {
                if (!cur.succeed)
                {
                    out->writeEOF(false);
                    return false;
                }
                else
                {
                    out->writeEOF(true);
                    return true;
                }
            }
        break;
        }
    }
}

Memory::Streams::StreamableObject::Status Socket_StreamBase::write(const void *buf, const size_t &count, Memory::Streams::StreamableObject::Status &wrStat)
{
    Memory::Streams::StreamableObject::Status cur;
    // TODO: report the right amount of data copied...
    bool r = writeFull(buf,count);
    if (!r)
        wrStat.succeed=cur.succeed=setFailedWriteState();
    else
    {
        cur.bytesWritten+=count;
        wrStat.bytesWritten+=count;
    }
    return cur;
}

std::pair<Socket_StreamBase *,Socket_StreamBase *> Socket_StreamBase::GetSocketPair()
{
    std::pair<Socket_StreamBase *,Socket_StreamBase *> p;

    p.first = nullptr;
    p.second = nullptr;

#ifndef _WIN32
    int sockets[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0)
    {
        // ERROR:...
    }
    else
    {
        p.first = new Socket_StreamBase();
        p.second = new Socket_StreamBase();

        p.first->setSocketFD(sockets[0]);
        p.second->setSocketFD(sockets[1]);
    }
#else
    // Emulate via TCP. (EXPERIMENTAL)

    Sockets::Socket_TCP * llsock = new Sockets::Socket_TCP, * lsock = nullptr, * rsock = new Sockets::Socket_TCP;
    llsock->listenOn(0,"127.0.0.1");
    rsock->connectTo("127.0.0.1",lsock->getPort());
    lsock = (Sockets::Socket_TCP *)llsock->acceptConnection();
    llsock->closeSocket();
    delete llsock;

    p.first = lsock;
    p.second = rsock;
#endif
    return p;
}

bool Socket_StreamBase::writeFull(const void *data)
{
    return writeFull(data,strlen((static_cast<const char *>(data))));
}

bool Socket_StreamBase::writeFull(const void *data, const uint64_t &datalen)
{
    // Init control variables:
    uint64_t remaining = datalen;           // data left to send
    const char* dataPtr = static_cast<const char*>(data); // data pointer.

    // Bucle para enviar datos en fragmentos hasta que se envíe todo
    while (remaining > 0)
    {
        // Determina el tamaño del fragmento (máximo 4096 bytes)
        uint64_t chunkSize = std::min(remaining, static_cast<uint64_t>(4096));

        // Envía el fragmento actual
        ssize_t sentBytes = partialWrite(dataPtr, static_cast<uint32_t>(chunkSize));

        // Manejo de errores
        if (sentBytes < 0)
        {
            // Error al enviar los datos
            shutdownSocket();
            return false;
        }

        if (sentBytes == 0)
        {
            // No se pudieron enviar bytes, posible error de conexión (buffer lleno?)
            shutdownSocket();
            return false;
        }

        // Actualiza el puntero y el conteo de datos pendientes
        dataPtr += sentBytes;
        remaining -= static_cast<uint64_t>(sentBytes);
    }

    // Todos los datos fueron enviados correctamente
    return true;
}


Socket_StreamBase * Socket_StreamBase::acceptConnection()
{
    return nullptr;
}

bool Socket_StreamBase::postAcceptSubInitialization()
{
    return true;
}

bool Socket_StreamBase::postConnectSubInitialization()
{
    return true;
}

bool Socket_StreamBase::readFull(void* data, const uint64_t& expectedDataBytesCount, uint64_t* receivedDataBytesCount)
{
    if (receivedDataBytesCount != nullptr) {
        *receivedDataBytesCount = 0;
    }

    // Validate input
    if (data == nullptr) {
        return false;
    }

    if (expectedDataBytesCount == 0) {
        return true;
    }

    uint64_t curReceivedBytesCount = 0;
    const size_t MAX_READ_BUFFER_SIZE = 4096;

    while (curReceivedBytesCount < expectedDataBytesCount)
    {
        // Calcular el tamaño máximo a leer en esta iteración
        size_t bytesToRead = std::min<uint64_t>(MAX_READ_BUFFER_SIZE, expectedDataBytesCount - curReceivedBytesCount);

        ssize_t partialReceivedBytesCount = partialRead(
            static_cast<char*>(data) + curReceivedBytesCount,
            static_cast<uint32_t>(bytesToRead)
            );

        if (partialReceivedBytesCount < 0)
        {
            // Read Error.
            return false;
        }
        else if (partialReceivedBytesCount == 0)
        {
            // Connection Closed.
            break;
        }
        else
        {
            curReceivedBytesCount += static_cast<uint64_t>(partialReceivedBytesCount);
        }
    }

    // Notify the received bytes count.
    if (receivedDataBytesCount != nullptr)
    {
        *receivedDataBytesCount = curReceivedBytesCount;
    }

    // If we received less...
    if (curReceivedBytesCount < expectedDataBytesCount)
    {
        // Not interested in less.
        return false;
    }

    // We received complete:
    return true;
}

void Socket_StreamBase::writeDeSync()
{
    // Action when everything is desynced... (better to stop R/W from the socket)
    shutdownSocket();
}

void Socket_StreamBase::readDeSync()
{
    // Action when everything is desynced... (better to stop R/W from the socket)
    shutdownSocket();
}

bool Socket_StreamBase::isConnected()
{
    if (!isActive())
        return false;

    struct sockaddr peer;
    socklen_t peer_len;
    peer_len = sizeof(peer);
    if (getpeername(sockfd, &peer, &peer_len) == -1)
    {
        closeSocket();
        return false;
    }
    return true;
}

bool Socket_StreamBase::listenOn(const uint16_t &, const char *, const int32_t &, const int32_t &)
{
    return false;
}

bool Socket_StreamBase::connectFrom(const char *, const char* , const uint16_t &, const uint32_t &)
{
	return false;
}
