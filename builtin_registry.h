#include <map>
#include <string>

#include "command.h"

namespace microshell {
namespace core {

using namespace std;

class BuiltinRegistry {
public:
  static BuiltinRegistry* instance() {
    if(!_instance) {
      _instance = new BuiltinRegistry;
    }
    return _instance;
  }

  template<typename B>
  bool register_factory(const string& name, BuiltinFactory *factory) {
    builtins[name] = factory;
    return true;
  }

  bool is_registered(const string& builtin_name) {
    return builtins.end() != builtins.find(builtin_name);
  }

  BuiltinCommand* build(const vector<string>& argv) {
    return builtins[argv[0]]->build(argv);
  }

private:
  static BuiltinRegistry* _instance;
  map<string, BuiltinFactory*> builtins;
};

}  // namespace core
}  // namespace ushell
