hunter_config(bcos-framework VERSION 3.0.0-local
	URL https://${URL_BASE}/cyjseagull/bcos-framework/archive/e8c005f1d4868030f21d23101d41f3b3f408a82b.tar.gz
	SHA1 f0f85d44f66ff343fddeaa871ce3d15540a4c201
	CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON HUNTER_KEEP_PACKAGE_SOURCES=ON
)

hunter_config(bcos-crypto
    VERSION 3.0.0-local
    URL https://${URL_BASE}/FISCO-BCOS/bcos-crypto/archive/255002b047b359a45c953d1dab29efd2ff6eb080.tar.gz
    SHA1 4d02de20be1f9bf79d762c5b8686368286504e07
    CMAKE_ARGS URL_BASE=${URL_BASE}
)

hunter_config(bcos-tars-protocol
    VERSION 3.0.0-local
    URL https://${URL_BASE}/FISCO-BCOS/bcos-tars-protocol/archive/126c8e86e1eec5313d9c8b4f5c5d1e2731a98401.tar.gz
    SHA1 cb341ff805e78cb91b6d626d8607ff51e733571b
    CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON URL_BASE=${URL_BASE}
)

hunter_config(bcos-front VERSION 3.0.0-c8ee499
        URL https://${URL_BASE}/FISCO-BCOS/bcos-front/archive/10880d313161ae12e6e5d5bc63876fd274c4bb7c.tar.gz
        SHA1 "e9c0cd550f411a5f577f9e2e97ad395d502b2fb3"
        CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON
)