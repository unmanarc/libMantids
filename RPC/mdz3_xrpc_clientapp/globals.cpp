#include "globals.h"

using namespace Mantids3::Application;

// LOGS:
Logs::AppLog * Globals::applog = nullptr;
boost::property_tree::ptree Globals::pLocalConfig;
Mantids3::RPC::RPCClientImpl * Globals::rpcImpl = nullptr;

Mantids3::Helpers::Mem::xBinContainer * Globals::masterKey=nullptr;


Globals::Globals()
{
}

void Globals::setLocalInitConfig(const boost::property_tree::ptree &config)
{
    // Initial config:
    pLocalConfig = config;
}

Mantids3::Application::Logs::AppLog *Globals::getAppLog()
{
    return applog;
}

void Globals::setAppLog(Mantids3::Application::Logs::AppLog *value)
{
    applog = value;
}

Mantids3::RPC::RPCClientImpl *Globals::getRpcImpl()
{
    return rpcImpl;
}

void Globals::setRpcImpl(Mantids3::RPC::RPCClientImpl *value)
{
    rpcImpl = value;
}

Mantids3::Helpers::Mem::xBinContainer *Globals::getMasterKey()
{
    return masterKey;
}

void Globals::setMasterKey(Mantids3::Helpers::Mem::xBinContainer *newMasterKey)
{
    masterKey = newMasterKey;
}
