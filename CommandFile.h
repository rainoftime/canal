#ifndef CANAL_COMMAND_FILE_H
#define CANAL_COMMAND_FILE_H

#include "Command.h"

namespace Canal
{
    class Interpreter;
}

class CommandFile : public Command
{
public:
    CommandFile(Commands &commands);

    // Implementation of Command::getCompletionMatches().
    virtual std::vector<std::string> getCompletionMatches(const std::vector<std::string> &args, int pointArg, int pointArgOffset) const;
    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> &args);

    // Might return NULL if no module is loaded.
    Canal::Interpreter *getInterpreter() { return mInterpreter; }

protected:
    Canal::Interpreter *mInterpreter;
};

#endif
