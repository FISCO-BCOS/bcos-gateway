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
 * @file P2PVersion.h
 * @author: octopus
 * @date 2021-06-16
 */

#pragma once
#include <cstdint>
#include <memory>
#include <utility>

namespace bcos
{
namespace gateway
{
enum ProtocolVersion : uint32_t
{
    None = 0,
    v1 = 1,
    minVersion = v1,
    maxVersion = v1,
};
class P2PVersion
{
public:
    using Ptr = std::shared_ptr<P2PVersion>;

public:
    std::string protocolVersionPairJson();
    std::pair<uint32_t, uint32_t> protocolVersionPair();
    std::pair<bool, std::pair<uint32_t, uint32_t> > fromProtocolVersionPairJson(
        const std::string& _json);
};
}  // namespace gateway
}  // namespace bcos
