#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include <json.hpp>

namespace save_settings
{
	nlohmann::json _settings_json;
	bool save_vision_to_file(const std::string& filename) 
	{
		std::ofstream file(filename);
		std::filesystem::path filePath(filename);
		if (filePath.has_parent_path()) {
			std::filesystem::create_directories(filePath.parent_path());
		}

		if (file.is_open()) 
		{
			file << _settings_json.dump(4);
			file.close();
			return true;
		}
		else
		{
			return false;
		}
	}

	bool load_vision_from_file(const std::string& filename)
	{
		std::ifstream file(filename);
		if (file.is_open()) 
		{
			file >> _settings_json;
			file.close();
			return true;
		}
		else
		{
			return false;
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
		
		}
	};
}

REGISTER_COMPONENT(save_settings::component)
