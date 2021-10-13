hunter_config(bcos-framework VERSION 3.0.0-local
    URL https://${URL_BASE}/FISCO-BCOS/bcos-framework/archive/e0a42cfd439e08f6693765682a4f0380399e726a.tar.gz
    SHA1 0a1ccf27f097dc8c3c762aea87cfd3de1381e315
    CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON #DEBUG=ON
)


hunter_config(bcos-crypto
    VERSION 3.0.0-local
    URL https://${URL_BASE}/FISCO-BCOS/bcos-crypto/archive/255002b047b359a45c953d1dab29efd2ff6eb080.tar.gz
    SHA1 4d02de20be1f9bf79d762c5b8686368286504e07
    CMAKE_ARGS URL_BASE=${URL_BASE}
)

hunter_config(bcos-tars-protocol
    VERSION 3.0.0-local
    URL  https://${URL_BASE}/FISCO-BCOS/bcos-tars-protocol/archive/4cc312d213256bc173eea72873c5237b9714c009.tar.gz
    SHA1 a6a18e6133d97e141341a55e947ad60f04be1bc0
    CMAKE_ARGS URL_BASE=${URL_BASE}
)

hunter_config(bcos-front VERSION 3.0.0-c8ee499
        URL https://${URL_BASE}/FISCO-BCOS/bcos-front/archive/10880d313161ae12e6e5d5bc63876fd274c4bb7c.tar.gz
        SHA1 "e9c0cd550f411a5f577f9e2e97ad395d502b2fb3"
        CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON
)