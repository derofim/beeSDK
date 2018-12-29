//
//  TestOpenssl.cpp
//  TestOpenssl
//
//  Created by 孙磊 on 2018/2/7.
//  Copyright © 2018年 sohu. All rights reserved.
//

#include "TestOpenssl.h"

#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/tls1.h>
#include <openssl/x509v3.h>
#include <openssl/ssl.h>
#include <openssl/dtls1.h>



void TestOpenSSL::test() {
    SSL_library_init();
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();
    RAND_poll();
    
    const SSL_METHOD* method = DTLS_server_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    SSL *ssl_ = SSL_new(ctx);

    int r = SSL_set_app_data(ssl_, this);
    printf("ssl add %x return %d \n",(unsigned int)(long)this, r);
    void *p = SSL_get_app_data(ssl_);
    printf("ssl get %x, this %x\n", (unsigned int)(long)p, (unsigned int)(long)this);
}

void TestOpenSSL::test1() {
    a_ = 0;
}
