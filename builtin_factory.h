#ifndef MICROSHELL_CORE_BUILTIN_FACTORY_H
#define MICROSHELL_CORE_BUILTIN_FACTORY_H

#include <memory>
#include <vector>

#include "command.h"

namespace microshell {
namespace core {

class BuiltinCommand;

// The interface for a `TypedBuiltinFactory', which can construct a particular
// (typed) builtin using an argv.
class BuiltinFactory {
public:
  virtual std::shared_ptr<BuiltinCommand> build(
    const std::vector<std::string>& argv
  ) = 0;
  virtual const std::string& get_name() const = 0;
};

template<class BUILTIN>
class TypedBuiltinFactory : public BuiltinFactory {
public:
  TypedBuiltinFactory(const std::string &name) : name(name) { }

  std::shared_ptr<BuiltinCommand> build(const std::vector<std::string>& argv) {
    return std::make_shared<BUILTIN>(BUILTIN(argv));
  }

  const std::string& get_name() const {
    return name;
  }
private:
  const std::string name;
};

}   // namespace core
}   // namespace microshell

#endif  // MICROSHELL_CORE_BUILTIN_FACTORY_H
