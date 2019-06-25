find_package(Mint)
set_package_properties(Mint PROPERTIES
    TYPE OPTIONAL
    DESCRIPTION "The Machine Integration Test Framework"
    URL https://github.hpe.com/labs/holodeck
    PURPOSE "Required to build the integration tests"
)

if (NOT MINT_FOUND) 
  message(STATUS "WARNING: mint library not found, hence disabling integration tests.")
  return()
endif()


include_directories(${MINT_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/test)
include_directories(${PROJECT_SOURCE_DIR}/test/integration)

function (add_alps_integration_test targetname)
  add_executable(integration_${targetname} ${CMAKE_CURRENT_SOURCE_DIR}/${targetname}.cc)
  set_target_properties(integration_${targetname} PROPERTIES OUTPUT_NAME ${targetname})
  target_link_libraries(integration_${targetname} pthread ${MINT_LIBRARY} alps gflags gtest)
  add_test(NAME integration_${targetname} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${targetname} CONFIGURATIONS integration)
  install (TARGETS integration_${targetname} DESTINATION bin/test/integration COMPONENT test)
endfunction()


#add_alps_integration_test(test_dummy)
add_alps_integration_test(test_atomic)
add_alps_integration_test(test_pegas)
add_alps_integration_test(test_pointer)
add_alps_integration_test(test_topology)
