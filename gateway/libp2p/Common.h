/*
 * Common.h
 *
 *      Author: ancelmo
 */

#pragma once

#include <bcos-framework/libutilities/Common.h>
#include <bcos-framework/libutilities/Exceptions.h>
#include <gateway/libnetwork/Common.h>

namespace bcos {
namespace gateway {

#define P2PMSG_LOG(LEVEL) LOG(LEVEL) << "[P2PService][P2PMessage] "
#define P2PSESSION_LOG(LEVEL) LOG(LEVEL) << "[P2PService][P2PSession] "
#define SERVICE_LOG(LEVEL) LOG(LEVEL) << "[P2PService][Service] "

struct P2PSessionInfo {
  NodeInfo nodeInfo;
  NodeIPEndpoint nodeIPEndpoint;
  P2PSessionInfo(NodeInfo const &_nodeInfo,
                 NodeIPEndpoint const &_nodeIPEndpoint)
      : nodeInfo(_nodeInfo), nodeIPEndpoint(_nodeIPEndpoint) {}

  P2PNodeID const &p2pNodeID() const { return nodeInfo.nodeID; }
};
using P2PSessionInfos = std::vector<P2PSessionInfo>;

} // namespace gateway
} // namespace bcos
