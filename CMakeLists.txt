# CMakeList.txt: проект CMake для mover_2; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

project ("mover_2")
set(Boost_USE_STATIC_LIBS        OFF)  # only find static libs
set(Boost_USE_DEBUG_LIBS        ON)  # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       OFF)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost REQUIRED)# Добавьте источник в исполняемый файл этого проекта.
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
# Добавьте источник в исполняемый файл этого проекта.
add_executable (mover_2 "mover_2.cpp" "mover_2.h")
add_compile_options(/await:strict)
target_link_libraries(mover_2 PRIVATE Boost::boost )
target_link_libraries(mover_2 PRIVATE Threads::Threads)
target_compile_features(mover_2 PRIVATE cxx_std_17)
target_compile_options(mover_2 PRIVATE /GR- /await /wd4238)
# TODO: Добавьте тесты и целевые объекты, если это необходимо.
