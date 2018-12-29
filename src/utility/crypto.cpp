#include "crypto.h"
#include "log/logger.h"
#include <sys/types.h> /* This will likely define BYTE_ORDER */

#ifndef BYTE_ORDER
#if (BSD >= 199103)
# include <machine/endian.h>
#else
#if defined(linux) || defined(__linux__)
# include <endian.h>
#else
#define LITTLE_ENDIAN   1234    /* least-significant byte first (vax, pc) */
#define BIG_ENDIAN      4321    /* most-significant byte first (IBM, net) */
#define PDP_ENDIAN      3412    /* LSB first in word, MSW first in long (pdp)*/

#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__) || \
    defined(vax) || defined(ns32000) || defined(sun386) || \
    defined(MIPSEL) || defined(_MIPSEL) || defined(BIT_ZERO_ON_RIGHT) || \
    defined(__alpha__) || defined(__alpha)
#define BYTE_ORDER    LITTLE_ENDIAN
#endif

#if defined(sel) || defined(pyr) || defined(mc68000) || defined(sparc) || \
    defined(is68k) || defined(tahoe) || defined(ibm032) || defined(ibm370) || \
    defined(MIPSEB) || defined(_MIPSEB) || defined(_IBMR2) || defined(DGUX) ||\
    defined(apollo) || defined(__convex__) || defined(_CRAY) || \
    defined(__hppa) || defined(__hp9000) || \
    defined(__hp9000s300) || defined(__hp9000s700) || \
    defined (BIT_ZERO_ON_LEFT) || defined(m68k) || defined(__sparc)
#define BYTE_ORDER  BIG_ENDIAN
#endif
#endif /* linux */
#endif /* BSD */
#endif /* BYTE_ORDER */

#ifndef BYTE_ORDER
#ifdef __BYTE_ORDER
#if defined(__LITTLE_ENDIAN) && defined(__BIG_ENDIAN)
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN __LITTLE_ENDIAN
#endif
#ifndef BIG_ENDIAN
#define BIG_ENDIAN __BIG_ENDIAN
#endif
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define BYTE_ORDER LITTLE_ENDIAN
#else
#define BYTE_ORDER BIG_ENDIAN
#endif
#endif
#endif
#endif

#define MX (((z >> 5) ^ (y << 2)) + ((y >> 3) ^ (z << 4))) ^ ((sum ^ y) + (key[(p & 3) ^ e] ^ z))
#define DELTA 0x9e3779b9

#define FIXED_KEY \
    size_t i;\
    uint8_t fixed_key[16];\
    memcpy(fixed_key, key, 16);\
    for (i = 0; (i < 16) && (fixed_key[i] != 0); ++i);\
    for (++i; i < 16; ++i) fixed_key[i] = 0;\

namespace bee {

///////////////////////////////////xxtea///////////////////////////////////////
static uint32_t * xxtea_to_uint_array(const uint8_t * data, size_t len, int inc_len, size_t * out_len) {
    uint32_t *out;
#if !(defined(BYTE_ORDER) && (BYTE_ORDER == LITTLE_ENDIAN))
    size_t i;
#endif
    size_t n;

    n = (((len & 3) == 0) ? (len >> 2) : ((len >> 2) + 1));

    if (inc_len) {
        out = (uint32_t *)calloc(n + 1, sizeof(uint32_t));
        if (!out) return NULL;
        out[n] = (uint32_t)len;
        *out_len = n + 1;
    } else {
        out = (uint32_t *)calloc(n, sizeof(uint32_t));
        if (!out) return NULL;
        *out_len = n;
    }
#if defined(BYTE_ORDER) && (BYTE_ORDER == LITTLE_ENDIAN)
    memcpy(out, data, len);
#else
    for (i = 0; i < len; ++i) {
        out[i >> 2] |= (uint32_t)data[i] << ((i & 3) << 3);
    }
#endif

    return out;
}

static uint8_t * xxtea_to_ubyte_array(const uint32_t * data, size_t len, int inc_len, size_t * out_len) {
    uint8_t *out;
#if !(defined(BYTE_ORDER) && (BYTE_ORDER == LITTLE_ENDIAN))
    size_t i;
#endif
    size_t m, n;

    n = len << 2;

    if (inc_len) {
        m = data[len - 1];
        n -= 4;
        if ((m < n - 3) || (m > n)) return NULL;
        n = m;
    }

    out = (uint8_t *)malloc(n + 1);

#if defined(BYTE_ORDER) && (BYTE_ORDER == LITTLE_ENDIAN)
    memcpy(out, data, n);
#else
    for (i = 0; i < n; ++i) {
        out[i] = (uint8_t)(data[i >> 2] >> ((i & 3) << 3));
    }
#endif

    out[n] = '\0';
    *out_len = n;

    return out;
}

static uint32_t * xxtea_uint_encrypt(uint32_t * data, size_t len, uint32_t * key) {
    uint32_t n = (uint32_t)len - 1;
    uint32_t z = data[n], y, p, q = 6 + 52 / (n + 1), sum = 0, e;

    if (n < 1) return data;

    while (0 < q--) {
        sum += DELTA;
        e = sum >> 2 & 3;

        for (p = 0; p < n; p++) {
            y = data[p + 1];
            z = data[p] += MX;
        }

        y = data[0];
        z = data[n] += MX;
    }

    return data;
}

static uint32_t * xxtea_uint_decrypt(uint32_t * data, size_t len, uint32_t * key) {
    uint32_t n = (uint32_t)len - 1;
    uint32_t z, y = data[0], p, q = 6 + 52 / (n + 1), sum = q * DELTA, e;

    if (n < 1) return data;

    while (sum != 0) {
        e = sum >> 2 & 3;

        for (p = n; p > 0; p--) {
            z = data[p - 1];
            y = data[p] -= MX;
        }

        z = data[n];
        y = data[0] -= MX;
        sum -= DELTA;
    }

    return data;
}

static uint8_t * xxtea_ubyte_encrypt(const uint8_t * data, size_t len, const uint8_t * key, size_t * out_len) {
    uint8_t *out;
    uint32_t *data_array, *key_array;
    size_t data_len, key_len;

    if (!len) return NULL;

    data_array = xxtea_to_uint_array(data, len, 1, &data_len);
    if (!data_array) return NULL;

    key_array  = xxtea_to_uint_array(key, 16, 0, &key_len);
    if (!key_array) {
        free(data_array);
        return NULL;
    }

    out = xxtea_to_ubyte_array(xxtea_uint_encrypt(data_array, data_len, key_array), data_len, 0, out_len);

    free(data_array);
    free(key_array);

    return out;
}

static uint8_t * xxtea_ubyte_decrypt(const uint8_t * data, size_t len, const uint8_t * key, size_t * out_len) {
    uint8_t *out;
    uint32_t *data_array, *key_array;
    size_t data_len, key_len;

    if (!len) return NULL;

    data_array = xxtea_to_uint_array(data, len, 0, &data_len);
    if (!data_array) return NULL;

    key_array  = xxtea_to_uint_array(key, 16, 0, &key_len);
    if (!key_array) {
        free(data_array);
        return NULL;
    }

    out = xxtea_to_ubyte_array(xxtea_uint_decrypt(data_array, data_len, key_array), data_len, 1, out_len);

    free(data_array);
    free(key_array);

    return out;
}

// public functions

char* xxtea_encrypt(const char* data, size_t len, const char* key, size_t * out_len) {
    FIXED_KEY
        return (char *)xxtea_ubyte_encrypt((uint8_t *)data, len, fixed_key, out_len);
}

char* xxtea_decrypt(const char* data, size_t len, const char* key, size_t * out_len) {
    FIXED_KEY
        return (char *)xxtea_ubyte_decrypt((uint8_t *)data, len, fixed_key, out_len);
}

////////////////////////////////////rsa//////////////////////////////////////
bool rsa_generate_key(RSA *&rsa_key, std::string &pub_key_str) {
    bool    ret  = true;
    RSA     *rsa = NULL;
    BIGNUM  *bn  = NULL;
    BIO     *bio = NULL;
    do {
        rsa = RSA_new();
        if (NULL == rsa) {
            ret = false;
            break;
        }

        bn = BN_new();
        if (bn == NULL) {
            ret = false;
            break;
        }

        if (1 != BN_set_word(bn, RSA_F4)) {
            ret = false;
            break;
        }

        if (1 != RSA_generate_key_ex(rsa, RSA_BIT_WIDTH, bn, NULL)) {
            ret = false;
            break;
        }

        bio = BIO_new(BIO_s_mem());
        if (bio == NULL) {
            ret = false;
            break;
        }

        PEM_write_bio_RSA_PUBKEY(bio, rsa);
        size_t len = BIO_pending(bio);
        pub_key_str.resize(len);
        BIO_read(bio, (char*)pub_key_str.data(), len);
        rsa_key = rsa;
        rsa = NULL;
    } while (0);

    if (rsa != NULL) {
        RSA_free(rsa);
    }

    if (bn != NULL) {
        BN_free(bn);
    }

    if (bio != NULL) {
        BIO_free(bio);
    }
    return ret;
}

bool rsa_generate_from_pub_key(const char *pub_key, size_t pub_key_len, RSA *&rsa) {
    bool ret = true;
    BIO *keybio = NULL;
    do {
        if (pub_key == NULL || pub_key_len == 0) {
            ret = false;
            break;
        }

        keybio = BIO_new_mem_buf((void *)pub_key, pub_key_len);
        if (NULL == keybio) {
            break;
        }

        rsa = PEM_read_bio_RSA_PUBKEY(keybio, NULL, NULL, NULL);        
        if (rsa == NULL) {
            ret = false;
            break;
        }
    } while (0);

    if (keybio != NULL) {
        BIO_free(keybio);
    }
    return ret;
}

bool rsa_transfer(RSA *rsa, uint8_t *in_data, const uint32_t in_len, uint8_t *out_data, int32_t &out_len, const TransType type, int32_t padding) {
    bool ret = true;
    do {
        if (DO_ENC == type) {
            // in len should smaller than rsa_len - 11 if using RSA_PKCS!_PADDING
            if ((out_len = RSA_public_encrypt(in_len, in_data, out_data, rsa, padding)) < 0) {
                ERR_load_crypto_strings();
                int32_t ec = ERR_get_error();
                const char * lib = ERR_lib_error_string(ec);
                const char * fun = ERR_func_error_string(ec);
                const char * reason = ERR_reason_error_string(ec);
                Logger::ER("RSA", "\n\033[01;31mRSA_public_encrypt failed.\n%s\n%s\n%s\n",lib,fun,reason);
                ERR_free_strings();
                ret = false;
                break;
            }
        } else if (DO_DEC == type) {
            if ( (out_len = RSA_private_decrypt(in_len, in_data, out_data, rsa, padding)) < 0) {
                char buf[256] = {0};
                ERR_load_crypto_strings();
                ERR_error_string(ERR_get_error(),buf);
                Logger::ER("RSA", "\n\033[01;31mRSA_private_decrypt failed.%s\n",buf);
                ERR_free_strings();
                ret = false;
                break;
            }
        }
    } while (0);
    return ret;
}

bool rsa_encrypt(RSA *rsa, uint8_t *in_data, const uint32_t in_len, uint8_t *out_data,int32_t &out_len, int32_t padding) {
    bool ret = true;
    do {
        if (rsa == NULL || in_data == NULL || in_len == 0 || out_data == NULL) {
            ret = false;
            break;
        }

        uint32_t real_len = 0;
        uint32_t rsa_size = RSA_size(rsa);
        if (padding == RSA_PKCS1_PADDING) {
            real_len = in_len > (rsa_size - 11 ) ? (rsa_size - 11) : in_len;
        } else if (padding == RSA_NO_PADDING) {
            real_len = rsa_size;
        } else if (padding == RSA_PKCS1_OAEP_PADDING) {
            real_len = in_len > (rsa_size - 41 ) ? (rsa_size - 41) : in_len;
        }

        ret = rsa_transfer(rsa, in_data, real_len, out_data, out_len, DO_ENC, padding);
        if (!ret) {
            break;
        }

        if (out_len == -1) {
            ret = false;
            break;
        }
    } while (0);
    return ret;

}

bool rsa_decrypt(RSA *rsa, uint8_t *in_data, const uint32_t in_len, uint8_t *out_data, int32_t &out_len, int32_t padding) {
    bool ret = true;
    do {
        if (rsa == NULL || in_data == NULL || in_len == 0 || out_data == NULL) {
            ret = false;
            break;
        }

        ret = rsa_transfer(rsa, in_data, in_len, out_data, out_len, DO_DEC, padding);
        if (!ret) {
            break;
        }

        if (out_len == -1) {
            ret = false;
            break;
        }
    } while (0);
    return ret;
}

//////////////////////////////////////rc4////////////////////////////////////
bool rc4(const uint8_t *input, int32_t input_len, const uint8_t *key, int32_t key_len, uint8_t *output, int32_t &output_len) {
    bool ret = true;
    do {
        RC4_KEY rc4_key;
        RC4_set_key(&rc4_key, key_len, key);
        RC4(&rc4_key, input_len, (const unsigned char *)input, (unsigned char*)output);
        output_len = input_len;
    } while (0);
    return ret;
}

////////////////////////////////////aes-ecb//////////////////////////////////////
bool aes_encrypt_ecb(const uint8_t *input, int32_t input_len, const uint8_t *key, int32_t key_len, uint8_t *output, int32_t &output_len) {
    bool ret = true;
    EVP_CIPHER_CTX *ctx = NULL;
    uint8_t *enc_out = NULL;

    do {
        int32_t padding = AES_BLOCK_SIZE - (input_len & kAESBlockSizeMask);
        int32_t out_buffer_len = input_len + padding;
        enc_out = new uint8_t[out_buffer_len];

        ctx = EVP_CIPHER_CTX_new();
        if (NULL == ctx) {
            ret = false;
            break;
        }

        if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL)) {
            ret = false;
            break;
        }

        if (1 != EVP_EncryptUpdate(ctx, enc_out, &output_len, input, input_len)) {
            ret = false;
            break;
        }

        int32_t final_out_len = 0; 
        if (1 != EVP_EncryptFinal_ex(ctx, enc_out + output_len, &final_out_len)) {
            ret = false;
            break;
        }

        output = enc_out;
        output_len += final_out_len;
        enc_out = NULL;
    } while (0);

    if (ctx != NULL) {
        EVP_CIPHER_CTX_free(ctx);
    }

    if (enc_out != NULL) {
        delete [] enc_out;
    }

    return ret;
}

bool aes_decrypt_ecb(const uint8_t *input, int32_t input_len, const uint8_t *key, int32_t key_len, uint8_t *output, int32_t &output_len) {
    bool ret = true;
    EVP_CIPHER_CTX *ctx = NULL;

    do {
        if (input == NULL || output == NULL) {
            ret = false;
            break;
        }

        ctx = EVP_CIPHER_CTX_new();
        if (NULL == ctx) {
            ret = false;
            break;
        }

        if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL)) {
            ret = false;
            break;
        }

        if (1 != EVP_DecryptUpdate(ctx, output, &output_len, input, input_len)) {
            ret = false;
            break;
        }

        int32_t final_out_len = 0; 
        if (1 != EVP_DecryptFinal_ex(ctx, output + output_len, &final_out_len)) {
            ret = false;
            break;
        }

        output_len += final_out_len;
    } while (0);

    if (ctx != NULL) {
        EVP_CIPHER_CTX_free(ctx);
    }

    return ret;
}

////////////////////////////////////base64//////////////////////////////////////
static void replace(std::string &target, const std::string &search, const std::string &replacement) {
    if (search == replacement)
        return;
    if (search == "")
        return;
    std::string::size_type i = std::string::npos;
    std::string::size_type lastPos = 0;
    while ((i = target.find(search, lastPos)) != std::string::npos) {
        target.replace(i, search.length(), replacement);
        lastPos = i + replacement.length();
    }
}

std::string base64_encode(unsigned char *buffer, unsigned int length) {
    BIO *bmem;
    BIO *b64;
	BUF_MEM *bptr;

	b64 = BIO_new(BIO_f_base64());
	bmem = BIO_new(BIO_s_mem());

	b64 = BIO_push(b64, bmem);
	BIO_write(b64, buffer, length);
    std::string result;
	if (BIO_flush(b64) == 1) {
		BIO_get_mem_ptr(b64, &bptr);
		result = std::string(bptr->data, bptr->length);
	}

	BIO_free_all(b64);

	replace(result, "\n", "");
	replace(result, "\r", "");

	return result;
}

std::string base64_decode(unsigned char *buffer, unsigned int length) {
	// create a memory buffer containing base64 encoded data
	BIO* bmem = BIO_new_mem_buf((void *) buffer, length);

	// push a Base64 filter so that reading from buffer decodes it
	BIO *bioCmd = BIO_new(BIO_f_base64());
	// we don't want newlines
	BIO_set_flags(bioCmd, BIO_FLAGS_BASE64_NO_NL);
	bmem = BIO_push(bioCmd, bmem);

	char *pOut = new char[length];

	int finalLen = BIO_read(bmem, (void*) pOut, length);
	BIO_free_all(bmem);
    std::string result(pOut, finalLen);
	delete[] pOut;
	return result;
}

////////////////////////////////////sha1//////////////////////////////////////
bool sha1(uint8_t *in_data, size_t in_len, uint8_t *out_data, int32_t out_len) {
    if (in_data == NULL || in_len == 0 || out_data == NULL || out_len != SHA_DIGEST_LENGTH) {
        return false;
    }

    SHA_CTX c;
    SHA1_Init(&c);
    SHA1_Update(&c, in_data, in_len);
    SHA1_Final(out_data, &c);

    return true;
}

/////////////////////////////////////zlib/////////////////////////////////////
bool gz_compress(Bytef *data, uLong ndata, Bytef *zdata, uLong *nzdata) {
    z_stream c_stream;
    int err = 0;

    if (data && ndata > 0) {
        c_stream.zalloc = (alloc_func)0;
        c_stream.zfree = (free_func)0;
        c_stream.opaque = (voidpf)0;
        if (deflateInit2(&c_stream, /*Z_DEFAULT_COMPRESSION*/Z_BEST_SPEED, Z_DEFLATED,
            -MAX_WBITS, 9, /*Z_DEFAULT_STRATEGY*/Z_HUFFMAN_ONLY) != Z_OK) return false;
        c_stream.next_in  = data;
        c_stream.avail_in  = ndata;
        c_stream.next_out = zdata;
        c_stream.avail_out  = *nzdata;
        while (c_stream.avail_in != 0 && c_stream.total_out < *nzdata) {
            if (deflate(&c_stream, Z_NO_FLUSH) != Z_OK) return false;
        }
        if (c_stream.avail_in != 0) return true;
        for (;;) {
            if ((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END) break;
            if (err != Z_OK) return false;
        }
        if (deflateEnd(&c_stream) != Z_OK) return false;
        *nzdata = c_stream.total_out;
        return true;
    }
    return false;
}

int32_t gz_decompress(Byte *zdata, uLong nzdata, Byte *data, uLong *ndata) {
    int32_t err = 0;
    z_stream d_stream = {0}; /* decompression stream */
    static char dummy_head[2] = {
        0x8 + 0x7 * 0x10,
        (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
    };

    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;
    d_stream.next_in  = zdata;
    d_stream.avail_in = 0;
    d_stream.next_out = data;

    if (inflateInit2(&d_stream, -MAX_WBITS) != Z_OK) 
        return -1;

    while (d_stream.total_out < *ndata && d_stream.total_in < nzdata) {
        d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
        if ((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END) break;
        if (err != Z_OK ) {
            if (err == Z_DATA_ERROR)
            {
                d_stream.next_in = (Bytef*) dummy_head;
                d_stream.avail_in = sizeof(dummy_head);

                if ((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK)
                    return -1;
            }
            else 
                return -1;
        }
    }

    if (inflateEnd(&d_stream) != Z_OK)
        return -1;

    *ndata = d_stream.total_out;
    return 0;
}

//////////////////////////////////cleanup////////////////////////////////////////
void cleanup_crypto_data() {
    CRYPTO_cleanup_all_ex_data();
}

} // namespace bee
