cmake_minimum_required(VERSION 3.30)

project(Differentiator CXX)

set(CMAKE_CXX_FLAGS_DEBUG "\
-pthread -g -D _DEBUG -O0 -Wall -Wextra -Wno-pointer-arith \
-fsanitize=address,undefined,leak,shift,float-divide-by-zero,signed-integer-overflow\
")
set(CMAKE_CXX_FLAGS_RELEASE "-pthread -O3 -g -DNDEBUG -march=native")

set(SOURCES
    "src/Differentiator.cpp"
    "src/LatexWriter.cpp"
    "src/main.cpp"
    "src/Optimiser.cpp"
    "src/RecursiveDescent.cpp"
    "src/Sort.cpp"
    "src/StringFunctions.cpp"
    "src/Tree.cpp"
    "src/Utils.cpp"
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE headers/)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/tex)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/log)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/log/dot)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/log/img)
