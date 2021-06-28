set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_library(${P2P_TARGET} ${SRC_LIST} ${HEADERS})

target_link_libraries(${P2P_TARGET} PUBLIC bcos-framework::utilities ${NETWORK_TARGET})
target_compile_options(${P2P_TARGET} PRIVATE -Wno-error -Wno-unused-variable)
