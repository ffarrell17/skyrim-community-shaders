#include "Weather.h"
#include "imgui_stdlib.h"

using namespace Configuration;

Weather::Weather()
{
	Name = "New Weather";
	Ids.push_back(0);
}

Weather::Weather(json& o_json)
{
	Name = o_json["Name"];

	auto ids = o_json["Ids"];
	for (const auto& el : ids) {
		uint32_t id;
		if (el.is_string()) {
			std::string elStr = el.get<std::string>();

			if (elStr.size() > 1 && elStr[0] == 'x')
				elStr = elStr.substr(1);

			try {
				id = std::stoul(elStr, nullptr, 16);
			} catch (const std::exception& e) {
				logger::error("Failed to parse weather id [{}]. Exception [{}]", el.get<std::string>(), e.what());
				continue;
			}
		} else if (el.is_number_unsigned()) {
			id = el.get<uint32_t>();
		} else {
			continue;  // log
		}

		Ids.push_back(id);
	}

	Settings.Load(o_json["Settings"], false);
}

void Weather::Save(json& o_json)
{
	o_json["Name"] = Name;
	o_json["Ids"] = Ids;
	Settings.Save(o_json["Settings"], false);
}

bool TryGetHex(const std::string& input, int& val)
{
	std::istringstream iss(input);
	iss >> std::hex >> val;
	return !iss.fail() && iss.eof();
}

uint32_t HexStringToUint32(const std::string& hexString)
{
	uint32_t result = 0;
	std::istringstream iss(hexString);
	iss >> std::hex >> result;

	if (iss.fail()) {
		// Invalid hex string, handle the error
		// For example, you can return a default value or throw an exception
		throw std::invalid_argument("Invalid hexadecimal string");
	}

	return result;
}

void Weather::Draw()
{
	_updated = _updatedIds = false;

	_updated = ImGui::InputText("Name", &Name);

	for (int i = 0; i < Ids.size(); ++i) {
		ImGui::PushID(i);

		auto label = i == 0 ? "Ids" : "";
		
		auto* matchingWeather = RE::TESForm::LookupByID<RE::TESWeather>(Ids[i]);

		// Hex string - Uppercase and 8 digits
		std::string hexString = Helpers::UI::Uint32ToHexString(Ids[i]);

		char buffer[9];
		size_t bufferSize = sizeof(buffer);
		strncpy_s(buffer, bufferSize, hexString.c_str(), bufferSize - 1);
		buffer[bufferSize - 1] = '\0'; 

		// TODO: Change input colour to indicate weather found
		if (ImGui::InputText(label, buffer, sizeof(buffer), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase)) {
			try {
				Ids[i] = HexStringToUint32(buffer);
				_updated = _updatedIds = true;
			} catch (const std::exception&) {
				// Handle the exception
			}
		}

		ImGui::SameLine();
		if (!matchingWeather) ImGui::BeginDisabled();
		if (ImGui::Button("Apply") && matchingWeather)
		{
			const auto& sky = RE::Sky::GetSingleton();
			if (sky) {
				sky->SetWeather(matchingWeather, true, false);
			}
		}
		if (!matchingWeather) ImGui::EndDisabled();


		// Button to remove the ID only if there's more than one ID
		if (Ids.size() > 1) {
			ImGui::SameLine();
			if (ImGui::Button(("Remove##" + std::to_string(i)).c_str())) {
				Ids.erase(Ids.begin() + i);
				ImGui::PopID(); 
				_updated = _updatedIds = true;
				break; 
			}
		}

		ImGui::PopID();
	}

	
	if (ImGui::Button("Add Id")) {
		Ids.push_back(0); 
		_updated = _updatedIds = true;
	}

	ImGui::Spacing();
	ImGui::Spacing();

	Settings.Draw("weatherFeatures", true, true);
}

bool Configuration::Weather::HasUpdated()
{
	return _updated || Settings.HasUpdated();
}

bool Configuration::Weather::HasUpdatedIds()
{
	return _updatedIds;
}

void Configuration::Weather::ResetUpdatedState()
{
	_updated = _updatedIds = false;
	Settings.ResetUpdatedState();
}
