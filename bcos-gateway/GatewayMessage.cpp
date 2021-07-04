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
/** @file GatewayMessage.cpp
 *  @author octopus
 *  @date 2021-07-04
 */

#include <bcos-gateway/Common.h>
#include <bcos-gateway/GatewayMessage.h>
#include <boost/asio/detail/socket_ops.hpp>

using namespace bcos;
using namespace bcos::gateway;

#define CHECK_OFFSET_WITH_THROW_EXCEPTION(offset, length)                                    \
    do                                                                                       \
    {                                                                                        \
        if ((offset) > (length))                                                             \
        {                                                                                    \
            throw std::out_of_range("Out of range error, offset:" + std::to_string(offset) + \
                                    " ,length: " + std::to_string(length));                  \
        }                                                                                    \
    } while (0);

bool GatewayMessage::encode(bytes& _buffer)
{
    // parameters check
    if (m_groupID.size() > MAX_GROUPID_LENGTH)
    {
        GATEWAY_MSG_LOG(ERROR) << LOG_DESC("groupID length overflow")
                               << LOG_KV("groupID length", m_groupID.size());
        return false;
    }

    if (!m_srcNodeID || m_srcNodeID->empty() || (m_srcNodeID->size() > MAX_NODEID_LENGTH))
    {
        GATEWAY_MSG_LOG(ERROR) << LOG_DESC("srcNodeID length valid")
                               << LOG_KV(
                                      "srcNodeID length", (m_srcNodeID ? m_srcNodeID->size() : 0));
        return false;
    }

    if (m_dstNodeID && (m_dstNodeID->size() > MAX_NODEID_LENGTH))
    {
        GATEWAY_MSG_LOG(ERROR) << LOG_DESC("dstNodeID length valid")
                               << LOG_KV(
                                      "dstNodeID length", (m_dstNodeID ? m_dstNodeID->size() : 0));
        return false;
    }

    // groupID length
    uint16_t groupIDLength =
        boost::asio::detail::socket_ops::host_to_network_short((uint16_t)m_groupID.size());
    _buffer.insert(_buffer.end(), (byte*)&groupIDLength, (byte*)&groupIDLength + 2);
    // groupID
    _buffer.insert(_buffer.end(), m_groupID.begin(), m_groupID.end());

    // srcNodeID length
    uint16_t srcNodeIDLen =
        boost::asio::detail::socket_ops::host_to_network_short((uint16_t)m_srcNodeID->size());
    _buffer.insert(_buffer.end(), (byte*)&srcNodeIDLen, (byte*)&srcNodeIDLen + 2);
    // srcNodeID
    _buffer.insert(_buffer.end(), m_srcNodeID->begin(), m_srcNodeID->end());

    // dstNodeID length
    uint16_t dstNodeIDLen =
        boost::asio::detail::socket_ops::host_to_network_short((uint16_t)m_dstNodeID->size());
    _buffer.insert(_buffer.end(), (byte*)&dstNodeIDLen, (byte*)&dstNodeIDLen + 2);
    // dstNodeID
    _buffer.insert(_buffer.end(), m_dstNodeID->begin(), m_dstNodeID->end());
    _buffer.insert(_buffer.end(), m_payload.begin(), m_payload.end());

    return true;
}

///       groupID length    :1 bytes
///       groupID           : bytes
///       nodeID length     :2 bytes
///       src nodeID        : bytes
///       nodeID length     :2 bytes:
///       dst nodeIDs       : bytes
ssize_t GatewayMessage::decode(bytesConstRef _buffer)
{
    size_t offset = 0;
    size_t length = _buffer.size();

    try
    {
        CHECK_OFFSET_WITH_THROW_EXCEPTION((offset + OPTIONS_MIN_LENGTH), length);

        // groupID length
        uint16_t groupIDLength =
            boost::asio::detail::socket_ops::network_to_host_short(*((uint16_t*)&_buffer[offset]));
        offset += 2;

        // groupID
        if (groupIDLength > 0)
        {
            CHECK_OFFSET_WITH_THROW_EXCEPTION(offset + groupIDLength, length);
            m_groupID.assign(&_buffer[offset], &_buffer[offset] + groupIDLength);
            offset += groupIDLength;
        }

        // srcNodeID length
        CHECK_OFFSET_WITH_THROW_EXCEPTION(offset + 2, length);
        uint16_t srcNodeIDLen =
            boost::asio::detail::socket_ops::network_to_host_short(*((uint16_t*)&_buffer[offset]));
        offset += 2;

        if (srcNodeIDLen > 0)
        {
            CHECK_OFFSET_WITH_THROW_EXCEPTION(offset + srcNodeIDLen, length);
            m_srcNodeID->insert(m_srcNodeID->begin(), (byte*)&_buffer[offset],
                (byte*)&_buffer[offset] + srcNodeIDLen);
            offset += srcNodeIDLen;
        }

        // dstNodeID length
        CHECK_OFFSET_WITH_THROW_EXCEPTION(offset + 2, length);
        uint16_t dstNodeIDLen =
            boost::asio::detail::socket_ops::network_to_host_short(*((uint16_t*)&_buffer[offset]));
        offset += 2;

        if (dstNodeIDLen > 0)
        {
            CHECK_OFFSET_WITH_THROW_EXCEPTION(offset + dstNodeIDLen, length);
            m_dstNodeID->insert(m_dstNodeID->begin(), (byte*)&_buffer[offset],
                (byte*)&_buffer[offset] + dstNodeIDLen);
            offset += dstNodeIDLen;
        }

        m_payload = _buffer.getCroppedData(offset);
    }
    catch (const std::exception& e)
    {
        GATEWAY_MSG_LOG(ERROR) << LOG_DESC("decode message error")
                               << LOG_KV("e", boost::diagnostic_information(e));
        // invalid packet?
        return -1;
    }

    return offset;
}
