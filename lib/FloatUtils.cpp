#include "FloatUtils.h"
#include "Utils.h"

namespace Canal {
namespace Float {
namespace Utils {

const llvm::fltSemantics &
getSemantics(const llvm::Type &type)
{
    CANAL_ASSERT(type.isFloatingPointTy());

#if (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 1) || LLVM_VERSION_MAJOR > 3
    if (type.isHalfTy())
        return llvm::APFloat::IEEEhalf;
    else
#endif
    if (type.isFloatTy())
        return llvm::APFloat::IEEEsingle;
    else if (type.isDoubleTy())
        return llvm::APFloat::IEEEdouble;
    else if (type.isFP128Ty())
        return llvm::APFloat::IEEEquad;
    else if (type.isPPC_FP128Ty())
        return llvm::APFloat::PPCDoubleDouble;
    else if (type.isX86_FP80Ty())
        return llvm::APFloat::x87DoubleExtended;

    CANAL_NOT_IMPLEMENTED();
}

const llvm::Type &
getType(const llvm::fltSemantics &semantics, llvm::LLVMContext &context)
{
#if (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 1) || LLVM_VERSION_MAJOR > 3
    if (&semantics == &llvm::APFloat::IEEEhalf)
        return *llvm::Type::getHalfTy(context);
    else
#endif
    if (&semantics == &llvm::APFloat::IEEEsingle)
        return *llvm::Type::getFloatTy(context);
    else if (&semantics == &llvm::APFloat::IEEEdouble)
        return *llvm::Type::getDoubleTy(context);
    else if (&semantics == &llvm::APFloat::IEEEquad)
        return *llvm::Type::getFP128Ty(context);
    else if (&semantics == &llvm::APFloat::PPCDoubleDouble)
        return *llvm::Type::getPPC_FP128Ty(context);
    else if (&semantics == &llvm::APFloat::x87DoubleExtended)
        return *llvm::Type::getX86_FP80Ty(context);

    CANAL_NOT_IMPLEMENTED();
}

llvm::APInt
toInteger(const llvm::APFloat &num, unsigned bitWidth, bool isSigned, llvm::APFloat::opStatus &status)
{
    llvm::APInt result(bitWidth, 0, isSigned);
    llvm::SmallVector<uint64_t, 4> parts(result.getNumWords());
    bool isExact;
    status = num.convertToInteger(parts.data(),
                                  bitWidth,
                                  isSigned,
                                  llvm::APFloat::rmTowardZero,
                                  &isExact);

    if (status == llvm::APFloat::opOK || status == llvm::APFloat::opInexact)
        return llvm::APInt(bitWidth,
                       (unsigned)parts.size(),
                       (const uint64_t*)parts.data());
    else return llvm::APInt(bitWidth, 0);
}

} // namespace Utils
} // namespace Float
} // namespace Canal
