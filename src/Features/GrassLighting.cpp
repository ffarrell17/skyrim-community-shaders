#include "GrassLighting.h"

#include "State.h"
#include "Util.h"
#include "Helpers/Time.h"

#include "Features/Clustered.h"
#include <Helpers/UI.h>

enum class GrassShaderTechniques
{
	RenderDepth = 8,
};

void GrassLightingSettings::Draw()
{
	if (ImGui::TreeNodeEx("Complex Grass", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::TextWrapped(
			"Specular highlights for complex grass.\n"
			"Functions the same as on other objects.");

		Glossiness.DrawSlider("Glossiness", 1.0f, 100.0f);

		SpecularStrength.DrawSlider("Specular Strength", 0.0f, 1.0f);

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Effects", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::TextWrapped(
			"Soft lighting controls how evenly lit an object is.\n"
			"Back lighting illuminates the back face of an object.\n"
			"Combined to model the transport of light through the surface.");

		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
		SubsurfaceScatteringAmount.DrawSlider("Subsurface Scattering Amount", 0.0f, 1.0f);

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::TextWrapped("Fix for grass not being affected by sunlight scale.");
		EnableDirLightFix.DrawCheckbox("Enable Directional Light Fix");

		ImGui::TextWrapped("Enables point lights on grass like other objects. Slightly impacts performance if there are a lot of lights.");

		EnablePointLights.DrawCheckbox("Enable Point Lights");

		ImGui::TreePop();
	}
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

			perFrameData.Glossiness = settings.Glossiness.Value;
			perFrameData.SpecularStrength = settings.SpecularStrength.Value;
			perFrameData.SubsurfaceScatteringAmount = settings.SubsurfaceScatteringAmount.Value;
			perFrameData.EnableDirLightFix = settings.EnableDirLightFix.Value;
			perFrameData.EnablePointLights = settings.EnablePointLights.Value;

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