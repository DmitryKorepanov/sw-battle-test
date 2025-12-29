cmake_minimum_required(VERSION 3.20)
project(sw_battle_test LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Keep it simple: auto-collect sources for this small project.
# CONFIGURE_DEPENDS makes CMake re-run when files are added/removed.
file(GLOB_RECURSE SW_SOURCES CONFIGURE_DEPENDS
	src/*.cpp
	src/*.hpp
)

add_executable(sw_battle_test ${SW_SOURCES})
target_compile_features(sw_battle_test PRIVATE cxx_std_20)
target_include_directories(sw_battle_test PUBLIC src/)

# --- Tests (no 3rd-party deps) ---
include(CTest) # defines BUILD_TESTING and enables CTest integration
if(BUILD_TESTING)
	add_executable(sw_battle_unit_tests
		tests/unit_tests.cpp
		src/Core/GameWorld.cpp
	)
	target_compile_features(sw_battle_unit_tests PRIVATE cxx_std_20)
	target_include_directories(sw_battle_unit_tests PUBLIC src/)
	add_test(NAME sw_battle_unit_tests COMMAND sw_battle_unit_tests)
endif()
