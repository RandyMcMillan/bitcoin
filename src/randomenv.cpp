// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <randomenv.h>

#include <compat/cpuid.h>
#include <crypto/sha512.h>
#include <support/cleanse.h>
#include <util/time.h> // for GetTime()
#ifdef WIN32
#include <compat.h> // for Windows API
#endif

#include <algorithm>
#include <atomic>
#include <climits>
#include <thread>
#include <vector>

#include <stdint.h>
#include <string.h>
#ifndef WIN32
#include <sys/types.h> // must go before a number of other headers
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <unistd.h>
#endif
#ifdef __MACH__
#include <mach/mach_time.h>
#endif
#if HAVE_DECL_GETIFADDRS
#include <ifaddrs.h>
#endif
#if HAVE_DECL_SYSCTLBYNAME
#include <sys/sysctl.h>
#endif
#ifdef __linux__
#include <sys/auxv.h>
#endif

//! Necessary on some platforms
extern char** environ;

namespace {

void RandAddSeedPerfmon(CSHA512& hasher)
{
#ifdef WIN32
    // Don't need this on Linux, OpenSSL automatically uses /dev/urandom
    // Seed with the entire set of perfmon data

    // This can take up to 2 seconds, so only do it every 10 minutes
    static std::atomic<std::chrono::seconds> last_perfmon{std::chrono::seconds{0}};
    auto last_time = last_perfmon.load();
    auto current_time = GetTime<std::chrono::seconds>();
    if (current_time < last_time + std::chrono::minutes{10}) return;
    last_perfmon = current_time;

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

#ifndef WIN32
void AddSockaddr(CSHA512& hasher, const struct sockaddr *addr)
{
    if (addr == nullptr) return;
    switch (addr->sa_family) {
    case AF_INET:
        hasher.Write((const unsigned char*)addr, sizeof(sockaddr_in));
        break;
    case AF_INET6:
        hasher.Write((const unsigned char*)addr, sizeof(sockaddr_in6));
        break;
    default:
        hasher.Write((const unsigned char*)&addr->sa_family, sizeof(addr->sa_family));
    }
}

void AddStat(CSHA512& hasher, const struct stat& sb)
{
    hasher << sb.st_dev << sb.st_ino << sb.st_mode << sb.st_nlink << sb.st_uid << sb.st_gid << sb.st_size << sb.st_blksize << sb.st_blocks << sb.st_atime << sb.st_mtime << sb.st_ctime;
}

void AddFile(CSHA512& hasher, const char *path)
{
    struct stat sb;
    int f = open(path, O_RDONLY);
    if (f != -1) {
        unsigned char fbuf[1024];
        int n;
        hasher.Write((const unsigned char*)&f, sizeof(f));
        if (fstat(f, &sb) == 0) {
            AddStat(hasher, sb);
        }
        do {
            n = read(f, fbuf, sizeof(fbuf));
            if (n > 0) hasher.Write(fbuf, n);
            /* not bothering with EINTR handling. */
        } while (n == sizeof(fbuf));
        close(f);
    }
}

void AddPath(CSHA512& hasher, const char *path)
{
    struct stat sb;
    if (stat(path, &sb) == 0) {
        hasher.Write((const unsigned char*)path, strlen(path) + 1);
        AddStat(hasher, sb);
    }
}
#endif

void AddSysctl(CSHA512& hasher, const char* path)
{
#if HAVE_DECL_SYSCTLBYNAME
    unsigned char buffer[4096];
    size_t siz = 4096;
    int ret = sysctlbyname(path, buffer, &siz, nullptr, 0);
    if (ret == 0 || (ret == -1 && errno == ENOMEM)) {
        hasher.Write((const unsigned char*)path, strlen(path) + 1);
        hasher.Write(buffer, std::min<size_t>(siz, 4096));
    }
#endif
}

#ifdef HAVE_GETCPUID
void AddCPUID(CSHA512& hasher)
{
    uint32_t ax, bx, cx, dx;
    GetCPUID(0, 0, ax, bx, cx, dx);
    hasher << ax << bx << cx << dx;
    uint32_t max = ax;
    for (uint32_t leaf = 1; leaf <= max; ++leaf) {
        for (uint32_t subleaf = 0;; ++subleaf) {
            GetCPUID(leaf, subleaf, ax, bx, cx, dx);
            hasher << leaf << subleaf << ax << bx << cx << dx;
            // Iterate over subleaves for leaf 4, 11, 13
            if (leaf != 4 && leaf != 11 && leaf != 13) break;
            if ((leaf == 4 || leaf == 13) && ax == 0) break;
            if (leaf == 11 && (cx & 0xFF00) == 0) break;
        }
    }
}
#endif
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

    // Current resource usage.
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        hasher << usage.ru_utime.tv_sec << usage.ru_utime.tv_usec << usage.ru_stime.tv_sec << usage.ru_stime.tv_usec;
        hasher << usage.ru_maxrss << usage.ru_minflt << usage.ru_majflt << usage.ru_inblock << usage.ru_oublock;
        hasher << usage.ru_nvcsw << usage.ru_nivcsw;
    }

#ifdef __linux__
    AddFile(hasher, "/proc/diskstats");
    AddFile(hasher, "/proc/vmstat");
    AddFile(hasher, "/proc/schedstat");
    AddFile(hasher, "/proc/zoneinfo");
    AddFile(hasher, "/proc/meminfo");
    AddFile(hasher, "/proc/softirqs");
    AddFile(hasher, "/proc/stat");
    AddFile(hasher, "/proc/self/status");
#endif
#endif

    AddSysctl(hasher, "vm.loadavg");
    AddSysctl(hasher, "vm.swapusage");

    // Stack and heap location
    void* addr = malloc(4097);
    hasher << &addr << addr;
    free(addr);
}

void RandAddStaticEnv(CSHA512& hasher)
{
    // Some compile-time static properties
    uint32_t x;
    x = ((CHAR_MIN < 0) << 30) + (sizeof(void *) << 16) + (sizeof(long) << 8) + sizeof(int);
    hasher << x;
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
    x = (__GNUC_PATCHLEVEL__ << 16) + (__GNUC_MINOR__ << 8) + __GNUC__;
    hasher << x;
#endif
#if defined(_MSC_VER)
    x = _MSC_VER;
    hasher << x;
#endif
#if defined(__STDC_VERSION__)
    x = __STDC_VERSION__;
    hasher << x;
#endif
#if defined(_XOPEN_VERSION)
    x = _XOPEN_VERSION;
    hasher << x;
#endif

#ifdef __linux__
    // Information available through getauxval()
#  ifdef AT_HWCAP
    hasher << getauxval(AT_HWCAP);
#  endif
#  ifdef AT_HWCAP2
    hasher << getauxval(AT_HWCAP2);
#  endif
#  ifdef AT_RANDOM
    hasher << getauxval(AT_RANDOM);
#  endif
#  ifdef AT_PLATFORM
    const char* platform_str = (const char*)getauxval(AT_PLATFORM);
    if (platform_str) hasher.Write((const unsigned char*)platform_str, strlen(platform_str) + 1);
#  endif
#  ifdef AT_EXECFN
    const char* exec_str = (const char*)getauxval(AT_EXECFN);
    if (exec_str) hasher.Write((const unsigned char*)exec_str, strlen(exec_str) + 1);
#  endif
#endif // __linux__

#ifdef HAVE_GETCPUID
    AddCPUID(hasher);
#endif

    // Memory locations
    hasher << &x << &RandAddStaticEnv << &malloc << &errno << &environ;

    // Hostname
    char hname[256];
    if (gethostname(hname, 256) == 0) {
        hasher.Write((const unsigned char*)hname, strnlen(hname, 256));
    }

#if HAVE_DECL_GETIFADDRS
    // Network interfaces
    struct ifaddrs *ifad = NULL;
    getifaddrs(&ifad);
    struct ifaddrs *ifit = ifad;
    while (ifit != NULL) {
        hasher.Write((const unsigned char*)&ifit, sizeof(ifit));
        hasher.Write((const unsigned char*)ifit->ifa_name, strlen(ifit->ifa_name) + 1);
        hasher.Write((const unsigned char*)&ifit->ifa_flags, sizeof(ifit->ifa_flags));
        AddSockaddr(hasher, ifit->ifa_addr);
        AddSockaddr(hasher, ifit->ifa_netmask);
        AddSockaddr(hasher, ifit->ifa_dstaddr);
        ifit = ifit->ifa_next;
    }
    freeifaddrs(ifad);
#endif

#ifndef WIN32
    // UNIX kernel information
    struct utsname name;
    if (uname(&name) != -1) {
        hasher.Write((const unsigned char*)&name.sysname, strlen(name.sysname) + 1);
        hasher.Write((const unsigned char*)&name.nodename, strlen(name.nodename) + 1);
        hasher.Write((const unsigned char*)&name.release, strlen(name.release) + 1);
        hasher.Write((const unsigned char*)&name.version, strlen(name.version) + 1);
        hasher.Write((const unsigned char*)&name.machine, strlen(name.machine) + 1);
    }

    /* Path and filesystem provided data */
    AddPath(hasher, "/");
    AddPath(hasher, ".");
    AddPath(hasher, "/tmp");
    AddPath(hasher, "/home");
    AddPath(hasher, "/proc");
#ifdef __linux__
    AddFile(hasher, "/proc/cmdline");
    AddFile(hasher, "/proc/cpuinfo");
    AddFile(hasher, "/proc/version");
#endif
    AddFile(hasher, "/etc/passwd");
    AddFile(hasher, "/etc/group");
    AddFile(hasher, "/etc/hosts");
    AddFile(hasher, "/etc/resolv.conf");
    AddFile(hasher, "/etc/timezone");
    AddFile(hasher, "/etc/localtime");
    AddFile(hasher, "/etc/hostconfig");
#endif

    // For MacOS/BSDs, gather data through sysctl instead of /proc
    AddSysctl(hasher, "hw.cpufrequency");
    AddSysctl(hasher, "hw.cpusubtype");
    AddSysctl(hasher, "hw.cputype");
    AddSysctl(hasher, "hw.l1dcachesize");
    AddSysctl(hasher, "hw.l1icachesize");
    AddSysctl(hasher, "hw.l2cachesize");
    AddSysctl(hasher, "hw.l3cachesize");
    AddSysctl(hasher, "hw.machine");
    AddSysctl(hasher, "hw.machine_arch");
    AddSysctl(hasher, "hw.memsize");
    AddSysctl(hasher, "hw.model");
    AddSysctl(hasher, "hw.ncpu");
    AddSysctl(hasher, "hw.physmem");
    AddSysctl(hasher, "hw.usermem");
    AddSysctl(hasher, "hw.realmem");
    AddSysctl(hasher, "kern.bootargs");
    AddSysctl(hasher, "kern.boottime");
    AddSysctl(hasher, "kern.domainname");
    AddSysctl(hasher, "kern.hostid");
    AddSysctl(hasher, "kern.hostname");
    AddSysctl(hasher, "kern.ostype");
    AddSysctl(hasher, "kern.osreldate");
    AddSysctl(hasher, "kern.osrelease");
    AddSysctl(hasher, "kern.osrevision");
    AddSysctl(hasher, "kern.uuid");
    AddSysctl(hasher, "kern.version");
    AddSysctl(hasher, "kern.waketime");

    // Env variables
    if (environ) {
        for (size_t i = 0; environ[i]; ++i) {
            hasher.Write((const unsigned char*)environ[i], strlen(environ[i]));
        }
    }

    // Process, thread, user, session, group, ... ids.
#ifdef WIN32
    hasher << GetCurrentProcessId() << GetCurrentThreadId();
#else
    hasher << getpid() << getppid() << getsid(0) << getpgid(0) << getuid() << geteuid() << getgid() << getegid();
#endif
    hasher << std::this_thread::get_id();
}
