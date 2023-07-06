#pragma once
#include "PCH.h"
#include "ShaderSettings.h"
#include "Weather.h"

namespace Configuration
{
	struct Location
	{
		std::string Name;
		std::vector<RE::FormID> Ids;
		//ShaderSettings Settings;
		std::vector<std::shared_ptr<Weather>> Weathers;

		Location(json& json);
		void Save(json& o_json);
		void Draw();
	};
}