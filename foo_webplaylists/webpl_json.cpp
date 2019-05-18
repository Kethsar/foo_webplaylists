#include "stdafx.h"
#include "webpl_structs.h"

void to_json(json &j, const Track &t) {
	j = json{ { "artist", t.artist },{ "title", t.title },{ "length", t.length } };
}

void from_json(const json &j, Track &t) {
	j.at("artist").get_to(t.artist);
	j.at("title").get_to(t.title);
	j.at("length").get_to(t.length);
}

void to_json(json &j, const LibraryTrack &t) {
	j = json{ { "sub_index", t.sub_index },{ "path", t.path },{ "track", json(t.track) } };
}

void from_json(const json &j, LibraryTrack &t) {
	j.at("sub_index").get_to(t.sub_index);
	j.at("path").get_to(t.path);
	j.at("track").get_to(t.track);
}

void to_json(json &j, const PlaylistTrack &t) {
	j = json{ { "index", t.index },{ "track", json(t.track) },{ "seperator", json(t.seperator)} };
}

void from_json(const json &j, PlaylistTrack &t) {
	j.at("index").get_to(t.index);
	j.at("track").get_to(t.track);
	j.at("seperator").get_to(t.seperator);
}

void to_json(json &j, const Playlist &t) {
	j = json{ { "playlist_index", t.playlist_index },{"playlist_name", t.playlist_name},{ "tracks", json(t.tracks) } };
}

void from_json(const json &j, Playlist &t) {
	j.at("playlist_index").get_to(t.playlist_index);
	j.at("playlist_name").get_to(t.playlist_name);
	j.at("tracks").get_to(t.tracks);
}