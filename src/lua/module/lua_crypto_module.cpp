#include "lua_crypto_module.h"
#include "lua_video_cache.h"

namespace bee {

LuaCryptoModule::LuaCryptoModule() {

}

LuaCryptoModule::~LuaCryptoModule() {

}

int32_t LuaCryptoModule::lua_open_module(lua_State *l) {
    luaL_Reg reg[] = {
        { "encrypt",        LuaCryptoModule::lua_encrypt },
        { "decrypt",        LuaCryptoModule::lua_decrypt },
        { "decrypt_cache",  LuaCryptoModule::lua_decrypt_cache },
        { "read_dec_cache", LuaCryptoModule::lua_read_dec_cache },
        { NULL,             NULL }
    };

    std::string name = "crypto";
    std::string version = "1.0.1";
    return LuaModuleBase::open_module(l, reg, name, version);
}

int32_t LuaCryptoModule::lua_encrypt(lua_State *l) {
    ModuleWrapper<LuaCryptoModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return -1;
    }

    int32_t i = (int32_t)lua_tointeger(l, -1);
    lua_settop(l, 0);
    lua_pushinteger(l, i);
    return 1;
}

int32_t LuaCryptoModule::lua_decrypt(lua_State *l) {
    int32_t ret = -1;
    do {
        if (!check_args(l, 6)) {
            break;
        }

        ModuleWrapper<LuaCryptoModule> *wrapper = get_module_wrapper(l);
        if (wrapper == NULL) {
            break;
        }

        int32_t algo_offset         = 1;
        int32_t input_buff_offset   = 2;
        int32_t input_len_offset    = 3;
        int32_t key_offset          = 4;
        int32_t output_buff_offset  = 5;
        int32_t output_len_offset   = 6;

        if (!lua_isinteger(l, algo_offset) || 
            !lua_islightuserdata(l, input_buff_offset) || 
            !lua_isinteger(l, input_len_offset) || 
            !lua_isstring(l, key_offset) ||
            !lua_islightuserdata(l, output_buff_offset) || 
            !lua_isinteger(l, output_len_offset)) {
            break;
        }

        int32_t algo = (int32_t)lua_tointeger(l, algo_offset);
        if (algo < 0 || algo >= ALGO_COUNT) {
            break;
        }

        uint8_t *input_buff = (uint8_t*)lua_topointer(l, input_buff_offset);
        if (input_buff == NULL) {
            break;
        }

        int32_t input_len = (int32_t)lua_tointeger(l, input_len_offset);
        if (input_len <= 0) {
            break;
        }

        const char *key = (char*)lua_tostring(l, key_offset);
        if (key == NULL) {
            break;
        }

        uint8_t *output_buff = (uint8_t*)lua_topointer(l, output_buff_offset);
        if (output_buff == NULL) {
            break;
        }

        int32_t output_len = (int32_t)lua_tointeger(l, output_len_offset);
        if (output_len <= 0) {
            break;
        }

        if (!wrapper->module->decrypt(static_cast<AlgoType>(algo), input_buff, (size_t)input_len, (uint8_t*)key, strlen(key), output_buff, output_len)) {
            break;
        }

        lua_settop(l, 0);
        lua_pushinteger(l, output_len);
        ret = 1;
    } while (0);
    return ret;
}

int32_t LuaCryptoModule::lua_decrypt_cache(lua_State *l) {
    int32_t ret = -1;
    do {
        if (!check_args(l, 4)) {
            break;
        }

        ModuleWrapper<LuaCryptoModule> *wrapper = get_module_wrapper(l);
        if (wrapper == NULL) {
            break;
        }

        int32_t algo_offset = -4;
        int32_t cache_offset = -3;
        int32_t len_offset = -2;
        int32_t key_offset = -1;

        if (!lua_isinteger(l, algo_offset) || !lua_islightuserdata(l, cache_offset) || !lua_isinteger(l, len_offset) || !lua_isstring(l, key_offset)) {
            break;
        }

        int32_t algo = (int32_t)lua_tointeger(l, algo_offset);
        if (algo < 0 || algo >= ALGO_COUNT) {
            break;
        }

        LuaVideoCache *cache = (LuaVideoCache*)lua_topointer(l, cache_offset);
        if (cache == NULL) {
            break;
        }

        int32_t len = (int32_t)lua_tointeger(l, len_offset);
        if (len <= 0) {
            break;
        }

        const char *key = (char*)lua_tostring(l, key_offset);
        if (key == NULL) {
            break;
        }

        int32_t output_len = 0;
        if (!wrapper->module->decrypt_cache(static_cast<AlgoType>(algo), cache, (size_t)len, (uint8_t*)key, strlen(key), output_len)) {
            break;
        }

        lua_settop(l, 0);
        lua_pushinteger(l, output_len);
        ret = 1;
    } while (0);
    return ret;
}

int32_t LuaCryptoModule::lua_read_dec_cache(lua_State *l) {
    int32_t ret = -1;
    do {
        if (!check_args(l, 2)) {
            break;
        }

        ModuleWrapper<LuaCryptoModule> *wrapper = get_module_wrapper(l);
        if (wrapper == NULL) {
            break;
        }

        int32_t buff_offset = -2;
        int32_t len_offset = -1;

        if (!lua_islightuserdata(l, buff_offset) || !lua_isinteger(l, len_offset)) {
            break;
        }

        uint8_t *buf = (uint8_t *)lua_topointer(l, buff_offset);
        uint32_t len = (uint32_t)lua_tointeger(l, len_offset);
        if (buf == NULL || len == 0) {
            break;
        }

        size_t read_len = 0;
        if (!wrapper->module->read_dec_cache(buf, len, read_len)) {
            break;
        }

        lua_settop(l, 0);
        lua_pushinteger(l, read_len);
        ret = 1;
    } while (0);
    return ret;
}

bool LuaCryptoModule::encrypt(AlgoType algo, const uint8_t *input, int32_t input_len, const uint8_t *key, int32_t key_len, uint8_t *output, int32_t &output_len) {
    bool ret = true;
    do {
        switch (algo) {
        case ALGO_AES:
            ret = aes_encrypt_ecb(input, input_len, key, key_len, output, output_len);
            break;
        case ALGO_RC4:
            ret = rc4(input, input_len, key, key_len, output, output_len);
            break;
        case ALGO_XXTEA:
        default:
            ret = false;
            break;
        }
    } while (0);
    return ret;
}

bool LuaCryptoModule::decrypt(AlgoType algo, const uint8_t *input, int32_t input_len, const uint8_t *key, int32_t key_len, uint8_t *output, int32_t &output_len) {
    bool ret = true;
    do {
        switch (algo) {
        case ALGO_AES:
            ret = aes_decrypt_ecb(input, input_len, key, key_len, output, output_len);
            break;
        case ALGO_RC4:
            ret = rc4(input, input_len, key, key_len, output, output_len);
            break;
        case ALGO_XXTEA:
        default:
            ret = false;
            break;
        }
    } while (0);
    return ret;
}

bool LuaCryptoModule::decrypt_cache(AlgoType algo, LuaVideoCache *cache, size_t size, const uint8_t *key, int32_t key_len, int32_t &output_len) {
    bool ret = true;
    do {
        ret = allocate_decrypt_cache(size);
        if (!ret) {
            break;
        }

        if (dec_input_ == NULL || dec_output_ == NULL) {
            ret = false;
            break;
        }

        ret = fill_dec_input(cache, size);
        if (!ret) {
            break;
        }

        ret = decrypt(algo, (uint8_t*)dec_input_->data(),dec_input_->size(), key, key_len, (uint8_t*)dec_output_->data(), output_len);
        if (!ret) {
            break;
        }

        dec_output_->resize((size_t)output_len);
    } while (0);
    return ret;
}

bool LuaCryptoModule::allocate_decrypt_cache(size_t size) {
    bool ret = true;
    do {
        if (size == 0) {
            ret = false;
            break;
        }

        if (dec_input_ == NULL || dec_input_->capacity() < size) {
            dec_input_.reset(new IOBuffer(size));
        }

        if (dec_output_ == NULL || dec_output_->capacity() < size) {
            dec_output_.reset(new IOBuffer(size));
        }
    } while (0);
    return ret;
}

bool LuaCryptoModule::fill_dec_input(LuaVideoCache *cache, size_t size) {
    bool ret = true;
    do {
        if (cache == NULL || size == 0) {
            ret = false;
            break;
        }

        dec_input_->resize(size);

        //在LUA层必须保证数据充足
        if (size != cache->ReadData((uint8_t*)dec_input_->data(), size)) {
            ret = false;
            break;
        }
    } while (0);
    return ret;
}

bool LuaCryptoModule::read_dec_cache(uint8_t *buff, size_t len, size_t &read_len) {
    bool ret = true;
    do {
        if (buff == NULL || len == 0) {
            ret = false;
            break;
        }

        read_len = std::min<size_t>(len, dec_output_->size());
        memcpy(buff, dec_output_->data(), read_len);
        dec_output_->consume(read_len);
    } while (0);
    return ret;
}

bool LuaCryptoModule::decrypt_buffer(AlgoType algo, IOBuffer *input, const uint8_t *key, int32_t key_len, IOBuffer *output) {
    bool ret = true;
    do {
        if (input == NULL || output == NULL) {
            ret = false;
            break;
        }

        if (input->empty() || output->empty()) {
            ret = false;
            break;
        }

        int32_t output_len = 0;
        ret = decrypt(algo, (uint8_t*)input->data(),input->size(), key, key_len, (uint8_t*)output->data(), output_len);
        if (!ret) {
            break;
        }

        output->resize((size_t)output_len);
    } while (0);
    return ret;
}

} // namespace bee
