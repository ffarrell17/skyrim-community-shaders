#pragma once
#include "TODInfo.h"
#include "Helpers/UI.h"
#include "Helpers/Math.h"

namespace Configuration
{

	template <typename T>
	struct TODValue
	{
		T Dawn;  // sunrise starting
		T Sunrise;
		T Day;
		T Sunset;
		T Dusk;  // sunset finished
		T Night;
		T InteriorDay;
		T InteriorNight;


		TODValue()
		{ }

		TODValue(T defaultVal)
		{
			SetAll(defaultVal);
		}

		template <typename U = T>  // Only if T != double
		TODValue(typename std::enable_if<!std::is_same<U, double>::value, double>::type defaultVal)
		{
			SetAll(static_cast<T>(defaultVal));
		}

		template <typename U = T>  // Only if T != int
		TODValue(typename std::enable_if<!std::is_same<U, int>::value, int>::type defaultVal)
		{
			SetAll(static_cast<T>(defaultVal));
		}

		~TODValue()
		{ }

		TODValue& operator=(const T& value)
		{
			SetAll(value);
			return *this;
		}

		bool operator==(const TODValue<T>& other) const
		{
			return Get() == other.Get();
		}

		bool operator<(const TODValue<T>& other) const
		{
			return Get() < other.Get();
		}

		bool operator>(const TODValue<T>& other) const
		{
			return Get() < other.Get();
		}



		/* TODValue<T> operator*(double f) const
		{
			return TODValue<T>(static_cast<T>(Get() * f));
		}

		TODValue<T> operator*(T f) const
		{
			return TODValue<T>(static_cast<T>(Get() * f));
		}

		TODValue<T> operator*(const TODValue<T>& other) const
		{
			return TODValue<T>(Get() * other.Get());
		}

		TODValue<T> operator/(const TODValue<T>& other) const
		{
			if (other.Get() == 0) {
				throw std::runtime_error("Division by zero!");
			}
			return TODValue<T>(Get() / other.Get());
		}

		TODValue<T> operator-(const TODValue<T>& other) const
		{
			return TODValue<T>(Get() - other.Get());
		}

		TODValue<T> operator+(const TODValue<T>& other) const
		{
			return TODValue<T>(Get() + other.Get());
		}*/

		double operator*(double f) const
		{
			return Get() * f;
		}

		template <typename U = T>  // Only if T != double
		double operator*(typename std::enable_if<!std::is_same<U, double>::value, double>::type f) const
		{
			return Get() * f;
		}

		double operator*(const TODValue<T>& other) const
		{
			return Get() * other.Get();
		}

		double operator/(const TODValue<T>& other) const
		{
			if (other.Get() == 0) {
				throw std::runtime_error("Division by zero!");
			}
			return Get() / other.Get();
		}

		T operator-(const TODValue<T>& other) const
		{
			return Get() - other.Get();
		}

		T operator+(const TODValue<T>& other) const
		{
			return Get() + other.Get();
		}

		void SetAll(T val)
		{
			Dawn = Sunrise = Day = Sunset = Dusk = Night = InteriorDay = InteriorNight = val;
			_drawAllVals = false;
		}

		bool AllEqual() const
		{
			return (Dawn == Sunrise && Sunrise == Day && Day == Sunset && Sunset == Dusk && Dusk == Night && Night == InteriorDay && InteriorDay == InteriorNight);
		}

		T Get() const
		{
			if (AllEqual())
				return Day;

			const auto& todInfo = TODInfo::GetSingleton();

			if (!todInfo.Valid)
				return Day;

			try {
				if (todInfo.Exterior) {
					switch (todInfo.TimePeriodType) {
					case Configuration::TODInfo::TimePeriod::NightToDawn:
						return Helpers::Math::Lerp(Night, Dawn, todInfo.TimePeriodPercentage);

					case Configuration::TODInfo::TimePeriod::DawnToSunrise:
						return Helpers::Math::Lerp(Dawn, Sunrise, todInfo.TimePeriodPercentage);

					case Configuration::TODInfo::TimePeriod::SunriseToDay:
						return Helpers::Math::Lerp(Sunrise, Day, todInfo.TimePeriodPercentage);

					case Configuration::TODInfo::TimePeriod::DayToSunset:
						return Helpers::Math::Lerp(Day, Sunset, todInfo.TimePeriodPercentage);

					case Configuration::TODInfo::TimePeriod::SunsetToDusk:
						return Helpers::Math::Lerp(Sunset, Dusk, todInfo.TimePeriodPercentage);

					case Configuration::TODInfo::TimePeriod::DuskToNight:
						return Helpers::Math::Lerp(Dusk, Night, todInfo.TimePeriodPercentage);

					default:
						return Day;
					}
				} else {
					switch (todInfo.TimePeriodType) {
					case Configuration::TODInfo::TimePeriod::DuskToNight:
					case Configuration::TODInfo::TimePeriod::NightToDawn:
						return InteriorNight;
					default:
						return InteriorDay;
					}
				}
			} catch (std::exception& e) {
				logger::error("Error getting TOD value for of type [{}]. Error: {}", typeid(T).name(), e.what());
				return Day;
			}
		}

		bool DrawSliderScalar(std::string label, ImGuiDataType_ drawType, T min, T max, const char* format = NULL)
		{
			return DrawSliderScalar(label, drawType, TODValue<T>(min), TODValue<T>(max), format);
		}

		bool DrawSliderScalar(std::string label, ImGuiDataType_ drawType, TODValue<T> min, T max, const char* format = NULL)
		{
			return DrawSliderScalar(label, drawType, min, TODValue<T>(max), format);
		}

		bool DrawSliderScalar(std::string label, ImGuiDataType_ drawType, T min, TODValue<T> max, const char* format = NULL)
		{
			return DrawSliderScalar(label, drawType, TODValue<T>(min), max, format);
		}
		// Allowing other TODValues to mark the range per TOD
		bool DrawSliderScalar(std::string label, ImGuiDataType_ drawType, TODValue<T> min, TODValue<T> max, const char* format = NULL)
		{
			Helpers::UI::CustomCheckbox("TOD Value", &_drawAllVals);

			bool updated = false;

			if (_drawAllVals) {
				ImGui::SameLine();
				ImGui::Dummy(ImVec2(20.0f, 0.0f));
				ImGui::SameLine();

				ImGui::SetNextItemWidth(200.0f);
				std::string str = std::to_string(Get());
				ImGui::InputText("Current Value", const_cast<char*>(str.c_str()), str.size() + 1, ImGuiInputTextFlags_ReadOnly);

				updated = updated || ImGui::SliderScalar((label + " Dawn").c_str(), drawType, static_cast<void*>(&Dawn), &min.Dawn, &max.Dawn, format);
				updated = updated || ImGui::SliderScalar((label + " Sunrise").c_str(), drawType, static_cast<void*>(&Sunrise), &min.Sunrise, &max.Sunrise, format);
				updated = updated || ImGui::SliderScalar((label + " Day").c_str(), drawType, static_cast<void*>(&Day), &min.Day, &max.Day, format);
				updated = updated || ImGui::SliderScalar((label + " Sunset").c_str(), drawType, static_cast<void*>(&Sunset), &min.Sunset, &max.Sunset, format);
				updated = updated || ImGui::SliderScalar((label + " Dusk").c_str(), drawType, static_cast<void*>(&Dusk), &min.Dusk, &max.Dusk, format);
				updated = updated || ImGui::SliderScalar((label + " Night").c_str(), drawType, static_cast<void*>(&Night), &min.Night, &max.Night, format);
				updated = updated || ImGui::SliderScalar((label + " InteriorDay").c_str(), drawType, static_cast<void*>(&InteriorDay), &min.InteriorDay, &max.InteriorDay, format);
				updated = updated || ImGui::SliderScalar((label + " InteriorNight").c_str(), drawType, static_cast<void*>(&InteriorNight), &min.InteriorNight, &max.InteriorNight, format);
			} else {
				if (ImGui::SliderScalar(label.c_str(), drawType, static_cast<void*>(&Day), &min, &max, format)) {
					SetAll(Day);
					updated = true;
				}
			}

			return updated;
		}

		void DrawCheckBox(std::string label)
		{
			Helpers::UI::CustomCheckbox("TOD Value", &_drawAllVals);

			bool updated = false;

			if (_drawAllVals) {
				// Current val goes here

				updated = updated || ImGui::Checkbox(label + " Dawn", &Dawn);
				updated = updated || ImGui::Checkbox(label + " Sunrise", &Sunrise);
				updated = updated || ImGui::Checkbox(label + " Day", &Day);
				updated = updated || ImGui::Checkbox(label + " Sunset", &Sunset);
				updated = updated || ImGui::Checkbox(label + " Dusk", &Dusk);
				updated = updated || ImGui::Checkbox(label + " Night", &Night);
				updated = updated || ImGui::Checkbox(label + " InteriorDay", &InteriorDay);
				updated = updated || ImGui::Checkbox(label + " InteriorNight", &InteriorNight);

			} else {
				if (ImGui::Checkbox(label, &Day)) {
					SetAll(Day);
					updated = true;
				}
			}
		}

		static bool IsTODValue(const nlohmann::json& jsonValue)
		{
			// Check if the JSON value is an object
			if (!jsonValue.is_object()) {
				return false;
			}

			// Check if all the required keys exist in the JSON object
			bool allExist =  jsonValue.contains("Dawn") && jsonValue.contains("Sunrise") && jsonValue.contains("Day") && jsonValue.contains("Sunset") && jsonValue.contains("Dusk") && jsonValue.contains("Night") && jsonValue.contains("InteriorDay") && jsonValue.contains("InteriorNight");
		
			if (!allExist)
				return false;

			if (std::is_floating_point<T>::value) {
				return jsonValue["Day"].is_number_float();
			}
			if (std::is_integral<T>::value) {
				return jsonValue["Day"].is_number();
			}
			if (std::is_same<T, bool>::value) {
				return jsonValue["Day"].is_boolean();
			}

			return false; // Unknown
		}

	private:
		bool _drawAllVals = true;
	};
	
}

namespace nlohmann
{
	template <typename T>
	struct adl_serializer<Configuration::TODValue<T>>
	{
		static void to_json(json& j, const Configuration::TODValue<T>& value)
		{
			//if (value.AllEqual()) {
			if (value.Dawn == value.Sunrise && value.Sunrise == value.Day && value.Day == value.Sunset && value.Sunset == value.Dusk && value.Dusk == value.Night && value.Night == value.InteriorDay && value.InteriorDay == value.InteriorNight) {
				j = value.Day;  // Store as a single value
			} else {
				j = nlohmann::json{
					{ "Dawn", value.Dawn },
					{ "Sunrise", value.Sunrise },
					{ "Day", value.Day },
					{ "Sunset", value.Sunset },
					{ "Dusk", value.Dusk },
					{ "Night", value.Night },
					{ "InteriorDay", value.InteriorDay },
					{ "InteriorNight", value.InteriorNight }
				};
			}
		}

		static void from_json(const json& j, Configuration::TODValue<T>& value)
		{
			if (j.is_object()) {
				value.Dawn = j.at("Dawn").get<T>();
				value.Sunrise = j.at("Sunrise").get<T>();
				value.Day = j.at("Day").get<T>();
				value.Sunset = j.at("Sunset").get<T>();
				value.Dusk = j.at("Dusk").get<T>();
				value.Night = j.at("Night").get<T>();
				value.InteriorDay = j.at("InteriorDay").get<T>();
				value.InteriorNight = j.at("InteriorNight").get<T>();
			} else {
				T val = j.get<T>();
				value.SetAll(val);
			}
		}
	};
}