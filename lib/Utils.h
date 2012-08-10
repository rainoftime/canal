#ifndef LIBCANAL_UTILS_H
#define LIBCANAL_UTILS_H

#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Casting.h>
#include <cstdlib>
#include <string>

// Fatal error.  Writes a message to stderr and terminates the
// application.
//
// If you do not want to show any message, use CANAL_DIE instead.
#define CANAL_FATAL_ERROR(msg)                                   \
    {                                                            \
        llvm::errs() << __FILE__ << ":"                          \
                     << __LINE__ << "("                          \
                     << __FUNCTION__ << ") fatal error: "        \
                     << msg << "\n"                              \
                     << Canal::getCurrentBacktrace();            \
        exit(1);                                                 \
    }

// Error.  Writes a message to stderr.  Program continues to run.
#define CANAL_ERROR(msg)                                         \
    {                                                            \
        llvm::errs() << __FILE__ << ":"                          \
                     << __LINE__ << "("                          \
                     << __FUNCTION__ << ") error: "              \
                     << msg << "\n"                              \
                     << Canal::getCurrentBacktrace();            \
    }

// Assertion check.  On failure, the expression is written to stderr
// and the application is terminated.
#define CANAL_ASSERT(expr)                                              \
    if (expr) ;                                                         \
    else                                                                \
    {                                                                   \
        llvm::errs() << __FILE__ << ":"                                 \
                     << __LINE__ << "("                                 \
                     << __FUNCTION__ << ") assert failed: "             \
                     << #expr << "\n"                                   \
                     << Canal::getCurrentBacktrace();                   \
        exit(1);                                                        \
    }

// Assertion check.  On failuer, a message and the expression is
// written to stderr and the application is terminated.
#define CANAL_ASSERT_MSG(expr, msg)                                     \
    if (expr) ;                                                         \
    else                                                                \
    {                                                                   \
        llvm::errs() << __FILE__ << ":"                                 \
                     << __LINE__ << "("                                 \
                     << __FUNCTION__ << ") assert failed: "             \
                     << msg << " [" << #expr << "]\n"                   \
                     << Canal::getCurrentBacktrace();                   \
        exit(1);                                                        \
    }

// Termination.  The location where the program terminated is written
// to stderr and the application is terminated.
#define CANAL_DIE()                                                     \
    {                                                                   \
        llvm::errs() << __FILE__ << ":"                                 \
                     << __LINE__ << "("                                 \
                     << __FUNCTION__ << "): dead code location reached\n" \
                     << Canal::getCurrentBacktrace();                   \
        exit(1);                                                        \
    }

// Termination.  The location where the program terminated is written
// to stderr and the application is terminated.
#define CANAL_DIE_MSG(msg)                                              \
    {                                                                   \
        llvm::errs() << __FILE__ << ":"                                 \
                     << __LINE__ << "("                                 \
                     << __FUNCTION__ << "): dead code location reached" \
                     << ": " << msg << "\n"                             \
                     << Canal::getCurrentBacktrace();                   \
        exit(1);                                                        \
    }

// Report a function or a code block that is not implemented but it
// should be.  The location is written to stderr. The application
// continues to run.
#define CANAL_NOT_IMPLEMENTED()                                         \
    {                                                                   \
        llvm::errs() << __FILE__ << ":"                                 \
                     << __LINE__ << "("                                 \
                     << __FUNCTION__ << "): not implemented\n"          \
                     << Canal::getCurrentBacktrace();                   \
        exit(1);                                                        \
    }

namespace llvm {
class APInt;
class Constant;
class Type;
class Value;
} // namespace llvm

namespace Canal {

class SlotTracker;

std::string toString(const llvm::APInt &num);
std::string toString(const llvm::Type &type);
std::string toString(const llvm::Constant &constant);

std::string indent(const std::string &input, int spaces);
std::string indentExceptFirstLine(const std::string &input, int spaces);

// @param slotTracker
//   Slot Tracker with value's function assigned.
// @returns
//   Empty string when no name was found.
std::string getName(const llvm::Value &value,
                    SlotTracker &slotTracker);

std::string getCurrentBacktrace();

template <class X, class Y> inline typename llvm::cast_retty<X, Y>::ret_type
llvmCast(const Y &Val)
{
    CANAL_ASSERT_MSG(llvm::isa<X>(Val),
                     "cast<Ty>() argument of incompatible type!");
    return llvm::cast_convert_val<X, Y,
        typename llvm::simplify_type<Y>::SimpleType>::doit(Val);
}


} // namespace Canal

#endif // LIBCANAL_UTILS_H
