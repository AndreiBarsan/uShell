#include <string>
#include <vector>

#include <cstring>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtin_registry.h"
#include "command.h"
#include "shell.h"
#include "util.h"

#define REGISTER_BUILTIN(TYPE, name) \
  bool is_registered_##name = BuiltinRegistry::instance() \
        ->register_factory<TYPE>(#name, new TypedBuiltinFactory< TYPE >);

namespace microshell {
namespace core {

using namespace std;

int DiskCommand::invoke(Shell *shell) {
  string comma_args = 
    util::merge_with(argv.begin() + 1, argv.end(), ", ");
  shell->out("Invoking program [" + argv[0] + "] with args [" +
              comma_args + "]");
  pid_t child_pid = fork();
  if(-1 == child_pid) {
    // TODO(andrei) Shell::perror().
    shell->eout("Could not crete a child to run the command. "
                "OS says [" + string(strerror(errno)) + "]. errno = " +
                to_string(errno));
    return -1;
  }
  else if(0 == child_pid) {
    this->handle_child(shell);
    // No return, the child will just 'exec' or 'exit' (on error).
  }
  
  return this->handle_parent(shell, child_pid);
}
  
int DiskCommand::handle_parent(Shell *shell, pid_t child_pid) {
  shell->out("Spawned child. Waiting for child to terminate.");
  int child_status;
  int waitpid_options = 0;
  waitpid(child_pid, &child_status, waitpid_options);
  shell->out("Child has terminated. Exit code: " + to_string(child_status));
  return child_status;
}

void DiskCommand::handle_child(Shell *shell) {
  char **argv = util::get_raw_array(this->argv);
  // 'environ' is inherited from the parent.
  char **parent_env = environ;
  execve(argv[0], argv, parent_env);
  // 'execve' doesn't return if it's successful.
  shell->eout("Failed to 'execve'. OS says [" + string(strerror(errno))
               + "]. errno = " + to_string(errno));
  shell->eout("Terminating child.");
  exit(-1);
}

int ExitBuiltin::invoke(Shell *shell) {
  shell->out("Bye!");
  exit(0);
  return -1;
}
REGISTER_BUILTIN(ExitBuiltin, exit);

int PwdBuiltin::invoke(Shell *shell) {
  cout << shell->get_working_directory() << endl;
  return 0;
}
REGISTER_BUILTIN(PwdBuiltin, pwd);

int CdBuiltin::invoke(Shell *) {
  return 0;
}
REGISTER_BUILTIN(CdBuiltin, cd);

BuiltinRegistry* BuiltinRegistry::_instance = nullptr;


}  // namespace core
}  // namespace microshell
