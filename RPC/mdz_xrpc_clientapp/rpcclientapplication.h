#ifndef RPCCLIENTAPPLICATION_H
#define RPCCLIENTAPPLICATION_H

#include <mdz_prg_service/globalarguments.h>
#include <mdz_prg_service/application.h>
#include <mdz_hlp_functions/mem.h>
#include <memory>

#include "globals.h"
//#include "rpcclientimpl.h"

namespace Mantids { namespace RPC {

class RPCClientApplication : public Mantids::Application::Application
{
public:
    RPCClientApplication( Mantids::Helpers::Mem::xBinContainer * masterKey ) {
        Globals::setMasterKey(masterKey);
    }
    virtual ~RPCClientApplication() override = default;

    void _shutdown() override;
    void _initvars(int argc, char *argv[], Mantids::Application::Arguments::GlobalArguments * globalArguments) override;
    bool _config(int argc, char *argv[], Mantids::Application::Arguments::GlobalArguments * globalArguments) override;
    int _start(int argc, char *argv[], Mantids::Application::Arguments::GlobalArguments * globalArguments) override;


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
    virtual void rpcInitVars(int argc, char *argv[], Mantids::Application::Arguments::GlobalArguments * globalArguments)=0;
    /**
     * @brief _config Function called for config parsing / program initialization
     * @param argc
     * @param argv
     * @param globalArguments
     * @return
     */
    virtual bool rpcConfig(int argc, char *argv[], Mantids::Application::Arguments::GlobalArguments * globalArguments)=0;
    /**
     * @brief _start function called for program start
     * @param argc
     * @param argv
     * @param globalArguments
     * @return
     */
    virtual int rpcStart(int argc, char *argv[], Mantids::Application::Arguments::GlobalArguments * globalArguments)=0;

    virtual void processRetrievedConfig() {}

    //
    uint32_t appVersionMajor=0,appVersionMinor=0,appVersionSubMinor=1;
    std::string versionCodeName;
    std::string defaultConfigDir;
    bool retrieveConfig = false;
};

}}


#endif // RPCCLIENTAPPLICATION_H
