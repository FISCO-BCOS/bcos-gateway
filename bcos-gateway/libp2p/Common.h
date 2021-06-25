/*
 * Common.h
 *
 *      Author: ancelmo
 */

#pragma once

#include <bcos-framework/libutilities/Common.h>
#include <bcos-framework/libutilities/Exceptions.h>
#include <bcos-gateway/libnetwork/Common.h>

namespace bcos
{
namespace gateway
{
#define P2PMSG_LOG(LEVEL) BCOS_LOG(LEVEL) << "[P2PService][P2PMessage]"
#define P2PSESSION_LOG(LEVEL) BCOS_LOG(LEVEL) << "[P2PService][P2PSession]"
#define SERVICE_LOG(LEVEL) BCOS_LOG(LEVEL) << "[P2PService][Service]"

struct P2PSessionInfo
{
    P2PInfo p2pInfo;
    NodeIPEndpoint nodeIPEndpoint;
    P2PSessionInfo(P2PInfo const& _p2pInfo, NodeIPEndpoint const& _nodeIPEndpoint)
      : p2pInfo(_p2pInfo), nodeIPEndpoint(_nodeIPEndpoint)
    {}

    P2pID const& p2pID() const { return p2pInfo.p2pID; }
};
using P2PSessionInfos = std::vector<P2PSessionInfo>;

}  // namespace gateway
}  // namespace bcos
