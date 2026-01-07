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
	if (type == ArrayType::LevelList && !typeinfo_cast<LevelCell*>(firstObject)) return log::info("line 18");
	if ((type == ArrayType::LevelBrowser || type == ArrayType::Gauntlet) && !typeinfo_cast<GJGameLevel*>(firstObject)) return log::info("line 19");

	int attempts = 0, jumps = 0, objects = 0, clicks = 0, stars = 0, moons = 0, awardedStars = 0, awardedMoons = 0, completed = 0, levels = 0;
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

		levels += 1;
	}

	if (levels == 0) {
		FLAlertLayer* nothing = FLAlertLayer::create("AdvancedSumAttempts", fmt::format("<cy>No info available!</c>\n\n<co>Try downloading {} more level{}, then check back again!</c>", array->count(), array->count() != 1 ? "s" : ""), "OK");
		nothing->m_noElasticity = true;
		nothing->show();
		return;
	}

	const std::string& warning = levels != array->count() ? fmt::format("\n\n<co>This information is incomplete. Try downloading {} more level{}, then check back later!</c>", (array->count() - levels), (array->count() - levels > 1) ? "s" : "") : "";

	if (gjso && static_cast<int>(gjso->m_searchType) == 98) {
		FLAlertLayer* alertEditor = FLAlertLayer::create(
			nullptr,
			"AdvancedSumAttempts",
			fmt::format(
				"You have {} attempts, {} jumps, and {} clicks across {} levels.\n\n"
				"These levels use a total of at least {} objects.",
				attempts, jumps, clicks,
				levels, objects
			),
			"Close", nullptr,
			false, 320.f, 1.f);
		alertEditor->m_noElasticity = true;
		alertEditor->show();
		return;
	}

	FLAlertLayer* alert = FLAlertLayer::create(
		"AdvancedSumAttempts",
		fmt::format(
			"You have {} attempts, {} jumps, "
			"{} clicks, {} stars, and {} moons "
			"across {} available levels ({} total).\n\n"
			"These levels use a total of at least {} objects "
			"and are worth {} stars and {} moons total.{}",
			attempts, jumps,
			clicks, awardedStars, awardedMoons,
			levels, array->count(),
			objects,
			stars, moons,
			warning
		), "OK");
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
