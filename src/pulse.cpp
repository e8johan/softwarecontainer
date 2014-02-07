/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <stdio.h>
#include "pulse.h"
#include "debug.h"

Pulse::Pulse(const char *socket):
	m_api(0), m_context(0), m_socket(socket), m_index(-1)
{
	/* Create mainloop */
	m_mainloop = pa_threaded_mainloop_new();
	pa_threaded_mainloop_start(m_mainloop);

	if (m_mainloop) {
		/* Set up connection to pulse server */
		pa_threaded_mainloop_lock(m_mainloop);
		m_api = pa_threaded_mainloop_get_api(m_mainloop);
		m_context = pa_context_new(m_api, "pulsetest");
		pa_context_set_state_callback (m_context, stateCallback, this);

		int err = pa_context_connect(
			m_context,        /* context */
			NULL,              /* default server */
			PA_CONTEXT_NOFAIL, /* keep reconnection on failure */
			NULL );            /* use default spawn api */

		if (err != 0)
			log_error("Pulse error: %s", pa_strerror(err));

		pa_threaded_mainloop_unlock (m_mainloop);
	} else {
		log_error("Failed to create pulse mainloop->");
	}
}

Pulse::~Pulse()
{
	if (m_mainloop) {
		/* Unload module if module loaded successfully on startup */
		if (m_index != -1)
		{
			pa_threaded_mainloop_lock(m_mainloop);
			pa_context_unload_module(
				m_context,
				m_index,
				unloadCallback,
				this);
			pa_threaded_mainloop_wait(m_mainloop);
			pa_threaded_mainloop_unlock(m_mainloop);
		}

		/* Release pulse */
		pa_threaded_mainloop_lock(m_mainloop);
		pa_context_disconnect(m_context);
		pa_context_unref(m_context);
		pa_threaded_mainloop_unlock(m_mainloop);

		pa_threaded_mainloop_stop(m_mainloop);
		pa_threaded_mainloop_free(m_mainloop);
	}
	log_debug("pulse: Teardown complete");
}

const char *Pulse::socketName()
{
	std::string socket(m_socket);
	return socket.substr(socket.rfind('/') + 1).c_str();
}

std::string Pulse::environment()
{
	std::string env;
	env += "PULSE_SERVER=";
	env += "/deployed_app/";
	env += socketName();
	return env;
}

void Pulse::loadCallback(pa_context *c, uint32_t index, void *userdata)
{
	Pulse *p = static_cast<Pulse*>(userdata);
	p->m_index = (int)index;
	log_error("pulse: Loaded module %d", p->m_index);

	pa_threaded_mainloop_signal(p->m_mainloop, 0);
}

void Pulse::unloadCallback(pa_context *c, int success, void *userdata)
{
	Pulse *p = static_cast<Pulse*>(userdata);
	if (success)
		log_debug("pulse: Unloaded module %d", p->m_index);
	else
		log_debug("pulse: Failed to unload module %d", p->m_index);

	pa_threaded_mainloop_signal(p->m_mainloop, 0);
}

void Pulse::stateCallback(pa_context *context, void *userdata)
{
	Pulse *p = static_cast<Pulse*>(userdata);
	char socket[1031]; /* 1024 + 7 for "socket=" */

	switch (pa_context_get_state(context)) {
	case PA_CONTEXT_READY:
		log_debug("Connection is up, loading module");
		snprintf(socket, sizeof(socket), "socket=%s", p->m_socket);
		pa_context_load_module(
			context,
			"module-native-protocol-unix",
			socket,
			loadCallback,
			userdata);
		break;
	case PA_CONTEXT_CONNECTING:
		log_debug("pulse: Connecting");
		break;
	case PA_CONTEXT_AUTHORIZING:
		log_debug("pulse: Authorizing");
		break;
	case PA_CONTEXT_SETTING_NAME:
		log_debug("pulse: Setting name");
		break;
	case PA_CONTEXT_UNCONNECTED:
		log_debug("pulse: Unconnected");
		break;
	case PA_CONTEXT_FAILED:
		log_debug("pulse: Failed");
		break;
	case PA_CONTEXT_TERMINATED:
		log_debug("pulse: Terminated");
		break;
	}
}