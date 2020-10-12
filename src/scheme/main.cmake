include(cmake/scheme/exe.cmake)
target_link_libraries(${AKT_TARGET} PRIVATE ${llvm_libs})