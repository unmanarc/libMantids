#include "socket_chain_endpointbase.h"

using namespace Mantids3::Network::Sockets::ChainProtocols;

Socket_Chain_EndPointBase::Socket_Chain_EndPointBase()
{

}

Socket_Chain_EndPointBase::~Socket_Chain_EndPointBase()
{

}

bool Socket_Chain_EndPointBase::isEndPoint()
{
    return true;
}
