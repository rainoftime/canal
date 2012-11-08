#include "Constructors.h"
#include "IntegerContainer.h"
#include "Utils.h"
#include "ArraySingleItem.h"
#include "ArrayExactSize.h"
#include "FloatInterval.h"
#include "Pointer.h"
#include "PointerUtils.h"
#include "Structure.h"
#include "Environment.h"
#include "State.h"
#include "APIntUtils.h"
#include <llvm/Constants.h>
#include <llvm/Function.h>
#include <llvm/ADT/OwningPtr.h>

namespace Canal {

Constructors::Constructors(const Environment &environment)
    : mEnvironment(environment)
{
}

Domain *
Constructors::create(const llvm::Type &type) const
{
    CANAL_ASSERT_MSG(!type.isVoidTy(), "Cannot create value of type Void.");

    if (type.isIntegerTy())
    {
        llvm::IntegerType &integerType =
            llvmCast<llvm::IntegerType>(type);

        return createInteger(integerType.getBitWidth());
    }

    if (type.isFloatingPointTy())
    {
        const llvm::fltSemantics *semantics =
            getFloatingPointSemantics(type);

        return createFloat(*semantics);
    }

    if (type.isPointerTy())
    {
        const llvm::PointerType &pointerType =
            llvmCast<const llvm::PointerType>(type);

        CANAL_ASSERT_MSG(pointerType.getElementType(),
                         "Element type must be known.");

        return createPointer(*pointerType.getElementType());
    }

    if (type.isArrayTy() || type.isVectorTy())
    {
        uint64_t size;
        if (type.isArrayTy())
        {
            const llvm::ArrayType &arrayType =
                llvm::cast<llvm::ArrayType>(type);

            size = arrayType.getNumElements();
        }
        else
        {
            const llvm::VectorType &vectorType =
                llvm::cast<llvm::VectorType>(type);

            size = vectorType.getNumElements();
        }

        llvm::OwningPtr<Domain> value(create(*type.getContainedType(0)));
        return createArray(size, *value);
    }

    if (type.isStructTy())
    {
        const llvm::StructType &structType =
            llvmCast<llvm::StructType>(type);

        std::vector<Domain*> members;
        for (unsigned i = 0; i < structType.getNumElements(); i ++)
            members.push_back(create(*structType.getElementType(i)));

        Structure *structure = new Structure(mEnvironment, members);
        return structure;
    }

    CANAL_DIE_MSG("Unsupported llvm::Type::TypeID: " << type.getTypeID());
}

Domain *
Constructors::create(const llvm::Constant &value,
                     const llvm::Value &place,
                     const State *state) const
{
    if (llvm::isa<llvm::UndefValue>(value))
        return create(*value.getType());

    if (llvm::isa<llvm::ConstantInt>(value))
    {
        const llvm::ConstantInt &intValue =
            llvmCast<llvm::ConstantInt>(value);

        const llvm::APInt &i = intValue.getValue();
        return createInteger(i);
    }

    if (llvm::isa<llvm::ConstantPointerNull>(value))
    {
        const llvm::ConstantPointerNull &nullValue =
            llvmCast<llvm::ConstantPointerNull>(value);

        const llvm::PointerType &pointerType = *nullValue.getType();
        Domain *constPointer;
        constPointer = createPointer(*pointerType.getElementType());

        Pointer::Utils::addTarget(*constPointer,
                                  Pointer::Target::Constant,
                                  &place,
                                  NULL,
                                  std::vector<Domain*>(),
                                  NULL);

        return constPointer;
    }

    if (llvm::isa<llvm::ConstantExpr>(value))
    {
        CANAL_ASSERT_MSG(state,
                         "State is mandatory for constant expressions.");

        const llvm::ConstantExpr &exprValue =
            llvmCast<llvm::ConstantExpr>(value);

        Domain *variable = NULL;
        bool deleteVariable = false;
        const llvm::Value &firstValue = **value.op_begin();
        if (llvm::isa<llvm::ConstantExpr>(firstValue))
        {
            variable = create(llvmCast<llvm::ConstantExpr>(firstValue),
                              place,
                              state);

            deleteVariable = true;
        }
        else
            variable = state->findVariable(**value.op_begin());

        CANAL_ASSERT_MSG(variable, "It is expected that variable used"
                         " in constant expressions is available.");

        Domain *result = NULL;
        switch (exprValue.getOpcode())
        {
        case llvm::Instruction::GetElementPtr:
            result = createGetElementPtr(exprValue, *variable, place);
            break;
        case llvm::Instruction::BitCast:
            result = createBitCast(exprValue, *variable, place);
            break;
        default:
            CANAL_NOT_IMPLEMENTED();
        }

        if (deleteVariable)
            delete variable;

        return result;
    }

    if (llvm::isa<llvm::ConstantFP>(value))
    {
        const llvm::ConstantFP &fp = llvmCast<llvm::ConstantFP>(value);

        const llvm::APFloat &number = fp.getValueAPF();
        return createFloat(number);
    }

    if (llvm::isa<llvm::ConstantStruct>(value))
    {
        const llvm::ConstantStruct &structValue =
            llvmCast<llvm::ConstantStruct>(value);

        uint64_t elementCount = structValue.getType()->getNumElements();
        std::vector<Domain*> members;
        for (uint64_t i = 0; i < elementCount; ++i)
        {
            members.push_back(create(*structValue.getOperand(i),
                                     place,
                                     state));
        }

        return new Structure(mEnvironment, members);
    }

    if (llvm::isa<llvm::ConstantVector>(value))
    {
        const llvm::ConstantVector &vectorValue =
            llvmCast<llvm::ConstantVector>(value);

        // VectorType::getNumElements returns unsigned int.
        unsigned elementCount = vectorValue.getType()->getNumElements();
        std::vector<Domain*> values;
        for (unsigned i = 0; i < elementCount; ++i)
        {
            values.push_back(create(*vectorValue.getOperand(i),
                                    place,
                                    state));
        }

        return createArray(values);
    }

    if (llvm::isa<llvm::ConstantArray>(value))
    {
        const llvm::ConstantArray &arrayValue =
            llvmCast<llvm::ConstantArray>(value);

        // ArrayType::getNumElements returns uint64_t.
        uint64_t elementCount = arrayValue.getType()->getNumElements();
        std::vector<Domain*> values;
        for (uint64_t i = 0; i < elementCount; ++i)
        {
            values.push_back(create(*arrayValue.getOperand(i),
                                    place,
                                    state));
        }

        return createArray(values);
    }

#if (LLVM_MAJOR == 3 && LLVM_MINOR >= 1) || LLVM_MAJOR > 3
    if (llvm::isa<llvm::ConstantDataSequential>(value))
    {
         const llvm::ConstantDataSequential &sequentialValue =
            llvmCast<llvm::ConstantDataSequential>(value);

        unsigned elementCount = sequentialValue.getNumElements();
        std::vector<Domain*> values;
        for (unsigned i = 0; i < elementCount; ++i)
        {
            values.push_back(create(*sequentialValue.getElementAsConstant(i),
                                    place,
                                    state));
        }

        return createArray(values);
   }
#endif

    if (llvm::isa<llvm::ConstantAggregateZero>(value))
    {
        const llvm::Type *type = value.getType();
        Domain *result = Constructors::create(*type);
        result->setZero(&place);
        return result;
    }

    if (llvm::isa<llvm::Function>(value))
    {
        const llvm::Function &functionValue =
            llvmCast<llvm::Function>(value);

        Domain *constPointer;
        constPointer = createPointer(*functionValue.getFunctionType());

        Pointer::Utils::addTarget(*constPointer,
                                  Pointer::Target::Function,
                                  &place,
                                  &value,
                                  std::vector<Domain*>(),
                                  NULL);

        return constPointer;
    }

    CANAL_DIE_MSG("not implemented for " << typeid(value).name());
}

Domain *
Constructors::createInteger(unsigned bitWidth) const
{
    return new Integer::Container(mEnvironment, bitWidth);
}

Domain *
Constructors::createInteger(const llvm::APInt &number) const {
    return new Integer::Container(mEnvironment, number);
}

Domain *
Constructors::createFloat(const llvm::fltSemantics &semantics) const {
    return new Float::Interval(mEnvironment, semantics);
}

Domain *
Constructors::createFloat(const llvm::APFloat &number) const {
    return new Float::Interval(mEnvironment, number);
}

Domain *
Constructors::createArray(Domain *size, Domain *value) const {
    return new Array::SingleItem(mEnvironment, size, value);
}

Domain *
Constructors::createArray(uint64_t size, const Domain &value) const {
    return new Array::ExactSize(mEnvironment, size, value);
}

Domain *
Constructors::createArray(const std::vector<Domain*> &values) const {
    return new Array::ExactSize(mEnvironment, values);
}

Domain *
Constructors::createPointer(const llvm::Type &type) const {
    return new Pointer::Pointer(mEnvironment, type);
}

const llvm::fltSemantics *
Constructors::getFloatingPointSemantics(const llvm::Type &type)
{
    CANAL_ASSERT(type.isFloatingPointTy());

    const llvm::fltSemantics *semantics;
#if (LLVM_MAJOR == 3 && LLVM_MINOR >= 1) || LLVM_MAJOR > 3
    if (type.isHalfTy())
        semantics = &llvm::APFloat::IEEEhalf;
    else
#endif
    if (type.isFloatTy())
        semantics = &llvm::APFloat::IEEEsingle;
    else if (type.isDoubleTy())
        semantics = &llvm::APFloat::IEEEdouble;
    else if (type.isFP128Ty())
        semantics = &llvm::APFloat::IEEEquad;
    else if (type.isPPC_FP128Ty())
        semantics = &llvm::APFloat::PPCDoubleDouble;
    else if (type.isX86_FP80Ty())
        semantics = &llvm::APFloat::x87DoubleExtended;
    else
        CANAL_DIE();
    return semantics;
}

Domain *
Constructors::createGetElementPtr(const llvm::ConstantExpr &value,
                                  const Domain &variable,
                                  const llvm::Value &place) const
{
    std::vector<Domain*> offsets;
    llvm::ConstantExpr::const_op_iterator it = value.op_begin() + 1;
    for (; it != value.op_end(); ++it)
    {
        const llvm::ConstantInt &constant =
            llvmCast<llvm::ConstantInt>(**it);

        CANAL_ASSERT_MSG(constant.getBitWidth() <= 64,
                         "Cannot handle GetElementPtr offset"
                         " with more than 64 bits.");

        // Convert to 64-bit if necessary.
        llvm::APInt extended = APIntUtils::sext(constant.getValue(), 64);
        offsets.push_back(new Integer::Container(mEnvironment, extended));
    }

    const llvm::PointerType &pointerType =
        llvmCast<const llvm::PointerType>(*value.getType());

    // GetElementPtr on a Pointer
    const Pointer::Pointer *pointer =
        dynCast<const Pointer::Pointer*>(&variable);

    if (pointer)
    {
        return pointer->getElementPtr(offsets,
                                      *pointerType.getElementType());
    }

    // GetElementPtr on anything except a pointer.  For example, it is
    // called on arrays and structures.
    Domain *result;
    result = createPointer(*pointerType.getElementType());

    Pointer::Utils::addTarget(*result,
                              Pointer::Target::Block,
                              &place,
                              *value.op_begin(),
                              offsets,
                              NULL);

    return result;
}

Domain *
Constructors::createBitCast(const llvm::ConstantExpr &value,
                            const Domain &variable,
                            const llvm::Value &place) const
{
    // BitCast from Pointer.  It is always a bitcast to some other
    // pointer.
    const Pointer::Pointer *pointer =
        dynCast<const Pointer::Pointer*>(&variable);

    const llvm::PointerType *pointerType =
        llvmCast<const llvm::PointerType>(value.getType());

    if (pointer)
    {
        CANAL_ASSERT(pointerType);
        return pointer->bitcast(*pointerType->getElementType());
    }

    // BitCast from anything to a pointer.
    if (pointerType)
    {
        Domain *result;
        result = createPointer(*pointerType->getElementType());

        Pointer::Utils::addTarget(*result,
                                  Pointer::Target::Block,
                                  &place,
                                  *value.op_begin(),
                                  std::vector<Domain*>(),
                                  NULL);

        return result;
    }

    // BitCast from non-pointer to another non-pointer.
    CANAL_NOT_IMPLEMENTED();
}

} // namespace Canal
