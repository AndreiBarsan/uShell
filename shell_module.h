#ifndef SHELL_MODULE_H
#define SHELL_MODULE_H

#include <memory>
#include <vector>

#include "builtin_factory.h"
#include "command.h"
#include "shell.h"

namespace microshell {
namespace core {
  /**
   * A self-contained module providing µShell with additional functionality.
   * This functionality is exposed via new builtin commands, hooks and
   * callbacks.
   */
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
    virtual vector<shared_ptr<BuiltinFactory>> get_builtins() = 0;
  };
}   // namespace core
}   // namespace microshell

#endif  // SHELL_MODULE_H
