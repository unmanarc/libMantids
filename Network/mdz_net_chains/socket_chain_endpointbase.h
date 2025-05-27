#ifndef SOCKETCHAINENDPOINTBASE_H
#define SOCKETCHAINENDPOINTBASE_H

#include "socket_chain_protocolbase.h"

namespace Mantids { namespace Network { namespace Sockets { namespace ChainProtocols {


class Socket_Chain_EndPointBase : public Socket_Chain_ProtocolBase
{
public:
    Socket_Chain_EndPointBase();
    virtual ~Socket_Chain_EndPointBase() override;
    bool isEndPoint() override;

protected:
    virtual void * getThis() override = 0;
};

}}}}

#endif // SOCKETCHAINENDPOINTBASE_H
