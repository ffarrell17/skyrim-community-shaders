#include "Location.h"


Location::Location(json& json)
{
	Name = json["Name"];

	auto ids = json["Ids"];
	for (const auto& el : ids) {
		uint32_t id = el;
		Ids.push_back(id);
	}

	//Settings.Load(json["Settings"]);

	auto weatherArray = json["Weathers"];
	for (auto& el : weatherArray) {
		Weathers.push_back(std::make_shared<Weather>(el));
	}
}

void Location::Save(json& o_json)
{
	o_json["Name"] = Name;
	o_json["Ids"] = Ids;
	//Settings.Save(o_json["Settings"]);

	if (!Weathers.empty()) {
		for (auto& weather : Weathers) {
			json weatherJson;
			weather->Save(weatherJson);
			o_json["Weathers"].push_back(weatherJson);
		}
	}
}

void Location::Draw()
{
	char inputBuffer[51];                                      // Character array to hold the input (including null-terminator)
	strcpy_s(inputBuffer, sizeof(inputBuffer), Name.c_str());  // Copy the existing string into the input buffer
	ImGui::InputText("Name", inputBuffer, sizeof(inputBuffer));
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		Name = inputBuffer;  // Update the std::string with the new value
	}

	// Ids go here

	//Settings.Draw();

	// Weathers go here
}