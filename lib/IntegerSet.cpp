#include "IntegerSet.h"
#include "Utils.h"
#include "FloatInterval.h"
#include "Environment.h"
#include "IntegerInterval.h"
#include "ProductMessage.h"
#include "FieldMinMax.h"

namespace Canal {
namespace Integer {

unsigned int Set::SET_THRESHOLD = 40;

Set::Set(const Environment &environment,
         unsigned bitWidth)
    : Domain(environment, Domain::IntegerSetKind),
      mTop(false),
      mBitWidth(bitWidth)
{
    CANAL_ASSERT(mBitWidth > 0);
}

Set::Set(const Environment &environment,
         const llvm::APInt &constant)
    : Domain(environment, Domain::IntegerSetKind),
      mTop(false),
      mBitWidth(constant.getBitWidth())
{
    mValues.insert(constant);
    CANAL_ASSERT(mBitWidth > 0);
}

Set::Set(const Set &value)
    : Domain(value),
      mValues(value.mValues),
      mTop(value.mTop),
      mBitWidth(value.mBitWidth)
{
}

bool
Set::signedMin(llvm::APInt &result) const
{
    if (mTop)
        result = llvm::APInt::getSignedMinValue(mBitWidth);
    else
    {
        if (mValues.empty())
            return false;

        // Find lowest negative number
        Utils::USet::const_iterator bound =
            mValues.lower_bound(llvm::APInt::getSignedMinValue(mBitWidth));

        if (bound == mValues.end())
        {
            // If there is no negative number in this set then
            // the first element in this set is lowest
            result = *mValues.begin();
        }
        else
            result = *bound;
    }

    return true;
}

bool
Set::signedMax(llvm::APInt &result) const
{
    if (mTop)
        result = llvm::APInt::getSignedMaxValue(mBitWidth);
    else
    {
        if (mValues.empty())
            return false;

        //Find lowest negative number
        Utils::USet::const_iterator bound = mValues.lower_bound(llvm::APInt::getSignedMinValue(mBitWidth));
        if (bound == mValues.end() || //If there is no negative number in this set
            bound == mValues.begin())
        { //or first element in this set is negative
            result = *mValues.rbegin(); //then the last element in this set is highest
        }
        else { //There are some positive numbers as well
            result = *(--bound); //then the highest number is the one directly preceeding lowest negative number
        }
    }

    return true;
}

bool
Set::unsignedMin(llvm::APInt &result) const
{
    if (mTop)
        result = llvm::APInt::getMinValue(mBitWidth);
    else
    {
        if (mValues.empty())
            return false;

        // We assume the set is sorted by unsigned comparison.
        result = *mValues.begin();
    }
    return true;
}

bool
Set::unsignedMax(llvm::APInt &result) const
{
    if (mTop)
        result = llvm::APInt::getMaxValue(mBitWidth);
    else
    {
        if (mValues.empty())
            return false;

        // We assume the set is sorted by unsigned comparison.
        result = *mValues.rbegin();
    }
    return true;
}

bool
Set::isConstant() const
{
    return (!mTop && mValues.size() == 1);
}

bool
Set::isTrue() const
{
    return isConstant() &&
        *mValues.begin() == llvm::APInt(1, 1);
}

bool
Set::isFalse() const
{
    return isConstant() &&
        *mValues.begin() == llvm::APInt(1, 0);
}

Set *
Set::clone() const
{
    return new Set(*this);
}

size_t
Set::memoryUsage() const
{
    size_t result = sizeof(Set);
    result += mValues.size() * sizeof(llvm::APInt);
    return result;
}

std::string
Set::toString() const
{
    StringStream ss;
    ss << "integerSet";
    if (mTop)
        ss << " top";
    else if (mValues.empty())
        ss << " empty";
    ss << "\n";

    Utils::USet::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        ss << "    " << Canal::toString(*it) << "\n";

    return ss.str();
}

void
Set::setZero(const llvm::Value *place)
{
    mTop = false;
    mValues.clear();
    mValues.insert(llvm::APInt::getNullValue(mBitWidth));
}

bool
Set::operator==(const Domain &value) const
{
    if (this == &value)
        return true;

    const Set &set = checkedCast<Set>(value);
    if (mTop != set.mTop)
        return false;

    if (mTop)
        return true;

    // Compare values only if the top is not set, otherwise we would
    // get false inequality.
    return mValues == set.mValues;
}

bool
Set::operator<(const Domain &value) const
{
    const Set &set = checkedCast<Set>(value);

    CANAL_ASSERT(getBitWidth() == set.getBitWidth());

    if (isTop()) return set.isTop();
    if (set.isTop()) return true;

    Utils::USet::iterator mine = mValues.begin(), other = set.mValues.begin();
    Utils::USet::key_compare comp = mValues.key_comp();

    for (; mine != mValues.end(); mine ++, other ++) {
        if (other == set.mValues.end()) return false;
        for (; *mine != *other && !comp(*mine, *other) ; other ++ ) {
            if (other == set.mValues.end()) return false;
        }
        if (*mine != *other) return false;
    }
    return true;
}

Set &
Set::join(const Domain &value)
{
    if (mTop)
        return *this;

    const Set &set = checkedCast<Set>(value);
    if (set.isTop())
        setTop();
    else
    {
        CANAL_ASSERT_MSG(set.getBitWidth() == getBitWidth(),
                         "Different bit width in merge: "
                         << set.getBitWidth()
                         << " bit value merged to "
                         << getBitWidth() << " bit value");

        mValues.insert(set.mValues.begin(),
                       set.mValues.end());

        if (mValues.size() > SET_THRESHOLD)
            setTop();
    }

    return *this;
}

Set &
Set::meet(const Domain &value)
{
    if (isBottom())
        return *this;

    if (value.isTop())
        return *this;

    if (value.isBottom())
    {
        setBottom();
        return *this;
    }

    const Set &set = checkedCast<Set>(value);
    CANAL_ASSERT(mBitWidth == set.mBitWidth);
    if (isTop()) {
        mTop = false;
        mValues = set.mValues;
    }
    else
    {
        Utils::USet intersection;
        std::set_intersection(mValues.begin(),
                              mValues.end(),
                              set.mValues.begin(),
                              set.mValues.end(),
                              std::inserter(intersection, intersection.end()),
                              Utils::UCompare());

        mValues = intersection;
    }

    return *this;
}

bool
Set::isBottom() const
{
    return mValues.empty() && !mTop;
}

void
Set::setBottom()
{
    mValues.clear();
    mTop = false;
}

bool
Set::isTop() const
{
    return mTop;
}

void
Set::setTop()
{
    mValues.clear();
    mTop = true;
}

float
Set::accuracy() const
{
    if (mTop)
        return 0;
    // Perfectly accurate.  TODO: consider lowering the accuracy
    // depending on the number of elements.
    return 1;
}

Set &
Set::add(const Domain &a, const Domain &b)
{
    return applyOperation(a, b, &llvm::APInt::operator+, NULL);
}

Set &
Set::sub(const Domain &a, const Domain &b)
{
    return applyOperation(a, b, &llvm::APInt::operator-, NULL);
}

Set &
Set::mul(const Domain &a, const Domain &b)
{
#if (LLVM_VERSION_MAJOR == 2 && LLVM_VERSION_MINOR < 9)
    return applyOperation(a, b, &llvm::APInt::operator*, NULL);
#else
    return applyOperation(a, b, NULL, &llvm::APInt::smul_ov);
#endif
}

Set &
Set::udiv(const Domain &a, const Domain &b)
{
    return applyOperationDivision(a, b, &llvm::APInt::udiv);
}

Set &
Set::sdiv(const Domain &a, const Domain &b)
{
    return applyOperationDivision(a, b, &llvm::APInt::sdiv);
}

Set &
Set::urem(const Domain &a, const Domain &b)
{
    return applyOperationDivision(a, b, &llvm::APInt::urem);
}

Set &
Set::srem(const Domain &a, const Domain &b)
{
    return applyOperationDivision(a, b, &llvm::APInt::srem);
}

Set &
Set::shl(const Domain &a, const Domain &b)
{
    return applyOperation(a, b, &llvm::APInt::shl, NULL);
}

Set &
Set::lshr(const Domain &a, const Domain &b)
{
    return applyOperation(a, b, &llvm::APInt::lshr, NULL);
}

Set &
Set::ashr(const Domain &a, const Domain &b)
{
    return applyOperation(a, b, &llvm::APInt::ashr, NULL);
}

Set &
Set::and_(const Domain &a, const Domain &b)
{
    return applyOperation(a, b, &llvm::APInt::operator&, NULL);
}

Set &
Set::or_(const Domain &a, const Domain &b)
{
    return applyOperation(a, b, &llvm::APInt::operator|, NULL);
}

Set &
Set::xor_(const Domain &a, const Domain &b)
{
    return applyOperation(a, b, &llvm::APInt::operator^, NULL);
}

static bool
intersects(const Set &a,
           const Set &b)
{
    // Signed and unsigned does not matter, it contains specific values
    Utils::USet::const_iterator ita = a.mValues.begin(),
        itb = b.mValues.begin();

    // Algorithm inspired by std::set_intersection
    while (ita != a.mValues.end() && itb != b.mValues.end())
    {
        if ((*ita).slt(*itb))
            ++ita;
        else if ((*itb).slt(*ita))
            ++itb;
        else
            return true;
    }

    return false;
}

Set &
Set::icmp(const Domain &a, const Domain &b,
                  llvm::CmpInst::Predicate predicate)
{
    const Set &aa = checkedCast<Set>(a),
        &bb = checkedCast<Set>(b);

    if (aa.isBottom() || bb.isBottom())
    {
        setBottom(); // Undefined
        return *this;
    }

    if (aa.isTop() || bb.isTop())
    {
        setTop(); // Could be both
        return *this;
    }

    setBottom();
    mBitWidth = 1;
    llvm::APInt minA, maxA, minB, maxB;
    aa.signedMin(minA);
    aa.signedMax(maxA);
    bb.signedMin(minB);
    bb.signedMax(maxB);
    //Assert: aa.getBitWidth() = bb.getBitWidth()
    switch (predicate)
    {
    case llvm::CmpInst::ICMP_EQ:  // equal
        // If both sets are equal, the result is 1.  If
        // set intersection is empty, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mValues.size() == 1 && aa.mValues == bb.mValues)
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (intersects(aa, bb))
            setTop();
        else
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));

        break;
    case llvm::CmpInst::ICMP_NE:  // not equal
        // If both sets are equal, the result is 0.  If
        // set intersection is empty, the result is 1.
        // Otherwise the result is the top value (both 0 and 1).
        if (!intersects(aa, bb))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (aa.mValues.size() == 1 && aa.mValues == bb.mValues)
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_UGT: // unsigned greater than
        // If the lowest element from the first set is
        // unsigned greater than the largest element from the second
        // set, the result is 1.  If the largest element from
        // the first set is unsigned lower than the lowest
        // element from the second set, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mValues.begin()->ugt(*bb.mValues.rbegin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (aa.mValues.rbegin()->ult(*bb.mValues.begin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_UGE: // unsigned greater or equal
        // If the largest element from the first set is
        // unsigned lower or equal the lowest element from the second
        // set, the result is 1.  If the lowest element from
        // the first set is unsigned larger or equal than the largest
        // element from the second set, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mValues.begin()->uge(*bb.mValues.rbegin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (aa.mValues.rbegin()->ule(*bb.mValues.begin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_ULT: // unsigned less than
        // If the largest element from the first set is
        // unsigned lower than the lowest element from the second
        // set, the result is 1.  If the lowest element from
        // the first set is unsigned larger than the largest
        // element from the second set, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mValues.rbegin()->ult(*bb.mValues.begin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (aa.mValues.begin()->ugt(*bb.mValues.rbegin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_ULE: // unsigned less or equal
        // If the largest element from the first set is
        // unsigned lower or equal the lowest element from the second
        // set, the result is 1.  If the lowest element from
        // the first set is unsigned larger or equal than the largest
        // element from the second set, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mValues.rbegin()->ule(*bb.mValues.begin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (aa.mValues.begin()->uge(*bb.mValues.rbegin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_SGT: // signed greater than
        // If the lowest element from the first set is
        // signed greater than the largest element from the second
        // set, the result is 1.  If the largest element from
        // the first set is signed lower than the lowest
        // element from the second set, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (minA.sgt(maxB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (maxA.slt(minB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_SGE: // signed greater or equal
        // If the lowest element from the first set is
        // signed greater or equal than the largest element from the second
        // set, the result is 1.  If the largest element from
        // the first set is signed lower or equal than the lowest
        // element from the second set, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (minA.sge(maxB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (maxA.sle(minB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_SLT: // signed less than
        // If the largest element from the first set is
        // signed lower than the lowest element from the second
        // set, the result is 1.  If the lowest element from
        // the first set is signed larger than the largest
        // element from the second set, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (maxA.slt(minB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (minA.sgt(maxB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_SLE: // signed less or equal
        // If the largest element from the first set is
        // signed lower or equal the lowest element from the second
        // set, the result is 1.  If the lowest element from
        // the first set is signed larger or equal than the largest
        // element from the second set, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (maxA.sle(minB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (minA.sge(maxB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    default:
        CANAL_DIE();
    }

    return *this;
}

Set &
Set::fcmp(const Domain &a,
          const Domain &b,
          llvm::CmpInst::Predicate predicate)
{
    const Float::Interval &aa = checkedCast<Float::Interval>(a),
        &bb = checkedCast<Float::Interval>(b);

    int result = aa.compare(bb, predicate);
    switch (result)
    {
    case -1:
        setBottom();
        break;
    case 0:
        mTop = false;
        mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        break;
    case 1:
        mTop = false;
        mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        break;
    case 2:
        setTop();
        break;
    default:
        CANAL_DIE();
    }

    return *this;
}

Set &
Set::trunc(const Domain &value)
{
    const Set &set = checkedCast<Set>(value);
    mTop = set.mTop;
    Utils::USet::const_iterator it = set.mValues.begin();
    for (; it != set.mValues.end(); ++it)
        mValues.insert(Utils::trunc(*it, getBitWidth()));

    return *this;
}

Set &
Set::zext(const Domain &value)
{
    const Set &set = checkedCast<Set>(value);
    mTop = set.mTop;
    Utils::USet::const_iterator it = set.mValues.begin();
    for (; it != set.mValues.end(); ++it)
        mValues.insert(Utils::zext(*it, getBitWidth()));

    return *this;
}

Set &
Set::sext(const Domain &value)
{
    const Set &set = checkedCast<Set>(value);
    mTop = set.mTop;
    Utils::USet::const_iterator it = set.mValues.begin();
    for (; it != set.mValues.end(); ++it)
        mValues.insert(Utils::sext(*it, getBitWidth()));

    return *this;
}

Set &
Set::fptoui(const Domain &value)
{
    Integer::Interval tmp = Interval(mEnvironment, getBitWidth());
    fromInterval(tmp.fptoui(value));
    return *this;
}

Set &
Set::fptosi(const Domain &value)
{
    Integer::Interval tmp = Interval(mEnvironment, getBitWidth());
    fromInterval(tmp.fptosi(value));
    return *this;
}

const llvm::IntegerType &
Set::getValueType() const
{
    return *llvm::Type::getIntNTy(mEnvironment.getContext(), getBitWidth());
}

Set &
Set::applyOperation(const Domain &a,
                    const Domain &b,
                    Utils::Operation operation1,
                    Utils::OperationWithOverflow operation2)
{
    const Set &aa = checkedCast<Set>(a),
        &bb = checkedCast<Set>(b);

    CANAL_ASSERT(this != &a && this != &b);
    setBottom();
    if (aa.isBottom() || bb.isBottom()) return *this;
    if (aa.isTop() || bb.isTop())
    {
        setTop();
        return *this;
    }

    CANAL_ASSERT(aa.getBitWidth() == bb.getBitWidth());
    Utils::USet::const_iterator aaIt = aa.mValues.begin();
    for (; aaIt != aa.mValues.end(); ++aaIt)
    {
        Utils::USet::const_iterator bbIt = bb.mValues.begin();
        for (; bbIt != bb.mValues.end(); ++bbIt)
        {
            if (operation1)
                mValues.insert(((*aaIt).*(operation1))(*bbIt));
            else
            {
                bool overflow;
                mValues.insert(((*aaIt).*(operation2))(*bbIt, overflow));
                if (overflow)
                {
                    setTop();
                    return *this;
                }
            }

            if (mValues.size() > SET_THRESHOLD)
            {
                setTop();
                return *this;
            }
        }
    }

    return *this;
}

Set &
Set::applyOperationDivision(const Domain &a,
                            const Domain &b,
                            Utils::Operation operation1)
{
    const Set &aa = checkedCast<Set>(a),
        &bb = checkedCast<Set>(b);

    CANAL_ASSERT(this != &a && this != &b);
    setBottom();
    if (aa.isBottom() || bb.isBottom()) return *this;
    if (aa.isTop() || bb.isTop())
    {
        setTop();
        return *this;
    }

    CANAL_ASSERT(aa.getBitWidth() == bb.getBitWidth());
    Utils::USet::const_iterator aaIt = aa.mValues.begin();

    if (bb.mValues.size() == 1 && *bb.mValues.begin() == 0)
    { //Only division by zero
        setTop();
        return *this;
    }

    for (; aaIt != aa.mValues.end(); ++aaIt)
    {
        Utils::USet::const_iterator bbIt = bb.mValues.begin();
        for (; bbIt != bb.mValues.end(); ++bbIt)
        {
            if (*bbIt == 0) continue; //Avoid division by zero
            mValues.insert(((*aaIt).*(operation1))(*bbIt));

            if (mValues.size() > SET_THRESHOLD)
            {
                setTop();
                return *this;
            }
        }
    }

    return *this;
}

Set &
Set::fromInterval(const Interval &interval) {
    mBitWidth = interval.getBitWidth();
    if (interval.isSignedBottom() || interval.isUnsignedBottom()) {
        setBottom();
    }
    else if (interval.isTop()){
        setTop();
    }
    else {
        mTop = false;
        mValues.clear();
        //Signed and unsigned are the same -> we store them ordered by unsigned comparator
        llvm::APInt from, to, diff;
        CANAL_ASSERT(interval.signedMin(from) && interval.signedMax(to));
        diff = to - from;
        if (diff.ult(SET_THRESHOLD)) { //Store every value
            for (; from.slt(to); ++from) {
                mValues.insert(from);
            }
            mValues.insert(from); //We need upper bound as well, but upper bound + 1 may not fit into bitwidth
        }
        else {
            CANAL_ASSERT(interval.unsignedMin(from) && interval.unsignedMax(to));
            diff = to - from;
            if (diff.ult(SET_THRESHOLD)) { //Store every value
                for (; from.ult(to); ++from) {
                    mValues.insert(from);
                }
                mValues.insert(from);
            }
            else setTop();
        }
    }
    return *this;
}

void
Set::extract(Product::Message& message) const
{
    Integer::Interval interval(mEnvironment, getBitWidth());
    interval.resetSignedFlags();
    signedMin(interval.mSignedFrom);
    signedMax(interval.mSignedTo);

    interval.resetUnsignedFlags();
    unsignedMin(interval.mUnsignedFrom);
    unsignedMax(interval.mUnsignedTo);

    Field::MinMax* minMax = new Field::MinMax(interval);
    message.mFields[Product::MessageField::FieldMinMaxKind] = minMax;
}


void
Set::refine(const Product::Message& message)
{
    Product::Message::const_iterator it = message.mFields.find(Product::MessageField::FieldMinMaxKind);
    if(it != message.mFields.end())
    {
        Field::MinMax* minMax = checkedCast<Field::MinMax>(it->second);
        Set set(mEnvironment, mBitWidth);
        set.fromInterval(*minMax->mInterval);
        meet(set);
    }
}


} // namespace Integer
} // namespace Canal
