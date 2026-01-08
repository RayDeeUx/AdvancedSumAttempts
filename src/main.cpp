#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/modify/LevelListLayer.hpp>
#include <Geode/modify/GauntletLayer.hpp>

using namespace geode::prelude;

enum class ArrayType {
	LevelBrowser,
	LevelList,
	Gauntlet
};

void findSumAndDisplay(CCArray* array, const ArrayType type, GJSearchObject* gjso) {
	if (!Mod::get()->getSettingValue<bool>("enabled")) return log::info("line 14");
	if (!array || (type == ArrayType::LevelBrowser && !gjso)) return log::info("line 15");
	if (CCScene::get() && CCScene::get()->getChildByID("ungeil.higher_or_lower/HLLayer")) {
		// prevent cheating, i guess
		FLAlertLayer* aler = FLAlertLayer::create("SIKE!", "You're not getting any hints from here, bucko.", "I Understand");
		aler->m_noElasticity = true;
		aler->show();
		return;
	}

	CCObject* firstObject = array->objectAtIndex(0);
	if (type == ArrayType::LevelList && !typeinfo_cast<LevelCell*>(firstObject)) return log::info("line 25");
	if ((type == ArrayType::LevelBrowser || type == ArrayType::Gauntlet) && !typeinfo_cast<GJGameLevel*>(firstObject)) return log::info("line 26");

	int attempts = 0, jumps = 0, objects = 0, clicks = 0, stars = 0, moons = 0, orbs = 0, awardedStars = 0, awardedMoons = 0, minTimestamps = 0, timestamps = 0, maxTimestamps = 0, completed = 0, levels = 0;
	bool foundXL = false;
	GameLevelManager* glm = GameLevelManager::get();
	for (CCObject* obj : CCArrayExt<CCObject*>(array)) {
		if (!obj) continue;

		GJGameLevel* theLevel = nullptr;
		int rating = 0;
		if (type == ArrayType::LevelList && static_cast<LevelCell*>(obj)->m_level) {
			theLevel = glm->getSavedLevel(static_cast<LevelCell*>(obj)->m_level->m_levelID.value());
			rating = static_cast<LevelCell*>(obj)->m_level->m_stars.value();
		} else if (type == ArrayType::LevelBrowser) {
			const bool isType98 = static_cast<int>(gjso->m_searchType) == 98;
			if (isType98) theLevel = static_cast<GJGameLevel*>(obj);
			else theLevel = glm->getSavedLevel(static_cast<GJGameLevel*>(obj)->m_levelID.value());
			if (!theLevel && !isType98) continue;
			if (theLevel && !isType98 && static_cast<std::string>(theLevel->m_levelString).empty()) continue;
			if (!isType98) rating = static_cast<GJGameLevel*>(obj)->m_stars.value();
			else rating = 0;
		} else if (type == ArrayType::Gauntlet) {
			theLevel = glm->getSavedGauntletLevel(static_cast<GJGameLevel*>(obj)->m_levelID.value());
			rating = static_cast<GJGameLevel*>(obj)->m_stars.value();
		}

		if (!theLevel || static_cast<std::string>(theLevel->m_levelString).empty()) continue;

		attempts += theLevel->m_attempts.value();
		jumps += theLevel->m_jumps.value();
		objects += theLevel->m_objectCount.value();
		clicks += theLevel->m_clicks.value();

		const bool isPlatformer = theLevel->isPlatformer();
		if (isPlatformer) moons += rating;
		else stars += rating;

		if (theLevel->m_normalPercent.value() > 99) {
			completed += 1;
			if (isPlatformer) awardedMoons += rating;
			else awardedStars += rating;
		}

		timestamps += theLevel->m_timestamp;
		if (theLevel->m_timestamp > 0) {
			minTimestamps += theLevel->m_timestamp;
			maxTimestamps += theLevel->m_timestamp;
		} else if (theLevel->m_levelLength > -1 && theLevel->m_levelLength < 5) {
			if (theLevel->m_levelLength == 0) {
				maxTimestamps += (8 * 240);
			} else if (theLevel->m_levelLength == 1) {
				minTimestamps += ( 9 * 240);
				maxTimestamps += (29 * 240);
			} else if (theLevel->m_levelLength == 2) {
				minTimestamps += (30 * 240);
				maxTimestamps += (59 * 240);
			} else if (theLevel->m_levelLength == 3) {
				minTimestamps += ( 60 * 240);
				maxTimestamps += (119 * 240);
			} else {
				minTimestamps += (120 * 240);
				if (!foundXL) foundXL = true;
			}
		}

		// auto levels don't count, silly!
		if (rating > 1 && rating < 11) {
			// it's entirely possible these numbers are wrong, fandom wikis can sometimes be the worst
			if (rating == 2) orbs += 50;
			else if (rating == 3) orbs += 75;
			else if (rating == 4) orbs += 125;
			else if (rating == 5) orbs += 175;
			else if (rating == 6) orbs += 225;
			else if (rating == 7) orbs += 275;
			else if (rating == 8) orbs += 350;
			else if (rating == 9) orbs += 425;
			else orbs += 500;
		}

		levels += 1;
	}

	if (levels == 0) {
		FLAlertLayer* nothing = FLAlertLayer::create("AdvancedSumAttempts", fmt::format("<cy>No info available!</c>\n\n<co>Try downloading {} more level{}, then check back again!</c>", array->count(), array->count() != 1 ? "s" : ""), "OK");
		nothing->m_noElasticity = true;
		nothing->show();
		return;
	}

	if (gjso && static_cast<int>(gjso->m_searchType) == 98) {
		FLAlertLayer* alertEditor = FLAlertLayer::create(
			nullptr,
			"AdvancedSumAttempts",
			fmt::format(
				"You have <co>{} attempt{}</c>, <cc>{} jump{}</c>, and <cd>{} click{}</c> across <cb>{} level{}</c>.\n\n"
				"Together, these levels use at least <ca>{} object{}</c>.",
				attempts, attempts != 1 ? "s" : "", jumps, jumps != 1 ? "s" : "", clicks, clicks != 1 ? "s" : "",
				levels, levels != 1 ? "s" : "", objects, objects != 1 ? "s" : ""
			),
			"Close", nullptr, 420.f,
			false, 320.f, 1.f);
		alertEditor->m_noElasticity = true;
		alertEditor->show();
		return;
	}

	// these things can happen, apparently. why? i'll never know
	int tempTimestamp = 0;
	if (maxTimestamps < minTimestamps) {
		tempTimestamp = minTimestamps;
		minTimestamps = maxTimestamps;
		maxTimestamps = tempTimestamp;
	}

	const std::string& warning = levels != array->count() ? fmt::format("\n\n<co>This information is incomplete. Try downloading {} more level{}, then check back later!</c>", (array->count() - levels), (array->count() - levels > 1) ? "s" : "") : "";
	const std::string& timestampsString = (timestamps == minTimestamps && timestamps == maxTimestamps && minTimestamps == maxTimestamps) ? fmt::format("It will take at least <cy>{} second{}</c> to beat the <cb>{} level{}</c> available.", timestamps / 240, timestamps / 240 != 1 ? "s" : "", levels, levels != 1 ? "s" : "") : fmt::format("It will take somewhere between <cg>{} second{}</c> and <cr>{} second{}</c>{} (calculated <cy>{} second{}</c>) to beat the <cb>{} level{}</c> available.{}{}", minTimestamps / 240, minTimestamps / 240 != 1 ? "s" : "", maxTimestamps / 240, maxTimestamps / 240 != 1 ? "s" : "", foundXL ? "<c_>*</c>" : "", timestamps / 240, timestamps / 240 != 1 ? "s" : "", levels, levels != 1 ? "s" : "", foundXL ? " <c_>(Perhaps even longer, with XL levels.)</c>" : "", timestamps < 1 && levels == array->count() ? "\n\n(This estimate is <c_>VERY</c> rough, as none of these levels have a known level duration. Try viewing the levels individually using the BetterInfo mod, if you have it.)" : "");


	FLAlertLayer* alert = FLAlertLayer::create(
		nullptr,
		"AdvancedSumAttempts",
		fmt::format(
			"You have <co>{} attempt{}</c>, <cc>{} jump{}</c>, "
			"<cd>{} click{}</c>, <cs>{} collected star{}</c>, "
			"and <cj>{} collected moon{}</c> across <cb>{} available level{}</c>,\n"
			"of which <cg>{} are completed</c> and <cf>{} total on screen</c>.\n\n"
			"Together, these <cb>*available* levels</c> use at least <ca>{} object{}</c> "
			"and can give you <cs>{} star{}</c> and <cj>{} moon{}</c> (<cl>{} orb{}</c>).\n\n"
			"{}{}",
			attempts, attempts != 1 ? "s" : "", jumps, jumps != 1 ? "s" : "",
			clicks, clicks != 1 ? "s" : "", awardedStars, awardedStars != 1 ? "s" : "", awardedMoons, awardedMoons != 1 ? "s" : "",
			levels, levels != 1 ? "s" : "", completed, array->count(),
			objects, objects != 1 ? "s" : "",
			stars, stars != 1 ? "s" : "", moons, moons != 1 ? "s" : "", orbs, orbs != 1 ? "s" : "",
			timestampsString, warning
		),
		"Close", nullptr, 420.f,
		false, 300.f, (6.f / 7.f));
	alert->m_noElasticity = true;
	alert->show();
}

class $modify(MyLevelBrowserLayer, LevelBrowserLayer) {
	bool init(GJSearchObject* obj) {
		if (!LevelBrowserLayer::init(obj)) return false;
		if (!Mod::get()->getSettingValue<bool>("enabled")) return true;
		if (static_cast<int>(obj->m_searchType) == 9 || static_cast<int>(obj->m_searchType) == 14 || static_cast<int>(obj->m_searchType) == 20 || static_cast<int>(obj->m_searchType) == 25 || static_cast<int>(obj->m_searchType) > 100) return true;
		if (m_levels && !typeinfo_cast<GJGameLevel*>(m_levels->objectAtIndex(0))) return true;

		CCNode* topBorder = this->querySelector("GJListLayer > top-border");
		if (!topBorder) return true;

		CCMenu* newMenu = CCMenu::create();
		newMenu->ignoreAnchorPointForPosition(false);
		newMenu->setPosition(CCScene::get()->getContentSize() / 2.f);
		newMenu->setContentSize(CCScene::get()->getContentSize());
		this->addChild(newMenu);
		newMenu->setID("attempts-sum-menu"_spr);

		CCSprite* infoBtn = CCSprite::createWithSpriteFrameName("GJ_infoBtn_001.png");
		infoBtn->setScale(.5f);
		CCMenuItemSpriteExtra* btn = CCMenuItemSpriteExtra::create(infoBtn, this, menu_selector(MyLevelBrowserLayer::onAttemptSum));
		newMenu->addChild(btn);
		btn->setPosition({topBorder->getPositionX() - 103.f, topBorder->getPositionY() + 47.f});
		btn->setID("attempts-sum-button"_spr);

		return true;
	}
	void setupLevelBrowser(cocos2d::CCArray* items) {
		LevelBrowserLayer::setupLevelBrowser(items);
		if (!Mod::get()->getSettingValue<bool>("enabled")) return;

		CCNode* fooBar = this->getChildByID("attempts-sum-menu"_spr);
		if (!fooBar) return;
		if (!typeinfo_cast<GJGameLevel*>(m_levels->objectAtIndex(0))) return fooBar->setVisible(false);
		if (!m_list || !m_list->m_listView || !m_list->m_listView->m_tableView || !m_list->m_listView->m_tableView->m_cellArray) return fooBar->setVisible(false);
		if (m_list->m_listView->m_tableView->m_cellArray->count() < 1) return fooBar->setVisible(false);
		fooBar->setVisible(true);

		fooBar->setZOrder(fooBar->getZOrder() + 1);
		fooBar->setZOrder(fooBar->getZOrder() - 1);
	}
	void onAttemptSum(CCObject* sender) {
		if (m_levels) findSumAndDisplay(m_levels, ArrayType::LevelBrowser, m_searchObject);
	}
};

class $modify(MyLevelListLayer, LevelListLayer) {
	bool init(GJLevelList* p0) {
		if (!LevelListLayer::init(p0)) return false;
		if (!Mod::get()->getSettingValue<bool>("enabled")) return true;

		CCNode* leftSideMenu = this->getChildByID("left-side-menu");
		if (!leftSideMenu) return true;

		CCSprite* infoBtn = CCSprite::createWithSpriteFrameName("GJ_infoBtn_001.png");
		CCMenuItemSpriteExtra* btn = CCMenuItemSpriteExtra::create(infoBtn, this, menu_selector(MyLevelListLayer::onAttemptSum));
		leftSideMenu->addChild(btn);
		if (leftSideMenu->getLayout()) leftSideMenu->updateLayout();
		btn->setID("attempts-sum-button"_spr);

		return true;
	}
	void onAttemptSum(CCObject* sender) {
		if (!m_list || !m_list->m_listView || !m_list->m_listView->m_tableView || !m_list->m_listView->m_tableView->m_cellArray) return;
		findSumAndDisplay(m_list->m_listView->m_tableView->m_cellArray, ArrayType::LevelList, nullptr);
	}
};

class $modify(MyGauntletLayer, GauntletLayer) {
	static void onModify(auto& self) {
		(void) self.setHookPriorityAfterPost("GauntletLayer::init", "jacob375.gauntletlevelvault");
	}
	bool init(GauntletType p0) {
		if (!GauntletLayer::init(p0)) return false;
		if (!Mod::get()->getSettingValue<bool>("enabled")) return true;

		CCNode* exitMenu = this->getChildByID("exit-menu");
		if (!exitMenu) return true;

		CCSprite* infoBtn = CCSprite::createWithSpriteFrameName("GJ_infoBtn_001.png");
		infoBtn->setScale(.6f);
		CCMenuItemSpriteExtra* btn = CCMenuItemSpriteExtra::create(infoBtn, this, menu_selector(MyGauntletLayer::onAttemptSum));
		exitMenu->addChild(btn);
		btn->setZOrder(-1); // just below jacob's gauntlet integration button, but well above exit button itself
		if (CCNode* fooBar = exitMenu->getChildByID("jacob375.gauntletlevelvault/gauntlet-levels")) fooBar->setZOrder(-1);
		if (exitMenu->getLayout()) exitMenu->updateLayout();
		btn->setID("attempts-sum-button"_spr);

		return true;
	}
	void onAttemptSum(CCObject* sender) {
		if (m_levels) findSumAndDisplay(m_levels, ArrayType::Gauntlet, nullptr);
	}
};
