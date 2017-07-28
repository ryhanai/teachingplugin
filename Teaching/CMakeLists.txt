option(BUILD_TEACHING_PLUGIN "Building Teaching plugin" ON)

if(NOT BUILD_TEACHING_PLUGIN)
  return()
endif()

set(libname CnoidTeachingPlugin)

if(NOT ENABLE_PYTHON)
  find_package(PythonLibs REQUIRED)
endif()

set(sources
  ActivityEditor.cpp
  ActivityEditorBase.cpp
  ArgumentDialog.cpp
  Calculator.cpp
  ChoreonoidUtil.cpp
  ControllerManager.cpp
  DataBaseDetailManager.cpp
  DataBaseManager.cpp
  DecisionDialog.cpp
  ExecEnvDialog.cpp
  FlowActivityEditor.cpp
  FlowSearchDialog.cpp
  FlowView.cpp
  LoggerUtil.cpp
  MetaDataView.cpp
  ModelDialog.cpp
  ParameterDialog.cpp
  ParameterView.cpp
  PythonWrapper.cpp
  SettingDialog.cpp
  StateMachineParams.cpp
  StateMachineView.cpp
  TaskExecuteManager.cpp
  TaskExecutionView.cpp
  TaskExecutor.cpp
  TaskInstanceView.cpp
  TeachingPlugin.cpp
  TeachingTypes.cpp
  TeachingUtil.cpp
)

set(headers
  ActivityEditorBase.h
  ActivityEditor.h
  ArgumentDialog.h
  Calculator.h
  ChoreonoidUtil.h
  ControllerBase.h
  CommandDefTypes.h
  ControllerManager.h
  DataBaseManager.h
  DecisionDialog.h
  ExecEnvDialog.h
  FlowActivityEditor.h
  FlowSearchDialog.h
  FlowView.h
  LoggerUtil.h
  MetaDataView.h
  ModelDialog.h
  ParameterDialog.h
  ParameterView.h
  PythonWrapper.h
  SettingDialog.h
  StateMachineView.h
  TaskExecuteManager.h
  TaskExecutionView.h
  TaskExecutor.h
  TaskInstanceView.h
  TeachingTypes.h
  TeachingUtil.h
  QtUtil.h
  )

set(qtheaders
  ActivityEditorBase.h
  ArgumentDialog.h
  DecisionDialog.h
  ExecEnvDialog.h
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

make_gettext_mofiles(${libname} mofiles)
add_cnoid_plugin(${libname} SHARED ${sources} ${headers} ${mofiles} ${RC_SRCS})

if(NOT QT5)
  target_link_libraries(${libname} Qt4::QtSql)
else()
  qt5_use_modules(${libname} Sql)
  target_link_libraries(${libname} ${QT_QTSQL_LIBRARY})
endif()

include_directories(${CONTROLLER_DIRS} ${PYTHON_INCLUDE_DIRS})

target_link_libraries(${libname} CnoidBodyPlugin ${PYTHON_LIBRARIES})
#target_link_libraries(${libname} CnoidBodyPlugin ${PYTHON_LIBRARIES} TaskParamAdjust)

if(UNIX)
  add_definitions("-std=c++11")

elseif(MSVC)
  set_target_properties(${libname} PROPERTIES LINK_FLAGS /NODEFAULTLIB:LIBCMT )
  set_target_properties(${libname} PROPERTIES DEBUG_POSTFIX d )
  set_target_properties(${libname} PROPERTIES COMPILE_FLAGS "/wd4250 /wd4275 /wd4018 /wd4305")

endif(UNIX)
apply_common_setting_for_plugin(${libname}  "${headers}")
