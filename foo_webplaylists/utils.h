#pragma once
#include "stdafx.h"
#include <mutex>
#include <condition_variable>
#include "webpl_structs.h"

void get_lib_query_items(const char* query, std::vector<LibraryTrack> &libtracks, titleformat_object_wrapper &fmt_artist,
	titleformat_object_wrapper &fmt_title, titleformat_object_wrapper &fmt_length, std::mutex &mtx,
	std::condition_variable &cv, bool &finished);
void remove_duplicates(const t_size pl_index, std::mutex &mtx, std::condition_variable &cv, bool &finished);
void copy_playlist_tracks(const t_size to_index, const t_size from_index, const std::vector<t_size> &track_indexes,
	std::mutex &mtx, std::condition_variable &cv, bool &finished);
void get_playlists(std::vector<Playlist> &playlists, std::mutex &mtx, std::condition_variable &cv, bool &finished);
void get_playlist_data(const std::string &pl_index_str, Playlist &playlist, titleformat_object_wrapper &fmt_artist,
	titleformat_object_wrapper &fmt_title, titleformat_object_wrapper &fmt_length, titleformat_object_wrapper &fmt_seperator,
	std::mutex &mtx, std::condition_variable &cv, bool &finished);
void move_playlist_items(const t_size pl_index, std::vector<t_size> &track_indexes, const t_size move_to, std::mutex &mtx,
	std::condition_variable &cv, bool &finished);
void remove_playlist_items(const t_size pl_index, const std::vector<t_size> &track_indexes, std::mutex &mtx,
	std::condition_variable &cv, bool &finished);
void copy_libtracks_to_playlist(const t_size pl_index, const std::vector<LibraryTrack> &libtracks, std::mutex &mtx,
	std::condition_variable &cv, bool &finished);

class webpl_libman_callback : public library_manager::enum_callback {
public:
	webpl_libman_callback(metadb_handle_list_ref list) : metadb_handles(list) {
		metadb_handles.remove_all();
	}

	bool on_item(const metadb_handle_ptr& p_item) {
		metadb_handles.add_item(p_item);
		return true;
	}

	const metadb_handle_list get_metadb_handles() {
		return metadb_handles;
	}

private:
	metadb_handle_list_ref metadb_handles;
};