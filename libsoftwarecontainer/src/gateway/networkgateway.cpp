
/*
 * Copyright (C) 2016 Pelagicore AB
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * For further information see LICENSE
 */


#include <cstring>
#include "ifaddrs.h"
#include "unistd.h"
#include "networkgateway.h"
#include "jansson.h"
#include "generators.h"

constexpr const char *NetworkGateway::BRIDGE_INTERFACE;

NetworkGateway::NetworkGateway() :
    Gateway(ID),
    m_internetAccess(false),
    m_interfaceInitialized(false)
{
}

NetworkGateway::~NetworkGateway()
{
}

ReturnCode NetworkGateway::readConfigElement(const json_t *element)
{
    if (!read(element, "internet-access", m_internetAccess)) {
        log_error() << "either key \"internet-access\" missing or not a bool in json configuration";
        return ReturnCode::FAILURE;
    }

    if (m_internetAccess) {
        log_info() << "Internet access will be enabled";

        if (!read(element, "gateway", m_gateway)) {
            log_error() << "either key \"gateway\" missing or not a string in json configuration";
        }

        if (m_gateway.length() == 0) {
            log_error() << "Value for \"gateway\" key is an empty string";
            return ReturnCode::FAILURE;
        }
    } else {
        log_info() << "Internet access will be disabled";
    }
    return ReturnCode::SUCCESS;
}

bool NetworkGateway::activateGateway()
{
    if (!hasContainer()) {
        log_error() << "activate was called on an EnvironmentGateway which has no associated container";
        return false;
    }

    if (m_gateway.size() != 0) {
        log_debug() << "Default gateway set to " << m_gateway;
    } else {
        m_internetAccess = false;
        log_debug() << "No gateway. Network access will be disabled";
    }

    if (!isBridgeAvailable()) {
        log_error() << "Bridge not available.";
        return false;
    }

    if (m_internetAccess) {
        generateIP();
        return up();
    } else {
        return down();
    }
}

bool NetworkGateway::teardownGateway()
{
    return true;
}

const std::string NetworkGateway::ip()
{
    return m_ip;
}

bool NetworkGateway::generateIP()
{
    log_debug() << "Generating ip-address";
    const char *ipAddrNet = m_gateway.substr(0, m_gateway.size() - 1).c_str();

    m_ip = m_generator.gen_ip_addr(ipAddrNet);
    log_debug() << "IP set to " << m_ip;

    return true;
}

bool NetworkGateway::setDefaultGateway()
{
    log_debug() << "Attempting to set default gateway";
    std::string cmdDel = "route del default gw " + m_gateway;
    executeInContainer(cmdDel);

    std::string cmdAdd = "route add default gw " + m_gateway;
    if (isError(executeInContainer(cmdAdd))) {
        log_error() << "Could not set default gateway.";
        return false;
    }
    return true;
}

bool NetworkGateway::up()
{
    log_debug() << "Attempting to configure eth0 to 'up state'";
    std::string cmd;

    if (!m_interfaceInitialized) {
        cmd = "ifconfig eth0 " + ip() + " netmask 255.255.255.0 up";
        m_interfaceInitialized = true;
    } else {
        cmd = "ifconfig eth0 up";
    }

    if (isError(executeInContainer(cmd))) {
        log_error() << "Configuring eth0 to 'up state' failed.";
        return false;
    }

    /* The route to the default gateway must be added
       each time the interface is brought up again */
    return setDefaultGateway();
}

bool NetworkGateway::down()
{
    log_debug() << "Attempting to configure eth0 to 'down state'";
    std::string cmd = "ifconfig eth0 down";
    if (isError(executeInContainer(cmd))) {
        log_error() << "Configuring eth0 to 'down state' failed.";
        return false;
    }
    return true;
}


bool NetworkGateway::isBridgeAvailable()
{
    log_debug() << "Checking bridge availability";
    std::string cmd = StringBuilder() << "ifconfig | grep -C 2 \""
                                      << BRIDGE_INTERFACE << "\" | grep -q \""
                                      << m_gateway << "\"";

    if (system(cmd.c_str()) == 0) {
        return true;
    } else {
        log_error() << "No network bridge configured";
    }

    return false;
}
