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
 * @file Gateway.h
 * @author: octopus
 * @date 2021-04-19
 */

#pragma once

#include <bcos-framework/interfaces/front/FrontServiceInterface.h>
#include <bcos-framework/interfaces/gateway/GatewayInterface.h>
#include <bcos-gateway/Common.h>
#include <bcos-gateway/GatewayNodeManager.h>
#include <bcos-gateway/libp2p/Service.h>

namespace bcos
{
namespace gateway
{
class Gateway : public GatewayInterface, public std::enable_shared_from_this<Gateway>
{
public:
    using Ptr = std::shared_ptr<Gateway>;

public:
    Gateway() {}
    virtual ~Gateway() { stop(); }

    virtual void start() override;
    virtual void stop() override;

public:
    /**
     * @brief: get connected peers
     * @param _peerRespFunc:
     * @return void
     */
    virtual void asyncGetPeers(PeerRespFunc _peerRespFunc) override;

    /**
     * @brief: construct Message object
     */
    std::shared_ptr<P2PMessage> newP2PMessage(const std::string& _groupID,
        bcos::crypto::NodeIDPtr _srcNodeID, bcos::crypto::NodeIDPtr _dstNodeID,
        bytesConstRef _payload);
    std::shared_ptr<P2PMessage> newP2PMessage(
        const std::string& _groupID, bcos::crypto::NodeIDPtr _srcNodeID, bytesConstRef _payload);

    /**
     * @brief: register FrontService
     * @param _groupID: groupID
     * @param _nodeID: nodeID
     * @param _frontServiceInterface: FrontService
     * @return bool
     */
    virtual bool registerFrontService(const std::string& _groupID, bcos::crypto::NodeIDPtr _nodeID,
        bcos::front::FrontServiceInterface::Ptr _frontServiceInterface);

    /**
     * @brief: unregister FrontService
     * @param _groupID: groupID
     * @param _nodeID: nodeID
     * @return bool
     */
    virtual bool unregisterFrontService(
        const std::string& _groupID, bcos::crypto::NodeIDPtr _nodeID);
    /**
     * @brief: send message
     * @param _groupID: groupID
     * @param _srcNodeID: the sender nodeID
     * @param _dstNodeID: the receiver nodeID
     * @param _payload: message payload
     * @param _errorRespFunc: error func
     * @return void
     */
    virtual void asyncSendMessageByNodeID(const std::string& _groupID,
        bcos::crypto::NodeIDPtr _srcNodeID, bcos::crypto::NodeIDPtr _dstNodeID,
        bytesConstRef _payload, ErrorRespFunc _errorRespFunc) override;

    /**
     * @brief: send message to multiple nodes
     * @param _groupID: groupID
     * @param _srcNodeID: the sender nodeID
     * @param _nodeIDs: the receiver nodeIDs
     * @param _payload: message payload
     * @return void
     */
    virtual void asyncSendMessageByNodeIDs(const std::string& _groupID,
        bcos::crypto::NodeIDPtr _srcNodeID, const bcos::crypto::NodeIDs& _nodeIDs,
        bytesConstRef _payload) override;

    /**
     * @brief: send broadcast message
     * @param _groupID: groupID
     * @param _srcNodeID: the sender nodeID
     * @param _payload: message payload
     * @return void
     */
    virtual void asyncSendBroadcastMessage(const std::string& _groupID,
        bcos::crypto::NodeIDPtr _srcNodeID, bytesConstRef _payload) override;

    /**
     * @brief: receive p2p message
     * @param _groupID: groupID
     * @param _srcNodeID: the sender nodeID
     * @param _dstNodeID: the receiver nodeID
     * @param _payload: message content
     * @param _errorRespFunc: error func
     * @return void
     */
    virtual void onReceiveP2PMessage(const std::string& _groupID,
        bcos::crypto::NodeIDPtr _srcNodeID, bcos::crypto::NodeIDPtr _dstNodeID,
        bytesConstRef _payload, ErrorRespFunc _errorRespFunc = ErrorRespFunc());

    /**
     * @brief: receive group broadcast message
     * @param _groupID: groupID
     * @param _srcNodeID: the sender nodeID
     * @param _payload: message content
     * @return void
     */
    virtual void onReceiveBroadcastMessage(
        const std::string& _groupID, bcos::crypto::NodeIDPtr _srcNodeID, bytesConstRef _payload);

public:
    P2PInterface::Ptr p2pInterface() const { return m_p2pInterface; }
    void setP2PInterface(P2PInterface::Ptr _p2pInterface) { m_p2pInterface = _p2pInterface; }

    GatewayNodeManager::Ptr gatewayNodeManager() { return m_gatewayNodeManager; }
    void setGatewayNodeManager(GatewayNodeManager::Ptr _gatewayNodeManager)
    {
        m_gatewayNodeManager = _gatewayNodeManager;
    }

private:
    // p2p service interface
    P2PInterface::Ptr m_p2pInterface;
    // GatewayNodeManager
    GatewayNodeManager::Ptr m_gatewayNodeManager;
};

}  // namespace gateway
}  // namespace bcos
