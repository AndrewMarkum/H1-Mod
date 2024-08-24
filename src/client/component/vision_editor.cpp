#include <std_include.hpp>

#ifdef DEBUG
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "game/dvars.hpp"

#include "gui.hpp"
#include "save_settings.hpp"

#include <utils/string.hpp>
#include <utils/hook.hpp>

#pragma region macros

#define ADD_FLOAT(__string__, __dvar__) \
		ADD_FLOAT_STEP(__string__, __dvar__, 0.01f); \

#define ADD_FLOAT_STEP(__string__, __dvar__, __step__ ) \
		if (__dvar__ != nullptr) ImGui::DragFloat(__string__, &__dvar__->current.value, __step__, __dvar__->domain.value.min, __dvar__->domain.value.max); \

#define ADD_COLOUR(__string__, __dvar__) \
		if (__dvar__ != nullptr) ImGui::ColorEdit3(__string__, __dvar__->current.vector); \

#define ADD_INT(__string__, __dvar__) \
		if (__dvar__ != nullptr) ImGui::DragInt(__string__, &__dvar__->current.integer, 1.0f, __dvar__->domain.integer.min, __dvar__->domain.integer.max); \

#define ADD_BOOL(__string__, __dvar__) \
		if (__dvar__ != nullptr) ImGui::Checkbox(__string__, &__dvar__->current.enabled);

#pragma endregion

namespace gui::vision_editor
{
	namespace
	{
		void render_window()
		{
			static auto* enabled = &gui::enabled_menus["vision_editor"];

#pragma region dvars
			static auto* sm_sunEnable = game::Dvar_FindVar("sm_sunEnable");
			static auto* sm_sunShadowScale = game::Dvar_FindVar("sm_sunShadowScale");
			static auto* sm_spotLimit = game::Dvar_FindVar("sm_spotLimit");
			static auto* sm_qualitySpotShadow = game::Dvar_FindVar("sm_qualitySpotShadow");
			static auto* sm_usedSunCascadeCount = game::Dvar_FindVar("sm_usedSunCascadeCount");
			static auto* sm_sunSampleSizeNear = game::Dvar_FindVar("sm_sunSampleSizeNear");
			static auto* sm_sunFilterRadius = game::Dvar_FindVar("sm_sunFilterRadius");
			static auto* sm_spotFilterRadius = game::Dvar_FindVar("sm_spotFilterRadius");
			static auto* r_specularColorScale = game::Dvar_FindVar("r_specularColorScale");
			static auto* r_diffuseColorScale = game::Dvar_FindVar("r_diffuseColorScale");
			static auto* r_veil = game::Dvar_FindVar("r_veil");
			static auto* r_veilStrength = game::Dvar_FindVar("r_veilStrength");
			static auto* r_veilBackgroundStrength = game::Dvar_FindVar("r_veilBackgroundStrength");
			static auto* r_tonemap = game::Dvar_FindVar("r_tonemap");
			static auto* r_tonemapAuto = game::Dvar_FindVar("r_tonemapAuto");
			static auto* r_tonemapBlend = game::Dvar_FindVar("r_tonemapBlend");
			static auto* r_tonemapLockAutoExposureAdjust = game::Dvar_FindVar("r_tonemapLockAutoExposureAdjust");
			static auto* r_tonemapAutoExposureAdjust = game::Dvar_FindVar("r_tonemapAutoExposureAdjust");
			static auto* r_tonemapExposure = game::Dvar_FindVar("r_tonemapExposure");
			static auto* r_tonemapExposureAdjust = game::Dvar_FindVar("r_tonemapExposureAdjust");
			static auto* r_tonemapMaxExposure = game::Dvar_FindVar("r_tonemapMaxExposure");
			static auto* r_tonemapAdaptSpeed = game::Dvar_FindVar("r_tonemapAdaptSpeed");
			static auto* r_tonemapDarkEv = game::Dvar_FindVar("r_tonemapDarkEv");
			static auto* r_tonemapMidEv = game::Dvar_FindVar("r_tonemapMidEv");
			static auto* r_tonemapLightEv = game::Dvar_FindVar("r_tonemapLightEv");
			static auto* r_tonemapDarkExposureAdjust = game::Dvar_FindVar("r_tonemapDarkExposureAdjust");
			static auto* r_tonemapMidExposureAdjust = game::Dvar_FindVar("r_tonemapMidExposureAdjust");
			static auto* r_tonemapLightExposureAdjust = game::Dvar_FindVar("r_tonemapLightExposureAdjust");
			static auto* r_tonemapMinExposureAdjust = game::Dvar_FindVar("r_tonemapMinExposureAdjust");
			static auto* r_tonemapMaxExposureAdjust = game::Dvar_FindVar("r_tonemapMaxExposureAdjust");
			static auto* r_tonemapWhite = game::Dvar_FindVar("r_tonemapWhite");
			static auto* r_tonemapShoulder = game::Dvar_FindVar("r_tonemapShoulder");
			static auto* r_tonemapCrossover = game::Dvar_FindVar("r_tonemapCrossover");
			static auto* r_tonemapToe = game::Dvar_FindVar("r_tonemapToe");
			static auto* r_tonemapBlack = game::Dvar_FindVar("r_tonemapBlack");
			static auto* r_aoDiminish = game::Dvar_FindVar("r_aoDiminish");
			static auto* r_ssao = game::Dvar_FindVar("r_ssao");
			static auto* r_ssaoStrength = game::Dvar_FindVar("r_ssaoStrength");
			static auto* r_ssaoPower = game::Dvar_FindVar("r_ssaoPower");
			static auto* r_ssaoMinStrengthDepth = game::Dvar_FindVar("r_ssaoMinStrengthDepth");
			static auto* r_ssaoMaxStrengthDepth = game::Dvar_FindVar("r_ssaoMaxStrengthDepth");
			static auto* r_ssaoWidth = game::Dvar_FindVar("r_ssaoWidth");
			static auto* r_ssaoGapFalloff = game::Dvar_FindVar("r_ssaoGapFalloff");
			static auto* r_ssaoGradientFalloff = game::Dvar_FindVar("r_ssaoGradientFalloff");
			static auto* r_ssaoFadeDepth = game::Dvar_FindVar("r_ssaoFadeDepth");
			static auto* r_ssaoRejectDepth = game::Dvar_FindVar("r_ssaoRejectDepth");
			static auto* r_sky_fog_min_angle = game::Dvar_FindVar("r_sky_fog_min_angle");
			static auto* r_sky_fog_max_angle = game::Dvar_FindVar("r_sky_fog_max_angle");
			static auto* r_sky_fog_intensity = game::Dvar_FindVar("r_sky_fog_intensity");

			static auto* r_filmTweakBrightness = game::Dvar_FindVar("r_filmTweakBrightness");
			static auto* r_filmTweakContrast = game::Dvar_FindVar("r_filmTweakContrast");
			static auto* r_filmTweakDarkTint = game::Dvar_FindVar("r_filmTweakDarkTint");
			static auto* r_filmTweakDesaturation = game::Dvar_FindVar("r_filmTweakDesaturation");
			static auto* r_filmTweakDesaturationDark = game::Dvar_FindVar("r_filmTweakDesaturationDark");
			static auto* r_filmTweakEnable = game::Dvar_FindVar("r_filmTweakEnable");
			static auto* r_filmTweakInvert = game::Dvar_FindVar("r_filmTweakInvert");
			static auto* r_filmTweakLightTint = game::Dvar_FindVar("r_filmTweakLightTint");
			static auto* r_filmTweakMediumTint = game::Dvar_FindVar("r_filmTweakMediumTint");
			static auto* r_primaryLightTweakDiffuseStrength = game::Dvar_FindVar("r_primaryLightTweakDiffuseStrength");
			static auto* r_primaryLightTweakSpecularStrength = game::Dvar_FindVar("r_primaryLightTweakSpecularStrength");
			static auto* r_viewModelPrimaryLightTweakDiffuseStrength = game::Dvar_FindVar("r_viewModelPrimaryLightTweakDiffuseStrength");
			static auto* r_viewModelPrimaryLightTweakSpecularStrength = game::Dvar_FindVar("r_viewModelPrimaryLightTweakSpecularStrength");

			static auto* r_aoUseTweaks = game::Dvar_FindVar("r_aoUseTweaks");
			static auto* r_colorScaleUseTweaks = game::Dvar_FindVar("r_colorScaleUseTweaks");
			static auto* r_filmUseTweaks = game::Dvar_FindVar("r_filmUseTweaks");
			static auto* r_primaryLightUseTweaks = game::Dvar_FindVar("r_primaryLightUseTweaks");
			static auto* r_skyFogUseTweaks = game::Dvar_FindVar("r_skyFogUseTweaks");
			static auto* r_ssaoUseTweaks = game::Dvar_FindVar("r_ssaoUseTweaks");
			static auto* r_tonemapUseTweaks = game::Dvar_FindVar("r_tonemapUseTweaks");
			static auto* r_veilUseTweaks = game::Dvar_FindVar("r_veilUseTweaks");
			static auto* r_viewModelPrimaryLightUseTweaks = game::Dvar_FindVar("r_viewModelPrimaryLightUseTweaks");
			static auto* sm_shadowUseTweaks = game::Dvar_FindVar("sm_shadowUseTweaks");
#pragma endregion

			ImGui::Begin("Toggle Tweaks");
			ADD_BOOL("ao (screen-space)", r_ssaoUseTweaks);
			ADD_BOOL("ao", r_aoUseTweaks);
			ADD_BOOL("colour scale", r_colorScaleUseTweaks);
			ADD_BOOL("film", r_filmUseTweaks);
			ADD_BOOL("primary light (viewmodel)", r_viewModelPrimaryLightUseTweaks);
			ADD_BOOL("primary light", r_primaryLightUseTweaks);
			ADD_BOOL("shadows", sm_shadowUseTweaks);
			ADD_BOOL("sky fog", r_skyFogUseTweaks);
			ADD_BOOL("tonemapping", r_tonemapUseTweaks);
			ADD_BOOL("veil", r_veilUseTweaks);
			ImGui::End();

			ImGui::SetNextWindowSizeConstraints(ImVec2(500, 500), ImVec2(1000, 1000));
			ImGui::Begin("Vision Editor");

			if (ImGui::CollapsingHeader("ambient occlusion"))
			{
				ADD_FLOAT("ao diminish", r_aoDiminish);
				ADD_INT("ssao mode", r_ssao);
				ADD_FLOAT("ssao strength", r_ssaoStrength);
				ADD_FLOAT("ssao power", r_ssaoPower);
				ADD_FLOAT("ssao min strength depth", r_ssaoMinStrengthDepth);
				ADD_FLOAT("ssao max strength depth", r_ssaoMaxStrengthDepth);
				ADD_FLOAT("ssao width", r_ssaoWidth);
				ADD_FLOAT("ssao gap falloff", r_ssaoGapFalloff);
				ADD_FLOAT("ssao gradient falloff", r_ssaoGradientFalloff);
				ADD_FLOAT("ssao fade depth", r_ssaoFadeDepth);
				ADD_FLOAT("ssao reject depth", r_ssaoRejectDepth);
			}

			if (ImGui::CollapsingHeader("color scale"))
			{
				ADD_FLOAT("specular color scale", r_specularColorScale);
				ADD_FLOAT("diffuse color scale", r_diffuseColorScale);
			}

			if (ImGui::CollapsingHeader("filmtweaks"))
			{
				ADD_BOOL("enable tweak", r_filmTweakEnable);
				ADD_FLOAT("brightness", r_filmTweakBrightness);
				ADD_FLOAT("contrast", r_filmTweakContrast);
				ADD_BOOL("invert", r_filmTweakInvert);
				ADD_COLOUR("light tint", r_filmTweakLightTint);
				ADD_COLOUR("medium tint", r_filmTweakMediumTint);
				ADD_COLOUR("dark tint", r_filmTweakDarkTint);
				ADD_FLOAT("desaturation", r_filmTweakDesaturation);
				ADD_FLOAT("desaturation dark", r_filmTweakDesaturationDark);
			}

			if (ImGui::CollapsingHeader("HDR tonemapping"))
			{
				ADD_INT("mode", r_tonemap);
				ADD_BOOL("blend between exposures", r_tonemapBlend);
				ADD_BOOL("auto-exposure", r_tonemapAuto);
				ADD_BOOL("lock auto-exposure adjust", r_tonemapLockAutoExposureAdjust);
				ADD_FLOAT("auto-exposure adjust (0 = auto)", r_tonemapAutoExposureAdjust);
				ADD_FLOAT("exposure", r_tonemapExposure);
				ADD_FLOAT("exposure adjust", r_tonemapExposureAdjust);
				ADD_FLOAT("max exposure", r_tonemapMaxExposure);
				ADD_FLOAT("adapt speed", r_tonemapAdaptSpeed);
				ADD_FLOAT("dark ev", r_tonemapDarkEv);
				ADD_FLOAT("mid ev", r_tonemapMidEv);
				ADD_FLOAT("light ev", r_tonemapLightEv);
				ADD_FLOAT("dark exposure adjust", r_tonemapDarkExposureAdjust);
				ADD_FLOAT("mid exposure adjust", r_tonemapMidExposureAdjust);
				ADD_FLOAT("light exposure adjust", r_tonemapLightExposureAdjust);
				ADD_FLOAT_STEP("black point", r_tonemapBlack, 1);
				ADD_FLOAT_STEP("white point", r_tonemapWhite, 10);
				ADD_FLOAT("shoulder control", r_tonemapShoulder);
				ADD_FLOAT("crossover control", r_tonemapCrossover);
				ADD_FLOAT("toe control", r_tonemapToe);
			}

			if (ImGui::CollapsingHeader("primary light (viewmodel)"))
			{
				ADD_FLOAT("diffuse strength (vm)", r_viewModelPrimaryLightTweakDiffuseStrength);
				ADD_FLOAT("specular strength (vm)", r_viewModelPrimaryLightTweakSpecularStrength);
			}

			if (ImGui::CollapsingHeader("primary light"))
			{
				ADD_FLOAT("diffuse strength", r_primaryLightTweakDiffuseStrength);
				ADD_FLOAT("specular strength", r_primaryLightTweakSpecularStrength);
			}

			if (ImGui::CollapsingHeader("shadows"))
			{
				ADD_BOOL("enable sun", sm_sunEnable);
				ADD_FLOAT("sun shadow scale", sm_sunShadowScale);
				ADD_INT("spot limit", sm_spotLimit);
				ADD_BOOL("quality spot shadow", sm_qualitySpotShadow);
				ADD_INT("used sun cascade count", sm_usedSunCascadeCount);
				ADD_FLOAT("sun sample size near", sm_sunSampleSizeNear);
				ADD_FLOAT("sun filter radius", sm_sunFilterRadius);
				ADD_FLOAT("spot filter radius", sm_spotFilterRadius);
			}

			if (ImGui::CollapsingHeader("sky fog"))
			{
				ADD_FLOAT_STEP("min angle", r_sky_fog_min_angle, 1.0f);
				ADD_FLOAT_STEP("max angle", r_sky_fog_max_angle, 1.0f);
				ADD_FLOAT("intensity", r_sky_fog_intensity);
			}
		
			if (ImGui::CollapsingHeader("veiling luminance (HDR glow)"))
			{
				ADD_BOOL("enable veil", r_veil);
				ADD_FLOAT("strength", r_veilStrength);
				ADD_FLOAT("background strength", r_veilBackgroundStrength);
			}

			if (ImGui::Button("Save to file"))
			{
				if (!strcmp(game::Dvar_FindVar("mapname")->current.string, "frontend"))
					return;

				ADD_BOOL("ao (screen-space)", r_ssaoUseTweaks);
				ADD_BOOL("ao", r_aoUseTweaks);
				ADD_BOOL("colour scale", r_colorScaleUseTweaks);
				ADD_BOOL("film", r_filmUseTweaks);
				ADD_BOOL("primary light (viewmodel)", r_viewModelPrimaryLightUseTweaks);
				ADD_BOOL("primary light", r_primaryLightUseTweaks);
				ADD_BOOL("shadows", sm_shadowUseTweaks);
				ADD_BOOL("sky fog", r_skyFogUseTweaks);
				ADD_BOOL("tonemapping", r_tonemapUseTweaks);
				ADD_BOOL("veil", r_veilUseTweaks);

				save_settings::_settings_json["toggletweaks"]["r_ssaoUseTweaks"] = r_ssaoUseTweaks->current.enabled;
				save_settings::_settings_json["toggletweaks"]["r_aoUseTweaks"] = r_aoUseTweaks->current.enabled;
				save_settings::_settings_json["toggletweaks"]["r_filmUseTweaks"] = r_filmUseTweaks->current.enabled;
				save_settings::_settings_json["toggletweaks"]["r_viewModelPrimaryLightUseTweaks"] = r_viewModelPrimaryLightUseTweaks->current.enabled;
				save_settings::_settings_json["toggletweaks"]["r_primaryLightUseTweaks"] = r_primaryLightUseTweaks->current.enabled;
				save_settings::_settings_json["toggletweaks"]["sm_shadowUseTweaks"] = sm_shadowUseTweaks->current.enabled;
				save_settings::_settings_json["toggletweaks"]["r_skyFogUseTweaks"] = r_skyFogUseTweaks->current.enabled;
				save_settings::_settings_json["toggletweaks"]["r_tonemapUseTweaks"] = r_tonemapUseTweaks->current.enabled;
				save_settings::_settings_json["toggletweaks"]["r_veilUseTweaks"] = r_veilUseTweaks->current.enabled;


				if(r_ssaoUseTweaks->current.enabled)
				{
					save_settings::_settings_json["ssao"]["ssao_r_aoDiminish"] = r_aoDiminish->current.value;
					save_settings::_settings_json["ssao"]["ssao_r_ssao"] = r_ssao->current.integer;
					save_settings::_settings_json["ssao"]["ssao_r_ssaoStrength"] = r_ssaoStrength->current.value;
					save_settings::_settings_json["ssao"]["ssao_r_ssaoPower"] = r_ssaoPower->current.value;
					save_settings::_settings_json["ssao"]["ssao_r_ssaoMinStrengthDepth"] = r_ssaoMinStrengthDepth->current.value;
					save_settings::_settings_json["ssao"]["ssao_r_ssaoMaxStrengthDepth"] = r_ssaoMaxStrengthDepth->current.value;
					save_settings::_settings_json["ssao"]["ssao_r_ssaoWidth"] = r_ssaoWidth->current.value;
					save_settings::_settings_json["ssao"]["ssao_r_ssaoGapFalloff"] = r_ssaoGapFalloff->current.value;
					save_settings::_settings_json["ssao"]["ssao_r_ssaoGradientFalloff"] = r_ssaoGradientFalloff->current.value;
					save_settings::_settings_json["ssao"]["ssao_r_ssaoFadeDepth"] = r_ssaoFadeDepth->current.value;
					save_settings::_settings_json["ssao"]["ssao_r_ssaoRejectDepth"] = r_ssaoRejectDepth->current.value;
				}

				if(r_aoUseTweaks->current.enabled)
				{
					save_settings::_settings_json["ao"]["ao_r_aoDiminish"] = r_aoDiminish->current.value;
					save_settings::_settings_json["ao"]["ao_r_ssao"] = r_ssao->current.integer;
					save_settings::_settings_json["ao"]["ao_r_ssaoStrength"] = r_ssaoStrength->current.value;
					save_settings::_settings_json["ao"]["ao_r_ssaoPower"] = r_ssaoPower->current.value;
					save_settings::_settings_json["ao"]["ao_r_ssaoMinStrengthDepth"] = r_ssaoMinStrengthDepth->current.value;
					save_settings::_settings_json["ao"]["ao_r_ssaoMaxStrengthDepth"] = r_ssaoMaxStrengthDepth->current.value;
					save_settings::_settings_json["ao"]["ao_r_ssaoWidth"] = r_ssaoWidth->current.value;
					save_settings::_settings_json["ao"]["ao_r_ssaoGapFalloff"] = r_ssaoGapFalloff->current.value;
					save_settings::_settings_json["ao"]["ao_r_ssaoGradientFalloff"] = r_ssaoGradientFalloff->current.value;
					save_settings::_settings_json["ao"]["ao_r_ssaoFadeDepth"] = r_ssaoFadeDepth->current.value;
					save_settings::_settings_json["ao"]["ao_r_ssaoRejectDepth"] = r_ssaoRejectDepth->current.value;
				}

				if(r_colorScaleUseTweaks->current.enabled)
				{
					save_settings::_settings_json["colorScale"]["cs_r_specularColorScale"] = r_specularColorScale->current.value;
					save_settings::_settings_json["colorScale"]["cs_r_diffuseColorScale"] = r_diffuseColorScale->current.integer;
				}

				if (r_filmUseTweaks->current.enabled)
				{
					save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakEnable"] = r_filmTweakEnable->current.enabled;
					save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakBrightness"] = r_filmTweakBrightness->current.value;
					save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakContrast"] = r_filmTweakContrast->current.value;
					save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakInvert"] = r_filmTweakInvert->current.enabled;
					save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakLightTint"] = r_filmTweakLightTint->current.vector;
					save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakMediumTint"] = r_filmTweakMediumTint->current.vector;
					save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakDarkTint"] = r_filmTweakDarkTint->current.vector;
					save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakDesaturation"] = r_filmTweakDesaturation->current.value;
					save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakDesaturationDark"] = r_filmTweakDesaturationDark->current.value;
				}

				if (r_viewModelPrimaryLightUseTweaks->current.enabled)
				{
					save_settings::_settings_json["viewmodelprimarylight"]["plvm_r_viewModelPrimaryLightTweakDiffuseStrength"] = r_viewModelPrimaryLightTweakDiffuseStrength->current.value;
					save_settings::_settings_json["viewmodelprimarylight"]["plvm_r_viewModelPrimaryLightTweakSpecularStrength"] = r_viewModelPrimaryLightTweakSpecularStrength->current.value;
				}

				if (r_primaryLightUseTweaks->current.enabled)
				{
					save_settings::_settings_json["primarylight"]["pl_r_PrimaryLightTweakDiffuseStrength"] = r_primaryLightTweakDiffuseStrength->current.value;
					save_settings::_settings_json["primarylight"]["pl_r_PrimaryLightTweakSpecularStrength"] = r_primaryLightTweakDiffuseStrength->current.value;
				}

				if (sm_shadowUseTweaks->current.enabled)
				{
					save_settings::_settings_json["shadow"]["sut_sm_sunEnable"] = sm_sunEnable->current.enabled;
					save_settings::_settings_json["shadow"]["sut_sm_sunShadowScale"] = sm_sunShadowScale->current.value;
					save_settings::_settings_json["shadow"]["sut_sm_spotLimit"] = sm_spotLimit->current.integer;
					save_settings::_settings_json["shadow"]["sut_sm_qualitySpotShadow"] = sm_qualitySpotShadow->current.enabled;
					save_settings::_settings_json["shadow"]["sut_sm_usedSunCascadeCount"] = sm_usedSunCascadeCount->current.integer;
					save_settings::_settings_json["shadow"]["sut_sm_sunSampleSizeNear"] = sm_sunSampleSizeNear->current.value;
					save_settings::_settings_json["shadow"]["sut_sm_sunFilterRadius"] = sm_sunFilterRadius->current.value;
					save_settings::_settings_json["shadow"]["sut_sm_spotFilterRadius"] = sm_spotFilterRadius->current.value;
				}

				if (r_skyFogUseTweaks->current.enabled)
				{
					save_settings::_settings_json["skyfog"]["skyf_r_sky_fog_min_angle"] = r_sky_fog_min_angle->current.value;
					save_settings::_settings_json["skyfog"]["skyf_r_sky_fog_max_angle"] = r_sky_fog_max_angle->current.value;
					save_settings::_settings_json["skyfog"]["skyf_r_sky_fog_intensity"] = r_sky_fog_intensity->current.value;
				}

				if (r_tonemapUseTweaks->current.enabled)
				{
					save_settings::_settings_json["tonemap"]["tone_r_tonemap"] = r_tonemap->current.integer;
					save_settings::_settings_json["tonemap"]["tone_r_tonemapBlend"] = r_tonemapBlend->current.enabled;
					save_settings::_settings_json["tonemap"]["tone_r_tonemapAuto"] = r_tonemapAuto->current.enabled;
					save_settings::_settings_json["tonemap"]["tone_r_tonemapLockAutoExposureAdjust"] = r_tonemapLockAutoExposureAdjust->current.enabled;
					save_settings::_settings_json["tonemap"]["tone_r_tonemapAutoExposureAdjust"] = r_tonemapAutoExposureAdjust->current.value;
					save_settings::_settings_json["tonemap"]["tone_r_tonemapExposure"] = r_tonemapExposure->current.value;
					save_settings::_settings_json["tonemap"]["tone_r_tonemapExposureAdjust"] = r_tonemapExposureAdjust->current.value;
					save_settings::_settings_json["tonemap"]["tone_r_tonemapMaxExposure"] = r_tonemapMaxExposure->current.value;
					save_settings::_settings_json["tonemap"]["tone_r_r_tonemapAdaptSpeed"] = r_tonemapAdaptSpeed->current.value;
					save_settings::_settings_json["tonemap"]["tone_r_tonemapDarkEv"] = r_tonemapDarkEv->current.value;
					save_settings::_settings_json["tonemap"]["tone_r_tonemapMidEv"] = r_tonemapMidEv->current.value;
					save_settings::_settings_json["tonemap"]["tone_r_tonemapLightEv"] = r_tonemapLightEv->current.value;
					save_settings::_settings_json["tonemap"]["tone_r_tonemapDarkExposureAdjust"] = r_tonemapDarkExposureAdjust->current.value;
					save_settings::_settings_json["tonemap"]["tone_r_tonemapMidExposureAdjust"] = r_tonemapMidExposureAdjust->current.value;
					save_settings::_settings_json["tonemap"]["tone_r_tonemapLightExposureAdjust"] = r_tonemapLightExposureAdjust->current.value;
					save_settings::_settings_json["tonemap"]["tone_r_tonemapBlack"] = r_tonemapBlack->current.value;
					save_settings::_settings_json["tonemap"]["tone_r_tonemapWhite"] = r_tonemapWhite->current.value;
					save_settings::_settings_json["tonemap"]["tone_r_tonemapShoulder"] = r_tonemapShoulder->current.value;
					save_settings::_settings_json["tonemap"]["tone_r_tonemapCrossover"] = r_tonemapCrossover->current.value;
					save_settings::_settings_json["tonemap"]["tone_r_tonemapToe"] = r_tonemapToe->current.value;
				}

				if (r_veilUseTweaks->current.enabled)
				{
					save_settings::_settings_json["veil"]["veil_r_veil"] = r_veil->current.enabled;
					save_settings::_settings_json["veil"]["veil_r_veilStrength"] = r_veilStrength->current.value;
					save_settings::_settings_json["veil"]["veil_r_veilBackgroundStrength"] = r_veilBackgroundStrength->current.value;
				}

				save_settings::save_vision_to_file(std::string(std::string("IW5r/maps/") + game::Dvar_FindVar("mapname")->current.string + std::string("/vision/" + std::string(game::Dvar_FindVar("mapname")->current.string) + "_vision.json")));
			}
			ImGui::SameLine();

			if (ImGui::Button("Load from file"))
			{
				save_settings::load_vision_from_file(std::string(std::string("IW5r/maps/") + game::Dvar_FindVar("mapname")->current.string + std::string("/vision/" + std::string(game::Dvar_FindVar("mapname")->current.string) + "_vision.json")));
				
				r_ssaoUseTweaks->current.enabled = save_settings::_settings_json["toggletweaks"]["r_ssaoUseTweaks"].get<bool>();
				r_aoUseTweaks->current.enabled = save_settings::_settings_json["toggletweaks"]["r_aoUseTweaks"].get<bool>();
				r_filmUseTweaks->current.enabled = save_settings::_settings_json["toggletweaks"]["r_filmUseTweaks"].get<bool>();
				r_viewModelPrimaryLightUseTweaks->current.enabled = save_settings::_settings_json["toggletweaks"]["r_viewModelPrimaryLightUseTweaks"].get<bool>();
				r_primaryLightUseTweaks->current.enabled = save_settings::_settings_json["toggletweaks"]["r_primaryLightUseTweaks"].get<bool>();
				sm_shadowUseTweaks->current.enabled = save_settings::_settings_json["toggletweaks"]["sm_shadowUseTweaks"].get<bool>();
				r_skyFogUseTweaks->current.enabled = save_settings::_settings_json["toggletweaks"]["r_skyFogUseTweaks"].get<bool>();
				r_tonemapUseTweaks->current.enabled = save_settings::_settings_json["toggletweaks"]["r_tonemapUseTweaks"].get<bool>();
				r_veilUseTweaks->current.enabled = save_settings::_settings_json["toggletweaks"]["r_veilUseTweaks"].get<bool>();

				// SSAO settings
				if (save_settings::_settings_json.contains("ssao")) {
					r_aoDiminish->current.value = save_settings::_settings_json["ssao"]["ssao_r_aoDiminish"];
					r_ssao->current.integer = save_settings::_settings_json["ssao"]["ssao_r_ssao"];
					r_ssaoStrength->current.value = save_settings::_settings_json["ssao"]["ssao_r_ssaoStrength"];
					r_ssaoPower->current.value = save_settings::_settings_json["ssao"]["ssao_r_ssaoPower"];
					r_ssaoMinStrengthDepth->current.value = save_settings::_settings_json["ssao"]["ssao_r_ssaoMinStrengthDepth"];
					r_ssaoMaxStrengthDepth->current.value = save_settings::_settings_json["ssao"]["ssao_r_ssaoMaxStrengthDepth"];
					r_ssaoWidth->current.value = save_settings::_settings_json["ssao"]["ssao_r_ssaoWidth"];
					r_ssaoGapFalloff->current.value = save_settings::_settings_json["ssao"]["ssao_r_ssaoGapFalloff"];
					r_ssaoGradientFalloff->current.value = save_settings::_settings_json["ssao"]["ssao_r_ssaoGradientFalloff"];
					r_ssaoFadeDepth->current.value = save_settings::_settings_json["ssao"]["ssao_r_ssaoFadeDepth"];
					r_ssaoRejectDepth->current.value = save_settings::_settings_json["ssao"]["ssao_r_ssaoRejectDepth"];
				}

				// AO settings
				if (save_settings::_settings_json.contains("ao")) {
					r_aoDiminish->current.value = save_settings::_settings_json["ao"]["ao_r_aoDiminish"];
					r_ssao->current.integer = save_settings::_settings_json["ao"]["ao_r_ssao"];
					r_ssaoStrength->current.value = save_settings::_settings_json["ao"]["ao_r_ssaoStrength"];
					r_ssaoPower->current.value = save_settings::_settings_json["ao"]["ao_r_ssaoPower"];
					r_ssaoMinStrengthDepth->current.value = save_settings::_settings_json["ao"]["ao_r_ssaoMinStrengthDepth"];
					r_ssaoMaxStrengthDepth->current.value = save_settings::_settings_json["ao"]["ao_r_ssaoMaxStrengthDepth"];
					r_ssaoWidth->current.value = save_settings::_settings_json["ao"]["ao_r_ssaoWidth"];
					r_ssaoGapFalloff->current.value = save_settings::_settings_json["ao"]["ao_r_ssaoGapFalloff"];
					r_ssaoGradientFalloff->current.value = save_settings::_settings_json["ao"]["ao_r_ssaoGradientFalloff"];
					r_ssaoFadeDepth->current.value = save_settings::_settings_json["ao"]["ao_r_ssaoFadeDepth"];
					r_ssaoRejectDepth->current.value = save_settings::_settings_json["ao"]["ao_r_ssaoRejectDepth"];
				}

				// Color Scale settings
				if (save_settings::_settings_json.contains("colorScale")) {
					r_specularColorScale->current.value = save_settings::_settings_json["colorScale"]["cs_r_specularColorScale"];
					r_diffuseColorScale->current.integer = save_settings::_settings_json["colorScale"]["cs_r_diffuseColorScale"];
				}

				// Film Tweaks settings
				if (save_settings::_settings_json.contains("filmtweaks")) {
					r_filmTweakEnable->current.enabled = save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakEnable"];
					r_filmTweakBrightness->current.value = save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakBrightness"];
					r_filmTweakContrast->current.value = save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakContrast"];
					r_filmTweakInvert->current.enabled = save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakInvert"];

					game::Dvar_SetColor(r_filmTweakLightTint,
						save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakLightTint"][0].get<float>(),
						save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakLightTint"][1].get<float>(),
						save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakLightTint"][2].get<float>(),
						save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakLightTint"][3].get<float>());
					game::Dvar_SetColor(r_filmTweakMediumTint,
						save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakMediumTint"][0].get<float>(),
						save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakMediumTint"][1].get<float>(),
						save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakMediumTint"][2].get<float>(),
						save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakMediumTint"][3].get<float>());
					game::Dvar_SetColor(r_filmTweakDesaturationDark,
						save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakDarkTint"][0].get<float>(),
						save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakDarkTint"][1].get<float>(),
						save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakDarkTint"][2].get<float>(),
						save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakDarkTint"][3].get<float>());

					r_filmTweakDesaturation->current.value = save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakDesaturation"];
					r_filmTweakDesaturationDark->current.value = save_settings::_settings_json["filmtweaks"]["ft_r_filmTweakDesaturationDark"];
				}

				// View Model Primary Light settings
				if (save_settings::_settings_json.contains("viewmodelprimarylight")) {
					r_viewModelPrimaryLightTweakDiffuseStrength->current.value = save_settings::_settings_json["viewmodelprimarylight"]["plvm_r_viewModelPrimaryLightTweakDiffuseStrength"];
					r_viewModelPrimaryLightTweakSpecularStrength->current.value = save_settings::_settings_json["viewmodelprimarylight"]["plvm_r_viewModelPrimaryLightTweakSpecularStrength"];
				}

				// Primary Light settings
				if (save_settings::_settings_json.contains("primarylight")) {
					r_primaryLightTweakDiffuseStrength->current.value = save_settings::_settings_json["primarylight"]["pl_r_PrimaryLightTweakDiffuseStrength"];
					r_primaryLightTweakSpecularStrength->current.value = save_settings::_settings_json["primarylight"]["pl_r_PrimaryLightTweakSpecularStrength"];
				}

				// Shadow settings
				if (save_settings::_settings_json.contains("shadow")) {
					sm_sunEnable->current.enabled = save_settings::_settings_json["shadow"]["sut_sm_sunEnable"];
					sm_sunShadowScale->current.value = save_settings::_settings_json["shadow"]["sut_sm_sunShadowScale"];
					sm_spotLimit->current.integer = save_settings::_settings_json["shadow"]["sut_sm_spotLimit"];
					sm_qualitySpotShadow->current.enabled = save_settings::_settings_json["shadow"]["sut_sm_qualitySpotShadow"];
					sm_usedSunCascadeCount->current.integer = save_settings::_settings_json["shadow"]["sut_sm_usedSunCascadeCount"];
					sm_sunSampleSizeNear->current.value = save_settings::_settings_json["shadow"]["sut_sm_sunSampleSizeNear"];
					sm_sunFilterRadius->current.value = save_settings::_settings_json["shadow"]["sut_sm_sunFilterRadius"];
					sm_spotFilterRadius->current.value = save_settings::_settings_json["shadow"]["sut_sm_spotFilterRadius"];
				}

				// Sky Fog settings
				if (save_settings::_settings_json.contains("skyfog")) {
					r_sky_fog_min_angle->current.value = save_settings::_settings_json["skyfog"]["skyf_r_sky_fog_min_angle"];
					r_sky_fog_max_angle->current.value = save_settings::_settings_json["skyfog"]["skyf_r_sky_fog_max_angle"];
					r_sky_fog_intensity->current.value = save_settings::_settings_json["skyfog"]["skyf_r_sky_fog_intensity"];
				}

				// Tonemap settings
				if (save_settings::_settings_json.contains("tonemap")) {
					r_tonemap->current.integer = save_settings::_settings_json["tonemap"]["tone_r_tonemap"];
					r_tonemapBlend->current.enabled = save_settings::_settings_json["tonemap"]["tone_r_tonemapBlend"];
					r_tonemapAuto->current.enabled = save_settings::_settings_json["tonemap"]["tone_r_tonemapAuto"];
					r_tonemapLockAutoExposureAdjust->current.enabled = save_settings::_settings_json["tonemap"]["tone_r_tonemapLockAutoExposureAdjust"];
					r_tonemapAutoExposureAdjust->current.value = save_settings::_settings_json["tonemap"]["tone_r_tonemapAutoExposureAdjust"];
					r_tonemapExposure->current.value = save_settings::_settings_json["tonemap"]["tone_r_tonemapExposure"];
					r_tonemapExposureAdjust->current.value = save_settings::_settings_json["tonemap"]["tone_r_tonemapExposureAdjust"];
					r_tonemapMaxExposure->current.value = save_settings::_settings_json["tonemap"]["tone_r_tonemapMaxExposure"];
					r_tonemapAdaptSpeed->current.value = save_settings::_settings_json["tonemap"]["tone_r_r_tonemapAdaptSpeed"];
					r_tonemapDarkEv->current.value = save_settings::_settings_json["tonemap"]["tone_r_tonemapDarkEv"];
					r_tonemapMidEv->current.value = save_settings::_settings_json["tonemap"]["tone_r_tonemapMidEv"];
					r_tonemapLightEv->current.value = save_settings::_settings_json["tonemap"]["tone_r_tonemapLightEv"];
					r_tonemapDarkExposureAdjust->current.value = save_settings::_settings_json["tonemap"]["tone_r_tonemapDarkExposureAdjust"];
					r_tonemapMidExposureAdjust->current.value = save_settings::_settings_json["tonemap"]["tone_r_tonemapMidExposureAdjust"];
					r_tonemapLightExposureAdjust->current.value = save_settings::_settings_json["tonemap"]["tone_r_tonemapLightExposureAdjust"];
					r_tonemapBlack->current.value = save_settings::_settings_json["tonemap"]["tone_r_tonemapBlack"];
					r_tonemapWhite->current.value = save_settings::_settings_json["tonemap"]["tone_r_tonemapWhite"];
					r_tonemapShoulder->current.value = save_settings::_settings_json["tonemap"]["tone_r_tonemapShoulder"];
					r_tonemapCrossover->current.value = save_settings::_settings_json["tonemap"]["tone_r_tonemapCrossover"];
					r_tonemapToe->current.value = save_settings::_settings_json["tonemap"]["tone_r_tonemapToe"];
				}

				// Veil settings
				if (save_settings::_settings_json.contains("veil")) {
					r_veil->current.enabled = save_settings::_settings_json["veil"]["veil_r_veil"];
					r_veilStrength->current.value = save_settings::_settings_json["veil"]["veil_r_veilStrength"];
					r_veilBackgroundStrength->current.value = save_settings::_settings_json["veil"]["veil_r_veilBackgroundStrength"];
				}

			
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

			gui::register_menu("vision_editor", "Vision Editor", render_window);
		}
	};
}

REGISTER_COMPONENT(gui::vision_editor::component)
#endif
