#include "job_control.h"

#include <csignal>

// Helper macro for instantiating builtin factories.
#define _(name, type) (std::make_shared<TypedBuiltinFactory<type>>(TypedBuiltinFactory<type>(name)))

namespace microshell {
namespace modules {
namespace job_control {

using namespace std;
using namespace microshell::core;

void JobControl::initialize(const microshell::core::Shell& shell) {
}

vector<shared_ptr<BuiltinFactory>> JobControl::get_builtins() {
  return vector<shared_ptr<BuiltinFactory>> {
    _("bg", BgBuiltin),
    _("disown", DisownBuiltin),
    _("fg", FgBuiltin),
    _("jobs", JobsBuiltin),
    _("kill", KillBuiltin),
    _("killall", KillallBuiltin)
  };
}

int BgBuiltin::invoke(Shell *) {
  return 0;
}

int DisownBuiltin::invoke(Shell *) {
  return 0;
}

int FgBuiltin::invoke(Shell *) {
  return 0;
}

int JobsBuiltin::invoke(Shell *) {
  return 0;
}

int KillBuiltin::invoke(Shell *) {
  return 0;
}

int KillallBuiltin::invoke(Shell *) {
  return 0;
}

int WaitBuiltin::invoke(Shell *) {
  return 0;
}

}   // namespace job_control
}   // namespace modules
}   // namespace microshell

