#include <std_include.hpp>

#ifdef DEBUG
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "game/dvars.hpp"

#include "gui.hpp"

#include "dvars.hpp"
#include "scheduler.hpp"
#include "console.hpp"
#include "utils/string.hpp"

#include <utils/string.hpp>
#include <utils/hook.hpp>

#include "../game/scripting/execution.hpp"
#include "../game/scripting/entity.hpp"

#include "dvars.hpp"

#pragma region macros

#define READ_BOOL(__var__, __dvar__) \
		if (__dvar__ != nullptr) __var__ = &__dvar__->current.enabled; \

#define READ_FLOAT(__var__, __dvar__) \
		if (__dvar__ != nullptr) __var__ = __dvar__->current.value; \

#define READ_VEC3(__var__, __dvar__) \
		if (__dvar__ != nullptr) \
		{ \
			for (auto i = 0; i < 3; i++) \
				__var__[i] = __dvar__->current.color[i]; \
		} \

#define WRITE_BOOL(__name__, __var__) \
		if (ImGui::Checkbox(__name__, &__var__)) _write_fog(); \

#define WRITE_FLOAT(__name__, __var__, __step__, __dvar__) \
		if (__dvar__ != nullptr && ImGui::DragFloat(__name__, &__var__, __step__, __dvar__->domain.value.min, __dvar__->domain.value.max)) _write_fog(); \

#define WRITE_VEC3(__name__, __var__, __step__, __dvar__) \
		if (__dvar__ != nullptr && ImGui::DragFloat3(__name__, __var__, __step__, __dvar__->domain.value.min, __dvar__->domain.value.max)) _write_fog(); \

#define WRITE_COLOUR(__name__, __var__) \
		if (ImGui::ColorEdit3(__name__, __var__)) _write_fog(); \

#define ATMOS_WRITE_BOOL(__name__, __var__) \
		if (ImGui::Checkbox(__name__, &__var__)) build_atmos_fog_buffer(); \

#define ATMOS_WRITE_FLOAT(__name__, __var__, __step__, __dvar__) \
		if (__dvar__ != nullptr && ImGui::DragFloat(__name__, &__var__, __step__, __dvar__->domain.value.min, __dvar__->domain.value.max)) build_atmos_fog_buffer(); \

#define ATMOS_WRITE_VEC3(__name__, __var__, __step__, __dvar__) \
		if (__dvar__ != nullptr && ImGui::DragFloat3(__name__, __var__, __step__, __dvar__->domain.value.min, __dvar__->domain.value.max)) build_atmos_fog_buffer(); \

#define ATMOS_WRITE_COLOUR(__name__, __var__) \
		if (ImGui::ColorEdit3(__name__, __var__)) build_atmos_fog_buffer(); \

#define M_LN2 0.69314718f

#pragma endregion

namespace gui::fog_editor
{
	bool parse_art_file = true;

	utils::hook::detour cg_parse_client_visionset_triggers;
	void cg_parse_client_visionset_triggers_stub(const char* buffer)
	{
		return cg_parse_client_visionset_triggers.invoke<void>(parse_art_file ? buffer : nullptr);
	}


	const char* _configs_string = "atmosfog %s";
	const char* va_call(const char* fmt, ...)
	{
		static thread_local utils::string::va_provider<8, 256> provider;

		va_list ap;
		va_start(ap, _configs_string);

		const char* result = provider.get(_configs_string, ap);

		va_end(ap);
		return result;
	}

	bool _trigger = false;
	float _g_fogStartDistReadOnly = 0.f;
	float _g_fogHalfDistReadOnly = 0.f;
	float _g_fogMaxOpacityReadOnly = 0.f;
	bool _g_heightFogEnabledReadOnly = false;
	float _g_heightFogHalfPlaneDistanceReadOnly = 0.f;
	float _g_heightFogBaseHeightReadOnly = 0.f;
	float _g_fogColorReadOnly[3] = { 0.f, 0.f, 0.f };
	float _g_fogColorIntensityReadOnly = 0.f;

	bool _g_sunFogEnabledReadOnly = false;
	float _g_sunFogColorReadOnly[4] = { 1.f, 0.f, 0.f };
	float _g_sunFogScaleReadOnly = 0.f;
	float _g_sunFogDirReadOnly[2] = {};
	float _g_sunFogBeginFadeAngleReadOnly = 0.f;
	float _g_sunFogEndFadeAngleReadOnly = 0.f;
	float _g_sunFogColorIntensityReadOnly = 1.f;
	float _r_sky_fog_min_angle = 1.f;
	float _r_sky_fog_max_angle = 1.f;
	float _r_sky_fog_intensity = 1.f;

	char _fog_vision_id[100];

	void _server_thread() // gsc calls
	{
		//if (_trigger)
		//{
			//scripting::entity level = scripting::entity(scripting::entity({ static_cast<uint16_t>(*::game::levelEntityId), 0 }));
			//scripting::entity _visionset_ent = scripting::call_script_function(level, "scripts/mp/main", "setup_fog", { (strcmp(_fog_vision_id, "") == 0) ? game::Dvar_FindVar("mapname")->current.string : _fog_vision_id }).as<scripting::entity>();

			//const char* pos = scripting::get_function_pos(/*"maps/mp/_art"*/"scripts/mp/main", "setup_fog");
			//game::VariableValue _execent_ref = scripting::exec_ent_thread(level, pos, { (strcmp(_fog_vision_id, "") == 0) ? game::Dvar_FindVar("mapname")->current.string : _fog_vision_id }).get_raw();
			//game::scr_entref_t _visionset_ref = { static_cast<uint16_t>(game::Scr_GetSelf(_execent_ref.u.entityOffset)), 0 };
			//scripting::entity _visionset_ent = { _visionset_ref };

			//scripting::set_entity_field(_visionset_ent, "startdist", _g_fogStartDistReadOnly);
			//scripting::set_entity_field(_visionset_ent, "halfwaydist", _g_fogHalfDistReadOnly);
			//scripting::set_entity_field(_visionset_ent, "red", _g_fogColorReadOnly[0]);
			//scripting::set_entity_field(_visionset_ent, "green", _g_fogColorReadOnly[1]);
			//scripting::set_entity_field(_visionset_ent, "blue", _g_fogColorReadOnly[3]);
			//scripting::set_entity_field(_visionset_ent, "hdrcolorintensity", _g_fogColorIntensityReadOnly);
			//scripting::set_entity_field(_visionset_ent, "maxopacity", _g_fogMaxOpacityReadOnly);
			//scripting::set_entity_field(_visionset_ent, "transitiontime", 0.0);
			//scripting::set_entity_field(_visionset_ent, "sunfogenabled", _g_sunFogEnabledReadOnly);
			//scripting::set_entity_field(_visionset_ent, "sunred", _g_sunFogColorReadOnly[0]);
			//scripting::set_entity_field(_visionset_ent, "sungreen", _g_sunFogColorReadOnly[1]);
			//scripting::set_entity_field(_visionset_ent, "sunblue", _g_sunFogColorReadOnly[3]);
			//scripting::set_entity_field(_visionset_ent, "hdrsuncolorintensity", _g_sunFogColorIntensityReadOnly);
			//scripting::set_entity_field(_visionset_ent, "sundir", _g_sunFogDirReadOnly);
			//scripting::set_entity_field(_visionset_ent, "sunbeginfadeangle", _g_sunFogBeginFadeAngleReadOnly);
			//scripting::set_entity_field(_visionset_ent, "sunendfadeangle", _g_sunFogEndFadeAngleReadOnly);
			//scripting::set_entity_field(_visionset_ent, "normalfogscale", _g_sunFogScaleReadOnly);
			//scripting::set_entity_field(_visionset_ent, "skyfogintensity", _r_sky_fog_intensity);
			//scripting::set_entity_field(_visionset_ent, "skyfogminangle", _r_sky_fog_min_angle);
			//scripting::set_entity_field(_visionset_ent, "skyfogmaxangle", _r_sky_fog_max_angle);


		//	//const char* _pos = scripting::get_function_pos("common_scripts/utility", "set_fog_to_ent_values_dfog");
		//	//scripting::exec_ent_thread(level, _pos, { (strcmp(_fog_vision_id, "") == 0) ? game::Dvar_FindVar("mapname")->current.string : _fog_vision_id, 0 });

		//	game::Scr_AddFloat(_r_sky_fog_max_angle);
		//	game::Scr_AddFloat(_r_sky_fog_min_angle);
		//	game::Scr_AddFloat(_r_sky_fog_intensity);
		//	game::Scr_AddFloat(_g_sunFogColorIntensityReadOnly);
		//	game::Scr_AddFloat(_g_sunFogEndFadeAngleReadOnly);
		//	game::Scr_AddFloat(_g_sunFogBeginFadeAngleReadOnly);
		//	game::Scr_AddVector(_g_sunFogDirReadOnly);
		//	game::Scr_AddFloat(_g_sunFogScaleReadOnly);
		//	game::Scr_AddVector(_g_sunFogColorReadOnly);
		//	game::Scr_AddBool(_g_sunFogEnabledReadOnly);
		//	game::Scr_AddFloat(_g_fogColorIntensityReadOnly);
		//	game::Scr_AddVector(_g_fogColorReadOnly);
		//	game::Scr_AddFloat(_g_heightFogBaseHeightReadOnly);
		//	game::Scr_AddFloat(_g_heightFogHalfPlaneDistanceReadOnly);
		//	game::Scr_AddBool(_g_heightFogEnabledReadOnly);
		//	game::Scr_AddFloat(_g_fogMaxOpacityReadOnly);
		//	game::Scr_AddFloat(_g_fogHalfDistReadOnly);
		//	game::Scr_AddFloat(_g_fogStartDistReadOnly);
		//	//game::Scr_SetFog(0, "SetExpFogExt", 0, 0, 0);
		//	game::Scr_ExecThread(0x182, 18); // 0x4ECDE0
		//	game::Scr_ClearOutParams();

		//	console::info("-c parsed exponential fog");
		//	_trigger = false;
		//}
	}

	bool _r_fog = true;

	bool atmos_height_fog = false;
	float atmos_height_base = 0.0f;
	float atmos_height_half_dist = 100000.0f;
	float atmos_start_dist = 0.0f;
	float atmos_half_dist = 5000.0f;
	float atmos_sun_fog_colour[3] = {};
	float atmos_haze_colour[3] = {};
	float atmos_haze_strength = 0.5f;
	float atmos_haze_spread = 0.25f;
	float atmos_extinction_strength = 1.0f;
	float atmos_inscatter_strength = 0.0f;
	float atmos_distance_scale = 1.0f;
	float atmos_sky_distance = 100000.0f;
	bool atmos_sky_angular_falloff = false;
	float atmos_sky_falloff_start = 0.0f;
	float atmos_sky_falloff_range = 90.0f;
	float atmos_sun_dir[3] = {};

	void build_fog_buffer()
	{
		_configs_string = "setexpfog %s";
		auto buffer = utils::string::va(
			"%g %g %g %g %g %g %g %.0f 1 %g %g %g %g %g %g %g %g %g %g %g %g %g",
			_g_fogStartDistReadOnly,
			M_LN2 / _g_fogHalfDistReadOnly,
			_g_fogColorReadOnly[0] / 255,
			_g_fogColorReadOnly[1] / 255,
			_g_fogColorReadOnly[2] / 255,
			_g_fogColorIntensityReadOnly,
			_g_fogMaxOpacityReadOnly,
			0.0f, // transition time
			_g_sunFogColorReadOnly[0] / 255,
			_g_sunFogColorReadOnly[1] / 255,
			_g_sunFogColorReadOnly[2] / 255,
			_g_sunFogColorIntensityReadOnly,
			_g_sunFogDirReadOnly,
			_g_sunFogBeginFadeAngleReadOnly,
			_g_sunFogEndFadeAngleReadOnly,
			_r_sky_fog_intensity,
			_r_sky_fog_min_angle,
			_r_sky_fog_max_angle,
			_g_heightFogEnabledReadOnly,
			_g_heightFogBaseHeightReadOnly,
			_g_heightFogHalfPlaneDistanceReadOnly);

			game::G_SetFog(buffer, 0, 1);
	}

	void build_atmos_fog_buffer()
	{
		_configs_string = "atmosfog %s";
		auto buffer = utils::string::va(
			"%g %g %.0f %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
			atmos_start_dist,
			M_LN2 / atmos_half_dist,
			0.0f, // transition time
			atmos_sun_fog_colour[0],
			atmos_sun_fog_colour[1],
			atmos_sun_fog_colour[2],
			atmos_haze_colour[0],
			atmos_haze_colour[1],
			atmos_haze_colour[2],
			atmos_haze_strength,
			atmos_haze_spread,
			atmos_extinction_strength,
			atmos_inscatter_strength,
			atmos_distance_scale,
			atmos_sky_distance,
			atmos_sky_angular_falloff ? atmos_sky_falloff_start : -90.0f,
			atmos_sky_angular_falloff ? atmos_sky_falloff_range : 9000.0f,
			atmos_sun_dir[0],
			atmos_sun_dir[1],
			atmos_sun_dir[2],
			atmos_height_fog ? atmos_height_base : 0.0f,
			M_LN2 / (atmos_height_fog ? atmos_height_half_dist : 100000.0f));

			game::G_SetFog(buffer, 0, 1);
	}

	void render_window()
	{
#pragma region dvars
		static auto* r_fog = game::Dvar_FindVar("r_fog");

		static auto* g_sunFogBeginFadeAngleReadOnly = game::Dvar_FindVar("g_sunFogBeginFadeAngleReadOnly");
		static auto* g_heightFogEnabledReadOnly = game::Dvar_FindVar("g_heightFogEnabledReadOnly");
		static auto* g_fogColorReadOnly = game::Dvar_FindVar("g_fogColorReadOnly");
		static auto* g_fogMaxOpacityReadOnly = game::Dvar_FindVar("g_fogMaxOpacityReadOnly");
		static auto* g_fogStartDistReadOnly = game::Dvar_FindVar("g_fogStartDistReadOnly");
		static auto* g_sunFogEndFadeAngleReadOnly = game::Dvar_FindVar("g_sunFogEndFadeAngleReadOnly");
		static auto* g_sunFogColorReadOnly = game::Dvar_FindVar("g_sunFogColorReadOnly");
		static auto* g_fogColorIntensityReadOnly = game::Dvar_FindVar("g_fogColorIntensityReadOnly");
		static auto* g_heightFogBaseHeightReadOnly = game::Dvar_FindVar("g_heightFogBaseHeightReadOnly");
		static auto* g_fogHalfDistReadOnly = game::Dvar_FindVar("g_fogHalfDistReadOnly");
		static auto* g_heightFogHalfPlaneDistanceReadOnly = game::Dvar_FindVar("g_heightFogHalfPlaneDistanceReadOnly");
		static auto* g_sunFogColorIntensityReadOnly = game::Dvar_FindVar("g_sunFogColorIntensityReadOnly");
		static auto* g_sunFogDirReadOnly = game::Dvar_FindVar("g_sunFogDirReadOnly");
		static auto* g_sunFogEnabledReadOnly = game::Dvar_FindVar("g_sunFogEnabledReadOnly");
		static auto* g_sunFogScaleReadOnly = game::Dvar_FindVar("g_sunFogScaleReadOnly");
		static auto* r_sky_fog_min_angle = game::Dvar_FindVar("r_sky_fog_min_angle");
		static auto* r_sky_fog_max_angle = game::Dvar_FindVar("r_sky_fog_max_angle");

		static auto* g_atmosFogStartDistanceReadOnly = game::Dvar_FindVar("g_atmosFogStartDistanceReadOnly");
		static auto* g_atmosFogHalfPlaneDistanceReadOnly = game::Dvar_FindVar("g_atmosFogHalfPlaneDistanceReadOnly");
		static auto* g_atmosFogSunFogColorReadOnly = game::Dvar_FindVar("g_atmosFogSunFogColorReadOnly");
		static auto* g_atmosFogHazeColorReadOnly = game::Dvar_FindVar("g_atmosFogHazeColorReadOnly");
		static auto* g_atmosFogHazeStrengthReadOnly = game::Dvar_FindVar("g_atmosFogHazeStrengthReadOnly");
		static auto* g_atmosFogHazeSpreadReadOnly = game::Dvar_FindVar("g_atmosFogHazeSpreadReadOnly");
		static auto* g_atmosFogExtinctionStrengthReadOnly = game::Dvar_FindVar("g_atmosFogExtinctionStrengthReadOnly");
		static auto* g_atmosFogInScatterStrengthReadOnly = game::Dvar_FindVar("g_atmosFogInScatterStrengthReadOnly");
		static auto* g_atmosFogDistanceScaleReadOnly = game::Dvar_FindVar("g_atmosFogDistanceScaleReadOnly");
		static auto* g_atmosFogSkyDistanceReadOnly = game::Dvar_FindVar("g_atmosFogSkyDistanceReadOnly");
		static auto* g_atmosFogSunDirectionReadOnly = game::Dvar_FindVar("g_atmosFogSunDirectionReadOnly");
		static auto* g_atmosFogHeightFogEnabledReadOnly = game::Dvar_FindVar("g_atmosFogHeightFogEnabledReadOnly");
		static auto* g_atmosFogSkyAngularFalloffEnabledReadOnly = game::Dvar_FindVar("g_atmosFogSkyAngularFalloffEnabledReadOnly");
		static auto* g_atmosFogSkyFalloffStartAngleReadOnly = game::Dvar_FindVar("g_atmosFogSkyFalloffStartAngleReadOnly");
		static auto* g_atmosFogSkyFalloffAngleRangeReadOnly = game::Dvar_FindVar("g_atmosFogSkyFalloffAngleRangeReadOnly");
		static auto* g_atmosFogHeightFogBaseHeightReadOnly = game::Dvar_FindVar("g_atmosFogHeightFogBaseHeightReadOnly");
		static auto* g_atmosFogHeightFogHalfPlaneDistanceReadOnly = game::Dvar_FindVar("g_atmosFogHeightFogHalfPlaneDistanceReadOnly");
#pragma endregion

		ImGui::SetNextWindowSizeConstraints(ImVec2(500, 500), ImVec2(1000, 1000));
		ImGui::Begin("Fog Editor");

		static char visionset_out[100] = { 0 };
		if (ImGui::InputText("Fog Vision Set", visionset_out, 100))
		{
			strcpy(_fog_vision_id, visionset_out);
		}

		if (ImGui::Button("read dvars"))
		{
			READ_BOOL(atmos_height_fog, g_atmosFogHeightFogEnabledReadOnly);
			READ_FLOAT(atmos_height_base, g_atmosFogHeightFogBaseHeightReadOnly);
			READ_FLOAT(atmos_height_half_dist, g_atmosFogHeightFogHalfPlaneDistanceReadOnly);
			READ_FLOAT(atmos_start_dist, g_atmosFogStartDistanceReadOnly);
			READ_FLOAT(atmos_half_dist, g_atmosFogHalfPlaneDistanceReadOnly);
			READ_VEC3(atmos_sun_fog_colour, g_atmosFogSunFogColorReadOnly);
			READ_VEC3(atmos_haze_colour, g_atmosFogHazeColorReadOnly);
			READ_FLOAT(atmos_haze_strength, g_atmosFogHazeStrengthReadOnly);
			READ_FLOAT(atmos_haze_spread, g_atmosFogHazeSpreadReadOnly);
			READ_FLOAT(atmos_extinction_strength, g_atmosFogExtinctionStrengthReadOnly);
			READ_FLOAT(atmos_inscatter_strength, g_atmosFogInScatterStrengthReadOnly);
			READ_FLOAT(atmos_distance_scale, g_atmosFogDistanceScaleReadOnly);
			READ_FLOAT(atmos_sky_distance, g_atmosFogSkyDistanceReadOnly);
			READ_FLOAT(atmos_sky_angular_falloff, g_atmosFogSkyAngularFalloffEnabledReadOnly);
			READ_FLOAT(atmos_sky_falloff_start, g_atmosFogSkyFalloffStartAngleReadOnly);
			READ_FLOAT(atmos_sky_falloff_range, g_atmosFogSkyFalloffAngleRangeReadOnly);
			READ_VEC3(atmos_sun_dir, g_atmosFogSunDirectionReadOnly);

			READ_FLOAT(_g_fogStartDistReadOnly, g_fogStartDistReadOnly);
			READ_FLOAT(_g_fogHalfDistReadOnly, g_fogHalfDistReadOnly);
			READ_FLOAT(_g_fogMaxOpacityReadOnly, g_fogMaxOpacityReadOnly);
			READ_BOOL(_g_heightFogEnabledReadOnly, g_heightFogEnabledReadOnly);
			READ_FLOAT(_g_heightFogHalfPlaneDistanceReadOnly, g_heightFogHalfPlaneDistanceReadOnly);
			READ_FLOAT(_g_heightFogBaseHeightReadOnly, g_heightFogBaseHeightReadOnly);
			READ_VEC3(_g_fogColorReadOnly, g_fogColorReadOnly);
			READ_FLOAT(_g_fogColorIntensityReadOnly, g_fogColorIntensityReadOnly);
			READ_BOOL(_g_sunFogEnabledReadOnly, g_sunFogEnabledReadOnly);
			READ_VEC3(_g_sunFogColorReadOnly, g_sunFogColorReadOnly);
			READ_FLOAT(_g_sunFogScaleReadOnly, g_sunFogScaleReadOnly);
			READ_VEC3(_g_sunFogDirReadOnly, g_sunFogDirReadOnly);
			READ_FLOAT(_g_sunFogBeginFadeAngleReadOnly, g_sunFogBeginFadeAngleReadOnly);
			READ_FLOAT(_g_sunFogEndFadeAngleReadOnly, g_sunFogEndFadeAngleReadOnly);
			READ_FLOAT(_g_sunFogColorIntensityReadOnly, g_sunFogColorIntensityReadOnly);
			READ_FLOAT(_r_sky_fog_min_angle, r_sky_fog_min_angle);
			READ_FLOAT(_r_sky_fog_max_angle, r_sky_fog_max_angle);
		}

		ImGui::Checkbox("parse fog file on map load", &parse_art_file);

		if (ImGui::Checkbox("show fog", &_r_fog))
		{
			dvars::override::set_bool("r_fog", _r_fog);
			dvars::dvar_set_bool(r_fog, _r_fog);
		}

		if (ImGui::CollapsingHeader("Exponential Fog"))
		{
			if(ImGui::DragFloat("start dist", &_g_fogStartDistReadOnly)) build_fog_buffer();
			if(ImGui::DragFloat("half dist", &_g_fogHalfDistReadOnly)) build_fog_buffer();
			if(ImGui::DragFloat3("fog color", _g_fogColorReadOnly)) build_fog_buffer();
			if(ImGui::DragFloat("fog color intensity", &_g_fogColorIntensityReadOnly)) build_fog_buffer();
			if(ImGui::DragFloat("fog max opacity", &_g_fogMaxOpacityReadOnly)) build_fog_buffer();
			
			if(ImGui::DragFloat3("sun fog color", _g_sunFogColorReadOnly)) build_fog_buffer();
			if(ImGui::DragFloat("sun fog color intensity", &_g_sunFogColorIntensityReadOnly)) build_fog_buffer();
			if(ImGui::DragFloat2("sun fog dir", _g_sunFogDirReadOnly)) build_fog_buffer();
			if(ImGui::DragFloat("sun fog begin fade angle", &_g_sunFogBeginFadeAngleReadOnly)) build_fog_buffer();
			if(ImGui::DragFloat("sun fog end fade angle", &_g_sunFogEndFadeAngleReadOnly)) build_fog_buffer();
			if(ImGui::DragFloat("sky fog intensity", &_r_sky_fog_intensity)) build_fog_buffer();
			if(ImGui::DragFloat("sky fog min angle", &_r_sky_fog_min_angle)) build_fog_buffer();
			if(ImGui::DragFloat("sky fog max angle", &_r_sky_fog_max_angle)) build_fog_buffer();
			
			if(ImGui::Checkbox("height fog enabled", &_g_heightFogEnabledReadOnly)) build_fog_buffer();
			if(ImGui::DragFloat("height fog base height", &_g_heightFogBaseHeightReadOnly)) build_fog_buffer();
			if(ImGui::DragFloat("height fog half plane distance", &_g_heightFogHalfPlaneDistanceReadOnly)) build_fog_buffer();
		}

		if (ImGui::CollapsingHeader("Atmospheric Fog"))
		{
			ATMOS_WRITE_BOOL("enable height fog", atmos_height_fog);
			ATMOS_WRITE_FLOAT("height base", atmos_height_base, 10.0f, g_atmosFogHeightFogBaseHeightReadOnly);
			ATMOS_WRITE_FLOAT("height half dist", atmos_height_half_dist, 10.0f, g_atmosFogHeightFogHalfPlaneDistanceReadOnly);
			ATMOS_WRITE_FLOAT("start dist", atmos_start_dist, 10.0f, g_atmosFogStartDistanceReadOnly);
			ATMOS_WRITE_FLOAT("half dist", atmos_half_dist, 10.0f, g_atmosFogHalfPlaneDistanceReadOnly);
			ATMOS_WRITE_COLOUR("sun fog color", atmos_sun_fog_colour);
			ATMOS_WRITE_COLOUR("haze color", atmos_haze_colour);
			ATMOS_WRITE_FLOAT("haze strength", atmos_haze_strength, 0.01f, g_atmosFogHazeStrengthReadOnly);
			ATMOS_WRITE_FLOAT("haze spread", atmos_haze_spread, 0.01f, g_atmosFogHazeSpreadReadOnly);
			ATMOS_WRITE_FLOAT("extinction strength", atmos_extinction_strength, 0.01f, g_atmosFogExtinctionStrengthReadOnly);
			ATMOS_WRITE_FLOAT("inscatter strength", atmos_inscatter_strength, 0.1f, g_atmosFogInScatterStrengthReadOnly);
			ATMOS_WRITE_FLOAT("distance scale", atmos_distance_scale, 0.1f, g_atmosFogDistanceScaleReadOnly);
			ATMOS_WRITE_FLOAT("sky distance", atmos_sky_distance, 100.0f, g_atmosFogSkyDistanceReadOnly);
			ATMOS_WRITE_BOOL("sky angular falloff", atmos_sky_angular_falloff);
			ATMOS_WRITE_FLOAT("sky falloff start", atmos_sky_falloff_start, 1.0f, g_atmosFogSkyFalloffStartAngleReadOnly);
			ATMOS_WRITE_FLOAT("sky falloff range", atmos_sky_falloff_range, 10.0f, g_atmosFogSkyFalloffAngleRangeReadOnly);
			ATMOS_WRITE_VEC3("sun direction", atmos_sun_dir, 0.01f, g_atmosFogSunDirectionReadOnly);
		}

		ImGui::End();
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			if (game::environment::is_dedi())
			{
				return;
			}
			
			cg_parse_client_visionset_triggers.create(0x10F920_b, &cg_parse_client_visionset_triggers_stub);
			utils::hook::call(0x413173_b, va_call);
			scheduler::loop(_server_thread, scheduler::pipeline::server);
		}
	};
}

REGISTER_COMPONENT(gui::fog_editor::component)
#endif
