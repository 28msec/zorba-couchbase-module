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

FIND_PACKAGE_WIN32 (NAME LibCouchbase FOUND_VAR LIBCOUCHBASE_FOUND SEARCH_NAMES libcouchbase)

IF (LIBCOUCHBASE_FOUND)
  FIND_PACKAGE_DLLS_WIN32 (${FOUND_LOCATION} libcouchbase.dll)
  SET(CMAKE_REQUIRED_INCLUDES "${LIBCOUCHBASE_INCLUDE_DIR}")
  SET(CMAKE_REQUIRED_LIBRARIES "${LIBCOUCHBASE_LIBRARY}")
ENDIF (LIBCOUCHBASE_FOUND)  
