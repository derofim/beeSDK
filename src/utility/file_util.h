#ifndef __FILE_UTIL_H__
#define __FILE_UTIL_H__

#include <string>
#include <vector>

namespace bee {

class FileUtil {
public:
	static bool create_long_directory(const std::string& strDirectory);
	static bool is_file_writable(const std::string& filename);
	static bool is_file_exist(const std::string& filename);
	static bool delete_file(const std::string& pathname);	
	static bool list_dir(const std::string& dir, std::vector<std::string>& filelist);
	static bool rename_file(const std::string& ofile, const std::string& nfile);
    static size_t get_file_size(const std::string& filename);
    static std::string path_cat(const std::string& dir, const std::string& filename);
    static std::string get_cwd();
    static std::string str_trim(const std::string& str);
};

} // namespace bee

#endif // #ifndef __FILE_UTIL_H__
