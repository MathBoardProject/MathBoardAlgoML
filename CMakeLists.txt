cmake_minimum_required(VERSION 3.11.0)
project(mathboard VERSION 0.0.1 LANGUAGES C CXX)

cmake_policy(SET CMP0135 NEW)

enable_testing()

include(CTest)
include(CheckCXXCompilerFlag)
include(FetchContent)

# Enable flag if supported
function(enable_cxx_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" flag_already_set)
    if(flag_already_set EQUAL -1)
        check_cxx_compiler_flag("${flag}" flag_supported)
        if(flag_supported)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
        endif()
        unset(flag_supported CACHE)
    endif()
endfunction()

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Gather source and header files
file (GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file (GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "src/*.hpp")
file (GLOB_RECURSE TESTS CONFIGURE_DEPENDS "tests/*.cpp")

# Check if OpenCV is installed
find_package(OpenCV REQUIRED)

# Fetch GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Fetch spdlog
FetchContent_Declare(
      spdlog
      GIT_REPOSITORY https://github.com/gabime/spdlog.git
      GIT_TAG v1.15.0
)
FetchContent_MakeAvailable(spdlog)

# Enable extra warnings
enable_cxx_compiler_flag_if_supported("-Wall")
enable_cxx_compiler_flag_if_supported("-Wextra")
enable_cxx_compiler_flag_if_supported("-pedantic")

# Add project binary
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Add include directories
target_include_directories(${PROJECT_NAME}
	PRIVATE src
	PRIVATE ${OpenCV_INCLUDE_DIRS}
    PRIVATE spdlog::spdlog
)

# Add libraries to link
target_link_libraries(${PROJECT_NAME}
	PRIVATE ${OpenCV_LIBS}
    PRIVATE spdlog::spdlog
)

# Add MathBoard as a static library
# Needed for testing
add_library(${PROJECT_NAME}_lib STATIC ${SOURCES} ${HEADERS})

# Add include directories
target_include_directories(${PROJECT_NAME}_lib
	PRIVATE src
	PRIVATE ${OpenCV_INCLUDE_DIRS}
    PRIVATE spdlog::spdlog
)

# Add libraries to link
target_link_libraries(${PROJECT_NAME}_lib
	PRIVATE ${OpenCV_LIBS}
    PRIVATE spdlog::spdlog
)


# Add tests binary
add_executable(tests ${TESTS})

# Link GoogleTest and MathBoard library
target_link_libraries(tests
	PRIVATE GTest::gtest_main
    PRIVATE mathboard_lib
)

include(GoogleTest)
gtest_discover_tests(tests)
