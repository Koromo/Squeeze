set(TEST_SOURCES clazz.cpp
                 main.cpp
                 script.cpp
                 table.cpp)

include_directories(SYSTEM ${SQUEEZE_INCLUDE_DIR} ${SQUIRREL_INCLUDE_DIR} ${CPPUTEST_INCLUDE_DIR})
link_directories(${SQUIRREL_LIB_DIR} ${CPPUTEST_LIB_DIR})

add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests squirrel sqstdlib cpputest cpputestext winmm)

install(TARGETS tests RUNTIME DESTINATION bin)
install(FILES test.nut DESTINATION bin)