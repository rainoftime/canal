#include "Constant.h"
#include <llvm/Constants.h>
#include <llvm/Support/raw_ostream.h>

namespace Canal {

Constant::Constant(const llvm::Constant *constant) : mConstant(constant)
{
}

Constant* Constant::clone() const
{
    return new Constant(*this);
}

bool Constant::operator==(const Value &value) const
{
    // Check if rhs has the same type.
    const Constant *constant = dynamic_cast<const Constant*>(&value);
    if (!constant)
        return false;

    return mConstant == constant->mConstant;
}

size_t Constant::memoryUsage() const
{
    return sizeof(Constant);
}

void Constant::printToStream(llvm::raw_ostream &ostream) const
{
    ostream << "Constant(" << *mConstant << ")";
}

bool Constant::isAPInt() const
{
    return llvm::isa<llvm::ConstantInt>(mConstant);
}

const llvm::APInt &Constant::getAPInt() const
{
    return llvm::cast<llvm::ConstantInt>(mConstant)->getValue();
}

} // namespace Canal
