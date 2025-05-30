#include "socket_tls.h"

#include "socket_streambase.h"


#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#include <limits>
#include <string>
#include <stdexcept>

#include <openssl/ssl.h>
#include <openssl/dh.h>
#include <openssl/x509_vfy.h>
#include <openssl/stack.h>

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#ifdef _WIN32
#include <openssl/safestack.h>
#include <mdz_mem_vars/w32compat.h>
#endif


using namespace std;
using namespace Mantids::Network::Sockets;

Socket_TLS::Socket_TLS() : keys(&bIsServer)
{
#ifndef WIN32
    // Ignore sigpipes in this thread (eg. SSL_Write on closed socket):
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
#endif

    setCertValidation(CERT_X509_VALIDATE);
    tlsParent = nullptr;
    bIsServer = false;
    sslh = nullptr;
    ctx = nullptr;
}

Socket_TLS::~Socket_TLS()
{
    if (sslh)
        SSL_free (sslh);
    if (ctx)
        SSL_CTX_free(ctx);
}

void Socket_TLS::prepareTLS()
{
    // Register the error strings for libcrypto & libssl
    SSL_load_error_strings ();
    ERR_load_crypto_strings();
    // Register the available ciphers and digests
    SSL_library_init ();

#ifndef _WIN32
    sigset_t sigPipeSet;
    sigemptyset(&sigPipeSet);
    sigaddset(&sigPipeSet, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &sigPipeSet, NULL);
#endif
}

void Socket_TLS::setTLSParent(Socket_TLS *parent)
{
    tlsParent = parent;
}

bool Socket_TLS::postConnectSubInitialization()
{
    if (sslh!=nullptr)
        return false; // already connected (don't connect again)

    bIsServer = false;

    if (!createTLSContext())
    {
        return false;
    }

    if (!(sslh = SSL_new(ctx)))
    {
        sslErrors.push_back("SSL_new failed.");
        return false;
    }    

    SSL_set_mode(sslh, SSL_MODE_ENABLE_PARTIAL_WRITE);
    SSL_set_mode(sslh, SSL_MODE_AUTO_RETRY);

    // If there is any configured PSK, put the key in the static list here...
    bool usingPSK = keys.linkPSKWithTLSHandle(sslh);

    // Initialize TLS client certificates and keys
    if (!keys.initTLSKeys(ctx,sslh, &sslErrors))
    {
        parseErrors();
        return false;
    }    

    if ( !(keys.getCAPath().empty()) || keys.getVerifyDefaultLocations() )
        SSL_set_verify(sslh, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT | (certValidation==CERT_X509_NOVALIDATE?SSL_VERIFY_NONE:0) , nullptr);
    else
    {
        // If there is no CA...
        certValidation=CERT_X509_NOVALIDATE;
        //SSL_set_verify(sslh, CERT_X509_NOVALIDATE, nullptr);
    }


    if (SSL_set_fd (sslh, sockfd) != 1)
    {
        sslErrors.push_back("SSL_set_fd failed.");
        return false;
    }

    if ( SSL_get_error(sslh, SSL_connect (sslh)) != SSL_ERROR_NONE )
    {
        parseErrors();
        return false;
    }

    if ( certValidation!=CERT_X509_NOVALIDATE )
    {
        // Using PKI, need to validate the certificate.
        // connected+validated!
        return validateTLSConnection(usingPSK) || certValidation==CERT_X509_CHECKANDPASS;
    }
    // no validate here...
    else
        return true;
}

bool Socket_TLS::postAcceptSubInitialization()
{
    if (sslh!=nullptr)
        return false; // already connected (don't connect again)

    bIsServer = true;

    if (!createTLSContext())
    {
        return false;
    }

    // ssl empty, create a new one.
    if (!(sslh = SSL_new(ctx)))
    {
        sslErrors.push_back("SSL_new failed.");
        return false;
    }

    SSL_set_mode(sslh, SSL_MODE_ENABLE_PARTIAL_WRITE);
    SSL_set_mode(sslh, SSL_MODE_AUTO_RETRY);

    // If the parent have any PSK key, pass everything to the current socket_tls.
    *(keys.getPSKServerWallet()) = *(tlsParent->keys.getPSKServerWallet());

    bool usingPSK = getIsUsingPSK();
    // If there is any configured PSK, link the key in the static list...
    if ( usingPSK )
    {
        keys.linkPSKWithTLSHandle(sslh);
    }

    // in server mode, use the parent keys...
    if (!tlsParent->keys.initTLSKeys(ctx,sslh, &sslErrors))
    {
        parseErrors();
        return false;
    }

    if ( !tlsParent->keys.getCAPath().empty() || tlsParent->keys.getVerifyDefaultLocations() )
        SSL_set_verify(sslh, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT | (certValidation==CERT_X509_NOVALIDATE?SSL_VERIFY_NONE:0) , nullptr);
    else
    {
        // If there is no CA...
        certValidation=CERT_X509_NOVALIDATE;
        //SSL_set_verify(sslh, CERT_X509_NOVALIDATE, nullptr);
    }

    if (SSL_set_fd (sslh, sockfd) != 1)
    {
        sslErrors.push_back("SSL_set_fd failed.");
        return false;
    }

    int err;
    if ((err=SSL_accept(sslh)) != 1)
    {
        parseErrors();
        return false;
    }

    if ( certValidation!=CERT_X509_NOVALIDATE )
    {
        // Using PKI, need to validate the certificate.
        // connected+validated!
        return validateTLSConnection(usingPSK) || certValidation==CERT_X509_CHECKANDPASS;
    }
    // no validate here...
    else
        return true;
}

SSL_CTX *Socket_TLS::createServerSSLContext()
{
#if TLS_MAX_VERSION == TLS1_VERSION
    return SSL_CTX_new (TLSv1_server_method());
#elif TLS_MAX_VERSION == TLS1_1_VERSION
    return SSL_CTX_new (TLSv1_1_server_method());
#elif TLS_MAX_VERSION == TLS1_2_VERSION
    return SSL_CTX_new (TLSv1_2_server_method());
#elif TLS_MAX_VERSION >= TLS1_3_VERSION
    return SSL_CTX_new (TLS_server_method());
#endif
}

SSL_CTX *Socket_TLS::createClientSSLContext()
{
#if TLS_MAX_VERSION == TLS1_VERSION
    return SSL_CTX_new (TLSv1_client_method());
#elif TLS_MAX_VERSION == TLS1_1_VERSION
    return SSL_CTX_new (TLSv1_1_client_method());
#elif TLS_MAX_VERSION == TLS1_2_VERSION
    return SSL_CTX_new (TLSv1_2_client_method());
#elif TLS_MAX_VERSION >= TLS1_3_VERSION
    return SSL_CTX_new (TLS_client_method());
#endif
}

bool Socket_TLS::getIsServer() const
{
    return bIsServer;
}

bool Socket_TLS::getIsUsingPSK()
{
    if (bIsServer)
        return tlsParent->keys.getPSKServerWallet()->usingPSK;
    else
        return keys.getPSKClientValue()->usingPSK;
}


bool Socket_TLS::createTLSContext()
{
    // create new SSL Context.

    if (ctx)
    {
        throw std::runtime_error("Can't reuse the TLS socket. Create a new one.");
    }

    if (bIsServer)
    {
        ctx = createServerSSLContext();
        if (!ctx)
        {
            sslErrors.push_back("TLS_server_method() Failed.");
            return false;
        }
    }
    else
    {
        ctx = createClientSSLContext();
        if (!ctx)
        {
            sslErrors.push_back("TLS_client_method() Failed.");
            return false;
        }
    }


    return true;
}

void Socket_TLS::parseErrors()
{
    char buf[512];
    unsigned long int err;
    while ((err = ERR_get_error()) != 0)
    {
        ERR_error_string_n(err, buf, sizeof(buf));
        sslErrors.push_back(buf);
    }
}

bool Socket_TLS::validateTLSConnection(const bool & usingPSK)
{
    if (!sslh)
        return false;

    bool bValid  = false;

    if (!usingPSK)
    {
        X509 *cert;
        cert = SSL_get_peer_certificate(sslh);
        if ( cert != nullptr )
        {
            long res = SSL_get_verify_result(sslh);
            if (res == X509_V_OK)
            {
                bValid = true;
            }
            else
            {
                sslErrors.push_back("Peer TLS/SSL Certificate Verification Error (" + std::to_string(res) + "): " + std::string(X509_verify_cert_error_string(res)));
            }
            X509_free(cert);
        }
        else
            sslErrors.push_back("Peer TLS/SSL Certificate does not exist.");
    }
    else
    {
        bValid = true;
    }

    return bValid;
}

ssize_t Socket_TLS::iPartialRead(
    void *data, const uint32_t &datalen, int ttl)
{

    if (!sslh)
    {
        lastError = "SSL handle is null";
        return -1;
    }

    if (ttl == 0)
    {
        return -1;
    }

    int readBytes = SSL_read(sslh, data, static_cast<int>(datalen));

    if (readBytes > 0)
    {
        lastError = "";
        return readBytes;
    }
    else if (readBytes == 0)
    {
        // The connection has been closed.
        lastError = "Connection closed by peer";
        return 0;
    }

    // Error mgmt:
    int sslError = SSL_get_error(sslh, readBytes);
    switch (sslError)
    {
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
        // Try Again after 10ms...
        usleep(10000);
        return iPartialRead(data,datalen,ttl-1);
    case SSL_ERROR_SYSCALL:
    {
        int errnoCopy = errno;
        char errorBuffer[256];
        strerror_r(errnoCopy, errorBuffer, sizeof(errorBuffer));
        lastError = std::string("System call error: ") + errorBuffer;
        break;
    }
    case SSL_ERROR_ZERO_RETURN:
    case SSL_ERROR_SSL:
        // SSL Specific error.
        {
            parseErrors();
            lastError = std::string("SSL Layer Error");
            break;
        }
    default:
        lastError = "Unknown SSL error occurred";
    }

    // In case of error, close the connection.
    Socket_TCP::iShutdown();

    return -1;
}

ssize_t Socket_TLS::iPartialWrite(
    const void *data, const uint32_t &datalen, int ttl)
{
    if (!sslh)
    {
        return -1;
    }

    if (ttl == 0)
    {
        return -1;
    }

    int chunkSize = static_cast<int>(datalen);
    if ( datalen>static_cast<uint64_t>(std::numeric_limits<int>::max()) )
    {
        throw std::runtime_error("Data size exceeds the maximum allowed for partial write.");
    }

    // TODO: sigpipe here?
    int sentBytes = SSL_write(sslh, data, chunkSize);

    if (sentBytes > 0)
    {
        lastError = "";
        return sentBytes;
    }
    else if (sentBytes == 0)
    {
        // Closed.
        lastError = "Connection closed";
        return sentBytes;
    }
    else
    {
        int sslErr = SSL_get_error(sslh, sentBytes);
        switch(sslErr) {
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
            // Wait 10ms... and try again...
            usleep(10000);
            return iPartialWrite(data,datalen,ttl-1);
        case SSL_ERROR_SYSCALL:
        {
            int errnoCopy = errno;
            char errorBuffer[256];
            strerror_r(errnoCopy, errorBuffer, sizeof(errorBuffer));
            lastError = std::string("System call error: ") + errorBuffer;
            parseErrors();
            Socket_TCP::iShutdown();
            return -1;
        }
        case SSL_ERROR_ZERO_RETURN:
        case SSL_ERROR_SSL:
        default:
            // Other SSL errors
            lastError = std::string("SSL Layer Error");
            parseErrors();
            Socket_TCP::iShutdown();
            return -1;
        }
    }

}

Socket_TLS::eCertValidationOptions Socket_TLS::getCertValidation() const
{
    return certValidation;
}

void Socket_TLS::setCertValidation(eCertValidationOptions newCertValidation)
{
    certValidation = newCertValidation;
}


string Socket_TLS::getTLSConnectionCipherName()
{
    if (!sslh) return "";
    return SSL_get_cipher_name(sslh);
}

std::list<std::string> Socket_TLS::getTLSErrorsAndClear()
{
    std::list<std::string> sslErrors2 = sslErrors;
    sslErrors.clear();
    return sslErrors2;
}

string Socket_TLS::getTLSPeerCN()
{
    if (!sslh) return "";

    if (!getIsUsingPSK())
    {
        char certCNText[512];
        memset(certCNText,0,sizeof(certCNText));

        X509 * cert = SSL_get_peer_certificate(sslh);
        if(cert)
        {
            X509_NAME * certName = X509_get_subject_name(cert);
            if (certName)
                X509_NAME_get_text_by_NID(certName,NID_commonName,certCNText,511);
            X509_free(cert);
        }
        return std::string(certCNText);
    }
    else
    {
        // the remote host is the server... no configured id.
        if (!bIsServer)
            return "server";
        else
            return keys.getPSKServerWallet()->connectedClientID;
    }
}

int Socket_TLS::iShutdown(int mode)
{
    if (!sslh && getIsServer())
    {
        // Then is a listening socket... (shutdown like a normal tcp/ip connection)
        return Socket::iShutdown(mode);
    }
    else if (!sslh)
    {
        return -4;
    }
    else if (shutdown_proto_wr || (SSL_get_shutdown(sslh) & SSL_SENT_SHUTDOWN))
    {
        // Already shutted down.
        return -1;
    }
    else
    {

        // Messages from https://www.openssl.org/docs/manmaster/man3/SSL_shutdown.html
        switch (SSL_shutdown (sslh))
        {
        case 0:
            // The shutdown is not yet finished: the close_notify was sent but the peer did not send it back yet. Call SSL_read() to do a bidirectional shutdown.
            return -2;
        case 1:
            // The write shutdown was successfully completed. The close_notify alert was sent and the peer's close_notify alert was received.
            //shutdown_proto_rd = true;
            shutdown_proto_wr = true;
            return 0;
            // Call the private TCP iShutdown to shutdown RD on the socket ?...
            //return Socket_TCP::iShutdown(mode);
        default:
            //return Socket::iShutdown(); //The shutdown was not successful. Try... socket shutdown.
            return -3;
        }
    }
}

bool Socket_TLS::isSecure() { return true; }

void Socket_TLS::setIsServerMode(bool value)
{
    bIsServer = value;
}

Socket_TLS::sCipherBits Socket_TLS::getTLSConnectionCipherBits()
{
    sCipherBits cb;
    if (!sslh)
        return cb;
    cb.aSymBits = SSL_get_cipher_bits(sslh, &cb.symBits);
    return cb;
}

string Socket_TLS::getTLSConnectionProtocolVersion()
{
    if (!sslh)
        return "";
    return SSL_get_version(sslh);
}

Mantids::Network::Sockets::Socket_StreamBase * Socket_TLS::acceptConnection()
{
    char remotePair[INET6_ADDRSTRLEN];

    bIsServer = true;

    Socket_StreamBase * acceptedTCPSock = Socket_TCP::acceptConnection();

    if (!acceptedTCPSock)
        return nullptr;

    Socket_TLS * acceptedTLSSock = new Socket_TLS; // Convert to this thing...

    // Set current retrieved socket info.
    acceptedTCPSock->getRemotePair(remotePair);
    acceptedTLSSock->setRemotePair(remotePair);
    acceptedTLSSock->setRemotePort(acceptedTCPSock->getRemotePort());

    // Inehrit certificates...
    acceptedTLSSock->setTLSParent(this);

    // Set contexts and modes...
    acceptedTLSSock->setIsServerMode(bIsServer);

    // now we should copy the file descriptor:
    acceptedTLSSock->setSocketFD(acceptedTCPSock->adquireSocketFD());
    delete acceptedTCPSock;

    // After this, the postInitialization will be called by the acceptor thread.
    return acceptedTLSSock;
}

ssize_t Socket_TLS::partialRead(void *data, const uint32_t &datalen)
{
    std::unique_lock<std::mutex> lock(mutexRead);

    return iPartialRead(data,datalen);
}

ssize_t Socket_TLS::partialWrite(const void * data, const uint32_t & datalen)
{
    std::unique_lock<std::mutex> lock(mutexWrite);

    return iPartialWrite(data,datalen);
}
