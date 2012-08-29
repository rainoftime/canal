#include "lib/IntegerRange.h"
#include "lib/Utils.h"
#include "lib/Environment.h"
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/ManagedStatic.h>

using namespace Canal;

static void
testConstructors(const Environment &environment)
{
    Integer::Range range(environment, 1);
    CANAL_ASSERT(range.mEmpty && !range.mSignedTop && !range.mUnsignedTop);
    CANAL_ASSERT(range.mSignedFrom == range.mSignedTo);
    CANAL_ASSERT(range.mUnsignedFrom == range.mUnsignedTo);
    CANAL_ASSERT(range.mSignedFrom.getSExtValue() == 0);
    CANAL_ASSERT(range.mUnsignedFrom.getZExtValue() == 0);
}

static void
testSignedMin(const Environment &environment)
{
}

static void
testSignedMax(const Environment &environment)
{
}

static void
testUnsignedMin(const Environment &environment)
{
}

static void
testUnsignedMax(const Environment &environment)
{
}

static void
testIsSingleValue(const Environment &environment)
{
}

static void
testCloneCleaned(const Environment &environment)
{
}

static void
testEquality(const Environment &environment)
{
    Integer::Range range1(environment, 1),
        range2(environment, 1),
        range3(environment, 2);

    // Test empty ranges.
    CANAL_ASSERT(range1 == range2);
    CANAL_ASSERT(range1 != range3);
}

static void
testMerge(const Environment &environment)
{
}

static void
testMemoryUsage(const Environment &environment)
{
}

static void
testToString(const Environment &environment)
{
}

static void
testMatchesString(const Environment &environment)
{
}

static void
testAdd(const Environment &environment)
{
}

static void
testSub(const Environment &environment)
{
}

static void
testMul(const Environment &environment)
{
}

static void
testUDiv(const Environment &environment)
{
}

static void
testSDiv(const Environment &environment)
{
}

static void
testURem(const Environment &environment)
{
}

static void
testSRem(const Environment &environment)
{
}

static void
testShl(const Environment &environment)
{
}

static void
testLshr(const Environment &environment)
{
}

static void
testAshr(const Environment &environment)
{
}

static void
testAnd(const Environment &environment)
{
}

static void
testOr(const Environment &environment)
{
}

static void
testXor(const Environment &environment)
{
}

static void
testIcmp(const Environment &environment)
{
}

static void
testAccuracy(const Environment &environment)
{
}

static void
testIsBottom(const Environment &environment)
{
}

static void
testSetBottom(const Environment &environment)
{
}

static void
testIsTop(const Environment &environment)
{
}

static void
testSetTop(const Environment &environment)
{
}

int
main(int argc, char **argv)
{
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y;  // Call llvm_shutdown() on exit.

    llvm::Module module("testModule", context);
    Environment environment(module);

    testConstructors(environment);
    testSignedMin(environment);
    testSignedMax(environment);
    testUnsignedMin(environment);
    testUnsignedMax(environment);
    testIsSingleValue(environment);
    testCloneCleaned(environment);
    testEquality(environment);
    testMerge(environment);
    testMemoryUsage(environment);
    testToString(environment);
    testMatchesString(environment);
    testAdd(environment);
    testSub(environment);
    testMul(environment);
    testUDiv(environment);
    testSDiv(environment);
    testURem(environment);
    testSRem(environment);
    testShl(environment);
    testLshr(environment);
    testAshr(environment);
    testAnd(environment);
    testOr(environment);
    testXor(environment);
    testIcmp(environment);
    testAccuracy(environment);
    testIsBottom(environment);
    testSetBottom(environment);
    testIsTop(environment);
    testSetTop(environment);

    return 0;
}