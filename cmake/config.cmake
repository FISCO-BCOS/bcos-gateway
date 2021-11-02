hunter_config(bcos-framework VERSION 3.0.0-local
	URL https://${URL_BASE}/FISCO-BCOS/bcos-framework/archive/79a6e580c61a1add5fabea51b10e1524c2cdc972.tar.gz
    SHA1 ee17224b750a4c2ae53884e847c13d2eff028364
	CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON
)

hunter_config(bcos-crypto
	VERSION 3.0.0-local
	URL https://${URL_BASE}/FISCO-BCOS/bcos-crypto/archive/25c8edb7d5cbadb514bbce9733573c8ffdb3600d.tar.gz
	SHA1 4a1649e7095f5db58a5ae0671b2278bcccc25f1d
)

hunter_config(bcos-tars-protocol
    VERSION 3.0.0-local
    URL https://${URL_BASE}/FISCO-BCOS/bcos-tars-protocol/archive/325b1c6971e3f42f527996cdf990b722a5eb4d50.tar.gz
    SHA1 06b904725472b1063c6cfc34894cd1979a22ad8a
    CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON URL_BASE=${URL_BASE}
)

hunter_config(bcos-front VERSION 3.0.0-c8ee499
        URL https://${URL_BASE}/FISCO-BCOS/bcos-front/archive/10880d313161ae12e6e5d5bc63876fd274c4bb7c.tar.gz
        SHA1 "e9c0cd550f411a5f577f9e2e97ad395d502b2fb3"
        CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON
)