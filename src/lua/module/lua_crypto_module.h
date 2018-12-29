#ifndef __LUA_CRYPTO_MODULE_H__
#define __LUA_CRYPTO_MODULE_H__

#include "lua_module_mgr.h"
#include "utility/crypto.h"
#include "utility/iobuffer.h"

namespace bee {

////////////////////////////////////LuaCryptoModule//////////////////////////////////////
class LuaVideoCache;
class LuaCryptoModule : public LuaModuleBase<LuaCryptoModule> {
public:
    LuaCryptoModule();
    ~LuaCryptoModule();

public:
    static int32_t lua_open_module(lua_State *l);
    static int32_t lua_encrypt(lua_State *l);
    static int32_t lua_decrypt(lua_State *l);
    static int32_t lua_decrypt_cache(lua_State *l);
    static int32_t lua_read_dec_cache(lua_State *l);

protected:
    bool encrypt(AlgoType algo, const uint8_t *input, int32_t input_len, const uint8_t *key, int32_t key_len, uint8_t *output, int32_t &output_len);
    bool decrypt(AlgoType algo, const uint8_t *input, int32_t input_len, const uint8_t *key, int32_t key_len, uint8_t *output, int32_t &output_len);
    bool decrypt_cache(AlgoType algo, LuaVideoCache *cache, size_t size, const uint8_t *key, int32_t key_len, int32_t &output_len);
    bool allocate_decrypt_cache(size_t size);
    bool fill_dec_input(LuaVideoCache *cache, size_t size);
    bool read_dec_cache(uint8_t *buff, size_t len, size_t &read_len);
    bool decrypt_buffer(AlgoType algo, IOBuffer *input, const uint8_t *key, int32_t key_len, IOBuffer *output);

protected:
    std::shared_ptr<IOBuffer> dec_input_;
    std::shared_ptr<IOBuffer> dec_output_;
};

} // namespace bee

#endif
