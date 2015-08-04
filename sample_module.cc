#include "sample_module.h"

#include <memory>
#include <string>

#include "shell.h"

namespace microshell {
namespace modules {
namespace sample_module {

  using namespace microshell::core;
  using namespace std;

  void SampleModule::initialize(const Shell& shell) {
    shell.out("Initalized sample module.");
  }

  vector<shared_ptr<BuiltinFactory>> SampleModule::get_builtins() {
    return vector<shared_ptr<BuiltinFactory>> {
      make_shared<TypedBuiltinFactory<MooBuiltin>>(
        TypedBuiltinFactory<MooBuiltin>()
      )
    };
  }

  int MooBuiltin::invoke(Shell *shell) {
    shell->out("MOO!");
    return 0;
  }

}   // namespace sample_module
}   // namespace modules
}   // namespace microshell
