#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H

#include <map>
#include <string>

#include "command.h"
#include "shell.h"
#include "shell_module.h"
#include "util.h"

namespace microshell {
namespace modules {
namespace job_control {

  /**
   * The shell module tasked with managing running jobs, whether in the
   * foreground or in the background.
   *
   * Provides aliases(?):
   *    - %
   *
   * Provides builtins:
   *    - fg
   *    - bg
   *    - jobs
   *    - kill
   *    - killall
   */
  class JobControl : public microshell::core::ShellModule {
    void initialize(const microshell::core::Shell&) override;
    std::vector<std::shared_ptr<microshell::core::BuiltinFactory>> get_builtins() override;
  };

}   // namespace job_control
}   // namespace modules
}   // namespace microshell

#endif  // JOB_CONTROL_H

