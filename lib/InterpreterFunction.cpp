#include "InterpreterFunction.h"
#include "InterpreterBasicBlock.h"
#include "Constructors.h"
#include "Environment.h"
#include "Domain.h"
#include "Utils.h"
#include <sstream>

namespace Canal {
namespace Interpreter {

Function::Function(const llvm::Function &function,
                   const Constructors &constructors)
    : mFunction(function), mEnvironment(constructors.getEnvironment())
{
    // Initialize input state.
    {
        llvm::Function::const_arg_iterator it = function.arg_begin(),
            itend = function.arg_end();

        for (; it != itend; ++it)
        {
            Domain *argument = constructors.create(*it->getType());
            mInputState.addFunctionVariable(*it, argument);
        }
    }

    // Initialize basic blocks.
    {
        llvm::Function::const_iterator it = function.begin(),
            itend = function.end();

        for (; it != itend; ++it)
            mBasicBlocks.push_back(new BasicBlock(*it, constructors));
    }

    // Initialize output state.
    const llvm::Type *returnType = mFunction.getReturnType();
    if (!returnType->isVoidTy())
        mOutputState.setReturnedValue(constructors.create(*returnType));
}

Function::~Function()
{
    std::vector<BasicBlock*>::const_iterator it = mBasicBlocks.begin();
    for (; it != mBasicBlocks.end(); ++it)
        delete *it;
}

const llvm::BasicBlock &
Function::getLlvmEntryBlock() const
{
    return mFunction.getEntryBlock();
}

BasicBlock &
Function::getBasicBlock(const llvm::BasicBlock &llvmBasicBlock) const
{
    std::vector<BasicBlock*>::const_iterator it = mBasicBlocks.begin();
    for (; it != mBasicBlocks.end(); ++it)
    {
        if (&(*it)->getLlvmBasicBlock() == &llvmBasicBlock)
            return **it;
    }

    CANAL_FATAL_ERROR("Failed to find certain basic block.");
}

llvm::StringRef
Function::getName() const
{
    return mFunction.getName();
}

void
Function::initializeInputState(BasicBlock &basicBlock, State &state) const
{
    const llvm::BasicBlock &llvmBasicBlock = basicBlock.getLlvmBasicBlock();
    state.merge(basicBlock.getInputState());

    // Merge out states of predecessors to input state of
    // current block.
    llvm::const_pred_iterator it = llvm::pred_begin(&llvmBasicBlock),
        itend = llvm::pred_end(&llvmBasicBlock);

    for (; it != itend; ++it)
    {
        BasicBlock &predBlock = getBasicBlock(**it);
        state.merge(predBlock.getOutputState());
    }

    if (&llvmBasicBlock == &getLlvmEntryBlock())
        state.merge(mInputState);
}

void
Function::updateOutputState()
{
    std::vector<BasicBlock*>::const_iterator it = mBasicBlocks.begin();
    for (; it != mBasicBlocks.end(); ++it)
    {
        if (!llvm::isa<llvm::ReturnInst>((*it)->getLlvmBasicBlock().getTerminator()))
            continue;

        // Merge global blocks, global variables.  Merge function
        // blocks that do not belong to this function.  Merge returned
        // value.
        mOutputState.mergeGlobal((*it)->getOutputState());
        mOutputState.mergeReturnedValue((*it)->getOutputState());
        mOutputState.mergeForeignFunctionBlocks((*it)->getOutputState(),
                                                mFunction);
    }
}

size_t
Function::memoryUsage() const
{
    size_t result = sizeof(Function) - 2 * sizeof(State);
    result += mInputState.memoryUsage();
    result += mOutputState.memoryUsage();
    std::vector<BasicBlock*>::const_iterator it = mBasicBlocks.begin();
    for (; it != mBasicBlocks.end(); ++it)
        result += (*it)->memoryUsage();

    return result;
}

std::string
Function::toString() const
{
    std::stringstream ss;

    ss << "*******************************************" << std::endl;
    ss << "** function " << mFunction.getName().str() << std::endl;
    ss << "*******************************************" << std::endl;
    ss << std::endl;

    // Print function arguments.
    {
        SlotTracker &slotTracker = mEnvironment.getSlotTracker();
        llvm::Function::ArgumentListType::const_iterator it =
            mFunction.getArgumentList().begin(),
            itend = mFunction.getArgumentList().end();

        for (; it != itend; ++it)
            ss << mInputState.toString(*it, slotTracker);

        if (mFunction.getArgumentList().begin() != itend)
            ss << std::endl;
    }

    // Print function result.
    if (!mFunction.getReturnType()->isVoidTy())
    {
        ss << "returnedValue = ";
        if (mOutputState.getReturnedValue())
        {
            ss << Canal::indentExceptFirstLine(
                mOutputState.getReturnedValue()->toString(),
                16);
        }
        else
            ss << "undefined" << std::endl;

        ss << std::endl;
    }

    // Print basic blocks.
    {
        std::vector<BasicBlock*>::const_iterator it = mBasicBlocks.begin();
        for (; it != mBasicBlocks.end(); ++it)
            ss << (*it)->toString();
    }

    return ss.str();
}

} // namespace Interpreter
} // namespace Canal

