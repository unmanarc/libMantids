#include "socket_multiplexer.h"
#include <cstdint>

using namespace Mantids3::Network::Multiplexor;

bool Socket_Multiplexer::processMultiplexedSocketCommand_Plugin_JSON16()
{
    bool readen;
    json jMsg;
    std::string pluginId = multiplexedSocket->readStringEx<uint8_t>(&readen), sMsg;

    if (!readen)
        return false;

    sMsg = multiplexedSocket->readStringEx<uint32_t>(&readen, PLUGIN_MAX_DATA);

    if (!readen)
        return false;

    Helpers::JSONReader2 reader;
    if ( reader.parse(sMsg,jMsg) )
    {
        plugins[pluginId]->processJSON16(jMsg);
        return true;
    }
    else
    {
        // parse problem?
        return false;
    }
}

bool Socket_Multiplexer::processMultiplexedSocketCommand_Plugin_Data()
{
    bool readen;
    unsigned int datalen = PLUGIN_MAX_DATA;
    std::string pluginId = multiplexedSocket->readStringEx<uint8_t>(&readen);
    if (!readen) return false;
    char * pluginData = ((char *)multiplexedSocket->readBlockWAllocEx<uint32_t>( &datalen ));
    if (!pluginData) return false;
    if (plugins.find(pluginId) != plugins.end())
    {
        plugins[pluginId]->processData(pluginData,datalen);
    }
    delete [] pluginData;
    return true;
}

bool Socket_Multiplexer::plugin_SendData(const std::string &pluginId, void *data, const uint32_t &datalen, bool lock)
{
    if (noSendData) return false;

    if (lock) mtLock_multiplexedSocket.lock();
    if (!multiplexedSocket->writeU<uint8_t>(DataStructs::MPLX_PLUGIN_DATA))
    {
        if (lock) mtLock_multiplexedSocket.unlock();
        return false;
    }
    if (!multiplexedSocket->writeStringEx<uint8_t>(pluginId))
    {
        if (lock) mtLock_multiplexedSocket.unlock();
        return false;
    }
    if (!multiplexedSocket->writeBlockEx<uint32_t>(data,datalen))
    {
        if (lock) mtLock_multiplexedSocket.unlock();
        return false;
    }
    if (lock) mtLock_multiplexedSocket.unlock();
    return true;
}

bool Socket_Multiplexer::plugin_SendJson(const std::string &pluginId, const json &jData, bool lock)
{
    if (noSendData) return false;
    if (lock) mtLock_multiplexedSocket.lock();
    if (!multiplexedSocket->writeU<uint8_t>(DataStructs::MPLX_PLUGIN_JSON))
    {
        if (lock) mtLock_multiplexedSocket.unlock();
        return false;
    }
    if (!multiplexedSocket->writeStringEx<uint8_t>(pluginId))
    {
        if (lock) mtLock_multiplexedSocket.unlock();
        return false;
    }
    if (!multiplexedSocket->writeStringEx<uint32_t>( Mantids3::Helpers::jsonToString( jData )  ))
    {
        if (lock) mtLock_multiplexedSocket.unlock();
        return false;
    }
    if (lock) mtLock_multiplexedSocket.unlock();
    return true;
}
