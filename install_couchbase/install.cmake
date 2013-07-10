# Script for Downloading and configuring the Couchbase Server to
#run with the Module tests.
#
# Usage: cmake [ -Dsystem=value ...]
#          -P install.cmake
#   Be sure all -D options come before -P !
#   Valid options:
#     system = system to indetify the couchbase server to download and 
#              install, currently only ubuntu32 and ubuntu64 are valid 
#              options.
#
# Example: cmake -Dsystem=ubuntu64 -P install.cmake


# uninstalling couchbase server
# run:
# sudo dpkg -r couchbase-server
# sudo --purge couchbase-server
# sudo rm -rf /opt/couchbase/

#Figure out what directory we're running in
get_filename_component(cwd ${CMAKE_CURRENT_LIST_FILE} PATH)

FIND_PROGRAM(WGET wget DOC "tool for downloading couchbase server")

IF(SKIP_COUCHBASE_DOWNLOAD)
  SET(COUCHBASE_DOWNLOAD OFF)
ELSE(SKIP_COUCHBASE_DOWNLOAD)
  SET(COUCHBASE_DOWNLOAD ON)
ENDIF(SKIP_COUCHBASE_DOWNLOAD)

#Couchbase Configure
#Check what version of couchbase server to download
SET (COUCHBASE_VER "2.0.1")
IF(system)
  SET(DOWNLOAD_PATH "http://packages.couchbase.com/releases/${COUCHBASE_VER}")
  IF (${system} STREQUAL "ubuntu32")
    SET(COUCHBASE_DEB_NAME
      "couchbase-server-community_x86_${COUCHBASE_VER}.deb")
  ELSEIF (${system} STREQUAL "ubuntu64")
    SET(COUCHBASE_DEB_NAME
      "couchbase-server-community_x86_64_${COUCHBASE_VER}.deb")
  ELSE (${system} STREQUAL "ubuntu32")
    MESSAGE(FATAL_ERROR "Invalid value for system (available values ubuntu32 and ubuntu64")
  ENDIF (${system} STREQUAL "ubuntu32")  
ELSE(system)
  MESSAGE(FATAL_ERROR "the variable 'system' must be set")
ENDIF(system)

#Couchbase Download
IF(COUCHBASE_DOWNLOAD)
  IF(WGET)
  MESSAGE(STATUS "wget found")
  MESSAGE(STATUS "Starting Download...")
  EXECUTE_PROCESS(COMMAND "${WGET}" "${DOWNLOAD_PATH}/${COUCHBASE_DEB_NAME}")
  ELSE(WGET)
  MESSAGE(FATAL_ERROR "Wget was not found, cannot download Couchbase Server")
  ENDIF(WGET)
ENDIF(COUCHBASE_DOWNLOAD)

#Couchbase Install
MESSAGE(STATUS "Installing Couchbase Server...")
EXECUTE_PROCESS(COMMAND sudo apt-get install libssl0.9.8)
EXECUTE_PROCESS(COMMAND sudo dpkg -i ${COUCHBASE_DEB_NAME})

#Couchbase C SDK Install
MESSAGE(STATUS "Installing Couchbase C SDK...")
EXECUTE_PROCESS(COMMAND sudo "${WGET}" -O/etc/apt/sources.list.d/couchbase.list http://packages.couchbase.com/ubuntu/couchbase-ubuntu1204.list)
EXECUTE_PROCESS(COMMAND "${WGET}" -O- http://packages.couchbase.com/ubuntu/couchbase.key
  COMMAND sudo apt-key add -)
EXECUTE_PROCESS(COMMAND sudo apt-get update)
EXECUTE_PROCESS(COMMAND sudo apt-get install libcouchbase2 libcouchbase-dev)

#Couchbase Setup
MESSAGE(STATUS "Starting Setup...")
MESSAGE(STATUS "Running node-init...")
EXECUTE_PROCESS(COMMAND /opt/couchbase/bin/couchbase-cli node-init -c 127.0.0.1 --node-init-data-path=/opt/couchbase/var/lib/couchbase/data/ -u admin -p password)
MESSAGE(STATUS "Running cluster-init...")
EXECUTE_PROCESS(COMMAND /opt/couchbase/bin/couchbase-cli cluster-init -c 127.0.0.1 --cluster-init-ramsize=2048 -u admin -p password)
MESSAGE(STATUS "Running bucket-create...")
EXECUTE_PROCESS(COMMAND /opt/couchbase/bin/couchbase-cli bucket-create -c 127.0.0.1 --bucket=default --bucket-type=couchbase --bucket-port=11211 --bucket-ramsize=200 --bucket-replica=1 -u admin -p password)
MESSAGE(STATUS "Couchbase Server Configure for testing, Username=admin Password=password bucket-name=default")
EXECUTE_PROCESS(COMMAND rm ${COUCHBASE_DEB_NAME})

