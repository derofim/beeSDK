#include "lua_http_reader.h"
#include "lua_http_session.h"
#include "utility/iobuffer.h"

namespace bee {

LuaHttpReader::LuaHttpReader(
    std::shared_ptr<LuaHttpSession> http_session,
    uint8_t *buff, 
    size_t size, 
    lua_State *main, 
    lua_State *co): 
    http_session_(http_session),
    buff_(buff),
    size_(size),
    pos_(0),
    main_(main),
    co_(co) {

}

LuaHttpReader::~LuaHttpReader() {

}

bool LuaHttpReader::read() {
    bool ret = false;
    do {
        LuaHttpSession::Ptr http_session = http_session_.lock();
        if (http_session == NULL) {
            break;
        }

        //Post here,so as to break recursive,and yield & resume are necessary. 
        ret = http_session->async_load();
    } while (0);
    return ret;
}

void LuaHttpReader::close() {
    main_ = NULL;
    co_ = NULL;
}

bool LuaHttpReader::notify(BeeErrorCode e1, const boost::system::error_code &ec2, size_t size) {
    bool ret = true;
    do {
        if (e1 != kBeeErrorCode_Success || ec2) {
            ret = false;
            break;
        }

        LuaHttpSession::Ptr http_session = http_session_.lock();
        if (http_session == NULL) {
            ret = false;
            break;
        }

        size_t buf_left = size_ - pos_;
        size_t read_len = std::min<size_t>(size, buf_left);
        read_len = http_session->read_stream(buff_ + pos_, read_len);
        if (read_len == 0) {
            ret = false;
            break;
        }

        pos_ += read_len;

        if (!completed()) {
            ret = http_session->async_load();
            break;
        }

        int32_t rs = push_return_value();
        ret = resume(rs);
    } while (0);

    if (!ret) {
        give_up();
    }
    return ret;
}

int32_t LuaHttpReader::push_return_value() {
    if (co_ == NULL) {
        return 0;
    } else {
        lua_settop(co_, 0);
        lua_pushinteger(co_, (lua_Integer)pos_);
        return 1;
    }
}

bool LuaHttpReader::resume(int32_t count) {
    bool ret = true;
    do {
        if (co_ == NULL || main_ == NULL) {
            ret = false;
            break;
        }

        lua_resume(co_, main_, count);
    } while (0);
    return ret;
}

bool LuaHttpReader::give_up() {
    bool ret = true;
    do {
        if (co_ == NULL || main_ == NULL) {
            ret = false;
            break;
        }

        lua_resume(co_, main_, 0);
    } while (0);
    return ret;
}

int32_t LuaHttpReader::on_wait_result(lua_State *l, int32_t status, lua_KContext ctx) {
    int32_t nresult = lua_gettop(l);
    return nresult;
}

/////////////////////////////////////LuaHttpSomeReader/////////////////////////////////////
LuaHttpSomeReader::LuaHttpSomeReader(
    std::shared_ptr<LuaHttpSession> http_session,
    uint8_t *buff, 
    size_t size, 
    lua_State *main, 
    lua_State *co):
    LuaHttpReader(http_session, buff, size, main, co) {

}

LuaHttpSomeReader::~LuaHttpSomeReader() {

}

bool LuaHttpSomeReader::completed() {
    return pos_>0?true:false;
}

/////////////////////////////////////LuaHttpExactlyReader/////////////////////////////////////
LuaHttpExactlyReader::LuaHttpExactlyReader(
    std::shared_ptr<LuaHttpSession> http_session,
    uint8_t *buff, 
    size_t size, 
    lua_State *main, 
    lua_State *co):
    LuaHttpReader(http_session, buff, size, main, co) {

}

LuaHttpExactlyReader::~LuaHttpExactlyReader() {

}

bool LuaHttpExactlyReader::completed() {
    return pos_ == size_;
}

/////////////////////////////////////LuaHttpExactlyReader/////////////////////////////////////
LuaHttpScanfReader::LuaHttpScanfReader(
    std::shared_ptr<LuaHttpSession> http_session,
    uint8_t *buff, 
    size_t size, 
    lua_State *main, 
    lua_State *co):
    LuaHttpExactlyReader(http_session, buff, size, main, co),
    read_pos_(0) {

}

LuaHttpScanfReader::~LuaHttpScanfReader() {

}

int32_t LuaHttpScanfReader::push_return_value() {
    int32_t count = scanf();
    return count;
}

bool LuaHttpScanfReader::init(size_t size, const std::vector<DataField> &format) {
    bool ret = true;
    do {
        if (size == 0 || format.empty()) {
            ret = false;
            break;
        }

        if (buff_ != NULL) {
            delete [] buff_;
        }

        size_   = size;
        io_buffer_.reset(new IOBuffer(size_));
        buff_   = (uint8_t*)io_buffer_->data();
        format_ = format;
    } while (0);
    return ret;
}

int32_t LuaHttpScanfReader::scanf() {
    int32_t ret = 0;
    do {
        if (co_ == NULL || format_.empty()) {
            break;
        }

        lua_settop(co_, 0);

        std::vector<DataField>::const_iterator iter = format_.begin();
        for (;iter != format_.end();++iter,++ret) {
            switch (iter->type) {
            case eDataType_int8_t:
            case eDataType_uint8_t:
                {
                    uint8_t v = 0;
                    for (size_t i = 0;i < iter->length;++i) {
                        read_data((uint8_t*)&v, kDataTypeSize[iter->type]);
                        lua_pushinteger(co_, v);
                    }
                }
                break;
            case eDataType_int16_t:
            case eDataType_uint16_t:
                {
                    uint16_t v = 0;
                    for (size_t i = 0;i < iter->length;++i) {
                        read_data((uint8_t*)&v, kDataTypeSize[iter->type]);
                        if (iter->hton) {
                            v = ntohs(v);
                        }
                        lua_pushinteger(co_, v);
                    }
                }
                break;
            case eDataType_int32_t:
            case eDataType_uint32_t:
                {
                    uint32_t v = 0;
                    for (size_t i = 0;i < iter->length;++i) {
                        read_data((uint8_t*)&v, kDataTypeSize[iter->type]);
                        if (iter->hton) {
                            v = ntohl(v);
                        }
                        lua_pushinteger(co_, v);
                    }
                }
                break;
            case eDataType_int64_t:
            case eDataType_uint64_t:
                {
                    uint64_t v = 0;
                    for (size_t i = 0;i < iter->length;++i) {
                        read_data((uint8_t*)&v, kDataTypeSize[iter->type]);
                        if (iter->hton) {
                            v = _ntohll(v);
                        }
                        lua_pushinteger(co_, (uint32_t)v);
                    }
                }
                break;
            case eDataType_double:
                {
                    double v = 0;
                    for (size_t i = 0;i < iter->length;++i) {
                        read_data((uint8_t*)&v, kDataTypeSize[iter->type]);
                        lua_pushnumber(co_, v);
                    }
                }
                break;
            case eDataType_byte_array:
                {
                    IOBuffer data(iter->length);
                    read_data((uint8_t*)data.data(), data.size());
                    lua_pushlstring(co_, (char*)data.data(), data.size());
                }
                break;
            case eDataType_string:
                {
                    char *str = new char[iter->length + 1];
                    str[iter->length] = '\0';
                    read_data((uint8_t*)str, iter->length);
                    lua_pushstring(co_, str);
                    delete [] str;
                }
                break;
            default:
                break;
            }
        }
    } while (0);
    return ret;
}

uint32_t LuaHttpScanfReader::read_data(uint8_t *dst, uint32_t len) {
    uint32_t ret = 0;
    do {
        if (buff_ == NULL || size_ == 0 || dst == NULL || len == 0) {
            break;
        }

        if (read_pos_ >= size_) {
            break;
        }

        size_t left = size_ - read_pos_;
        ret = std::min<size_t>(len, left);
        memcpy(dst, buff_ + read_pos_, ret);
        read_pos_ += ret;
    } while (0);
    return ret;
}

} // namespace bee
