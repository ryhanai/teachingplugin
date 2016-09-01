if(ENABLE_INSTALL_RPATH)
  set(CMAKE_INSTALL_RPATH "$ORIGIN/..")
endif()

set(TEACHINGPLUGINS "Teaching"  CACHE STRING
  "Sub-directories containing plugins to compile in the each directory")

if(CNOID_VERSION EQUAL 1.0 OR CNOID_VERSION EQUAL 1.1)
  add_definitions(-DCNOID_10_11)
endif()

if(CNOID_VERSION EQUAL 1.0 OR CNOID_VERSION EQUAL 1.1 OR CNOID_VERSION EQUAL 1.2  OR CNOID_VERSION EQUAL 1.3 )
  add_definitions(-DCNOID_10_11_12_13)
endif()

link_directories(${PYTHON_LIBRARY})
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /fp:precise")

foreach(subdirectory ${TEACHINGPLUGINS})
  add_subdirectory(${subdirectory})
endforeach()

