#include "IntegerBits.h"
#include "Constant.h"
#include "Utils.h"
#include <sstream>

namespace Canal {
namespace Integer {

Bits::Bits(unsigned numBits) : mBits0(numBits, 0), mBits1(numBits, 0)
{
}

Bits::Bits(const llvm::APInt &number) : mBits0(~number), mBits1(number)
{
}

int
Bits::getBitValue(unsigned pos) const
{
#if (LLVM_MAJOR == 2 && LLVM_MINOR < 9)
    // Old interface replaced in LLVM 2.9.
    llvm::APInt bit(llvm::APInt::getBitsSet(mBits0.getBitWidth(), pos, pos + 1));
#else
    llvm::APInt bit(llvm::APInt::getOneBitSet(mBits0.getBitWidth(), pos));
#endif
    if ((mBits1 & bit).getBoolValue())
        return (mBits0 & bit).getBoolValue() ? 2 : 1;
    else
        return (mBits0 & bit).getBoolValue() ? 0 : -1;
}

void
Bits::setBitValue(unsigned pos, int value)
{
#if (LLVM_MAJOR == 2 && LLVM_MINOR < 9)
    // Old interface replaced in LLVM 2.9.
    llvm::APInt bit(llvm::APInt::getBitsSet(mBits0.getBitWidth(), pos, pos + 1));
#else
    llvm::APInt bit(llvm::APInt::getOneBitSet(mBits0.getBitWidth(), pos));
#endif
    switch (value)
    {
    case -1:
        mBits0 &= ~bit;
        mBits1 &= ~bit;
        break;
    case 0:
        mBits0 |= bit;
        mBits1 &= ~bit;
        break;
    case 1:
        mBits0 &= ~bit;
        mBits1 |= bit;
        break;
    case 2:
        mBits0 |= bit;
        mBits1 |= bit;
        break;
    default:
        CANAL_DIE();
    }
}

bool
Bits::signedMin(llvm::APInt &result) const
{
    CANAL_ASSERT_MSG(result.getBitWidth() == getBitWidth(),
                     "The bit width must be the same.");

    // Not available on LLVM 2.8:
    //result.clearAllBits();
    result.clear(result.getBitWidth());

    for (int i = 0; i < getBitWidth(); ++i)
    {
        switch (getBitValue(i))
        {
        case -1:
            // A bit has unknown (undefined) value.
            return false;
        case 0:
            break;
        case 1:
            // Not available on LLVM 2.8:
            //result.setBit(i);
            result.set(i);
            break;
        case 2:
            // If not sign bit...
            if (i != getBitWidth() - 1)
                result.set(i);
            break;
        default:
            CANAL_DIE();
        }
    }

    return true;
}

bool
Bits::signedMax(llvm::APInt &result) const
{
    CANAL_ASSERT_MSG(result.getBitWidth() == getBitWidth(),
                     "The bit width must be the same.");

    result.clear(result.getBitWidth());

    for (int i = 0; i < getBitWidth(); ++i)
    {
        switch (getBitValue(i))
        {
        case -1:
            // A bit has unknown (undefined) value.
            return false;
        case 0:
            break;
        case 1:
            result.set(i);
            break;
        case 2:
            // If sign bit...
            if (i == getBitWidth() - 1)
                result.set(i);
            break;
        default:
            CANAL_DIE();
        }
    }

    return true;
}

bool
Bits::unsignedMin(llvm::APInt &result) const
{
    CANAL_ASSERT_MSG(result.getBitWidth() == getBitWidth(),
                     "The bit width must be the same.");

    result.clear(result.getBitWidth());

    for (int i = 0; i < getBitWidth(); ++i)
    {
        switch (getBitValue(i))
        {
        case -1:
            // A bit has unknown (undefined) value.
            return false;
        case 0:
        case 2: // We choose 0 when both 0 and 1 are available...
            break;
        case 1:
            result.set(i);
            break;
        default:
            CANAL_DIE();
        }
    }

    return true;
}

bool
Bits::unsignedMax(llvm::APInt &result) const
{
    CANAL_ASSERT_MSG(result.getBitWidth() == getBitWidth(),
                     "The bit width must be the same.");

    result.clear(result.getBitWidth());

    for (int i = 0; i < getBitWidth(); ++i)
    {
        switch (getBitValue(i))
        {
        case -1:
            // A bit has unknown (undefined) value.
            return false;
        case 0:
            break;
        case 1:
        case 2: // We choose 1 when both 0 and 1 are available...
            result.set(i);
            break;
        default:
            CANAL_DIE();
        }
    }

    return true;
}

Bits *
Bits::clone() const
{
    return new Bits(*this);
}

Bits *
Bits::cloneCleaned() const
{
    return new Bits(getBitWidth());
}

bool
Bits::operator==(const Value& value) const
{
    const Bits *other = dynCast<const Bits*>(&value);
    if (!other)
        return false;
    return mBits0 == other->mBits0 && mBits1 == other->mBits1;
}

void
Bits::merge(const Value &value)
{
    const Constant *constant = dynCast<const Constant*>(&value);
    if (constant)
    {
        CANAL_ASSERT(constant->isAPInt());
        mBits0 |= ~constant->getAPInt();
        mBits1 |= constant->getAPInt();
        return;
    }

    const Bits &bits = dynCast<const Bits&>(value);
    mBits0 |= bits.mBits0;
    mBits1 |= bits.mBits1;
}

size_t
Bits::memoryUsage() const
{
    return sizeof(Bits);
}

std::string
Bits::toString() const
{
    std::stringstream ss;
    ss << "bits ";
    for (int pos = 0; pos < mBits0.getBitWidth(); ++pos)
    {
        switch (getBitValue(pos))
        {
        case -1: ss << "_"; break;
        case  0: ss << "0"; break;
        case  1: ss << "1"; break;
        case  2: ss << "T"; break;
        default: CANAL_DIE();
        }
    }
    ss << std::endl;
    return ss.str();
}

bool
Bits::matchesString(const std::string &text) const
{
}

void
Bits::add(const Value &a, const Value &b)
{
    setTop();
}

void
Bits::sub(const Value &a, const Value &b)
{
    setTop();
}

void
Bits::mul(const Value &a, const Value &b)
{
    setTop();
}

void
Bits::udiv(const Value &a, const Value &b)
{
    setTop();
}

void
Bits::sdiv(const Value &a, const Value &b)
{
    setTop();
}

void
Bits::urem(const Value &a, const Value &b)
{
    setTop();
}

void
Bits::srem(const Value &a, const Value &b)
{
    setTop();
}

void
Bits::shl(const Value &a, const Value &b)
{
    setTop();
}

void
Bits::lshr(const Value &a, const Value &b)
{
    setTop();
}

void
Bits::ashr(const Value &a, const Value &b)
{
    setTop();
}

static void
bitOperation(Bits &result,
             const Value &a,
             const Value &b,
             int(*operation)(int,int))
{
    const Bits &aa = dynCast<const Bits&>(a),
        &bb = dynCast<const Bits&>(b);
    CANAL_ASSERT(result.getBitWidth() == aa.getBitWidth() &&
                 aa.getBitWidth() == bb.getBitWidth());
    for (int pos = 0; pos < aa.getBitWidth(); ++pos)
    {
        result.setBitValue(pos, operation(aa.getBitValue(pos),
                                          bb.getBitValue(pos)));
    }
}

// First number in a pair is mBits1, second is mBits0
// 00 and 00 = 00
// 00 and 01 = 01
// 00 and 10 = 00
// 00 and 11 = 11
// 10 and 01 = 01
// 10 and 10 = 10
// 10 and 11 = 11
// 01 and 01 = 01
// 01 and 11 = 01
// 11 and 11 = 11
static int
bitAnd(int valueA, int valueB)
{
    if (valueA == 0 || valueB == 0)
        return 0;
    else if (valueA == 2 || valueB == 2)
        return 2;
    else
        return (valueA == -1 || valueB == -1) ? -1 : 1;
}

void
Bits::and_(const Value &a, const Value &b)
{
    bitOperation(*this, a, b, bitAnd);
}

// First number in a pair is mBits1, second is mBits0
// 00 or 00 = 00
// 00 or 01 = 00
// 00 or 10 = 10
// 00 or 11 = 11
// 10 or 01 = 10
// 10 or 10 = 10
// 10 or 11 = 10
// 01 or 01 = 01
// 01 or 11 = 11
// 11 or 11 = 11
static int
bitOr(int valueA, int valueB)
{
    if ((valueA == 0 || valueA == 1) && (valueB == 0 || valueB == 1))
        return (valueA || valueB) ? 1 : 0;
    else if (valueA == 2 || valueB == 2)
        return 2;
    else
        return (valueA == 1 || valueB == 1) ? 1 : -1;
}

void
Bits::or_(const Value &a, const Value &b)
{
    bitOperation(*this, a, b, bitOr);
}

// First number in a pair is mBits1, second is mBits0
// 00 xor 00 = 00
// 00 xor 01 = 00
// 00 xor 10 = 10
// 00 xor 11 = 11
// 10 xor 01 = 10
// 10 xor 10 = 01
// 10 xor 11 = 11
// 01 xor 01 = 01
// 01 xor 11 = 11
// 11 xor 11 = 11
static int
bitXor(int valueA, int valueB)
{
    if ((valueA == 0 || valueA == 1) && (valueB == 0 || valueB == 1))
        return (valueA xor valueB) ? 1 : 0;
    else if (valueA == 2 || valueB == 2)
        return 2;
    else
        return (valueA == 1 || valueB == 1) ? 1 : -1;
}

void
Bits::xor_(const Value &a, const Value &b)
{
    bitOperation(*this, a, b, bitXor);
}

void
Bits::icmp(const Value &a, const Value &b,
           llvm::CmpInst::Predicate predicate)
{
    CANAL_NOT_IMPLEMENTED();
}

float
Bits::accuracy() const
{
    int variableBits = 0;
    for (int pos = 0; pos < getBitWidth(); ++pos)
    {
        if (getBitValue(pos) == 2)
            ++variableBits;
    }

    return 1.0 - (variableBits / (float)getBitWidth());
}

bool
Bits::isBottom() const
{
    return mBits0.countPopulation() == 0 &&
        mBits1.countPopulation() == 0;
}

void
Bits::setBottom()
{
    mBits0 = mBits1 = 0;
}

bool
Bits::isTop() const
{
    return mBits0.countPopulation() == mBits0.getBitWidth() &&
        mBits1.countPopulation() == mBits1.getBitWidth();
}

void
Bits::setTop()
{
#if (LLVM_MAJOR == 2 && LLVM_MINOR < 9)
    // Old interface replaced in LLVM 2.9.
    mBits0.set();
    mBits1.set();
#else
    mBits0.setAllBits();
    mBits1.setAllBits();
#endif
}

} // namespace Integer
} // namespace Canal
