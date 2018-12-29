#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#include "common.h"
#include <zlib.h>

namespace bee {

///////////////////////////////////Global Definition///////////////////////////////////////
typedef enum TransType {
    DO_ENC = 0x10,
    DO_DEC,
}TransType;

typedef enum AlgoType {
    ALGO_AES = 0,		// ascii of '0'
    ALGO_RC4,
    ALGO_XXTEA,
    ALGO_COUNT
}AlgoType;

#define RSA_BIT_WIDTH               1024
#define PADDING_TYPE	            RSA_PKCS1_PADDING
#define CA_SIGNATURE_SIZE           (18 + SHA_DIGEST_LENGTH)
#define ENCRYPT_KEY_SIZE	        16
#define ENCRYPT_BUFFER_SIZE         (RSA_BIT_WIDTH/8)

#if (RSA_BIT_WIDTH == 1024)
//#pragma message("RSA bits 1024")
// 1024 bits RSA
#define PUBLIC_KEY_LENGTH           272
#define SIGNATURE_LENGTH            54 //(4*4+2+20)
#define ENCRYPT_KEY_LENGTH          891
#elif (RSA_BIT_WIDTH == 512) 
//#pragma message("RSA bits 1024")
// 512 bits RSA
#define PUBLIC_KEY_LENGTH           182
#define SIGNATURE_LENGTH            54 //(4*4+2+20)
#define ENCRYPT_KEY_LENGTH          497
#endif

const int32_t kAESBlockSizeMask = 0xF;

////////////////////////////////////xxtea//////////////////////////////////////
char* xxtea_encrypt(const char *data, size_t len, const char* key, size_t * out_len);
char* xxtea_decrypt(const char* data, size_t len, const char* key, size_t * out_len);

////////////////////////////////////rsa//////////////////////////////////////
bool rsa_generate_key(RSA *&rsa_key, std::string &pub_key_str);
bool rsa_generate_from_pub_key(const char *pub_key, size_t pub_key_len, RSA *&rsa);
bool rsa_transfer(RSA *rsa, uint8_t *in_data, const uint32_t in_len, uint8_t *out_data, int32_t &out_len, const TransType type, int32_t padding);
bool rsa_encrypt(RSA *rsa, uint8_t *in_data, const uint32_t in_len, uint8_t *out_data,int32_t &out_len, int32_t padding);
bool rsa_decrypt(RSA *rsa, uint8_t *in_data, const uint32_t in_len, uint8_t *out_data, int32_t &out_len, int32_t padding);

////////////////////////////////////rc4//////////////////////////////////////
bool rc4(const uint8_t *input, int32_t input_len, const uint8_t *key, int32_t key_len, uint8_t *output, int32_t &output_len);

////////////////////////////////////aes-ecb//////////////////////////////////////
bool aes_encrypt_ecb(const uint8_t *input, int32_t input_len, const uint8_t *key, int32_t key_len, uint8_t *output, int32_t &output_len);
bool aes_decrypt_ecb(const uint8_t *input, int32_t input_len, const uint8_t *key, int32_t key_len, uint8_t *output, int32_t &output_len);

////////////////////////////////////base64//////////////////////////////////////
std::string base64_encode(unsigned char *buffer, unsigned int length);
std::string base64_decode(unsigned char *buffer, unsigned int length);

////////////////////////////////////sha1//////////////////////////////////////
bool sha1(uint8_t *in_data, size_t in_len, uint8_t *out_data, int32_t out_len);

////////////////////////////////////gz//////////////////////////////////////
bool gz_compress(Bytef *data, uLong ndata, Bytef *zdata, uLong *nzdata);
int32_t gz_decompress(Byte *zdata, uLong nzdata, Byte *data, uLong *ndata);

//////////////////////////////////cleanup////////////////////////////////////////
void cleanup_crypto_data();

} // namespace bee

#endif // _CRYPTO_H
