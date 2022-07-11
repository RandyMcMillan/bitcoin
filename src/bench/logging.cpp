// Copyright (c) 2020-2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <bench/bench.h>
#include <logging.h>
#include <test/util/setup_common.h>


static void Logging(benchmark::Bench& bench, const std::vector<const char*>& extra_args, const std::function<void()>& log)
{
    // Reset any enabled logging categories from a previous benchmark run.
    LogInstance().DisableCategory(BCLog::LogFlags::ALL);

    TestingSetup test_setup{
        CBaseChainParams::REGTEST,
        extra_args,
    };

    bench.run([&] { log(); });
}

static void LogPrintWithCategory(benchmark::Bench& bench)
{
    Logging(bench, {"-logthreadnames=0", "-debug=net"}, [] { LogPrint(BCLog::NET, "%s\n", "test"); });
}

static void LogPrintWithoutCategory(benchmark::Bench& bench)
{
    Logging(bench, {"-logthreadnames=0", "-debug=0"}, [] { LogPrint(BCLog::NET, "%s\n", "test"); });
}

static void LogPrintfWithThreadNames(benchmark::Bench& bench)
{
    Logging(bench, {"-logthreadnames=1"}, [] { LogPrintf("%s\n", "test"); });
}

static void LogPrintfWithoutThreadNames(benchmark::Bench& bench)
{
    Logging(bench, {"-logthreadnames=0"}, [] { LogPrintf("%s\n", "test"); });
}

static void LogWithoutWriteToFile(benchmark::Bench& bench)
{
    Logging(bench, {"-nodebuglogfile", "-debug=1"}, [] {
        LogPrintf("%s\n", "test");
        LogPrint(BCLog::NET, "%s\n", "test");
    });
}

BENCHMARK(LogPrintWithCategory, benchmark::PriorityLevel::HIGH);
BENCHMARK(LogPrintWithoutCategory, benchmark::PriorityLevel::HIGH);
BENCHMARK(LogPrintfWithThreadNames, benchmark::PriorityLevel::HIGH);
BENCHMARK(LogPrintfWithoutThreadNames, benchmark::PriorityLevel::HIGH);
BENCHMARK(LogWithoutWriteToFile, benchmark::PriorityLevel::HIGH);
