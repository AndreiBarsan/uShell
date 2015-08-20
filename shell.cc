// C++ includes
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

// C includes
#include <cstdlib>
#include <cstdio>
#include <cstring>

// OS-specific
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "builtin_registry.h"
#include "command.h"
#include "job_control.h"
#include "sample_module.h"
#include "shell.h"
#include "util.h"

namespace microshell {
namespace core {

using namespace std;
using namespace microshell::modules;

const int SHELL_FATAL = -1;

// Singleton initialization.
Shell* Shell::instance = nullptr;

void handle_sigint(int signo) {
  // TODO(andrei) This will be invalid for a brief moment between when this
  // signal handler is set up and when the shell initialization finishes.
  Shell *shell = Shell::get();

  if(SIGINT != signo) {
    // TODO(andrei) Signal2name.
    shell->fatal("Signal mismatch.  Expecting SIGINT, got " + to_string(signo));
  }

  if (shell->get_waiting_for_child()) {
    shell->info("C-c while child was running.");
  }
  else {
    shell->info("C-c while NO child was running. Terminating shell.");
    // TODO(andrei) Ensure that we synchronize properly.
    shell->exit();
  }
}

Shell::Shell(const vector<string> &) :
    exit_requested(false),
    home_directory(util::get_current_home()),
    standard_output(cout),
    error_output(cerr),
    name("ush"),
    username(util::get_current_user()),
    waiting_for_child(false) {
  this->load_default_modules();
  cout << "Welcome to microshell, " << username << "!" << endl;
  if(!util::getcwd(&this->working_directory)) {
    eout("Failed to get the current working directory.");
    // TODO(andrei) Display what `~' resolves to between parentheses.  If it
    // can't be resolved, use `/' instead.
    eout("Defaulting to `~'.");
    // TODO(andrei) Use setter and actually resolve $HOME.
    this->working_directory = "~";
  }

  string envpath = getenv("PATH");
  this->path = util::split(envpath, ':');

  this->info("Setting up signal handlers...");
  if(SIG_ERR == signal(SIGINT, handle_sigint)) {
    this->fatal("Could not set up sigint handler (C-c terminate support).");
  }
}

int Shell::interactive() {
  while (!exit_requested) {
    string command_text = read_command();
    if(0 == command_text.length()) {
      cout << endl;
      continue;
    }

    shared_ptr<Command> command;
    string error;
    if(parse_command(command_text, command, &error)) {
      interpret_command(*command);
    }
    else {
      eout(error);
    }
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

const Shell* Shell::out(const string& message) {
  this->standard_output << message << endl;
  return this;
}

const Shell* Shell::eout(const string& message) {
  this->error_output << message << endl;
  return this;
}

const Shell* Shell::info(const string& message) {
  this->out(this->name + ": " + message);
  return this;
}

const Shell* Shell::warning(const string& message) {
  this->out(this->name + " (warning): " + message);
  return this;
}

const Shell* Shell::error(const string& message) {
  this->eout(this->name + " (error): " + message);
  return this;
}

void Shell::fatal(const string& message) {
  this->eout("FATAL: " + message);
  ::exit(SHELL_FATAL);
}

void Shell::exit() {
  if(this->exit_requested) {
    this->fatal("Requested exit twice.");
  }

  // This way, the shell gets a chance to shut down in a clean fashion without
  // having to call `exit()'.
  // TODO(andrei) Test that nothing else is ever run after `exit_requested' is
  // set, no matter where.
  this->exit_requested = true;
}

template<class MODULE_TYPE>
int Shell::load_module(shared_ptr<MODULE_TYPE> module) {
  module->initialize(*this);
  auto builtins = module->get_builtins();
  for(auto bf = builtins.begin(); bf != builtins.end(); ++bf) {
    // TODO(andrei) Use managed memory in builtin registry.
    // TODO(andrei) Builtin factories should know the builtin's name.
    BuiltinRegistry::instance()->register_factory<MODULE_TYPE>(
      (*bf)->get_name(), *bf
    );
  }
  loaded_modules.push_back(module);
  return 0;
}

string Shell::get_prompt() const {
  return "(" + this->working_directory + ") " + this->prompt;
}

string Shell::read_command() {
  char *line = readline(get_prompt().c_str());

  // This happens if e.g. the user enters an EOF character (C-D).
  if(!line) {
    this->exit();
    return "";
  }

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
                          shared_ptr<Command> &command,
                          string *error) const {
  // TODO(andrei) YACC this.
  // TODO(andrei) Customizable IFS variable.
  vector<string> argv = util::split(command_text, ' ');

  // Perform expansion for every parameter.
  for(string& arg : argv) {
    arg = expand(arg);
  }

  const string& program_name = argv[0];

  if(is_builtin(argv[0])) {
    command = construct_builtin(argv);
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
      command = make_shared<DiskCommand>(new DiskCommand(argv));
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

shared_ptr<BuiltinCommand> Shell::construct_builtin(const vector<string>& argv) const {
  return BuiltinRegistry::instance()->build(argv);
}

int Shell::load_default_modules() {
  this->load_module(
    make_shared<sample_module::SampleModule>(sample_module::SampleModule())
  );
  this->load_module(
    make_shared<job_control::JobControl>(job_control::JobControl())
  );
  return 0;
}

bool Shell::get_waiting_for_child() const {
  return this->waiting_for_child;
}

// TODO(andrei) Restructure this method.  In many cases a child may not have an
// exit status, such as in the case when it is killed by a signal (9, force
// kill or 11, out of memory).
int Shell::wait_child(int child_pid) {
  int child_status;
  int child_exit_code = -1;
  int waitpid_options = 0;//WEXITED | WSTOPPED;

  this->waiting_for_child = true;
  // TODO(andrei) Consider using wait4 and logging rusage data.
  while(true) {
    if(!waitpid(child_pid, &child_status, waitpid_options)) {
      this->fatal("Waitpid returned 0.");
    }
    this->info("Woken up!");

    if(WIFEXITED(child_status)) {
      child_exit_code = WEXITSTATUS(child_status);
      this->info(
        "Child exited normally (exit code: " + to_string(child_exit_code) + ")."
      );
      break;
    }

    if(WIFSIGNALED(child_status)) {
      int child_murdering_signal = WTERMSIG(child_status);
      this->info(strsignal(child_murdering_signal));
      this->info(
        "Child killed by signal " + to_string(child_murdering_signal) + "."
      );
      break;
    }

    if(WIFSTOPPED(child_status)) {
      int child_stopper = WSTOPSIG(child_status);
      this->info(strsignal(child_stopper));
      this->info("Child stopped by signal " + to_string(child_stopper) + ".");
      break;
    }
  }
  signal(SIGCHLD, SIG_IGN);
  this->waiting_for_child = false;

  // TODO(andrei) This is broken on killed/suspended children.
  return child_status;
}

}  // namespace core
}  // namespace microshell
