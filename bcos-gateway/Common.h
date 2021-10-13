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
 * @file Common.h
 * @author: octopus
 * @date 2021-05-04
 */
#pragma once
#include <bcos-framework/interfaces/protocol/ServiceDesc.h>
#include <tarscpp/servant/Application.h>

#define GATEWAY_LOG(LEVEL) BCOS_LOG(LEVEL) << "[Gateway][Gateway]"
#define GATEWAY_CONFIG_LOG(LEVEL) BCOS_LOG(LEVEL) << "[Gateway][Config]"
#define GATEWAY_FACTORY_LOG(LEVEL) BCOS_LOG(LEVEL) << "[Gateway][Factory]"
#define INITIALIZER_LOG(LEVEL) BCOS_LOG(LEVEL) << "[Gateway][Initializer]"
#define NODE_MANAGER_LOG(LEVEL) BCOS_LOG(LEVEL) << "[Gateway][GatewayNodeManager]"

namespace bcos
{
namespace gateway
{
template <typename T, typename S, typename... Args>
std::shared_ptr<T> createServiceClient(
    std::string const& _appName, std::string const& _serviceName, const Args&... _args)
{
    auto servantName = bcos::protocol::getPrxDesc(_appName, _serviceName);
    auto prx = Application::getCommunicator()->stringToProxy<S>(servantName);
    return std::make_shared<T>(prx, _args...);
}
}  // namespace gateway
}  // namespace bcos