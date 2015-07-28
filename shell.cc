// C++ includes
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

string Shell::expand(const string& param) const {
  // TODO(andrei) Variable expansions happen first.

  if(0 == param.find("~")) {
    string rest = param.substr(1);
    return util::merge_paths(home_directory, rest);
  }

  return param;
}

string Shell::resolve_path(const string& path) const {
  if(util::is_absolute_path(path)) {
    return path;
  }

  // The VFS can handle the `..' sequences, but we want to handle them
  // ourselves in order to display the path in a clearer way.

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

string Shell::get_prompt() const {
  return "(" + this->working_directory + ") " + this->prompt;
}

string Shell::read_command() {
  char* line = readline(get_prompt().c_str());

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
                                  string *error) const {
  // TODO(andrei) YACC this.
  // TODO(andrei) Do we really want a customizable IFS variable?
  vector<string> argv = util::split(command_text, ' ');

  // Perform expansion for every parameter.
  for(string& arg : argv) {
    arg = expand(arg);
  }

  const string& program_name = argv[0];

  if(is_builtin(argv[0])) {
    *command = construct_builtin(argv);
  }
  else {
    // If the path actually points to a directory, we want to catch that.
    string full_path = resolve_path(program_name);
    if (util::is_directory(full_path)) {
      // TODO(andrei) zsh-like auto-cd functionality could go here.
      *error = "Cannot execute [" + argv[0] + "]. It's a directory.";
      return false;
    }

    string binary_path;
    if(resolve_binary_name(argv[0], &binary_path)) {
      argv[0] = binary_path;
      *command = new DiskCommand(argv);
    } else {
      *error = "Command not found: [" + argv[0] + "]";
      return false;
    }
  }

  return true;
}

bool Shell::resolve_binary_name(const string& name, string* full_path) const {
  // No lookup needed for absolute paths.
  if(util::is_absolute_path(name)) {
    *full_path = name;
    return true;
  }

  // Search in our current directory.
  {
    string path = util::merge_paths(working_directory, name);
    if(util::is_file(path)) {
      *full_path = path;
      return true;
    }
  }

  // Search the PATH.
  for(const string& p : this->path) {
    string path = util::merge_paths(p, name);
    if(util::is_file(path)) {
      *full_path = path;
      return true;
    }
  }

  return false;
}

bool Shell::is_builtin(const string& builtin_name) const {
  return BuiltinRegistry::instance()->is_registered(builtin_name);
}

BuiltinCommand* Shell::construct_builtin(const vector<string>& argv) const {
  return BuiltinRegistry::instance()->build(argv);
}

}  // namespace core
}  // namespace microshell
