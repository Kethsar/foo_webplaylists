#pragma once

#include "stdafx.h"
#include <string>
#include <json.hpp>
#include <vector>

using json = nlohmann::json;

struct Track {
	std::string artist;
	std::string title;
	std::string length;
};

void to_json(json &j, const Track &t);
void from_json(const json &j, Track &t);

struct LibraryTrack {
	Track track;
	std::string path;
	t_size sub_index;
};

void to_json(json &j, const LibraryTrack &t);
void from_json(const json &j, LibraryTrack &t);

struct PlaylistTrack {
	Track track;
	t_size index;
	std::string seperator;
};

void to_json(json &j, const PlaylistTrack &t);
void from_json(const json &j, PlaylistTrack &t);

struct Playlist {
	t_size playlist_index;
	std::string playlist_name;
	std::vector<PlaylistTrack> tracks;
	Playlist() { playlist_index = -1; }
};

void to_json(json &j, const Playlist &t);
void from_json(const json &j, Playlist &t);