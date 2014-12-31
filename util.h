#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>

// TODO(andrei) Separate platform-independent from platform-dependent code.
namespace util {
  // Convenience method for converting an ugly (argc, argv) pair into
  // a typesafe vector<string>.
  std::vector<std::string> argv_to_strvec(int count, char** argv);

  bool is_absolute_path(const std::string& path);

  std::string merge_paths(const std::string& path);

  std::string merge_paths(const std::string& path1, const std::string& path2);

  std::vector<std::string> split(const std::string &s, char delim);

  std::string merge_with(const std::vector<std::string>::iterator& start,
                         const std::vector<std::string>::iterator& end,
                         const std::string delimitator);

  // ``Unwraps'' the vector `v', exposing a plain `char**' containing
  // the raw data from `v'.  Null terminates the result as well,
  // since many functions such as `execve' expect null-terminated
  // arrays.  The caller takes ownership of the array.
  char** get_raw_array(const std::vector<std::string>& v);

  // Wraps around the corresponding syscalls and returns the program's
  // current working directoty.
  bool getcwd(std::string *cwd);
  bool setcwd(const std::string& cwd);

  bool is_file(const std::string& name);
  bool is_regular_file(const std::string& name);
  bool is_directory(const std::string& name);

  std::string get_current_home();
  std::string get_current_user();

}  // namespace util

#endif  // UTIL_H
