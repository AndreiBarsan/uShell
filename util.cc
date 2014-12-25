#include <sstream>
#include <string>
#include <vector>

#include <cstring>

#include "util.h"

namespace util {

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
      ret[i] = strdup(v[i].c_str());
    }
    ret[v.size()] = nullptr;
    return ret;
  }

}  // namespace util
