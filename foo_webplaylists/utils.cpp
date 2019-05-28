#include "stdafx.h"
#include "utils.h"

void get_lib_query_items(const char* query, std::vector<LibraryTrack> &libtracks, titleformat_object_wrapper &fmt_artist,
	titleformat_object_wrapper &fmt_title, titleformat_object_wrapper &fmt_length, std::mutex &mtx,
	std::condition_variable &cv, bool &finished) {
	std::unique_lock<std::mutex> lck(mtx);

	metadb_handle_list handles;
	webpl_libman_callback cb(handles);
	static_api_ptr_t<library_manager> lm;

	lm->enum_items(cb);

	auto filter = static_api_ptr_t<search_filter_manager>()->create(query);
	bool* matches = new bool[handles.get_count()];

	filter->test_multi(handles, matches);

	pfc::string8 buf;
	for (t_size i = 0, c = handles.get_count(); i < c; i++) {
		if (!matches[i]) continue;

		LibraryTrack lt;
		auto meta = handles[i];

		meta->format_title(nullptr, buf, fmt_artist, nullptr);
		lt.track.artist = buf.get_ptr();

		meta->format_title(nullptr, buf, fmt_title, nullptr);
		lt.track.title = buf.get_ptr();

		meta->format_title(nullptr, buf, fmt_length, nullptr);
		lt.track.length = buf.get_ptr();

		lt.sub_index = meta->get_subsong_index();
		lt.path = meta->get_path();

		libtracks.push_back(lt);
	}

	delete[] matches;

	finished = true;
	cv.notify_one();
}

void remove_duplicates(const t_size pl_index, std::mutex &mtx, std::condition_variable &cv, bool &finished) {
	std::unique_lock<std::mutex> lck(mtx);

	static_api_ptr_t<playlist_manager_v4> pm;
	metadb_handle_list metalist;
	pm->playlist_get_all_items(pl_index, metalist);
	pfc::bit_array_bittable table(metalist.get_count());

	for (t_size i = 0, c = metalist.get_count(); i < (c - 1); i++) {
		if (table.get(i)) continue;

		auto meta = metalist[i];
		for (t_size j = (i + 1); j < c; j++) {
			if (meta == metalist[j]) {
				table.set(j, true);
			}
		}
	}

	pm->playlist_remove_items(pl_index, table);

	finished = true;
	cv.notify_one();
}

void copy_playlist_tracks(const t_size to_index, const t_size from_index, const std::vector<t_size> &track_indexes,
	std::mutex &mtx, std::condition_variable &cv, bool &finished) {
	std::unique_lock<std::mutex> lck(mtx);

	static_api_ptr_t<playlist_manager_v4> pm;
	metadb_handle_list metalist;
	pfc::bit_array_bittable table(pm->playlist_get_item_count(from_index));

	for (const auto i : track_indexes) {
		table.set(i, true);
	}

	pm->playlist_get_items(from_index, metalist, table);
	pm->playlist_add_items(to_index, metalist, pfc::bit_array_true());

	finished = true;
	cv.notify_one();
}

void get_playlists(std::vector<Playlist> &playlists, std::mutex &mtx, std::condition_variable &cv, bool &finished) {
	std::unique_lock<std::mutex> lck(mtx);

	pfc::string8 buf;
	static_api_ptr_t<playlist_manager_v4> pm;
	auto pl_count = pm->get_playlist_count();

	for (t_size i = 0; i < pl_count; i++) {
		Playlist pl;
		pl.playlist_index = i;

		pm->playlist_get_name(i, buf);
		pl.playlist_name = buf.get_ptr();

		playlists.push_back(pl);
	}

	finished = true;
	cv.notify_one();
}

void get_playlist_data(const std::string &pl_index_str, Playlist &playlist,	titleformat_object_wrapper &fmt_artist,
	titleformat_object_wrapper &fmt_title, titleformat_object_wrapper &fmt_length, titleformat_object_wrapper &fmt_seperator,
	std::mutex &mtx, std::condition_variable &cv, bool &finished) {
	std::unique_lock<std::mutex> lck(mtx);

	pfc::string8 buf;
	static_api_ptr_t<playlist_manager_v4> pm;
	t_size index;

	if (pl_index_str.length() == 0)
		index = pm->get_active_playlist();
	else
		index = std::stoi(pl_index_str);

	auto item_count = pm->playlist_get_item_count(index);

	playlist.playlist_index = index;
	playlist.tracks.reserve(item_count);

	pm->playlist_get_name(index, buf);
	playlist.playlist_name = buf.get_ptr();

	if (item_count < 10000) {
		for (t_size i = 0; i < item_count; i++) {
			PlaylistTrack pt;
			pt.index = i;

			pm->playlist_item_format_title(index, i, nullptr, buf,
				fmt_artist, nullptr, playback_control::display_level_none);
			pt.track.artist = buf.get_ptr();

			pm->playlist_item_format_title(index, i, nullptr, buf,
				fmt_title, nullptr, playback_control::display_level_none);
			pt.track.title = buf.get_ptr();

			pm->playlist_item_format_title(index, i, nullptr, buf,
				fmt_length, nullptr, playback_control::display_level_none);
			pt.track.length = buf.get_ptr();

			pm->playlist_item_format_title(index, i, nullptr, buf,
				fmt_seperator, nullptr, playback_control::display_level_none);
			pt.seperator = buf.get_ptr();

			playlist.tracks.push_back(pt);
		}
	}

	finished = true;
	cv.notify_one();
}

void move_playlist_items(const t_size pl_index, std::vector<t_size> &track_indexes, const t_size move_to, std::mutex &mtx,
	std::condition_variable &cv, bool &finished) {
	std::unique_lock<std::mutex> lck(mtx);

	static_api_ptr_t<playlist_manager_v4> pm;
	auto pl_count = pm->playlist_get_item_count(pl_index);
	std::vector<t_size> order;
	order.reserve(pl_count);
	std::sort(track_indexes.begin(), track_indexes.end());

	t_size index = 0;
	for (; index < move_to; index++) {
		if (!std::binary_search(track_indexes.begin(), track_indexes.end(), index))
			order.push_back(index);
	}

	for (const auto i : track_indexes) {
		order.push_back(i);
	}

	for (; index < pl_count; index++) {
		if (!std::binary_search(track_indexes.begin(), track_indexes.end(), index))
			order.push_back(index);
	}

	pm->playlist_reorder_items(pl_index, order.data(), pl_count);

	finished = true;
	cv.notify_one();
}

void remove_playlist_items(const t_size pl_index, const std::vector<t_size> &track_indexes, std::mutex &mtx,
	std::condition_variable &cv, bool &finished) {
	std::unique_lock<std::mutex> lck(mtx);

	static_api_ptr_t<playlist_manager_v4> pm;
	pfc::bit_array_bittable table(pm->playlist_get_item_count(pl_index));

	for (const auto i : track_indexes) {
		table.set(i, true);
	}

	pm->playlist_remove_items(pl_index, table);

	finished = true;
	cv.notify_one();
}

void copy_libtracks_to_playlist(const t_size pl_index, const std::vector<LibraryTrack> &libtracks, std::mutex &mtx,
	std::condition_variable &cv, bool &finished) {
	std::unique_lock<std::mutex> lck(mtx);

	static_api_ptr_t<playlist_manager_v4> pm;
	static_api_ptr_t<metadb> mdb;
	metadb_handle_list metalist;

	// Almost certain this should be using playlist_incoming_item_filter_v2::process_locations_async
	// But that has no option for the sub index
	for (const auto &lt : libtracks) {
		make_playable_location ploc(lt.path.c_str(), lt.sub_index);
		auto metah = mdb->handle_create(ploc);
		metalist.add_item(metah);
	}

	// Possibly make this playlist_insert_items and add an at_index param
	pm->playlist_add_items(pl_index, metalist, pfc::bit_array_true());

	finished = true;
	cv.notify_one();
}