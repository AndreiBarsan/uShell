#ifndef SHELL_MODULE_H
#define SHELL_MODULE_H

#include <string>
#include <vector>

#include "command.h"
#include "shell.h"

namespace microshell {
namespace core {
  class ShellModule {
  public:
    /**
     * Used to set up various interactions with the shell, such as hooks and
     * callbacks (if applicable).
     */
    virtual void initialize(const Shell&) = 0;

    /**
     * Used by this module to expose its functionality to the shell's user
     * through various new builtin commands, if applicable.  Called during
     * shell initialization or on module load.
     */
    virtual vector<BuiltinCommand> get_builtins() = 0;
  };
}   // namespace core
}   // namespace microshell

#endif  // SHELL_MODULE_H
