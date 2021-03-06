#ifndef SHELL_H
#define SHELL_H

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "command.h"
#include "shell.h"
#include "shell_module.h"
#include "util.h"

namespace microshell {
namespace core {

using namespace std;

class Shell {
public:
  static Shell* initialize(const vector<string>& args) {
    Shell::instance = new Shell(args);
    return Shell::instance;
  }

  static Shell* get() {
    if (nullptr == Shell::instance) {
      cerr << "Tried to access shell before initialization." << endl;
      ::exit(-1);
    }

    return Shell::instance;
  }

  // Run in REPL mode--display a prompt, read a command, run it, and repeat
  // until the shell terminates.
  int interactive();

  // Perform various expansions (tilde, variable (dollar), dollar-brace etc.).
  // Currently performs just tilde expansion.
  string expand(const string& param) const;

  // Given `path', resolve it based on the current working directory.
  string resolve_path(const string& path) const;

  string& get_working_directory();
  string get_working_directory() const;
  void set_working_directory(const string& directory);

  // TODO(andrei) Might be cleaner to replace these with ostream& directly.
  // TODO(andrei) Clear separation between logging and output.

  // Output functions
  const Shell* out(const string& message);
  const Shell* eout(const string& message);

  // Logging functions
  const Shell* info(const string& message);
  const Shell* warning(const string& message);
  const Shell* error(const string& message);
  void fatal(const string& message);

  // Clean up and signal that we want to terminate the shell program.  Doesn't
  // exit immediately, in order to allow the shell to clean up after itself.
  void exit();

  // Load the specified module and register it (e.g. its hooks or provided
  // commands) in the shell.
  //
  // Returns 0 on success and an error code on failure.
  template<class MODULE_TYPE>
  int load_module(shared_ptr<MODULE_TYPE> module);

  bool get_waiting_for_child() const;

  // Wait for the given child process to complete, and return its exit code.
  int wait_child(int child_pid);

protected:
  Shell(const vector<string>& args);

  string get_prompt() const;

  string read_command();

  int interpret_command(Command &cmd);

  bool parse_command(const string& command_text,
                     shared_ptr<Command> &command,
                     string *error) const;

  bool resolve_binary_name(const string& name, string* full_path) const;

  bool is_builtin(const string& builtin_name) const;

  shared_ptr<BuiltinCommand> construct_builtin(const vector<string>& argv) const;

private:
  static Shell *instance;

  bool exit_requested;
  std::string prompt = "ush >> ";

  // The list of folders found inside the PATH environment variable.
  std::vector<std::string> path;

  // The current working directory of the shell.
  std::string working_directory;
  // The home directory of the active user.
  std::string home_directory;

  // The streams used by the shell instance to output all text.
  std::ostream& standard_output;
  std::ostream& error_output;

  // The shell's name.
  std::string name;
  // The current user's name.
  std::string username;

  std::vector<std::shared_ptr<ShellModule>> loaded_modules;

  // This method initializes the shell's core modules (e.g. job control).
  //
  // Returns 0 on success and a nonzero error code on failure.
  int load_default_modules();

  // TODO(andrei) Proper state management using e.g. an enum.
  // Whether the shell is currently running a child process in the foreground.
  bool waiting_for_child;
};

}  // namespace core
}  // namespace microshell

#endif  // SHELL_H
