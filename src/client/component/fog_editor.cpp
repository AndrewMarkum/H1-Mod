#include <std_include.hpp>

#ifdef DEBUG
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "game/dvars.hpp"

#include "gui.hpp"

#include "dvars.hpp"

#include <utils/string.hpp>
#include <utils/hook.hpp>

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
	namespace
	{
		bool parse_art_file = true;

		utils::hook::detour cg_parse_client_visionset_triggers;
		void cg_parse_client_visionset_triggers_stub(const char* buffer)
		{
			return cg_parse_client_visionset_triggers.invoke<void>(parse_art_file ? buffer : nullptr);
		}

		bool _trigger = false;
		float _g_fogStartDistReadOnly = 0.f;
		float _g_fogHalfDistReadOnly = 0.f;
		float _g_fogMaxOpacityReadOnly = 0.f;
		bool _g_heightFogEnabledReadOnly = false;
		float _g_heightFogHalfPlaneDistanceReadOnly = 0.f;
		float _g_heightFogBaseHeightReadOnly = 0.f;
		float _g_fogColorReadOnly[4] = { 0.f, 0.f, 0.f, 0.f };
		float _g_fogColorIntensityReadOnly = 0.f;

		bool _g_sunFogEnabledReadOnly = false;
		float _g_sunFogColorReadOnly[4] = { 0.f, 0.f, 0.f, 0.f };
		float _g_sunFogScaleReadOnly = 0.f;
		float _g_sunFogDirReadOnly[3] = {};
		float _g_sunFogBeginFadeAngleReadOnly = 0.f;
		float _g_sunFogEndFadeAngleReadOnly = 0.f;
		float _g_sunFogColorIntensityReadOnly = 1.f;
		float _r_sky_fog_min_angle = 1.f;
		float _r_sky_fog_max_angle = 1.f;

		utils::hook::detour g_glass_update;
		void g_glass_update_stub() // gsc calls
		{
			if (_trigger)
			{
				game::Scr_AddFloat(_r_sky_fog_max_angle);
				game::Scr_AddFloat(_r_sky_fog_min_angle);
				game::Scr_AddFloat(_g_sunFogColorIntensityReadOnly);
				game::Scr_AddFloat(_g_sunFogEndFadeAngleReadOnly);
				game::Scr_AddFloat(_g_sunFogBeginFadeAngleReadOnly);
				game::Scr_AddVector(_g_sunFogDirReadOnly);
				game::Scr_AddFloat(_g_sunFogScaleReadOnly);
				game::Scr_AddVector(_g_sunFogColorReadOnly);
				game::Scr_AddBool(_g_sunFogEnabledReadOnly);
				game::Scr_AddFloat(_g_fogColorIntensityReadOnly);
				game::Scr_AddVector(_g_fogColorReadOnly);
				game::Scr_AddFloat(_g_heightFogBaseHeightReadOnly);
				game::Scr_AddFloat(_g_heightFogHalfPlaneDistanceReadOnly);
				game::Scr_AddBool(_g_heightFogEnabledReadOnly);
				game::Scr_AddFloat(_g_fogMaxOpacityReadOnly);
				game::Scr_AddFloat(_g_fogHalfDistReadOnly);
				game::Scr_AddFloat(_g_fogStartDistReadOnly);
				game::Scr_SetFog(-1, "SetExpFog", 0);
				_trigger = false;
			}

			g_glass_update.invoke<void>();
		}

		bool _r_fog = true;
		float _r_fog_depthhack_scale = 1.f;
		int _r_fog_ev_adjust = 0.f;

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

		void _write_fog()
		{
			_trigger = true;
		}

		void build_atmos_fog_buffer()
		{
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
			static auto* enabled = &gui::enabled_menus["fog_editor"];

#pragma region dvars
			static auto* r_fog = game::Dvar_FindVar("r_fog");
			static auto* r_fog_depthhack_scale = game::Dvar_FindVar("r_fog_depthhack_scale");
			static auto* r_fog_ev_adjust = game::Dvar_FindVar("r_fog_ev_adjust");

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

				//READ_FLOAT(_g_fogStartDistReadOnly, g_fogStartDistReadOnly);
				//READ_FLOAT(_g_fogHalfDistReadOnly, g_fogHalfDistReadOnly);
				//READ_FLOAT(_g_fogMaxOpacityReadOnly, g_fogMaxOpacityReadOnly);
				//READ_BOOL(_g_heightFogEnabledReadOnly, g_heightFogEnabledReadOnly);
				//READ_FLOAT(_g_heightFogHalfPlaneDistanceReadOnly, g_heightFogHalfPlaneDistanceReadOnly);
				//READ_FLOAT(_g_heightFogBaseHeightReadOnly, g_heightFogBaseHeightReadOnly);
				//READ_VEC3(_g_fogColorReadOnly, g_fogColorReadOnly);
				//READ_FLOAT(_g_fogColorIntensityReadOnly, g_fogColorIntensityReadOnly);
				//READ_BOOL(_g_sunFogEnabledReadOnly, g_sunFogEnabledReadOnly);
				//READ_VEC3(_g_sunFogColorReadOnly, g_sunFogColorReadOnly);
				//READ_FLOAT(_g_sunFogScaleReadOnly, g_sunFogScaleReadOnly);
				//READ_VEC3(_g_sunFogDirReadOnly, g_sunFogDirReadOnly);
				//READ_FLOAT(_g_sunFogBeginFadeAngleReadOnly, g_sunFogBeginFadeAngleReadOnly);
				//READ_FLOAT(_g_sunFogEndFadeAngleReadOnly, g_sunFogEndFadeAngleReadOnly);
				//READ_FLOAT(_g_sunFogColorIntensityReadOnly, g_sunFogColorIntensityReadOnly);
				//READ_FLOAT(_r_sky_fog_min_angle, r_sky_fog_min_angle);
				//READ_FLOAT(_r_sky_fog_max_angle, r_sky_fog_max_angle);
			}

			ImGui::Checkbox("parse fog file on map load", &parse_art_file);

			if (ImGui::Checkbox("show fog", &_r_fog))
			{
				dvars::override::set_bool("r_fog", _r_fog);
				dvars::dvar_set_bool(r_fog, _r_fog);
			}

			if (ImGui::CollapsingHeader("Generic Fog"))
			{
				WRITE_FLOAT("start dist", _g_fogStartDistReadOnly, .1f, g_fogStartDistReadOnly);
				WRITE_FLOAT("half dist", _g_fogHalfDistReadOnly, .1f, g_fogHalfDistReadOnly);
				WRITE_FLOAT("max opacity", _g_fogMaxOpacityReadOnly, .1f, g_fogMaxOpacityReadOnly);
				WRITE_BOOL("fog height enabled", _g_heightFogEnabledReadOnly);
				WRITE_FLOAT("fog half plane distance", _g_heightFogHalfPlaneDistanceReadOnly, .1f, g_heightFogHalfPlaneDistanceReadOnly);
				WRITE_FLOAT("fog base height", _g_heightFogBaseHeightReadOnly, .1f, g_heightFogBaseHeightReadOnly);
				WRITE_COLOUR("fog color", _g_fogColorReadOnly);
				WRITE_FLOAT("fog color intensity", _g_fogColorIntensityReadOnly, .1f, g_fogColorIntensityReadOnly);
				WRITE_BOOL("sun fog enabled", _g_sunFogEnabledReadOnly);
				WRITE_COLOUR("sun fog color", _g_sunFogColorReadOnly);
				WRITE_FLOAT("sun fog color intensity", _g_sunFogColorIntensityReadOnly, .1f, g_sunFogColorIntensityReadOnly);
				WRITE_VEC3("sun fog dir", _g_sunFogDirReadOnly, .1f, g_sunFogDirReadOnly);
				WRITE_FLOAT("sun fog begin fade angle", _g_sunFogBeginFadeAngleReadOnly, .1f, g_sunFogBeginFadeAngleReadOnly);
				WRITE_FLOAT("sun fog end fade angle", _g_sunFogEndFadeAngleReadOnly, .1f, g_sunFogEndFadeAngleReadOnly);
				WRITE_FLOAT("sun fog scale", _g_sunFogScaleReadOnly, .1f, g_sunFogScaleReadOnly);
				WRITE_FLOAT("sky fog min angle", _r_sky_fog_min_angle, .1f, r_sky_fog_min_angle);
				WRITE_FLOAT("sky fog max angle", _r_sky_fog_max_angle, .1f, r_sky_fog_max_angle);
				WRITE_BOOL("height fog enabled", _g_heightFogEnabledReadOnly);
				WRITE_FLOAT("height fog base height", _g_heightFogBaseHeightReadOnly, .1f, g_heightFogBaseHeightReadOnly);
				WRITE_FLOAT("height fog half plane distance", _g_heightFogHalfPlaneDistanceReadOnly, .1f, g_heightFogHalfPlaneDistanceReadOnly);
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
			g_glass_update.create(0x417940_b, &g_glass_update_stub);
			gui::register_menu("fog_editor", "Fog Editor", render_window);
		}
	};
}

REGISTER_COMPONENT(gui::fog_editor::component)
#endif
