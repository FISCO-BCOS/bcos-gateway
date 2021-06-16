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
/** @file Service.h
 *  @author monan
 *  @modify first draft
 *  @date 20180910
 *  @author chaychen
 *  @modify realize encode and decode, add timeout, code format
 *  @date 20180911
 */

#pragma once
#include <bcos-framework/interfaces/crypto/KeyFactory.h>
#include <bcos-gateway/Gateway.h>
#include <bcos-gateway/libp2p/P2PInterface.h>
#include <bcos-gateway/libp2p/P2PSession.h>

#include <map>
#include <memory>
#include <unordered_map>

namespace bcos
{
namespace gateway
{
class Host;
class P2PMessage;
class Gateway;

class Service : public P2PInterface, public std::enable_shared_from_this<Service>
{
public:
    Service();
    virtual ~Service() { stop(); }

    using Ptr = std::shared_ptr<Service>;

    virtual void start() override;
    virtual void stop() override;
    virtual void heartBeat();

    virtual bool actived() { return m_run; }
    P2pID id() const override { return m_nodeID; }
    void setId(const P2pID& _nodeID) { m_nodeID = _nodeID; }

    virtual void onConnect(
        NetworkException e, P2PInfo const& p2pInfo, std::shared_ptr<SessionFace> session);
    virtual void onDisconnect(NetworkException e, P2PSession::Ptr p2pSession);
    virtual void onMessage(NetworkException e, SessionFace::Ptr session, Message::Ptr message,
        std::weak_ptr<P2PSession> p2pSessionWeakPtr);

    std::shared_ptr<P2PMessage> sendMessageByNodeID(
        P2pID nodeID, std::shared_ptr<P2PMessage> message) override;
    void sendMessageBySession(int _packetType, bytesConstRef _payload, P2PSession::Ptr _p2pSession);
    void sendRespMessageBySession(
        bytesConstRef _payload, P2PMessage::Ptr _p2pMessage, P2PSession::Ptr _p2pSession);
    void asyncSendMessageByNodeID(P2pID nodeID, std::shared_ptr<P2PMessage> message,
        CallbackFuncWithSession callback, Options options = Options()) override;

    void asyncBroadcastMessage(std::shared_ptr<P2PMessage> message, Options options) override;

    virtual std::map<NodeIPEndpoint, P2pID> staticNodes() { return m_staticNodes; }
    virtual void setStaticNodes(const std::set<NodeIPEndpoint>& staticNodes)
    {
        for (const auto& endpoint : staticNodes)
        {
            m_staticNodes.insert(std::make_pair(endpoint, ""));
        }
    }

    P2PSessionInfos sessionInfos() override;  ///< Only connected node

    bool isConnected(P2pID const& nodeID) const override;

    virtual std::shared_ptr<Host> host() override { return m_host; }
    virtual void setHost(std::shared_ptr<Host> host) { m_host = host; }

    std::shared_ptr<MessageFactory> messageFactory() override { return m_messageFactory; }
    virtual void setMessageFactory(std::shared_ptr<MessageFactory> _messageFactory)
    {
        m_messageFactory = _messageFactory;
    }

    std::shared_ptr<bcos::crypto::KeyFactory> keyFactory() { return m_keyFactory; }

    void setKeyFactory(std::shared_ptr<bcos::crypto::KeyFactory> _keyFactory)
    {
        m_keyFactory = _keyFactory;
    }

    std::weak_ptr<Gateway> gateway() { return m_gateway; }

    void setGateway(std::weak_ptr<Gateway> _gateway) { m_gateway = _gateway; }

    void updateStaticNodes(std::shared_ptr<SocketFace> const& _s, P2pID const& nodeId);

    void registerDisconnectHandler(std::function<void(NetworkException, P2PSession::Ptr)> _handler)
    {
        m_disconnectionHandlers.push_back(_handler);
    }

    std::shared_ptr<P2PSession> getP2PSessionByNodeId(P2pID const& _nodeID) override
    {
        RecursiveGuard l(x_sessions);
        auto it = m_sessions.find(_nodeID);
        if (it != m_sessions.end())
        {
            return it->second;
        }
        return nullptr;
    }

    uint32_t statusSeq();

private:
    std::vector<std::function<void(NetworkException, P2PSession::Ptr)>> m_disconnectionHandlers;

    std::shared_ptr<bcos::crypto::KeyFactory> m_keyFactory;

    std::weak_ptr<Gateway> m_gateway;

    std::map<NodeIPEndpoint, P2pID> m_staticNodes;
    bcos::RecursiveMutex x_nodes;

    std::shared_ptr<Host> m_host;

    std::unordered_map<P2pID, P2PSession::Ptr> m_sessions;
    mutable bcos::RecursiveMutex x_sessions;

    std::shared_ptr<MessageFactory> m_messageFactory;

    P2pID m_nodeID;

    std::shared_ptr<boost::asio::deadline_timer> m_timer;

    bool m_run = false;
};

}  // namespace gateway
}  // namespace bcos
