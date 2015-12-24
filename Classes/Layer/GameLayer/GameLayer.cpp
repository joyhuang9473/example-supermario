#include "GameLayer.h"
#include "VisibleRect.h"
#include "../../GameManager.h"
#include "../../Controller/KeyboardController.h"
#include "../../Scene/GameScene.h"
#include "../../Scene/MenuScene.h"

USING_NS_CC;

#define PTM_RATIO 32
#define GAMEMANAGER GameManager::getInstance()

bool GameLayer::init() {
	if (!Layer::init()) {
		return false;
	}

    // Stage
    auto stageFile = GAMEMANAGER->getCurStageFile();
    if (stageFile == "") {
        stageFile = "default.plist";
    }

    GAMEMANAGER->readStageInfo(stageFile);

    // Map
    auto map = TMXTiledMap::create(GAMEMANAGER->getCurMapName());
	map->setScale(2.8);
	map->setMapSize(map->getMapSize()*2.8);

    // Player
	auto player = Player::create();
	this->setPlayer(player);

	// Mission
	this->isInterrupt = false;

	this->bindRoleToMap(player, map);

	this->m_map = map;
	this->m_player = player;
	this->addChild(this->m_map, -1);
	this->addChild(this->m_player);

    this->schedule(schedule_selector(GameLayer::logic));
    return true;
}

void GameLayer::setPlayer(Player* player) {
	Size visibleSize = Director::getInstance()->getVisibleSize();

	KeyboardController* keyboardController = KeyboardController::create();
	keyboardController->registerWithKeyboardDispatcher();
	keyboardController->setRole(player);

	player->setController(keyboardController);
	player->addChild(keyboardController);
	player->getFSM()->doEvent("stand");
	player->setPosition(visibleSize.width / 2, visibleSize.height / 2);
}

void GameLayer::bindRoleToMap(Role* role, cocos2d::TMXTiledMap* map) {
	role->setTiledMap(map);
}

void GameLayer::logic(float dt) {
	if (this->isInterrupt) {
		return;
	}

	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto playerSize = this->m_player->getSprite()->getContentSize();
	TMXObjectGroup* playerObject = this->m_map->getObjectGroup("player");
	ValueMap endPointMap = playerObject->getObject("end_point");
	Vec2 endPoint = Vec2(endPointMap["x"].asFloat() * this->m_map->getScale(),
						endPointMap["y"].asFloat() * this->m_map->getScale());

	if (this->m_player->getPosition().x <= playerSize.width / 2) {
		this->m_player->setPositionX(playerSize.width / 2);
	} else if (this->m_player->getPosition().x >= this->m_map->getMapSize().width * this->m_map->getTileSize().width - playerSize.width/2) {
		this->m_player->setPositionX(this->m_map->getMapSize().width * this->m_map->getTileSize().width - playerSize.width/2);
	}

	if (this->m_player->getPosition().y >= visibleSize.height - playerSize.height) {
		this->m_player->setPositionY(visibleSize.height - playerSize.height);
	}

	/*
	* Mission Complete
	*/
	
	if (this->m_player->getPosition().x > endPoint.x) {
		//Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("missionComplete");
		this->isInterrupt = true;

		auto visibleSize = Director::getInstance()->getVisibleSize();
		auto label = Label::createWithTTF("Mission Complete", "Marker Felt.ttf", 72);

		label->setPosition(Vec2(endPoint.x, 2 * visibleSize.height / 3));
		this->addChild(label);
	}

	/*
	* Mission Failed
	*/
	//Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("missionFailed");

}

void GameLayer::initPhysics() {
	this->setPhysicsBodyForRole(this->m_player);
	this->m_world->setGravity(Vec2(0.0, -90.0));

	/*
	* Bounding: land, block, pipe 
	*/
	for (int i = 1; i <= 52; i++) {
		TMXObjectGroup* boundObject = this->m_map->getObjectGroup(String::createWithFormat("bound%d", i)->getCString());

		Vec2 bound_start = Vec2(
			boundObject->getObject("bound_start")["x"].asFloat() * this->m_map->getScale(),
			boundObject->getObject("bound_start")["y"].asFloat() * this->m_map->getScale());
		Vec2 bound_end = Vec2(
			boundObject->getObject("bound_end")["x"].asFloat() * this->m_map->getScale(),
			boundObject->getObject("bound_end")["y"].asFloat() * this->m_map->getScale());
		Size bound_size = Size(fabs(bound_start.x - bound_end.x), fabs(bound_start.y - bound_end.y));

		auto boundSp = Sprite::create();
		auto boundBody = PhysicsBody::createEdgeBox(bound_size, PHYSICSBODY_MATERIAL_DEFAULT);
		boundSp->setPosition(bound_start.x + bound_size.width / 2, bound_start.y + (bound_size.height / 2));
		boundSp->setPhysicsBody(boundBody);
		this->addChild(boundSp);
	}

	/*
	* Monster
	*/
	TMXObjectGroup* boundObject = this->m_map->getObjectGroup("enemy");
	ValueVector objects = boundObject->getObjects();

	for (ValueVector::iterator iter = objects.begin(); iter != objects.end(); ++iter) {
		Value object = *(iter);
		ValueMap map = object.asValueMap();
		Sprite* objectSp = nullptr;

		if (map["name"].asString() == "mushroom") {
			objectSp = Sprite::create("mushroom.png");
		}
		else if (map["name"].asString() == "tortoise") {
			objectSp = Sprite::create("tortoise.png");
		}
		else if (map["name"].asString() == "flower") {
			objectSp = Sprite::create("flower.png");
		}

		Vec2 postion = Vec2(map["x"].asFloat() * this->m_map->getScale(),
							map["y"].asFloat() * this->m_map->getScale());

		objectSp->setScale(this->m_map->getScale());

		Size size = objectSp->getContentSize();
		auto body = PhysicsBody::createBox(size);

		objectSp->setPhysicsBody(body);
		objectSp->setPosition(postion.x, postion.y);
		this->addChild(objectSp);
	}

}

void GameLayer::setPhysicsBodyForRole(Role* role) {
	auto body = PhysicsBody::createBox(role->getBodyBox().actual.size);
	body->setRotationEnable(false);
	role->setPhysicsBody(body);
}

void GameLayer::updatePhysicsBody(float dt) {

}

void GameLayer::updateActionScope(float dt) {

}

bool GameLayer::collisionDetection(const BoundingBox &bodyBoxA, const BoundingBox &bodyBoxB) {
	return true;
}

void GameLayer::setPhysicsWorld(cocos2d::PhysicsWorld* world) {
	this->m_world = world;
}
