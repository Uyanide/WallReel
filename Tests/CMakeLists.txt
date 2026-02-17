
project(Tests LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Test)

add_executable(tst_configmgr
    tst_configmgr.cpp
)

add_executable(tst_imagemodel
    tst_imagemodel.cpp
)

add_test(NAME tst_configmgr COMMAND tst_configmgr)
add_test(NAME tst_imagemodel COMMAND tst_imagemodel)

target_link_libraries(tst_configmgr PRIVATE
    Qt6::Core
    Qt6::Test
    Qt6::Gui
    wallreel-core
)

target_link_libraries(tst_imagemodel PRIVATE
    Qt6::Core
    Qt6::Test
    Qt6::Gui
    Qt6::Quick
    wallreel-core
)

target_include_directories(tst_configmgr PRIVATE
    ${CMAKE_SOURCE_DIR}/WallReel/Core
    ${CMAKE_BINARY_DIR}/generated
)

target_include_directories(tst_imagemodel PRIVATE
    ${CMAKE_SOURCE_DIR}/WallReel/Core
    ${CMAKE_BINARY_DIR}/generated
)
