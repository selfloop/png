set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB ReductionRules_SRC "*.cpp")

add_library(ReductionRules ${ReductionRules_SRC})
add_dependencies(ReductionRules ptrie-ext)

target_link_libraries(ReductionRules)
