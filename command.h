#ifndef COMMAND_H
#define COMMAND_H

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <sys/types.h>

namespace microshell {
namespace core {

// TODO(andrei) Make sure this macro is appropriate.  Perhaps using a default
// ``coreutils''-like module with all the basic builtins (e.g. `cd', `ls',
// etc.) might make more sense.
#define DECLARE_BUILTIN(name)                                                 \
  class name##Builtin : public BuiltinCommand {                               \
  public:                                                                     \
    using BuiltinCommand::BuiltinCommand;                                     \
    name##Builtin(const name##Builtin* other) : name##Builtin(*other) { };    \
    virtual int invoke(Shell *shell) override;                                \
    string get_name() const override { return "#name"; }                      \
  };

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
public:
  SimpleCommand(const vector<string>& argv) : argv(argv) { }

protected:
  vector<string> argv;
};

// Upon invocation, performs a fork and exec in order to run 'argv' (see
// `SimpleCommand').
class DiskCommand : public SimpleCommand {
public:
  using SimpleCommand::SimpleCommand;
  DiskCommand(const DiskCommand* other) : DiskCommand(*other) { };
  int invoke(Shell *shell);

private:
  int handle_parent(Shell *shell, pid_t child_pid);
  void handle_child(Shell *shell);
};

class BuiltinCommand : public SimpleCommand {
public:
  using SimpleCommand::SimpleCommand;
  virtual string get_name() const = 0;
};

// The interface for a `TypedBuiltinFactory', which can construct a particular
// (typed) builtin using an argv.
class BuiltinFactory {
  public:
    virtual shared_ptr<BuiltinCommand> build(const vector<string>& argv) = 0;
};

template<class BUILTIN>
class TypedBuiltinFactory : public BuiltinFactory {
  public:
    shared_ptr<BuiltinCommand> build(const vector<string>& argv) {
      return make_shared<BUILTIN>(new BUILTIN(argv));
    }
};

DECLARE_BUILTIN(Exit);

class PwdBuiltin : public BuiltinCommand {
  public:
    using BuiltinCommand::BuiltinCommand;
    PwdBuiltin(const PwdBuiltin* other) : PwdBuiltin(*other) { };
    int invoke(Shell *shell);
    string get_name() const { return "pwd"; }
};

class CdBuiltin : public BuiltinCommand {
  public:
    using BuiltinCommand::BuiltinCommand;
    CdBuiltin(const CdBuiltin* other) : CdBuiltin(*other) { };
    int invoke(Shell *shell);
    string get_name() const { return "cd"; }
};

}  // namespace core
}  // namespace microshell

#endif  // COMMAND_H
