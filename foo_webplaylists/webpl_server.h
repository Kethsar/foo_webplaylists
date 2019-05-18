#pragma once

#include "stdafx.h"
#include "httplib.h"
#include <thread>

class webpl_server {
public:
	webpl_server();
	~webpl_server();

	titleformat_object_wrapper fmt_artist = titleformat_object_wrapper("%artist%");
	titleformat_object_wrapper fmt_title = titleformat_object_wrapper("%title%");
	titleformat_object_wrapper fmt_length = titleformat_object_wrapper("%length%");
	titleformat_object_wrapper fmt_seperator = titleformat_object_wrapper("%directoryname%[ / %album%]");

private:
	void register_server_endpoints();
	void start();

	void get_playlist(const httplib::Request& req, httplib::Response& res);
	void get_playlist_names(const httplib::Request& req, httplib::Response& res);
	void post_query_library(const httplib::Request& req, httplib::Response& res);
	void post_copy_playlist_tracks(const httplib::Request& req, httplib::Response& res);
	void post_remove_duplicates(const httplib::Request& req, httplib::Response& res);
	void post_move_playlist_items(const httplib::Request& req, httplib::Response& res);

	httplib::Server m_server;
	std::thread m_thread;
};