add_library(physics)

add_subdirectory(systems)
add_subdirectory(components)

target_sources(physics 
PRIVATE
  Mathematics.cpp
  Mathematics.h
)

target_include_directories(physics 
PUBLIC
  ${PROJECT_SOURCE_DIR}
PRIVATE 
  ${GLM_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/pilcrow/dependencies
)

target_link_libraries(physics 
PUBLIC
  engine
  slot_map 
  detection_idiom
PRIVATE 
)

#The filter the library will be in
set_target_properties(physics PROPERTIES FOLDER pilcrow/modules)

pilcrow_source_group(CMAKE_CURRENT_LIST_DIR physics)
pilcrow_create_reflection_target(physics)
