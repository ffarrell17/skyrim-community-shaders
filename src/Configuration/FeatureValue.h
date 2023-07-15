#pragma once
#include "TODInfo.h"
#include "Helpers/UI.h"
#include "Helpers/Math.h"

namespace Configuration
{
	// Types to differentiate logic when handling settings collections
	// and individual settings display logic
	enum class FeatureSettingsType
	{
		General,				 // General user settings
		WeatherOverrideDefault,  // Default override when no override is present
		WeatherOverride          // Override settings
	};
	
	// Types to differentiate logic when handling settings collections
	// and individual settings display logic
	enum class TOD
	{
		SunriseStart,
		SunriseEnd,
		Day,
		SunsetStart,
		SunsetEnd,
		Night,
		InteriorDay,
		InteriorNight,
	};

	struct FeatureValueGeneric
	{
	public:

		std::string _id = GenerateGuidAsString();
		FeatureSettingsType _type = FeatureSettingsType::General;
		bool _isTOD = false;
		std::shared_ptr<bool> _isOverwritten = std::make_shared<bool>(false);
		std::shared_ptr<bool> _hasValue = std::make_shared<bool>(true);
		bool _updated = false;

		virtual void SetAsLerp(FeatureValueGeneric* start, FeatureValueGeneric* end, float progress) = 0;
		virtual void Copy(FeatureValueGeneric* fv) = 0;
		virtual std::string ToString() = 0;
		
		bool HasValue() const
		{
			return *_hasValue;
		}

		void Release()
		{
			*_hasValue = false;
			_isTOD = false;
		}

		bool IsTODValue() const
		{
			return _isTOD;
		}

		void SetTODValue(bool todVal)
		{
			_isTOD = todVal;
		}

		bool IsWeatherOverrideEnabled();

		virtual void SetAsOverrideVal(FeatureValueGeneric* fv) = 0;

		/* bool IsOverwritten()
		{
			return *_isOverwritten;
		}

		void SetOverwritten(bool overwritten)
		{
			*_isOverwritten = overwritten;
		}*/

		bool HasUpdated() const
		{
			return _updated;
		}

		void ResetUpdated()
		{
			_updated = false;
		}

		void SetType(FeatureSettingsType type)
		{
			_type = type;
		}

	private:
		std::string GenerateGuidAsString()
		{
			GUID guid;
			CoCreateGuid(&guid);

			return GuidToString(guid);
		}

		std::string GuidToString(GUID guid)
		{
			char guid_cstr[39];
			snprintf(guid_cstr, sizeof(guid_cstr),
				"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
				guid.Data1, guid.Data2, guid.Data3,
				guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
				guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

			return std::string(guid_cstr);
		}
	};

	typedef FeatureValueGeneric fv_any;

	template <typename T>
	struct FeatureValue : FeatureValueGeneric
	{
	public:

		T Value;

	private:

		T* OverrideVal;

		T _sunriseStart;
		T _sunriseEnd;
		T _day;
		T _sunsetStart;
		T _sunsetEnd;
		T _night;

		T _interiorDay;
		T _interiorNight;

		ImGuiDataType_ _imGuiDataType;

	public:


		FeatureValue(FeatureSettingsType type = FeatureSettingsType::General)
		{ 
			_type = type;
			if (_type == FeatureSettingsType::General)
				*_hasValue = true;

			if constexpr(std::is_same_v<T, int> || std::is_same_v<T, int32_t>) {
				_imGuiDataType = ImGuiDataType_S32;
			} else if constexpr (std::is_same_v<T, uint32_t>) {
				_imGuiDataType = ImGuiDataType_U32;
			} else if constexpr (std::is_same_v<T, float>) {
				_imGuiDataType = ImGuiDataType_Float;
			} else if constexpr (std::is_same_v<T, double>) {
				_imGuiDataType = ImGuiDataType_Double;
			}
		}

		~FeatureValue() 
		{ }

		FeatureValue(T defaultVal, FeatureSettingsType type = FeatureSettingsType::General) :
			FeatureValue(type)
		{
			Value = defaultVal;
			*_hasValue = true;
		}

		void SetValue(T val)
		{
			Value = val;
			*_hasValue = true;
		}

		std::string ToString()
		{
			return std::to_string(Value);
		}

		virtual void SetAsLerp(FeatureValueGeneric* start, FeatureValueGeneric* end, float progress) override
		{
			FeatureValue<T>* startFV = dynamic_cast<FeatureValue<T>*>(start);
			FeatureValue<T>* endFV = dynamic_cast<FeatureValue<T>*>(end);

			Value = Helpers::Math::Lerp(startFV->Value, endFV->Value, progress);			
		}

		virtual void Copy(FeatureValueGeneric* fv) override
		{
			FeatureValue<T>* newFV = dynamic_cast<FeatureValue<T>*>(fv);

			Value = newFV->Value;

			_isTOD = newFV->IsTODValue();
			*_hasValue = newFV->HasValue();
			_updated = newFV->HasUpdated();

			if (fv->_type == FeatureSettingsType::General) {
				if (_type == FeatureSettingsType::WeatherOverrideDefault || _type == FeatureSettingsType::WeatherOverride) {
					logger::info("here 1");
					_hasValue = fv->_isOverwritten;
				}
			} else if (fv->_type == FeatureSettingsType::WeatherOverrideDefault && _type == FeatureSettingsType::WeatherOverride) {
				logger::info("here 2");
				_hasValue = fv->_hasValue;
				_isOverwritten = fv->_isOverwritten;
			}

			//T _sunriseStart;
			//T _sunriseEnd;
			//T _day;
			//T _sunsetStart;
			//T _sunsetEnd;
			//T _night;

			//T _interiorDay;
			//T _interiorNight;
		}

		T GetTODValue(TOD tod) {
			switch (tod) {
			case Configuration::TOD::Night:
				return _night;
			case Configuration::TOD::SunriseStart:
				return _sunriseStart;
			case Configuration::TOD::SunriseEnd:
				return _sunriseEnd;
			case Configuration::TOD::Day:
				return _day;
			case Configuration::TOD::SunsetStart:
				return _sunriseStart;
			case Configuration::TOD::SunsetEnd:
				return _sunriseEnd;
			case Configuration::TOD::Night:
				return _night;
			case Configuration::TOD::InteriorDay:
				return _night;
			case Configuration::TOD::InteriorNight:
				return _night;
			default:
				return _day;
			}
		}

		/* FeatureValue& operator=(const T& value)
		{
			SetAll(value);
			return *this;
		}

		bool operator==(const FeatureValue<T>& other) const
		{
			return Get() == other.Get();
		}

		bool operator<(const FeatureValue<T>& other) const
		{
			return Get() < other.Get();
		}

		bool operator>(const FeatureValue<T>& other) const
		{
			return Get() < other.Get();
		}

		double operator*(double f) const
		{
			return Get() * f;
		}

		template <typename U = T>  // Only if T != double
		double operator*(typename std::enable_if<!std::is_same<U, double>::value, double>::type f) const
		{
			return Get() * f;
		}

		double operator*(const FeatureValue<T>& other) const
		{
			return Get() * other.Get();
		}

		double operator/(const FeatureValue<T>& other) const
		{
			if (other.Get() == 0) {
				throw std::runtime_error("Division by zero!");
			}
			return Get() / other.Get();
		}

		T operator-(const FeatureValue<T>& other) const
		{
			return Get() - other.Get();
		}

		T operator+(const FeatureValue<T>& other) const
		{
			return Get() + other.Get();
		}

		void SetAll(T val)
		{
			SunriseStart = SunriseEnd = Day = SunsetStart = SunsetEnd = Night = InteriorDay = InteriorNight = val;
			_drawAllVals = false;
		}*/

		/*bool AllTODEqual() const
		{
			return (_sunriseStart == _sunriseEnd && _sunriseEnd == _day && _day == _sunsetStart && _sunsetStart == _sunsetEnd && _sunsetEnd == _night && _night == _interiorDay && _interiorDay == _interiorNight);
		}*/

		/*UpdateValue()
		{
			Value = Get();
		}*/

		/* T CalculateTOD() const
		{
			if (AllTODEqual())
				return _day;

			const auto& todInfo = TODInfo::GetSingleton();

			if (!todInfo.Valid)
				return _day;

			try {
				if (todInfo.Exterior) {
					switch (todInfo.TimePeriodType) {
					case Configuration::TODInfo::TimePeriod::NightToDawn:
						return Helpers::Math::Lerp(_night, _sunriseStart, todInfo.TimePeriodPercentage);

					case Configuration::TODInfo::TimePeriod::DawnToSunrise:
						return Helpers::Math::Lerp(_sunriseStart, _sunriseEnd, todInfo.TimePeriodPercentage);

					case Configuration::TODInfo::TimePeriod::SunriseToDay:
						return Helpers::Math::Lerp(_sunriseEnd, _day, todInfo.TimePeriodPercentage);

					case Configuration::TODInfo::TimePeriod::DayToSunset:
						return Helpers::Math::Lerp(_day, _sunsetStart, todInfo.TimePeriodPercentage);

					case Configuration::TODInfo::TimePeriod::SunsetToDusk:
						return Helpers::Math::Lerp(_sunsetStart, _sunsetEnd, todInfo.TimePeriodPercentage);

					case Configuration::TODInfo::TimePeriod::DuskToNight:
						return Helpers::Math::Lerp(_sunsetEnd, _night, todInfo.TimePeriodPercentage);

					default:
						return _day;
					}
				} else {
					switch (todInfo.TimePeriodType) {
					case Configuration::TODInfo::TimePeriod::DuskToNight:
					case Configuration::TODInfo::TimePeriod::NightToDawn:
						return _interiorNight;
					default:
						return _interiorDay;
					}
				}
			} catch (std::exception& e) {
				logger::error("Error getting TOD value for of type [{}]. Error: {}", typeid(T).name(), e.what());
				return _day;
			}
		}*/

		void DrawCheckbox(std::string label)
		{
			PreDraw();

			bool val;
			if (*_isOverwritten && IsWeatherOverrideEnabled()) {
				// Display overwritten val
				if (OverrideVal)
					val = static_cast<bool>(*OverrideVal);
			} else {
				val = static_cast<bool>(Value);
			}

			if (ImGui::Checkbox(label.c_str(), &val)) {
				Value = val;
				_updated = true;
			}

			PostDraw();
		}

		void DrawSlider(std::string label, T min, T max)
		{
			FeatureValue<T> fvMin(min);
			FeatureValue<T> fvMax(max);
			DrawSlider(label, fvMin, fvMax);
		}

		void DrawSlider(std::string label, FeatureValue<T> min, T max)
		{
			FeatureValue<T> fvMax(max);
			DrawSlider(label, min, fvMax);
		}

		void DrawSlider(std::string label, T min, FeatureValue<T> max)
		{
			FeatureValue<T> fvMin(min);
			DrawSlider(label, fvMin, max);
		}

		void DrawSlider(std::string label, FeatureValue<T>& min, FeatureValue<T>& max)
		{
			PreDraw(label, min, max);
			
			if (*_isOverwritten && IsWeatherOverrideEnabled()) {
				// Display overwritten val

				T val;
				if (OverrideVal != nullptr)
					val = *OverrideVal;

				ImGui::SliderScalar(label.c_str(), _imGuiDataType, static_cast<void*>(&val), &min.Value, &max.Value);
			} else {
				if (ImGui::SliderScalar(label.c_str(), _imGuiDataType, static_cast<void*>(&Value), &min.Value, &max.Value)) {
					_updated = true;
				}
			}

			PostDraw();
		}

		bool drawIsDisabled;
		bool showTOD = false;
		void PreDraw()
		{
			_updated = false;
			drawIsDisabled = false;
			if (*_isOverwritten && IsWeatherOverrideEnabled()) {
				ImGui::BeginDisabled();
				drawIsDisabled = true;
			}

			if (_type == FeatureSettingsType::WeatherOverrideDefault || _type == FeatureSettingsType::WeatherOverride) {
				
				if (_type == FeatureSettingsType::WeatherOverrideDefault) {
					ImGui::PushID(_id.c_str());
					if (ImGui::Checkbox("O", &*_hasValue))
						_updated = true;
					ImGui::PopID();
					ImGui::SameLine();
				}

				if (!*_isOverwritten && !*_hasValue) {
					ImGui::BeginDisabled();
					drawIsDisabled = true;
				}
			}
		}

		void PreDraw(std::string label, FeatureValue<T>& min, FeatureValue<T>& max)
		{
			_updated = false;
			drawIsDisabled = false;
			if (*_isOverwritten && IsWeatherOverrideEnabled()) {
				ImGui::BeginDisabled();
				drawIsDisabled = true;
			}

			if (_type == FeatureSettingsType::WeatherOverrideDefault || _type == FeatureSettingsType::WeatherOverride) {
				if (_type == FeatureSettingsType::WeatherOverrideDefault) {
					ImGui::PushID(_id.c_str());
					if (ImGui::Checkbox("O", &*_hasValue))
						_updated = true;
					ImGui::PopID();
					ImGui::SameLine();
				}

				if (!*_isOverwritten && !*_hasValue) {
					ImGui::BeginDisabled();
					drawIsDisabled = true;
				}

				if (_isTOD) {
					// change next button style
				}

				if (ImGui::Button("TOD")) {
					showTOD = true;
				}

				if (showTOD)
					DrawTODWindow(label, min, max);

				ImGui::SameLine();

				if (_isTOD && !drawIsDisabled) {
					ImGui::BeginDisabled();
					drawIsDisabled = true;
				}
			}
		}

		void PostDraw()
		{
			if (drawIsDisabled) {
				ImGui::EndDisabled();
			}
		}

		void DrawTODWindow(std::string label, FeatureValue<T>& min, FeatureValue<T>& max)
		{
			ImGui::DockSpaceOverViewport(NULL, ImGuiDockNodeFlags_PassthruCentralNode);

			ImGui::SetNextWindowSize({ 200, 200 }, ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowPos({ 1000, 400 }, ImGuiCond_FirstUseEver);
			ImGui::Begin(("Time Of Day Editor: " + label).c_str());

			if (ImGui::Checkbox("Use Time Of Day Settings", &_isTOD)) {
				_updated = true;
			}

			_updated = _updated || ImGui::SliderScalar("Sunrise Start", _imGuiDataType, &_sunriseStart, &min._sunriseStart, &max._sunriseStart);
			_updated = _updated || ImGui::SliderScalar("Sunrise End", _imGuiDataType, &_sunriseEnd, &min._sunriseEnd, &max._sunriseEnd);
			_updated = _updated || ImGui::SliderScalar("Day", _imGuiDataType, &_day, &min._day, &max._day);
			_updated = _updated || ImGui::SliderScalar("Sunset Start", _imGuiDataType, &_sunsetStart, &min._sunsetStart, &max._sunsetStart);
			_updated = _updated || ImGui::SliderScalar("Sunset End", _imGuiDataType, &_sunsetEnd, &min._sunsetEnd, &max._sunsetEnd);
			_updated = _updated || ImGui::SliderScalar("Night", _imGuiDataType, &_night, &min._night, &max._night);
			ImGui::Spacing();
			_updated = _updated || ImGui::SliderScalar("Interior Day", _imGuiDataType, &_interiorDay, &min._interiorDay, &max._interiorDay);
			_updated = _updated || ImGui::SliderScalar("Interior Night", _imGuiDataType, &_interiorNight, &min._interiorNight, &max._interiorNight);

			if (ImGui::Button("Close"))
				showTOD = true;

			ImGui::End();
		}

		virtual void SetAsOverrideVal(FeatureValueGeneric* fv) override
		{
			FeatureValue<T>* newFV = dynamic_cast<FeatureValue<T>*>(fv);
			OverrideVal = &newFV->Value;
		}

		/*bool DrawSliderScalar(std::string label, ImGuiDataType_ drawType, T min, T max, const char* format = NULL)
		{
			FeatureValue<T> todMin(min);
			FeatureValue<T> todMax(max);
			return DrawSliderScalar(label, drawType, todMin, todMax, format);
		}

		bool DrawSliderScalar(std::string label, ImGuiDataType_ drawType, FeatureValue<T> min, T max, const char* format = NULL)
		{
			FeatureValue<T> todMax(max);
			return DrawSliderScalar(label, drawType, min, todMax, format);
		}

		bool DrawSliderScalar(std::string label, ImGuiDataType_ drawType, T min, FeatureValue<T> max, const char* format = NULL)
		{
			FeatureValue<T> todMin(min);
			return DrawSliderScalar(label, drawType, todMin, max, format);
		}
		// Allowing other TODValues to mark the range per TOD
		 bool DrawSliderScalar(std::string label, ImGuiDataType_ drawType, FeatureValue<T>& min, FeatureValue<T>& max, const char* format = NULL)
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

				updated = updated || ImGui::SliderScalar((label + " Sunrise Start").c_str(), drawType, static_cast<void*>(&SunriseStart), &min.SunriseStart, &max.SunriseStart, format);
				updated = updated || ImGui::SliderScalar((label + " Sunrise End").c_str(), drawType, static_cast<void*>(&SunriseEnd), &min.SunriseEnd, &max.SunriseEnd, format);
				updated = updated || ImGui::SliderScalar((label + " Day").c_str(), drawType, static_cast<void*>(&Day), &min.Day, &max.Day, format);
				updated = updated || ImGui::SliderScalar((label + " Sunset Start").c_str(), drawType, static_cast<void*>(&SunsetStart), &min.SunsetStart, &max.SunsetStart, format);
				updated = updated || ImGui::SliderScalar((label + " Sunset End").c_str(), drawType, static_cast<void*>(&SunsetEnd), &min.SunsetEnd, &max.SunsetEnd, format);
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
		}*/

		/* void DrawCheckBox(std::string label)
		{
			Helpers::UI::CustomCheckbox("TOD Value", &_drawAllVals);

			bool updated = false;

			if (_drawAllVals) {
				// Current val goes here

				updated = updated || ImGui::Checkbox(label + " Sunrise Start", &SunriseStart);
				updated = updated || ImGui::Checkbox(label + " Sunrise End", &SunriseEnd);
				updated = updated || ImGui::Checkbox(label + " Day", &Day);
				updated = updated || ImGui::Checkbox(label + " Sunset Start", &SunsetStart);
				updated = updated || ImGui::Checkbox(label + " Sunset End", &SunsetEnd);
				updated = updated || ImGui::Checkbox(label + " Night", &Night);
				updated = updated || ImGui::Checkbox(label + " InteriorDay", &InteriorDay);
				updated = updated || ImGui::Checkbox(label + " InteriorNight", &InteriorNight);

			} else {
				if (ImGui::Checkbox(label, &Day)) {
					SetAll(Day);
					updated = true;
				}
			}
		}*/

		/* static bool IsTODValue(const nlohmann::json& jsonValue)
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
		}*/
	};

	typedef FeatureValue<int32_t> fv_int32;
	typedef FeatureValue<uint32_t> fv_uint32;
	typedef FeatureValue<float> fv_float;
	typedef FeatureValue<double> fv_double;

}






namespace nlohmann
{
	template <typename T>
	struct GenericFVJsonHandler
	{
		static void to_json(json& j, const Configuration::FeatureValue<T>& value)
		{
			if (value.HasValue()) {
				if (!value.IsTODValue()) {
					j = value.Value;
				} else {
					//TOD logic
				}
			}
		}

		static void from_json(const json& j, Configuration::FeatureValue<T>& value)
		{
			if (j.is_object()) {
				// TOD logic
			} else if (!j.is_null()) {
				value.SetValue(j.get<T>());
			} else {
				value.Release();
			}
		}
	};

	template <typename T>
	struct adl_serializer<Configuration::FeatureValue<T>>
	{
		static void to_json(json& j, const Configuration::FeatureValue<T>& value) {
			GenericFVJsonHandler<T>::to_json(j, value); 
		}

		static void from_json(const json& j, Configuration::FeatureValue<T>& value) {
			GenericFVJsonHandler<T>::from_json(j, value);  
		}
	};

	// All typedef types forwarding to generic struct
	
	template <>
	struct adl_serializer<Configuration::fv_int32>
	{
		static void to_json(json& j, const Configuration::fv_int32& value) {
			GenericFVJsonHandler<int32_t>::to_json(j, value); 
		}
		static void from_json(const json& j, Configuration::fv_int32& value) { 
			GenericFVJsonHandler<int32_t>::from_json(j, value);  
		}
	};

	template <>
	struct adl_serializer<Configuration::fv_uint32>
	{ 
		static void to_json(json& j, const Configuration::fv_uint32& value) { 
			GenericFVJsonHandler<uint32_t>::to_json(j, value); 
		}
		static void from_json(const json& j, Configuration::fv_uint32& value) { 
			GenericFVJsonHandler<uint32_t>::from_json(j, value); 
		}
	};

	template <>
	struct adl_serializer<Configuration::fv_float>
	{
		static void to_json(json& j, const Configuration::fv_float& value) { 
			GenericFVJsonHandler<float>::to_json(j, value); 
		}
		static void from_json(const json& j, Configuration::fv_float& value) { 
			GenericFVJsonHandler<float>::from_json(j, value); 
		}
	};

	template <>
	struct adl_serializer<Configuration::fv_double>
	{
		static void to_json(json& j, const Configuration::fv_double& value) { 
			GenericFVJsonHandler<double>::to_json(j, value); 
		}
		static void from_json(const json& j, Configuration::fv_double& value) { 
			GenericFVJsonHandler<double>::from_json(j, value); 
		}
	};
}

			/* if (j.is_object()) {
				value.SunriseStart = j.at("SunriseStart").get<T>();
				value.SunriseEnd = j.at("SunriseEnd").get<T>();
				value.Day = j.at("Day").get<T>();
				value.SunsetStart = j.at("SunsetStart").get<T>();
				value.SunsetEnd = j.at("SunsetEnd").get<T>();
				value.Night = j.at("Night").get<T>();
				value.InteriorDay = j.at("InteriorDay").get<T>();
				value.InteriorNight = j.at("InteriorNight").get<T>();
			} else {
				T val = j.get<T>();
				value.SetAll(val);
			}*/

		//if (value.AllEqual()) {
		/* if (value.SunriseStart == value.SunriseEnd && value.SunriseEnd == value.Day && value.Day == value.SunsetStart && value.SunsetStart == value.SunsetEnd && value.SunsetEnd == value.Night && value.Night == value.InteriorDay && value.InteriorDay == value.InteriorNight) {
				j = value.Day;  // Store as a single value
			} else {
				j = nlohmann::json{
					{ "SunriseStart", value.SunriseStart },
					{ "SunriseEnd", value.SunriseEnd },
					{ "Day", value.Day },
					{ "SunsetStart", value.SunsetStart },
					{ "SunsetEnd", value.SunsetEnd },
					{ "Night", value.Night },
					{ "InteriorDay", value.InteriorDay },
					{ "InteriorNight", value.InteriorNight }
				};
			}*/
		//}
		//}

		//static void to_json(json& j, const Configuration::FeatureValue<T>& value)
		//{
			//if (value.HasValue()) {
			//	if (!value.IsTODValue()) {
			//		j = value.Value;
			//	} else {
					//TOD logic
			//		j = value.Value;
			//	}

				//if (value.AllEqual()) {
				/* if (value.SunriseStart == value.SunriseEnd && value.SunriseEnd == value.Day && value.Day == value.SunsetStart && value.SunsetStart == value.SunsetEnd && value.SunsetEnd == value.Night && value.Night == value.InteriorDay && value.InteriorDay == value.InteriorNight) {
				j = value.Day;  // Store as a single value
			} else {
				j = nlohmann::json{
					{ "SunriseStart", value.SunriseStart },
					{ "SunriseEnd", value.SunriseEnd },
					{ "Day", value.Day },
					{ "SunsetStart", value.SunsetStart },
					{ "SunsetEnd", value.SunsetEnd },
					{ "Night", value.Night },
					{ "InteriorDay", value.InteriorDay },
					{ "InteriorNight", value.InteriorNight }
				};
			}*/
			//}
		//}

	// Typedef versions:
//#include "FeatureValue.cpp"  // Include the implementation file
//#endif