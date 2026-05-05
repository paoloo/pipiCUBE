// Reduced CmdDispatcher table sizes for RP2040.
// Default DISPATCH_TABLE_SIZE=150 wastes ~3.5 KB on a 264 KB system;
// 30 entries covers all registered commands with margin.
#ifndef CMDDISPATCHER_COMMANDDISPATCHERIMPLCFG_HPP_
#define CMDDISPATCHER_COMMANDDISPATCHERIMPLCFG_HPP_

enum {
    CMD_DISPATCHER_DISPATCH_TABLE_SIZE   = 30,
    CMD_DISPATCHER_SEQUENCER_TABLE_SIZE  = 5,
};

#endif
