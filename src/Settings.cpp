#include "Settings.h"
#include "SpecialStaggerHandler.h"

namespace ModernStaggerLock
{
	using json = nlohmann::json;

	static auto MainConfig = COMPILE_PROXY("ModernStaggerLock.ini"sv);

	MSLSettings::MSLSettings()
	{
		MainConfig.Bind<0.0, 1.0>(SmallStaggerMagnitude, 0.0);
		MainConfig.Bind<0.0, 1.0>(MediumStaggerMagnitude, 0.5);
		MainConfig.Bind<0.0, 1.0>(LargeStaggerMagnitude, 0.75);
		MainConfig.Bind<0.0, 1.0>(LargestStaggerMagnitude, 1.0);
		MainConfig.Bind(DisableJumpWhenStagger, true);

		// --- Claude fork additions ---
		MainConfig.Bind<0.0, 1.0>(MinStaggerMagnitude, 0.15);
		MainConfig.Bind(SuppressStaggerDuringSexScene, true);

		MainConfig.Load();

		PrintSettingValue(SmallStaggerMagnitude);
		PrintSettingValue(MediumStaggerMagnitude);
		PrintSettingValue(LargeStaggerMagnitude);
		PrintSettingValue(LargestStaggerMagnitude);
		PrintSettingValue(MinStaggerMagnitude);
		PrintSettingValue(SuppressStaggerDuringSexScene);

		// Cache the SexLab animating faction once, now that forms are loaded.
		if (auto* dh = RE::TESDataHandler::GetSingleton()) {
			sexlabAnimatingFaction = dh->LookupForm<RE::TESFaction>(0x00E50F, "SexLab.esm"sv);
			INFO("SexLab AnimatingFaction (0xE50F): {}", sexlabAnimatingFaction ? "found - scene guard active" : "not present - scene guard inert");
		}

		auto MSLFilesName = DKUtil::Config::GetAllFiles<false>("Data\\SKSE\\Plugins\\ModernStaggerLock"sv, ".json"sv, {}, "_MSL"sv);
		auto SpecialStaggerHandler = SpecialStaggerHandler::GetSingleton();
		for (const auto& fileName : MSLFilesName) {
			try {
				json js = json::parse(std::ifstream(fileName));
				for (auto& element : js) {
					if (element.contains("QuickRecoveryEvents")) {
						auto eventLists = element.at("QuickRecoveryEvents").get<std::vector<std::string>>();
						quickRecoveryEvents.insert(quickRecoveryEvents.end(), eventLists.begin(), eventLists.end());
					} else if (element.contains("SpecialStaggerTransData")) {
						auto transData = element.at("SpecialStaggerTransData").get<SpecialStaggerTransactionData>();
						auto animID = HashSpecialStaggerID(transData.animName.data(), transData.animName.size());
						SpecialStaggerHandler->transDataMap.insert({ animID, transData });
					}
				}
			} catch (const json::exception& ex) {
				ERROR("Caught expection when parsing file \"{}\":\n {}", fileName, ex.what());
			}
		}

		for (auto eventName : quickRecoveryEvents) {
			INFO("Get a Quick Recovery Event Name: \"{}\"", eventName);
		}

		for (auto transData : SpecialStaggerHandler->transDataMap) {
			INFO("Get a Special Stagger Anim Name: \"{}\"", transData.second.animName);
		}
	}

}