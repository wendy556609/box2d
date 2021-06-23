#include "Level01.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

#define MAX_2(X,Y) (X)>(Y) ? (X) : (Y)

using namespace cocostudio::timeline;

Level01::~Level01()
{
#ifdef BOX2D_DEBUG
	if (_DebugDraw != NULL) delete _DebugDraw;
#endif

	if (_b2World != nullptr) delete _b2World;
	//  for releasing Plist&Texture
	//	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
	Director::getInstance()->getTextureCache()->removeUnusedTextures();

	CC_SAFE_DELETE(_car);
	CC_SAFE_DELETE(_draw);
	CC_SAFE_DELETE(_leftBtn);
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

#ifdef BOX2D_DEBUG
	auto background = dynamic_cast<Node*>(_csbRoot->getChildByName("background"));
	background->setVisible(false);
	#endif

	// 建立 Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//重力方向
	bool AllowSleep = true;			//允許睡著
	_b2World = new (std::nothrow) b2World(Gravity);	//創建世界
	_b2World->SetAllowSleeping(AllowSleep);	//設定物件允許睡著

	//繪圖
	_draw = new (nothrow) CDraw();
	_draw->init(*_b2World, *this);

	auto btn = dynamic_cast<Sprite*>(_csbRoot->getChildByName("btn_left"));
	btn->setVisible(false);
	_leftBtn= new (nothrow) CButton();
	_leftBtn->init("runnormal.png", "runon.png", btn->getPosition(), *this);
	_leftBtn->setIcon("arrow.png", false);

	btn = dynamic_cast<Sprite*>(_csbRoot->getChildByName("btn_right"));
	btn->setVisible(false);
	_rightBtn = new (nothrow) CButton();
	_rightBtn->init("runnormal.png", "runon.png", btn->getPosition(), *this);
	_rightBtn->setIcon("arrow.png", true);

	_car = new (nothrow) CCar();
	_car->init(*_csbRoot, *_b2World);

	_contactListener.setCollisionTarget(*_car->getCarSprite());

	setGoalSensor();
	setPullJoint();
	setSeesaw();
	setMouseJoint();
	setButton();

	createStaticBoundary();

#ifdef BOX2D_DEBUG
	//DebugDrawInit
	_DebugDraw = nullptr;
	_DebugDraw = new GLESDebugDraw(PTM_RATIO);
	//設定DebugDraw
	_b2World->SetDebugDraw(_DebugDraw);
	//選擇繪製型別
	uint32 flags = 0;
	flags += GLESDebugDraw::e_shapeBit;						//繪製形狀
	flags += GLESDebugDraw::e_pairBit;
	flags += GLESDebugDraw::e_jointBit;
	flags += GLESDebugDraw::e_centerOfMassBit;
	flags += GLESDebugDraw::e_aabbBit;
	//設定繪製類型
	_DebugDraw->SetFlags(flags);
#endif

	_b2World->SetContactListener(&_contactListener);

	auto listener = EventListenerTouchOneByOne::create();	//創建一個一對一的事件聆聽器
	listener->onTouchBegan = CC_CALLBACK_2(Level01::onTouchBegan, this);		//加入觸碰開始事件
	listener->onTouchMoved = CC_CALLBACK_2(Level01::onTouchMoved, this);		//加入觸碰移動事件
	listener->onTouchEnded = CC_CALLBACK_2(Level01::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(Level01::update));

	return true;
}


void Level01::readBlocksCSBFile(const char* csbfilename)
{
	auto csbRoot = CSLoader::createNode(csbfilename);
	csbRoot->setPosition(_visibleSize.width / 2.0f, _visibleSize.height / 2.0f);
	addChild(csbRoot, 1);
	for (size_t i = 1; i <= 3; i++)
	{
		// 產生所需要的 Sprite file name int plist 
		std::ostringstream ostr;
		std::string objname;
		ostr << "block1_0" << i; objname = ostr.str();
	}
}

void Level01::readSceneFile(const char* csbfilename)
{
	auto csbRoot = CSLoader::createNode(csbfilename);
	csbRoot->setPosition(_visibleSize.width / 2.0f, _visibleSize.height / 2.0f);
	addChild(csbRoot, 1);
	char tmp[20] = "";
	for (size_t i = 1; i <= 12; i++)
	{
		// 產生所需要的 Sprite file name int plist 
		std::ostringstream ostr;
		std::string objname;
		ostr << "XXX_"; ostr.width(2); ostr.fill('0'); ostr << i; objname = ostr.str();
		//		sprintf(tmp, "XXX_%02d", i);
	}
}

void Level01::update(float dt)
{
	int velocityIterations = 8;	// 速度迭代次數
	int positionIterations = 1; // 位置迭代次數
	// Instruct the world to perform a single step of simulation.
	// It is generally best to keep the time step and iterations fixed.
	_b2World->Step(dt, velocityIterations, positionIterations);

	_car->update(dt);

	if (_contactListener._isFinish) {
		_car->setFinish(_goalPos);
		_stopLight->setSpriteFrame("orange05.png");
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
			Point move;
			//move = Point((body->GetPosition().x + 0.05f), body->GetPosition().y);
			//body->SetTransform(b2Vec2(move.x, move.y), body->GetAngle());
			Sprite* bodyData = static_cast<Sprite*>(body->GetUserData());
			bodyData->setPosition(body->GetPosition().x * PTM_RATIO, body->GetPosition().y * PTM_RATIO);
			bodyData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
		}
	}
}

void Level01::setGoalSensor() {
	auto sensorSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("car_sensor"));
	_goalPos = sensorSprite->getPosition();
	Size  size = sensorSprite->getContentSize();

	b2BodyDef sensorBodyDef;
	sensorBodyDef.position.Set(_goalPos.x / PTM_RATIO, _goalPos.y / PTM_RATIO);
	sensorBodyDef.type = b2_staticBody;
	sensorBodyDef.userData = sensorSprite;

	b2Body* sensorBody = _b2World->CreateBody(&sensorBodyDef);
	b2PolygonShape sensorShape;
	sensorShape.SetAsBox((size.width - 4) * 0.5f / PTM_RATIO, (size.height - 4) * 0.5f / PTM_RATIO);

	b2FixtureDef sensorFixDef;
	sensorFixDef.shape = &sensorShape;
	sensorFixDef.isSensor = true;
	sensorFixDef.density = 1000.0f;
	sensorBody->CreateFixture(&sensorFixDef);

	auto lightSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("stopLight"));
	lightSprite->setVisible(false);
	_stopLight = Sprite::createWithSpriteFrameName("orange02.png");
	_stopLight->setPosition(lightSprite->getPosition());
	_stopLight->setScale(lightSprite->getScale());
	addChild(_stopLight, 3);
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
	auto boxSprite= dynamic_cast<Sprite*>(_csbRoot->getChildByName("rect_pul01"));
	Point posA = boxSprite->getPosition();
	Size size = boxSprite->getContentSize();
	float scaleX = boxSprite->getScaleX();
	float scaleY = boxSprite->getScaleY();

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(posA.x / PTM_RATIO, posA.y / PTM_RATIO);
	bodyDef.userData = boxSprite;

	b2Body* bodyA = _b2World->CreateBody(&bodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox((size.width - 6) * scaleX * 0.5f / PTM_RATIO, (size.height - 6) * scaleY * 0.5f / PTM_RATIO);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 10.0f;
	fixtureDef.friction = 0.2f;
	bodyA->CreateFixture(&fixtureDef);

	staticDef.position.Set(posA.x / PTM_RATIO, (posA.y + 200) / PTM_RATIO);
	b2Body* staticBodyA = _b2World->CreateBody(&staticDef);
	staticBodyA->CreateFixture(&staticFixtureDef);

	b2PrismaticJointDef priJoint;
	priJoint.Initialize(staticBodyA, bodyA, bodyA->GetWorldCenter(), b2Vec2(0, 1));
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

	b2Body* bodyB = _b2World->CreateBody(&bodyDef);
	boxShape.SetAsBox((size.width - 6) * scaleX * 0.5f / PTM_RATIO, (size.height - 6) * scaleY * 0.5f / PTM_RATIO);

	fixtureDef.shape = &boxShape;
	fixtureDef.density = 10.0f;
	fixtureDef.friction = 0.2f;
	bodyB->CreateFixture(&fixtureDef);

	staticDef.position.Set(posB.x / PTM_RATIO, (posA.y + 200) / PTM_RATIO);
	b2Body* staticBodyB = _b2World->CreateBody(&staticDef);
	staticBodyB->CreateFixture(&staticFixtureDef);

	priJoint.Initialize(staticBodyB, bodyB, bodyB->GetWorldCenter(), b2Vec2(0, 1.0f));
	_b2World->CreateJoint(&priJoint);

	b2Vec2 vec1= b2Vec2(posA.x / PTM_RATIO, posA.y / PTM_RATIO);
	b2Vec2 vec2 = b2Vec2(posB.x / PTM_RATIO, posA.y / PTM_RATIO);
	b2PulleyJointDef jointDef;
	jointDef.Initialize(bodyA, bodyB, vec1, vec2, bodyA->GetWorldCenter(), bodyB->GetWorldCenter(), 1.0f);
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
	boxShape.SetAsBox((size.width - 4)* scaleX * 0.5f / PTM_RATIO, (size.height - 4 ) * scaleY * 0.5f / PTM_RATIO);

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
		else
			_draw->onTouchBegan(touchLoc);
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
		else
			_draw->onTouchMoved(touchLoc);
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
		else
			_draw->onTouchEnded(touchLoc);
	}
}

void Level01::createStaticBoundary()
{
	// 先產生 Body, 設定相關的參數
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.userData = NULL;

	b2Body* body = _b2World->CreateBody(&bodyDef);

	b2EdgeShape edgeShape;
	b2FixtureDef edgeFixtureDef;
	edgeFixtureDef.shape = &edgeShape;
	edgeFixtureDef.density = 1.0f; edgeFixtureDef.friction = 0.25f; edgeFixtureDef.restitution = 0.25f;

	//bottom edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, 0.0f / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);
	_bottomBody = body;

	//left edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(0.0f / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	//top edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, _visibleSize.height / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	//right edge
	edgeShape.Set(b2Vec2(_visibleSize.width / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	for (size_t i = 1; i <= WallCount; i++)
	{
		std::ostringstream ostr;
		std::string objname;
		ostr << "wall1_"; ostr.width(2); ostr.fill('0'); ostr << i;
		objname = ostr.str();

		auto wall = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		Point pos = wall->getPosition();
		Size size = wall->getContentSize();
		float angle = wall->getRotation();
		float scale = wall->getScaleX();

		Point lep1, lep2, wep1, wep2;
		lep1.y = 0; lep1.x = -(size.width - 4) / 2.0f;
		lep2.y = 0; lep2.x = (size.width - 4) / 2.0f;

		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scale;
		cocos2d::Mat4::createRotationZ(angle * M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = pos.x;
		modelMatrix.m[7] = pos.y;

		wep1.x = lep1.x * modelMatrix.m[0] + lep1.y * modelMatrix.m[1] + modelMatrix.m[3];
		wep1.y = lep1.x * modelMatrix.m[4] + lep1.y * modelMatrix.m[5] + modelMatrix.m[7];
		wep2.x = lep2.x * modelMatrix.m[0] + lep2.y * modelMatrix.m[1] + modelMatrix.m[3];
		wep2.y = lep2.x * modelMatrix.m[4] + lep2.y * modelMatrix.m[5] + modelMatrix.m[7];

		edgeShape.Set(b2Vec2(wep1.x / PTM_RATIO, wep1.y / PTM_RATIO), b2Vec2(wep2.x / PTM_RATIO, wep2.y / PTM_RATIO));
		body->CreateFixture(&edgeFixtureDef);
	}

	for (size_t i = 1; i <= rectWallCount; i++)
	{
		std::ostringstream ostr;
		std::string objname;
		ostr << "rect1_"; ostr.width(2); ostr.fill('0'); ostr << i;
		objname = ostr.str();

		auto box = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		Point pos = box->getPosition();
		Size size = box->getContentSize();
		float scaleX = box->getScaleX();
		float scaleY = box->getScaleY();

		bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);

		body = _b2World->CreateBody(&bodyDef);

		b2FixtureDef fixtureDef;
		fixtureDef.density = 1.0f; fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
		b2PolygonShape boxShape;
		boxShape.SetAsBox((size.width - 4) * scaleX * 0.5f / PTM_RATIO, (size.height - 4) * scaleY * 0.5f / PTM_RATIO);
		fixtureDef.shape = &boxShape;
		body->CreateFixture(&fixtureDef);
	}
}

CContactListener::CContactListener()
{
	_carSprite = nullptr;
	_BtnSprite = nullptr;

	_isFinish = false;
	_isClickBtn = false;
}

//
// 只要是兩個 body 的 fixtures 碰撞，就會呼叫這個函式
//
void CContactListener::BeginContact(b2Contact* contact)
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
void CContactListener::EndContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();

}

void CContactListener::setCollisionTarget(cocos2d::Sprite& targetSprite) {
	_carSprite = &targetSprite;
}

#ifdef BOX2D_DEBUG
//改寫繪製方法
void Level01::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif