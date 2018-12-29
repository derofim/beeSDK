#ifndef __ALGORITHM_H__
#define __ALGORITHM_H__

#include "common.h"

#include <string>
#include <memory>
#ifdef WIN32
#pragma comment(lib, "ole32.lib")
#else
#include <sys/time.h>
#include "utility/socket_api.h"
#endif 

inline std::string w2b(const std::wstring& _src) {
#ifdef WIN32
	int nBufSize = ::WideCharToMultiByte(GetACP(), 0, _src.c_str(),-1, NULL, 0, 0, FALSE);

	char *szBuf = new char[nBufSize + 1];

	::WideCharToMultiByte(GetACP(), 0, _src.c_str(),-1, szBuf, nBufSize, 0, FALSE);

	std::string strRet(szBuf);

	delete []szBuf;
	szBuf = NULL;

	return strRet;
#else
  return _src;
#endif
}

inline std::wstring b2w(const std::string& _src) {
#ifdef WIN32
	//计算字符串 string 转成 wchar_t 之后占用的内存字节数
	int nBufSize = ::MultiByteToWideChar(GetACP(),0,_src.c_str(),-1,NULL,0); 

	//为 wsbuf 分配内存 BufSize 个字节
	wchar_t *wsBuf = new wchar_t[nBufSize + 1];

	//转化为 unicode 的 WideString
	::MultiByteToWideChar(GetACP(),0,_src.c_str(),-1,wsBuf,nBufSize); 

	std::wstring wstrRet(wsBuf);

	delete []wsBuf;
	wsBuf = NULL;

	return wstrRet;
#else
  return _src;
#endif
}

inline std::string Wide2Utf8(const std::wstring& _src) {
#ifdef WIN32
	int nBufSize = ::WideCharToMultiByte(CP_UTF8, 0, _src.c_str(),-1, NULL, 0, NULL, NULL);

	char *szBuf = new char[nBufSize + 1];

	::WideCharToMultiByte(CP_UTF8, 0, _src.c_str(),-1, szBuf, nBufSize, NULL, NULL);

	std::string strRet(szBuf);

	delete []szBuf;
	szBuf = NULL;

	return strRet;
#else
  return _src;
#endif
}

inline std::wstring Utf82Wide(const std::string& _src) {
#ifdef WIN32
	//计算字符串 string 转成 wchar_t 之后占用的内存字节数
	int nBufSize = ::MultiByteToWideChar(CP_UTF8,0,_src.c_str(),-1,NULL,0); 

	//为 wsbuf 分配内存 BufSize 个字节
	wchar_t *wsBuf = new wchar_t[nBufSize + 1];

	//转化为 unicode 的 WideString
	::MultiByteToWideChar(CP_UTF8,0,_src.c_str(),-1,wsBuf,nBufSize); 

	std::wstring wstrRet(wsBuf);

	delete []wsBuf;
	wsBuf = NULL;

	return wstrRet;
#else
  return _src;
#endif
}

inline void Split(std::vector<std::string>& result, const std::string& input, const std::string& spliter) {
    std::string::size_type spliter_length = spliter.size();
    std::string::size_type last_pos = 0;

    while(last_pos < input.length())
    {
        std::string::size_type pos = input.find(spliter, last_pos);
        if( pos == std::string::npos )
        {
            result.push_back(input.substr(last_pos));
            return;
        }
        result.push_back(input.substr(last_pos, pos - last_pos));
        last_pos = pos + spliter_length;
    }

}

inline bool is_digit2(std::string &instr) { 
    for(size_t i=0;i<instr.size();i++) {   
        if ((instr.at(i)>'9') || (instr.at(i)<'0')) { 
            return false;		 
        }
    } 
    return true; 
}

inline void url_path_remove(std::string& url, const std::string& param) {
    std::string::size_type pos_start = url.find(param);

    if (pos_start != std::string::npos && pos_start != 0 && pos_start != url.size()-1) {
        std::string::size_type pos_end = url.find("&", pos_start+param.size());
        if (pos_end == std::string::npos) {
            url.resize(pos_start);
        } else {
            std::string url_left = url.substr(0, pos_start);
            url_left.append(url.substr(pos_end+1));
            url = url_left;
        }
    }
}

inline uint32_t IpToUint(const char* ip) {
#ifdef WIN32
    return (uint32_t)inet_addr(ip);
#else
    struct in_addr addr;
    if ( 0 != inet_aton(ip, &addr) ) {
        return addr.s_addr;
    }
    return 0;
#endif
}

inline uint32_t ip2uint(const std::string &ip_s) {
	uint32_t num[4];
	int i;
	size_t pos_start = 0,pos;

	for(i=0;i<4;i++) {
		pos = ip_s.find('.',pos_start);
		std::string one;
		if(pos == std::string::npos) {
			one = ip_s.substr(pos_start);
		} else {
			one = ip_s.substr(pos_start,pos-pos_start);
			pos_start = pos + 1;
		}
		num[i] = atoi(one.c_str());
	}
	if(i==4) {
		uint32_t ip = 0;
		ip += num[0]<<24;
		ip += num[1]<<16;
		ip += num[2]<<8;
		ip += num[3];
		return ip;
	} else {
		return 0;
	}
}

//输入本地字节序
inline std::string uint2ip(uint32_t ip_uint) {
#ifdef WIN32
    struct in_addr  addr;
    addr.S_un.S_addr = ip_uint;
    return std::string(::inet_ntoa(addr));
#else
    char ip_buff[32];
    ::inet_ntop(AF_INET, &ip_uint, ip_buff, 32);
    return std::string(ip_buff);
#endif
}

template<typename T>
std::weak_ptr<T> weaked_from_this(T* ptr) {
    return ptr->shared_from_this();
}

inline int32_t rand_r(uint32_t *seedp) {
    *seedp = (*seedp) * 1103515245 + 12345;
    return((int32_t)((*seedp)/65536) % 32768);
}

inline uint64_t htonll(uint64_t host_long_long) {
    uint64_t r = htonl(host_long_long >> 32);
    r = r << 32;
    r |= htonl(host_long_long & 0xFFFFFFFF);
    return r;
}

inline uint64_t ntohll(uint64_t net_long_long) {
    uint64_t r = ntohl(net_long_long >> 32);
    r = r << 32;
    r |= ntohl(net_long_long & 0xFFFFFFFF);
    return r;
}

#endif
