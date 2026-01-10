cmake_minimum_required(VERSION 3.20)
project(sw_battle_test LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 1. Collect all sources
file(GLOB_RECURSE SW_SOURCES CONFIGURE_DEPENDS
	src/*.cpp
	src/*.hpp
)

# 2. Main App (uses everything)
add_executable(sw_battle_test ${SW_SOURCES})
target_compile_features(sw_battle_test PRIVATE cxx_std_20)
target_include_directories(sw_battle_test PUBLIC src/)

# --- Tests (no 3rd-party deps) ---
include(CTest) # defines BUILD_TESTING and enables CTest integration
if(BUILD_TESTING)
	# Prepare sources for unit tests: remove main.cpp to avoid duplicate entry point
	set(TEST_SOURCES ${SW_SOURCES})
	list(FILTER TEST_SOURCES EXCLUDE REGEX "main\\.cpp$")

	# 1. Unit Tests (linked against all core/features code)
	add_executable(sw_battle_unit_tests
		tests/unit_tests.cpp
		${TEST_SOURCES}
	)
	target_compile_features(sw_battle_unit_tests PRIVATE cxx_std_20)
	target_include_directories(sw_battle_unit_tests PUBLIC src/)
	add_test(NAME sw_battle_unit_tests COMMAND sw_battle_unit_tests)

	# 2. Integration Smoke Test
	add_test(NAME smoke_test_example COMMAND $<TARGET_FILE:sw_battle_test> ${CMAKE_CURRENT_SOURCE_DIR}/commands_example.txt)

	# 3. Deterministic Integration Test
	add_test(NAME integration_test_combat COMMAND $<TARGET_FILE:sw_battle_test> ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration_scenario.txt)
	
	set_tests_properties(integration_test_combat PROPERTIES 
		PASS_REGULAR_EXPRESSION "UNIT_ATTACKED attackerUnitId=1 targetUnitId=2(.|\n)*UNIT_DIED unitId=2"
	)
endif()
