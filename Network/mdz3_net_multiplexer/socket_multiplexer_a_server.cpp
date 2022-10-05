#include "socket_multiplexer.h"
#include "vars.h"
#include <cstdint>
#include <thread>

using namespace Mantids::Network::Multiplexor;

bool Socket_Multiplexer::multiplexedSocket_sendLineConnectionAnswer(const DataStructs::sLineID &lineId, const DataStructs::eLineAcceptAnswerMSG &msg, const uint32_t &localLineWindow)
{
    json x;
    return multiplexedSocket_sendLineConnectionAnswer(lineId, msg, x, localLineWindow);
}

bool Socket_Multiplexer::multiplexedSocket_sendLineConnectionAnswer(const DataStructs::sLineID & lineId, const DataStructs::eLineAcceptAnswerMSG &msg, const json &answerValue, const uint32_t & localLineWindow)
{
    if (noSendData) return false;
    std::unique_lock<std::timed_mutex> lock(mtLock_multiplexedSocket);

    if (!multiplexedSocket->writeU<uint8_t>(DataStructs::MPLX_LINE_CONNECT_ANS))
    {
        return false;
    }
    if (!sendOnMultiplexedSocket_LineID(lineId.remoteLineId))
    {
        return false;
    }
    if (!sendOnMultiplexedSocket_LineID(lineId.localLineId))
    {
        return false;
    }
    if (!multiplexedSocket->writeU<uint32_t>(localLineWindow))
    {
        return false;
    }
    if (!multiplexedSocket->writeU<uint8_t>(msg))
    {
        return false;
    }
    if (!multiplexedSocket->writeStringEx<uint32_t>(  Mantids::Helpers::jsonToString(answerValue)
                                                    ,JSON_MAX_DATA))
    {
        return false;
    }
    return true;
}


void Socket_Multiplexer::server_AcceptConnection_Callback(DataStructs::sLineID lineId,const uint32_t &remoteWindowSize, const json &connectionParams)
{
    std::shared_ptr<Socket_Multiplexed_Line> sock = registerLine();

    if (!sock->isValidLine())
    {
        multiplexedSocket_sendLineConnectionAnswer(lineId,DataStructs::INIT_LINE_ANS_BADLOCALLINE);
        return;
    }

    if (!cbServerConnectAcceptor.callbackFunction)
    {
        multiplexedSocket_sendLineConnectionAnswer(lineId,DataStructs::INIT_LINE_ANS_NOCALLBACK);
        stopAndRemoveLine(sock);
        return;
    }

    // HERE WE CALL THE CALLBACK FUNCTION!!! -> should not take to long or block to work properly.
    Network::Sockets::Socket_StreamBase * ssock = cbServerConnectAcceptor.callbackFunction(cbServerConnectAcceptor.obj,sock->getLineID().localLineId,connectionParams);

    if (!ssock)
    {
        multiplexedSocket_sendLineConnectionAnswer(lineId,DataStructs::INIT_LINE_ANS_BADSERVERSOCK);
        stopAndRemoveLine(sock);
        return;
    }

    sock->setRemoteWindowSize(remoteWindowSize);
    sock->setLineRemoteID(lineId.remoteLineId);
    lineId = sock->getLineID();
    multiplexedSocket_sendLineConnectionAnswer(lineId,DataStructs::INIT_LINE_ANS_ESTABLISHED, sock->getLocalWindowSize());

    sock->processLine(ssock,this);

    // TODO: prevent double delete??

    // remove/close the remote connection
    multiplexedSocket_sendLineData(lineId,nullptr,0);
    // unregister the connection...
    stopAndRemoveLine(sock);

    if (cbServerConnectionFinished.callbackFunction) cbServerConnectionFinished.callbackFunction(cbServerConnectionFinished.obj, lineId.localLineId, ssock);

    if (destroySocketOnServer) delete ssock;
}

bool Socket_Multiplexer::processMultiplexedSocketCommand_Line_Connect(bool authorized)
{
    bool readen;
    std::string connectionParamsStr;

    DataStructs::sServerLineInitThreadParams * thrParams = new DataStructs::sServerLineInitThreadParams;
    if (!thrParams)
    {
        return false; // MEMORY PROBLEM => CLOSE THE LINE...
    }
    thrParams->multiPlexer = this;

    thrParams->lineID.remoteLineId = recvFromMultiplexedSocket_LineID(&readen);
    thrParams->remoteWindowSize = multiplexedSocket->readU<uint32_t>(&readen);
    connectionParamsStr = multiplexedSocket->readStringEx<uint32_t>(&readen, JSON_MAX_DATA);

    if (readen)
    {
        if (authorized)
        {
            Helpers::JSONReader2 reader;
            if ( reader.parse(connectionParamsStr,thrParams->jConnectionParams) )
            {
                std::thread(serverAcceptConnectionThread, thrParams).detach();
                multiplexedSocket_sendLineConnectionAnswer(thrParams->lineID, DataStructs::INIT_LINE_ANS_THREADED);
                return true;
                // TODO: handle thread fail as follows:
                //    multiplexedSocket_sendLineConnectionAnswer(thrParams->lineID, INIT_LINE_ANS_THREADFAILED);

            }
            else
                multiplexedSocket_sendLineConnectionAnswer(thrParams->lineID, DataStructs::INIT_LINE_ANS_BADPARAMS);
        }
        else
            multiplexedSocket_sendLineConnectionAnswer(thrParams->lineID, DataStructs::INIT_LINE_ANS_NOTAUTHORIZED);
    }

    delete thrParams;

    return false;
}

