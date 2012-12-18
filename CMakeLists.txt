# Copyright 2012 The FLWOR Foundation.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT (zorba_couchbase_module)
ENABLE_TESTING ()
INCLUDE (CTest)

IF (WIN32)
  # On Windows we use proxy modules that try to guess first the location
  # of the required third party libraries. This will search in order in:
  # 1. the path pointed by ZORBA_THIRD_PARTY_REQUIREMENTS
  # 2. the Program Files directory available on the users computer
  # 3. the PATH environment variable
  # The logic is implemented by the macros in the ProxyFindModule.cmake module.
  LIST (APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules/Windows")
ENDIF (WIN32)
LIST (APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules")

FIND_PACKAGE (Zorba REQUIRED HINTS "${ZORBA_BUILD_DIR}")
MESSAGE(STATUS "Zorba_USE_FILE ${Zorba_USE_FILE}")
INCLUDE ("${Zorba_USE_FILE}")

IF (ZORBA_SUPPRESS_LIBCOUCHBASE)
  MESSAGE (STATUS "ZORBA_SUPRESS_COUCHBASE is true - not searching for LibCouchbase.")
ELSE (ZORBA_SUPRESS_LIBCOUCHBASE)
  
  MESSAGE (STATUS "Looking for LibCouchbase")
  FIND_PACKAGE ("LibCouchbase")

  IF (LIBCOUCHBASE_FOUND)
    MESSAGE (STATUS "Found LibCouchbase --" ${LIBCOUCHBASE_LIBRARIES})
    INCLUDE_DIRECTORIES (${LIBCOUCHBASE_INCLUDE_DIR})  

    ADD_SUBDIRECTORY("src")
    ADD_TEST_DIRECTORY("${PROJECT_SOURCE_DIR}/test")
  ELSE (LIBCOUCHBASE_FOUND)
    MESSAGE (STATUS "LibCouchbase library not found; couchbase module skipped")
  ENDIF (LIBCOUCHBASE_FOUND)
ENDIF (ZORBA_SUPPRESS_LIBCOUCHBASE)

DONE_DECLARING_ZORBA_URIS()
