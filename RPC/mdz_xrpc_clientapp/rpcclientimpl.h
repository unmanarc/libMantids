#ifndef RPCCLIENTIMPL_H
#define RPCCLIENTIMPL_H

#include "mdz_net_sockets/socket_streambase.h"
#include <mdz_xrpc_fast/fastrpc.h>
#include <mdz_net_sockets/socket_tls.h>

#include <atomic>
#include <string>

namespace Mantids { namespace RPC {

class RPCClientImpl
{
public:
    struct PSKIdKey {
        std::string id;
        std::string psk;
    };

    RPCClientImpl();
    virtual ~RPCClientImpl() = default;

    /**
     * @brief runRPClient0 Run the class RPC Client (to be called from a thread)
     * @param rpcImpl RPC Client Implementation object
     */
    [[noreturn]] static void runRPClient0(RPCClientImpl * rpcImpl);
    /**
     * @brief runRPClient run RPC Client and made connection/authentication/processing (will block indefinitely)
     */
    [[noreturn]] void runRPClient();
    /**
     * @brief retrieveConfigFromLocalFile Retrieve the jRetrievedConfig from the local file.
     * @return true if can retrieve, otherwise false.
     */
    bool retrieveConfigFromLocalFile();
    /**
     * @brief retrieveConfigFromC2 Retrieve config from the C2
     * @return true if retrieved, otherwise false
     */
    bool retrieveConfigFromC2();


    /**
     * @brief updateAndSaveConfig Set the new configuration into the config file for the next boot.
     * @param newConfig new configuration
     * @return true if updated.
     */
    bool updateAndSaveConfig(const json& newConfig);

    // TODO: thread-safety: if you are accessing this object during a thread from the rpc client,
    // you should not be able to modify it, and during the initialization, both components are running.
    json getJRetrievedConfig();


    /**
     * @brief loadPSK Load the Pre-Shared Key (PSK) for TLS authentication
     * @return PSKIdKey structure containing the PSK ID and key
     */
    PSKIdKey loadPSK();

    /**
     * @brief defaultPSK Virtual Function to be called when PSK is not defined.
     * @return pair with identity and default PSK
     */
    virtual PSKIdKey defaultPSK();

protected:
    /**
     * @brief connectedToC2AfterFailingToLoadC2Config This function is called back when the first connection to the C2 was not successful.
     * and the new connection was made after it. And because we can't load the C2 configuration here, not execute nothing here,
     * we recommend you to _exit(-1111); the program.
     */
    virtual void connectedToC2AfterFailingToLoadC2Config() = 0;
    /**
     * @brief addMethods This function is called back once to add the RPC client methods to be available to the C2.
     */
    virtual void addMethods() = 0;
    /**
     * @brief postConnect This function is called back after the connection is made and before any authentication
     * @param sockRPCClient TLS Socket
     * @return if false, the connection will not continue
     */
    virtual bool postConnect(Mantids::Network::Sockets::Socket_TLS * sockRPCClient) { return  true; }

    virtual std::string decryptStr(const std::string & src) { return src; }
    virtual std::string encryptStr(const std::string & src) { return src; }

    std::string getClientConfigCmd,updateClientConfigLoadTimeCmd;

    Mantids::RPC::Fast::FastRPC fastRPC;
    json jRetrievedConfig;
    std::atomic<bool> failedToRetrieveC2Config;

};
}}


#endif // RPCCLIENTIMPL_H
