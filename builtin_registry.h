#include <map>
#include <memory>
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
  bool register_factory(const string& name, shared_ptr<BuiltinFactory> factory) {
    builtins[name] = factory;
    return true;
  }

  bool is_registered(const string& builtin_name) {
    return builtins.end() != builtins.find(builtin_name);
  }

  shared_ptr<BuiltinCommand> build(const vector<string>& argv) {
    cout << "Builtins: " << argv[0] << " " << builtins[argv[0]] << endl;
    return builtins[argv[0]]->build(argv);
  }

private:
  static BuiltinRegistry* _instance;
  map<string, shared_ptr<BuiltinFactory>> builtins;
};

}  // namespace core
}  // namespace ushell
