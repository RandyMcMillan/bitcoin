// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <randomenv.h>

#include <crypto/sha512.h>
#include <support/cleanse.h>
#include <util/time.h> // for GetTime()
#ifdef WIN32
#include <compat.h> // for Windows API
#endif

#include <algorithm>
#include <thread>
#include <vector>

#include <stdint.h>
#include <string.h>
#ifndef WIN32
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#ifdef __MACH__
#include <mach/mach_time.h>
#endif

namespace {

void RandAddSeedPerfmon(CSHA512& hasher)
{
#ifdef WIN32
    // Don't need this on Linux, OpenSSL automatically uses /dev/urandom
    // Seed with the entire set of perfmon data

    // This can take up to 2 seconds, so only do it every 10 minutes
    static int64_t nLastPerfmon;
    if (GetTime() < nLastPerfmon + 10 * 60)
        return;
    nLastPerfmon = GetTime();

    std::vector<unsigned char> vData(250000, 0);
    long ret = 0;
    unsigned long nSize = 0;
    const size_t nMaxSize = 10000000; // Bail out at more than 10MB of performance data
    while (true) {
        nSize = vData.size();
        ret = RegQueryValueExA(HKEY_PERFORMANCE_DATA, "Global", nullptr, nullptr, vData.data(), &nSize);
        if (ret != ERROR_MORE_DATA || vData.size() >= nMaxSize)
            break;
        vData.resize(std::max((vData.size() * 3) / 2, nMaxSize)); // Grow size of buffer exponentially
    }
    RegCloseKey(HKEY_PERFORMANCE_DATA);
    if (ret == ERROR_SUCCESS) {
        hasher.Write(vData.data(), nSize);
        memory_cleanse(vData.data(), nSize);
    } else {
        // Performance data is only a best-effort attempt at improving the
        // situation when the OS randomness (and other sources) aren't
        // adequate. As a result, failure to read it is isn't considered critical,
        // so we don't call RandFailure().
        // TODO: Add logging when the logger is made functional before global
        // constructors have been invoked.
    }
#endif
}

/** Helper to easily feed data into a CSHA512.
 *
 * Note that this does not serialize the passed object (like stream.h's << operators do).
 * Its raw memory representation is used directly.
 */
template<typename T>
CSHA512& operator<<(CSHA512& hasher, const T& data) {
    hasher.Write((const unsigned char*)&data, sizeof(data));
    return hasher;
}

} // namespace

void RandAddDynamicEnv(CSHA512& hasher)
{
    RandAddSeedPerfmon(hasher);

#ifdef WIN32
    FILETIME ftime;
    GetSystemTimeAsFileTime(&ftime);
    hasher << ftime;
#else
#  ifdef __MACH__
    hasher << mach_absolute_time();
#  else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    hasher << ts.tv_sec << ts.tv_nsec;
#  endif
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    hasher << tv.tv_sec << tv.tv_usec;
#endif
}

void RandAddStaticEnv(CSHA512& hasher)
{
#ifdef WIN32
    hasher << GetCurrentProcessId() << GetCurrentThreadId();
#else
    hasher << getpid();
#endif
    hasher << std::this_thread::get_id();
}
