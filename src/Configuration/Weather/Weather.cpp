#include "Weather.h"

using namespace Configuration;

Weather::Weather(bool defaultWeather) :
	FeatureSettings(defaultWeather ? FeatureSettingsType::WeatherOverrideDefault : FeatureSettingsType::WeatherOverride)
{
	_isDefaultWeather = defaultWeather;
	if (_isDefaultWeather) {
		Name = "Default Weather";
	} else {
		Name = "New Weather";
		Ids.push_back(0);
	}
}

Weather::Weather(json& o_json, std::map<Feature*, json> featureConfigMap, bool defaultWeather) :
	Weather(defaultWeather)
{
	Load(o_json, featureConfigMap);
}

void Configuration::Weather::Load(json& o_json, std::map<Feature*, json>& featureConfigMap)
{
	Name = o_json["Name"];

	Ids.clear();
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

	Load(featureConfigMap);
}

void Configuration::Weather::Load(std::map<Feature*, json>& featureConfigMap)
{
	std::map<Feature*, json> weatherFeatureConfigMap;
	for (Feature* feature : Feature::GetFeatureList()) {
		json weatherSettings;
		if (!featureConfigMap[feature].is_null() && !featureConfigMap[feature][Name].is_null()) {
			weatherSettings = featureConfigMap[feature][Name];
		}
		weatherFeatureConfigMap.insert(std::make_pair(feature, weatherSettings));
	}

	FeatureSettings.Load(weatherFeatureConfigMap);
}

void Weather::Save(json& o_json, std::map<Feature*, json>& featureConfigMap)
{
	o_json["Name"] = Name;
	o_json["Ids"] = Ids;

	Save(featureConfigMap);
}

void Weather::Save(std::map<Feature*, json>& featureConfigMap)
{
	std::map<Feature*, json> weatherFeatureConfigMap;
	for (Feature* feature : Feature::GetFeatureList()) {
		weatherFeatureConfigMap.insert(std::make_pair(feature, json()));
	}

	FeatureSettings.Save(weatherFeatureConfigMap);

	for (Feature* feature : Feature::GetFeatureList()) {
		featureConfigMap[feature][Name] = weatherFeatureConfigMap[feature];
	}
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

	if (!_isDefaultWeather) {
		_updated = ImGui::InputText("Name", &Name);
	}

	if (!_isDefaultWeather) {
		DrawIdList();
		ImGui::Spacing();
		ImGui::Spacing();
	}

	FeatureSettings.Draw();
}

void Weather::DrawIdList()
{
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
		if (!matchingWeather)
			ImGui::BeginDisabled();
		if (ImGui::Button("Apply") && matchingWeather) {
			const auto& sky = RE::Sky::GetSingleton();
			if (sky) {
				sky->SetWeather(matchingWeather, true, false);
			}
		}
		if (!matchingWeather)
			ImGui::EndDisabled();

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
}

bool Configuration::Weather::HasUpdated()
{
	return _updated || FeatureSettings.HasUpdated();
}

bool Configuration::Weather::HasUpdatedIds()
{
	return _updatedIds;
}

void Configuration::Weather::ResetUpdatedState()
{
	_updated = _updatedIds = false;
	FeatureSettings.ResetUpdatedState();
}
