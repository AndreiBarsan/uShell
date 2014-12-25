#include <string>
#include <vector>

#include <cstring>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "command.h"
#include "shell.h"
#include "util.h"

namespace microshell {
namespace core {

using namespace std;

int DiskCommand::invoke(Shell *shell) {
  string comma_args = 
    util::merge_with(argv.begin() + 1, argv.end(), ", ");
  cout << "Invoking program [" << argv[0] << "] with args ["
       << comma_args << "]" << endl;
  pid_t child_pid = fork();
  if(-1 == child_pid) {
    this->fork_error(errno);
    return -1;
  }
  else if(0 == child_pid) {
    this->handle_child();
    // No return, the child will just 'exec' or 'exit' (on error).
  }

  return this->handle_parent(child_pid);
}
  
int DiskCommand::handle_parent(pid_t child_pid) {
  cout << "Spawned child. Waiting for child to terminate." << endl;
  int child_status;
  int waitpid_options = 0;
  waitpid(child_pid, &child_status, waitpid_options);
  cout << "Child has terminated. Exit code: "
       << child_status << endl;
  return child_status;
}

void DiskCommand::handle_child() {
  char **argv = util::get_raw_array(this->argv);
  // 'environ' is inherited from the parent.
  char **parent_env = environ;
  execve(argv[0], argv, parent_env);
  // 'execve' doesn't return if it's successful.
  cerr << "Failed to 'execve'. OS says ["
       << strerror(errno) << "]. errno = " << errno << endl
       << "Terminating child." << endl;
  exit(-1);
}

void DiskCommand::fork_error(int error) {
  cerr << "Could not create a child to run the command. "
       << "OS says [" << strerror(error) << "]. errno = "
       << error << endl;
}

}  // namespace core
}  // namespace microshell
