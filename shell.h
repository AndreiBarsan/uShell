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
  Shell();

  int interactive();

protected:
  void output_prompt(ostream& output);

  string read_command(istream& input);

  int interpret_command(Command &cmd);

  bool parse_command(const string& command_text,
                                    Command **command,
                                    string *error);

  bool resolve_binary_name(const string& name, string* full_path);

  bool is_builtin(const string& builtin_name);

  BuiltinCommand* get_builtin(const vector<string>& argv);

private:
  bool exit_requested; 
  string prompt = "ush >> ";
  map<string, BuiltinCommand*> builtin_table = {
//    { "exit", new ExitBuiltin },
//    { "pwd", new PwdBuiltin }
  };

  // The list of folders found inside the PATH environment variable.
  vector<string> path;
  // The current working directory of the shell.
  string working_directory;
};

}  // namespace core
}  // namespace microshell

#endif  // SHELL_H
