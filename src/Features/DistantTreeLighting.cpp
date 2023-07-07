#include "DistantTreeLighting.h"
#include "..\Configuration\ConfigurationManager.h"

#include "State.h"
#include "Util.h"
#include "Helpers/UI.h"

bool DistantTreeLighting::ConfigSettings::DrawSettings(bool& featureEnabled, bool isConfigOverride, std::shared_ptr<FeatureSettings> defaultSettings)
{
	bool updated = false;

	featureEnabled = featureEnabled;  //to hide warning

	if (!isConfigOverride && ImGui::TreeNodeEx("Complex Tree LOD", ImGuiTreeNodeFlags_DefaultOpen)) {
			
		ImGui::TextWrapped(
			"Enables advanced lighting simulation on tree LOD.\n"
			"Requires DynDOLOD.\n"
			"See https://dyndolod.info/ for more information.");
		updated = updated || ImGui::Checkbox("Enable Complex Tree LOD", (bool*)&EnableComplexTreeLOD);

		ImGui::TreePop();
	}

	if (!isConfigOverride && ImGui::TreeNodeEx("Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
			
		ImGui::TextWrapped("Fix for trees not being affected by sunlight scale.");
		updated = updated || ImGui::Checkbox("Enable Directional Light Fix", (bool*)&EnableDirLightFix);

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Effects", ImGuiTreeNodeFlags_DefaultOpen)) {
			
		if (isConfigOverride) Helpers::UI::BeginOptionalSection<TODValue<float>>(SubsurfaceScatteringAmount, 0.5f);

		ImGui::TextWrapped(
			"Soft lighting controls how evenly lit an object is.\n"
			"Back lighting illuminates the back face of an object.\n"
			"Combined to model the transport of light through the surface.");
		updated = updated || SubsurfaceScatteringAmount->DrawSliderScalar("Subsurface Scattering Amount", ImGuiDataType_Float, 0.0f, 1.0f);

		if (isConfigOverride) Helpers::UI::EndOptionalSection(SubsurfaceScatteringAmount);

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Vanilla", ImGuiTreeNodeFlags_DefaultOpen)) {

		if (isConfigOverride) Helpers::UI::BeginOptionalSection<TODValue<float>>(FogDimmerAmount, 1.0f);

		ImGui::TextWrapped("Darkens lighting relative fog strength.");

			

		updated = updated || FogDimmerAmount->DrawSliderScalar("Fog Dimmer Amount", ImGuiDataType_Float, 0.0f, 1.0f);

		if (isConfigOverride) Helpers::UI::EndOptionalSection(FogDimmerAmount);

		ImGui::TreePop();
	}

	return updated;
}

DistantTreeLighting::ShaderSettings DistantTreeLighting::ConfigSettings::ToShaderSettings()
{
	ShaderSettings settings;
	settings.EnableComplexTreeLOD = EnableComplexTreeLOD;
	settings.EnableDirLightFix = EnableDirLightFix;
	settings.FogDimmerAmount = FogDimmerAmount->Get();
	settings.SubsurfaceScatteringAmount = SubsurfaceScatteringAmount->Get();
	return settings;
}

enum class DistantTreeShaderTechniques
{
	DistantTreeBlock = 0,
	Depth = 1,
};

void DistantTreeLighting::ModifyDistantTree(const RE::BSShader*, const uint32_t descriptor)
{
	if (auto player = RE::PlayerCharacter::GetSingleton()) {
		if (auto worldSpace = player->GetWorldspace()) {
			if (lastWorldSpace != worldSpace) {
				lastWorldSpace = worldSpace;
				if (auto name = worldSpace->GetFormEditorID()) {
					CSimpleIniA ini;
					ini.SetUnicode();
					auto path = std::format("Data\\Textures\\Terrain\\{}\\Trees\\{}TreeLOD.ini", name, name);
					ini.LoadFile(path.c_str());
					complexAtlasTexture = ini.GetBoolValue("Information", "ComplexAtlasTexture", false);
				} else {
					complexAtlasTexture = false;
				}
			}
		}
	}

	const auto technique = descriptor & 1;
	if (technique != static_cast<uint32_t>(DistantTreeShaderTechniques::Depth)) {
		PerPass perPassData{};
		ZeroMemory(&perPassData, sizeof(perPassData));

		auto& shaderState = RE::BSShaderManager::State::GetSingleton();
		RE::NiTransform& dalcTransform = shaderState.directionalAmbientTransform;

		Util::StoreTransform3x4NoScale(perPassData.DirectionalAmbient, dalcTransform);

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
		perPassData.EyePosition.x = position.x - eyePosition.x;
		perPassData.EyePosition.y = position.y - eyePosition.y;
		perPassData.EyePosition.z = position.z - eyePosition.z;

		auto sunLight = skyrim_cast<RE::NiDirectionalLight*>(accumulator->GetRuntimeData().activeShadowSceneNode->GetRuntimeData().sunLight->light.get());
		if (sunLight) {
			auto imageSpaceManager = RE::ImageSpaceManager::GetSingleton();

			perPassData.DirLightScale = imageSpaceManager->data.baseData.hdr.sunlightScale * sunLight->GetLightRuntimeData().fade;

			perPassData.DirLightColor.x = sunLight->GetLightRuntimeData().diffuse.red;
			perPassData.DirLightColor.y = sunLight->GetLightRuntimeData().diffuse.green;
			perPassData.DirLightColor.z = sunLight->GetLightRuntimeData().diffuse.blue;

			auto& direction = sunLight->GetWorldDirection();
			perPassData.DirLightDirection.x = direction.x;
			perPassData.DirLightDirection.y = direction.y;
			perPassData.DirLightDirection.z = direction.z;
		}

		perPassData.ComplexAtlasTexture = complexAtlasTexture;

		perPassData.Settings = configSettings->ToShaderSettings();

		perPass->Update(perPassData);

		auto context = RE::BSGraphics::Renderer::GetSingleton()->GetRuntimeData().context;

		ID3D11Buffer* buffers[2];
		context->VSGetConstantBuffers(2, 1, buffers);  // buffers[0]
		buffers[1] = perPass->CB();
		context->VSSetConstantBuffers(2, ARRAYSIZE(buffers), buffers);
		context->PSSetConstantBuffers(2, ARRAYSIZE(buffers), buffers);

		auto renderer = RE::BSGraphics::Renderer::GetSingleton();
		ID3D11ShaderResourceView* views[1]{};
		views[0] = renderer->GetRuntimeData().renderTargets[RE::RENDER_TARGET::kSHADOW_MASK].SRV;
		context->PSSetShaderResources(17, ARRAYSIZE(views), views);
	}
}

void DistantTreeLighting::Draw(const RE::BSShader* shader, const uint32_t descriptor)
{
	switch (shader->shaderType.get()) {
	case RE::BSShader::Type::DistantTree:
		ModifyDistantTree(shader, descriptor);
		break;
	}
}

void DistantTreeLighting::SetupResources()
{
	perPass = new ConstantBuffer(ConstantBufferDesc<PerPass>());
}

std::shared_ptr<FeatureSettings> DistantTreeLighting::CreateConfig()
{
	return std::make_shared<DistantTreeLighting::ConfigSettings>();
}

void DistantTreeLighting::ApplyConfig(std::shared_ptr<FeatureSettings> config)
{
	configSettings = std::dynamic_pointer_cast<DistantTreeLighting::ConfigSettings>(config);
}