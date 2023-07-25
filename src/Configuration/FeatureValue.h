#pragma once
#include "TODInfo.h"
#include "Helpers/UI.h"
#include "Helpers/Math.h"
#include "Helpers/Time.h"
#include <implot.h>
#include "Menu.h"

#include <IconsMaterialDesign.h>

namespace Configuration
{
	union testU
	{
		float floatValue;
		int32_t intValue;
		uint32_t uintValue;
	};

	// Types to differentiate logic when handling settings collections
	// and individual settings display logic
	enum class FeatureSettingsType
	{
		General,				 // General user settings
		WeatherOverrideDefault,  // Default override when no override is present
		WeatherOverride          // Override settings
	};

	struct FeatureValueGeneric
	{
	protected:

		std::string _id = GenerateGuidAsString();
		FeatureSettingsType _type = FeatureSettingsType::General;
		bool _isTOD = false;
		std::shared_ptr<bool> _isOverwritten = std::make_shared<bool>(false);
		std::shared_ptr<bool> _hasValue = std::make_shared<bool>(true);
		bool _updated = false;

	public:

		virtual void TODUpdate() = 0;
		
		virtual std::string ToString() = 0;
		
		bool HasValue() const;
		void Release();

		bool IsTODValue() const;
		void SetIsTODValue(bool todVal);

		void SetType(FeatureSettingsType type);
		FeatureSettingsType GetType();

		std::shared_ptr<bool> GetIsOverwrittenPtr();
		void SetIsOverwrittenPtr(std::shared_ptr<bool> isOverwritten);

		std::shared_ptr<bool> GetHasValuePtr();
		void SetHasValuePtr(std::shared_ptr<bool> hasValue);

		virtual void SetAsLerp(FeatureValueGeneric* start, FeatureValueGeneric* end, float progress) = 0;

		virtual void Copy(FeatureValueGeneric* fv) = 0;
		virtual void Link(FeatureValueGeneric* fv) = 0;
		void CopyAndLink(FeatureValueGeneric* fv);

		virtual void SetAsOverrideVal(FeatureValueGeneric* fv) = 0;

		bool HasUpdated() const;
		void ResetUpdated();

		virtual testU toU() = 0;

	protected:
		bool IsWeatherOverrideEnabled();

	private:

		std::string GenerateGuidAsString();
		std::string GuidToString(GUID guid);
	};

	typedef FeatureValueGeneric fv_any;

	template <typename T>
	struct TODVals
	{
		T SunriseStart;
		T SunriseEnd;
		T Day;
		T SunsetStart;
		T SunsetEnd;
		T Night;

		T InteriorDay;
		T InteriorNight;
	};

	template <typename T>
	struct FeatureValue : FeatureValueGeneric
	{
	public:
		T Value;

	private:
		T* OverrideVal;
		TODVals<T> _todVals;

		ImGuiDataType_ _imGuiDataType;

	public:

		testU toU()
		{
			testU t;
			if constexpr (std::is_same_v<T, int> || std::is_same_v<T, int32_t>) {
				t.intValue = Value;
			} else if constexpr (std::is_same_v<T, uint32_t>) {
				t.uintValue = Value;
			} else if constexpr (std::is_same_v<T, float>) {
				t.floatValue = Value;
			}
			return t;
		}

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
			SetAllTODVals(Value);
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

		TODVals<T> GetTODVals() const
		{
			return _todVals;
		}

		void SetTODVals(TODVals<T> todVals)
		{
			_todVals = todVals;
		}

		virtual void SetAsLerp(FeatureValueGeneric* start, FeatureValueGeneric* end, float progress) override
		{
			start->TODUpdate();
			end->TODUpdate();
			FeatureValue<T>* startFV = dynamic_cast<FeatureValue<T>*>(start);
			FeatureValue<T>* endFV = dynamic_cast<FeatureValue<T>*>(end);

			Value = Helpers::Math::Lerp(startFV->Value, endFV->Value, progress);			
		}

		virtual void Copy(FeatureValueGeneric* fv) override
		{
			fv->TODUpdate();
			FeatureValue<T>* newFV = dynamic_cast<FeatureValue<T>*>(fv);

			Value = newFV->Value;

			_isTOD = newFV->IsTODValue();
			if (!_isTOD) {
				SetAllTODVals(Value);
			} else {
				auto fvTodVals = newFV->GetTODVals();

				_todVals.SunriseStart = fvTodVals.SunriseStart;
				_todVals.SunriseEnd = fvTodVals.SunriseEnd;
				_todVals.Day = fvTodVals.Day;
				_todVals.SunsetStart = fvTodVals.SunsetStart;
				_todVals.SunsetEnd = fvTodVals.SunsetEnd;
				_todVals.Night = fvTodVals.Night;
				_todVals.InteriorDay = fvTodVals.InteriorDay;
				_todVals.InteriorNight = fvTodVals.InteriorNight;
			}


			*_hasValue = newFV->HasValue();
			_updated = newFV->HasUpdated();
		}

		virtual void Link(FeatureValueGeneric* fv) override
		{
			if (fv->GetType() == FeatureSettingsType::General) {
				if (_type == FeatureSettingsType::WeatherOverrideDefault || _type == FeatureSettingsType::WeatherOverride) {
					fv->SetIsOverwrittenPtr(_hasValue);
				}
			} else if (fv->GetType() == FeatureSettingsType::WeatherOverrideDefault && _type == FeatureSettingsType::WeatherOverride) {
				_hasValue = fv->GetHasValuePtr();
				_isOverwritten = fv->GetIsOverwrittenPtr();
			}
		}

		void SetAllTODVals(T val)
		{
			_todVals.SunriseStart = _todVals.SunriseEnd = _todVals.Day = _todVals.SunsetStart = _todVals.SunsetEnd = _todVals.Night = _todVals.InteriorDay = _todVals.InteriorNight = val;
		}

		virtual void TODUpdate() override
		{
			if (!_isTOD)
				return;

			TODInfo* todInfo = TODInfo::GetSingleton();

			if (!todInfo->Valid)
				Value = _todVals.Day;

			try {
				if (todInfo->Exterior) {
					switch (todInfo->TimePeriodType) {
					case Configuration::TODInfo::TimePeriod::NightToSunriseStart:
						Value = Helpers::Math::Lerp(_todVals.Night, _todVals.SunriseStart, todInfo->TimePeriodPercentage);
						break;
					case Configuration::TODInfo::TimePeriod::SunriseStartToSunriseEnd:
						Value = Helpers::Math::Lerp(_todVals.SunriseStart, _todVals.SunriseEnd, todInfo->TimePeriodPercentage);
						break;
					case Configuration::TODInfo::TimePeriod::SunriseEndToDay:
						Value = Helpers::Math::Lerp(_todVals.SunriseEnd, _todVals.Day, todInfo->TimePeriodPercentage);
						break;
					case Configuration::TODInfo::TimePeriod::DayToSunsetStart:
						Value = Helpers::Math::Lerp(_todVals.Day, _todVals.SunsetStart, todInfo->TimePeriodPercentage);
						break;
					case Configuration::TODInfo::TimePeriod::SunsetStartToSunsetEnd:
						Value = Helpers::Math::Lerp(_todVals.SunsetStart, _todVals.SunsetEnd, todInfo->TimePeriodPercentage);
						break;
					case Configuration::TODInfo::TimePeriod::SunsetEndToNight:
						Value = Helpers::Math::Lerp(_todVals.SunsetEnd, _todVals.Night, todInfo->TimePeriodPercentage);
						break;
					default:
						Value = _todVals.Day;
						break;
					}
				} else {
					switch (todInfo->TimePeriodType) {
					case Configuration::TODInfo::TimePeriod::SunsetEndToNight:
					case Configuration::TODInfo::TimePeriod::NightToSunriseStart:
						Value = _todVals.InteriorNight;
						break;
					default:
						Value = _todVals.InteriorDay;
						break;
					}
				}
			} catch (std::exception& e) {
				logger::error("Error getting TOD value for of type [{}]. Error: {}", typeid(T).name(), e.what());
				Value = _todVals.Day;
			}
		}

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
					PushUniqueId("Override");
					if (ImGui::Button(!*_hasValue ? ICON_MD_LOCK : ICON_MD_LOCK_OPEN)) {
						*_hasValue = !*_hasValue;
					}
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
					PushUniqueId("Override");
					if (ImGui::Button(!*_hasValue ? ICON_MD_LOCK : ICON_MD_LOCK_OPEN)) {
						*_hasValue = !*_hasValue;
					}
					ImGui::PopID();
					ImGui::SameLine();
				}

				if (!*_isOverwritten && !*_hasValue) {
					ImGui::BeginDisabled();
					drawIsDisabled = true;
				}

				if (_isTOD) {
					PushUniqueId("TOD4");
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.6f, 1.0f));
					ImGui::PopID();
				}

				PushUniqueId("TOD");
				if (ImGui::Button(ICON_MD_SCHEDULE)) {
					showTOD = true;
				}
				ImGui::PopID();

				if (_isTOD) {
					ImGui::PopStyleColor();
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

		void PushUniqueId(std::string idPrefix)
		{
			std::string id = _id + '_' + idPrefix;
			ImGui::PushID(id.c_str());
		}

		void PostDraw()
		{
			if (drawIsDisabled) {
				ImGui::EndDisabled();
			}
		}

		T TODMin(bool exterior = true)
		{
			if (exterior)
				return std::min<T>(std::min<T>(std::min<T>(std::min<T>(std::min<T>(_todVals.Night, _todVals.SunriseStart), _todVals.SunriseEnd), _todVals.Day), _todVals.SunsetStart), _todVals.SunsetEnd);
			return std::min<T>(_todVals.InteriorDay, _todVals.InteriorNight);
		}

		T TODMax(bool exterior = true)
		{
			if (exterior)
				return std::max<T>(std::max<T>(std::max<T>(std::max<T>(std::max<T>(_todVals.Night, _todVals.SunriseStart), _todVals.SunriseEnd), _todVals.Day), _todVals.SunsetStart), _todVals.SunsetEnd);
			return std::max<T>(_todVals.InteriorDay, _todVals.InteriorNight);
		}

		struct TODValMap
		{
			T* val;
			double time;
			std::string label;
			T min;
			T max;
		};

		void DrawTODWindow(std::string label, FeatureValue<T>& min, FeatureValue<T>& max)
		{
			min = min;
			max = max;
			ImGui::DockSpaceOverViewport(NULL, ImGuiDockNodeFlags_PassthruCentralNode);

			PushUniqueId("TOD3");
			ImGui::SetNextWindowSize({ 1000, 400 }, ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowPos({ 1000, 400 }, ImGuiCond_FirstUseEver);
			ImGui::Begin(("TOD: " + label).c_str(), &showTOD);
			ImGui::PopID();

			PushUniqueId("TOD2");
			if (ImGui::Checkbox("Use Time Of Day", &_isTOD)) {
				_updated = true;
			}
			ImGui::PopID();

			static bool showSliders = false;
			static bool showInterior = false;

			//ImGui::SameLine();
			//ImGui::Dummy(ImVec2(100.0f, 0.0f));
			//ImGui::SameLine();
			ImGui::Text("Graph");
			ImGui::SameLine();
			Helpers::UI::ToggleButton("", &showSliders);
			ImGui::SameLine();
			ImGui::Text("Sliders");

			ImGui::SameLine();
			ImGui::Dummy(ImVec2(100.0f, 0.0f));
			ImGui::SameLine();

			ImGui::Text("Exterior");
			ImGui::SameLine();
			Helpers::UI::ToggleButton("", &showInterior);
			ImGui::SameLine();
			ImGui::Text("Interior");

			std::string str = TemplateToString(Value);
			//ImGui::InputText("Current Value", &str, ImGuiInputTextFlags_ReadOnly);

			if (!_isTOD)
				ImGui::BeginDisabled();

			TODInfo* todInfo = TODInfo::GetSingleton();

			static const int secsInHr = 60 * 60;
			static const int secsInDay = 24 * secsInHr;

			double nightTimeL = secsInDay;
			double sunriseStartTime = secsInDay + Helpers::Time::TimeToSeconds(todInfo->SunriseBeginTime);
			double sunriseEndTime = secsInDay + Helpers::Time::TimeToSeconds(todInfo->SunriseEndTime);
			double dayTime = secsInDay + 12 * secsInHr;
			double sunsetStartTime = secsInDay + Helpers::Time::TimeToSeconds(todInfo->SunsetBeginTime);
			double sunsetEndTime = secsInDay + Helpers::Time::TimeToSeconds(todInfo->SunsetEndTime);
			double nightTimeR = secsInDay * 2;

			const auto minTodVals = min.GetTODVals();
			const auto maxTodVals = max.GetTODVals();

			std::vector<TODValMap> todVals;

			if (!showInterior) {
				todVals = {
					{ &_todVals.Night, nightTimeL, "Night", minTodVals.Night, maxTodVals.Night },
					{ &_todVals.SunriseStart, sunriseStartTime, "Sunrise Start", minTodVals.SunriseStart, maxTodVals.SunriseStart },
					{ &_todVals.SunriseEnd, sunriseEndTime, "Sunrise End", minTodVals.SunriseEnd, maxTodVals.SunriseEnd },
					{ &_todVals.Day, dayTime, "Day", minTodVals.Day, maxTodVals.Day },
					{ &_todVals.SunsetStart, sunsetStartTime, "Sunset Start", minTodVals.SunsetStart, maxTodVals.SunsetStart },
					{ &_todVals.SunsetEnd, sunsetEndTime, "Sunset End", minTodVals.SunsetEnd, maxTodVals.SunsetEnd },
					{ &_todVals.Night, nightTimeR, "Night", minTodVals.Night, maxTodVals.Night }
				};
			} else {
				todVals = {
					{ &_todVals.InteriorNight, nightTimeL, "Night", minTodVals.InteriorNight, maxTodVals.InteriorNight },
					{ &_todVals.InteriorDay, dayTime, "Day", minTodVals.InteriorDay, maxTodVals.InteriorDay },
					{ &_todVals.InteriorNight, nightTimeR, "Night", minTodVals.InteriorNight, maxTodVals.InteriorNight }
				};
			}

			if (!showSliders) {
				// Use ImPlot to create the TOD graph
				// Examples: https://traineq.org/implot_demo/src/implot_demo.html
				if (ImPlot::BeginPlot("  ", ImVec2(-FLT_MIN, -FLT_MIN) /*, ImPlotFlags_Crosshairs*/)) {
					
					ImPlot::SetupLegend(ImPlotLocation_North, ImPlotLegendFlags_None);

					T minT = min.TODMin(!showInterior);
					T maxT = max.TODMax(!showInterior);
					T dif = maxT - minT;

					double minYaxis = minT - (dif * 0.1);
					double maxYaxis = maxT + (dif * 0.1);

					// Setup Axes

					ImPlot::SetupAxes("Time (hr)", label.c_str(), ImPlotAxisFlags_NoMenus | ImPlotAxisFlags_NoSideSwitch | ImPlotAxisFlags_NoHighlight);
					ImPlot::SetupAxesLimits(secsInDay - 30 * 60, secsInDay * 2 + 30 * 60, minYaxis, maxYaxis);
					ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);

					static double xAxisVal[] = { secsInDay, secsInDay + 2 * secsInHr, secsInDay + 4 * secsInHr, secsInDay + 6 * secsInHr, secsInDay + 8 * secsInHr, secsInDay + 10 * secsInHr, secsInDay + 12 * secsInHr, secsInDay + 14 * secsInHr, secsInDay + 16 * secsInHr, secsInDay + 18 * secsInHr, secsInDay + 20 * secsInHr, secsInDay + 22 * secsInHr, secsInDay + 24 * secsInHr };
					static const char* xAxisLabel[] = { "0", "2", "4", "6", "8", "10", "12", "14", "16", "18", "20", "22", "24" };
					ImPlot::SetupAxisTicks(ImAxis_X1, xAxisVal, 13, xAxisLabel, false);

					ImPlot::SetupAxis(ImAxis_X2, 0, ImPlotAxisFlags_AuxDefault | ImPlotAxisFlags_NoDecorations);
					ImPlot::SetAxis(ImAxis_X2);
					ImPlot::SetupAxisLimits(ImAxis_X2, secsInDay - 30 * 60, secsInDay * 2 + 30 * 60);

					// Tags for times of interest
					for (int i = 0; i < todVals.size(); i++) {
						ImPlot::TagX(todVals[i].time, ImVec4(1, 1, 1, 1), todVals[i].label.c_str());
					}

					// Lines for times of interest
					ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
					for (int i = 0; i < todVals.size(); i++) {
						PlotVerticalLine(todVals[i].time, minYaxis, maxYaxis, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
					}

					// Show current tag value on y-axis
					AddCurrentValueTag(todVals, dif);
					
					// Plot line of all values
					// Making line disappear off screen both side
					int size = static_cast<int>(todVals.size()) + 2;
					double xs1[9], ys1[9];

					xs1[0] = todVals[todVals.size() - 2].time - secsInDay;  //sunsetEndTime - secsInDay;
					ys1[0] = *todVals[todVals.size() - 2].val;   //_todVals.SunsetEnd;
					for (int i = 0; i < todVals.size(); i++) {
						xs1[i+1] = todVals[i].time;
						ys1[i+1] = *todVals[i].val;
					}
					xs1[size - 1] = todVals[1].time + secsInDay;
					ys1[size - 1] = *todVals[1].val;
					ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 2);
					ImPlot::PlotLine(label.c_str(), xs1, ys1, size);

					// Add drag points for each tod val
					ImVec4 dragPointColour = ImVec4(1, 1, 0, 1);
					float dragPointSize = 4.0f;
					for (int i = 0; i < todVals.size(); i++) {
						PlotDragPoint(i, todVals[i].time, *todVals[i].val, todVals[i].min, todVals[i].max, dragPointColour, dragPointSize);
					}

					// Add current time line
					double currentTime = Helpers::Time::TimeToSeconds(todInfo->Time);
					static double currentX[2], currentY[2];
					currentX[0] = currentTime + secsInDay;
					currentY[0] = minYaxis;
					currentX[1] = currentTime + secsInDay;
					currentY[1] = maxYaxis;

					ImPlot::SetNextLineStyle(ImVec4(0.2f, 0.4f, 0.6f, 1.0f), 2);
					std::string cu = "Time";
					ImPlot::PlotLine(cu.c_str(), currentX, currentY, 2);

					// Add current value annotation
					bool above = true;
					bool right = 23 * 60 * 60 > currentTime;
					if (!showInterior) {
						switch (todInfo->TimePeriodType) {
						case TODInfo::NightToSunriseStart:
							above = _todVals.Night > _todVals.SunriseStart;
							break;
						case TODInfo::SunriseStartToSunriseEnd:
							above = _todVals.SunriseStart > _todVals.SunriseEnd;
							break;
						case TODInfo::SunriseEndToDay:
							above = _todVals.SunriseEnd > _todVals.Day;
							break;
						case TODInfo::DayToSunsetStart:
							above = _todVals.Day > _todVals.SunsetStart;
							break;
						case TODInfo::SunsetStartToSunsetEnd:
							above = _todVals.SunsetStart > _todVals.SunsetEnd;
							break;
						case TODInfo::SunsetEndToNight:
							above = _todVals.SunsetEnd > _todVals.Night;
							break;
						}
					} else {
						switch (todInfo->TimePeriodType) {
						case TODInfo::NightToSunriseStart:
						case TODInfo::SunriseStartToSunriseEnd:
						case TODInfo::SunriseEndToDay:
							above = _todVals.InteriorNight > _todVals.InteriorDay;
							break;
						case TODInfo::DayToSunsetStart:
						case TODInfo::SunsetStartToSunsetEnd:
						case TODInfo::SunsetEndToNight:
							above = _todVals.InteriorNight < _todVals.InteriorDay;
							break;
						}
					}

					if (!right)
						above = !above;

					ImPlot::Annotation(currentTime + secsInDay, static_cast<double>(Value), ImVec4(0.2f, 0.4f, 0.6f, 1.0f), ImVec2(right ? 5.0f : -5.0f, above ? -10.0f : 10.0f), false, str.c_str());

					ImPlot::EndPlot();
				}
			} else {
				
				// Displaying Sliders

				//for (auto& todVal : todVals) {
				for (int i = 0; i < todVals.size() - 1; i++) {
					PushUniqueId(todVals[i].label);
					_updated = _updated || ImGui::SliderScalar(todVals[i].label.c_str(), _imGuiDataType, &*todVals[i].val, &todVals[i].min, &todVals[i].max);
					ImGui::PopID();
				}

			}

			if (!_isTOD)
				ImGui::EndDisabled();

			ImGui::End();
		}

		void AddCurrentValueTag(std::vector<TODValMap>& todVals, T minMaxDiff)
		{
			auto plotPoint = ImPlot::GetPlotMousePos();

			double disp = plotPoint.y;

			for (const auto& todVal : todVals) {
				if ((std::abs(todVal.time - plotPoint.x) < 60 * 10) && (std::abs(static_cast<double>(*todVal.val) - plotPoint.y) < (minMaxDiff * 0.015))) {
					disp = *todVal.val;
					break;
				}
			}

			ImPlot::TagY(disp, ImVec4(0.2f, 0.4f, 0.6f, 1.0f), TemplateToString(static_cast<T>(disp)).c_str());
		}

		void PlotVerticalLine(double xVal, double minY, double maxY, ImVec4 colour)
		{
			double x[2], y[2];
			x[0] = xVal;
			y[0] = minY;
			x[1] = xVal;
			y[1] = maxY;

			ImPlot::SetNextLineStyle(colour, 2);
			ImPlot::PlotLine("", x, y, 2);
		}

		void PlotDragPoint(int id, double& x, T& y, T& min, T& max, ImVec4 colour, float size)
		{
			double xCopy = x;
			double yCopy = static_cast<double>(y);
			if (ImPlot::DragPoint(id, &xCopy, &yCopy, colour, size, 0)) {
				y = static_cast<T>(yCopy);
				if (y < min)
					y = min;
				else if (y > max)
					y = max;
			}
			xCopy = x;
			//ImPlot::Annotation(x, yCopy, ImVec4(0.2f, 0.4f, 0.6f, 1.0f), ImVec2( 5.0f, 10.0f), false, std::to_string(y).c_str());


			PlotMinMaxLine(x - 30 * 60, x + 30 * 60, static_cast<double>(min));
			PlotMinMaxLine(x - 30 * 60, x + 30 * 60, static_cast<double>(max));
		}

		void PlotMinMaxLine(double xstart, double xend, double yval)
		{
			ImVec4 colour = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
			ImPlot::SetNextLineStyle(colour, 1);

			double x[2], y[2];
			x[0] = xstart;
			y[0] = yval;
			x[1] = xend;
			y[1] = yval;
			ImPlot::PlotLine("", x, y, 2);
		}

		virtual void SetAsOverrideVal(FeatureValueGeneric* fv) override
		{
			FeatureValue<T>* newFV = dynamic_cast<FeatureValue<T>*>(fv);
			OverrideVal = &newFV->Value;
		}

		std::string TemplateToString(T val)
		{
			if (std::is_same<T, float>::value) {
				std::stringstream stream;
				stream << std::fixed << std::setprecision(3) << val;
				return stream.str();
			} else {
				return std::to_string(val);
			}
		}
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
					auto tod = value.GetTODVals();
					j = nlohmann::json{
						{ "SunriseStart", tod.SunriseStart },
						{ "SunriseEnd", tod.SunriseEnd },
						{ "Day", tod.Day },
						{ "SunsetStart", tod.SunsetStart },
						{ "SunsetEnd", tod.SunsetEnd },
						{ "Night", tod.Night },
						{ "InteriorDay", tod.InteriorDay },
						{ "InteriorNight", tod.InteriorNight }
					};
				}
			}
		}

		static void from_json(const json& j, Configuration::FeatureValue<T>& value)
		{
			if (j.is_object()) {
				auto tod = value.GetTODVals();
				tod.SunriseStart = j.at("SunriseStart").get<T>();
				tod.SunriseEnd = j.at("SunriseEnd").get<T>();
				tod.Day = j.at("Day").get<T>();
				tod.SunsetStart = j.at("SunsetStart").get<T>();
				tod.SunsetEnd = j.at("SunsetEnd").get<T>();
				tod.Night = j.at("Night").get<T>();
				tod.InteriorDay = j.at("InteriorDay").get<T>();
				tod.InteriorNight = j.at("InteriorNight").get<T>();
				value.SetTODVals(tod);
				value.SetIsTODValue(true);
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