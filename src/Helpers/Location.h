#pragma once
#include "PCH.h"

namespace Helpers
{
	class Location
	{
	public:

		static bool GetCurrentLocationID(RE::FormID& id);
		static bool GetWorldSpaceID(RE::FormID& id);
		static bool IsValidInterior();
		static bool IsValidInterior(const RE::PlayerCharacter* player);

		static std::string GetWorldSpaceIDString();
	};
}