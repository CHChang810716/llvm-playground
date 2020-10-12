hunter_add_package(Arkitekto)
find_package(Arkitekto CONFIG REQUIRED)
if(BUILD_TEST)
    hunter_add_package(GTest)
    find_package(GTest CONFIG REQUIRED)
endif()
find_package(Threads REQUIRED)

hunter_add_package(LLVM)
find_package(LLVM CONFIG REQUIRED)
llvm_map_components_to_libnames(llvm_libs support core irreader)

hunter_add_package(pegtl)
find_package(pegtl CONFIG REQUIRED)