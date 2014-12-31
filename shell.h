#ifndef SHELL_H
#define SHELL_H

#include <iostream>
#include <map>
#include <string>

#include "command.h"
#include "shell.h"
#include "util.h"

namespace microshell {
namespace core {

using namespace std;

class Shell {
public:
  Shell(const vector<string>& args);

  int interactive();

  // Given `path', resolve it based on the current working directory.
  // Supports tilde expansion, 
  string resolve_path(const string& path);

  string& get_working_directory();
  string get_working_directory() const;
  void set_working_directory(const string& directory);

  const Shell* out(const string& message) const;
  const Shell* eout(const string& message) const;

protected:
  void output_prompt();

  string read_command();

  int interpret_command(Command &cmd);

  bool parse_command(const string& command_text,
                                    Command **command,
                                    string *error);

  bool resolve_binary_name(const string& name, string* full_path);

  bool is_builtin(const string& builtin_name);

  BuiltinCommand* construct_builtin(const vector<string>& argv);
  
private:
  bool exit_requested; 
  string prompt = "ush >> ";

  // The list of folders found inside the PATH environment variable.
  vector<string> path;

  // The current working directory of the shell.
  string working_directory;
  // The home directory of the active user.
  string home_directory;

  // The streams used by the shell instance to output all text.
  ostream& standard_output;
  ostream& error_output;

  // The shell's name.
  string name;
  // The current user's name.
  string username;
};

}  // namespace core
}  // namespace microshell

#endif  // SHELL_H
