# Copyright (c) 2023 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

function(add_boost_if_needed)
  #[=[
  TODO: Not all targets, which will be added in the future, require
        Boost. Therefore, a proper check will be appropriate here.

  Implementation notes:
  Although only Boost headers are used to build Bitcoin Core,
  we still leverage a standard CMake's approach to handle
  dependencies, i.e., the Boost::headers "library".
  A command target_link_libraries(target PRIVATE Boost::headers)
  will propagate Boost::headers usage requirements to the target.
  For Boost::headers such usage requirements is an include
  directory and other added INTERFACE properties.
  ]=]

  set(Boost_NO_BOOST_CMAKE ON)
  find_package(Boost 1.73.0 REQUIRED)
  mark_as_advanced(Boost_INCLUDE_DIR)
  set_target_properties(Boost::headers PROPERTIES IMPORTED_GLOBAL TRUE)
  target_compile_definitions(Boost::headers INTERFACE
    # We don't use multi_index serialization.
    BOOST_MULTI_INDEX_DISABLE_SERIALIZATION
  )
  if(DEFINED VCPKG_TARGET_TRIPLET)
    # Workaround for https://github.com/microsoft/vcpkg/issues/36955.
    target_compile_definitions(Boost::headers INTERFACE
      BOOST_NO_USER_CONFIG
    )
  endif()

  # Prevent use of std::unary_function, which was removed in C++17,
  # and will generate warnings with newer compilers for Boost
  # older than 1.80.
  # See: https://github.com/boostorg/config/pull/430.
  set(CMAKE_REQUIRED_DEFINITIONS -DBOOST_NO_CXX98_FUNCTION_BASE)
  set(CMAKE_REQUIRED_INCLUDES ${Boost_INCLUDE_DIR})
  include(CMakePushCheckState)
  cmake_push_check_state()
  include(TryAppendCXXFlags)
  set(CMAKE_REQUIRED_FLAGS ${working_compiler_werror_flag})
  set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
  check_cxx_source_compiles("
    #include <boost/config.hpp>
    " NO_DIAGNOSTICS_BOOST_NO_CXX98_FUNCTION_BASE
  )
  cmake_pop_check_state()
  if(NO_DIAGNOSTICS_BOOST_NO_CXX98_FUNCTION_BASE)
    target_compile_definitions(Boost::headers INTERFACE
      BOOST_NO_CXX98_FUNCTION_BASE
    )
  endif()

  if(BUILD_TESTS)
    include(CheckCXXSourceCompiles)
    check_cxx_source_compiles("
      #define BOOST_TEST_MAIN
      #include <boost/test/included/unit_test.hpp>
      " HAVE_BOOST_INCLUDED_UNIT_TEST_H
    )
    if(NOT HAVE_BOOST_INCLUDED_UNIT_TEST_H)
      message(FATAL_ERROR "Building test_bitcoin executable requested but boost/test/included/unit_test.hpp header not available.")
    endif()

    check_cxx_source_compiles("
      #define BOOST_TEST_MAIN
      #include <boost/test/included/unit_test.hpp>
      #include <boost/test/unit_test.hpp>
      " HAVE_BOOST_UNIT_TEST_H
    )
    if(NOT HAVE_BOOST_UNIT_TEST_H)
      message(FATAL_ERROR "Building test_bitcoin executable requested but boost/test/unit_test.hpp header not available.")
    endif()
  endif()

endfunction()