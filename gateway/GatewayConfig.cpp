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
/** @file GatewayConfig.cpp
 *  @author octopus
 *  @date 2021-05-19
 */

#include "GatewayConfig.h"
#include <bcos-framework/libutilities/FileUtility.h>
#include <boost/throw_exception.hpp>
#include <json/json.h>

using namespace bcos;
using namespace gateway;

bool GatewayConfig::isValidPort(int port) {
  if (port <= 1024 || port > 65535)
    return false;
  return true;
}

void GatewayConfig::hostAndPort2Endpoint(const std::string &_host,
                                         NodeIPEndpoint &_endpoint) {
  std::string ip;
  uint16_t port;

  std::vector<std::string> s;
  boost::split(s, _host, boost::is_any_of("]"), boost::token_compress_on);
  if (s.size() == 2) { // ipv6
    ip = s[0].data() + 1;
    port = boost::lexical_cast<int>(s[1].data() + 1);
  } else if (s.size() == 1) { // ipv4
    std::vector<std::string> v;
    boost::split(v, _host, boost::is_any_of(":"), boost::token_compress_on);
    ip = v[0];
    port = boost::lexical_cast<int>(v[1]);
  } else {
    GATEWAY_CONFIG_LOG(ERROR)
        << LOG_DESC("not valid host value") << LOG_KV("host", _host);
    BOOST_THROW_EXCEPTION(
        InvalidParameter() << errinfo_comment(
            "GatewayConfig: not valid host value, host=" + _host));
  }

  if (!isValidPort(port)) {
    GATEWAY_CONFIG_LOG(ERROR)
        << LOG_DESC("not valid port value") << LOG_KV("port", port);
    BOOST_THROW_EXCEPTION(InvalidParameter() << errinfo_comment(
                              "GatewayConfig: not valid port value, port=" +
                              std::to_string(port)));
  }

  boost::system::error_code ec;
  boost::asio::ip::address ip_address = boost::asio::ip::make_address(ip, ec);
  if (ec.value() != 0) {
    GATEWAY_CONFIG_LOG(ERROR)
        << LOG_DESC("not valid host value, make_address error")
        << LOG_KV("host", _host);
    BOOST_THROW_EXCEPTION(
        InvalidParameter() << errinfo_comment(
            "GatewayConfig: not valid host value make_address error, host=" +
            _host));
  }

  _endpoint = NodeIPEndpoint{ip_address, port};
}

void GatewayConfig::parseConnectedJson(
    const std::string &_json, std::set<NodeIPEndpoint> &_nodeIPEndpointSet) {
  /*
  {"nodes":["127.0.0.1:30355","127.0.0.1:30356"}]}
  */
  Json::Value root;
  Json::Reader jsonReader;

  try {
    if (!jsonReader.parse(_json, root)) {
      GATEWAY_CONFIG_LOG(ERROR)
          << "unable to parse connected nodes json" << LOG_KV("json:", _json);
      BOOST_THROW_EXCEPTION(InvalidParameter() << errinfo_comment(
                                "GatewayConfig: unable to parse p2p "
                                "connected nodes json"));
    }

    std::set<NodeIPEndpoint> nodeIPEndpointSet;
    Json::Value jNodes = root["nodes"];
    if (jNodes.isArray()) {
      unsigned int jNodesSize = jNodes.size();
      for (unsigned int i = 0; i < jNodesSize; i++) {
        std::string host = jNodes[i].asString();

        NodeIPEndpoint endpoint;
        hostAndPort2Endpoint(host, endpoint);
        _nodeIPEndpointSet.insert(endpoint);

        GATEWAY_CONFIG_LOG(INFO)
            << LOG_DESC("add one connected node") << LOG_KV("host", host);
      }
    }
  } catch (const std::exception &e) {
    GATEWAY_CONFIG_LOG(ERROR) << LOG_KV("parseConnectedJson error: ",
                                        boost::diagnostic_information(e));
    BOOST_THROW_EXCEPTION(e);
  }
}
void GatewayConfig::parseConnectedFile(
    const std::string &_file, std::set<NodeIPEndpoint> &_nodeIPEndpointSet) {
  boost::filesystem::path p(_file);
  auto content = readContentsToString(boost::filesystem::path(_file));
  if (content && !content->empty()) {
    return parseConnectedJson(*content.get(), _nodeIPEndpointSet);
  }
}

/// loads configuration items from the configuration file
void GatewayConfig::initConfig(std::string const &_configPath) {
  try {
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(_configPath, pt);
    initP2PConfig(pt);
    if (m_smSSL) {
      initSMCertConfig(pt);
    } else {
      initCertConfig(pt);
    }
  } catch (const std::exception &e) {
    GATEWAY_CONFIG_LOG(ERROR)
        << LOG_KV("initConfig error: ", boost::diagnostic_information(e));
    BOOST_THROW_EXCEPTION(e);
  }

  GATEWAY_CONFIG_LOG(INFO) << LOG_DESC("initConfig ok!")
                           << LOG_KV("listenIP", m_listenIP)
                           << LOG_KV("listenPort", m_listenPort)
                           << LOG_KV("smSSL", m_smSSL)
                           << LOG_KV("peers", m_connectedNodes.size());
}

/// loads p2p configuration items from the configuration file
void GatewayConfig::initP2PConfig(const boost::property_tree::ptree &_pt) {
  /*
  [p2p]
    ; ssl or sm ssl
    sm_ssl=true
    listen_ip=0.0.0.0
    listen_port=30300
    nodes_path=./
    nodes_file=nodes.json
    */
  bool smSSL = _pt.get<bool>("p2p.sm_ssl", false);
  std::string listenIP = _pt.get<std::string>("p2p.listen_ip", "0.0.0.0");
  int listenPort = _pt.get<int>("p2p.listen_port", 30300);
  if (!isValidPort(listenPort)) {
    BOOST_THROW_EXCEPTION(InvalidParameter() << errinfo_comment(
                              "initP2PConfig: invalid listen port, port=" +
                              std::to_string(listenPort)));
  }

  std::string path = _pt.get<std::string>("p2p.nodes_path", "./");
  std::string file = _pt.get<std::string>("p2p.nodes_file", "nodes.json");

  // load p2p connected nodes
  std::set<NodeIPEndpoint> nodes;
  parseConnectedFile(path + "/" + file, nodes);

  m_smSSL = smSSL;
  m_listenIP = listenIP;
  m_listenPort = (uint16_t)listenPort;
  m_connectedNodes = nodes;

  GATEWAY_CONFIG_LOG(DEBUG)
      << LOG_DESC("initP2PConfig ok!") << LOG_KV("listenIP", listenIP)
      << LOG_KV("listenPort", listenPort) << LOG_KV("smSSL", smSSL)
      << LOG_KV("jsonPath", path) << LOG_KV("jsonFile", file)
      << LOG_KV("nodes.size()", nodes.size());
}

/// loads ca configuration items from the configuration file
void GatewayConfig::initCertConfig(const boost::property_tree::ptree &_pt) {
  /*
  [cert]
    ; directory the certificates located in
    ca_path=./
    ; the ca certificate file
    ca_cert=ca.crt
    ; the node private key file
    node_key=node.key
    ; the node certificate file
    node_cert=node.crt
  */
  std::string caPath = _pt.get<std::string>("cert.ca_path", "./");
  std::string caCertFile =
      caPath + "/" + _pt.get<std::string>("cert.ca_cert", "ca.crt");
  std::string nodeCertFile =
      caPath + "/" + _pt.get<std::string>("cert.node_cert", "node.crt");
  std::string nodeKeyFile =
      caPath + "/" + _pt.get<std::string>("cert.node_key", "node.key");

  GATEWAY_CONFIG_LOG(INFO) << LOG_DESC("initCertConfig")
                           << LOG_KV("ca_path", caPath)
                           << LOG_KV("ca_cert", caCertFile)
                           << LOG_KV("node_cert", nodeCertFile)
                           << LOG_KV("node_key", nodeKeyFile);

  auto caCertContent =
      readContentsToString(boost::filesystem::path(caCertFile));
  if (!caCertContent || caCertContent->empty()) {
    BOOST_THROW_EXCEPTION(
        InvalidParameter() << errinfo_comment(
            "initCertConfig: unable to load ca cert, path=" + caCertFile));
  }
  auto nodeCertContent =
      readContentsToString(boost::filesystem::path(nodeCertFile));
  if (!nodeCertContent || nodeCertContent->empty()) {
    BOOST_THROW_EXCEPTION(
        InvalidParameter() << errinfo_comment(
            "initCertConfig: unable to load node cert, path=" + nodeCertFile));
  }
  auto nodeKeyContent =
      readContentsToString(boost::filesystem::path(nodeKeyFile));
  if (!nodeKeyContent || nodeKeyContent->empty()) {
    BOOST_THROW_EXCEPTION(
        InvalidParameter() << errinfo_comment(
            "initCertConfig: unable to load node key, path=" + nodeKeyFile));
  }

  CertConfig certConfig;
  certConfig.caCert = *caCertContent.get();
  certConfig.nodeCert = *nodeCertContent.get();
  certConfig.nodeKey = *nodeKeyContent.get();

  m_certConfig = certConfig;
}

// loads sm ca configuration items from the configuration file
void GatewayConfig::initSMCertConfig(const boost::property_tree::ptree &_pt) {
  /*
  [cert]
  ; directory the certificates located in
  ca_path=./
  ; the ca certificate file
  sm_ca_cert=sm_ca.crt
  ; the node private key file
  sm_node_key=sm_node.key
  ; the node certificate file
  sm_node_cert=sm_node.crt
  ; the node private key file
  sm_ennode_key=sm_ennode.key
  ; the node certificate file
  sm_ennode_cert=sm_ennode.crt
  */

  std::string caPath = _pt.get<std::string>("cert.ca_path", "./");
  std::string smCaCertFile =
      caPath + "/" + _pt.get<std::string>("cert.sm_ca_cert", "sm_ca.crt");
  std::string smNodeCertFile =
      caPath + "/" + _pt.get<std::string>("cert.sm_node_cert", "sm_node.crt");
  std::string smNodeKeyFile =
      caPath + "/" + _pt.get<std::string>("cert.sm_node_key", "sm_node.key");
  std::string smEnNodeCertFile =
      caPath + "/" +
      _pt.get<std::string>("cert.sm_ennode_cert", "sm_ennode.crt");
  std::string smEnNodeKeyFile =
      caPath + "/" +
      _pt.get<std::string>("cert.sm_ennode_key", "sm_ennode.key");

  GATEWAY_CONFIG_LOG(INFO) << LOG_DESC("initSMCertConfig")
                           << LOG_KV("ca_path", caPath)
                           << LOG_KV("sm_ca_cert", smCaCertFile)
                           << LOG_KV("sm_node_cert", smNodeCertFile)
                           << LOG_KV("sm_node_key", smNodeKeyFile)
                           << LOG_KV("sm_ennode_cert", smEnNodeCertFile)
                           << LOG_KV("sm_ennode_key", smEnNodeKeyFile);

  auto smCaCertContent =
      readContentsToString(boost::filesystem::path(smCaCertFile));
  if (!smCaCertContent || smCaCertContent->empty()) {
    BOOST_THROW_EXCEPTION(
        InvalidParameter() << errinfo_comment(
            "initCertConfig: unable to load sm ca cert, path=" + smCaCertFile));
  }

  auto smNodeCertContent =
      readContentsToString(boost::filesystem::path(smNodeCertFile));
  if (!smNodeCertContent || smNodeCertContent->empty()) {
    BOOST_THROW_EXCEPTION(
        InvalidParameter() << errinfo_comment(
            "initSMCertConfig: unable to load sm node cert, path=" +
            smNodeCertFile));
  }

  auto smNodeKeyContent =
      readContentsToString(boost::filesystem::path(smNodeKeyFile));
  if (!smNodeKeyContent || smNodeKeyContent->empty()) {
    BOOST_THROW_EXCEPTION(
        InvalidParameter() << errinfo_comment(
            "initSMCertConfig: unable to load sm node key, path=" +
            smNodeKeyFile));
  }

  auto smEnNodeCertContent =
      readContentsToString(boost::filesystem::path(smEnNodeCertFile));
  if (!smEnNodeCertContent || smEnNodeCertContent->empty()) {
    BOOST_THROW_EXCEPTION(
        InvalidParameter() << errinfo_comment(
            "initSMCertConfig: unable to load sm encrypt node cert, path=" +
            smEnNodeCertFile));
  }

  auto smEnNodeKeyContent =
      readContentsToString(boost::filesystem::path(smEnNodeKeyFile));
  if (!smEnNodeKeyContent || smEnNodeKeyContent->empty()) {
    BOOST_THROW_EXCEPTION(
        InvalidParameter() << errinfo_comment(
            "initSMCertConfig: unable to load sm encrypt node key, path=" +
            smEnNodeKeyFile));
  }

  SMCertConfig smCertConfig;
  smCertConfig.caCert = *smCaCertContent.get();
  smCertConfig.nodeCert = *smNodeCertContent.get();
  smCertConfig.nodeKey = *smNodeKeyContent.get();
  smCertConfig.enNodeCert = *smEnNodeCertContent.get();
  smCertConfig.enNodeKey = *smEnNodeKeyContent.get();

  m_smCertConfig = smCertConfig;
}
