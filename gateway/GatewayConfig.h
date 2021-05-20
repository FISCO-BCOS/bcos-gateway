/*
 * @CopyRight:
 * FISCO-BCOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FISCO-BCOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FISCO-BCOS.  If not, see <http://www.gnu.org/licenses/>
 * (c) 2016-2018 fisco-dev contributors.
 */
/** @file GatewayConfig.h
 *  @author octopus
 *  @date 2021-05-19
 */

#pragma once
#include "Common.h"
#include "libnetwork/Common.h"
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace bcos {
namespace gateway {

class GatewayConfig {
public:
  using Ptr = std::shared_ptr<GatewayConfig>;

  GatewayConfig() = default;
  ~GatewayConfig() = default;

public:
  // cert for ssl connection
  struct CertConfig {
    std::string caCert;
    std::string nodeKey;
    std::string nodeCert;
  };

  // cert for sm ssl connection
  struct SMCertConfig {
    std::string caCert;
    std::string nodeCert;
    std::string nodeKey;
    std::string enNodeCert;
    std::string enNodeKey;
  };

public:
  // check if the port valid
  bool isValidPort(int port);
  void hostAndPort2Endpoint(const std::string &_host,
                            NodeIPEndpoint &_endpoint);
  void parseConnectedJson(const std::string &_json,
                          std::set<NodeIPEndpoint> &_nodeIPEndpointSet);
  void parseConnectedFile(const std::string &_file,
                          std::set<NodeIPEndpoint> &_nodeIPEndpointSet);

public:
  // loads configuration items from the configuration file
  void initConfig(std::string const &_configPath);
  // loads p2p configuration items from the configuration file
  void initP2PConfig(const boost::property_tree::ptree &_pt);
  // loads ca configuration items from the configuration file
  void initCertConfig(const boost::property_tree::ptree &_pt);
  // loads sm ca configuration items from the configuration file
  void initSMCertConfig(const boost::property_tree::ptree &_pt);

public:
  uint32_t threadPoolSize() { return m_threadPoolSize; }
  bool smSSL() const { return m_smSSL; }
  std::string listenIP() const { return m_listenIP; }
  uint16_t listenPort() const { return m_listenPort; }

  std::string nodeCert() const {
    return smSSL() ? certConfig().nodeCert : smCertConfig().nodeCert;
  }

  CertConfig certConfig() const { return m_certConfig; }
  SMCertConfig smCertConfig() const { return m_smCertConfig; }
  const std::set<NodeIPEndpoint> &connectedNodes() const {
    return m_connectedNodes;
  }

private:
  // if SM SSL connection or not
  bool m_smSSL;
  // p2p network listen IP
  std::string m_listenIP;
  // p2p network listen Port
  uint16_t m_listenPort;
  //
  uint32_t m_threadPoolSize{16};
  // p2p connected nodes host list
  std::set<NodeIPEndpoint> m_connectedNodes;
  // cert config for ssl connection
  CertConfig m_certConfig;
  SMCertConfig m_smCertConfig;
};

} // namespace gateway
} // namespace bcos