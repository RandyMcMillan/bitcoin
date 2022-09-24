# Copyright (c) 2022 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

# Optional features and packages.

if(CCACHE)
  find_program(PROG_CCACHE ccache)
  if(PROG_CCACHE)
    set(CCACHE ON)
    if(MSVC)
      # See https://github.com/ccache/ccache/wiki/MS-Visual-Studio
      set(MSVC_CCACHE_WRAPPER_CONTENT "\"${PROG_CCACHE}\" \"${CMAKE_CXX_COMPILER}\"")
      set(MSVC_CCACHE_WRAPPER_FILENAME wrapped-cl.bat)
      file(WRITE ${CMAKE_BINARY_DIR}/${MSVC_CCACHE_WRAPPER_FILENAME} "${MSVC_CCACHE_WRAPPER_CONTENT} %*")
      set(CMAKE_VS_GLOBALS
        "CLToolExe=${MSVC_CCACHE_WRAPPER_FILENAME}"
        "CLToolPath=${CMAKE_BINARY_DIR}"
        "TrackFileAccess=false"
        "UseMultiToolTask=true"
        "DebugInformationFormat=OldStyle"
      )
    else()
      set(CMAKE_C_COMPILER_LAUNCHER ${PROG_CCACHE})
      set(CMAKE_CXX_COMPILER_LAUNCHER ${PROG_CCACHE})
    endif()
  elseif(CCACHE STREQUAL ON)
    message(FATAL_ERROR "ccache requested, but not found.")
  else()
    set(CCACHE OFF)
  endif()
  mark_as_advanced(PROG_CCACHE)
endif()

if(ENABLE_WALLET)
  if(WITH_SQLITE)
    cross_pkg_check_modules(sqlite sqlite3>=3.7.17 IMPORTED_TARGET)
    if(sqlite_FOUND)
      set(WITH_SQLITE ON)
      set(USE_SQLITE ON)
    else()
      if(WITH_SQLITE STREQUAL ON)
        message(FATAL_ERROR "SQLite requested, but not found.")
      endif()
      set(WITH_SQLITE OFF)
    endif()
  endif()

  if(WITH_BDB)
    find_package(BerkeleyDB 4.8)
    if(BerkeleyDB_FOUND)
      set(WITH_BDB ON)
      set(USE_BDB ON)
      if(NOT BerkeleyDB_VERSION VERSION_EQUAL 4.8)
        message(WARNING "Found Berkeley DB (BDB) other than 4.8.")
        if(WARN_INCOMPATIBLE_BDB)
          message(WARNING "BDB (legacy) wallets opened by this build would not be portable!\n"
                          "If this is intended, pass \"-DWARN_INCOMPATIBLE_BDB=OFF\".\n"
                          "Passing \"-DWITH_BDB=OFF\" will suppress this warning.\n")
        else()
          message(WARNING "BDB (legacy) wallets opened by this build will not be portable!")
        endif()
      endif()
    else()
      message(WARNING "Berkeley DB (BDB) required for legacy wallet support, but not found.\n"
                      "Passing \"-DWITH_BDB=OFF\" will suppress this warning.\n")
      set(WITH_BDB OFF)
    endif()
  endif()
else()
  set(WITH_SQLITE OFF)
  set(WITH_BDB OFF)
endif()

if(WITH_NATPMP)
  find_package(NATPMP)
  if(NATPMP_FOUND)
    set(WITH_NATPMP ON)
  else()
    if(WITH_NATPMP STREQUAL ON)
      message(FATAL_ERROR "libnatpmp requested, but not found.")
    else()
      message(WARNING "libnatpmp not found, disabling.\n"
                      "To skip libnatpmp check, use \"-DWITH_NATPMP=OFF\".\n")
    endif()
    set(WITH_NATPMP OFF)
  endif()
endif()

if(WITH_MINIUPNPC)
  find_package(MiniUPnPc)
  if(MiniUPnPc_FOUND)
    set(WITH_MINIUPNPC ON)
  else()
    if(WITH_MINIUPNPC STREQUAL ON)
      message(FATAL_ERROR "libminiupnpc requested, but not found.")
    else()
      message(WARNING "libminiupnpc not found, disabling.\n"
                      "To skip libminiupnpc check, use \"-DWITH_MINIUPNPC=OFF\".\n")
    endif()
    set(WITH_MINIUPNPC OFF)
  endif()
endif()

if(WITH_ZMQ)
  if(MSVC)
    find_package(ZeroMQ CONFIG)
  else()
    cross_pkg_check_modules(libzmq libzmq>=4 IMPORTED_TARGET)
    if(libzmq_FOUND)
      set_property(TARGET PkgConfig::libzmq APPEND PROPERTY
        INTERFACE_COMPILE_DEFINITIONS $<$<PLATFORM_ID:Windows>:ZMQ_STATIC>
      )
      set_property(TARGET PkgConfig::libzmq APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES $<$<PLATFORM_ID:Windows>:iphlpapi;ws2_32>
      )
    endif()
  endif()
  if(TARGET libzmq OR TARGET PkgConfig::libzmq)
    set(WITH_ZMQ ON)
  else()
    if(WITH_ZMQ STREQUAL ON)
      message(FATAL_ERROR "libzmq requested, but not found.")
    else()
      message(WARNING "libzmq not found, disabling.\n"
                      "To skip libzmq check, use \"-DWITH_ZMQ=OFF\".\n")
    endif()
    set(WITH_ZMQ OFF)
  endif()
endif()

include(CheckCXXSourceCompiles)
if(WITH_USDT)
  check_cxx_source_compiles("
  #include <sys/sdt.h>

  int main()
  {
    DTRACE_PROBE(\"context\", \"event\");
  }
  " HAVE_USDT_H)
  if(HAVE_USDT_H)
    set(ENABLE_TRACING TRUE)
    set(WITH_USDT ON)
  else()
    if(WITH_USDT STREQUAL ON)
      message(FATAL_ERROR "sys/sdt.h requested, but not found.")
    endif()
    set(WITH_USDT OFF)
  endif()
endif()

if(WITH_GUI AND WITH_QRENCODE)
  cross_pkg_check_modules(libqrencode libqrencode IMPORTED_TARGET)
  if(libqrencode_FOUND)
    set_target_properties(PkgConfig::libqrencode PROPERTIES
      INTERFACE_COMPILE_DEFINITIONS USE_QRCODE
    )
    set(WITH_QRENCODE ON)
  else()
    if(WITH_QRENCODE STREQUAL ON)
      message(FATAL_ERROR "libqrencode requested, but not found.")
    endif()
    set(WITH_QRENCODE OFF)
  endif()
endif()

if(WITH_SECCOMP)
  check_cxx_source_compiles("
  #include <linux/seccomp.h>
  #if !defined(__x86_64__)
  #  error Syscall sandbox is an experimental feature currently available only under Linux x86-64.
  #endif
  int main(){}
  " HAVE_SECCOMP_H)
  if(HAVE_SECCOMP_H)
    set(USE_SYSCALL_SANDBOX TRUE)
    set(WITH_SECCOMP ON)
  else()
    if(WITH_SECCOMP STREQUAL ON)
      message(FATAL_ERROR "linux/seccomp.h requested, but not found.")
    endif()
    set(WITH_SECCOMP OFF)
  endif()
endif()

if(WITH_EXTERNAL_SIGNER)
  if(WIN32)
    if(WITH_EXTERNAL_SIGNER STREQUAL ON)
      message(FATAL_ERROR "External signer is not supported on Windows.")
    endif()
    set(WITH_EXTERNAL_SIGNER OFF)
  endif()

  set(CMAKE_REQUIRED_INCLUDES ${Boost_INCLUDE_DIRS})
  set(CMAKE_REQUIRED_LIBRARIES Threads::Threads)

  # Boost 1.78 and 1.79 require the following workaround.
  # See: https://github.com/boostorg/process/issues/235
  set(CMAKE_REQUIRED_FLAGS "-Wno-error=narrowing")

  check_cxx_source_compiles("
  #include <boost/process.hpp>
  int main(){}
  " HAVE_BOOST_PROCESS_H)

  if(HAVE_BOOST_PROCESS_H)
    set(ENABLE_EXTERNAL_SIGNER TRUE)
    set(WITH_EXTERNAL_SIGNER ON)
  else()
    if(WITH_EXTERNAL_SIGNER STREQUAL ON)
      message(FATAL_ERROR "External signer support requested, but is not supported by this Boost.Process version.")
    endif()
    set(WITH_EXTERNAL_SIGNER OFF)
  endif()
  set(CMAKE_REQUIRED_FLAGS)
  set(CMAKE_REQUIRED_LIBRARIES)
  set(CMAKE_REQUIRED_INCLUDES)
endif()