
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


#include <string>
#include "filegateway.h"

FileGateway::FileGateway()
    : Gateway(ID)
    , m_settings({})
{
}

ReturnCode FileGateway::readConfigElement(const json_t *element)
{
    FileSetting setting;

    read(element, "path-host", setting.pathInHost);
    read(element, "path-container", setting.pathInContainer);
    read(element, "env-var-name", setting.envVarName);
    read(element, "env-var-prefix", setting.envVarPrefix);
    read(element, "env-var-suffix", setting.envVarSuffix);
    read(element, "create-symlink", setting.createSymlinkInContainer);
    read(element, "read-only", setting.readOnly);

    if (setting.pathInHost.size() == 0) {
        log_error() << "FileGateway config is lacking 'path-host' setting";
        return ReturnCode::FAILURE;
    }

    if (setting.pathInContainer.size() == 0) {
        log_error() << "FileGateway config is lacking 'path-container' setting";
        return ReturnCode::FAILURE;
    }

    m_settings.push_back(setting);
    return ReturnCode::SUCCESS;
}

bool FileGateway::activateGateway()
{
    if (m_settings.size() > 0) {
        for (FileSetting &setting : m_settings) {
            const std::string path = bindMount(setting);
            if (path.size() == 0) {
                log_error() << "Bind mount failed";
                return false;
            }

            if (setting.envVarName.size()) {
                std::string value = StringBuilder() << setting.envVarPrefix
                                                    << path << setting.envVarSuffix;
                setEnvironmentVariable(setting.envVarName, value);
            }

            if (setting.createSymlinkInContainer) {
                const std::string source = getContainer()->rootFS() + setting.pathInHost;
                getContainer()->createSymLink(source, path);
            }
        }
        return true;
    }

    return false;
}

std::string FileGateway::bindMount(const FileSetting &setting)
{
    std::string path;

    if (isDirectory(setting.pathInHost)) {
        if (isError(getContainer()->bindMountFolderInContainer(setting.pathInHost,
                                                    setting.pathInContainer, path,
                                                    setting.readOnly))) {
            log_error() << "Could not bind mount folder into container";
        }
    } else {
        if (isError(getContainer()->bindMountFileInContainer(setting.pathInHost,
                                                  setting.pathInContainer, path,
                                                  setting.readOnly))) {
            log_error() << "Could not bind mount file into container";
        }
    }
    return path;
}

bool FileGateway::teardownGateway()
{
    return true;
}
