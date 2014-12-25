// µShell (microShell, uShell, ush).  An experimental Unix shell implementation
// written in C++.
//
// Author: Andrei Barsan
// Project started in December 2014.
// License: TBD (will be free)

// C++
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// C includes
#include <cstdlib>
#include <cstdio>
#include <cstring>

// Linux-specific
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* Ideas and TODOS
 * ---------------
 *  - everything should have a proper interface and a dedicated implementation
 *  - make sure that code handling shell debug traps, pipes, variable handling etc.
 *  is as clean as possible
 *  - yacc should work to generate the grammar like in the regular Bash
 *  - always bear in mind that plugins will have to be able to change anything
 *  - consider blogging about progress
 *  - document self about all variants of redirection in Bash and ensure that they
 *    are all supported
 *  - possible milestones:
 *    - manual (limited) redirection support (i.e. look for >foo >>foo tokens)
 *    - simple job control
 *      - async tasks (&)
 *      - jobs
 *      - bg
 *      - fg
 *      - wait
 *      - disown
 *      - suspend
 *      - kill
 *      - killall
 *      - etc. (http://tldp.org/LDP/abs/html/x9644.html)
 *    - pipelines:
 *      - as above (`foo & bar`)
 *      - `foo; bar` (no pipes established)
 *      - `foo | bar`
 *      - mixed (`foo; bar | baz`)
 *    - figure out streamlined way to perform e2e testing
 */

namespace util {

  // Shush, even Google does it like this.
  // Yes, I know it can be done cleaner using varadic templates, but that
  // requires gcc 4.9, which I don't have.

  std::string merge_paths(const std::string& path) {
    return path;
  }

  std::string merge_paths(const std::string& path1, const std::string& path2) {
    return path1 + "/" + path2;
  }

  std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
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
    for(size_t i = 0; i < v.size(); ++i) {
      ret[i] = strdup(v[i].c_str());
    }
    ret[v.size()] = nullptr;
    return ret;
  }

}  // namespace util

namespace microshell {

namespace core {

using namespace std;

class Command {
  public:
    virtual ~Command() { }
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

  void fork_error(int error) {
    cerr << "Could not create a child to run the command. "
         << "OS says [" << strerror(error) << "]. errno = "
         << error << endl;
  }
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
    int invoke() {
      cout << "Bye!" << endl;
      exit(0);
      return -1;
    }
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

protected:
  void output_prompt(ostream& output) {
    output << prompt;
  }

  string read_command(istream& input) {
    // We have already output our prompt at this point.
    char* line = readline("");
    string command(line);
    if(command.size() > 0) {
      add_history(line);
    }
    free(line);
    return command;
  }

  int interpret_command(Command &cmd) {
    return cmd.invoke();
  }

  bool parse_command(const string& command_text,
                                    Command **command,
                                    string *error) {
    vector<string> argv = util::split(command_text, ' ');
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

  bool resolve_binary_name(const string& name, string* full_path) {
    // PLATFORM_SPECIFIC
    struct stat stat_buf;
    for(const string& p : path) {
      cout << "Inspecting: " << p << endl;
      string path = util::merge_paths(p, name);
      if(0 == stat(path.c_str(), &stat_buf)) {
        *full_path = path;
        return true;
      }
    }

    return false;
  }

  bool is_builtin(const string& builtin_name) {
    return builtin_table.end() != builtin_table.find(builtin_name);
  }

  BuiltinCommand* get_builtin(const vector<string>& argv) {
    return builtin_table[argv[0]];
  }

private:
  bool exit_requested; 
  string prompt = "ush >> ";
  map<string, BuiltinCommand*> builtin_table = {
    { "exit", new ExitBuiltin() }
  };

  // The list of folders found inside the PATH environment variable.
  vector<string> path;
};

}  // namespace core
}  // namespace microshell

int main(int argc, char **argv) {
    microshell::core::Shell shell;
    int result = shell.interactive();
    return result;
}
