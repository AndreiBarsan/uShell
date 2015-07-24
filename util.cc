#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <cstring>

#include <limits.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "util.h"

namespace util {

  using namespace std;

  vector<string> argv_to_strvec(int count, char** const argv) {
    vector<string> vargv;
    for(int i = 0; i < count; ++i) {
      vargv.emplace_back(argv[i]);
    }
    return vargv;
  }

  bool is_absolute_path(const std::string& path) {
    return path.length() > 0 && path[0] == '/';
  }

  // TODO(andrei) Proper varadic function.
  std::string merge_paths(const std::string& path) {
    return path;
  }

  std::string merge_paths(const std::string& path1, const std::string& path2) {
    return path1 + "/" + path2;
  }

  std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
      elems.push_back(item);
    }
    return elems;
  }

  std::string merge_with(const std::vector<std::string>::iterator& start,
                         const std::vector<std::string>::iterator& end,
                         const std::string delimitator) {
    std::stringstream ss;
    for(auto it = start; it != end; ++it) {
      ss << *it;
      if(it + 1 != end) {
        ss << delimitator;
      }
    }

    return ss.str();
  }

  char** get_raw_array(const std::vector<std::string>& v) {
    char** ret = new char*[v.size() + 1];
    for(size_t i = 0; i < v.size(); ++i) {
      ret[i] = ::strdup(v[i].c_str());
    }
    ret[v.size()] = nullptr;
    return ret;
  }

  bool getcwd(std::string *cwd) {
    // Note: PATH_MAX might not be sufficient.
    // See: insanecoding.blogspot.com/2007/11/pathmax-simply-isnt.html
    static char dir[PATH_MAX];
    if(::getcwd(dir, sizeof(dir))) {
      *cwd = std::string(dir);
      return true;
    }
    
    return false;
  }

  bool setcwd(const std::string& cwd) {
    return 0 == ::chdir(cwd.c_str());
  }

  bool is_file(const string& name) {
    struct stat stat_buf;
    return 0 == ::stat(name.c_str(), &stat_buf);
  }

  bool is_regular_file(const std::string& name) {
    struct stat stat_buf;
    if(0 == ::stat(name.c_str(), &stat_buf)) {
      return S_ISREG(stat_buf.st_mode);
    }

    return false;
  }

  bool is_directory(const std::string& name) {
    struct stat stat_buf;
    if(0 == ::stat(name.c_str(), &stat_buf)) {
      return S_ISDIR(stat_buf.st_mode);
    }

    return false;
  }

  string get_current_home() {
    char* home;
    if(nullptr == (home = ::getenv("HOME"))) {
      home = ::getpwuid(::getuid())->pw_dir;
      return string(home);
    }
    else {
      string ret(home);
      return ret;
    }
  }

  string get_current_user() {
    return string(::getpwuid(::getuid())->pw_name);
  }

}  // namespace util

