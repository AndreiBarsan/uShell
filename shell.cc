// C++
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

// C includes
#include <cstdlib>
#include <cstdio>

// Linux-specific
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h> // maybe remove
#include <sys/wait.h> // maybe remove

#include "command.h"
#include "shell.h"
#include "util.h"

namespace microshell {
namespace core {

using namespace std;

Shell::Shell() {
  exit_requested = false;
  cout << "Welcome to microshell!" << endl;
  if(!util::getcwd(&this->working_directory)) {
    cerr << "Failed to get the current working directory." << endl;
    cerr << "Defaulting to `~'." << endl;
    this->working_directory = "~";
  }
}

int Shell::interactive() {
  auto input = &cin;
  auto output = &cout;

  string envpath = getenv("PATH");
  this->path = util::split(envpath, ':');

  while (!exit_requested) {
    output_prompt(*output);
    string command_text = read_command(*input);
    if(0 == command_text.length()) {
      continue;
    }

    Command *command = nullptr;
    string error;
    if(parse_command(command_text, &command, &error)) {
      interpret_command(*command);
    }
    else {
      cout << error << endl;
    }

    delete command;
  }

  return 0;
} 

void Shell::output_prompt(ostream& output) {
  output << prompt;
}

string Shell::read_command(istream& input) {
  // We have already output our prompt at this point.
  char* line = readline("");
  string command(line);
  if(command.size() > 0) {
    add_history(line);
  }
  free(line);
  return command;
}

int Shell::interpret_command(Command &cmd) {
  return cmd.invoke(this);
}

bool Shell::parse_command(const string& command_text,
                                  Command **command,
                                  string *error) {
  // TODO(andrei) YACC this.
  vector<string> argv = util::split(command_text, ' ');

  // TODO(andrei) Tilde expand here.
  // TODO(andrei) Dollar(-brace) expand here.

  if(is_builtin(argv[0])) {
    *command = get_builtin(argv);
  }
  else {
    string full_path;
    if(resolve_binary_name(argv[0], &full_path)) {
      argv[0] = full_path;
      *command = new DiskCommand(argv);
    } else {
      *error = "Command not found: [" + argv[0] + "]";
      return false;
    }
  }

  *error = "";
  return true;
}

bool Shell::resolve_binary_name(const string& name, string* full_path) {
  // PLATFORM_SPECIFIC
  struct stat stat_buf;
  // No lookup needed for absolute paths.
  if(util::is_absolute_path(name)) {
    *full_path = name;
    return true;
  }

  // Search in our current directory.
  {
    string path = util::merge_paths(working_directory, name);
    if(0 == stat(path.c_str(), &stat_buf)) {
      *full_path = path;
      return true;
    }
  }
  
  // Search the PATH.
  for(const string& p : path) {
    string path = util::merge_paths(p, name);
    if(0 == stat(path.c_str(), &stat_buf)) {
      *full_path = path;
      return true;
    }
  }

  return false;
}

bool Shell::is_builtin(const string& builtin_name) {
  return builtin_table.end() != builtin_table.find(builtin_name);
}

BuiltinCommand* Shell::get_builtin(const vector<string>& argv) {
  return builtin_table[argv[0]];
}

}  // namespace core
}  // namespace microshell
