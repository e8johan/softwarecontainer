
# Copyright (C) 2016 Pelagicore AB
#
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
# WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
# BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
# ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
# SOFTWARE.
#
# For further information see LICENSE

generate_dbuscpp_hfile(${CMAKE_CURRENT_SOURCE_DIR}/softwarecontainer-agent.xml "SoftwareContainerAgent")

set(SOFTWARECONTAINERAGENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(softwarecontainer-agent
    src/main.cpp
    src/softwarecontaineragent.cpp
    src/softwarecontaineragentadaptor.cpp
    SoftwareContainerAgent_dbuscpp_adaptor.h
)

TARGET_LINK_LIBRARIES( softwarecontainer-agent
    ${DBusCpp_LIBRARIES}
    ${Glibmm_LIBRARIES}
    ${LXC_LIBRARIES}
    ${Jansson_LIBRARIES}
    ${IVILogging_LIBRARIES}
    softwarecontainerlib
)

INSTALL(TARGETS softwarecontainer-agent DESTINATION bin)

#
# When we install config files, we check for if prefix is /usr, and we treat
# this as installing system-wide, which means some config files should also
# be installed system-wide to other locations than they would otherwise have
# been installed to.
#

# Install the D-Bus configuration file
if (${CMAKE_INSTALL_PREFIX} STREQUAL "/usr")
    set(DBUS_SYSTEM_BUS_CONFIGURATION_FILES_DIR "/usr/share/dbus-1/system.d/")
else()
    set(DBUS_SYSTEM_BUS_CONFIGURATION_FILES_DIR "/etc/dbus-1/system.d")
endif()
install(FILES softwarecontainer-agent.conf DESTINATION ${DBUS_SYSTEM_BUS_CONFIGURATION_FILES_DIR})

# Install systemd configuration
if (ENABLE_SYSTEMD)
    message("SystemD file installation enabled")

    if (${CMAKE_INSTALL_PREFIX} STREQUAL "/usr")
        set(SYSTEMD_CONFIGURATION_FILES_DIR "/lib/systemd/system")
    else()
        set(SYSTEMD_CONFIGURATION_FILES_DIR "systemd/system")
    endif()
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/softwarecontainer-agent.service.in  ${CMAKE_CURRENT_BINARY_DIR}/softwarecontainer-agent.service)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/softwarecontainer-agent.service DESTINATION ${SYSTEMD_CONFIGURATION_FILES_DIR})
endif()

if (ENABLE_TEST)
   add_subdirectory(unit-test)
endif()
