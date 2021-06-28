set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_library(${NETWORK_TARGET} ${SRC_LIST} ${HEADERS})

target_link_libraries(${NETWORK_TARGET} PUBLIC bcos-framework::utilities)
target_compile_options(${NETWORK_TARGET} PRIVATE -Wno-error -Wno-unused-variable)