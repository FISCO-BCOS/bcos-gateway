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
 * @file GatewayNodeManager.h
 * @author: octopus
 * @date 2021-05-13
 */

#pragma once
#include <bcos-framework/interfaces/crypto/KeyFactory.h>
#include <bcos-framework/interfaces/front/FrontServiceInterface.h>
#include <bcos-framework/interfaces/gateway/GatewayInterface.h>
#include <bcos-gateway/Common.h>
#include <bcos-gateway/libnetwork/Common.h>
namespace bcos
{
namespace gateway
{
class GatewayNodeManager
{
public:
    using Ptr = std::shared_ptr<GatewayNodeManager>;

public:
    uint32_t statusSeq() { return m_statusSeq; }
    uint32_t increaseSeq()
    {
        uint32_t statusSeq = ++m_statusSeq;
        return statusSeq;
    }

    bool parseReceivedJson(const std::string& _json, uint32_t& statusSeq,
        std::unordered_map<std::string, std::set<std::string>>& nodeIDsMap);
    void updateNodeIDs(const P2pID& _p2pID, uint32_t _seq,
        const std::unordered_map<std::string, std::set<std::string>>& _nodeIDsMap);

    void onReceiveStatusSeq(const P2pID& _p2pID, uint32_t _statusSeq, bool& _statusSeqChanged);
    void onReceiveNodeIDs(const P2pID& _p2pID, const std::string& _nodeIDsJson);
    void onRequestNodeIDs(std::string& _nodeIDsJson);
    void onRemoveNodeIDs(const P2pID& _p2pID);
    void removeNodeIDsByP2PID(const std::string& _p2pID);

    bool queryP2pIDs(
        const std::string& _groupID, const std::string& _nodeID, std::set<P2pID>& _p2pIDs);
    bool queryP2pIDsByGroupID(const std::string& _groupID, std::set<P2pID>& _p2pIDs);
    bool queryNodeIDsByGroupID(const std::string& _groupID, std::set<std::string>& _nodeIDs);

    void showAllPeerGatewayNodeIDs();
    void notifyNodeIDs2FrontService();

    bcos::front::FrontServiceInterface::Ptr queryFrontServiceInterfaceByGroupIDAndNodeID(
        const std::string& _groupID, bcos::crypto::NodeIDPtr _nodeID);
    std::set<bcos::front::FrontServiceInterface::Ptr> queryFrontServiceInterfaceByGroupID(
        const std::string& _groupID);
    /**
     * @brief: register FrontService
     * @param _groupID: groupID
     * @param _nodeID: nodeID
     * @param _frontServiceInterface: FrontService
     * @return bool
     */
    bool registerFrontService(const std::string& _groupID, bcos::crypto::NodeIDPtr _nodeID,
        bcos::front::FrontServiceInterface::Ptr _frontServiceInterface);

    /**
     * @brief: unregister FrontService
     * @param _groupID: groupID
     * @param _nodeID: nodeID
     * @return bool
     */
    bool unregisterFrontService(const std::string& _groupID, bcos::crypto::NodeIDPtr _nodeID);

public:
    const std::unordered_map<std::string,
        std::unordered_map<std::string, bcos::front::FrontServiceInterface::Ptr>>&
    groupID2NodeID2FrontServiceInterface() const
    {
        return m_groupID2NodeID2FrontServiceInterface;
    }

    std::shared_ptr<bcos::crypto::KeyFactory> keyFactory() { return m_keyFactory; }

    void setKeyFactory(std::shared_ptr<bcos::crypto::KeyFactory> _keyFactory)
    {
        m_keyFactory = _keyFactory;
    }

private:
    std::shared_ptr<bcos::crypto::KeyFactory> m_keyFactory;
    // statusSeq
    std::atomic<uint32_t> m_statusSeq{1};
    // lock m_peerGatewayNodes
    mutable std::mutex x_peerGatewayNodes;
    // groupID => NodeID => set<P2pID>
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<P2pID>>>
        m_peerGatewayNodes;
    // P2pID => statusSeq
    std::unordered_map<std::string, uint32_t> m_p2pID2Seq;
    // lock m_groupID2FrontServiceInterface
    mutable std::mutex x_groupID2NodeID2FrontServiceInterface;
    // groupID => nodeID => FrontServiceInterface
    std::unordered_map<std::string,
        std::unordered_map<std::string, bcos::front::FrontServiceInterface::Ptr>>
        m_groupID2NodeID2FrontServiceInterface;
};
}  // namespace gateway
}  // namespace bcos
