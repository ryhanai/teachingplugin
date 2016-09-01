option(BUILD_TEACHING_PLUGIN "Building Teaching plugin" ON)

if(NOT BUILD_TEACHING_PLUGIN)
  return()
endif()

set(libname CnoidTeachingPlugin)

set(sources
  ArgumentDialog.cpp
  Calculator.cpp
  ChoreonoidUtil.cpp
  ControllerManager.cpp
  DataBaseDetailManager.cpp
  DataBaseManager.cpp
  FlowSearchDialog.cpp
  FlowView.cpp
  LoggerUtil.cpp
  MetaDataView.cpp
  ModelDialog.cpp
  ParameterDialog.cpp
  ParameterView.cpp
  SettingDialog.cpp
  StateMachineParams.cpp
  StateMachineView.cpp
  TaskExecutionView.cpp
  TaskExecutor.cpp
  TaskInstanceView.cpp
  TeachingPlugin.cpp
  TeachingTypes.cpp
  TeachingUtil.cpp
)

set(headers
  ArgumentDialog.h
  Calculator.h
  ChoreonoidUtil.h
  ControllerBase.h
  CommandDefTypes.h
  ControllerManager.h
  DataBaseManager.h
  FlowSearchDialog.h
  FlowView.h
  LoggerUtil.h
  MetaDataView.h
  ModelDialog.h
  ParameterDialog.h
  ParameterView.h
  SettingDialog.h
  StateMachineView.h
  TaskExecutionView.h
  TaskExecutor.h
  TaskInstanceView.h
  TeachingTypes.h
  TeachingUtil.h
  QtUtil.h
  )

set(qtheaders
  ArgumentDialog.h
  FlowSearchDialog.h
  FlowView.h
  ModelDialog.h
  MetaDataView.h
  ParameterDialog.h
  ParameterView.h
  SettingDialog.h
  StateMachineView.h
  TaskInstanceView.h
  )

make_gettext_mofiles(CnoidTeachingPlugin mofiles)

if(NOT QT5)
  QT4_ADD_RESOURCES(RC_SRCS TeachingPlugin.qrc)
  QT4_WRAP_CPP(sources
    ${qtheaders}
    OPTIONS "-DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED"
    )
else()
  QT5_ADD_RESOURCES(RC_SRCS TeachingPlugin.qrc)
  QT5_WRAP_CPP(sources
    ${qtheaders}
    OPTIONS "-DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED"
    )
endif()

add_cnoid_plugin(${libname} SHARED ${sources} ${headers} ${mofiles} ${RC_SRCS})

if(NOT QT5)
  target_link_libraries(${libname} Qt4::QtSql)
else()
  qt5_use_modules(${libname} Sql)
  target_link_libraries(${libname} ${QT_QTSQL_LIBRARY})
endif()

include_directories(${CONTROLLER_DIRS})

target_link_libraries(${libname} CnoidBodyPlugin)

if(UNIX)
  add_definitions("-std=c++11")

elseif(MSVC)
  set_target_properties(${libname} PROPERTIES LINK_FLAGS /NODEFAULTLIB:LIBCMT )
  set_target_properties(${libname} PROPERTIES DEBUG_POSTFIX d )
  set_target_properties(${libname} PROPERTIES COMPILE_FLAGS "/wd4250 /wd4275 /wd4018 /wd4305")

endif(UNIX)
apply_common_setting_for_plugin(${libname}  "${headers}")
