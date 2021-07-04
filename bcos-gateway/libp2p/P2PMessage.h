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
 * @file P2PMessage.h
 * @author: octopus
 * @date 2021-05-04
 */

#pragma once

#include <bcos-framework/libutilities/Common.h>
#include <bcos-gateway/libnetwork/Common.h>
#include <bcos-gateway/libnetwork/Message.h>
#include <bcos-gateway/libp2p/P2PVersion.h>

namespace bcos
{
namespace gateway
{
// Message type definition
enum MessageType
{
    Heartbeat = 0x1,
    Handshake = 0x2,
    RequestNodeIDs = 0x3,
    ResponseNodeIDs = 0x4,
    GatewayMessage = 0x5
};

enum MessageExt
{
    Response = 0x0001,
};

enum MessageDecodeStatus
{
    MESSAGE_ERROR = -1,
    MESSAGE_INCOMPLETE = 0,
};


/// Message format definition of gateway P2P network
///
/// fields:
///   length            :4 bytes
///   version           :2 bytes
///   packet type       :2 bytes
///   seq               :4 bytes
///   ext               :2 bytes
///   payload           :X bytes
class P2PMessage : public Message
{
public:
    using Ptr = std::shared_ptr<P2PMessage>;

    /// length(4) + version(2) + packetType(2) + seq(4) + ext(2)
    const static size_t MESSAGE_HEADER_LENGTH = 14;

public:
    P2PMessage() { m_payload = std::make_shared<bytes>(); }

    virtual ~P2PMessage() {}

public:
    virtual uint32_t length() const override { return m_length; }
    // virtual void setLength(uint32_t length) override { m_length = length; }

    virtual uint16_t version() const override { return m_version; }
    virtual void setVersion(uint16_t version) override { m_version = version; }

    virtual uint16_t packetType() const override { return m_packetType; }
    virtual void setPacketType(uint16_t packetType) override { m_packetType = packetType; }

    virtual uint32_t seq() const override { return m_seq; }
    virtual void setSeq(uint32_t seq) override { m_seq = seq; }

    virtual uint16_t ext() const override { return m_ext; }
    virtual void setExt(uint16_t _ext) override { m_ext = _ext; }

    std::shared_ptr<bytes> payload() const override { return m_payload; }
    void setPayload(std::shared_ptr<bytes> _payload) override { m_payload = _payload; }

    virtual bool isRespPacket() const override { return (m_ext & MessageExt::Response) != 0; }
    virtual void setRespPacket() override { m_ext |= MessageExt::Response; }

public:
    virtual bool encode(bytes& _buffer) override;
    virtual ssize_t decode(bytesConstRef _buffer) override;


protected:
    uint32_t m_length = 0;
    uint16_t m_version = 0;
    uint16_t m_packetType = 0;
    uint32_t m_seq = 0;
    uint16_t m_ext = 0;

    std::shared_ptr<bytes> m_payload;  ///< payload data
};

class P2PMessageFactory : public MessageFactory
{
public:
    using Ptr = std::shared_ptr<P2PMessageFactory>;
    virtual ~P2PMessageFactory() {}

public:
    virtual Message::Ptr buildMessage() { return buildMessage(ProtocolVersion::v1); }
    virtual Message::Ptr buildMessage(uint32_t version)
    {
        (void)version;
        // TODO: Adapt multiple version formats
        auto message = std::make_shared<P2PMessage>();
        return message;
    }
};

inline std::ostream& operator<<(std::ostream& _out, const P2PMessage _p2pMessage)
{
    _out << "P2PMessage {"
         << " length: " << _p2pMessage.length() << " version: " << _p2pMessage.version()
         << " packetType: " << _p2pMessage.packetType() << " seq: " << _p2pMessage.seq()
         << " ext: " << _p2pMessage.ext() << " }";
    return _out;
}

inline std::ostream& operator<<(std::ostream& _out, P2PMessage::Ptr _p2pMessage)
{
    _out << (*_p2pMessage.get());
    return _out;
}

}  // namespace gateway
}  // namespace bcos
