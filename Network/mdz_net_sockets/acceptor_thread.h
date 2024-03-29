#ifndef SATHREAD_H
#define SATHREAD_H

#include <thread>
#include "socket_streambase.h"

namespace Mantids { namespace Network { namespace Sockets { namespace Acceptors {

/**
 * Class for managing the client on his thread.
 */
class SAThread
{
public:
    /**
     * constructor
     */
    SAThread();
    /**
     * destructor
     */
    ~SAThread();
    /**
     * Start the thread of the client.
     */
    void start();
    /**
     * Kill the client socket
     */
    void stopSocket();
    /**
     * Set parent (stream acceptor object)
     * @param parent parent
     */
    void setParent(void * parent);
    /**
     * Set callback when connection is fully established (if your function returns false, the socket will not be destroyed by this)
     */
    void setCallbackOnConnect(bool (*_callbackOnConnect)(void *, Sockets::Socket_StreamBase *, const char *, bool), void *objOnConnected);
    /**
     * Set callback when protocol initialization failed (like bad X.509 on TLS)
     */
    void setCallbackOnInitFail(bool (*_callbackOnInitFailed)(void *, Sockets::Socket_StreamBase *, const char *, bool), void *objOnConnected);
    /**
     * Call callback
     * to be used from the client thread.
     */
    void postInitConnection();
    /**
     * Set socket
     */
    void setClientSocket(Sockets::Socket_StreamBase * _clientSocket);

    /**
     * @brief getRemotePair Get Remote Host Address
     * @return remote pair null terminated string.
     */
    const char * getRemotePair();

    bool getIsSecure() const;
    void setIsSecure(bool value);

private:
    static void thread_streamclient(SAThread * threadClient, void * threadedAcceptedControl);

    Sockets::Socket_StreamBase * clientSocket;
    bool (*callbackOnConnect)(void *,Sockets::Socket_StreamBase *, const char *, bool);
    bool (*callbackOnInitFail)(void *,Sockets::Socket_StreamBase *, const char *, bool);

    char remotePair[INET6_ADDRSTRLEN];
    bool isSecure;

    void *objOnConnect, *objOnInitFail;
    void * parent;
};

}}}}


#endif // SATHREAD_H
