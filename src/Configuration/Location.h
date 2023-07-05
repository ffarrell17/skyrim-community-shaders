#pragma once
#include "PCH.h"
#include "Weather.h"
#include "ShaderSettings.h"

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