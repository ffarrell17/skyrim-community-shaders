#pragma once
#include "PCH.h"

namespace Helpers
{
	class Time
	{
	public:
		static bool GetDateTime(tm& time);
		static bool GetTime(tm& time);
		static int TimeCmp(const tm& tm1, const tm& tm2);
		static int TimeToMinutes(const tm& time);
		static void TimeToString(const tm& time, char* buffer, std::size_t bufferSize);

	private:

		static int GetMinutes(RE::Calendar* calendar);
	};
}


using json = nlohmann::json;

namespace nlohmann
{	
	template <typename T>
	struct adl_serializer<std::optional<T>>
	{
		static void to_json(json& j, const std::optional<T>& opt)
		{
			if (opt.has_value()) {

				j = opt.value();
				 
			} else {
				j = nullptr;
			}
		}

		static void from_json(const json& j, std::optional<T>& opt)
		{
			if (j.is_null()) {
				opt = std::nullopt;
			} else {
				opt = j.get<T>();
			}
		}
	};

}  // namespace nlohmann
