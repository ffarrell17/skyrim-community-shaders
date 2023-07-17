#pragma once
#include "PCH.h"
#include "imgui_internal.h"

namespace Helpers
{
	class UI
	{
	public:

		template <typename T>
		static void BeginOptionalSection(std::optional<T>& option, T defaultVal)
		{
			ImGui::Unindent();

			bool notNull = option.has_value();
			if (CustomCheckbox("Override", &notNull)) {
				if (!notNull)
					option = std::nullopt;
				else {
					option = defaultVal;
				}
			}

			ImGui::Indent();

			if (!notNull)
				ImGui::BeginDisabled();
		}

		template <typename T1, typename T2>
		static void BeginOptionalSection(std::optional<T1>& option1, T1 defaultVal1, std::optional<T2>& option2, T2 defaultVal2)
		{
			ImGui::Unindent();

			bool option1IsNull = !option1.has_value();
			bool option2IsNull = !option2.has_value();

			bool eitherIsNull = option1IsNull || option2IsNull;
			bool bothNotNull = !eitherIsNull;

			if (CustomCheckbox("Override", &bothNotNull)) {
				if (bothNotNull) {
					option1 = defaultVal1;
					option2 = defaultVal2;
				} else {
					option1 = std::nullopt;
					option2 = std::nullopt;
				}
			}

			ImGui::Indent();

			if (eitherIsNull)
				ImGui::BeginDisabled();
		}

		template <typename T>
		static void EndOptionalSection(std::optional<T>& option)
		{
			bool notNull = option.has_value();

			if (!notNull)
				ImGui::EndDisabled();
		}

		template <typename T1, typename T2>
		static void EndOptionalSection(std::optional<T1>& option1, std::optional<T1>& option2)
		{
			bool option1IsNull = option1.has_value();
			bool option2IsNull = option2.has_value();

			bool eitherIsNull = option1IsNull || option2IsNull;

			if (eitherIsNull)
				ImGui::EndDisabled();
		}

		static bool CustomCheckbox(const char* label, bool* value, float boxSizeScale = 1.0f, float checkPadding = 2.0f)
		{
			ImVec2 size(ImGui::GetFontSize() * boxSizeScale, ImGui::GetFontSize() * boxSizeScale);

			ImGuiStyle& style = ImGui::GetStyle();
			ImVec2 pos = ImGui::GetCursorScreenPos();

			bool ret = false;

			ImGui::InvisibleButton(label, size);
			if (ImGui::IsItemClicked()) {
				*value = !*value;
				ret = true;
			}
			bool hovered = ImGui::IsItemHovered();
			bool pressed = ImGui::IsItemActive();

			ImDrawList* drawList = ImGui::GetWindowDrawList();
			drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), 
				ImGui::GetColorU32((pressed && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), style.FrameRounding);

			if (*value) {
				const ImVec2 checkPos(pos.x + checkPadding, pos.y + checkPadding);
				drawList->AddRectFilled(checkPos, ImVec2(checkPos.x + size.x - 2.0f * checkPadding, checkPos.y + size.y - 2.0f * checkPadding), ImGui::GetColorU32(ImGuiCol_CheckMark));
			}

			ImGui::SameLine();

			ImGui::Text("%s", label);

			return ret;
		}

		

		static std::string Uint32ToHexString(uint32_t value)
		{
			// Convert to string
			std::stringstream stream;
			stream << std::hex << value;
			std::string hexString = stream.str();

			// Make 8 digits
			hexString = std::string(8 - hexString.length(), '0') + hexString;

			// To Upper
#pragma warning(suppress: 4244)  //TODO: find a better way
			std::transform(hexString.begin(), hexString.end(), hexString.begin(), ::toupper);

			return hexString;
		}

		static void ToggleButton(const char* str_left, bool* v)
		{
			ImVec4* colors = ImGui::GetStyle().Colors;
			ImVec2 p = ImGui::GetCursorScreenPos();
			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			float height = ImGui::GetFrameHeight();
			float width = height * 1.55f;
			float radius = height * 0.50f;

			ImGui::InvisibleButton(str_left, ImVec2(width, height));
			if (ImGui::IsItemClicked())
				*v = !*v;
			//ImGuiContext& gg = *GImGui;
			//float ANIM_SPEED = 0.085f;
			//if (gg.LastActiveId == gg.CurrentWindow->GetID(str_left))  // && g.LastActiveIdTimer < ANIM_SPEED)
				//float t_anim = ImSaturate(gg.LastActiveIdTimer / ANIM_SPEED);
			if (ImGui::IsItemHovered())
				draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_ButtonActive] : ImVec4(0.78f, 0.78f, 0.78f, 1.0f)), height * 0.5f);
			else
				draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_Button] : ImVec4(0.85f, 0.85f, 0.85f, 1.0f)), height * 0.50f);
			draw_list->AddCircleFilled(ImVec2(p.x + radius + (*v ? 1 : 0) * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
		}
	};
}