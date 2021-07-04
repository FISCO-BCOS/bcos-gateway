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
/** @file GatewayMessage.h
 *  @author octopus
 *  @date 2021-07-04
 */

#pragma once

#include <bcos-framework/interfaces/crypto/KeyFactory.h>

namespace bcos
{
namespace gateway
{
enum GatewayMsgType
{
    PeerToPeerMessage = 0x1,
    BroadcastMessage = 0x2
};

class GatewayMessage : public std::enable_shared_from_this<GatewayMessage>
{
public:
    using Ptr = std::shared_ptr<GatewayMessage>;

public:
    /// groupID length(2) + srcNodeID length(2) + dstNodeID length(2)
    const static size_t OPTIONS_MIN_LENGTH = 6;

public:
    GatewayMessage()
    {
        m_srcNodeID = std::make_shared<bytes>();
        m_dstNodeID = std::make_shared<bytes>();
        m_payload = bcos::bytesConstRef();
    }

    virtual ~GatewayMessage() {}

    /// The maximum gateway transport protocol supported groupID length  65535
    const static size_t MAX_GROUPID_LENGTH = 65535;
    /// The maximum gateway transport protocol supported nodeID length  65535
    const static size_t MAX_NODEID_LENGTH = 65535;

    virtual bool encode(bytes& _buffer);
    virtual ssize_t decode(bytesConstRef _buffer);

public:
    uint32_t type() const { return m_type; }
    void setType(uint32_t type) { m_type = type; }

    std::string groupID() const { return m_groupID; }
    void setGroupID(const std::string& _groupID) { m_groupID = _groupID; }

    std::shared_ptr<bytes> srcNodeID() const { return m_srcNodeID; }
    void setSrcNodeID(std::shared_ptr<bytes> _srcNodeID) { m_srcNodeID = _srcNodeID; }

    std::shared_ptr<bytes> dstNodeID() { return m_dstNodeID; }
    void setDstNodeID(std::shared_ptr<bytes> _dstNodeID) { m_dstNodeID = _dstNodeID; }

    bcos::bytesConstRef payload() { return m_payload; }
    void setPayload(bcos::bytesConstRef _payload) { m_payload = _payload; }

protected:
    uint32_t m_type;
    std::string m_groupID;
    std::shared_ptr<bytes> m_srcNodeID;
    std::shared_ptr<bytes> m_dstNodeID;
    bcos::bytesConstRef m_payload;
};

class GatewayMessageFactory : public std::enable_shared_from_this<GatewayMessageFactory>
{
public:
    using Ptr = std::shared_ptr<GatewayMessageFactory>;

public:
    virtual GatewayMessage::Ptr buildMessage()
    {
        auto message = std::make_shared<GatewayMessage>();
        return message;
    }
};
}  // namespace gateway
}  // namespace bcos