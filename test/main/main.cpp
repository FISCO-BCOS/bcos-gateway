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

#include <bcos-crypto/signature/key/KeyFactoryImpl.h>
#include <bcos-framework/interfaces/protocol/Protocol.h>
#include <bcos-framework/libutilities/Common.h>
#include <bcos-front/FrontServiceFactory.h>
#include <bcos-gateway/GatewayFactory.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace std;
using namespace bcos;
using namespace gateway;

#define GATEWAY_MAIN_LOG(LEVEL) LOG(LEVEL) << "[Gateway][MAIN]"

int main(int argc, const char **argv) {
  if ((argc == 2) &&
      ((std::string(argv[1]) == "-h") || (std::string(argv[1]) == "--help"))) {
    std::cerr << "./gateway-exec-mini groupID nodeID ./config.ini" << std::endl;
    return -1;
  }

  if (argc <= 3) {
    std::cerr << "please input groupID、nodeID、config path" << std::endl;
    return -1;
  }

  std::string groupID = argv[1];
  std::string nodeID = argv[2];
  std::string configPath = argv[3];

  try {
    auto keyFactory = std::make_shared<bcos::crypto::KeyFactoryImpl>();
    auto gatewayFactory = std::make_shared<bcos::gateway::GatewayFactory>();
    auto frontServiceFactory =
        std::make_shared<bcos::front::FrontServiceFactory>();
    auto threadPool = std::make_shared<ThreadPool>("frontServiceTest", 16);

    // build gateway
    auto gateway = gatewayFactory->buildGateway(configPath);

    // create nodeID by nodeID str
    auto nodeIDPtr = keyFactory->createKey(
        bytesConstRef((bcos::byte *)nodeID.data(), nodeID.size()));

    frontServiceFactory->setGatewayInterface(gateway);

    // create frontService
    auto frontService =
        frontServiceFactory->buildFrontService(groupID, nodeIDPtr);

    // register message dispather for front service
    frontService->registerModuleMessageDispatcher(
        bcos::protocol::ModuleID::AMOP,
        [](bcos::crypto::NodeIDPtr _nodeID, bytesConstRef _data) {
          // do nothing, print message
          GATEWAY_MAIN_LOG(INFO)
              << LOG_DESC(" ==> front service receive message")
              << LOG_KV("from", _nodeID->hex())
              << LOG_KV("data size()", _data.size());
        });

    frontService->start();
    // register front service to gateway
    gateway->registerFrontService(groupID, nodeIDPtr, frontService);

    // start gateway
    gateway->start();

    int i = 0;
    while (true) {
      i += 1;
      std::this_thread::sleep_for(std::chrono::seconds(10));

      std::string randStr =
          boost::uuids::to_string(boost::uuids::random_generator()());
      auto payload =
          bytesConstRef((bcos::byte *)randStr.data(), randStr.size());

      frontService->asyncGetNodeIDs(
          [frontService, i,
           payload](Error::Ptr _error,
                    std::shared_ptr<const crypto::NodeIDs> _nodeIDs) {
            (void)_error;
            if (!_nodeIDs || _nodeIDs->empty()) {
              return;
            }

            auto index = i % _nodeIDs->size();
            auto nodeID = (*_nodeIDs)[index];

            GATEWAY_MAIN_LOG(INFO) << LOG_DESC(" ==> nodeids")
                                   << LOG_KV("nodeIDs size", _nodeIDs->size())
                                   << LOG_KV("nodeID", nodeID->hex());

            frontService->asyncSendMessageByNodeID(
                bcos::protocol::ModuleID::AMOP, _nodeIDs, payload, 0,
                bcos::front::CallbackFunc());
          });
    }

    if (gateway) {
      gateway->stop();
    }

  } catch (const std::exception &e) {
    std::cerr << "exception throw, error: " << boost::diagnostic_information(e)
              << std::endl;
    return -1;
  }

  std::cout << "gateway program exit normally." << std::endl;
  return 0;
}