set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(ReductionRules ReductionRule.cpp ReductionRuleA.cpp ReductionRuleB.cpp ReductionRuleC.cpp)
add_dependencies(ReductionRules ptrie-ext)

target_link_libraries(ReductionRules)
