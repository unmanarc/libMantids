#ifndef SOCKETCHAINBASE_H
#define SOCKETCHAINBASE_H

#include <utility>
#include <mdz3_net_sockets/socket_streambase.h>

namespace Mantids3 { namespace Network { namespace Sockets { namespace ChainProtocols {


class Socket_Chain_ProtocolBase
{
public:
    Socket_Chain_ProtocolBase();
    virtual ~Socket_Chain_ProtocolBase();

    virtual bool isEndPoint();
    std::pair<Mantids3::Network::Sockets::Socket_StreamBase *, Mantids3::Network::Sockets::Socket_StreamBase*> makeSocketChainPair();
    bool isServerMode() const;
    void setIsServerMode(bool value);

protected:
    virtual void * getThis() = 0;

private:
    bool serverMode;
};

}}}}

#endif // SOCKETCHAINBASE_H
