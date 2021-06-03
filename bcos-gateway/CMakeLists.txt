file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_subdirectory(libnetwork)
add_subdirectory(libp2p)

add_library(${BCOS_GATEWAY_TARGET} ${SRC_LIST} ${HEADERS})
target_link_libraries(${BCOS_GATEWAY_TARGET} PUBLIC bcos-framework::utilities jsoncpp_lib_static OpenSSL::SSL OpenSSL::Crypto ${NETWORK_TARGET} ${P2P_TARGET})
target_compile_options(${BCOS_GATEWAY_TARGET} PRIVATE -Wno-error -Wno-unused-variable)
