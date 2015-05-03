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

MESSAGE (STATUS "Looking for include file libcouchbase/couchbase.h")
FIND_PATH (
  LIBCOUCHBASE_INCLUDE_DIR
  libcouchbase/couchbase.h
  PATHS /opt/local/include)
MARK_AS_ADVANCED (LIBCOUCHBASE_INCLUDE_DIR)
MESSAGE (STATUS "Include path: ${LIBCOUCHBASE_INCLUDE_DIR}")

MESSAGE (STATUS "Looking for libary file couchbase/libcouchbase")
FIND_LIBRARY (
  LIBCOUCHBASE_LIBRARY
  NAMES couchbase libcouchbase
  PATHS /opt/local/lib)
MARK_AS_ADVANCED (LIBCOUCHBASE_LIBRARY)
MESSAGE (STATUS "Library path: ${LIBCOUCHBASE_INCLUDE_DIR}")

IF (LIBCOUCHBASE_INCLUDE_DIR AND LIBCOUCHBASE_LIBRARY)
  SET (LIBCOUCHBASE_FOUND 1)
  SET (LIBCOUCHBASE_LIBRARIES ${LIBCOUCHBASE_LIBRARY})
  SET (LIBCOUCHBASE_INCLUDE_DIRS ${LIBCOUCHBASE_INCLUDE_DIR})
  IF (NOT LIBCOUCHBASE_FIND_QUIETLY)
    MESSAGE (STATUS "Found libcouchbase library: " ${LIBCOUCHBASE_LIBRARY})
    MESSAGE (STATUS "Found libcouchbase include path : " ${LIBCOUCHBASE_INCLUDE_DIR})
  ENDIF (NOT LIBCOUCHBASE_FIND_QUIETLY)

  SET(CMAKE_REQUIRED_INCLUDES "${LIBCOUCHBASE_INCLUDE_DIR}")
  SET(CMAKE_REQUIRED_LIBRARIES "${LIBCOUCHBASE_LIBRARY}")

ELSE (LIBCOUCHBASE_INCLUDE_DIR AND LIBCOUCHBASE_LIBRARY)
  SET (LIBCOUCHBASE_FOUND 0)
  SET (LIBCOUCHBASE_LIBRARIES)
  SET (LIBCOUCHBASE_INCLUDE_DIRS)
ENDIF (LIBCOUCHBASE_INCLUDE_DIR AND LIBCOUCHBASE_LIBRARY)  
