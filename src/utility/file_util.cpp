#include "utility/file_util.h"
#include "utility/common.h"

#include <fcntl.h>
#ifdef WIN32
#include <io.h>
#include <direct.h>
#include <tchar.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif

namespace bee {

bool FileUtil::create_long_directory(const std::string& strDirectory) {
    int32_t index1 = -1, index2 = -1;
    std::string sNew;
    std::string slashset="/\\";
    std::string path = strDirectory;
#ifdef WIN32
    if (path[path.size() - 1] != '\\' && path[path.size() - 1] != '/') {
        path.append("\\");
    }
#else
    if (path[path.size() - 1] != '/') {
        path.append("/");
    }
#endif
    while (1) {
        index2 = index1 + 1;
        index1 = path.find_first_of(slashset,index2);
        if (index1 == (int32_t)std::string::npos) {
            break;
        }
        if (index1 == 0) {
            continue;
        }
        if (path[index1 - 1] == ':') {
            continue;
        }
        sNew = path.substr(0, index1);
#ifdef WIN32
        _mkdir(sNew.c_str());
#else
        mkdir(sNew.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    }
    return true;
}

bool FileUtil::is_file_writable(const std::string& filename) {
    bool res = true;
#ifdef WIN32
    if ( _access(filename.c_str(), 2) != 0) {
        printf("access errno %d\n", WSAGetLastError());
#else
    if (access(filename.c_str(), W_OK) != 0) {
        printf("access errno %d\n", errno);
#endif
        res = false;
    }
    
    return res;
}

bool FileUtil::is_file_exist(const std::string& filename) {
    bool res = false;
#ifdef WIN32
    res = (_access(filename.c_str(), 0) == 0);
#else
    res = (access(filename.c_str(), F_OK) == 0);
#endif
    return res;
}

bool FileUtil::delete_file(const std::string& pathname) {
    bool res = true;
    if ((res = is_file_writable(pathname)) != true) {
        return res;
    }

#ifdef WIN32
    if (_unlink(pathname.c_str()) != 0) {
        printf("unlink errno %d\n", WSAGetLastError());
#else
    if (unlink(pathname.c_str()) != 0) {
        printf("unlink errno %d\n", errno);
#endif
        res = false;
    }

    return res;
}

std::string FileUtil::path_cat(const std::string& dir, const std::string& filename) {
    std::string res(dir);
#ifdef WIN32
    if (res[res.size() - 1] != '/' && res[res.size() - 1] != '\\') {
        res += '\\';
    }
#else
    if (res[res.size() - 1] != '/') {
        res += '/';
    }
#endif

    res.append(filename);
    return res;
}

bool FileUtil::list_dir(const std::string& dir, std::vector<std::string>& filelist) {
    bool res = true;
    filelist.clear();

#ifdef WIN32
    WIN32_FIND_DATAA data;
    HANDLE handle = FindFirstFileA((dir + "\\*.*").c_str(), &data);
    if (handle != INVALID_HANDLE_VALUE) {
        do {
            // '..', '.' should not be included
            if ((strcmp(data.cFileName, "..") != 0) && (strcmp(data.cFileName, ".") != 0)) {
                filelist.push_back(data.cFileName);
            }
        } while (FindNextFileA(handle, &data));

        if (GetLastError() != ERROR_NO_MORE_FILES) {
            res = false;
        }
        FindClose(handle);
    } else {
        res = false;
    }
#else
    DIR *dp;
    struct dirent *dirp;
    if ((dp  = opendir(dir.c_str())) == NULL) {
        res = false;
    } else {
        while ((dirp = readdir(dp)) != NULL) {
            // '..', '.' should not be included
            if ((strcmp(dirp->d_name, "..") != 0) && (strcmp(dirp->d_name, ".") != 0)) {
                filelist.push_back(std::string(dirp->d_name));
            }
        }
        closedir(dp);
    }
#endif

    return res;
}

bool FileUtil::rename_file(const std::string& ofile, const std::string& nfile) {
    bool res = true;
    if ((res = is_file_writable(ofile)) != true) {
        return res;
    }

    if (rename(ofile.c_str(), nfile.c_str()) != 0) {
        res = false;
    }

    return res;
}

std::string FileUtil::get_cwd() {
    char buf[256] = { 0 };
#ifdef WIN32
    _getcwd( buf, sizeof(buf) );
#else
    getcwd( buf, sizeof(buf) );
#endif

    return buf;
}

size_t FileUtil::get_file_size(const std::string& filename) {
    int32_t fd = -1;
    int32_t result = 0;
#ifdef WIN32
    struct __stat64 buf;
    if ((fd = _open(filename.c_str(), _O_RDONLY)) < 0) {
        return 0;
    }
    if ((result = _fstat64(fd, &buf)) != 0) {
        _close(fd);
        return 0;
    }

    _close(fd);
#else
    struct stat buf;
    if ((fd = open(filename.c_str(), O_RDONLY)) < 0) {
        return 0;
    }
    if ((result = fstat(fd, &buf)) != 0) {
        close(fd);
        return 0;
    }

    close(fd);
#endif
    
    return (size_t)buf.st_size;
}

std::string FileUtil::str_trim(const std::string& str) {
    if (str.empty()) {
        return str;
    }

    std::string s;
    s.assign(str);
    std::string::iterator iter;
    for (iter = s.begin(); iter != s.end();) {
        if ((*iter == ' ') || (*iter == '\t')) {
            iter = s.erase(iter);
        } else {
            break;
        }
    }
    for (iter = s.end() - 1; iter != s.begin(); --iter) {
        if ((*iter != ' ') && (*iter != '\t')) {
            break;
        }
    }
    ++iter;
    for (; iter != s.end();) {
        if (*iter == ' ' || *iter == '\t') {
            iter = s.erase(iter);
        } else {
            break;
        }
    }

    return s;
}

} //namespace bee
