#include "stdafx.h"
#include "webpl_server.h"

#include <memory>

class webpl_initquit : public initquit {
public:
	void on_init() {
		m_server = std::make_unique<webpl_server>();
	}

	void on_quit() {
		m_server.reset();
	}

private:

	std::unique_ptr<webpl_server> m_server;
};


static initquit_factory_t<webpl_initquit> g_webpl_initquit_factory;