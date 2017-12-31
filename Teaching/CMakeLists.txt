option(BUILD_TEACHING_PLUGIN "Building Teaching plugin" ON)

if(NOT BUILD_TEACHING_PLUGIN)
  return()
endif()

set(libname CnoidTeachingPlugin)

if(NOT ENABLE_PYTHON)
  find_package(PythonLibs REQUIRED)
endif()

set(sources
  ArgumentDialog.cpp
  Calculator.cpp
  ChoreonoidUtil.cpp
  ControllerManager.cpp
  DataBaseDetailManager.cpp
  DataBaseManager.cpp
  DecisionDialog.cpp
  ExecEnvDialog.cpp
  FlowSearchDialog.cpp
  FlowView.cpp
  LoggerUtil.cpp
  MetaDataView.cpp
  ModelDialog.cpp
  ModelMasterDialog.cpp
  ParameterDialog.cpp
  ParameterView.cpp
  PythonWrapper.cpp
  SettingDialog.cpp
  StateMachineView.cpp
  TaskExecuteManager.cpp
  TaskExecutor.cpp
  TaskInstanceView.cpp
  TeachingDataHolder.cpp
  TeachingEventHandler.cpp
  TeachingPlugin.cpp
  TeachingTypes.cpp
  TeachingUtil.cpp

  NodeEditor/ActivityEditorBase.cpp
  NodeEditor/Connection.cpp
  NodeEditor/ConnectionBlurEffect.cpp
  NodeEditor/ConnectionGeometry.cpp
  NodeEditor/ConnectionGraphicsObject.cpp
  NodeEditor/ConnectionPainter.cpp
  NodeEditor/ConnectionState.cpp
  NodeEditor/ConnectionStyle.cpp
  NodeEditor/DataModelRegistry.cpp
  NodeEditor/FlowEditor.cpp
  NodeEditor/FlowScene.cpp
  NodeEditor/FlowViewStyle.cpp
  NodeEditor/models.cpp
  NodeEditor/Node.cpp
  NodeEditor/NodeConnectionInteraction.cpp
  NodeEditor/NodeDataModel.cpp
  NodeEditor/NodeGeometry.cpp
  NodeEditor/NodeGraphicsObject.cpp
  NodeEditor/NodePainter.cpp
  NodeEditor/NodeState.cpp
  NodeEditor/NodeStyle.cpp
  NodeEditor/ParamWidget.cpp
  NodeEditor/Properties.cpp
  NodeEditor/StateMachineEditor.cpp
  NodeEditor/StyleCollection.cpp
)

set(headers
  ArgumentDialog.h
  Calculator.h
  ChoreonoidUtil.h
  ControllerBase.h
  CommandDefTypes.h
  ControllerManager.h
  DataBaseManager.h
  DecisionDialog.h
  ExecEnvDialog.h
  FlowSearchDialog.h
  FlowView.h
  LoggerUtil.h
  MetaDataView.h
  ModelDialog.h
  ModelMasterDialog.h
  ParameterDialog.h
  ParameterView.h
  PythonWrapper.h
  SettingDialog.h
  StateMachineView.h
  TaskExecuteManager.h
  TaskExecutor.h
  TaskInstanceView.h
  TeachingDataHolder.h
  TeachingEventHandler.h
  TeachingTypes.h
  TeachingUtil.h
  QtUtil.h

  NodeEditor/ActivityEditorBase.hpp
  NodeEditor/Compiler.hpp
  NodeEditor/Connection.hpp
  NodeEditor/ConnectionBlurEffect.hpp
  NodeEditor/ConnectionGeometry.hpp
  NodeEditor/ConnectionGraphicsObject.hpp
  NodeEditor/ConnectionPainter.hpp
  NodeEditor/ConnectionState.hpp
  NodeEditor/ConnectionStyle.hpp
  NodeEditor/DataModelRegistry.hpp
  NodeEditor/Export.hpp
  NodeEditor/FlowEditor.hpp
  NodeEditor/FlowScene.hpp
  NodeEditor/FlowViewStyle.hpp
  NodeEditor/make_unique.hpp
  NodeEditor/models.hpp
  NodeEditor/Node.hpp
  NodeEditor/NodeConnectionInteraction.hpp
  NodeEditor/NodeData.hpp
  NodeEditor/NodeDataModel.hpp
  NodeEditor/NodeGeometry.hpp
  NodeEditor/NodeGraphicsObject.hpp
  NodeEditor/NodePainter.hpp
  NodeEditor/NodePainterDelegate.hpp
  NodeEditor/NodeState.hpp
  NodeEditor/NodeStyle.hpp
  NodeEditor/OperatingSystem.hpp
  NodeEditor/ParamWidget.hpp
  NodeEditor/PortType.hpp
  NodeEditor/Properties.hpp
  NodeEditor/QStringStdHash.hpp
  NodeEditor/QUuidStdHash.hpp
  NodeEditor/Serializable.hpp
  NodeEditor/StateMachineEditor.hpp
  NodeEditor/Style.hpp
  NodeEditor/StyleCollection.hpp
  )

set(qtheaders
  ArgumentDialog.h
  DecisionDialog.h
  ExecEnvDialog.h
  FlowSearchDialog.h
  FlowView.h
  ModelDialog.h
  ModelMasterDialog.h
  MetaDataView.h
  ParameterDialog.h
  ParameterView.h
  SettingDialog.h
  StateMachineView.h
  TaskInstanceView.h

  NodeEditor/ActivityEditorBase.hpp
  NodeEditor/Node.hpp
  NodeEditor/FlowScene.hpp
  NodeEditor/NodeDataModel.hpp
  NodeEditor/Connection.hpp
  NodeEditor/ParamWidget.hpp
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

target_compile_definitions(${libname} PUBLIC "-DNODE_EDITOR_STATIC")

apply_common_setting_for_plugin(${libname}  "${headers}")
