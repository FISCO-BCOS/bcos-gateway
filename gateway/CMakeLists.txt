file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_library(${BCOS_GATEWAY_TARGET} ${SRC_LIST} ${HEADERS})
target_link_libraries(${BCOS_GATEWAY_TARGET} PUBLIC bcos-framework::utilities)
target_compile_options(${BCOS_GATEWAY_TARGET} PRIVATE -Wno-error -Wno-unused-variable)
