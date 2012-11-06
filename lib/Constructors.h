#ifndef LIBCANAL_CONSTRUCTORS_H
#define LIBCANAL_CONSTRUCTORS_H

namespace llvm {
class Type;
class Constant;
class ConstantExpr;
class Value;
class APInt;
struct fltSemantics;
} // namespace llvm

namespace Canal {

class Domain;
class Environment;
class State;

class Constructors
{
protected:
    const Environment &mEnvironment;

public:
    Constructors(const Environment &environment);

    const Environment &getEnvironment() const { return mEnvironment; }

    Domain *create(const llvm::Type &type) const;

    /// @param state
    ///   State is used only for constant expressions such as
    ///   getelementptr and bitcast.  For other types of constants it
    ///   might be NULL.
    /// @returns
    ///   Returns a newly allocated value or NULL.  Caller takes
    ///   ownership of the returned value.
    Domain *create(const llvm::Constant &value,
                   const llvm::Value &place,
                   const State *state) const;

    Domain *createInteger(unsigned bitWidth) const;

    Domain *createInteger(const llvm::APInt &number) const;

    static const llvm::fltSemantics *
        getFloatingPointSemantics(const llvm::Type &type);

protected:
    Domain *createGetElementPtr(const llvm::ConstantExpr &value,
                                const Domain &variable,
                                const llvm::Value &place) const;

    Domain *createBitCast(const llvm::ConstantExpr &value,
                          const Domain &variable,
                          const llvm::Value &place) const;
};

} // namespace Canal

#endif // LIBCANAL_CONSTRUCTORS_H
