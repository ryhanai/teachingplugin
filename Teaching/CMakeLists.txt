option(BUILD_TEACHING_PLUGIN "Building Teaching plugin" ON)
option(USE_CPP_CONROLLER "Use C ++ version controller" OFF)

if(NOT BUILD_TEACHING_PLUGIN)
  return()
else()
  if(NOT BUILD_PYTHON_PLUGIN)
    message(FATAL_ERROR "The Teaching plugin requires the Python plugin")
  endif()
endif()

if(USE_CPP_CONROLLER)
  add_definitions(-DUSE_CPP_CONROLLER)
endif()

set(libname CnoidTeachingPlugin)

if(NOT ENABLE_PYTHON)
  find_package(PythonLibs REQUIRED)
endif()

set(sources
  ArgumentDialog.cpp
  ChoreonoidUtil.cpp
  ControllerManager.cpp
  DataBaseManager.cpp
  DataBaseDetailManager.cpp
  DataBaseFlowManager.cpp
  DataBaseMasterManager.cpp
  DecisionDialog.cpp
  FlowView.cpp
  LoggerUtil.cpp
  MetaDataView.cpp
  ModelDialog.cpp
  ModelMasterDialog.cpp
  ParameterDialog.cpp
  ParameterView.cpp
  SettingDialog.cpp
  StateMachineView.cpp
  TaskExecuteManager.cpp
  TaskExecutor.cpp
  TaskInstanceView.cpp
  TeachingDataHolder.cpp
  TeachingEventHandler.cpp
  TeachingFlowEventHandler.cpp
  TeachingDialogEventHandler.cpp
  TeachingMasterEventHandler.cpp
  TeachingPlugin.cpp
  TeachingTypes.cpp
  TeachingUtil.cpp
  TeachingUtilFlow.cpp
  TeachingUtilTask.cpp
  TrajectoryView.cpp

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

  TeachingFlowEventHandlerForTest.cpp
)

set(headers
  ArgumentDialog.h
  Parser.h
  ChoreonoidUtil.h
  ControllerBase.h
  CommandDefTypes.h
  ControllerManager.h
  DataBaseManager.h
  DecisionDialog.h
  FlowView.h
  LoggerUtil.h
  MetaDataView.h
  ModelDialog.h
  ModelMasterDialog.h
  ParameterDialog.h
  ParameterView.h
  SettingDialog.h
  StateMachineView.h
  TaskExecuteManager.h
  TaskExecutor.h
  TaskInstanceView.h
  TeachingDataHolder.h
  TeachingEventHandler.h
  TeachingMasterEventHandler.h
  TeachingTypes.h
  TeachingUtil.h
  TrajectoryView.h
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
  FlowView.h
  ModelDialog.h
  ModelMasterDialog.h
  MetaDataView.h
  ParameterDialog.h
  ParameterView.h
  SettingDialog.h
  StateMachineView.h
  TaskInstanceView.h
  TrajectoryView.h

  NodeEditor/ActivityEditorBase.hpp
  NodeEditor/Node.hpp
  NodeEditor/FlowScene.hpp
  NodeEditor/NodeDataModel.hpp
  NodeEditor/Connection.hpp
  NodeEditor/ParamWidget.hpp
  )

if(USE_CPP_CONROLLER)
  set(sources ${sources}
	Calculator.cpp
  )
  set(headers ${headers}
    Calculator.h
  )
else()
  set(sources ${sources}
    PythonControllerWrapper.cpp
    PythonWrapper.cpp
  )
  set(headers ${headers}
    PythonControllerWrapper.h
    PythonWrapper.h
  )
endif()

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

target_link_libraries(${libname} CnoidBodyPlugin CnoidPythonPlugin CnoidPyUtil ${PYTHON_LIBRARIES})
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

if(ENABLE_PYTHON)
  add_subdirectory(pybind11)
endif()
