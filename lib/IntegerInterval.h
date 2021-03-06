#ifndef LIBCANAL_INTEGER_INTERVAL_H
#define LIBCANAL_INTEGER_INTERVAL_H

#include "Domain.h"

namespace Canal {
namespace Integer {

/// Abstracts integer values as a interval min - max.
class Interval : public Domain
{
public:

    bool mSignedBottom;
    bool mSignedTop;
    /// The number is included in the interval.
    llvm::APInt mSignedFrom;
    /// The number is included in the interval.
    llvm::APInt mSignedTo;

    bool mUnsignedBottom;
    bool mUnsignedTop;
    /// The number is included in the interval.
    llvm::APInt mUnsignedFrom;
    /// The number is included in the interval.
    llvm::APInt mUnsignedTo;

public:
    /// @brief Standard constructor.
    ///
    /// Initializes an empty interval.
    Interval(const Environment &environment,
             unsigned bitWidth);

    /// @brief Standard constructor.
    Interval(const Environment &environment,
             const llvm::APInt &constant);

    /// @brief Standard constructor.
    Interval(const Environment &environment,
             const llvm::APInt &from,
             const llvm::APInt &to);

    /// Copy constructor.
    Interval(const Interval &value);

    unsigned getBitWidth() const
    {
        return mSignedFrom.getBitWidth();
    }

    /// Lowest signed number represented by this abstract domain.
    /// @param result
    ///   Filled by the minimum value if it is known.  Otherwise, the
    ///   value is undefined.
    /// @return
    ///   True if the result is known and the parameter was set to
    ///   correct value.
    bool signedMin(llvm::APInt &result) const;

    /// Highest signed number represented by this abstract domain.
    /// @param result
    ///   Filled by the maximum value if it is known.  Otherwise, the
    ///   value is undefined.
    /// @return
    ///   True if the result is known and the parameter was set to
    ///   correct value.
    bool signedMax(llvm::APInt &result) const;

    /// Lowest unsigned number represented by this abstract domain.
    /// @param result
    ///   Filled by the minimum value if it is known.  Otherwise, the
    ///   value is undefined.
    /// @return
    ///   True if the result is known and the parameter was set to
    ///   correct value.
    bool unsignedMin(llvm::APInt &result) const;

    /// Highest unsigned number represented by this abstract domain.
    /// @param result
    ///   Filled by the maximum value if it is known.  Otherwise, the
    ///   value is undefined.
    /// @return
    ///   True if the result is known and the parameter was set to
    ///   correct value.
    bool unsignedMax(llvm::APInt &result) const;

    /// Returns true if the interval represents a single number.  Signed
    /// and unsigned representations might differ, though.
    bool isConstant() const;

    /// Returns true if the interval represents a signed single value.
    bool isSignedConstant() const;

    /// Returns true if the interval represents a unsigned single value.
    bool isUnsignedConstant() const;

    /// Does the interval represent signle bit that is set to 1?
    bool isTrue() const;

    /// Does the interval represent signle bit that is set to 0?
    bool isFalse() const;

    /// Set signed top
    void setSignedTop();

    /// Set signed bottom
    void setSignedBottom();

    /// Is signed bottom?
    bool isSignedBottom() const;

    /// Is signed top?
    bool isSignedTop() const;

    /// Set signed flags (bottom and top) to false; therefore prepare signed part to set values
    void resetSignedFlags();

    /// Set unsigned top
    void setUnsignedTop();

    /// Set unsigned bottom
    void setUnsignedBottom();

    /// Is unsigned bottom?
    bool isUnsignedBottom() const;

    /// Is unsigned top?
    bool isUnsignedTop() const;

    /// Set unsigned flags (bottom and top) to false; therefore prepare unsigned part to set values
    void resetUnsignedFlags();

    /// Reset signed and unsigned flags
    void resetFlags();

    static bool classof(const Domain *value)
    {
        return value->getKind() == IntegerIntervalKind;
    }

    /// Support method for urem
    void urem_any_result(const Interval& divisor);

    /// Support method for srem
    void srem_any_result(const Interval& dividend, const llvm::APInt &largerDivisor);

public: // Implementation of Domain.
    /// Covariant return type.
    virtual Interval *clone() const;

    virtual size_t memoryUsage() const;

    virtual std::string toString() const;

    virtual void setZero(const llvm::Value *place);

    virtual bool operator==(const Domain& value) const;

    virtual bool operator<(const Domain &value) const;

    virtual Interval &join(const Domain &value);

    virtual Interval &meet(const Domain &value);

    virtual bool isBottom() const;

    virtual void setBottom();

    virtual bool isTop() const;

    virtual void setTop();

    virtual float accuracy() const;

    virtual Interval &add(const Domain &a, const Domain &b);

    virtual Interval &sub(const Domain &a, const Domain &b);

    virtual Interval &mul(const Domain &a, const Domain &b);

    virtual Interval &udiv(const Domain &a, const Domain &b);

    virtual Interval &sdiv(const Domain &a, const Domain &b);

    virtual Interval &urem(const Domain &a, const Domain &b);

    virtual Interval &srem(const Domain &a, const Domain &b);

    virtual Interval &shl(const Domain &a, const Domain &b);

    virtual Interval &lshr(const Domain &a, const Domain &b);

    virtual Interval &ashr(const Domain &a, const Domain &b);

    virtual Interval &and_(const Domain &a, const Domain &b);

    virtual Interval &or_(const Domain &a, const Domain &b);

    virtual Interval &xor_(const Domain &a, const Domain &b);

    virtual Interval &icmp(const Domain &a, const Domain &b,
                           llvm::CmpInst::Predicate predicate);

    virtual Interval &fcmp(const Domain &a, const Domain &b,
                           llvm::CmpInst::Predicate predicate);

    virtual Interval &trunc(const Domain &value);

    virtual Interval &zext(const Domain &value);

    virtual Interval &sext(const Domain &value);

    virtual Interval &fptoui(const Domain &value);

    virtual Interval &fptosi(const Domain &value);

public: // Product
    virtual void extract(Product::Message& message) const;
    virtual void refine(const Product::Message& message);

    /// Covariant return type.
    virtual const llvm::IntegerType &getValueType() const;
};

} // namespace Integer
} // namespace Canal

#endif // LIBCANAL_INTEGER_INTERVAL_H
