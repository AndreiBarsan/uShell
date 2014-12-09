#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <cstdlib>
#include <cstring>

#include <sys/wait.h>
#include <unistd.h>

/*
 * Ideas and such:
 *  - everything should have a proper interface and a dedicated implementation;
 *  - make sure that code handling shell debug traps, pipes, variable handling etc.
 *  is as clean as possible;
 * */

namespace microshell {

namespace util {
  std::vector<std::string> &split(const std::string &s,
                                  char delim,
                                  std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
      elems.push_back(item);
    }
    return elems;
  }

  std::string merge_with(const std::vector<std::string>::iterator& start,
                         const std::vector<std::string>::iterator& end,
                         const std::string delimitator) {
    std::stringstream ss;
    for(auto it = start; it != end; ++it) {
      ss << *it;
      if(it + 1 != end) {
        ss << delimitator;
      }
    }

    return ss.str();
  }

  /* ``Unwraps'' the vector `v', exposing a plain `char**' containing
   * the raw data from `v'.  Null terminates the result as well,
   * since many functions such as `execve' expect null-terminated
   * arrays.  The caller takes ownership of the array. */
  char** get_raw_array(const std::vector<std::string>& v) {
    char** ret = new char*[v.size() + 1];
    for(int i = 0; i < v.size(); ++i) {
      ret[i] = strdup(v[i].c_str());
    }
    ret[v.size()] = nullptr;
    return ret;
  }

}  // namespace util

namespace core {

using namespace std;

class Command {
  public:
    virtual int invoke() = 0;
};

class SimpleCommand : public Command {
};

/* Upon invocation, performs a fork and exec in order to
 * run 'argv'. */
class DiskCommand : public SimpleCommand {
public:
  DiskCommand(const vector<string>& argv) {
    this->argv = argv;
  }

  int invoke() {
    string comma_args = 
      microshell::util::merge_with(argv.begin() + 1,
                                   argv.end(),
                                   ", ");
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
    else {
      return this->handle_parent(child_pid);
    }
  }

private:
  vector<string> argv;

  int handle_parent(pid_t child_pid) {
    cout << "Spawned child. Waiting for child to terminate." << endl;
    int child_status;
    int waitpid_options = 0;
    waitpid(child_pid, &child_status, waitpid_options);
    cout << "Child has terminated. Exit code: "
         << child_status << endl;
    return child_status;
  }

  void handle_child() {
    char **argv = microshell::util::get_raw_array(this->argv);
    // 'environ' is inherited from the parent.
    char **parent_env = environ;
    execve(argv[0], argv, parent_env);
    // 'execve' doesn't return if it's successful.
    cerr << "Failed to 'execve'. OS says ["
         << strerror(errno) << "]. errno = " << errno << endl
         << "Terminating child." << endl;
    exit(-1);
  }

  void fork_error(int error) {
    cerr << "Could not create a child to run the command. "
         << "OS says [" << strerror(error) << "]. errno = "
         << error << endl;
  }
};

class BuiltinCommand : public SimpleCommand {
  // TODO(ioan) Implement some basic builtin commands.
};

class Shell {
public:
  Shell()
    : exit_requested(false) {
    cout << "Welcome to microshell!" << endl;
  }

  int interactive() {
    auto input = &cin;
    auto output = &cout;

    while (!exit_requested) {
      output_prompt(*output);
      string command_text = read_command(*input);
      if(0 == command_text.length()) {
        continue;
      }
      unique_ptr<Command> command(parse_command(command_text));
      interpret_command(*command);
    }
  } 

protected:
  void output_prompt(ostream& output) {
    output << "ush >> ";
  }

  string read_command(istream& input) {
    string command;
    // This needs fine tuning.  We should keep peeking in case the 
    // user types e.g. an arrow key and respond to that right away.
    getline(input, command);
    return command;
  }

  int interpret_command(Command &cmd) {
    return cmd.invoke();
  }

  unique_ptr<Command> parse_command(const string& command_text) {
    vector<string> argv;
    microshell::util::split(command_text, ' ', argv);
    return unique_ptr<Command>(new DiskCommand(argv));
  }

private:
  bool exit_requested;  
};

}  // namespace core
}  // namespace microshell

int main(int argc, char **argv) {
    microshell::core::Shell shell;
    int result = shell.interactive();
    return result;
}
