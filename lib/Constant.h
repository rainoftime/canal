#ifndef CANAL_CONSTANT_H
#define CANAL_CONSTANT_H

#include "Value.h"

namespace llvm {
    class Constant;
}

namespace Canal {

class Constant : public Value
{
public:
    Constant(const llvm::Constant *constant);

    // Implementation of Value::clone().
    // Covariant return type -- it really overrides Value::clone().
    virtual Constant* clone() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value &value) const;

protected:
    const llvm::Constant *mConstant;
};

} // namespace Canal

#endif // CANAL_CONSTANT_H
