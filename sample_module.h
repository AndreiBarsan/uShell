#ifndef MICROSHELL_MODULES_SAMPLE_MODULE_SAMPLE_MODULE_H
#define MICROSHELL_MODULES_SAMPLE_MODULE_SAMPLE_MODULE_H

#include <string>
#include <vector>

#include "command.h"
#include "shell.h"
#include "shell_module.h"

namespace microshell {
namespace modules {
namespace sample_module {
  class SampleModule : public microshell::core::ShellModule {
  public:
    void initialize(const microshell::core::Shell&) override;
    std::vector<std::shared_ptr<microshell::core::BuiltinFactory>> get_builtins() override;
  };

  DECLARE_BUILTIN(Moo);

}   // namespace sample_module
}   // namespace modules
}   // namespace microshell

#endif  // MICROSHELL_MODULES_SAMPLE_MODULE_SAMPLE_MODULE_H
