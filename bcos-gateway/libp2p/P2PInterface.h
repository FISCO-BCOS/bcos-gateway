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
/** @file P2PInterface.h
 *  @author chaychen
 *  @date 20180911
 */

#pragma once

#include <bcos-gateway/libnetwork/Host.h>
#include <bcos-gateway/libnetwork/SessionFace.h>
#include <bcos-gateway/libp2p/Common.h>
#include <bcos-gateway/libp2p/P2PMessage.h>
#include <memory>

namespace bcos
{
namespace stat
{
class NetworkStatHandler;
class ChannelNetworkStatHandler;
}  // namespace stat

namespace gateway
{
class P2PMessage;
class MessageFactory;
class P2PSession;
using CallbackFuncWithSession =
    std::function<void(NetworkException, std::shared_ptr<P2PSession>, std::shared_ptr<P2PMessage>)>;
using DisconnectCallbackFuncWithSession =
    std::function<void(NetworkException, std::shared_ptr<P2PSession>)>;
using P2PResponseCallback = std::function<void(Error::Ptr&& _error, std::shared_ptr<bytes> _data)>;
class P2PInterface
{
public:
    using Ptr = std::shared_ptr<P2PInterface>;
    virtual ~P2PInterface(){};

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual P2pID id() const = 0;

    virtual std::shared_ptr<P2PMessage> sendMessageByNodeID(
        P2pID nodeID, std::shared_ptr<P2PMessage> message) = 0;

    virtual void asyncSendMessageByNodeID(P2pID nodeID, std::shared_ptr<P2PMessage> message,
        CallbackFuncWithSession callback, Options options = Options()) = 0;

    virtual void asyncBroadcastMessage(std::shared_ptr<P2PMessage> message, Options options) = 0;

    virtual P2PSessionInfos sessionInfos() = 0;

    virtual bool isConnected(P2pID const& _nodeID) const = 0;

    virtual std::shared_ptr<Host> host() = 0;

    virtual std::shared_ptr<MessageFactory> messageFactory() = 0;

    virtual std::shared_ptr<P2PSession> getP2PSessionByNodeId(P2pID const& _nodeID) = 0;


    /**
     * @brief send message to the given p2p nodes
     *
     * @param _type the message type
     * @param _dstNodeID the dst node
     * @param _payload the data
     * @param options timeout option
     * @param _callback called when receive response
     */
    virtual void asyncSendMessageByP2PNodeID(int16_t _type, P2pID _dstNodeID,
        bytesConstRef _payload, Options options, P2PResponseCallback _callback) = 0;

    /**
     * @brief broadcast message to all p2p nodes
     *
     * @param _type the message type
     * @param _payload the payload
     */
    virtual void asyncBroadcastMessageToP2PNodes(
        int16_t _type, bytesConstRef _payload, Options _options) = 0;

    /**
     * @brief send message to the given nodeIDs
     */
    virtual void asyncSendMessageByP2PNodeIDs(int16_t _type, const std::vector<P2pID>& _nodeIDs,
        bytesConstRef _payload, Options _options) = 0;
};

}  // namespace gateway

}  // namespace bcos
