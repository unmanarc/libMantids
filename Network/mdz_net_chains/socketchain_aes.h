#ifndef SOCKETCHAIN_AES_H
#define SOCKETCHAIN_AES_H

#include <mdz_net_sockets/streamsocket.h>
#include "socketchainbase.h"
#include <openssl/evp.h>
#include <string.h>
#include <random>
#include <mdz_hlp_functions/mem.h>

namespace Mantids { namespace Network { namespace Chains { namespace Protocols {

// 16 bytes sent from very first IV
// and 112 bytes from this struct... (TOT: 128 bytes)
struct sHandShakeHeader {
    sHandShakeHeader()
    {
        ZeroBArray(reserved);
        ZeroBArray(phase2Key);
        ZeroBArray(IVSeed);
        memcpy(magicBytes,"IHDR",4);
    }
    ~sHandShakeHeader()
    {
        ZeroBArray(reserved);
        ZeroBArray(phase2Key);
        ZeroBArray(IVSeed);
        ZeroBArray(magicBytes);
    }

    // TOT: 112 bytes initial header.
    char magicBytes[4];
    char IVSeed[16];
    char phase2Key[32];
    char reserved[60];
} __attribute__((packed));

struct sSideParams {
    sSideParams()
    {
        aesBlock = nullptr;
        aesBlock_curSize = 0;
    }
    ~sSideParams()
    {
        cleanAESBlock();
        ZeroBArray(handShakeIV);
    }
    void cleanAESBlock( char * nAesBlock = nullptr , size_t nAesBlock_curSize = 0 )
    {
        if (aesBlock != nullptr)
        {
            memset(aesBlock,0,sizeof(aesBlock_curSize));
            delete [] aesBlock;
        }
        aesBlock_curSize = nAesBlock_curSize;
        aesBlock = nAesBlock;
    }
    bool appendAESBlock(char * nAesBlock, size_t nAesBlock_size)
    {
        char * nBlock = new char[nAesBlock_size+aesBlock_curSize];
        if (!nBlock) return false;
        if (aesBlock_curSize) memcpy(nBlock,aesBlock,aesBlock_curSize);
        memcpy(nBlock+aesBlock_curSize,nAesBlock,nAesBlock_size);
        cleanAESBlock(nBlock,nAesBlock_size+aesBlock_curSize);
        return true;
    }
    void reduce(size_t bytes)
    {
        if (bytes >= aesBlock_curSize)
        {
            cleanAESBlock();
        }
        else
        {
            char * nBlock = new char[aesBlock_curSize-bytes];
            if (!nBlock) return;
            if (aesBlock_curSize)
                memcpy(nBlock,aesBlock+bytes,aesBlock_curSize-bytes);
            cleanAESBlock(nBlock,aesBlock_curSize-bytes);
        }
    }
    bool cryptoXOR( char * dataBlock, size_t dblocksize, bool dontreduce = false)
    {
        if (dblocksize>aesBlock_curSize) return false;
        for (size_t i=0;i<dblocksize;i++)
        {
            dataBlock[i] = dataBlock[i]^aesBlock[i];
        }
        if (!dontreduce) reduce(dblocksize);
        return true;
    }
    char handShakeIV[16];
    char currentIV[16];
    std::mt19937_64 mt19937IV[2];
    sHandShakeHeader handshake;
    char * aesBlock;
    size_t aesBlock_curSize;
};

/**
 * @brief The SocketChain_AES class
 *        AES-PSK Stream Cipher using XOR.
 *        Suitable for security apps (Use with TLS).
 */
class SocketChain_AES  : public Mantids::Network::Streams::StreamSocket, public SocketChainBase
{
public:
    SocketChain_AES();
    ~SocketChain_AES();

    /**
     * @brief setCipherBlockSize Set the value of block generated by the cipher algorithm
     *        should only be used before the communication starts.
     * @param value block size.
     */
    void setAESRegenBlockSize(const size_t &value = 1024);

    /**
     * @brief setPhase1Key Set Phase 1 (header interchange) AES Key
     * @param pass AES Key.
     */
    void setPhase1Key256(const char *pass);
    void setPhase1Key(const char * pass);

    //////////////////////////////////////////
    // Overwritten functions:
    int partialRead(void * data, const uint32_t & datalen) override;
    int partialWrite(const void * data, const uint32_t & datalen) override;

    bool postAcceptSubInitialization() override;
    bool postConnectSubInitialization() override;

    static const EVP_CIPHER * openSSLInit();

protected:
    void * getThis() override { return this; }

private:
    void genRandomBytes(char * bytes, size_t size);
    void genRandomWeakBytes(char * bytes, size_t size);
    bool appendNewAESBlock(sSideParams * params, const char * key, const char * iv);
    void regenIV(sSideParams * param);
    /**
     * @brief genPlainText Get Plain Text for generating the AES Block.
     * @return plain text (you should delete it with delete[])
     */
    char * genPlainText();
    /**
     * @brief phase1Key Key Negotiation Password... (PSK)
     */
    char phase1Key[32];
    /**
     * @brief readParams remote read params
     */
    sSideParams readParams;
    /**
     * @brief writeParams local write params
     */
    sSideParams writeParams;

    size_t aesRegenBlockSize;
    bool initialized;
    const static EVP_CIPHER *cipher;
};

}}}}
#endif // SOCKETCHAIN_AES_H
