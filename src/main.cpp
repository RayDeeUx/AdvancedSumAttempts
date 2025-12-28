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
	if (!Mod::get()->getSettingValue<bool>("enabled")) return;
	if (!array || (type == ArrayType::LevelBrowser && !gjso)) return;

	CCObject* firstObject = array->objectAtIndex(0);
	if (type == ArrayType::LevelList && !typeinfo_cast<LevelCell*>(firstObject)) return;
	if ((type == ArrayType::LevelBrowser || type == ArrayType::Gauntlet) && !typeinfo_cast<GJGameLevel*>(firstObject)) return;

	int attempts = 0, jumps = 0, clicks = 0, levels = 0;
	GameLevelManager* glm = GameLevelManager::get()
	for (CCObject* obj : CCArrayExt<CCObject*>(array)) {
		GJGameLevel* theLevel;
		if (type == ArrayType::LevelList) {
			theLevel = glm->getSavedLevel(static_cast<LevelCell*>(obj)->m_level->m_levelID.value());
		} else if (type == ArrayType::LevelBrowser) {
			const bool isType98 = static_cast<int>(gjso->->m_searchType) == 98;
			if (isType98) theLevel = static_cast<GJGameLevel*>(obj);
			else theLevel = glm->getSavedLevel(static_cast<GJGameLevel*>(obj)->m_levelID.value());
			if (!theLevel && !isType98) continue;
			if (theLevel && !isType98 && static_cast<std::string>(theLevel->m_levelString).empty()) continue;
		} else if (type == ArrayType::Gauntlet) {
			theLevel = glm->getSavedGauntletLevel(static_cast<GJGameLevel*>(obj)->m_levelID.value());
		}

		if (!theLevel || static_cast<std::string>(theLevel->m_levelString).empty()) continue;

		attempts += theLevel->m_attempts.value();
		jumps += theLevel->m_jumps.value();
		clicks += theLevel->m_clicks.value();
		levels += 1;

		const std::string& warning = levels != array->count() ? fmt::format("\n<co>Try downloading {} more levels and checking back again!</c>", (array->count() - levels)) : "";
		FLAlertLayer::create("AdvancedSumAttempts", fmt::format("You have {} attempts, {} jumps, and {} clicks total across {} available levels ({} total).{}", attempts, jumps, clicks, levels, m_levels->count(), warning), "OK")->show();
	}
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
		if (!typeinfo_cast<GJGameLevel*>(m_levels->objectAtIndex(0))) return fooBar->removeMeAndCleanup();

		fooBar->setZOrder(fooBar->getZOrder() + 1);
		fooBar->setZOrder(fooBar->getZOrder() - 1);
	}
	void onAttemptSum(CCObject* sender) {
		if (!m_levels) return;
		/*
		if (!typeinfo_cast<GJGameLevel*>(m_levels->objectAtIndex(0))) return;
		if (!Mod::get()->getSettingValue<bool>("enabled")) return;
		int attempts = 0;
		int jumps = 0;
		int clicks = 0;
		int levels = 0;
		for (GJGameLevel* level : CCArrayExt<GJGameLevel*>(m_levels)) {
			if (!level) continue;
			if (static_cast<int>(m_searchObject->m_searchType) == 98) {
				attempts += level->m_attempts.value();
				jumps += level->m_jumps.value();
				clicks += level->m_clicks.value();
				levels += 1;
				continue;
			}
			GJGameLevel* realLevel = GameLevelManager::get()->getSavedLevel(level->m_levelID.value());
			if (static_cast<int>(m_searchObject->m_searchType) != 98 && !realLevel) continue;
			if (static_cast<int>(m_searchObject->m_searchType) != 98 && static_cast<std::string>(realLevel->m_levelString).empty()) continue;
			attempts += realLevel->m_attempts.value();
			jumps += realLevel->m_jumps.value();
			clicks += realLevel->m_clicks.value();
			levels += 1;
		}
		const std::string& warning = levels != m_levels->count() ? fmt::format("\n<co>Try downloading {} more levels and checking back again!</c>", (m_levels->count() - levels)) : "";
		FLAlertLayer::create("AdvancedSumAttempts", fmt::format("You have {} attempts, {} jumps, and {} clicks total across {} available levels ({} total).{}", attempts, jumps, clicks, levels, m_levels->count(), warning), "OK")->show();
		*/
		findSumAndDisplay(m_levels, ArrayType::LevelBrowser, m_searchObject);
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
		/*
		if (!Mod::get()->getSettingValue<bool>("enabled")) return;
		int attempts = 0;
		int jumps = 0;
		int clicks = 0;
		int levels = 0;
		for (LevelCell* levelCell : CCArrayExt<LevelCell*>(m_list->m_listView->m_tableView->m_cellArray)) {
			if (!levelCell || !levelCell->m_level) continue;
			GJGameLevel* realLevel = GameLevelManager::get()->getSavedLevel(levelCell->m_level->m_levelID.value());
			if (!realLevel) continue;
			if (static_cast<std::string>(realLevel->m_levelString).empty()) continue;
			attempts += realLevel->m_attempts.value();
			jumps += realLevel->m_jumps.value();
			clicks += realLevel->m_clicks.value();
			levels += 1;
		}
		const std::string& warning = levels != m_list->m_listView->m_tableView->m_cellArray->count() ? fmt::format("\n<co>Try downloading {} more levels and checking back again!</c>", (m_list->m_listView->m_tableView->m_cellArray->count() - levels)) : "";
		FLAlertLayer::create("AdvancedSumAttempts", fmt::format("You have {} attempts, {} jumps, and {} clicks total across {} available levels ({} total).{}", attempts, jumps, clicks, levels, m_list->m_listView->m_tableView->m_cellArray->count(), warning), "OK")->show();
		*/
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
		if (!m_levels) return;
		/*
		if (!Mod::get()->getSettingValue<bool>("enabled")) return;
		int attempts = 0;
		int jumps = 0;
		int clicks = 0;
		int levels = 0;
		for (GJGameLevel* level : CCArrayExt<GJGameLevel*>(m_levels)) {
			if (!level) continue;
			GJGameLevel* realLevel = GameLevelManager::get()->getSavedGauntletLevel(level->m_levelID.value());
			if (!realLevel) continue;
			if (static_cast<std::string>(realLevel->m_levelString).empty()) continue;
			attempts += realLevel->m_attempts.value();
			jumps += realLevel->m_jumps.value();
			clicks += realLevel->m_clicks.value();
			levels += 1;
		}
		const std::string& warning = levels != m_levels->count() ? fmt::format("\n<co>Try downloading {} more levels and checking back again!</c>", (m_levels->count() - levels)) : "";
		FLAlertLayer::create("AdvancedSumAttempts", fmt::format("You have {} attempts, {} jumps, and {} clicks total across {} available levels ({} total).{}", attempts, jumps, clicks, levels, m_levels->count(), warning), "OK")->show();
		*/
		findSumAndDisplay(m_levels, ArrayType::Gauntlet, nullptr);
	}
};