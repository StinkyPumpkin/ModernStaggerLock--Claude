#pragma once
#include "DKUtil/Config.hpp"
#include "DKUtil/Utility.hpp"
#include "PrecisionAPI.h"

namespace ModernStaggerLock
{
	using namespace DKUtil::Alias;

	class MSLSettings : public DKUtil::model::Singleton<MSLSettings>
	{
		friend DKUtil::model::Singleton<MSLSettings>;

	private:
		MSLSettings();

		template <class T>
		static void PrintSettingValue(const T& a_setting)
		{
			INFO("Setting:\"{}\" is {}"sv, a_setting.get_key(), a_setting.get_data());
		}

	public:
		Boolean DisableJumpWhenStagger{ "DisableJumpWhenStagger", "Control" };

		Double SmallStaggerMagnitude{ "SmallStaggerMagnitude", "StaggerLevel" };
		Double MediumStaggerMagnitude{ "MediumStaggerMagnitude", "StaggerLevel" };
		Double LargeStaggerMagnitude{ "LargeStaggerMagnitude", "StaggerLevel" };
		Double LargestStaggerMagnitude{ "LargestStaggerMagnitude", "StaggerLevel" };

		// --- Claude fork additions ---
		// Below this stagger magnitude the StaggerStart event is swallowed
		// entirely, so casual body bumps no longer become full locked staggers.
		// Real weapon-art unbalance (high magnitude) clears the floor and still
		// fires. The stock GetStaggerLevel() had an unconditional "return 1"
		// fallback that made every micro-stagger a Small stagger; this is the
		// real floor that never existed. See Hooks.cpp ShouldSuppressStagger.
		Double MinStaggerMagnitude{ "MinStaggerMagnitude", "StaggerLevel" };

		// Skip staggering actors that are in a SexLab scene (positioning members
		// shoves bodies together and fires micro-staggers mid-animation).
		Boolean SuppressStaggerDuringSexScene{ "SuppressStaggerDuringSexScene", "SexLab" };

		enum StaggerType : std::int32_t
		{
			kSmall = 0,
			kMedium,
			kLarge,
			kLargest,
			kTotal
		};

		static inline std::vector<std::string> quickRecoveryEvents = {};

		static inline PRECISION_API::IVPrecision5* ersh_Precision = nullptr;

		// SexLab.esm "SexLabAnimatingFaction" (0xE50F). Null when SexLab isn't in
		// the load order. Cached at kDataLoaded (see Settings.cpp).
		static inline RE::TESFaction* sexlabAnimatingFaction = nullptr;
	};
}