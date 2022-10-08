#include "socket_multiplexer_callbacks.h"

using namespace Mantids3::Network::Multiplexor;

Socket_Multiplexer_Callbacks::Socket_Multiplexer_Callbacks()
{

}

void Socket_Multiplexer_Callbacks::setCallback_ClientConnectAccepted(Network::Sockets::Socket_StreamBase * (*callbackFunction)(void *, std::shared_ptr<Socket_Multiplexed_Line>), void *obj)
{
    cbClientConnectAccepted.callbackFunction = callbackFunction;
    cbClientConnectAccepted.obj = obj;
}

void Socket_Multiplexer_Callbacks::setCallback_ClientConnectFailed(bool (*callbackFunction)(void *, std::shared_ptr<Socket_Multiplexed_Line>, DataStructs::eConnectFailedReason), void *obj)
{
    cbClientConnectFailed.callbackFunction = callbackFunction;
    cbClientConnectFailed.obj = obj;
}

void Socket_Multiplexer_Callbacks::setCallback_ServerConnectAcceptor(Network::Sockets::Socket_StreamBase * (*callbackFunction)(void *, const LineID &, const json &), void *obj)
{
    cbServerConnectAcceptor.obj = obj;
    cbServerConnectAcceptor.callbackFunction = callbackFunction;
}

void Socket_Multiplexer_Callbacks::setCallback_ServerConnectionFinished(void (*callbackFunction)(void *, const LineID &, Network::Sockets::Socket_StreamBase *), void *obj)
{
    cbServerConnectionFinished.obj = obj;
    cbServerConnectionFinished.callbackFunction = callbackFunction;
}
