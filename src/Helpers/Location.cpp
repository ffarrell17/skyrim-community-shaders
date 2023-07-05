#include "Location.h"
#include "UI.h"
using namespace Helpers;

bool Location::GetCurrentLocationID(RE::FormID& id)
{
	if (const auto& player = RE::PlayerCharacter::GetSingleton(); player) {
/* #ifndef SKYRIMVR

		if (const auto& currentLocation = player->GetPlayerRuntimeData().currentLocation; currentLocation) {
			id = currentLocation->GetFormID();
			return true;
		}
#else
		if (const auto& currentLocation = player->GetCurrentLocation(); currentLocation) {
			id = currentLocation->GetFormID();
			return true;
		}
#endif*/

		if (const auto& currentLocation = player->GetCurrentLocation(); currentLocation) {
			id = currentLocation->GetFormID();
			return true;
		}
	}

	return false;
}

bool Location::GetWorldSpaceID(RE::FormID& id)
{
	if (const auto& player = RE::PlayerCharacter::GetSingleton(); player) {
		if (player->GetWorldspace()) {
			id = player->GetWorldspace()->GetFormID();
			return true;
		}
	}

	return false;
}

bool Location::IsValidInterior()
{
	const auto& player = RE::PlayerCharacter::GetSingleton();
	return IsValidInterior(player);
}

bool Location::IsValidInterior(const RE::PlayerCharacter* player)
{
	return (player && player->parentCell && player->parentCell->IsInteriorCell() && player->parentCell->GetRuntimeData().lightingTemplate);
}

std::string Helpers::Location::GetWorldSpaceIDString()
{
	RE::FormID locId;
	if (Helpers::Location::GetWorldSpaceID(locId)) {
		return Helpers::UI::Uint32ToHexString(locId);
	}
	return "-";
}
