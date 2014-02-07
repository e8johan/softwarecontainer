/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include "dbusproxy.h"
#include "debug.h"

DBusProxy::DBusProxy(const char *socket, const char *config, ProxyType type):
	m_socket(socket), m_type(type)
{
	log_debug("Spawning %s proxy, socket: %s, config: %s",
		typeString(), m_socket, config);

	m_pid = fork();
	if (m_pid == 0) { /* child */
		/* This call never returns... */
		execlp("dbus-proxy", "dbus-proxy", m_socket, typeString(),
			config, NULL);
	} else {
		if (m_pid == -1)
			log_error("Failed to spawn DBus proxy!");
	}
}

DBusProxy::~DBusProxy()
{
	if (kill(m_pid, SIGTERM) == -1) {
		log_error("Failed to kill %s proxy!", typeString());
	} else {
		log_debug("Killed %s proxy!", typeString());
	}

	if (remove(m_socket) == -1) {
		log_error("Failed to remove %s proxy socket!", typeString());
	} else {
		log_debug("Removed %s proxy socket!", typeString());
	}
}

std::string DBusProxy::id()
{
	return "dbus-proxy";
}

bool DBusProxy::setConfig(const std::string &config)
{
	return true;
}

bool DBusProxy::activate()
{
	return true;
}

const char *DBusProxy::typeString()
{
	if (m_type == SessionProxy) {
		return "session";
	} else {
		return "system";
	}
}

const char *DBusProxy::socketName()
{
	// Return the filename after stripping directory info
	std::string socket(m_socket);
	return socket.substr(socket.rfind('/') + 1).c_str();
}

std::string DBusProxy::environment()
{
	log_debug("Requesting environment for %s with socket %s",
		typeString(), m_socket);

	std::string env;
	env += "DBUS_";
	if (m_type == SessionProxy) {
		env += "SESSION";
	} else {
		env += "SYSTEM";
	}
	env += "_BUS_ADDRESS=unix:path=";
	env += "/deployed_app/";
	env += socketName();

	return env;
}