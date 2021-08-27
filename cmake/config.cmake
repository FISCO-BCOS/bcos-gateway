hunter_config(bcos-framework VERSION 3.0.0-local
        URL "https://${URL_BASE}/FISCO-BCOS/bcos-framework/archive/65b1f4857369158bba7d5b0bb9d86c2c4d69d457.tar.gz"
        SHA1 "abae4c396c3a0a1615fd32961e35c7535e181a90"
        CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON
)

hunter_config(bcos-front VERSION 3.0.0-c8ee499
        URL "https://${URL_BASE}/FISCO-BCOS/bcos-front/archive/15e18804aab90def4c2ba7d811024df921f935de.tar.gz"
        SHA1 "1485c64a31b106f912aa1d1878da5d91dc0a2975"
        CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON
)

hunter_config(bcos-crypto VERSION 3.0.0-local
        URL https://${URL_BASE}/FISCO-BCOS/bcos-crypto/archive/25c8edb7d5cbadb514bbce9733573c8ffdb3600d.tar.gz
        SHA1 4a1649e7095f5db58a5ae0671b2278bcccc25f1d
        CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=ON HUNTER_PACKAGE_LOG_INSTALL=ON
)

hunter_config(wedpr-crypto VERSION 1.1.0-10f314de
	URL https://${URL_BASE}/WeBankBlockchain/WeDPR-Lab-Crypto/archive/10f314de45ec31ce9e330922b522ce173662ed33.tar.gz
	SHA1 626df59f87ea2c6bb5128f7d104588179809910b
	CMAKE_ARGS HUNTER_PACKAGE_LOG_BUILD=OFF HUNTER_PACKAGE_LOG_INSTALL=ON
)