/**
 *  Copyright (C) 2021 FISCO BCOS.
 *  SPDX-License-Identifier: Apache-2.0
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * @brief test for gateway
 * @file GatewayConfigTest.cpp
 * @author: octopus
 * @date 2021-05-17
 */

#include "gateway/GatewayConfig.h"
#include "gateway/GatewayFactory.h"
#include <bcos-framework/testutils/TestPromptFixture.h>
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

using namespace bcos;
using namespace gateway;
using namespace bcos::test;

BOOST_FIXTURE_TEST_SUITE(GatewayConfigTest, TestPromptFixture)

BOOST_AUTO_TEST_CASE(test_validPort) {
  auto config = std::make_shared<GatewayConfig>();
  BOOST_CHECK(!config->isValidPort(1024));
  BOOST_CHECK(!config->isValidPort(65536));
  BOOST_CHECK(config->isValidPort(30300));
}

BOOST_AUTO_TEST_CASE(test_hostAndPort2Endpoint) {
  auto config = std::make_shared<GatewayConfig>();

  {
    NodeIPEndpoint endpoint;
    BOOST_CHECK_NO_THROW(
        config->hostAndPort2Endpoint("127.0.0.1:1111", endpoint));
    BOOST_CHECK_EQUAL(endpoint.address(), "127.0.0.1");
    BOOST_CHECK_EQUAL(endpoint.port(), 1111);
    BOOST_CHECK(!endpoint.isIPv6());
  }

  {
    NodeIPEndpoint endpoint;
    BOOST_CHECK_NO_THROW(config->hostAndPort2Endpoint("[::1]:1234", endpoint));
    BOOST_CHECK_EQUAL(endpoint.address(), "::1");
    BOOST_CHECK_EQUAL(endpoint.port(), 1234);
    BOOST_CHECK(endpoint.isIPv6());
  }

  {
    NodeIPEndpoint endpoint;
    BOOST_CHECK_NO_THROW(
        config->hostAndPort2Endpoint("8.129.188.218:12345", endpoint));
    BOOST_CHECK_EQUAL(endpoint.address(), "8.129.188.218");
    BOOST_CHECK_EQUAL(endpoint.port(), 12345);
    BOOST_CHECK(!endpoint.isIPv6());
  }

  {
    NodeIPEndpoint endpoint;
    BOOST_CHECK_NO_THROW(config->hostAndPort2Endpoint(
        "[fe80::1a9d:50ae:3207:80d9]:54321", endpoint));
    BOOST_CHECK_EQUAL(endpoint.address(), "fe80::1a9d:50ae:3207:80d9");
    BOOST_CHECK_EQUAL(endpoint.port(), 54321);
    BOOST_CHECK(endpoint.isIPv6());
  }

  {
    NodeIPEndpoint endpoint;
    BOOST_CHECK_THROW(config->hostAndPort2Endpoint("abcdef:fff", endpoint),
                      std::exception);
  }
}

BOOST_AUTO_TEST_CASE(test_nodesJsonParser) {
  {
    std::string json = "{\"nodes\":[\"127.0.0.1:30300\",\"127.0.0.1:30301\","
                       "\"127.0.0.1:30302\"]}";
    auto config = std::make_shared<GatewayConfig>();
    std::set<NodeIPEndpoint> nodeIPEndpointSet;
    config->parseConnectedJson(json, nodeIPEndpointSet);
    BOOST_CHECK_EQUAL(nodeIPEndpointSet.size(), 3);
    BOOST_CHECK_EQUAL(config->threadPoolSize(), 16);
  }

  {
    std::string json = "{\"nodes\":[]}";
    auto config = std::make_shared<GatewayConfig>();
    std::set<NodeIPEndpoint> nodeIPEndpointSet;
    config->parseConnectedJson(json, nodeIPEndpointSet);
    BOOST_CHECK_EQUAL(nodeIPEndpointSet.size(), 0);
    BOOST_CHECK_EQUAL(config->threadPoolSize(), 16);
  }

  {
    std::string json = "{\"nodes\":[\"["
                       "fe80::1a9d:50ae:3207:80d9]:30302\","
                       "\"[fe80::1a9d:50ae:3207:80d9]:30303\"]}";
    auto config = std::make_shared<GatewayConfig>();
    std::set<NodeIPEndpoint> nodeIPEndpointSet;
    config->parseConnectedJson(json, nodeIPEndpointSet);
    BOOST_CHECK_EQUAL(nodeIPEndpointSet.size(), 2);
    BOOST_CHECK_EQUAL(config->threadPoolSize(), 16);
  }
}

BOOST_AUTO_TEST_CASE(test_nodesJsonFile) {
  {
    std::string jsonFile("../test/unittests/data/config/json/nodes_ipv4.json");
    auto config = std::make_shared<GatewayConfig>();
    std::set<NodeIPEndpoint> nodeIPEndpointSet;
    config->parseConnectedFile(jsonFile, nodeIPEndpointSet);
    BOOST_CHECK_EQUAL(nodeIPEndpointSet.size(), 3);
  }

  {
    std::string jsonFile("../test/unittests/data/config/json/nodes_ipv6.json");
    auto config = std::make_shared<GatewayConfig>();
    std::set<NodeIPEndpoint> nodeIPEndpointSet;
    config->parseConnectedFile(jsonFile, nodeIPEndpointSet);
    BOOST_CHECK_EQUAL(nodeIPEndpointSet.size(), 1);
  }
}

BOOST_AUTO_TEST_CASE(test_initConfig) {
  {
    std::string configIni("../test/unittests/data/config/config_ipv4.ini");
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(configIni, pt);
    auto config = std::make_shared<GatewayConfig>();
    config->initP2PConfig(pt);
    BOOST_CHECK_EQUAL(config->listenIP(), "127.0.0.1");
    BOOST_CHECK_EQUAL(config->listenPort(), 12345);
    BOOST_CHECK_EQUAL(config->smSSL(), false);
    BOOST_CHECK_EQUAL(config->connectedNodes().size(), 3);
  }
}

BOOST_AUTO_TEST_CASE(test_initSMConfig) {
  {
    std::string configIni("../test/unittests/data/config/config_ipv6.ini");

    auto config = std::make_shared<GatewayConfig>();
    config->initConfig(configIni);
    BOOST_CHECK_EQUAL(config->listenIP(), "0.0.0.0");
    BOOST_CHECK_EQUAL(config->listenPort(), 54321);
    BOOST_CHECK_EQUAL(config->smSSL(), true);
    BOOST_CHECK_EQUAL(config->connectedNodes().size(), 1);

    auto smCertConfig = config->smCertConfig();
    BOOST_CHECK_EQUAL(smCertConfig.caCert, R"(-----BEGIN CERTIFICATE-----
MIIBxDCCAWqgAwIBAgIJAMaiS2o+R1VyMAoGCCqBHM9VAYN1MDcxEDAOBgNVBAMM
B2dtY2hhaW4xEzARBgNVBAoMCmZpc2NvLWJjb3MxDjAMBgNVBAsMBWNoYWluMCAX
DTIxMDUxNzA3NDIwNVoYDzIxMjEwNDIzMDc0MjA1WjA3MRAwDgYDVQQDDAdnbWNo
YWluMRMwEQYDVQQKDApmaXNjby1iY29zMQ4wDAYDVQQLDAVjaGFpbjBZMBMGByqG
SM49AgEGCCqBHM9VAYItA0IABCe6kA8npZcBY8KVS1tQj97ODAkD7UcDcLK0pM1l
c8fI79eycQREoz+swi78cbthIybZbfDe3kElRxI4tgx+oPejXTBbMB0GA1UdDgQW
BBS+JKufsNiQc6jVDReJfORM95bl2jAfBgNVHSMEGDAWgBS+JKufsNiQc6jVDReJ
fORM95bl2jAMBgNVHRMEBTADAQH/MAsGA1UdDwQEAwIBBjAKBggqgRzPVQGDdQNI
ADBFAiEAxKkhjgoVeEh5i/IGpi8BmhF60DvTWUDxT32tGo1sUpwCIEbyBhHkjrsN
/EWiHcN1d/A2HP6F4N5CaHFCTRYkR7x+
-----END CERTIFICATE-----)");
    BOOST_CHECK_EQUAL(smCertConfig.nodeCert, R"(-----BEGIN CERTIFICATE-----
MIIBgjCCASigAwIBAgIJAK9b6T7ApKESMAoGCCqBHM9VAYN1MDsxEzARBgNVBAMM
CmFnZW5jeV9zb24xEzARBgNVBAoMCmZpc2NvLWJjb3MxDzANBgNVBAsMBmFnZW5j
eTAgFw0yMTA1MTcwNzQyMDZaGA8yMTIxMDQyMzA3NDIwNlowNDEOMAwGA1UEAwwF
bm9kZTAxEzARBgNVBAoMCmZpc2NvLWJjb3MxDTALBgNVBAsMBG5vZGUwWTATBgcq
hkjOPQIBBggqgRzPVQGCLQNCAATrj6G46y7Hk5Vlsi1E9ejx7Owu4GY2EfOgAdgq
+nc2eEgHgy6QHQ+/i0yt72OtC/vIq7qn1R34OWbosQ0IfKhhoxowGDAJBgNVHRME
AjAAMAsGA1UdDwQEAwIGwDAKBggqgRzPVQGDdQNIADBFAiEAh7qWC87kPxRVEnWS
asxTZeBQg1GM+Wu3fyZaRBwH7gICIFQihopLY8agGsMPrzKzWIj2hc+cMbbQg8S4
NrWyQ3Na
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
MIIBxTCCAWygAwIBAgIJAIfy0kiOnylLMAoGCCqBHM9VAYN1MDcxEDAOBgNVBAMM
B2dtY2hhaW4xEzARBgNVBAoMCmZpc2NvLWJjb3MxDjAMBgNVBAsMBWNoYWluMB4X
DTIxMDUxNzA3NDIwNVoXDTMxMDUxNTA3NDIwNVowOzETMBEGA1UEAwwKYWdlbmN5
X3NvbjETMBEGA1UECgwKZmlzY28tYmNvczEPMA0GA1UECwwGYWdlbmN5MFkwEwYH
KoZIzj0CAQYIKoEcz1UBgi0DQgAElVnsinyUfoX2eF0zkpy7wYq44DAbS+y+odoa
K8J0o/QZmtNYwHvw3LDVKdxoJ6Iq4cxsjiD9+1mPtI9AO7yT9aNdMFswHQYDVR0O
BBYEFIpzyHUcpLykGa1QmD3L3g43E9wIMB8GA1UdIwQYMBaAFL4kq5+w2JBzqNUN
F4l85Ez3luXaMAwGA1UdEwQFMAMBAf8wCwYDVR0PBAQDAgEGMAoGCCqBHM9VAYN1
A0cAMEQCICZfGr2Mi5rdpz7FJRGPkvzv0exSzvFf/DvY8jF+xkLnAiB+Rj7n70hz
nDajYP39P7rj/ZLPC6tr+At2ETQUsf4KiA==
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
MIIBxDCCAWqgAwIBAgIJAMaiS2o+R1VyMAoGCCqBHM9VAYN1MDcxEDAOBgNVBAMM
B2dtY2hhaW4xEzARBgNVBAoMCmZpc2NvLWJjb3MxDjAMBgNVBAsMBWNoYWluMCAX
DTIxMDUxNzA3NDIwNVoYDzIxMjEwNDIzMDc0MjA1WjA3MRAwDgYDVQQDDAdnbWNo
YWluMRMwEQYDVQQKDApmaXNjby1iY29zMQ4wDAYDVQQLDAVjaGFpbjBZMBMGByqG
SM49AgEGCCqBHM9VAYItA0IABCe6kA8npZcBY8KVS1tQj97ODAkD7UcDcLK0pM1l
c8fI79eycQREoz+swi78cbthIybZbfDe3kElRxI4tgx+oPejXTBbMB0GA1UdDgQW
BBS+JKufsNiQc6jVDReJfORM95bl2jAfBgNVHSMEGDAWgBS+JKufsNiQc6jVDReJ
fORM95bl2jAMBgNVHRMEBTADAQH/MAsGA1UdDwQEAwIBBjAKBggqgRzPVQGDdQNI
ADBFAiEAxKkhjgoVeEh5i/IGpi8BmhF60DvTWUDxT32tGo1sUpwCIEbyBhHkjrsN
/EWiHcN1d/A2HP6F4N5CaHFCTRYkR7x+
-----END CERTIFICATE-----)");
    BOOST_CHECK_EQUAL(smCertConfig.nodeKey, R"(-----BEGIN PRIVATE KEY-----
MIGHAgEAMBMGByqGSM49AgEGCCqBHM9VAYItBG0wawIBAQQgXu6OwQzuwTSw/e6w
xbbW+mL7WwXuTH+KZ839oNm2nfihRANCAATrj6G46y7Hk5Vlsi1E9ejx7Owu4GY2
EfOgAdgq+nc2eEgHgy6QHQ+/i0yt72OtC/vIq7qn1R34OWbosQ0IfKhh
-----END PRIVATE KEY-----)");
    BOOST_CHECK_EQUAL(smCertConfig.enNodeCert, R"(-----BEGIN CERTIFICATE-----
MIIBhDCCASqgAwIBAgIJAK9b6T7ApKETMAoGCCqBHM9VAYN1MDsxEzARBgNVBAMM
CmFnZW5jeV9zb24xEzARBgNVBAoMCmZpc2NvLWJjb3MxDzANBgNVBAsMBmFnZW5j
eTAgFw0yMTA1MTcwNzQyMDZaGA8yMTIxMDQyMzA3NDIwNlowNjEOMAwGA1UEAwwF
bm9kZTAxEzARBgNVBAoMCmZpc2NvLWJjb3MxDzANBgNVBAsMBmVubm9kZTBZMBMG
ByqGSM49AgEGCCqBHM9VAYItA0IABHyhDTxlw2OTchJbTmHMHgKEawnJcyZUXSrX
YLWBHXuZ0qXKO9VGz7zDAQbDus3hDMfLAOLGpDij2SJgwT19LCujGjAYMAkGA1Ud
EwQCMAAwCwYDVR0PBAQDAgM4MAoGCCqBHM9VAYN1A0gAMEUCIQDp1n6calICdam9
hLz1ycI6SnriQWoFJ/Q43yZAxCJJDAIgJl3hDHZ9I/xB4/sdHHCbZg6oPGcBtOx6
m5/W/FeeC5A=
-----END CERTIFICATE-----)");
    BOOST_CHECK_EQUAL(smCertConfig.enNodeKey, R"(-----BEGIN PRIVATE KEY-----
MIGHAgEAMBMGByqGSM49AgEGCCqBHM9VAYItBG0wawIBAQQgRmigmGuEIAafq9xl
msneevDnKooI70kCKSbinLa7SOWhRANCAAR8oQ08ZcNjk3ISW05hzB4ChGsJyXMm
VF0q12C1gR17mdKlyjvVRs+8wwEGw7rN4QzHywDixqQ4o9kiYME9fSwr
-----END PRIVATE KEY-----)");
  }
}

BOOST_AUTO_TEST_SUITE_END()
