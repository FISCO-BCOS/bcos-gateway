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
 * @file AMOPImpl.cpp
 * @author: octopus
 * @date 2021-10-26
 */
#include "AMOPImpl.h"
#include <bcos-framework/interfaces/protocol/CommonError.h>
#include <bcos-gateway/libnetwork/Common.h>
#include <boost/bind/bind.hpp>
using namespace bcos;
using namespace bcos::gateway;
using namespace bcos::amop;
using namespace bcos::protocol;

AMOPImpl::AMOPImpl(TopicManager::Ptr _topicManager, MessageFactory::Ptr _messageFactory,
    AMOPRequestFactory::Ptr _requestFactory, P2PInterface::Ptr _network, P2pID const& _p2pNodeID)
  : m_topicManager(_topicManager),
    m_messageFactory(_messageFactory),
    m_requestFactory(_requestFactory),
    m_network(_network),
    m_p2pNodeID(_p2pNodeID)
{
    m_threadPool = std::make_shared<ThreadPool>("amopDispatcher", 1);
    m_timer = std::make_shared<Timer>(TOPIC_SYNC_PERIOD, "topicSync");
    m_timer->registerTimeoutHandler([this]() { broadcastTopicSeq(); });
    m_network->registerHandlerByMsgType(MessageType::AMOPMessageType,
        boost::bind(&AMOP::onAMOPMessage, this, boost::placeholders::_1, boost::placeholders::_2,
            boost::placeholders::_3));
}

void AMOPImpl::start()
{
    m_timer->start();
}

void AMOPImpl::stop()
{
    m_timer->stop();
}

void AMOPImpl::broadcastTopicSeq()
{
    auto topicSeq = std::to_string(m_topicManager->topicSeq());
    auto buffer = buildAndEncodeMessage(
        AMOPMessage::Type::TopicSeq, bytesConstRef((byte*)topicSeq.data(), topicSeq.size()));
    m_network->asyncBroadcastMessageToP2PNodes(
        MessageType::AMOPMessageType, ref(*buffer), Options(0));
    AMOP_LOG(TRACE) << LOG_BADGE("broadcastTopicSeq") << LOG_KV("topicSeq", topicSeq);
    m_timer->restart();
}

// receive the topic seq of other nodes, and try to request the latest topic when seq falling behind
void AMOPImpl::onReceiveTopicSeqMessage(P2pID const& _nodeID, AMOPMessage::Ptr _msg)
{
    try
    {
        uint32_t topicSeq =
            boost::lexical_cast<uint32_t>(std::string(_msg->data().begin(), _msg->data().end()));
        if (!m_topicManager->checkTopicSeq(_nodeID, topicSeq))
        {
            AMOP_LOG(TRACE) << LOG_BADGE("onReceiveTopicSeqMessage") << LOG_KV("nodeID", _nodeID)
                            << LOG_KV("topicSeq", topicSeq);
            return;
        }

        AMOP_LOG(INFO) << LOG_BADGE("onReceiveTopicSeqMessage") << LOG_KV("nodeID", _nodeID)
                       << LOG_KV("topicSeq", topicSeq);

        auto buffer = buildAndEncodeMessage(AMOPMessage::Type::RequestTopic, bytesConstRef());
        Options option(0);
        m_network->asyncSendMessageByP2PNodeID(MessageType::AMOPMessageType, _nodeID,
            bytesConstRef(buffer->data(), buffer->size()), option,
            [_nodeID](Error::Ptr&& _error, int16_t, bytesPointer) {
                if (_error && (_error->errorCode() != CommonError::SUCCESS))
                {
                    AMOP_LOG(WARNING)
                        << LOG_BADGE("onReceiveTopicSeqMessage")
                        << LOG_DESC("receive error callback") << LOG_KV("dstNode", _nodeID)
                        << LOG_KV("errorCode", _error->errorCode())
                        << LOG_KV("errorMessage", _error->errorMessage());
                    return;
                }
            });
    }
    catch (const std::exception& e)
    {
        AMOP_LOG(ERROR) << LOG_DESC("onReceiveTopicSeqMessage") << LOG_KV("nodeID", _nodeID)
                        << LOG_KV("error", boost::diagnostic_information(e));
    }
}

/**
 * @brief: create message and encode the message to bytes
 * @param _type: message type
 * @param _data: message data
 * @return std::shared_ptr<bytes>
 */
std::shared_ptr<bytes> AMOPImpl::buildAndEncodeMessage(uint32_t _type, bcos::bytesConstRef _data)
{
    auto message = m_messageFactory->buildMessage();
    message->setType(_type);
    message->setData(_data);
    auto buffer = std::make_shared<bytes>();
    message->encode(*buffer.get());
    return buffer;
}

// receive topic response and update the local topicManager
void AMOPImpl::onReceiveResponseTopicMessage(P2pID const& _nodeID, AMOPMessage::Ptr _msg)
{
    try
    {
        uint32_t topicSeq;
        TopicItems topicItems;
        std::string topicJson = std::string(_msg->data().begin(), _msg->data().end());
        if (m_topicManager->parseTopicItemsJson(topicSeq, topicItems, topicJson))
        {
            m_topicManager->updateSeqAndTopicsByNodeID(_nodeID, topicSeq, topicItems);
        }
    }
    catch (const std::exception& e)
    {
        AMOP_LOG(ERROR) << LOG_BADGE("onReceiveResponseTopicMessage") << LOG_KV("nodeID", _nodeID)
                        << LOG_KV("error", boost::diagnostic_information(e));
    }
}

// response topic message to the given node
void AMOPImpl::onReceiveRequestTopicMessage(P2pID const& _nodeID, AMOPMessage::Ptr _msg)
{
    (void)_msg;
    try
    {
        // the current node subscribed topic info
        std::string topicJson = m_topicManager->queryTopicsSubByClient();

        AMOP_LOG(INFO) << LOG_BADGE("onReceiveRequestTopicMessage") << LOG_KV("nodeID", _nodeID)
                       << LOG_KV("topicJson", topicJson);

        auto buffer = buildAndEncodeMessage(AMOPMessage::Type::ResponseTopic,
            bytesConstRef((byte*)topicJson.data(), topicJson.size()));
        Options option(0);
        m_network->asyncSendMessageByP2PNodeID(MessageType::AMOPMessageType, _nodeID,
            bytesConstRef(buffer->data(), buffer->size()), option,
            [_nodeID](Error::Ptr&& _error, int16_t, bytesPointer) {
                if (_error && (_error->errorCode() != CommonError::SUCCESS))
                {
                    AMOP_LOG(WARNING)
                        << LOG_BADGE("onReceiveRequestTopicMessage")
                        << LOG_DESC("callback respones error") << LOG_KV("dstNode", _nodeID)
                        << LOG_KV("errorCode", _error->errorCode())
                        << LOG_KV("errorMessage", _error->errorMessage());
                }
            });
    }
    catch (const std::exception& e)
    {
        AMOP_LOG(ERROR) << LOG_BADGE("onReceiveRequestTopicMessage") << LOG_KV("nodeID", _nodeID)
                        << LOG_KV("error", boost::diagnostic_information(e));
    }
}

// receive AMOP request message from the given node
void AMOPImpl::onReceiveAMOPMessage(P2pID const& _nodeID, AMOPMessage::Ptr _msg,
    std::function<void(bytesPointer, int16_t)> const& _responseCallback)
{
    AMOP_LOG(TRACE) << LOG_BADGE("onReceiveAMOPMessage") << LOG_KV("nodeID", _nodeID);
    // AMOPRequest
    auto request = m_requestFactory->buildRequest(_msg->data());
    // message seq
    std::string topic = request->topic();
    std::vector<std::string> clients;
    m_topicManager->queryClientsByTopic(topic, clients);
    if (clients.empty())
    {
        auto amopMsg = m_messageFactory->buildMessage();
        auto buffer = std::make_shared<bcos::bytes>();
        amopMsg->setStatus(CommonError::NotFoundClientByTopicDispatchMsg);
        amopMsg->setType(AMOPMessage::Type::AMOPResponse);
        amopMsg->encode(*buffer);
        m_threadPool->enqueue([buffer, _responseCallback]() {
            _responseCallback(buffer, MessageType::AMOPMessageType);
        });
        AMOP_LOG(WARNING) << LOG_BADGE("onRecvAMOPMessage")
                          << LOG_DESC("no client subscribe the topic") << LOG_KV("topic", topic)
                          << LOG_KV("nodeID", _nodeID);
        return;
    }
    auto choosedClient = randomChoose(clients);
    auto amopRequestData = std::make_shared<bytes>();
    _msg->encode(*amopRequestData);
    auto clientService = m_topicManager->getServiceByClient(choosedClient);
    clientService->asyncNotifyAMOPMessage(bcos::rpc::AMOPNotifyMessageType::Unicast, topic,
        bytesConstRef(amopRequestData->data(), amopRequestData->size()),
        [_responseCallback](Error::Ptr&& _error, bytesPointer _responseData) {
            if (!_error || _error->errorCode() == CommonError::SUCCESS)
            {
                _responseCallback(_responseData, MessageType::WSMessageType);
                return;
            }
            auto const& errorMessage = _error->errorMessage();
            auto response = std::make_shared<bytes>(errorMessage.begin(), errorMessage.end());
            _responseCallback(response, MessageType::WSMessageType);
        });
}

// receive the AMOP broadcast message from given node
void AMOPImpl::onReceiveAMOPBroadcastMessage(P2pID const& _nodeID, AMOPMessage::Ptr _msg)
{
    // AMOPRequest
    auto request = m_requestFactory->buildRequest(_msg->data());
    // message seq
    std::string topic = request->topic();
    std::vector<std::string> clients;
    m_topicManager->queryClientsByTopic(topic, clients);
    if (clients.empty())
    {
        AMOP_LOG(WARNING) << LOG_BADGE("onRecvAMOPBroadcastMessage")
                          << LOG_DESC("no client subscribe the topic") << LOG_KV("topic", topic);
        return;
    }
    auto amopRequestData = std::make_shared<bytes>();
    _msg->encode(*amopRequestData);
    for (const auto& client : clients)
    {
        auto clientService = m_topicManager->getServiceByClient(client);
        AMOP_LOG(TRACE) << LOG_BADGE("onRecvAMOPBroadcastMessage")
                        << LOG_DESC("push message to client") << LOG_KV("topic", topic)
                        << LOG_KV("client", client);
        clientService->asyncNotifyAMOPMessage(bcos::rpc::AMOPNotifyMessageType::Broadcast, topic,
            bytesConstRef(amopRequestData->data(), amopRequestData->size()),
            [client](Error::Ptr&& _error, bytesPointer) {
                if (_error)
                {
                    AMOP_LOG(WARNING)
                        << LOG_BADGE("onRecvAMOPBroadcastMessage")
                        << LOG_DESC("asyncNotifyAMOPMessage error") << LOG_KV("client", client)
                        << LOG_KV("code", _error->errorCode())
                        << LOG_KV("msg", _error->errorMessage());
                }
            });
    }
    AMOP_LOG(TRACE) << LOG_DESC("onReceiveAMOPBroadcastMessage") << LOG_KV("nodeID", _nodeID);
}


// asyncSendMessage to the given topic
void AMOPImpl::asyncSendMessageByTopic(const std::string& _topic, bcos::bytesConstRef _data,
    std::function<void(bcos::Error::Ptr&&, int16_t, bytesPointer)> _respFunc)
{
    std::vector<P2pID> nodeIDs;
    m_topicManager->queryNodeIDsByTopic(_topic, nodeIDs);
    if (nodeIDs.empty())
    {
        auto errorPtr = std::make_shared<Error>(CommonError::NotFoundPeerByTopicSendMsg,
            "there has no node subscribe this topic, topic: " + _topic);
        if (_respFunc)
        {
            _respFunc(std::move(errorPtr), 0, nullptr);
        }

        AMOP_LOG(WARNING) << LOG_BADGE("asyncSendMessage")
                          << LOG_DESC("there has no node subscribe the topic")
                          << LOG_KV("topic", _topic);
        return;
    }
    auto buffer = buildAndEncodeMessage(AMOPMessage::Type::AMOPRequest, _data);

    class RetrySender : public std::enable_shared_from_this<RetrySender>
    {
    public:
        std::vector<P2pID> m_nodeIDs;
        std::shared_ptr<bytes> m_buffer;
        std::function<void(bcos::Error::Ptr&&, int16_t, bytesPointer)> m_callback;
        P2PInterface::Ptr m_network;

    public:
        void sendMessage()
        {
            if (m_nodeIDs.empty())
            {
                auto errorPtr = std::make_shared<Error>(
                    CommonError::AMOPSendMsgFailed, "unable to send message to peer by topic");
                if (m_callback)
                {
                    m_callback(std::move(errorPtr), 0, nullptr);
                }

                return;
            }
            auto choosedNodeID = randomChoose(m_nodeIDs);
            // erase in case of select the same node when retry
            m_nodeIDs.erase(m_nodeIDs.begin());
            // try to send message to node
            Options option(0);
            m_network->asyncSendMessageByP2PNodeID(MessageType::AMOPMessageType, choosedNodeID,
                bytesConstRef(m_buffer->data(), m_buffer->size()), option,
                [this, choosedNodeID](
                    Error::Ptr&& _error, int16_t _type, bytesPointer _responseData) {
                    if (_error && (_error->errorCode() != CommonError::SUCCESS))
                    {
                        AMOP_LOG(DEBUG)
                            << LOG_BADGE("RetrySender::sendMessage")
                            << LOG_DESC("asyncSendMessageByNodeID callback response error")
                            << LOG_KV("nodeID", choosedNodeID)
                            << LOG_KV("errorCode", _error->errorCode())
                            << LOG_KV("errorMessage", _error->errorMessage());
                        sendMessage();
                        return;
                    }
                    if (m_callback)
                    {
                        m_callback(nullptr, _type, _responseData);
                    }
                });
        }
    };

    auto sender = std::make_shared<RetrySender>();
    sender->m_nodeIDs = nodeIDs;
    sender->m_buffer = buffer;
    sender->m_network = m_network;
    sender->m_callback = _respFunc;
    // send message
    sender->sendMessage();
}

void AMOPImpl::asyncSendBroadbastMessageByTopic(
    const std::string& _topic, bcos::bytesConstRef _data)
{
    std::vector<std::string> nodeIDs;
    m_topicManager->queryNodeIDsByTopic(_topic, nodeIDs);
    if (nodeIDs.empty())
    {
        AMOP_LOG(WARNING) << LOG_BADGE("asyncSendBroadbastMessage")
                          << LOG_DESC("there no node subscribe this topic")
                          << LOG_KV("topic", _topic);
        return;
    }
    auto buffer = buildAndEncodeMessage(AMOPMessage::Type::AMOPBroadcast, _data);
    m_network->asyncSendMessageByP2PNodeIDs(MessageType::AMOPMessageType, nodeIDs,
        bytesConstRef(buffer->data(), buffer->size()), Options(0));
    AMOP_LOG(DEBUG) << LOG_BADGE("asyncSendBroadbastMessage") << LOG_DESC("send broadcast message")
                    << LOG_KV("topic", _topic) << LOG_KV("data size", _data.size());
}

void AMOPImpl::onAMOPMessage(
    NetworkException const& _e, P2PSession::Ptr _session, std::shared_ptr<P2PMessage> _message)
{
    m_threadPool->enqueue([this, _e, _session, _message]() {
        try
        {
            dispatcherAMOPMessage(_e, _session, _message);
        }
        catch (std::exception const& e)
        {
            AMOP_LOG(WARNING) << LOG_DESC("dispatcher AMOPMessage exception")
                              << LOG_KV("error", boost::diagnostic_information(e));
        }
    });
}

void AMOPImpl::dispatcherAMOPMessage(
    NetworkException const& _e, P2PSession::Ptr _session, std::shared_ptr<P2PMessage> _message)
{
    if (_e.errorCode() != 0 || !_message)
    {
        AMOP_LOG(WARNING) << LOG_DESC("onAMOPMessage error for NetworkException")
                          << LOG_KV("error", _e.what()) << LOG_KV("code", _e.errorCode());
        return;
    }
    if (_message->packetType() != MessageType::AMOPMessageType)
    {
        return;
    }
    auto amopMessage = m_messageFactory->buildMessage(ref(*_message->payload()));
    auto amopMsgType = amopMessage->type();
    auto fromNodeID = _session->p2pID();
    switch (amopMsgType)
    {
    case AMOPMessage::Type::TopicSeq:
        onReceiveTopicSeqMessage(fromNodeID, amopMessage);
        break;
    case AMOPMessage::Type::RequestTopic:
        onReceiveRequestTopicMessage(fromNodeID, amopMessage);
        break;
    case AMOPMessage::Type::ResponseTopic:
        onReceiveResponseTopicMessage(fromNodeID, amopMessage);
        break;
    case AMOPMessage::Type::AMOPRequest:
        onReceiveAMOPMessage(fromNodeID, amopMessage,
            [this, _session, _message](bytesPointer _responseData, int16_t _type) {
                auto responseP2PMsg = std::dynamic_pointer_cast<P2PMessage>(
                    m_network->messageFactory()->buildMessage());
                responseP2PMsg->setSeq(_message->seq());
                responseP2PMsg->setRespPacket();
                responseP2PMsg->setPayload(_responseData);
                responseP2PMsg->setPacketType(_type);
                _session->session()->asyncSendMessage(responseP2PMsg);
            });
        break;
    case AMOPMessage::Type::AMOPBroadcast:
        onReceiveAMOPBroadcastMessage(fromNodeID, amopMessage);
        break;
    default:
        AMOP_LOG(WARNING) << LOG_DESC("unkown AMOP message type") << LOG_KV("type", amopMsgType);
    }
}

void AMOPImpl::asyncRegisterClient(std::string const& _clientID, std::string const& _clientEndPoint,
    std::function<void(Error::Ptr&&)> _callback)
{
    m_topicManager->registerCient(_clientID, _clientEndPoint);
    if (!_callback)
    {
        return;
    }
    _callback(nullptr);
}

void AMOPImpl::asyncSubscribeTopic(std::string const& _clientID, std::string const& _topicInfo,
    std::function<void(Error::Ptr&&)> _callback)
{
    m_topicManager->subTopic(_clientID, _topicInfo);
    if (!_callback)
    {
        return;
    }
    _callback(nullptr);
}

void AMOPImpl::asyncRemoveTopic(std::string const& _clientID,
    std::vector<std::string> const& _topicList, std::function<void(Error::Ptr&&)> _callback)
{
    m_topicManager->removeTopics(_clientID, _topicList);
    if (!_callback)
    {
        return;
    }
    _callback(nullptr);
}