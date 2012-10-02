#include "CommandDump.h"
#include "State.h"
#include "Commands.h"
#include <llvm/Module.h>
#include <cstdio>

CommandDump::CommandDump(Commands &commands)
    : Command("dump",
              "",
              "Dump the state of the program interpretation to a file",
              "Dump the state of the program interpretation to a file",
              commands)
{
}

std::vector<std::string>
CommandDump::getCompletionMatches(const std::vector<std::string> &args,
                                  int pointArg,
                                  int pointArgOffset) const
{
    std::vector<std::string> result;
    if (pointArg > 1)
        return result;

    std::string arg = args[pointArg].substr(0, pointArgOffset);

    // TODO: file completion here

    return result;
}

void
CommandDump::run(const std::vector<std::string> &args)
{
    if (args.size() > 2)
    {
        puts("Too many parameters.");
        return;
    }

    if (args.size() < 2)
    {
        mCommands.executeLine("help dump");
        return;
    }

    if (!mCommands.getState())
    {
        puts("No program is loaded.  Load a program first.");
        return;
    }

    FILE *fp = fopen(args[1].c_str(), "w");
    fprintf(fp, "%s", mCommands.getState()->getInterpreter().getModule().toString().c_str());
    fclose(fp);

    puts("Interpretation state saved.");
}
