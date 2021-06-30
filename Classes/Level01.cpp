#include "Level01.h"

Level01::~Level01()
{
	CC_SAFE_DELETE(_ParticleSystem);
}

Scene* Level01::createScene()
{
	return Level01::create();
}

// on "init" you need to initialize your instance
bool Level01::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Scene::init())
	{
		return false;
	}

	//  For Loading Plist+Texture
	//	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");
	_visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");

	//標題 : 顯示目前 BOX2D 所介紹的功能
	_titleLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 48);
	_titleLabel->setPosition(_visibleSize.width / 2, _visibleSize.height * 0.9f);
	this->addChild(_titleLabel, 1);

	_csbRoot = CSLoader::createNode("Level01.csb");

	addChild(_csbRoot, 1);

	setObject();

	_b2World->SetContactListener(&_contactListener);

	auto listener = EventListenerTouchOneByOne::create();	//創建一個一對一的事件聆聽器
	listener->onTouchBegan = CC_CALLBACK_2(Level01::onTouchBegan, this);		//加入觸碰開始事件
	listener->onTouchMoved = CC_CALLBACK_2(Level01::onTouchMoved, this);		//加入觸碰移動事件
	listener->onTouchEnded = CC_CALLBACK_2(Level01::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(Level01::update));

	return true;
}

void Level01::update(float dt)
{
	int velocityIterations = 8;	// 速度迭代次數
	int positionIterations = 1; // 位置迭代次數
	// Instruct the world to perform a single step of simulation.
	// It is generally best to keep the time step and iterations fixed.
	_b2World->Step(dt, velocityIterations, positionIterations);

	_car->update(dt);

	b2Vec2 worldPos = b2Vec2(_car->getCarBody()->GetWorldPoint(_car->getCarLocPos()));
	Point pos = Point(worldPos.x * PTM_RATIO, worldPos.y * PTM_RATIO);
	_ParticleSystem->setPosition(pos);
	_ParticleSystem->update(dt);

	if (_contactListener._isFinish) {
		_car->setFinish(_goalPos);
		_stopLight->setSpriteFrame("orange05.png");
		_endCount += dt;
		if (_endCount >= ENDTIME) {
			NextLevel();
		}
	}

	if (_contactListener._isClickBtn) {
		if (!_bBtnClick) {
			_bBtnClick = true;
			_BtnBody->SetType(b2_staticBody);
		}
		else if (!_isDoorOpen) {
			_doorBody->SetType(b2_kinematicBody);
			_doorBody->SetLinearVelocity(b2Vec2(0, 1));
			float curY = _doorBody->GetWorldCenter().y * PTM_RATIO;
			if (curY >= _stopPosY) {
				_isDoorOpen = true;
				_doorBody->SetTransform(b2Vec2(_doorBody->GetWorldCenter().x, _stopPosY / PTM_RATIO), 0);
				_doorBody->SetType(b2_staticBody);
			}
		}
	}
	// 取得 _b2World 中所有的 body 進行處理
	// 最主要是根據目前運算的結果，更新附屬在 body 中 sprite 的位置
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		// 以下是以 Body 有包含 Sprite 顯示為例
		if (body->GetUserData() != NULL)
		{
			Sprite* bodyData = static_cast<Sprite*>(body->GetUserData());
			bodyData->setPosition(body->GetPosition().x * PTM_RATIO, body->GetPosition().y * PTM_RATIO);
			bodyData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
		}
	}

	platNode->clear();
	if (_plateBody != nullptr && _plateJoint != nullptr) {
		Vec2 vec1 = Vec2(_plateJoint->GetPosition().x * PTM_RATIO, _plateJoint->GetPosition().y * PTM_RATIO);
		Vec2 vec2 = Vec2(_plateBody->GetPosition().x * PTM_RATIO, _plateBody->GetPosition().y * PTM_RATIO);
		platNode->drawLine(vec1, vec2, Color4F::WHITE);
	}
	if (_plateBody1 != nullptr && _plateJoint1 != nullptr) {
		Vec2 vec1 = Vec2(_plateJoint1->GetPosition().x * PTM_RATIO, _plateJoint1->GetPosition().y * PTM_RATIO);
		Vec2 vec2 = Vec2(_plateBody1->GetPosition().x * PTM_RATIO, _plateBody1->GetPosition().y * PTM_RATIO);
		platNode->drawLine(vec1, vec2, Color4F::WHITE);
	}
}

void Level01::setObject()
{
	wallCount = WallCount;
	rectCount = rectWallCount;

	setInitObject();
	setGoalSensor();
	createStaticBoundary(wallCount, rectCount);

	_contactListener.setCollisionTarget(*_car->getCarSprite());

	platNode = DrawNode::create();
	this->addChild(platNode, 0);

	_ParticleSystem = new(nothrow)CParticleSystem();
	_ParticleSystem->init(*this);

	setPullJoint();
	setSeesaw();
	setButton();
	setMouseJoint();
}

void Level01::Replay() {
	// 先將這個 SCENE 的 update  從 schedule update 中移出
	this->unschedule(schedule_selector(Level01::update));
	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
	//  設定場景切換的特效
	TransitionFade* pageTurn = TransitionFade::create(1.0F, Level01::createScene());
	Director::getInstance()->replaceScene(pageTurn);
}

void Level01::NextLevel() {
	// 先將這個 SCENE 的 update  從 schedule update 中移出
	this->unschedule(schedule_selector(Level01::update));
	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
	//  設定場景切換的特效
	TransitionFade* pageTurn = TransitionFade::create(1.0F, Level02::createScene());
	Director::getInstance()->replaceScene(pageTurn);
}

void Level01::setPullJoint() {

	b2BodyDef staticDef;
	staticDef.type = b2_staticBody;
	staticDef.userData = NULL;

	b2CircleShape staticShape;
	staticShape.m_radius = 5 / PTM_RATIO;
	b2FixtureDef staticFixtureDef;
	staticFixtureDef.shape = &staticShape;

	//pulleyA
	auto boxSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("rect_pul01"));
	Point posA = boxSprite->getPosition();
	Size size = boxSprite->getContentSize();
	float scaleX = boxSprite->getScaleX();
	float scaleY = boxSprite->getScaleY();

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(posA.x / PTM_RATIO, posA.y / PTM_RATIO);
	bodyDef.userData = boxSprite;

	_plateBody = _b2World->CreateBody(&bodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox((size.width - 6) * scaleX * 0.5f / PTM_RATIO, (size.height - 6) * scaleY * 0.5f / PTM_RATIO);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 10.0f;
	fixtureDef.friction = 0.2f;
	_plateBody->CreateFixture(&fixtureDef);

	staticDef.position.Set(posA.x / PTM_RATIO, (posA.y + 200) / PTM_RATIO);
	_plateJoint = _b2World->CreateBody(&staticDef);
	_plateJoint->CreateFixture(&staticFixtureDef);

	b2PrismaticJointDef priJoint;
	priJoint.Initialize(_plateJoint, _plateBody, _plateBody->GetWorldCenter(), b2Vec2(0, 1));
	_b2World->CreateJoint(&priJoint);

	//pulleyB
	boxSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("rect_pul02"));
	Point posB = boxSprite->getPosition();
	size = boxSprite->getContentSize();
	scaleX = boxSprite->getScaleX();
	scaleY = boxSprite->getScaleY();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(posB.x / PTM_RATIO, posB.y / PTM_RATIO);
	bodyDef.userData = boxSprite;

	_plateBody1 = _b2World->CreateBody(&bodyDef);
	boxShape.SetAsBox((size.width - 6) * scaleX * 0.5f / PTM_RATIO, (size.height - 6) * scaleY * 0.5f / PTM_RATIO);

	fixtureDef.shape = &boxShape;
	fixtureDef.density = 10.0f;
	fixtureDef.friction = 0.2f;
	_plateBody1->CreateFixture(&fixtureDef);

	staticDef.position.Set(posB.x / PTM_RATIO, (posA.y + 200) / PTM_RATIO);
	_plateJoint1 = _b2World->CreateBody(&staticDef);
	_plateJoint1->CreateFixture(&staticFixtureDef);

	priJoint.Initialize(_plateJoint1, _plateBody1, _plateBody1->GetWorldCenter(), b2Vec2(0, 1.0f));
	_b2World->CreateJoint(&priJoint);

	b2Vec2 vec1 = b2Vec2(posA.x / PTM_RATIO, posA.y / PTM_RATIO);
	b2Vec2 vec2 = b2Vec2(posB.x / PTM_RATIO, posA.y / PTM_RATIO);
	b2PulleyJointDef jointDef;
	jointDef.Initialize(_plateBody, _plateBody1, vec1, vec2, _plateBody->GetWorldCenter(), _plateBody1->GetWorldCenter(), 1.0f);
	_b2World->CreateJoint(&jointDef);
}

void Level01::setSeesaw() {
	b2BodyDef bodyDef;
	bodyDef.userData = NULL;
	b2FixtureDef fixtureDef;

	//seesawBase
	auto baseSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("seesawBase"));
	Point pos = baseSprite->getPosition();
	Size size = baseSprite->getContentSize();
	float scaleX = baseSprite->getScaleX();
	float scaleY = baseSprite->getScaleY();

	Point lep[3], wep[3];
	lep[0].x = 0; lep[0].y = (size.height - 2) / 2.0f;
	lep[1].x = -(size.width - 2) / 2.0f; lep[1].y = -(size.height - 2) / 2.0f;
	lep[2].x = (size.width - 2) / 2.0f; lep[2].y = -(size.height - 2) / 2.0f;

	cocos2d::Mat4 modelMatrix, rotMatrix;
	modelMatrix.m[0] = scaleX;  // 先設定 X 軸的縮放
	modelMatrix.m[5] = scaleY;  // 先設定 Y 軸的縮放
	for (size_t j = 0; j < 3; j++)
	{   // 納入縮放與旋轉的 local space 的座標計算
		wep[j].x = lep[j].x * modelMatrix.m[0] + lep[j].y * modelMatrix.m[1];
		wep[j].y = lep[j].x * modelMatrix.m[4] + lep[j].y * modelMatrix.m[5];
	}
	b2Vec2 vecs[] = {
			b2Vec2(wep[0].x / PTM_RATIO, wep[0].y / PTM_RATIO),
			b2Vec2(wep[1].x / PTM_RATIO, wep[1].y / PTM_RATIO),
			b2Vec2(wep[2].x / PTM_RATIO, wep[2].y / PTM_RATIO)
	};
	b2PolygonShape triShape;
	triShape.Set(vecs, 3);
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);

	b2Body* seesawBaseBody = _b2World->CreateBody(&bodyDef);
	fixtureDef.shape = &triShape;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.1f;
	fixtureDef.restitution = 0.8f;
	seesawBaseBody->CreateFixture(&fixtureDef);

	//seesawBoard
	auto boardSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("seesawBoard"));
	pos = boardSprite->getPosition();
	size = boardSprite->getContentSize();
	scaleX = boardSprite->getScaleX();
	scaleY = boardSprite->getScaleY();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = boardSprite;

	b2Body* seesawBoardBody = _b2World->CreateBody(&bodyDef);

	b2PolygonShape seesawBoardShape;

	fixtureDef.shape = &seesawBoardShape;
	fixtureDef.density = 5.0f;
	fixtureDef.friction = 0.1f;
	fixtureDef.restitution = 0.1f;

	seesawBoardShape.SetAsBox((size.width - 4) * scaleX * 0.5f / PTM_RATIO, (size.height - 4) * scaleY * 0.5f / PTM_RATIO);
	seesawBoardBody->CreateFixture(&fixtureDef);

	//seesaw Joint
	b2RevoluteJointDef seesawJoint;
	seesawJoint.bodyA = seesawBaseBody;
	seesawJoint.localAnchorA.Set(0, 1.2f);
	seesawJoint.bodyB = seesawBoardBody;
	seesawJoint.localAnchorB.Set(0, 0);
	_b2World->CreateJoint(&seesawJoint);
}

void Level01::setButton() {
	//Btn Click
	_BtnSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("Btn_Click"));
	_contactListener._BtnSprite = _BtnSprite;
	Point pos = _BtnSprite->getPosition();
	Size size = _BtnSprite->getContentSize();
	float scaleX = _BtnSprite->getScaleX();
	float scaleY = _BtnSprite->getScaleY();

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = _BtnSprite;

	_BtnBody = _b2World->CreateBody(&bodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox((size.width - 4) * scaleX * 0.5f / PTM_RATIO, (size.height - 4) * scaleY * 0.5f / PTM_RATIO);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 6.0f;
	fixtureDef.friction = 0.25f;
	fixtureDef.restitution = 0;
	_BtnBody->CreateFixture(&fixtureDef);

	//Btn Click Joint
	b2BodyDef staticDef;
	staticDef.type = b2_staticBody;
	staticDef.userData = NULL;
	staticDef.position.Set((pos.x + 60) / PTM_RATIO, pos.y / PTM_RATIO);

	b2Body* ClickJoint = _b2World->CreateBody(&staticDef);

	boxShape.SetAsBox(1 * 0.5f / PTM_RATIO, 1 * 0.5f / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	ClickJoint->CreateFixture(&fixtureDef);

	b2PrismaticJointDef PriJoint;
	PriJoint.Initialize(ClickJoint, _BtnBody, _BtnBody->GetWorldCenter(), b2Vec2(1.0f, 0));
	_b2World->CreateJoint(&PriJoint);

	//Btn Sensor
	auto sensorSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("Btn_sensor"));
	pos = sensorSprite->getPosition();
	size = sensorSprite->getContentSize();
	scaleX = sensorSprite->getScaleX();
	scaleY = sensorSprite->getScaleY();

	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = NULL;

	b2Body* sensorBody = _b2World->CreateBody(&bodyDef);

	boxShape.SetAsBox((size.width - 4) * scaleX * 0.5f / PTM_RATIO, (size.height - 4) * scaleY * 0.5f / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 1001.0f;
	sensorBody->CreateFixture(&fixtureDef);

	//door
	auto doorSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("door"));
	pos = doorSprite->getPosition();
	size = doorSprite->getContentSize();
	scaleX = doorSprite->getScaleX();
	scaleY = doorSprite->getScaleY();
	_stopPosY = pos.y + 80.0f;

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = doorSprite;

	_doorBody = _b2World->CreateBody(&bodyDef);

	boxShape.SetAsBox((size.width - 4) * scaleX * 0.5f / PTM_RATIO, (size.height - 4) * scaleY * 0.5f / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 10.0f;
	fixtureDef.friction = 0.25f;
	fixtureDef.restitution = 0;
	_doorBody->CreateFixture(&fixtureDef);

	PriJoint.Initialize(ClickJoint, _doorBody, _doorBody->GetWorldCenter(), b2Vec2(0, 1.0f));
	_b2World->CreateJoint(&PriJoint);

	_bBtnClick = false;
	_isDoorOpen = false;
}

void Level01::setMouseJoint() {
	_frameSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("mouseJoint"));
	Point pos = _frameSprite->getPosition();
	Size size = _frameSprite->getContentSize();

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = _frameSprite;

	b2Body* body = _b2World->CreateBody(&bodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox((size.width - 4) * 0.5f / PTM_RATIO, (size.height - 4) * 0.5f / PTM_RATIO);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 5.0f;
	fixtureDef.restitution = 0.25f;
	fixtureDef.friction = 0.25f;
	body->CreateFixture(&fixtureDef);
	_bOnTouch = false;
}

bool Level01::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)//觸碰開始事件
{
	Point touchLoc = pTouch->getLocation();

	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		if (body->GetUserData() == NULL) continue; // 靜態物體不處理

		if (body->GetUserData() == _frameSprite) {
			Size size = _frameSprite->getContentSize();
			float fdistX = size.width / 2.0f;
			float fdistY = size.height / 2.0f;

			float x = body->GetPosition().x * PTM_RATIO - touchLoc.x;
			float y = body->GetPosition().y * PTM_RATIO - touchLoc.y;
			float tpdist = x * x + y * y;
			if (tpdist < fdistX * fdistY) {
				_bOnTouch = true;
				b2MouseJointDef mouseJointDef;
				mouseJointDef.bodyA = _bottomBody;
				mouseJointDef.bodyB = body;
				mouseJointDef.target = b2Vec2(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO);
				mouseJointDef.collideConnected = true;
				mouseJointDef.maxForce = 1000.0f * body->GetMass();
				_mouseJoint = dynamic_cast<b2MouseJoint*>(_b2World->CreateJoint(&mouseJointDef));
				body->SetAwake(true);
				break;
			}
		}
	}

	if (!_bOnTouch) {
		if (_leftBtn->onTouchBegan(touchLoc))
			_car->setState(LEFT);
		else if (_rightBtn->onTouchBegan(touchLoc))
			_car->setState(RIGHT);
		else if (_retryBtn->onTouchBegan(touchLoc)) {

		}
		else {
			_draw->onTouchBegan(touchLoc);
			_ParticleSystem->setColor(_draw->getPenColor());
			_ParticleSystem->onTouchBegan(touchLoc);
		}
			
	}

	return true;
}

void  Level01::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //觸碰移動事件
{
	Point touchLoc = pTouch->getLocation();

	if (_bOnTouch)
		_mouseJoint->SetTarget(b2Vec2(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO));
	else {
		if (_leftBtn->onTouchMoved(touchLoc))
			_car->setState(LEFT);
		else if (_rightBtn->onTouchMoved(touchLoc))
			_car->setState(RIGHT);
		else if (_retryBtn->onTouchMoved(touchLoc)) {

		}
		else {
			_draw->onTouchMoved(touchLoc);
			//_ParticleSystem->setColor(_draw->getPenColor());
			_ParticleSystem->setColor(_draw->getPenColor());
			_ParticleSystem->onTouchMoved(touchLoc);
		}
	}
}

void  Level01::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //觸碰結束事件 
{
	Point touchLoc = pTouch->getLocation();

	if (_bOnTouch) {
		_bOnTouch = false;
		if (_mouseJoint != NULL) {
			_b2World->DestroyJoint(_mouseJoint);
			_mouseJoint = NULL;
		}
	}
	else {
		if (_leftBtn->onTouchEnded(touchLoc))
			_car->setState(STOP);
		else if (_rightBtn->onTouchEnded(touchLoc))
			_car->setState(STOP);
		else if (_retryBtn->onTouchEnded(touchLoc)) {
			Replay();
		}
		else
			_draw->onTouchEnded(touchLoc);
	}
}

CLevel01ContactListener::CLevel01ContactListener()
{
	_carSprite = nullptr;
	_isFinish = false;

	_BtnSprite = nullptr;
	_isClickBtn = false;
}

//
// 只要是兩個 body 的 fixtures 碰撞，就會呼叫這個函式
//
void CLevel01ContactListener::BeginContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();

	// 車子是否碰到目的地
	if (BodyA->GetFixtureList()->GetDensity() == 1000.0f && BodyB->GetUserData() == _carSprite) {
		_isFinish = true;
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 1000.0f && BodyA->GetUserData() == _carSprite) {
		_isFinish = true;
	}

	//是否按下按鈕
	if (BodyA->GetFixtureList()->GetDensity() == 1001.0f && BodyB->GetUserData() == _BtnSprite) {
		_isClickBtn = true;
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 1001.0f && BodyA->GetUserData() == _BtnSprite) {
		_isClickBtn = true;
	}
}

//碰撞結束
void CLevel01ContactListener::EndContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();

}

void CLevel01ContactListener::setCollisionTarget(cocos2d::Sprite& targetSprite) {
	_carSprite = &targetSprite;
}