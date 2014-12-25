#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>

namespace util {
  std::string merge_paths(const std::string& path);

  std::string merge_paths(const std::string& path1, const std::string& path2);

  std::vector<std::string> split(const std::string &s, char delim);

  std::string merge_with(const std::vector<std::string>::iterator& start,
                         const std::vector<std::string>::iterator& end,
                         const std::string delimitator);

  /* ``Unwraps'' the vector `v', exposing a plain `char**' containing
   * the raw data from `v'.  Null terminates the result as well,
   * since many functions such as `execve' expect null-terminated
   * arrays.  The caller takes ownership of the array. */
  char** get_raw_array(const std::vector<std::string>& v);

}  // namespace util

#endif  // UTIL_H