#pragma once

#include <json.hpp>

namespace save_settings
{
	extern nlohmann::json _settings_json;
	bool save_vision_to_file(const std::string& filename);
	bool load_vision_from_file(const std::string& filename);
}
