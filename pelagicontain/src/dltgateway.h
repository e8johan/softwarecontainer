/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#pragma once

#include <string>
#include <unistd.h>
#include "gateway.h"
#include "systemcallinterface.h"

class DLTGateway: public Gateway {

	LOG_DECLARE_CLASS_CONTEXT("DLTG", "DLT gateway");

public:

	static constexpr const char* ID = "dlt";

	static constexpr const char* DLT_SOCKET_FOLDER = "/tmp/";
	static constexpr const char* SOCKET_FILE_NAME = "dlt";


	DLTGateway(SystemcallAbstractInterface &systemcallInterface,
			const std::string &gatewayDir, const std::string &name) :
			Gateway(), m_systemcallInterface(systemcallInterface) {
	}

	~DLTGateway() {
	}

	std::string id() override {
		return ID;
	}

	bool setConfig(const std::string &config) override {
		JSonParser parser(config);

		m_enabled = false;
		parser.readBoolean(ENABLED_FIELD, m_enabled);

		log_debug() << "Received config enabled: " << m_enabled;

		return true;
	}

	bool activate() override {
		if (m_enabled) {
			log_error() << "enabling DLT gateway";
			std::string d = logging::StringBuilder() << DLT_SOCKET_FOLDER << "/" << SOCKET_FILE_NAME;
			std::string path = getContainer().bindMountFileInContainer(d, SOCKET_FILE_NAME, false);
			createSymLinkInContainer(path, d);
		}

		return true;
	}

	bool teardown() override {
		return true;
	}

private:

	SystemcallAbstractInterface &m_systemcallInterface;
	bool m_enabled = false;
};
