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
 * @file main.cpp
 * @author: octopus
 * @date 2021-05-25
 */

#include <clocale>
#include <iostream>
#include <thread>

#include "../common/FrontServiceBuilder.h"

using namespace std;
using namespace bcos;
using namespace gateway;

#define GATEWAY_MAIN_LOG(LEVEL) LOG(LEVEL) << "[Gateway][MAIN]"

int main(int argc, const char** argv)
{
    if ((argc == 2) && ((std::string(argv[1]) == "-h") || (std::string(argv[1]) == "--help")))
    {
        std::cerr << "./gateway-exec-mini groupID nodeID ./config.ini" << std::endl;
        return -1;
    }

    if (argc <= 3)
    {
        std::cerr << "please input groupID、nodeID、config path" << std::endl;
        return -1;
    }

    std::string groupID = argv[1];
    std::string nodeID = argv[2];
    std::string configPath = argv[3];

    try
    {
        auto keyFactory = std::make_shared<bcos::crypto::KeyFactoryImpl>();
        auto gatewayFactory = std::make_shared<bcos::gateway::GatewayFactory>();
        auto frontServiceFactory = std::make_shared<bcos::front::FrontServiceFactory>();
        auto threadPool = std::make_shared<ThreadPool>("frontServiceTest", 16);

        // build gateway
        auto gateway = gatewayFactory->buildGateway(configPath);

        // create nodeID by nodeID str
        auto nodeIDPtr =
            keyFactory->createKey(bytesConstRef((bcos::byte*)nodeID.data(), nodeID.size()));

        frontServiceFactory->setGatewayInterface(gateway);

        // create frontService
        auto frontService = frontServiceFactory->buildFrontService(groupID, nodeIDPtr);

        // register message dispather for front service
        frontService->registerModuleMessageDispatcher(bcos::protocol::ModuleID::AMOP,
            [](bcos::crypto::NodeIDPtr _nodeID, const std::string& _id, bytesConstRef _data) {
                // do nothing, print message
                GATEWAY_MAIN_LOG(INFO)
                    << LOG_DESC(" ==> echo") << LOG_KV("from", _nodeID->hex()) << LOG_KV("id", _id)
                    << LOG_KV("msg", std::string(_data.begin(), _data.end()));
            });

        frontService->start();
        // register front service to gateway
        gateway->registerFrontService(groupID, nodeIDPtr, frontService);

        // start gateway
        gateway->start();

        while (true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            frontService->asyncGetNodeIDs(
                [frontService](Error::Ptr _error, std::shared_ptr<const crypto::NodeIDs> _nodeIDs) {
                    (void)_error;
                    if (!_nodeIDs || _nodeIDs->empty())
                    {
                        return;
                    }

                    for (const auto& nodeID : *_nodeIDs)
                    {
                        std::string randStr =
                            boost::uuids::to_string(boost::uuids::random_generator()());
                        auto payload = bytesConstRef((bcos::byte*)randStr.data(), randStr.size());

                        GATEWAY_MAIN_LOG(INFO) << LOG_DESC("request") << LOG_KV("to", nodeID->hex())
                                               << LOG_KV("msg", randStr);

                        frontService->asyncSendMessageByNodeID(bcos::protocol::ModuleID::AMOP,
                            nodeID, payload, 0,
                            [randStr](Error::Ptr _error, bcos::crypto::NodeIDPtr _nodeID,
                                bytesConstRef _data, const std::string& _id,
                                bcos::front::ResponseFunc _respFunc) {
                                if (_error)
                                {
                                    GATEWAY_MAIN_LOG(ERROR)
                                        << LOG_DESC("response") << LOG_KV("from", _nodeID->hex())
                                        << LOG_KV("id", _id) << LOG_KV("error", _error->errorCode())
                                        << LOG_KV("msg", _error->errorMessage());
                                    return;
                                }

                                auto retMsg = std::string(_data.begin(), _data.end());
                                if (retMsg == randStr)
                                {
                                    GATEWAY_MAIN_LOG(INFO)
                                        << LOG_DESC("response ok") << LOG_KV("id", _id)
                                        << LOG_KV("from", _nodeID->hex());
                                }
                                else
                                {
                                    GATEWAY_MAIN_LOG(INFO)
                                        << LOG_DESC("response error") << LOG_KV("id", _id)
                                        << LOG_KV("from", _nodeID->hex())
                                        << LOG_KV("sendMsg", randStr) << LOG_KV("retMsg", retMsg);
                                }
                            });
                    }
                });
        }

        if (gateway)
        {
            gateway->stop();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "exception throw, error: " << boost::diagnostic_information(e) << std::endl;
        return -1;
    }

    std::cout << "gateway program exit normally." << std::endl;
    return 0;
}