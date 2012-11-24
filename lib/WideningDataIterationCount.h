#ifndef LIBCANAL_WIDENING_DATA_ITERATION_COUNT_H
#define LIBCANAL_WIDENING_DATA_ITERATION_COUNT_H

#include "WideningDataInterface.h"
#include <map>

namespace Canal {
namespace Widening {

class DataIterationCount : public DataInterface
{
    std::map<const llvm::BasicBlock*, int> mCounts;

public:
    void increase(const llvm::BasicBlock &block);

    int count(const llvm::BasicBlock &block) const;

    virtual DataIterationCount *clone() const;

    static int ITERATION_COUNT;
};

} // namespace Widening
} // namespace Canal

#endif // LIBCANAL_WIDENING_DATA_ITERATION_COUNT_H
