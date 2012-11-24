#define DEBUG
#include "lib/COW.h"
#include "lib/Utils.h"
#include <iostream>
#include "lib/IntegerBitfield.h"
#include "lib/Pointer.h"
#include "lib/Environment.h"
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/ManagedStatic.h>

using namespace Canal;

static void intConstTest() {
    typedef COWConst<int> type;
    type tmp1(0);
    const type& tmp1ref(tmp1);
#if 0 //No ptr functionality
    CANAL_ASSERT(*tmp1ref == 0);
    CANAL_ASSERT(*tmp1 == 0); //Should not force object copy
#endif
    CANAL_ASSERT(tmp1ref.getCounter() == 1);

    type tmp2(tmp1);
    const type& tmp2ref(tmp2);
#if 0
    CANAL_ASSERT(*tmp1ref == 0);
    CANAL_ASSERT(*tmp2ref == 0);
#endif

    CANAL_ASSERT(tmp1ref.getCounter() == 2);
    CANAL_ASSERT(tmp2ref.getCounter() == 2);
}

static void intTest() {
    typedef COW<int> type;
    type tmp1(0);
    const type& tmp1ref(tmp1);

    CANAL_ASSERT(*tmp1ref == 0);
    CANAL_ASSERT(*tmp1 == 0); //Should not force object copy
    CANAL_ASSERT(tmp1ref.getCounter() == 1);

    type tmp2(tmp1);
    const type& tmp2ref(tmp2);
    CANAL_ASSERT(*tmp1ref == 0);
    CANAL_ASSERT(*tmp2ref == 0);

    CANAL_ASSERT(tmp1ref.getCounter() == 2);
    CANAL_ASSERT(tmp2ref.getCounter() == 2);

    CANAL_ASSERT(tmp2 == 0); //Constant object copy
    CANAL_ASSERT(tmp1ref.getCounter() == 2);
    CANAL_ASSERT(tmp2ref.getCounter() == 2);

    CANAL_ASSERT(*tmp2 == 0); //Forces object copy
    CANAL_ASSERT(tmp1ref.getCounter() == 1);
    CANAL_ASSERT(tmp2ref.getCounter() == 1);

    CANAL_ASSERT(tmp1ref == 0);
    CANAL_ASSERT(tmp2ref == 0);
    CANAL_ASSERT(tmp1ref == tmp2ref);
}

static Integer::Bitfield
BitfieldFactory(const Environment &environment, int number)
{
    return Integer::Bitfield(environment, llvm::APInt(sizeof(int)*8, number, number < 0));
}

static void IntegerBitfieldTest(const Environment &environment) {
    typedef COW<Integer::Bitfield> type;
    type tmp1(BitfieldFactory(environment, 10));
    const type& tmp1ref = tmp1;
    const Integer::Bitfield& tmpBitfield1 = tmp1;

    Integer::Bitfield tmpBitfield2 = tmp1ref;

    CANAL_ASSERT(tmp1 == BitfieldFactory(environment, 10));
    CANAL_ASSERT(tmpBitfield1.getBitWidth() == sizeof(int)*8);
    CANAL_ASSERT(tmpBitfield2.getBitWidth() == sizeof(int)*8);
#if 0
    CANAL_ASSERT(tmp1->getBitWidth() == sizeof(int)*8); //Test of constant method call
    CANAL_ASSERT(tmp1ref->getBitWidth() == sizeof(int) * 8);
#endif
    //tmp1->setBitValue(0, 1); //Should not and does not work
    tmp1.modifiable().setBitValue(0, 1); //This does work
    CANAL_ASSERT(tmp1 == BitfieldFactory(environment, 11));
    //Test XOR - Domain method
    tmp1.xor_(BitfieldFactory(environment, 0), BitfieldFactory(environment, 1));
    CANAL_ASSERT(tmpBitfield2 != tmp1);
    CANAL_ASSERT(tmp1 == BitfieldFactory(environment, 1));
    //Test isTop, setTop
    CANAL_ASSERT(tmp1.isTop() == false);
    tmp1.setTop();
    CANAL_ASSERT(tmp1.isTop() == true);

    type tmp2(tmp1);
    const Integer::Bitfield& tmpBitfield23 = dynCast<const Integer::Bitfield&>(tmp1ref); //Should not force write
    CANAL_ASSERT(tmp1ref.getCounter() == 2);
    const Integer::Bitfield& tmpBitfield22 = dynCast<const Integer::Bitfield&>(tmp2); //Should not force write
    CANAL_ASSERT(tmp1ref.getCounter() == 2);
}

static void PointerTest(const Environment &environment) {
    const llvm::PointerType &ptrType =
        *llvm::Type::getInt1PtrTy(environment.getContext());
    typedef COW<Pointer::Pointer> type;
    typedef const Pointer::Pointer type_const;
    type tmp1(Pointer::Pointer(environment, ptrType));
    const type& tmp1ref = dynCast<type&>(tmp1);
    const Pointer::Pointer& tmpPointer1 = tmp1;
    const Pointer::Pointer* ptrPointer1 = tmp1;
    CANAL_ASSERT(tmp1 == Pointer::Pointer(environment, ptrType));
    CANAL_ASSERT(ptrPointer1->isSingleTarget() == false);
    CANAL_ASSERT(tmpPointer1.isSingleTarget() == false);
#if 0
    CANAL_ASSERT(tmp1->isSingleTarget() == false); //Test of constant method call
    CANAL_ASSERT(tmp1ref->isSingleTarget() == false);
#endif
    llvm::Value* val = new llvm::GlobalVariable(llvm::Type::getInt1PtrTy(environment.getContext()), true, llvm::GlobalValue::ExternalLinkage);
    //tmp1.setZero(val); //Should not and does not work
    tmp1.modifiable().setZero(val); //This does work
    CANAL_ASSERT(tmpPointer1.isSingleTarget() == true);
    const Pointer::Pointer& tmpPointer3 = tmp1;
    //Test merge - Domain method
    tmp1.merge(tmpPointer1);
    CANAL_ASSERT(tmpPointer3 == tmp1);
    delete val;
}

int main() {
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y;  // Call llvm_shutdown() on exit.

    llvm::Module *module = new llvm::Module("testModule", context);
    Environment environment(module);

    intConstTest();
    intTest();
    IntegerBitfieldTest(environment);
    PointerTest(environment);
}