
project(Tests LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(tst_configmgr
    tst_configmgr.cpp
)

add_executable(tst_imagemodel
    tst_imagemodel.cpp
)

add_test(NAME tst_configmgr COMMAND tst_configmgr)
add_test(NAME tst_imagemodel COMMAND tst_imagemodel)

target_link_libraries(tst_configmgr PRIVATE
    Qt6::Test
    ${CORELIB_NAME}
)

target_link_libraries(tst_imagemodel PRIVATE
    Qt6::Test
    ${CORELIB_NAME}
)
