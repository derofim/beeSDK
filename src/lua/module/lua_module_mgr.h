#ifndef __LUA_MODULE_MGR_H__
#define __LUA_MODULE_MGR_H__

#include "utility/common.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace bee {

const size_t kModuleNameMaxLen = 64;
const size_t kModuleVersionMaxLen = 64;

////////////////////////////////////LuaObj//////////////////////////////////////
template <class T>
struct LuaObj {
    std::shared_ptr<T> obj;
    uint32_t handle;
    LuaObj() {
        handle = 0;
    }

    ~LuaObj() {
    }
};

////////////////////////////////////LuaObjMgr//////////////////////////////////////
template <class T>
class LuaObjMgr {
public:
    typedef std::unordered_map<uint32_t, std::shared_ptr<LuaObj<T> > > LuaObjTable;
    LuaObjMgr(){}
    virtual ~LuaObjMgr(){}

public:
    uint32_t new_lua_obj(std::shared_ptr<T> obj);
    std::shared_ptr<T> delete_lua_obj(uint32_t handle);
    std::shared_ptr<T> get_obj(uint32_t handle);
    void set_lua_obj(uint32_t handle, std::shared_ptr<T> obj);

protected:
    static uint32_t counter_;
    LuaObjTable obj_table_;
};

template <class T>
uint32_t LuaObjMgr<T>::counter_ = 0;

template <class T>
uint32_t LuaObjMgr<T>::new_lua_obj(std::shared_ptr<T> obj) {
    if (obj == NULL) {
        return 0;
    }

    std::shared_ptr<LuaObj<T> > lua_obj(new LuaObj<T>);

    lua_obj->obj = obj;
    lua_obj->handle = ++counter_;
    obj_table_[lua_obj->handle] = lua_obj;

    return lua_obj->handle;
}

template <class T>
std::shared_ptr<T> LuaObjMgr<T>::delete_lua_obj(uint32_t handle) {
    std::shared_ptr<T> obj;
    do {
        auto iter = obj_table_.find(handle);
        if (iter == obj_table_.end()) {
            break;
        }

        std::shared_ptr<LuaObj<T> > lua_obj = iter->second;
        if (lua_obj == NULL) {
            break;
        }

        obj = lua_obj->obj;
        obj_table_.erase(iter);
    } while (0);
    return obj;
}

template <class T>
std::shared_ptr<T> LuaObjMgr<T>::get_obj(uint32_t handle) {
    std::shared_ptr<T> obj;
    do {
        auto iter = obj_table_.find(handle);
        if (iter == obj_table_.end()) {
            break;
        }

        std::shared_ptr<LuaObj<T> > lua_obj = iter->second;
        if (lua_obj == NULL) {
            break;
        }

        obj = lua_obj->obj;
    } while (0);
    return obj;
}

template <class T>
void LuaObjMgr<T>::set_lua_obj(uint32_t handle, std::shared_ptr<T> obj) {
    do {
        auto iter = obj_table_.find(handle);
        if (iter == obj_table_.end()) {
            break;
        }

        std::shared_ptr<LuaObj<T> > lua_obj = iter->second;
        if (lua_obj == NULL) {
            break;
        }

        lua_obj->obj = obj;
    } while (0);
}

///////////////////////////////////ModuleWrapper///////////////////////////////////////
template <class T>
struct ModuleWrapper {
    T *module;

    ModuleWrapper() {
        module = NULL;
    }
};

////////////////////////////////////LuaModuleBase//////////////////////////////////////
template <class T>
class LuaModuleBase {
public:
    LuaModuleBase():ver_("1.0.1"),main_thread_(NULL){}
    virtual ~LuaModuleBase(){}

    static int32_t open_module(lua_State *l, luaL_Reg *fun, const std::string &name, const std::string &version, ModuleWrapper<T> **p = NULL);
    static int32_t close_module(lua_State *l);
    static ModuleWrapper<T> *get_module_wrapper(lua_State *l);
    static bool check_args(lua_State *l, int32_t n);
    virtual void on_close_module() {}
    lua_State *get_main_thread() {return main_thread_;}
    std::string ver_;
    lua_State *main_thread_;    
};

template <class T>
int32_t LuaModuleBase<T>::open_module(lua_State *l, luaL_Reg *fun, const std::string &name, const std::string &version, ModuleWrapper<T> **p) {
    if (l == NULL || fun == NULL || name.empty() || version.empty()) {
        return -1;
    }

    /* Module table */
    lua_newtable(l);

    /* Create userdata for wrapper */
    ModuleWrapper<T> *ex = (ModuleWrapper<T> *)lua_newuserdata(l, sizeof(ModuleWrapper<T>)); 
    if (p != NULL) {
        *p = ex;
    }

    /* Allocate module object */
    ex->module = new T();

    /* Create GC method to clean up */
    lua_newtable(l);
    lua_pushcfunction(l, close_module);
    lua_setfield(l, -2, "__gc");
    lua_setmetatable(l, -2);

    luaL_setfuncs(l, fun, 1);

    /* Set module name / version fields */
    lua_pushlstring(l, name.c_str(), name.size());
    lua_setfield(l, -2, "_NAME");
    lua_pushlstring(l, version.c_str(), version.size());
    lua_setfield(l, -2, "_VERSION");

    /* Certainly in main thread */
    ex->module->main_thread_ = l;
    return 1;
}

template <class T>
int32_t LuaModuleBase<T>::close_module(lua_State *l) {
    ModuleWrapper<T> *wrapper = (ModuleWrapper<T> *)lua_touserdata(l, 1);
    if (wrapper != NULL) {
        if (wrapper->module != NULL) {
            wrapper->module->on_close_module();
            delete wrapper->module;
            wrapper->module = NULL;
        }
    }
    return 0;
}

template <class T>
ModuleWrapper<T> *LuaModuleBase<T>::get_module_wrapper(lua_State *l) {
    ModuleWrapper<T> *wrapper = (ModuleWrapper<T>*)lua_touserdata(l, lua_upvalueindex(1));
    return wrapper;
}

template <class T>
bool LuaModuleBase<T>::check_args(lua_State *l, int32_t n) {
    bool ret = true;
    do {
        if (l == NULL) {
            ret = false;
            break;
        }

        int32_t args_len = lua_gettop(l);
        if (n != args_len) {
            ret = false;
            break;
        }
    } while (0);
    return ret;
}

} // namespace bee

#endif
