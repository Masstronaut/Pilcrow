add_library(shmup)

target_sources(shmup 
PRIVATE
  Source.cpp
  Source.hpp
)

target_include_directories(shmup 
PUBLIC
  ${PROJECT_SOURCE_DIR}
PRIVATE 
  ${GLM_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/pilcrow/dependencies
)

target_link_libraries(shmup 
PUBLIC
  engine
  slot_map 
  detection_idiom
PRIVATE 
)

#The filter the library will be in
set_target_properties(shmup PROPERTIES FOLDER pilcrow/modules)

pilcrow_source_group(CMAKE_CURRENT_LIST_DIR shmup)
pilcrow_create_reflection_target(shmup)
