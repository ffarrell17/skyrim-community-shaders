#include "GrassLighting.h"

#include "State.h"
#include "Util.h"

#include "Features/Clustered.h"
#include <Helpers/UI.h>

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
	GrassLighting::ConfigSettings,
	Glossiness,
	SpecularStrength,
	SubsurfaceScatteringAmount,
	EnableDirLightFix,
	EnablePointLights)

enum class GrassShaderTechniques
{
	RenderDepth = 8,
};

bool GrassLighting::ConfigSettings::DrawSettings(bool& featureEnabled, bool isConfigOverride)
{
	bool updated = false;

	featureEnabled = featureEnabled;

	if (ImGui::TreeNodeEx("Complex Grass", ImGuiTreeNodeFlags_DefaultOpen)) {
			
			
		ImGui::TextWrapped(
			"Specular highlights for complex grass.\n"
			"Functions the same as on other objects.");

		if (isConfigOverride) Helpers::UI::BeginOptionalSection<TODValue<float>>(Glossiness, 20.0f);
		updated = updated || Glossiness->DrawSliderScalar("Glossiness", ImGuiDataType_Float, 1.0f, 100.0f);
		if (isConfigOverride) Helpers::UI::EndOptionalSection(Glossiness);

		if (isConfigOverride) Helpers::UI::BeginOptionalSection<TODValue<float>>(SpecularStrength, 0.5f);
		updated = updated || SpecularStrength->DrawSliderScalar("Specular Strength", ImGuiDataType_Float, 0.0f, 1.0f);
		if (isConfigOverride) Helpers::UI::EndOptionalSection(SpecularStrength);
			
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Effects", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::TextWrapped(
			"Soft lighting controls how evenly lit an object is.\n"
			"Back lighting illuminates the back face of an object.\n"
			"Combined to model the transport of light through the surface.");
			
		if (isConfigOverride) Helpers::UI::BeginOptionalSection<TODValue<float>>(SubsurfaceScatteringAmount, 0.5f);
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
		updated = updated || SubsurfaceScatteringAmount->DrawSliderScalar("Subsurface Scattering Amount", ImGuiDataType_Float, 0.0f, 1.0f);
		if (isConfigOverride) Helpers::UI::EndOptionalSection(SubsurfaceScatteringAmount);

		ImGui::TreePop();
	}

	if (!isConfigOverride && ImGui::TreeNodeEx("Lights", ImGuiTreeNodeFlags_DefaultOpen)) {

		ImGui::TextWrapped("Fix for grass not being affected by sunlight scale.");
		updated = updated || ImGui::Checkbox("Enable Directional Light Fix", &EnableDirLightFix);

		ImGui::TextWrapped("Enables point lights on grass like other objects. Slightly impacts performance if there are a lot of lights.");

		updated = updated || ImGui::Checkbox("Enable Point Lights", &EnablePointLights);

		ImGui::TreePop();
	}

	return updated;
}

GrassLighting::ShaderSettings GrassLighting::ConfigSettings::ToShaderSettings()
{
	ShaderSettings settings;
	settings.Glossiness = Glossiness->Get();
	settings.SpecularStrength = SpecularStrength->Get();
	settings.SubsurfaceScatteringAmount = SubsurfaceScatteringAmount->Get();
	settings.EnableDirLightFix = (bool)EnableDirLightFix;
	settings.EnablePointLights = (bool)EnablePointLights;
	return settings;
}

void GrassLighting::ModifyGrass(const RE::BSShader*, const uint32_t descriptor)
{
	const auto technique = descriptor & 0b1111;
	if (technique != static_cast<uint32_t>(GrassShaderTechniques::RenderDepth)) {
		if (updatePerFrame) {
			PerFrame perFrameData{};
			ZeroMemory(&perFrameData, sizeof(perFrameData));

			auto& shaderState = RE::BSShaderManager::State::GetSingleton();
			RE::NiTransform& dalcTransform = shaderState.directionalAmbientTransform;

			Util::StoreTransform3x4NoScale(perFrameData.DirectionalAmbient, dalcTransform);

			auto accumulator = RE::BSGraphics::BSShaderAccumulator::GetCurrentAccumulator();
			auto& position = accumulator->GetRuntimeData().eyePosition;
			auto state = RE::BSGraphics::RendererShadowState::GetSingleton();

			RE::NiPoint3 eyePosition{};
			if (REL::Module::IsVR()) {
				// find center of eye position
				eyePosition = state->GetVRRuntimeData().posAdjust.getEye() + state->GetVRRuntimeData().posAdjust.getEye(1);
				eyePosition /= 2;
			} else
				eyePosition = state->GetRuntimeData().posAdjust.getEye();
			perFrameData.EyePosition.x = position.x - eyePosition.x;
			perFrameData.EyePosition.y = position.y - eyePosition.y;
			perFrameData.EyePosition.z = position.z - eyePosition.z;

			auto manager = RE::ImageSpaceManager::GetSingleton();
			perFrameData.SunlightScale = manager->data.baseData.hdr.sunlightScale;

			perFrameData.Settings = configSettings->ToShaderSettings();

			perFrame->Update(perFrameData);

			updatePerFrame = false;
		}

		Clustered::GetSingleton()->Bind(true);
		auto context = RE::BSGraphics::Renderer::GetSingleton()->GetRuntimeData().context;

		ID3D11Buffer* buffers[2];
		context->VSGetConstantBuffers(2, 1, buffers);  // buffers[0]
		buffers[1] = perFrame->CB();
		context->VSSetConstantBuffers(2, ARRAYSIZE(buffers), buffers);
		context->PSSetConstantBuffers(2, ARRAYSIZE(buffers), buffers);
	}
}

void GrassLighting::Draw(const RE::BSShader* shader, const uint32_t descriptor)
{
	switch (shader->shaderType.get()) {
	case RE::BSShader::Type::Grass:
		ModifyGrass(shader, descriptor);
		break;
	}
}

void GrassLighting::SetupResources()
{
	perFrame = new ConstantBuffer(ConstantBufferDesc<PerFrame>());
}

void GrassLighting::Reset()
{
	updatePerFrame = true;
}

std::shared_ptr<FeatureSettings> GrassLighting::CreateConfig()
{
	return std::make_shared<GrassLighting::ConfigSettings>();
}

std::shared_ptr<FeatureSettings> GrassLighting::ParseConfig(json& o_json)
{
	auto glConfig = std::dynamic_pointer_cast<GrassLighting::ConfigSettings>(CreateConfig());
	*glConfig = o_json;
	return glConfig;
}

void GrassLighting::SaveConfig(json& o_json, std::shared_ptr<FeatureSettings> config)
{
	auto glConfig = std::dynamic_pointer_cast<GrassLighting::ConfigSettings>(config);
	if (glConfig) {
		o_json = *glConfig;
	}
}

void GrassLighting::ApplyConfig(std::shared_ptr<FeatureSettings> config)
{
	configSettings = std::dynamic_pointer_cast<GrassLighting::ConfigSettings>(config);
}