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

#include <gateway/GatewayFactory.h>

using namespace std;
using namespace bcos;
using namespace gateway;

int main(int argc, const char **argv) {
  if (argc <= 1) {
    std::cerr << "please input config path" << std::endl;
    return -1;
  }

  std::string configPath = argv[1];
  if ((configPath == "-h") || (configPath == "--help")) {
    std::cerr << "./gateway-exec-mini ./config.ini" << std::endl;
    return -1;
  }

  try {
    auto factory = std::make_shared<bcos::gateway::GatewayFactory>();
    auto gateway = factory->buildGateway(configPath);
    gateway->start();

    while (true) {
      std::this_thread::sleep_for(std::chrono::seconds(10));
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