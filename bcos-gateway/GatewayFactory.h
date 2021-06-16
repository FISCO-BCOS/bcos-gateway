/*
 * @CopyRight:
 * FISCO-BCOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FISCO-BCOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FISCO-BCOS.  If not, see <http://www.gnu.org/licenses/>
 * (c) 2016-2018 fisco-dev contributors.
 */
/** @file GatewayFactory.h
 *  @author octopus
 *  @date 2021-05-17
 */

#pragma once

#include <bcos-framework/interfaces/crypto/KeyFactory.h>
#include <bcos-framework/interfaces/front/FrontServiceInterface.h>
#include <bcos-gateway/Gateway.h>
#include <bcos-gateway/GatewayConfig.h>
#include <boost/asio/ssl.hpp>

namespace bcos
{
namespace gateway
{
class GatewayFactory
{
public:
    using Ptr = std::shared_ptr<GatewayFactory>;

    GatewayFactory()
    {
        initCert2PubHexHandler();
        initSSLContextPubHexHandler();
    }

    virtual ~GatewayFactory() = default;

public:
    // init the function calc public hex from the cert
    void initCert2PubHexHandler();
    // init the function calc public key from the ssl context
    void initSSLContextPubHexHandler();

private:
    std::function<bool(X509* cert, std::string& pubHex)> m_sslContextPubHandler;

    std::function<bool(const std::string& priKey, std::string& pubHex)> m_certPubHexHandler;

public:
    std::function<bool(X509* cert, std::string& pubHex)> sslContextPubHandler()
    {
        return m_sslContextPubHandler;
    }

    std::function<bool(const std::string& priKey, std::string& pubHex)> certPubHexHandler()
    {
        return m_certPubHexHandler;
    }

public:
    // build ssl context
    std::shared_ptr<boost::asio::ssl::context> buildSSLContext(
        const GatewayConfig::CertConfig& _certConfig);
    // build sm ssl context
    std::shared_ptr<boost::asio::ssl::context> buildSSLContext(
        const GatewayConfig::SMCertConfig& _smCertConfig);

    /**
     * @brief: construct Gateway
     * @param _configPath: config.ini path
     * @return void
     */
    Gateway::Ptr buildGateway(const std::string& _configPath);
    /**
     * @brief: construct Gateway
     * @param _config: config parameter object
     * @return void
     */
    Gateway::Ptr buildGateway(GatewayConfig::Ptr _config);
};
}  // namespace gateway
}  // namespace bcos
