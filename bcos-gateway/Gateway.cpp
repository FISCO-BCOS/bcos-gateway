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
 * @file Gateway.cpp
 * @author: octopus
 * @date 2021-04-19
 */

#include <bcos-framework/interfaces/protocol/CommonError.h>
#include <bcos-framework/libutilities/Common.h>
#include <bcos-framework/libutilities/Exceptions.h>
#include <bcos-gateway/Common.h>
#include <bcos-gateway/Gateway.h>

using namespace bcos;
using namespace bcos::protocol;
using namespace bcos::gateway;

void Gateway::start() {
  if (m_p2pInterface) {
    m_p2pInterface->start();
  }

  const auto &groupID2NodeID2FrontServiceInterface =
      m_gatewayNodeManager->groupID2NodeID2FrontServiceInterface();
  if (groupID2NodeID2FrontServiceInterface.empty()) {
    GATEWAY_LOG(WARNING) << LOG_DESC("gateway has no registered front service");
  } else {
    for (const auto &group : groupID2NodeID2FrontServiceInterface) {
      for (const auto &node : group.second) {
        GATEWAY_LOG(INFO) << LOG_DESC("start") << LOG_KV("groupID", group.first)
                          << LOG_KV("nodeID", node.first);
      }
    }
  }

  GATEWAY_LOG(INFO) << LOG_DESC("start end.");

  return;
}

void Gateway::stop() {
  if (m_p2pInterface) {
    m_p2pInterface->stop();
  }

  GATEWAY_LOG(INFO) << LOG_DESC("stop end.");
  return;
}

std::shared_ptr<P2PMessage> Gateway::newP2PMessage(
    const std::string &_groupID, bcos::crypto::NodeIDPtr _srcNodeID,
    bcos::crypto::NodeIDPtr _dstNodeID, bytesConstRef _payload) {

  auto message = std::static_pointer_cast<P2PMessage>(
      m_p2pInterface->messageFactory()->buildMessage());

  message->setPacketType(MessageType::PeerToPeerMessage);
  message->setSeq(m_p2pInterface->messageFactory()->newSeq());
  message->options()->setGroupID(_groupID);
  message->options()->setSrcNodeID(_srcNodeID->encode());
  message->options()->dstNodeIDs().push_back(_dstNodeID->encode());
  message->setPayload(
      std::make_shared<bytes>(_payload.begin(), _payload.end()));

  return message;
}

std::shared_ptr<P2PMessage>
Gateway::newP2PMessage(const std::string &_groupID,
                       bcos::crypto::NodeIDPtr _srcNodeID,
                       bytesConstRef _payload) {
  auto message = std::static_pointer_cast<P2PMessage>(
      m_p2pInterface->messageFactory()->buildMessage());

  message->setPacketType(MessageType::BroadcastMessage);
  message->setSeq(m_p2pInterface->messageFactory()->newSeq());
  message->options()->setGroupID(_groupID);
  message->options()->setSrcNodeID(_srcNodeID->encode());
  message->setPayload(
      std::make_shared<bytes>(_payload.begin(), _payload.end()));

  return message;
}

/**
 * @brief: send message with retry
 * @param _p2pMessage: p2pMessage packet
 * @param _p2pIDs: destination p2pIDs
 * @param _errorRespFunc: error func
 * @return void
 */
void Gateway::asyncSendMessageByNodeIDWithRetry(
    std::shared_ptr<P2PMessage> _p2pMessage, std::set<P2pID> _p2pIDs,
    ErrorRespFunc _errorRespFunc) {
  if (_p2pIDs.empty()) {
    GATEWAY_LOG(ERROR) << LOG_DESC("send message failed after retries")
                       << LOG_KV("seq", std::to_string(_p2pMessage->seq()));

    // TODO: define error
    auto errorPtr = std::make_shared<Error>(
        CommonError::TIMEOUT, "send message failed after retries");
    _errorRespFunc(errorPtr);
    return;
  }

  try {
    // fetch the first gateway
    auto p2pID = *_p2pIDs.begin();
    // remove the first gatewayLog
    _p2pIDs.erase(_p2pIDs.begin());

    auto gatewayWeakPtr = std::weak_ptr<Gateway>(shared_from_this());

    auto callback =
        [gatewayWeakPtr, _p2pMessage, _p2pIDs, p2pID, _errorRespFunc](
            NetworkException e, std::shared_ptr<P2PSession> session,
            std::shared_ptr<P2PMessage> message) {
          (void)session;

          auto gatewayPtr = gatewayWeakPtr.lock();
          if (e.errorCode() != P2PExceptionType::Success) {
            GATEWAY_LOG(ERROR)
                << LOG_DESC("asyncSendMessageByNodeIDWithRetry network error")
                << LOG_KV("seq", _p2pMessage->seq()) << LOG_KV("p2pid", p2pID)
                << LOG_KV("errorCode", e.errorCode())
                << LOG_KV("errorMessage", e.what());
            if (gatewayPtr) {
              // send failed and retry to next gateway again
              gatewayPtr->asyncSendMessageByNodeIDWithRetry(
                  _p2pMessage, _p2pIDs, _errorRespFunc);
            }
            return;
          }

          try {
            int retCode = boost::lexical_cast<int>(std::string(
                message->payload()->begin(), message->payload()->end()));
            if (retCode == CommonError::SUCCESS) {
              GATEWAY_LOG(TRACE)
                  << LOG_DESC("asyncSendMessageByNodeIDWithRetry send message "
                              "successfully")
                  << LOG_KV("seq", _p2pMessage->seq())
                  << LOG_KV("p2pid", p2pID);

              // send this message successfully
              _errorRespFunc(nullptr);
            } else {
              GATEWAY_LOG(ERROR)
                  << LOG_DESC("asyncSendMessageByNodeIDWithRetry"
                              " peer gateway unable dispatch this message")
                  << LOG_KV("seq", _p2pMessage->seq()) << LOG_KV("p2pid", p2pID)
                  << LOG_KV("retCode", retCode);
              if (gatewayPtr) {
                // send failed and retry to next gateway again
                gatewayPtr->asyncSendMessageByNodeIDWithRetry(
                    _p2pMessage, _p2pIDs, _errorRespFunc);
              }
            }

          } catch (const std::exception &e) {
            GATEWAY_LOG(ERROR)
                << LOG_DESC(
                       "asyncSendMessageByNodeIDWithRetry unexpected error")
                << LOG_KV("seq", _p2pMessage->seq()) << LOG_KV("p2pid", p2pID)
                << LOG_KV("error", e.what());
          }
        };

    // TODO: how to set timeout, set it to 10000ms default temporarily
    m_p2pInterface->asyncSendMessageByNodeID(p2pID, _p2pMessage, callback,
                                             Options(10000));
  } catch (const std::exception &e) {
    GATEWAY_LOG(ERROR) << LOG_DESC("asyncSendMessageByNodeIDWithRetry")
                       << LOG_KV("seq", _p2pMessage->seq())
                       << LOG_KV("error", boost::diagnostic_information(e));

    auto errorPtr = std::make_shared<Error>(
        CommonError::TIMEOUT,
        "an exception occurred when send this message, error: " +
            boost::diagnostic_information(e));
    _errorRespFunc(errorPtr);
  }
}

/**
 * @brief: register FrontService
 * @param _groupID: groupID
 * @param _nodeID: nodeID
 * @param _frontServiceInterface: FrontService
 * @return bool
 */
bool Gateway::registerFrontService(
    const std::string &_groupID, bcos::crypto::NodeIDPtr _nodeID,
    bcos::front::FrontServiceInterface::Ptr _frontServiceInterface) {
  return m_gatewayNodeManager->registerFrontService(_groupID, _nodeID,
                                                    _frontServiceInterface);
}

/**
 * @brief: unregister FrontService
 * @param _groupID: groupID
 * @param _nodeID: nodeID
 * @return bool
 */
bool Gateway::unregisterFrontService(const std::string &_groupID,
                                     bcos::crypto::NodeIDPtr _nodeID) {
  return m_gatewayNodeManager->unregisterFrontService(_groupID, _nodeID);
}

/**
 * @brief: send message
 * @param _groupID: groupID
 * @param _srcNodeID: the sender nodeID
 * @param _dstNodeID: the receiver nodeID
 * @param _payload: message payload
 * @param _errorRespFunc: error func
 * @return void
 */
void Gateway::asyncSendMessageByNodeID(const std::string &_groupID,
                                       bcos::crypto::NodeIDPtr _srcNodeID,
                                       bcos::crypto::NodeIDPtr _dstNodeID,
                                       bytesConstRef _payload,
                                       ErrorRespFunc _errorRespFunc) {

  std::set<P2pID> p2pIDs;
  if (!m_gatewayNodeManager->queryP2pIDs(_groupID, _dstNodeID->hex(), p2pIDs)) {
    GATEWAY_LOG(ERROR) << LOG_DESC(
                              "could not find a gateway to send this message")
                       << LOG_KV("groupID", _groupID)
                       << LOG_KV("srcNodeID", _srcNodeID->hex())
                       << LOG_KV("dstNodeID", _dstNodeID->hex());

    auto errorPtr = std::make_shared<Error>(
        CommonError::TIMEOUT, "could not find a gateway to "
                              "send this message, groupID:" +
                                  _groupID +
                                  " ,dstNodeID:" + _dstNodeID->hex());

    _errorRespFunc(errorPtr);
    return;
  }

  // new message object and try to send the message
  auto p2pMessage = newP2PMessage(_groupID, _srcNodeID, _dstNodeID, _payload);
  asyncSendMessageByNodeIDWithRetry(p2pMessage, p2pIDs, _errorRespFunc);
}

/**
 * @brief: send message to multiple nodes
 * @param _groupID: groupID
 * @param _srcNodeID: the sender nodeID
 * @param _nodeIDs: the receiver nodeIDs
 * @param _payload: message content
 * @return void
 */
void Gateway::asyncSendMessageByNodeIDs(
    const std::string &_groupID, bcos::crypto::NodeIDPtr _srcNodeID,
    const bcos::crypto::NodeIDs &_dstNodeIDs, bytesConstRef _payload) {
  for (auto dstNodeID : _dstNodeIDs) {
    asyncSendMessageByNodeID(
        _groupID, _srcNodeID, dstNodeID, _payload,
        [_groupID, _srcNodeID, dstNodeID](Error::Ptr _error) {
          if (!_error) {
            return;
          }
          GATEWAY_LOG(TRACE) << LOG_DESC("asyncSendMessageByNodeIDs callback")
                             << LOG_KV("groupID", _groupID)
                             << LOG_KV("srcNodeID", _srcNodeID->hex())
                             << LOG_KV("dstNodeID", dstNodeID->hex())
                             << LOG_KV("errorCode", _error->errorCode());
        });
  }
}

/**
 * @brief: send message to all nodes
 * @param _groupID: groupID
 * @param _srcNodeID: the sender nodeID
 * @param _payload: message content
 * @return void
 */
void Gateway::asyncSendBroadcastMessage(const std::string &_groupID,
                                        bcos::crypto::NodeIDPtr _srcNodeID,
                                        bytesConstRef _payload) {
  std::set<P2pID> p2pIDs;
  if (!m_gatewayNodeManager->queryP2pIDsByGroupID(_groupID, p2pIDs)) {
    GATEWAY_LOG(ERROR) << LOG_DESC("could not find a gateway "
                                   "to send this broadcast message")
                       << LOG_KV("groupID", _groupID)
                       << LOG_KV("srcNodeID", _srcNodeID->hex());
    return;
  }

  auto p2pMessage = newP2PMessage(_groupID, _srcNodeID, _payload);
  for (const P2pID &p2pID : p2pIDs) {
    m_p2pInterface->asyncSendMessageByNodeID(p2pID, p2pMessage,
                                             CallbackFuncWithSession());
  }

  GATEWAY_LOG(TRACE) << "asyncSendBroadcastMessage send message"
                     << LOG_KV("groupID", _groupID);
}

/**
 * @brief: receive p2p message from p2p network module
 * @param _groupID: groupID
 * @param _srcNodeID: the sender nodeID
 * @param _dstNodeID: the receiver nodeID
 * @param _payload: message content
 * @param _callback: callback
 * @return void
 */
void Gateway::onReceiveP2PMessage(const std::string &_groupID,
                                  bcos::crypto::NodeIDPtr _srcNodeID,
                                  bcos::crypto::NodeIDPtr _dstNodeID,
                                  bytesConstRef _payload,
                                  ErrorRespFunc _errorRespFunc) {
  bcos::front::FrontServiceInterface::Ptr frontServiceInterface =
      m_gatewayNodeManager->queryFrontServiceInterfaceByGroupIDAndNodeID(
          _groupID, _dstNodeID);
  if (!frontServiceInterface) {
    GATEWAY_LOG(ERROR) << LOG_DESC("onReceiveP2PMessage unable to find front "
                                   "service to dispatch this message")
                       << LOG_KV("groupID", _groupID)
                       << LOG_KV("srcNodeID", _srcNodeID->hex())
                       << LOG_KV("dstNodeID", _dstNodeID->hex());

    auto errorPtr = std::make_shared<Error>(
        CommonError::TIMEOUT, "unable to find front service dispath message to "
                              "groupID:" +
                                  _groupID + " ,nodeID:" + _dstNodeID->hex());

    if (_errorRespFunc) {
      _errorRespFunc(errorPtr);
    }
    return;
  }

  frontServiceInterface->onReceiveMessage(
      _groupID, _srcNodeID, _payload,
      [_groupID, _srcNodeID, _dstNodeID, _errorRespFunc](Error::Ptr _error) {
        if (_errorRespFunc) {
          _errorRespFunc(_error);
        }
        GATEWAY_LOG(TRACE) << LOG_DESC("onReceiveP2PMessage callback")
                           << LOG_KV("groupID", _groupID)
                           << LOG_KV("srcNodeID", _srcNodeID->hex())
                           << LOG_KV("dstNodeID", _dstNodeID->hex())
                           << LOG_KV("errorCode",
                                     (_error ? _error->errorCode() : 0))
                           << LOG_KV("errorMessage",
                                     (_error ? _error->errorMessage() : ""));
      });
}

/**
 * @brief: receive group broadcast message
 * @param _groupID: groupID
 * @param _srcNodeID: the sender nodeID
 * @param _payload: message payload
 * @return void
 */
void Gateway::onReceiveBroadcastMessage(const std::string &_groupID,
                                        bcos::crypto::NodeIDPtr _srcNodeID,
                                        bytesConstRef _payload) {

  auto frontServiceInterfaces =
      m_gatewayNodeManager->queryFrontServiceInterfaceByGroupID(_groupID);
  for (const auto &frontServiceInterface : frontServiceInterfaces) {
    frontServiceInterface->onReceiveMessage(
        _groupID, _srcNodeID, _payload,
        [_groupID, _srcNodeID](Error::Ptr _error) {
          GATEWAY_LOG(TRACE)
              << LOG_DESC("onReceiveBroadcastMessage callback")
              << LOG_KV("groupID", _groupID)
              << LOG_KV("srcNodeID", _srcNodeID->hex())
              << LOG_KV("errorCode", (_error ? _error->errorCode() : 0))
              << LOG_KV("errorMessage", (_error ? _error->errorMessage() : ""));
        });
  }
}
