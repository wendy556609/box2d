#include "Level02.h"

Level02::~Level02()
{
	
}

Scene* Level02::createScene()
{
	return Level02::create();
}

// on "init" you need to initialize your instance
bool Level02::init()
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

	_csbRoot = CSLoader::createNode("Level02.csb");

	addChild(_csbRoot, 1);

	setObject();

	_b2World->SetContactListener(&_contactListener);

	auto listener = EventListenerTouchOneByOne::create();	//創建一個一對一的事件聆聽器
	listener->onTouchBegan = CC_CALLBACK_2(Level02::onTouchBegan, this);		//加入觸碰開始事件
	listener->onTouchMoved = CC_CALLBACK_2(Level02::onTouchMoved, this);		//加入觸碰移動事件
	listener->onTouchEnded = CC_CALLBACK_2(Level02::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(Level02::update));

	return true;
}

void Level02::update(float dt)
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
		_endCount += dt;
		if (_endCount >= ENDTIME) {
			NextLevel();
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

	if (_contactListener._isCut) {
		_contactListener._isCut = false;
		if (_contactListener.ropeNum != 99) {
			_b2World->DestroyJoint(ropeJoint);
			_b2World->DestroyJoint(ropeReJoint[_contactListener.ropeNum]);
			_contactListener.ropeNum = 99;
		}
		else if (_contactListener.rope2Num != 99) {
			_b2World->DestroyJoint(ropeJoint2);
			_b2World->DestroyJoint(ropeReJoint2[_contactListener.rope2Num]);
			_contactListener.rope2Num = 99;
		}
	}
}

void Level02::setObject()
{
	wallCount = WallCount;
	rectCount = rectWallCount;

	auto btn = dynamic_cast<Sprite*>(_csbRoot->getChildByName("runnormal_red"));
	btn->setVisible(false);
	_redBtn = new(nothrow)CButton();
	_redBtn->init("runnormal.png", "runon.png", btn->getPosition(), *this, Switch);
	_redBtn->setColor(Color3B::RED);
	_redBtn->setScale(btn->getScale());

	btn = dynamic_cast<Sprite*>(_csbRoot->getChildByName("runnormal_green"));
	btn->setVisible(false);
	_greenBtn = new(nothrow)CButton();
	_greenBtn->init("runnormal.png", "runon.png", btn->getPosition(), *this, Switch);
	_greenBtn->setColor(Color3B::GREEN);
	_greenBtn->setScale(btn->getScale());

	btn = dynamic_cast<Sprite*>(_csbRoot->getChildByName("runnormal_blue"));
	btn->setVisible(false);
	_blueBtn = new(nothrow)CButton();
	_blueBtn->init("runnormal.png", "runon.png", btn->getPosition(), *this, Switch);
	_blueBtn->setColor(Color3B::BLUE);
	_blueBtn->setScale(btn->getScale());

	setInitObject();
	setGoalSensor();
	createStaticBoundary(wallCount, rectCount);

	_contactListener.setCollisionTarget(*_car->getCarSprite());

	setPullJoint();
	setMouseJoint();
	setRopeJoint();
	setFilter();
}

void Level02::Replay() {
	// 先將這個 SCENE 的 update  從 schedule update 中移出
	this->unschedule(schedule_selector(Level02::update));
	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
	//  設定場景切換的特效
	TransitionFade* pageTurn = TransitionFade::create(1.0F, Level02::createScene());
	Director::getInstance()->replaceScene(pageTurn);
}

void Level02::NextLevel() {
	// 先將這個 SCENE 的 update  從 schedule update 中移出
	this->unschedule(schedule_selector(Level02::update));
	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
	//  設定場景切換的特效
	TransitionFade* pageTurn = TransitionFade::create(1.0F, Level03::createScene());
	Director::getInstance()->replaceScene(pageTurn);
}

void Level02::setPullJoint() {

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

	b2Body* bodyA = _b2World->CreateBody(&bodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox((size.width - 6) * scaleX * 0.5f / PTM_RATIO, (size.height - 6) * scaleY * 0.5f / PTM_RATIO);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 10.0f;
	fixtureDef.friction = 0.2f;
	bodyA->CreateFixture(&fixtureDef);

	staticDef.position.Set(posA.x / PTM_RATIO, (posA.y + 500) / PTM_RATIO);
	b2Body* staticBodyA = _b2World->CreateBody(&staticDef);
	staticBodyA->CreateFixture(&staticFixtureDef);

	b2PrismaticJointDef priJoint;
	priJoint.Initialize(staticBodyA, bodyA, bodyA->GetWorldCenter(), b2Vec2(0, 1));
	_b2World->CreateJoint(&priJoint);

	//pulleyB
	boxSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("filter1_03"));
	Point posB = boxSprite->getPosition();
	size = boxSprite->getContentSize();
	scaleX = boxSprite->getScaleX();
	scaleY = boxSprite->getScaleY();
	boxSprite->setColor(Color3B::BLUE);
	boxSprite->setOpacity(128);

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(posB.x / PTM_RATIO, posB.y / PTM_RATIO);
	bodyDef.userData = boxSprite;

	b2Body* bodyB = _b2World->CreateBody(&bodyDef);
	boxShape.SetAsBox((size.width - 6) * scaleX * 0.5f / PTM_RATIO, (size.height - 6) * scaleY * 0.5f / PTM_RATIO);

	fixtureDef.shape = &boxShape;
	fixtureDef.density = 10.0f;
	fixtureDef.friction = 0.2f;
	fixtureDef.filter.maskBits = 1 << 4 | 1;
	bodyB->CreateFixture(&fixtureDef);

	staticDef.position.Set(posB.x / PTM_RATIO, (posA.y + 500) / PTM_RATIO);
	b2Body* staticBodyB = _b2World->CreateBody(&staticDef);
	staticBodyB->CreateFixture(&staticFixtureDef);

	//平移Joint
	priJoint.Initialize(staticBodyB, bodyB, bodyB->GetWorldCenter(), b2Vec2(0, 1.0f));
	_b2World->CreateJoint(&priJoint);

	//Pulley Joint
	b2Vec2 vec1 = b2Vec2(posA.x / PTM_RATIO, (posA.y + 500) / PTM_RATIO);
	b2Vec2 vec2 = b2Vec2(posB.x / PTM_RATIO, (posB.y + 500) / PTM_RATIO);
	b2PulleyJointDef jointDef;
	jointDef.Initialize(bodyA, bodyB, vec1, vec2, bodyA->GetWorldCenter(), bodyB->GetWorldCenter(), 1.0f);
	_b2World->CreateJoint(&jointDef);
}

void Level02::setMouseJoint() {
	_cutSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("cut"));
	Point pos = _cutSprite->getPosition();
	Size size = _cutSprite->getContentSize();
	float scaleX = _cutSprite->getScaleX();
	float scaleY = _cutSprite->getScaleY();
	float angle = _cutSprite->getRotation();

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = _cutSprite;
	bodyDef.angle = (-angle) * M_PI / 180.0f;

	b2Body* body = _b2World->CreateBody(&bodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox((size.width - 4) * scaleX * 0.5f / PTM_RATIO, (size.height - 4) * scaleY * 0.5f / PTM_RATIO);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 5.0f;
	fixtureDef.restitution = 0.25f;
	fixtureDef.friction = 0.25f;
	body->CreateFixture(&fixtureDef);

	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, (pos.y + 300) / PTM_RATIO);
	bodyDef.userData = NULL;

	b2Body* circleBody = _b2World->CreateBody(&bodyDef);

	b2CircleShape circleShape;
	circleShape.m_radius = 1.0f / PTM_RATIO;

	fixtureDef.shape = &circleShape;
	fixtureDef.density = 1.0f;
	fixtureDef.restitution = 0;
	fixtureDef.friction = 0.25f;
	circleBody->CreateFixture(&fixtureDef);

	b2PrismaticJointDef preJoint;
	preJoint.Initialize(circleBody, body, body->GetWorldCenter(), b2Vec2(0, 0));
	_b2World->CreateJoint(&preJoint);

	_bOnTouch = false;

	/*---------------------cut sensor----------------------*/
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set((pos.x + 30.0f) / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = NULL;

	b2Body* sensor = _b2World->CreateBody(&bodyDef);

	boxShape.SetAsBox(1 / PTM_RATIO, 1 / PTM_RATIO);

	fixtureDef.shape = &boxShape;
	fixtureDef.density = 1001.0f;
	sensor->CreateFixture(&fixtureDef);

	b2RevoluteJointDef reJoint;
	reJoint.Initialize(sensor, body, sensor->GetWorldCenter());
	_b2World->CreateJoint(&reJoint);
}

void Level02::setRopeJoint() {
	/*------------orange-----------------*/
	//靜態物件
	auto staticSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("rope_static"));
	Point staticPos = staticSprite->getPosition();
	Size staticSize = staticSprite->getContentSize();
	float scaleX = staticSprite->getScaleX();
	float scaleY = staticSprite->getScaleY();

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(staticPos.x / PTM_RATIO, staticPos.y / PTM_RATIO);
	bodyDef.userData = staticSprite;

	b2Body* staticBody = _b2World->CreateBody(&bodyDef);
	b2FixtureDef fixtureDef;
	fixtureDef.density = 1.0f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	b2PolygonShape boxShape;
	boxShape.SetAsBox((staticSize.width - 4) * scaleX * 0.5f / PTM_RATIO, (staticSize.height - 4) * scaleY * 0.5f / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	staticBody->CreateFixture(&fixtureDef);

	//動態物件
	auto circleSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("rope"));
	Point circlePos = circleSprite->getPosition();
	Size circleSize = circleSprite->getContentSize();
	scaleX = circleSprite->getScaleX();
	scaleY = circleSprite->getScaleY();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(circlePos.x / PTM_RATIO, circlePos.y / PTM_RATIO);
	bodyDef.userData = circleSprite;

	b2Body* circleBody = _b2World->CreateBody(&bodyDef);
	b2CircleShape circleShape;
	circleShape.m_radius = (circleSize.width - 4) * scaleX * 0.5f / PTM_RATIO;
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 50.0f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	fixtureDef.filter.categoryBits = 1 << 1;
	circleBody->CreateFixture(&fixtureDef);

	//繩子關節
	b2RopeJointDef jointDef;
	jointDef.bodyA = staticBody;
	jointDef.bodyB = circleBody;
	jointDef.localAnchorA = b2Vec2(0, 0);
	jointDef.localAnchorB = b2Vec2(0, 20 / PTM_RATIO);
	jointDef.maxLength = (staticPos.y - circlePos.y - 30) / PTM_RATIO;
	jointDef.collideConnected = true;
	ropeJoint = dynamic_cast<b2RopeJoint*>(_b2World->CreateJoint(&jointDef));

	//繩子線段
	char tmp[20] = "";
	Sprite* ropeSprite[5] = { nullptr };
	Point pos[5];
	Size size[5];
	ropeBody[5] = { nullptr };

	bodyDef.type = b2_dynamicBody;
	fixtureDef.density = 0.01f;  fixtureDef.friction = 1.0f; fixtureDef.restitution = 0.0f;
	fixtureDef.shape = &boxShape;

	for (int i = 0; i < 5; i++)
	{
		std::ostringstream ostr;
		std::string objname;
		ostr << "rope01_"; ostr.width(2); ostr.fill('0');
		ostr << i + 1; objname = ostr.str();

		ropeSprite[i] = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		pos[i] = ropeSprite[i]->getPosition();
		size[i] = ropeSprite[i]->getContentSize();

		bodyDef.position.Set(pos[i].x / PTM_RATIO, pos[i].y / PTM_RATIO);
		bodyDef.userData = ropeSprite[i];
		ropeBody[i] = _b2World->CreateBody(&bodyDef);
		_contactListener.ropeBody[i] = ropeBody[i];
		boxShape.SetAsBox((size[i].width - 4) * 0.5f / PTM_RATIO, (size[i].height - 4) * 0.5f / PTM_RATIO);
		ropeBody[i]->CreateFixture(&fixtureDef);
	}

	//RevoluteJoint
	//連結staticBody和ropeBody[0]
	float locAnchor = 0.5f * (size[0].height - 5) / PTM_RATIO;
	b2RevoluteJointDef reJoint;
	reJoint.bodyA = staticBody;
	reJoint.localAnchorA.Set(0, -0.1f);
	reJoint.bodyB = ropeBody[0];
	reJoint.localAnchorB.Set(0, locAnchor);
	ropeReJoint[0] = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&reJoint));

	for (int i = 0; i < 4; i++)
	{
		reJoint.bodyA = ropeBody[i];
		reJoint.localAnchorA.Set(0, -locAnchor);
		reJoint.bodyB = ropeBody[i + 1];
		reJoint.localAnchorB.Set(0, locAnchor);
		ropeReJoint[i + 1] = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&reJoint));
	}
	reJoint.bodyA = ropeBody[4];
	reJoint.localAnchorA.Set(0, -locAnchor);
	reJoint.bodyB = circleBody;
	reJoint.localAnchorB.Set(0, 0.85f);
	ropeReJoint[4] = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&reJoint));

	/*------------platform-----------------*/
	//靜態物件
	staticSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("rope_static1"));
	staticPos = staticSprite->getPosition();
	staticSize = staticSprite->getContentSize();
	scaleX = staticSprite->getScaleX();
	scaleY = staticSprite->getScaleY();

	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(staticPos.x / PTM_RATIO, staticPos.y / PTM_RATIO);
	bodyDef.userData = staticSprite;

	staticBody = _b2World->CreateBody(&bodyDef);
	fixtureDef.density = 1.0f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;

	boxShape.SetAsBox((staticSize.width - 4) * scaleX * 0.5f / PTM_RATIO, (staticSize.height - 4) * scaleY * 0.5f / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	staticBody->CreateFixture(&fixtureDef);

	//動態物件
	auto boxSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("rope1"));
	Point boxPos = boxSprite->getPosition();
	Size boxSize = boxSprite->getContentSize();
	scaleX = boxSprite->getScaleX();
	scaleY = boxSprite->getScaleY();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(boxPos.x / PTM_RATIO, boxPos.y / PTM_RATIO);
	bodyDef.userData = boxSprite;

	b2Body* boxBody = _b2World->CreateBody(&bodyDef);
	
	boxShape.SetAsBox((boxSize.width - 4) * scaleX * 0.5f / PTM_RATIO, (boxSize.height - 4) * scaleY * 0.5f / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 1.0f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	boxBody->CreateFixture(&fixtureDef);

	//platform
	auto platSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("platForm"));
	Point platPos = platSprite->getPosition();
	Size platSize = platSprite->getContentSize();
	scaleX = platSprite->getScaleX();
	scaleY = platSprite->getScaleY();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(platPos.x / PTM_RATIO, platPos.y / PTM_RATIO);
	bodyDef.userData = platSprite;

	b2Body* platBody = _b2World->CreateBody(&bodyDef);

	boxShape.SetAsBox((platSize.width - 4) * scaleX * 0.5f / PTM_RATIO, (platSize.height - 4) * scaleY * 0.5f / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 1.0f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	platBody->CreateFixture(&fixtureDef);

	b2PrismaticJointDef priJoint;
	priJoint.Initialize(staticBody, platBody, platBody->GetWorldCenter(), b2Vec2(0, 1));
	_b2World->CreateJoint(&priJoint);

	//middle static
	staticSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("rope2"));
	Point staticPos1 = staticSprite->getPosition();
	staticSize = staticSprite->getContentSize();
	scaleX = staticSprite->getScaleX();
	scaleY = staticSprite->getScaleY();
	float angle = staticSprite->getRotation();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(staticPos1.x / PTM_RATIO, staticPos1.y / PTM_RATIO);
	bodyDef.userData = staticSprite;
	bodyDef.angle = (-angle) * M_PI / 180.0f;

	b2Body* staticBody1 = _b2World->CreateBody(&bodyDef);
	fixtureDef.density = 0.5f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	boxShape.SetAsBox((staticSize.width - 4) * scaleX * 0.5f / PTM_RATIO, (staticSize.height - 4) * scaleY * 0.5f / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	staticBody1->CreateFixture(&fixtureDef);

	reJoint.Initialize(staticBody1, platBody, (platBody->GetWorldCenter() - b2Vec2(0, 70 / PTM_RATIO)));
	_b2World->CreateJoint(&reJoint);

	reJoint.Initialize(staticBody1, platBody, (platBody->GetWorldCenter() + b2Vec2(0, 70 / PTM_RATIO)));
	_b2World->CreateJoint(&reJoint);

	reJoint.Initialize(staticBody1, boxBody, (boxBody->GetWorldCenter() - b2Vec2(0, 70 / PTM_RATIO)));
	_b2World->CreateJoint(&reJoint);

	reJoint.Initialize(staticBody1, boxBody, (boxBody->GetWorldCenter() + b2Vec2(0, 70 / PTM_RATIO)));
	_b2World->CreateJoint(&reJoint);

	//繩子關節
	jointDef.bodyA = staticBody;
	jointDef.bodyB = boxBody;
	jointDef.localAnchorA = b2Vec2(-20.0f / PTM_RATIO, 0);
	jointDef.localAnchorB = b2Vec2(0, 30.0f / PTM_RATIO);
	jointDef.maxLength = (staticPos.y - boxPos.y - 30) / PTM_RATIO;
	jointDef.collideConnected = true;
	ropeJoint2 = dynamic_cast<b2RopeJoint*>(_b2World->CreateJoint(&jointDef));

	//繩子線段
	Sprite* ropeSprite2[8] = { nullptr };
	Point pos2[8];
	Size size2[8];
	ropeBody2[8] = { nullptr };

	bodyDef.type = b2_dynamicBody;
	fixtureDef.density = 0.01f;  fixtureDef.friction = 1.0f; fixtureDef.restitution = 0.0f;
	fixtureDef.shape = &boxShape;

	for (int i = 0; i < 8; i++)
	{
		std::ostringstream ostr;
		std::string objname;
		ostr << "rope02_"; ostr.width(2); ostr.fill('0');
		ostr << i + 1; objname = ostr.str();

		ropeSprite2[i] = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		pos2[i] = ropeSprite2[i]->getPosition();
		size2[i] = ropeSprite2[i]->getContentSize();

		bodyDef.position.Set(pos2[i].x / PTM_RATIO, pos2[i].y / PTM_RATIO);
		bodyDef.userData = ropeSprite2[i];
		ropeBody2[i] = _b2World->CreateBody(&bodyDef);
		_contactListener.ropeBody2[i] = ropeBody2[i];
		boxShape.SetAsBox((size2[i].width - 4) * 0.5f / PTM_RATIO, (size2[i].height - 4) * 0.5f / PTM_RATIO);
		ropeBody2[i]->CreateFixture(&fixtureDef);
	}

	//RevoluteJoint
	//連結staticBody和ropeBody[0]
	locAnchor = 0.5f * (size[0].height - 5) / PTM_RATIO;

	reJoint.bodyA = staticBody;
	reJoint.localAnchorA.Set(-0.6f, -0.6f);
	reJoint.bodyB = ropeBody2[0];
	reJoint.localAnchorB.Set(0, locAnchor);
	ropeReJoint2[0] = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&reJoint));

	for (int i = 0; i < 7; i++)
	{
		reJoint.bodyA = ropeBody2[i];
		reJoint.localAnchorA.Set(0, -locAnchor);
		reJoint.bodyB = ropeBody2[i + 1];
		reJoint.localAnchorB.Set(0, locAnchor);
		ropeReJoint2[i + 1] = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&reJoint));
	}
	reJoint.bodyA = ropeBody2[7];
	reJoint.localAnchorA.Set(0, -locAnchor);
	reJoint.bodyB = boxBody;
	reJoint.localAnchorB.Set(0, 0.1f);
	ropeReJoint2[7] = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&reJoint));


}

void Level02::setFilter() {
	Color3B filterColor[2] = { Color3B::RED, Color3B::GREEN };

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	b2Body* body;

	b2FixtureDef fixtureDef;
	b2PolygonShape boxShape;
	fixtureDef.shape = &boxShape;

	std::ostringstream ostr;
	std::string objname;

	for (int i = 1; i <= 2; i++)
	{
		ostr.str("");
		ostr << "filter1_0" << i; objname = ostr.str();

		auto rectSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		bodyDef.userData = rectSprite;
		rectSprite->setColor(filterColor[i - 1]);
		rectSprite->setOpacity(128);
		Size size = rectSprite->getContentSize();
		Point pos = rectSprite->getPosition();
		float scaleX = rectSprite->getScaleX();
		float scaleY = rectSprite->getScaleY();

		bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO); // 設定板子所在的位置
		body = _b2World->CreateBody(&bodyDef);
		float bw = (size.width - 4) * scaleX;
		float bh = (size.height - 4) * scaleY;
		boxShape.SetAsBox(bw * 0.5f / PTM_RATIO, bh * 0.5f / PTM_RATIO);

		if(i==1)
			fixtureDef.filter.maskBits = 1 << 2 | 1;
		else 
			fixtureDef.filter.maskBits = 1 << 3 | 1;
		body->CreateFixture(&fixtureDef);
	}
}

bool Level02::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)//觸碰開始事件
{
	Point touchLoc = pTouch->getLocation();

	//MouseJoint
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		if (body->GetUserData() == NULL) continue; // 靜態物體不處理

		if (body->GetUserData() == _cutSprite) {
			Size size = _cutSprite->getContentSize();
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
		//Btn
		if (_leftBtn->onTouchBegan(touchLoc))
			_car->setState(LEFT);
		else if (_rightBtn->onTouchBegan(touchLoc))
			_car->setState(RIGHT);
		else if (_retryBtn->onTouchBegan(touchLoc)) {
			
		}
		else if (_redBtn->onTouchBegan(touchLoc)) {

		}
		else if (_greenBtn->onTouchBegan(touchLoc)) {

		}
		else if (_blueBtn->onTouchBegan(touchLoc)) {

		}
		else
			_draw->onTouchBegan(touchLoc);//繪圖
	}

	return true;
}

void  Level02::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //觸碰移動事件
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
		else if (_redBtn->onTouchMoved(touchLoc)) {

		}
		else if (_greenBtn->onTouchMoved(touchLoc)) {

		}
		else if (_blueBtn->onTouchMoved(touchLoc)) {

		}
		else
			_draw->onTouchMoved(touchLoc);
	}
}

void  Level02::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //觸碰結束事件 
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
		else if (_redBtn->onTouchEnded(touchLoc)) {
			if (_redBtn->getState()) {
				setPenColor(Color4F::RED, 2);
				if (_greenBtn->getState())
					_greenBtn->switchBtn();
				if (_blueBtn->getState())
					_blueBtn->switchBtn();
			}
			else
				setPenColor(Color4F::BLACK, 1);
		}
		else if (_greenBtn->onTouchEnded(touchLoc)) {
			if (_greenBtn->getState()) {
				setPenColor(Color4F::GREEN, 3);
				if (_redBtn->getState())
					_redBtn->switchBtn();
				if (_blueBtn->getState())
					_blueBtn->switchBtn();
			}
			else
				setPenColor(Color4F::BLACK, 1);
		}
		else if (_blueBtn->onTouchEnded(touchLoc)) {
			if (_blueBtn->getState()) {
				setPenColor(Color4F::BLUE, 4);
				if (_redBtn->getState())
					_redBtn->switchBtn();
				if (_greenBtn->getState())
					_greenBtn->switchBtn();
			}
			else
				setPenColor(Color4F::BLACK, 1);
		}
		else
			_draw->onTouchEnded(touchLoc);
	}
}

CLevel02ContactListener::CLevel02ContactListener()
{
	_carSprite = nullptr;

	_isFinish = false;
	_isCut = false;

	ropeNum = 99;
	rope2Num = 99;
}

//
// 只要是兩個 body 的 fixtures 碰撞，就會呼叫這個函式
//
void CLevel02ContactListener::BeginContact(b2Contact* contact)
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

	//剪刀碰到繩子
	if (BodyA->GetFixtureList()->GetDensity() == 1001.0f && BodyB->GetFixtureList()->GetDensity() == 0.01f) {
		for (int i = 0; i < 5; i++)
		{
			if (BodyB == ropeBody[i]) {
				_isCut = true;
				ropeNum = i;
				break;
			}
		}

		for (int i = 0; i < 8; i++)
		{
			if (BodyB == ropeBody2[i]) {
				_isCut = true;
				rope2Num = i;
				break;
			}
		}
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 1001.0f && BodyA->GetFixtureList()->GetDensity() == 0.01f) {
		for (int i = 0; i < 5; i++)
		{
			if (BodyA == ropeBody[i]) {
				_isCut = true;
				ropeNum = i;
				break;
			}
		}

		for (int i = 0; i < 8; i++)
		{
			if (BodyA == ropeBody2[i]) {
				_isCut = true;
				rope2Num = i;
				break;
			}
		}
	}

}

//碰撞結束
void CLevel02ContactListener::EndContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();

}

void CLevel02ContactListener::setCollisionTarget(cocos2d::Sprite& targetSprite) {
	_carSprite = &targetSprite;
}