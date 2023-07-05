#pragma once
#include "PCH.h"
#include "ShaderSettings.h"

namespace Configuration
{
	struct Weather
	{
		std::string Name;
		std::vector<RE::FormID> Ids;
		ShaderSettings Settings;

		Weather();

		Weather(json& o_json);

		void Save(json& o_json);

		void Draw();

		bool HasUpdated();
		bool HasUpdatedIds();
		void ResetUpdatedState();

	private:
		
		bool _updated = false;		// All updates
		bool _updatedIds = false;	// Ids changed updates only
	};
}