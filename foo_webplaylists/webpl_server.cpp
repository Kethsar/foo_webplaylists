#include "stdafx.h"
#include "utils.h"
#include "webpl_server.h"
#include "webpl_structs.h"
#include <mutex>
#include <condition_variable>

webpl_server::webpl_server() {
	register_server_endpoints();
	m_thread = std::thread([this] { start(); });
}

webpl_server::~webpl_server() {
	if (m_server.is_running())
		m_server.stop();
	if (m_thread.joinable())
		m_thread.join();
}

void webpl_server::register_server_endpoints() {
	m_server.Get("/", [](const httplib::Request& req, httplib::Response& res) {
		res.set_content("This seems to be working at least", "text/plain");
	});

	m_server.Get(R"(/api/playlist/(\d*))",
		std::bind(&webpl_server::get_playlist, this, std::placeholders::_1, std::placeholders::_2));

	m_server.Get("/api/playlist/names",
		std::bind(&webpl_server::get_playlist_names, this, std::placeholders::_1, std::placeholders::_2));

	m_server.Post("/api/query_library",
		std::bind(&webpl_server::post_query_library, this, std::placeholders::_1, std::placeholders::_2));

	m_server.Post("/api/playlist/copy_tracks",
		std::bind(&webpl_server::post_copy_playlist_tracks, this, std::placeholders::_1, std::placeholders::_2));

	m_server.Post("/api/playlist/copy_libtracks",
		std::bind(&webpl_server::post_copy_libtracks_to_playlist, this, std::placeholders::_1, std::placeholders::_2));

	m_server.Post("/api/playlist/remove/duplicates",
		std::bind(&webpl_server::post_remove_duplicates, this, std::placeholders::_1, std::placeholders::_2));

	m_server.Post("/api/playlist/remove",
		std::bind(&webpl_server::post_remove_playlist_items, this, std::placeholders::_1, std::placeholders::_2));

	m_server.Post("/api/playlist/move",
		std::bind(&webpl_server::post_move_playlist_items, this, std::placeholders::_1, std::placeholders::_2));
}

void webpl_server::get_playlist(const httplib::Request& req, httplib::Response& res) {
	Playlist pl;
	std::mutex mtx;
	std::condition_variable cv;
	std::string pl_index_str = req.matches[1].str();
	bool finished = false;

	auto f = std::bind(get_playlist_data, std::cref(pl_index_str), std::ref(pl), std::ref(fmt_artist), std::ref(fmt_title),
		std::ref(fmt_length), std::ref(fmt_seperator), std::ref(mtx), std::ref(cv), std::ref(finished));
	fb2k::inMainThread(f);

	std::unique_lock<std::mutex> lck(mtx);
	cv.wait(lck, [&] { return finished; });

	if (pl.playlist_index == -1) {
		json e;
		e["error"] = "Something broke";
		res.set_content(e.dump(), "application/json");
	}
	else if (pl.tracks.size() == 0) {
		json e;
		e["error"] = "Playlist (probably) had >10000 tracks";
		res.set_content(e.dump(), "application/json");
	}
	else {
		auto j = json(pl);
		res.set_content(j.dump(), "application/json");
	}
}

void webpl_server::get_playlist_names(const httplib::Request& req, httplib::Response& res) {
	std::vector<Playlist> playlists;
	std::mutex mtx;
	std::condition_variable cv;
	bool finished = false;

	auto f = std::bind(get_playlists, std::ref(playlists), std::ref(mtx), std::ref(cv), std::ref(finished));
	fb2k::inMainThread(f);

	std::unique_lock<std::mutex> lck(mtx);
	cv.wait(lck, [&] { return finished; });

	if (playlists.size() == 0) {
		json e;
		e["error"] = "Something broke";
		res.set_content(e.dump(), "application/json");
	}
	else {
		auto j = json(playlists);
		res.set_content(j.dump(), "application/json");
	}
}

void webpl_server::post_query_library(const httplib::Request& req, httplib::Response& res) {
	if (req.body.length() == 0) {
		json e;
		e["error"] = "Missing query data";
		res.set_content(e.dump(), "application/json");
		return;
	}

	json data = json::parse(req.body);
	if (!data.contains("q")) {
		json e;
		e["error"] = "Missing query key 'q' in json request";
		res.set_content(e.dump(), "application/json");
		return;
	}

	std::vector<LibraryTrack> libtracks;
	std::mutex mtx;
	std::condition_variable cv;
	auto query = data.at("q").get<std::string>();
	bool finished = false;

	auto f = std::bind(get_lib_query_items, query.c_str(), std::ref(libtracks), std::ref(fmt_artist),
		std::ref(fmt_title), std::ref(fmt_length), std::ref(mtx), std::ref(cv), std::ref(finished));
	fb2k::inMainThread(f);

	std::unique_lock<std::mutex> lck(mtx);
	cv.wait(lck, [&] { return finished; });

	auto j = json(libtracks);
	res.set_content(j.dump(), "application/json");
}

void webpl_server::post_copy_playlist_tracks(const httplib::Request& req, httplib::Response& res) {
	try {
		if (req.body.length() == 0) {
			json e;
			e["error"] = "Missing request data";
			res.set_content(e.dump(), "application/json");
			return;
		}

		auto data = json::parse(req.body);
		if (!data.contains("to") || !data.contains("from")) {
			json e;
			e["error"] = "Malformed request; Missing 'to' or 'from' keys";
			res.set_content(e.dump(), "application/json");
			return;
		}

		auto from = data["from"];
		if (!from.contains("index") || !from.contains("tracks")) {
			json e;
			e["error"] = "Malformed request; Missing 'index' or 'tracks' key in the 'from' object";
			res.set_content(e.dump(), "application/json");
			return;
		}

		std::mutex mtx;
		std::condition_variable cv;
		bool finished = false;
		auto to_index = data.at("to").get<t_size>();
		auto from_index = from.at("index").get<t_size>();
		auto track_indexes = from.at("tracks").get<std::vector<t_size>>();

		auto f = std::bind(copy_playlist_tracks, to_index, from_index, std::cref(track_indexes), std::ref(mtx), std::ref(cv), std::ref(finished));
		fb2k::inMainThread(f);

		std::unique_lock<std::mutex> lck(mtx);
		cv.wait(lck, [&] { return finished; });
	}
	catch (const std::exception &err) {
		json e;
		e["error"] = err.what();
		res.set_content(e.dump(), "application/json");
		return;
	}

	json j;
	j["success"] = true;
	res.set_content(j.dump(), "application/json");
}

void webpl_server::post_copy_libtracks_to_playlist(const httplib::Request& req, httplib::Response& res) {
	try {
		if (req.body.length() == 0) {
			json e;
			e["error"] = "Missing request data";
			res.set_content(e.dump(), "application/json");
			return;
		}

		auto data = json::parse(req.body);
		if (!data.contains("playlist") || !data.contains("tracks")) {
			json e;
			e["error"] = "Malformed request; Missing 'playlist' or 'tracks' keys";
			res.set_content(e.dump(), "application/json");
			return;
		}

		auto tracks = data["tracks"];
		if (!tracks.is_array()) {
			json e;
			e["error"] = "Malformed request; 'tracks' key does not appear to be an array";
			res.set_content(e.dump(), "application/json");
			return;
		}

		std::mutex mtx;
		std::condition_variable cv;
		bool finished = false;
		auto pl_index = data.at("playlist").get<t_size>();
		std::vector<LibraryTrack> libtracks;

		for (const auto &ele : tracks) {
			if (ele.contains("path")) {
				LibraryTrack lt;

				ele.at("path").get_to(lt.path);
				if (ele.contains("sub_index"))
					ele.at("sub_index").get_to(lt.sub_index);

				libtracks.push_back(lt);
			}
		}

		auto f = std::bind(copy_libtracks_to_playlist, pl_index, std::cref(libtracks), std::ref(mtx), std::ref(cv), std::ref(finished));
		fb2k::inMainThread(f);

		std::unique_lock<std::mutex> lck(mtx);
		cv.wait(lck, [&] { return finished; });
	}
	catch (const std::exception &err) {
		json e;
		e["error"] = err.what();
		res.set_content(e.dump(), "application/json");
		return;
	}

	json j;
	j["success"] = true;
	res.set_content(j.dump(), "application/json");
}

void webpl_server::post_remove_duplicates(const httplib::Request& req, httplib::Response& res) {
	try {
		if (req.body.length() == 0) {
			json e;
			e["error"] = "Missing request data";
			res.set_content(e.dump(), "application/json");
			return;
		}

		auto data = json::parse(req.body);
		if (!data.contains("playlist")) {
			json e;
			e["error"] = "Malformed request; Missing 'playlist' key";
			res.set_content(e.dump(), "application/json");
			return;
		}

		std::mutex mtx;
		std::condition_variable cv;
		bool finished = false;
		auto index = data.at("playlist").get<t_size>();

		auto f = std::bind(remove_duplicates, index, std::ref(mtx), std::ref(cv), std::ref(finished));
		fb2k::inMainThread(f);

		std::unique_lock<std::mutex> lck(mtx);
		cv.wait(lck, [&] { return finished; });
	}
	catch (const std::exception &err) {
		json e;
		e["error"] = err.what();
		res.set_content(e.dump(), "application/json");
		return;
	}

	json j;
	j["success"] = true;
	res.set_content(j.dump(), "application/json");
}

void webpl_server::post_remove_playlist_items(const httplib::Request& req, httplib::Response& res) {
	try {
		if (req.body.length() == 0) {
			json e;
			e["error"] = "Missing request data";
			res.set_content(e.dump(), "application/json");
			return;
		}

		auto data = json::parse(req.body);
		if (!data.contains("playlist") || !data.contains("tracks")) {
			json e;
			e["error"] = "Malformed request; Missing 'playlist' or 'tracks' key";
			res.set_content(e.dump(), "application/json");
			return;
		}

		std::mutex mtx;
		std::condition_variable cv;
		bool finished = false;
		auto index = data.at("playlist").get<t_size>();
		auto tracks = data.at("tracks").get<std::vector<t_size>>();

		auto f = std::bind(remove_playlist_items, index, std::cref(tracks), std::ref(mtx), std::ref(cv), std::ref(finished));
		fb2k::inMainThread(f);

		std::unique_lock<std::mutex> lck(mtx);
		cv.wait(lck, [&] { return finished; });
	}
	catch (const std::exception &err) {
		json e;
		e["error"] = err.what();
		res.set_content(e.dump(), "application/json");
		return;
	}

	json j;
	j["success"] = true;
	res.set_content(j.dump(), "application/json");
}

void webpl_server::post_move_playlist_items(const httplib::Request& req, httplib::Response& res) {
	try {
		if (req.body.length() == 0) {
			json e;
			e["error"] = "Missing request data";
			res.set_content(e.dump(), "application/json");
			return;
		}

		auto data = json::parse(req.body);
		if (!data.contains("playlist") || !data.contains("tracks") || !data.contains("move_to")) {
			json e;
			e["error"] = "Malformed request; Missing 'playlist', 'tracks', or 'delta' keys";
			res.set_content(e.dump(), "application/json");
			return;
		}

		std::mutex mtx;
		std::condition_variable cv;
		bool finished = false;
		auto index = data.at("playlist").get<t_size>();
		auto tracks = data.at("tracks").get<std::vector<t_size>>();
		auto move_to = data.at("move_to").get<t_size>();

		auto f = std::bind(move_playlist_items, index, std::ref(tracks), move_to, std::ref(mtx), std::ref(cv), std::ref(finished));
		fb2k::inMainThread(f);

		std::unique_lock<std::mutex> lck(mtx);
		cv.wait(lck, [&] { return finished; });
	}
	catch (const std::exception &err) {
		json e;
		e["error"] = err.what();
		res.set_content(e.dump(), "application/json");
		return;
	}

	json j;
	j["success"] = true;
	res.set_content(j.dump(), "application/json");
}

void webpl_server::start() {
	m_server.listen("0.0.0.0", 8080);
}