#ifndef SOCKETCHAINXOR_H
#define SOCKETCHAINXOR_H

#include <mdz_net_sockets/socket_streambase.h>
#include "socket_chain_protocolbase.h"

namespace Mantids { namespace Network { namespace Sockets { namespace ChainProtocols {

/**
 * @brief The SocketChainXOR class
 *        Proof of concept of socket transformation, don't use for security applications.
 */
class Socket_Chain_XOR : public Mantids::Network::Sockets::Socket_StreamBase, public Socket_Chain_ProtocolBase
{
public:
    Socket_Chain_XOR();

    // Overwritten functions:
    ssize_t partialRead(void * data, const uint32_t & datalen) override;
    ssize_t partialWrite(const void * data, const uint32_t & datalen) override;

    // Private functions:
    char getXorByte() const;
    void setXorByte(char value);

protected:
    void * getThis() override { return this; }

private:
    char * getXorCopy(const void *data, const uint32_t & datalen);
    char xorByte;
};


}}}}

#endif // SOCKETCHAINXOR_H
