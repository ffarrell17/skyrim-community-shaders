#include "Weather.h"
#include "Location.h"
#include "UI.h"

using namespace Helpers;


bool Weather::GetCurrentClimate(RE::TESClimate*& climate)
{
	if (RE::Sky* sky = RE::Sky::GetSingleton(); sky) {
		if (RE::TESClimate* currentClimate = sky->currentClimate; currentClimate) {
			climate = currentClimate;
			return true;
		}
	}
	return false;
}

bool Weather::GetWeatherTransition(float& t)
{
	if (const auto& sky = RE::Sky::GetSingleton(); sky) {
		if (Location::IsValidInterior()) {
			t = sky->lightingTransition == 0.00f ? 1.00f : sky->lightingTransition;
		} else {
			t = sky->currentWeatherPct;
		}
		return true;
	}
	return false;
}

/*bool Weather::GetCurrentWeather(RE::FormID& id)
{
	const auto& sky = RE::Sky::GetSingleton();
	const auto& player = RE::PlayerCharacter::GetSingleton();

	if (Location::IsValidInterior(player)) {
		if (sky->currentRoom) {
			const auto& room = sky->currentRoom.get();
			if (room) {
				const auto& roomRef = room->extraList.GetByType(RE::ExtraDataType::kRoomRefData);
				if (roomRef) {
					const auto& roomRefData = static_cast<RE::ExtraRoomRefData*>(roomRef)->data;
					if (roomRefData && roomRefData->lightingTemplate) {
						id = roomRefData->lightingTemplate->GetFormID();
						return true;
					}
				}
			}
		}
		id = player->parentCell->GetRuntimeData().lightingTemplate->GetFormID();
		return true;
	} else if (sky && sky->currentWeather) {
		id = sky->currentWeather->GetFormID();
		return true;
	}

	return false;
}*/

bool Weather::GetOutgoingWeather(RE::FormID& id)
{
	const auto& sky = RE::Sky::GetSingleton();

	if (Location::IsValidInterior()) {
		if (sky->previousRoom) {
			const auto& room = sky->previousRoom.get();
			if (room) {
				const auto& roomRef = room->extraList.GetByType(RE::ExtraDataType::kRoomRefData);
				if (roomRef) {
					const auto& roomRefData = static_cast<RE::ExtraRoomRefData*>(roomRef)->data;
					if (roomRefData && roomRefData->lightingTemplate) {
						id = roomRefData->lightingTemplate->GetFormID();
						return true;
					}
				}
			}
		}
		id = 0;
		return true;
	} else if (sky && sky->lastWeather) {
		id = sky->lastWeather->GetFormID();
		return true;
	}

	return false;
}

/* bool Weather::GetWeatherTransition(float& t)
{
	const auto& sky = RE::Sky::GetSingleton();

	if (sky) {
		t = sky->currentWeatherPct;
		return true;
	}

	return false;
}

*/bool Weather::GetCurrentWeather(RE::FormID& id)
{
	const auto& sky = RE::Sky::GetSingleton();

	if (sky && sky->currentWeather) {
		id = sky->currentWeather->GetFormID();
		return true;
	}

	return false;
} /*

bool Weather::GetOutgoingWeather(RE::FormID&& id)
{
	const auto& sky = RE::Sky::GetSingleton();

	if (sky && sky->lastWeather) {
		id = sky->lastWeather->GetFormID();
		return true;
	}

	return false;
}*/

bool Weather::GetSkyMode(RE::FormID& mode)
{
	const auto& sky = RE::Sky::GetSingleton();

	if (sky) {
		mode = sky->mode.underlying();
		return true;
	}

	return false;
}

int32_t Weather::GetClassification(RE::TESWeather* weather)
{
	typedef RE::TESWeather::WeatherDataFlag Flags;
	const auto& flags = weather->data.flags;

	if ((flags & Flags::kPleasant) != Flags::kNone)
		return 0;
	if ((flags & Flags::kCloudy) != Flags::kNone)
		return 1;
	if ((flags & Flags::kRainy) != Flags::kNone)
		return 2;
	if ((flags & Flags::kSnow) != Flags::kNone)
		return 3;

	return 0xFFFFFFFF;
}

bool Weather::GetCurrentWeatherClassification(int& c)
{
	const auto& sky = RE::Sky::GetSingleton();
	if (sky && sky->currentWeather) {
		c = GetClassification(sky->currentWeather);
		return true;
	}

	return false;
}

bool Weather::GetOutgoingWeatherClassification(int& c)
{
	const auto& sky = RE::Sky::GetSingleton();
	if (sky && sky->currentWeather) {
		c = GetClassification(sky->currentWeather);
		return true;
	}

	return false;
}

std::string Helpers::Weather::GetWeatherTransitionString()
{
	float weatherTrans;
	if (Helpers::Weather::GetWeatherTransition(weatherTrans)) {
		return std::to_string(weatherTrans);
	}
	return "-";
}

std::string Helpers::Weather::GetCurrentWeatherString()
{
	RE::FormID currentWeatherId;
	if (Helpers::Weather::GetCurrentWeather(currentWeatherId)) {
		return Helpers::UI::Uint32ToHexString(currentWeatherId);
	}
	return "-";
}

std::string Helpers::Weather::GetOutgoingWeatherString()
{
	RE::FormID outgoingWeatherId;
	if (Helpers::Weather::GetOutgoingWeather(outgoingWeatherId)) {
		return Helpers::UI::Uint32ToHexString(outgoingWeatherId);
	}
	return "-";
}
