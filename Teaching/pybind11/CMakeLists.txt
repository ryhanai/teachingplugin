set(target PyTeachingPlugin)

set(sources
  PyTeachingPlugin.cpp
  TPUtil.cpp
  )

add_cnoid_python_module(${target} ${sources})
target_link_libraries(${target} CnoidTeachingPlugin CnoidPyBase)
