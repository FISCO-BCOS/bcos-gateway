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
 * @file P2PMessage.cpp
 * @author: octopus
 * @date 2021-05-04
 */

#include <bcos-gateway/Common.h>
#include <bcos-gateway/libp2p/Common.h>
#include <bcos-gateway/libp2p/P2PMessage.h>
#include <boost/asio/detail/socket_ops.hpp>

using namespace bcos;
using namespace bcos::gateway;
using namespace bcos::crypto;

bool P2PMessage::encode(bytes& _buffer)
{
    _buffer.clear();

    // set length to zero first
    uint32_t length = MESSAGE_HEADER_LENGTH + m_payload->size();
    uint16_t version = boost::asio::detail::socket_ops::host_to_network_short(m_version);
    uint16_t packetType = boost::asio::detail::socket_ops::host_to_network_short(m_packetType);
    uint32_t seq = boost::asio::detail::socket_ops::host_to_network_long(m_seq);
    uint16_t ext = boost::asio::detail::socket_ops::host_to_network_short(m_ext);

    _buffer.insert(_buffer.end(), (byte*)&length, (byte*)&length + 4);
    _buffer.insert(_buffer.end(), (byte*)&version, (byte*)&version + 2);
    _buffer.insert(_buffer.end(), (byte*)&packetType, (byte*)&packetType + 2);
    _buffer.insert(_buffer.end(), (byte*)&seq, (byte*)&seq + 4);
    _buffer.insert(_buffer.end(), (byte*)&ext, (byte*)&ext + 2);
    _buffer.insert(_buffer.end(), m_payload->begin(), m_payload->end());

    return true;
}

ssize_t P2PMessage::decode(bytesConstRef _buffer)
{
    // check if packet header fully received
    if (_buffer.size() < P2PMessage::MESSAGE_HEADER_LENGTH)
    {
        return MessageDecodeStatus::MESSAGE_INCOMPLETE;
    }

    int32_t offset = 0;

    // length field
    m_length =
        boost::asio::detail::socket_ops::network_to_host_long(*((uint32_t*)&_buffer[offset]));
    offset += 4;

    // version
    m_version =
        boost::asio::detail::socket_ops::network_to_host_short(*((uint16_t*)&_buffer[offset]));
    offset += 2;

    // packetType
    m_packetType =
        boost::asio::detail::socket_ops::network_to_host_short(*((uint16_t*)&_buffer[offset]));
    offset += 2;

    // seq
    m_seq = boost::asio::detail::socket_ops::network_to_host_long(*((uint32_t*)&_buffer[offset]));
    offset += 4;

    // ext
    m_ext = boost::asio::detail::socket_ops::network_to_host_short(*((uint16_t*)&_buffer[offset]));
    offset += 2;

    // check if packet header fully received
    if (_buffer.size() < m_length)
    {
        return MessageDecodeStatus::MESSAGE_INCOMPLETE;
    }

    auto data = _buffer.getCroppedData(offset, m_length - offset);
    // payload
    m_payload = std::make_shared<bytes>(data.begin(), data.end());

    return m_length;
}
