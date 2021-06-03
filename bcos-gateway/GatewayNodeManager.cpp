
/*
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
 * @file GatewayNodeManager.cpp
 * @author: octopus
 * @date 2021-05-13
 */
#include <bcos-gateway/GatewayNodeManager.h>
#include <json/json.h>

using namespace std;
using namespace bcos;
using namespace gateway;

/**
 * @brief: register FrontService
 * @param _groupID: groupID
 * @param _nodeID: nodeID
 * @param _frontServiceInterface: FrontService
 * @return void
 */
bool GatewayNodeManager::registerFrontService(
    const std::string &_groupID, bcos::crypto::NodeIDPtr _nodeID,
    bcos::front::FrontServiceInterface::Ptr _frontServiceInterface) {
  bool isExist = false;
  {
    std::lock_guard<std::mutex> l(x_groupID2NodeID2FrontServiceInterface);
    auto it = m_groupID2NodeID2FrontServiceInterface.find(_groupID);
    if (it != m_groupID2NodeID2FrontServiceInterface.end()) {
      auto innerIt = it->second.find(_nodeID->hex());
      isExist = (innerIt != it->second.end());
    }

    if (!isExist) {
      m_groupID2NodeID2FrontServiceInterface[_groupID][_nodeID->hex()] =
          _frontServiceInterface;
      increaseSeq();
    }
  }

  if (!isExist) {
    NODE_MANAGER_LOG(INFO) << LOG_DESC("registerFrontService")
                           << LOG_KV("groupID", _groupID)
                           << LOG_KV("nodeID", _nodeID->hex())
                           << LOG_KV("statusSeq", m_statusSeq);
  } else {
    NODE_MANAGER_LOG(WARNING)
        << LOG_DESC("registerFrontService front service already exist")
        << LOG_KV("groupID", _groupID) << LOG_KV("nodeID", _nodeID->hex());
  }

  return !isExist;
}

/**
 * @brief: unregister FrontService
 * @param _groupID: groupID
 * @param _nodeID: nodeID
 * @return bool
 */
bool GatewayNodeManager::unregisterFrontService(
    const std::string &_groupID, bcos::crypto::NodeIDPtr _nodeID) {
  bool isOK = false;
  {
    std::lock_guard<std::mutex> l(x_groupID2NodeID2FrontServiceInterface);
    auto it = m_groupID2NodeID2FrontServiceInterface.find(_groupID);
    if (it != m_groupID2NodeID2FrontServiceInterface.end()) {
      auto innerIt = it->second.find(_nodeID->hex());
      if (innerIt != it->second.end()) {
        it->second.erase(innerIt);
        increaseSeq();
        isOK = true;
        if (it->second.empty()) {
          m_groupID2NodeID2FrontServiceInterface.erase(it);
        }
      }
    }
  }

  if (isOK) {
    NODE_MANAGER_LOG(INFO) << LOG_DESC("unregisterFrontService")
                           << LOG_KV("groupID", _groupID)
                           << LOG_KV("nodeID", _nodeID->hex())
                           << LOG_KV("statusSeq", m_statusSeq);
  } else {
    NODE_MANAGER_LOG(WARNING)
        << LOG_DESC("unregisterFrontService front service not exist")
        << LOG_KV("groupID", _groupID) << LOG_KV("nodeID", _nodeID->hex());
  }

  return isOK;
}

bcos::front::FrontServiceInterface::Ptr
GatewayNodeManager::queryFrontServiceInterfaceByGroupIDAndNodeID(
    const std::string &_groupID, bcos::crypto::NodeIDPtr _nodeID) {

  bcos::front::FrontServiceInterface::Ptr frontServiceInterface = nullptr;
  {
    std::lock_guard<std::mutex> l(x_groupID2NodeID2FrontServiceInterface);
    auto it = m_groupID2NodeID2FrontServiceInterface.find(_groupID);
    if (it != m_groupID2NodeID2FrontServiceInterface.end()) {
      auto innerIt = it->second.find(_nodeID->hex());
      if (innerIt != it->second.end()) {
        frontServiceInterface = innerIt->second;
      }
    }
  }

  if (!frontServiceInterface) {
    NODE_MANAGER_LOG(WARNING)
        << LOG_DESC("queryFrontServiceInterfaceByGroupIDA"
                    "ndNodeID front service of the node"
                    " not exist")
        << LOG_KV("groupID", _groupID) << LOG_KV("nodeID", _nodeID->hex());
  }

  return frontServiceInterface;
}

std::set<bcos::front::FrontServiceInterface::Ptr>
GatewayNodeManager::queryFrontServiceInterfaceByGroupID(
    const std::string &_groupID) {
  std::set<bcos::front::FrontServiceInterface::Ptr> frontServiceInterfaces;
  {
    std::lock_guard<std::mutex> l(x_groupID2NodeID2FrontServiceInterface);
    auto it = m_groupID2NodeID2FrontServiceInterface.find(_groupID);
    if (it != m_groupID2NodeID2FrontServiceInterface.end()) {
      for (const auto innerIt : it->second) {
        frontServiceInterfaces.insert(frontServiceInterfaces.begin(),
                                      innerIt.second);
      }
    }
  }

  if (frontServiceInterfaces.empty()) {
    NODE_MANAGER_LOG(WARNING)
        << LOG_DESC("queryFrontServiceInterfaceByGroupID front service of the "
                    "group not exist")
        << LOG_KV("groupID", _groupID);
  }

  return frontServiceInterfaces;
}

void GatewayNodeManager::onReceiveStatusSeq(const P2pID &_p2pID,
                                            uint32_t _statusSeq,
                                            bool &_statusSeqChanged) {

  _statusSeqChanged = true;
  {
    std::lock_guard<std::mutex> l(x_peerGatewayNodes);

    auto it = m_p2pID2Seq.find(_p2pID);
    if (it != m_p2pID2Seq.end()) {
      _statusSeqChanged = (_statusSeq != it->second);
    }
  }

  if (_statusSeqChanged) {
    NODE_MANAGER_LOG(INFO) << LOG_DESC("onReceiveStatusSeq")
                           << LOG_KV("p2pid", _p2pID)
                           << LOG_KV("statusSeq", _statusSeq)
                           << LOG_KV("seqChanged", _statusSeqChanged);
  } else {
    NODE_MANAGER_LOG(DEBUG)
        << LOG_DESC("onReceiveStatusSeq") << LOG_KV("p2pid", _p2pID)
        << LOG_KV("statusSeq", _statusSeq)
        << LOG_KV("seqChanged", _statusSeqChanged);
  }
}

void GatewayNodeManager::updateNodeIDs(
    const P2pID &_p2pID, uint32_t _seq,
    const std::unordered_map<std::string, std::set<std::string>> &_nodeIDsMap) {
  NODE_MANAGER_LOG(INFO) << LOG_DESC("updateNodeIDs") << LOG_KV("p2pid", _p2pID)
                         << LOG_KV("statusSeq", _seq);

  std::lock_guard<std::mutex> l(x_peerGatewayNodes);
  // remove peer nodeIDs info first
  removeNodeIDsByP2PID(_p2pID);
  // insert current nodeIDs info
  for (const auto &nodeIDs : _nodeIDsMap) {
    for (const auto &nodeID : nodeIDs.second) {
      m_peerGatewayNodes[nodeIDs.first][nodeID].insert(_p2pID);
    }
  }
  // update seq
  m_p2pID2Seq[_p2pID] = _seq;
}

void GatewayNodeManager::removeNodeIDsByP2PID(const std::string &_p2pID) {
  // remove all nodeIDs info belong to p2pID
  for (auto it = m_peerGatewayNodes.begin(); it != m_peerGatewayNodes.end();) {
    for (auto innerIt = it->second.begin(); innerIt != it->second.end();) {
      for (auto innerIt2 = innerIt->second.begin();
           innerIt2 != innerIt->second.end();) {
        if (*innerIt2 == _p2pID) {
          innerIt2 = innerIt->second.erase(innerIt2);
        } else {
          ++innerIt2;
        }
      } // for (auto innerIt2

      if (innerIt->second.empty()) {
        innerIt = it->second.erase(innerIt);
      } else {
        ++innerIt;
      }
    } // for (auto innerIt

    if (it->second.empty()) {
      it = m_peerGatewayNodes.erase(it);
    } else {
      ++it;
    }
  } // for (auto it
}

bool GatewayNodeManager::parseReceivedJson(
    const std::string &_json, uint32_t &statusSeq,
    std::unordered_map<std::string, std::set<std::string>> &nodeIDsMap) {
  /*
  sample:
  {"statusSeq":1,"nodeInfoList":[{"groupID":"group1","nodeIDs":["a0","b0","c0"]},{"groupID":"group2","nodeIDs":["a1","b1","c1"]},{"groupID":"group3","nodeIDs":["a2","b2","c2"]}]}
  */
  Json::Value root;
  Json::Reader jsonReader;

  try {
    if (!jsonReader.parse(_json, root)) {
      NODE_MANAGER_LOG(ERROR) << "parseReceivedJson unable to parse this json"
                              << LOG_KV("json:", _json);
      return false;
    }

    statusSeq = root["statusSeq"].asUInt();
    auto jsonArraySize = root["nodeInfoList"].size();

    for (unsigned int i = 0; i < jsonArraySize; i++) {
      auto jNode = root["nodeInfoList"][i];

      // groupID
      std::string groupID = jNode["groupID"].asString();
      // nodeID set
      std::set<std::string> nodeIDsSet;
      auto nodeIDsSize = jNode["nodeIDs"].size();
      for (unsigned int j = 0; j < nodeIDsSize; j++) {
        auto nodeID = jNode["nodeIDs"][j].asString();
        nodeIDsSet.insert(nodeID);
      }

      nodeIDsMap[groupID] = nodeIDsSet;
    }

    NODE_MANAGER_LOG(INFO) << LOG_DESC("parseReceivedJson ")
                           << LOG_KV("statusSeq", statusSeq)
                           << LOG_KV("json", _json);
    return true;

  } catch (const std::exception &e) {
    NODE_MANAGER_LOG(ERROR) << LOG_DESC("parseReceivedJson error: " +
                                        boost::diagnostic_information(e));
    return false;
  }
}

void GatewayNodeManager::onReceiveNodeIDs(const P2pID &_p2pID,
                                          const std::string &_nodeIDsJson) {
  // parser info json first
  uint32_t statusSeq;
  std::unordered_map<std::string, std::set<std::string>> nodeIDsMap;
  if (parseReceivedJson(_nodeIDsJson, statusSeq, nodeIDsMap)) {
    updateNodeIDs(_p2pID, statusSeq, nodeIDsMap);
  }
}

void GatewayNodeManager::onRequestNodeIDs(std::string &_nodeIDsJson) {
  // groupID => nodeIDs list
  std::unordered_map<std::string, std::set<std::string>> localGroup2NodeIDs;
  uint32_t seq = 0;
  {
    std::lock_guard<std::mutex> l(x_groupID2NodeID2FrontServiceInterface);
    seq = statusSeq();
    for (const auto &groupID2NodeID2FrontServiceInterface :
         m_groupID2NodeID2FrontServiceInterface) {
      for (const auto &nodeID2FrontServiceInterface :
           groupID2NodeID2FrontServiceInterface.second) {
        localGroup2NodeIDs[groupID2NodeID2FrontServiceInterface.first].insert(
            nodeID2FrontServiceInterface.first);
      }
    }
  }

  // generator json first
  try {
    Json::Value jArray = Json::Value(Json::arrayValue);
    for (const auto &group2NodeIDs : localGroup2NodeIDs) {
      Json::Value jNode;
      jNode["groupID"] = group2NodeIDs.first;
      jNode["nodeIDs"] = Json::Value(Json::arrayValue);
      for (const auto &nodeID : group2NodeIDs.second) {
        jNode["nodeIDs"].append(nodeID);
      }
      jArray.append(jNode);
    }

    Json::Value jResp;
    jResp["statusSeq"] = seq;
    jResp["nodeInfoList"] = jArray;

    Json::FastWriter writer;
    _nodeIDsJson = writer.write(jResp);

    NODE_MANAGER_LOG(INFO) << LOG_DESC("onRequestNodeIDs ")
                           << LOG_KV("seq", seq)
                           << LOG_KV("json", _nodeIDsJson);

  } catch (const std::exception &e) {
    NODE_MANAGER_LOG(ERROR) << LOG_DESC("onRequestNodeIDs error: " +
                                        boost::diagnostic_information(e));
  }
}

void GatewayNodeManager::onRemoveNodeIDs(const P2pID &_p2pID) {

  NODE_MANAGER_LOG(INFO) << LOG_DESC("onRemoveNodeIDs")
                         << LOG_KV("p2pid", _p2pID);

  std::lock_guard<std::mutex> l(x_peerGatewayNodes);
  // remove statusSeq info
  removeNodeIDsByP2PID(_p2pID);
  m_p2pID2Seq.erase(_p2pID);
}

bool GatewayNodeManager::queryP2pIDs(const std::string &_groupID,
                                     const std::string &_nodeID,
                                     std::set<P2pID> &_p2pIDs) {
  std::lock_guard<std::mutex> l(x_peerGatewayNodes);

  auto it = m_peerGatewayNodes.find(_groupID);
  if (it == m_peerGatewayNodes.end()) {
    return false;
  }

  auto innerIt = it->second.find(_nodeID);
  if (innerIt == it->second.end()) {
    return false;
  }

  _p2pIDs.insert(innerIt->second.begin(), innerIt->second.end());

  return true;
}

bool GatewayNodeManager::queryP2pIDsByGroupID(const std::string &_groupID,
                                              std::set<P2pID> &_p2pIDs) {

  std::lock_guard<std::mutex> l(x_peerGatewayNodes);

  auto it = m_peerGatewayNodes.find(_groupID);
  if (it == m_peerGatewayNodes.end()) {
    return false;
  }

  for (const auto &nodeMap : it->second) {
    _p2pIDs.insert(nodeMap.second.begin(), nodeMap.second.end());
  }

  return true;
}
