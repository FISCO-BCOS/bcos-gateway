hunter_config(bcos-framework VERSION 3.0.0-local
	URL https://${URL_BASE}/FISCO-BCOS/bcos-framework/archive/b8dcb0547d3dd38380a92cac6a28e1514f3987f4.tar.gz
	SHA1 bec196b369ff41e1d9560b670cbb235a8947b5d1
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
    URL https://${URL_BASE}/FISCO-BCOS/bcos-tars-protocol/archive/077867b87163f9805d6fb116f3528dda1eaaf368.tar.gz
    SHA1 89dab12040b706649bee4f5dcbffa32a247a5284
    CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON URL_BASE=${URL_BASE}
)

hunter_config(bcos-front VERSION 3.0.0-c8ee499
        URL https://${URL_BASE}/FISCO-BCOS/bcos-front/archive/10880d313161ae12e6e5d5bc63876fd274c4bb7c.tar.gz
        SHA1 "e9c0cd550f411a5f577f9e2e97ad395d502b2fb3"
        CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON
)