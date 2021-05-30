file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

set(BCOS_GATE_WAY_EXEC_TARGET "gate-way-exec-test")
add_executable(${BCOS_GATE_WAY_EXEC_TARGET} ${SRC_LIST})
target_link_libraries(${BCOS_GATE_WAY_EXEC_TARGET} PUBLIC  ${BCOS_GATEWAY_TARGET} ${RPC_TARGET} ${NETWORK_TARGET} bcos-framework::utilities OpenSSL::SSL OpenSSL::Crypto)
target_compile_options(${BCOS_GATE_WAY_EXEC_TARGET} PRIVATE -Wno-error -Wno-unused-variable)
