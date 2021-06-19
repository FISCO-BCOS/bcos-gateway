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
 * @file P2PVersion.cpp
 * @author: octopus
 * @date 2021-06-16
 */

#include <bcos-gateway/libp2p/Common.h>
#include <bcos-gateway/libp2p/P2PVersion.h>
#include <json/json.h>

using namespace bcos;
using namespace bcos::gateway;

std::pair<uint32_t, uint32_t> P2PVersion::protocolVersionPair()
{
    return std::make_pair<uint32_t, uint32_t>(
        ProtocolVersion::minVersion, ProtocolVersion::maxVersion);
}

std::string P2PVersion::protocolVersionPairJson()
{
    P2PVERSION_LOG(INFO) << LOG_KV("minVersion", ProtocolVersion::minVersion)
                         << LOG_KV("maxVersion", ProtocolVersion::maxVersion);
    auto jsonValue = "{\"minVersion\":" + std::to_string(ProtocolVersion::minVersion) +
                     ",\"maxVersion\":" + std::to_string(ProtocolVersion::maxVersion) + "}";
    return jsonValue;
}

std::pair<bool, std::pair<uint32_t, uint32_t>> P2PVersion::fromProtocolVersionPairJson(
    const std::string& _json)
{
    try
    {
        Json::Value root;
        Json::Reader jsonReader;
        if (jsonReader.parse(_json, root))
        {
            uint minVersion = root["minVersion"].asUInt();
            uint maxVersion = root["maxVersion"].asUInt();
            P2PVERSION_LOG(INFO) << LOG_DESC("fromProtocolVersionPairJson")
                                 << LOG_KV("minVersion", minVersion)
                                 << LOG_KV("maxVersion", maxVersion);
            return std::pair<bool, std::pair<uint32_t, uint32_t>>(
                true, std::pair<uint32_t, uint32_t>(minVersion, maxVersion));
        }

        P2PVERSION_LOG(ERROR) << LOG_DESC("unable parse json") << LOG_KV("json", _json);
    }
    catch (const std::exception& e)
    {
        P2PVERSION_LOG(ERROR) << LOG_DESC("cannot parse json") << LOG_KV("json", _json);
    }

    return std::pair<bool, std::pair<uint32_t, uint32_t>>(false, std::pair<uint32_t, uint32_t>());
}
