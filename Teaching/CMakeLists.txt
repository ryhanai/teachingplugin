option(BUILD_TEACHING_PLUGIN "Building Teaching plugin" ON)

if(NOT BUILD_TEACHING_PLUGIN)
  return()
endif()

set(libname CnoidTeachingPlugin)

set(CONTROLLER_DIRS ../../SampleHiroControllerPlugin)

set(sources
  ArgumentDialog.cpp
  ArgumentDialogMock.cpp
  Calculator.cpp
  ChoreonoidUtil.cpp
  DataBaseDetailManager.cpp
  DataBaseManager.cpp
  FlowSearchDialog.cpp
  FlowSearchDialogMock.cpp
  FlowView.cpp
  FlowViewMock.cpp
  LoggerUtil.cpp
  MetaDataView.cpp
  MetaDataViewMock.cpp
  ParameterDialog.cpp
  ParameterDialogMock.cpp
  ParameterView.cpp
  ParameterViewMock.cpp
  SettingDialog.cpp
  SettingDialogMock.cpp
  StateMachineParams.cpp
  StateMachineView.cpp
  StateMachineViewMock.cpp
  TaskExecutionView.cpp
  TaskExecutor.cpp
  TaskInstanceView.cpp
  TaskInstanceViewMock.cpp
  TeachingPlugin.cpp
  TeachingTypes.cpp
  TeachingUtil.cpp
  ${CONTROLLER_DIRS}/SampleHiroController.cpp
)

set(headers
  ArgumentDialog.h
  Calculator.h
  ChoreonoidUtil.h
  CommandDefTypes.h
  DataBaseManager.h
  FlowSearchDialog.h
  FlowView.h
  LoggerUtil.h
  MetaDataView.h
  ParameterDialog.h
  ParameterView.h
  SettingDialog.h
  StateMachineView.h
  TaskExecutionView.h
  TaskExecutor.h
  TaskInstanceView.h
  TeachingTypes.h
  TeachingUtil.h
  )

make_gettext_mofiles(CnoidTeachingPlugin mofiles)

if(NOT QT5)
  QT4_ADD_RESOURCES(RC_SRCS TeachingPlugin.qrc)
else()
  QT5_ADD_RESOURCES(RC_SRCS TeachingPlugin.qrc)
endif()

#set(CONTROLLER_INCLUDE_DIRS ../../SampleHiroControllerPlugin)
include_directories(${CONTROLLER_DIRS})

add_cnoid_plugin(${libname} SHARED ${sources} ${headers} ${mofiles} ${RC_SRCS})

target_link_libraries(${libname} Qt4::QtSql)
target_link_libraries(${libname} CnoidBodyPlugin)
#target_link_libraries(${libname} CnoidSampleHiroControllerPlugin)

if(UNIX)
  add_definitions("-std=c++11")

elseif(MSVC)
  set_target_properties(${libname} PROPERTIES LINK_FLAGS /NODEFAULTLIB:LIBCMT )
  set_target_properties(${libname} PROPERTIES DEBUG_POSTFIX d )
  set_target_properties(${libname} PROPERTIES COMPILE_FLAGS "/wd4250 /wd4275 /wd4018 /wd4305")

endif(UNIX)
apply_common_setting_for_plugin(${libname}  "${headers}")
