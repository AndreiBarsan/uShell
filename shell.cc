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

#include "builtin_registry.h"
#include "command.h"
#include "shell.h"
#include "util.h"

namespace microshell {
namespace core {

using namespace std;

Shell::Shell(const vector<string> &) :
    home_directory(util::get_current_home()),
    standard_output(cout),
    error_output(cerr),
    name("ush"),
    username(util::get_current_user()) {
  exit_requested = false;
  cout << "Welcome to microshell, " << username << "!" << endl;
  if(!util::getcwd(&this->working_directory)) {
    eout("Failed to get the current working directory.");
    eout("Defaulting to `~'.");
    this->working_directory = "~";
  }

  string envpath = getenv("PATH");
  this->path = util::split(envpath, ':');
}

int Shell::interactive() {
  while (!exit_requested) {
    output_prompt();
    string command_text = read_command();
    if(0 == command_text.length()) {
      cout << endl;
      continue;
    }

    Command *command = nullptr;
    string error;
    if(parse_command(command_text, &command, &error)) {
      interpret_command(*command);
    }
    else {
      eout(error);
    }

    delete command;
  }

  return 0;
} 
  
string Shell::resolve_path(const string& path) {
  if(util::is_absolute_path(path)) {
    return path;
  }

  // The VFS can handle the `..' sequences, but we want to handle them
  // ourselves in order to display the path in a clearer way.
  
  if(0 == path.find("~/")) {
    cout << "HOME path" << endl;
    string rest = path.substr(2);
    return util::merge_paths(home_directory, rest);
  }

  return util::merge_paths(working_directory, path);
}

string& Shell::get_working_directory() {
  return working_directory;
}
  
void Shell::set_working_directory(const string& directory) {
  util::setcwd(directory);
  working_directory = directory;
}

string Shell::get_working_directory() const {
  return working_directory;
}

const Shell* Shell::out(const string& message) const {
  this->standard_output << this->name << ": " << message << endl;
  return this;
}

const Shell* Shell::eout(const string& message) const {
  this->error_output << this->name << ": " << message << endl;
  return this;
}

void Shell::output_prompt() {
  this->standard_output << "(" << this->working_directory << ") " << this->prompt;
}

string Shell::read_command() {
  // We have already output our prompt at this point.
  char* line = readline("");
  
  // This happens if e.g. the user enters an EOF character (C-D).
  // Bash/zsh simply terminate in this case, even if they're interactive.
  // Should we do the same?
  if(!line) return "";

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
    *command = construct_builtin(argv);
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
  return BuiltinRegistry::instance()->is_registered(builtin_name);
}

BuiltinCommand* Shell::construct_builtin(const vector<string>& argv) {
  return BuiltinRegistry::instance()->build(argv);
}

}  // namespace core
}  // namespace microshell
