#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>

#include <sys/types.h>

namespace microshell {
namespace core {

using namespace std;

class Shell;

// Models every possible shell command, from simple program invocations to
// pipelines, functions and command substitutions.
class Command {
  public:
    virtual ~Command() { }
    virtual int invoke(Shell *) = 0;
};

// A simple command with no particular structure, such as a disk command
// (e.g. `foo -bar') or a builtin (e.g. `pwd').
class SimpleCommand : public Command {
};

// Upon invocation, performs a fork and exec in order to
// run 'argv'.
class DiskCommand : public SimpleCommand {
public:
  DiskCommand(const vector<string>& argv) : argv(argv) { }

  int invoke(Shell *shell);

private:
  vector<string> argv;

  int handle_parent(pid_t child_pid);

  void handle_child();
  void fork_error(int error);
};

class BuiltinCommand : public SimpleCommand {
public:
  BuiltinCommand(const string& name) : name(name) { }
  
  string get_name() const {
    return name;
  }

private:
  const string name;
};

class ExitBuiltin : public BuiltinCommand {
  public:
    ExitBuiltin() : BuiltinCommand("exit") { }
    int invoke(Shell *shell);
};

class PwdBuiltin : public BuiltinCommand {
  public:
    PwdBuiltin() : BuiltinCommand("pwd") { }
    int invoke(Shell *shell);
};

}  // namespace core
}  // namespace microshell

#endif  // COMMAND_H
