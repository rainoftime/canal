#include "lib/Pointer.h"
#include "lib/Utils.h"
#include "lib/Environment.h"
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/ManagedStatic.h>

using namespace Canal;

static Environment *gEnvironment;

static void
testConstructors()
{
    const llvm::PointerType &type =
        *llvm::Type::getInt1PtrTy(gEnvironment->getContext());

    Pointer::InclusionBased pointer(*gEnvironment, type);
    CANAL_ASSERT(pointer.mTargets.size() == 0);
}

static void
testEquality()
{
    const llvm::PointerType &type =
        *llvm::Type::getInt1PtrTy(gEnvironment->getContext());

    Pointer::InclusionBased a(*gEnvironment, type);
    Pointer::InclusionBased b(*gEnvironment, type);

    // Test empty abstract pointers.
    assert(a.mTargets.size() == 0);
    assert(a == b);
    a.merge(b);
    assert(a.mTargets.size() == 0);
}

int
main(int argc, char **argv)
{
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y;  // Call llvm_shutdown() on exit.

    llvm::Module *module = new llvm::Module("testModule", context);
    gEnvironment = new Environment(module);

    testConstructors();
    testEquality();

    delete gEnvironment;
    return 0;
}
