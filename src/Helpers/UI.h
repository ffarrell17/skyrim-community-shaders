#pragma once
#include "PCH.h"

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

		template <typename T>
		static void EndOptionalSection(std::optional<T>& option)
		{
			bool notNull = option.has_value();

			if (!notNull)
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
	};
}