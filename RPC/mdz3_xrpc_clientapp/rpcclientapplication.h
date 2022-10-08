#ifndef RPCCLIENTAPPLICATION_H
#define RPCCLIENTAPPLICATION_H

#include <mdz3_prg_service/globalarguments.h>
#include <mdz3_prg_service/application.h>
#include <mdz3_helpers/mem.h>
#include <memory>
#include "globals.h"
//#include "rpcclientimpl.h"

namespace Mantids3 { namespace RPC {

class RPCClientApplication : public Mantids3::Application::Application
{
public:
    RPCClientApplication( Mantids3::Helpers::Mem::xBinContainer * masterKey ) {
        Globals::setMasterKey(masterKey);
        retrieveConfig = false;
    }
    void _shutdown();
    void _initvars(int argc, char *argv[], Mantids3::Application::Arguments::GlobalArguments * globalArguments);
    bool _config(int argc, char *argv[], Mantids3::Application::Arguments::GlobalArguments * globalArguments);
    int _start(int argc, char *argv[], Mantids3::Application::Arguments::GlobalArguments * globalArguments);


protected:
    /**
     * @brief _shutdown function called for program shutdown (close here your descriptors, connections, etc)
     */
    virtual void rpcShutdown()=0;
    /**
     * @brief _initvars Function called for variables initialization
     * @param argc
     * @param argv
     * @param globalArguments
     */
    virtual void rpcInitVars(int argc, char *argv[], Mantids3::Application::Arguments::GlobalArguments * globalArguments)=0;
    /**
     * @brief _config Function called for config parsing / program initialization
     * @param argc
     * @param argv
     * @param globalArguments
     * @return
     */
    virtual bool rpcConfig(int argc, char *argv[], Mantids3::Application::Arguments::GlobalArguments * globalArguments)=0;
    /**
     * @brief _start function called for program start
     * @param argc
     * @param argv
     * @param globalArguments
     * @return
     */
    virtual int rpcStart(int argc, char *argv[], Mantids3::Application::Arguments::GlobalArguments * globalArguments)=0;

    virtual void processRetrievedConfig() {};

    //
    uint32_t appVersionMajor,appVersionMinor,appVersionSubMinor;
    std::string versionCodeName;
    std::string defaultConfigDir;
    bool retrieveConfig;
};

}}


#endif // RPCCLIENTAPPLICATION_H
