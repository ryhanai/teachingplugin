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

# qt
if(NOT MSVC OR MSVC_VERSION LESS 1700)
  find_package(Qt4 4.7.0 REQUIRED)
  set(QT_USE_QTSQL TRUE)
  include(${QT_USE_FILE})
  add_definitions(-DQT_NO_KEYWORDS)

#  set(libraries QtSql)
#  link_libraries(QtSql4)

#  list(APPEND QT_INST_LIBRARIES 
#		optimized QtSql debug QtSqld)
#  install_external_libraries(${QT_BINARY_DIR} ${QT_LIBRARY_DIR} ${QT_INST_LIBRARIES})

endif()

if(MSVC)
  set(QHULL_DIR "C:/qhull-2012.1")
  #set(QHULL_DIR ${QHULL_DIR} CACHE PATH "set the direcotry of qhull")
  if(NOT QHULL_DIR)
    message(FATAL_ERROR "Please specify the Qhull top directory to QHULL_DIR.")
  else()
    set(QHULL_INCLUDE_DIRS ${QHULL_DIR}/src)
    set(QHULL_LIB_DIRS ${QHULL_DIR}/lib)
    set(QHULL_BIN_DIRS ${QHULL_DIR}/bin)
	if(EXISTS "${QHULL_LIB_DIRS}/qhull6.lib")
	 set(QHULL_NAME "qhull6")
	else()
	 set(QHULL_NAME "qhull")
	endif()
  endif()

  #set(SERVING_PROJECT_TRUNK_DIR ${SERVING_PROJECT_TRUNK_DIR} CACHE PATH "set the directory of project trunk")
  #set(SERVING_PROJECT_TRUNK_DIR "C:/Work/09_ServingSystem/x1_devel/trunk")
  #if(NOT SERVING_PROJECT_TRUNK_DIR)
  #  message(FATAL_ERROR "Please specify the Serving-plugin top directory to SERVING_PROJECT_TRUNK_DIR.")
  #else()
  #  set(SERVING_PLUGIN_INCLUDE_DIR ${SERVING_PROJECT_TRUNK_DIR}/src)
  #endif()

  include_directories(${QHULL_INCLUDE_DIRS})
  #include_directories(${SERVING_PLUGIN_INCLUDE_DIR})
  link_directories(${QHULL_LIB_DIRS})
  link_directories(${PYTHON_LIBRARY})

  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /fp:precise")
endif()

foreach(subdirectory ${TEACHINGPLUGINS})
  add_subdirectory(${subdirectory})
endforeach()

#add_subdirectory(IKFast)
#add_subdirectory(RobotModels)


