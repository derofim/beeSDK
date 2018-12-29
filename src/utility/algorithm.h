#ifndef __ALGORITHM_H__
#define __ALGORITHM_H__

#include "utility/common.h"

#include <string>
#include <memory>
#ifdef WIN32
#pragma comment(lib, "ole32.lib")
#pragma comment(lib,"Iphlpapi.lib")
#else
#include <sys/time.h>
#endif 

namespace bee {

inline void Split(std::vector<std::string>& result, const std::string& input, const std::string& spliter) {
    std::string::size_type spliter_length = spliter.size();
    std::string::size_type last_pos = 0;

    while (last_pos < input.length())
    {
        std::string::size_type pos = input.find(spliter, last_pos);
        if ( pos == std::string::npos )
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
		if (pos == std::string::npos) {
			one = ip_s.substr(pos_start);
		} else {
			one = ip_s.substr(pos_start,pos-pos_start);
			pos_start = pos + 1;
		}
		num[i] = atoi(one.c_str());
	}
	if (i==4) {
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

template<typename T>
std::weak_ptr<T> weaked_from_this(T* ptr) {
    return ptr->shared_from_this();
}

inline int32_t rand_r(uint32_t *seedp) {
    *seedp = (*seedp) * 1103515245 + 12345;
    return((int32_t)((*seedp)/65536) % 32768);
}

inline uint64_t _htonll(uint64_t host_long_long) {
    uint64_t r = htonl(host_long_long >> 32);
    r = r << 32;
    r |= htonl(host_long_long & 0xFFFFFFFF);
    return r;
}

inline uint64_t _ntohll(uint64_t net_long_long) {
    uint64_t r = ntohl(net_long_long >> 32);
    r = r << 32;
    r |= ntohl(net_long_long & 0xFFFFFFFF);
    return r;
}

inline std::string bee_itoa(int32_t number) {
    std::ostringstream s;
    s << number;
    return s.str();
}

} // namespace bee

#endif
