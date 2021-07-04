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
/** @file P2PSession.h
 *  @author monan
 *  @date 20181112
 */

#pragma once

#include <bcos-gateway/libnetwork/Common.h>
#include <bcos-gateway/libnetwork/SessionFace.h>
#include <bcos-gateway/libp2p/Common.h>
#include <bcos-gateway/libp2p/P2PMessage.h>
#include <bcos-gateway/libp2p/P2PVersion.h>
#include <memory>

namespace bcos
{
namespace gateway
{
class P2PMessage;
class Service;

class P2PSession : public std::enable_shared_from_this<P2PSession>
{
public:
    using Ptr = std::shared_ptr<P2PSession>;

    P2PSession();

    virtual ~P2PSession();

    virtual void start();
    virtual void stop(DisconnectReason reason);
    virtual bool actived() { return m_run; }

    virtual SessionFace::Ptr session() { return m_session; }
    virtual void setSession(std::shared_ptr<SessionFace> session) { m_session = session; }

    virtual P2PMessageFactory::Ptr messageFactory() { return m_messageFactory; }
    virtual void setMessageFactory(P2PMessageFactory::Ptr messageFactory)
    {
        m_messageFactory = messageFactory;
    }

    virtual ProtocolVersion protocolVersion() const { return m_protocolVersion; }
    virtual void setProtocolVersion(ProtocolVersion protocolVersion)
    {
        m_protocolVersion = protocolVersion;
    }
    virtual P2pID p2pID() { return m_p2pInfo.p2pID; }
    virtual void setP2PInfo(P2PInfo const& p2pInfo) { m_p2pInfo = p2pInfo; }
    virtual P2PInfo const& p2pInfo() const& { return m_p2pInfo; }

    virtual std::weak_ptr<Service> service() { return m_service; }
    virtual void setService(std::weak_ptr<Service> service) { m_service = service; }

public:
    virtual void asyncSendMessage(uint32_t packetType, std::shared_ptr<bytes> payload,
        Options options, SessionCallbackFunc callback);
    virtual void asyncSendResponse(std::shared_ptr<bytes> payload, P2PMessage::Ptr _p2pMessage);

private:
    SessionFace::Ptr m_session;
    /// gateway p2p info;
    P2PInfo m_p2pInfo;
    // p2p protocol version, set by p2p handshake
    ProtocolVersion m_protocolVersion = ProtocolVersion::None;
    //
    P2PMessageFactory::Ptr m_messageFactory;

    std::weak_ptr<Service> m_service;
    bool m_run = false;
};

}  // namespace gateway
}  // namespace bcos
