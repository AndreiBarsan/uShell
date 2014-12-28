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
  string& get_working_directory();
  string get_working_directory() const;

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

  // The streams used by the shell instance to output all text.
  ostream& standard_output;
  ostream& error_output;

  string name;
};

}  // namespace core
}  // namespace microshell

#endif  // SHELL_H
