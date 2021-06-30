#include "Level04.h"

Level04::~Level04()
{
	CC_SAFE_DELETE(_ParticleSystem);
}

Scene* Level04::createScene()
{
	return Level04::create();
}

// on "init" you need to initialize your instance
bool Level04::init()
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

	_csbRoot = CSLoader::createNode("Level04.csb");

	addChild(_csbRoot, 1);

	setObject();

	_b2World->SetContactListener(&_contactListener);

	auto listener = EventListenerTouchOneByOne::create();	//創建一個一對一的事件聆聽器
	listener->onTouchBegan = CC_CALLBACK_2(Level04::onTouchBegan, this);		//加入觸碰開始事件
	listener->onTouchMoved = CC_CALLBACK_2(Level04::onTouchMoved, this);		//加入觸碰移動事件
	listener->onTouchEnded = CC_CALLBACK_2(Level04::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(Level04::update));

	return true;
}

void Level04::update(float dt)
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

	if (_contactListener._isClickBtn) {
		if (!_bBtnClick) {
			_bBtnClick = true;
			_BtnBody->SetType(b2_staticBody);
		}
		else if (!_isdoorOpen) {
			_doorBody->SetType(b2_kinematicBody);
			_doorBody->SetLinearVelocity(b2Vec2(1, 0));
			float curX = _doorBody->GetWorldCenter().x * PTM_RATIO;
			if (curX >= _stopPosX) {
				_isdoorOpen = true;
				_doorBody->SetLinearVelocity(b2Vec2(0, 0));
				_doorBody->SetTransform(b2Vec2(_stopPosX / PTM_RATIO, _doorBody->GetWorldCenter().y), 0);
				_doorBody->SetType(b2_kinematicBody);
			}
		}
	}

	if (!_isStop) {
		float curY = _autoPlateBody->GetPosition().y * PTM_RATIO;
		switch (_Type)
		{
		case 0:
			if (curY > _initPosY) {
				_autoPlateBody->SetLinearVelocity(b2Vec2(0, -2));
			}
			else {
				_autoPlateBody->SetLinearVelocity(b2Vec2(0, 0));
				b2Vec2 vec = b2Vec2(_autoPlateBody->GetPosition().x, _initPosY / PTM_RATIO);
				_autoPlateBody->SetTransform(vec, _autoPlateBody->GetAngle());
				_isStop = true;
				_Type = 1;
			}
			break;
		case 1:
			if (curY < _stopPosY) {
				_autoPlateBody->SetLinearVelocity(b2Vec2(0, 2));
			}
			else {
				_autoPlateBody->SetLinearVelocity(b2Vec2(0, 0));
				b2Vec2 vec = b2Vec2(_autoPlateBody->GetPosition().x, _stopPosY / PTM_RATIO);
				_autoPlateBody->SetTransform(vec, _autoPlateBody->GetAngle());
				_isStop = true;
				_Type = 0;
			}
			break;
		default:
			break;
		}
	}
	else {
		_fTime += dt;
		if (_fTime >= 1.0f) {
			_fTime = 0;
			_isStop = false;
		}
	}

	if (_contactListener._iType == 0) {
		_pullBtnPlateBody->SetLinearVelocity(b2Vec2(0, 0));
	}
	else if (_contactListener._iType == 1) {	//down
		float curY = _pullBtnPlateBody->GetPosition().y * PTM_RATIO;
		if (curY >= _downPosY)
			_pullBtnPlateBody->SetLinearVelocity(b2Vec2(0, -1));
		else {
			_pullBtnPlateBody->SetLinearVelocity(b2Vec2(0, 0));
			b2Vec2 vec = b2Vec2(_pullBtnPlateBody->GetPosition().x, _downPosY / PTM_RATIO);
			_pullBtnPlateBody->SetTransform(vec, _pullBtnPlateBody->GetAngle());
		}
	}
	else if (_contactListener._iType == 2) {	//up
		float curY = _pullBtnPlateBody->GetPosition().y * PTM_RATIO;
		if (curY <= _upPosY)
			_pullBtnPlateBody->SetLinearVelocity(b2Vec2(0, 1));
		else {
			_pullBtnPlateBody->SetLinearVelocity(b2Vec2(0, 0));
			b2Vec2 vec = b2Vec2(_pullBtnPlateBody->GetPosition().x, _upPosY / PTM_RATIO);
			_pullBtnPlateBody->SetTransform(vec, _pullBtnPlateBody->GetAngle());
		}
	}

	platNode->clear();
	if (_plateBody != nullptr && _gearBody != nullptr) {
		Vec2 vec1 = Vec2(_plateBody->GetPosition().x * PTM_RATIO, _plateBody->GetPosition().y * PTM_RATIO);
		Vec2 vec2 = Vec2((_gearBody->GetPosition().x - 20.0f / PTM_RATIO) * PTM_RATIO, _gearBody->GetPosition().y * PTM_RATIO);
		platNode->drawLine(vec1, vec2, Color4F::WHITE);
	}
	if (_plateBody1 != nullptr && _gearBody != nullptr) {
		Vec2 vec1 = Vec2(_plateBody1->GetPosition().x * PTM_RATIO, (_plateBody1->GetPosition().y + 20.0f / PTM_RATIO) * PTM_RATIO);
		Vec2 vec2 = Vec2((_gearBody->GetPosition().x + 20.0f / PTM_RATIO) * PTM_RATIO, _gearBody->GetPosition().y * PTM_RATIO);
		platNode->drawLine(vec1, vec2, Color4F::WHITE);
	}
}

void Level04::setObject()
{
	wallCount = WallCount;
	rectCount = rectWallCount;

	setInitObject();
	setGoalSensor();
	createStaticBoundary(wallCount, rectCount);

	_contactListener.setCollisionTarget(*_car->getCarSprite());

	_ParticleSystem = new(nothrow)CParticleSystem();
	_ParticleSystem->init(*this);

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

	platNode = DrawNode::create();
	this->addChild(platNode, 0);

	setButton();
	setFilter();
	setPullJoint();
	setAutoPlate();
	setPullBtn();
	setPullBtnSensor();
}

void Level04::Replay() {
	// 先將這個 SCENE 的 update  從 schedule update 中移出
	this->unschedule(schedule_selector(Level04::update));
	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
	//  設定場景切換的特效
	TransitionFade* pageTurn = TransitionFade::create(1.0F, Level04::createScene());
	Director::getInstance()->replaceScene(pageTurn);
}

void Level04::NextLevel() {

}

void Level04::setButton() {
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
	staticDef.position.Set((pos.x + 10.0f) / PTM_RATIO, pos.y / PTM_RATIO);

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

	//floor
	auto doorSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("door"));
	pos = doorSprite->getPosition();
	size = doorSprite->getContentSize();
	scaleX = doorSprite->getScaleX();
	scaleY = doorSprite->getScaleY();
	_stopPosX = pos.x + 218.0f;

	bodyDef.type = b2_kinematicBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = doorSprite;

	_doorBody = _b2World->CreateBody(&bodyDef);

	boxShape.SetAsBox((size.width - 4) * scaleX * 0.5f / PTM_RATIO, (size.height - 4) * scaleY * 0.5f / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 10.0f;
	fixtureDef.friction = 0.25f;
	fixtureDef.restitution = 0;
	_doorBody->CreateFixture(&fixtureDef);

	PriJoint.Initialize(ClickJoint, _doorBody, _doorBody->GetWorldCenter(), b2Vec2(1.0f, 0));
	_b2World->CreateJoint(&PriJoint);

	_bBtnClick = false;
	_isdoorOpen = false;
}

void Level04::setPullJoint() {
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
	fixtureDef.density = 5.0f;
	fixtureDef.friction = 0.2f;
	_plateBody->CreateFixture(&fixtureDef);

	staticDef.position.Set(posA.x / PTM_RATIO, (posA.y + 500) / PTM_RATIO);
	_plateJoint = _b2World->CreateBody(&staticDef);
	_plateJoint->CreateFixture(&staticFixtureDef);

	b2PrismaticJoint* priJoint;
	b2PrismaticJointDef priJointDef;
	priJointDef.Initialize(_plateJoint, _plateBody, _plateBody->GetWorldCenter(), b2Vec2(0, 1));
	priJoint = dynamic_cast<b2PrismaticJoint*>(_b2World->CreateJoint(&priJointDef));

	//pulleyB
	_dount = dynamic_cast<Sprite*>(_csbRoot->getChildByName("circle_pul01"));
	Point posB = _dount->getPosition();
	size = _dount->getContentSize();
	float scale = _dount->getScale();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(posB.x / PTM_RATIO, posB.y / PTM_RATIO);
	bodyDef.userData = _dount;

	_plateBody1 = _b2World->CreateBody(&bodyDef);

	b2CircleShape circleShape;
	circleShape.m_radius = (size.width - 4) * scale * 0.5f / PTM_RATIO;

	fixtureDef.shape = &circleShape;
	fixtureDef.density = 15.0f;
	fixtureDef.friction = 0.25f;
	fixtureDef.restitution = 0;
	fixtureDef.filter.categoryBits = 1 << 1;
	_plateBody1->CreateFixture(&fixtureDef);

	staticDef.position.Set(posB.x / PTM_RATIO, (posA.y + 500) / PTM_RATIO);
	_plateJoint1 = _b2World->CreateBody(&staticDef);
	_plateJoint1->CreateFixture(&staticFixtureDef);

	//平移Joint
	priJointDef.Initialize(_plateJoint1, _plateBody1, _plateBody1->GetWorldCenter(), b2Vec2(0, 0));
	_b2World->CreateJoint(&priJointDef);

	//Pulley Joint
	b2Vec2 vec1 = b2Vec2(posA.x / PTM_RATIO, (posA.y + 500) / PTM_RATIO);
	b2Vec2 vec2 = b2Vec2(posB.x / PTM_RATIO, (posB.y + 500) / PTM_RATIO);
	b2PulleyJointDef jointDef;
	jointDef.Initialize(_plateBody, _plateBody1, vec1, vec2, _plateBody->GetWorldCenter(), _plateBody1->GetWorldCenter(), 1.0f);
	_b2World->CreateJoint(&jointDef);

	//Gear
	auto gearSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("pullGear"));
	Point gearPos = gearSprite->getPosition();
	Size gearSize = gearSprite->getContentSize();
	float gearScale = gearSprite->getScale();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(gearPos.x / PTM_RATIO, gearPos.y / PTM_RATIO);
	bodyDef.userData = gearSprite;

	_gearBody = _b2World->CreateBody(&bodyDef);

	circleShape.m_radius = (gearSize.width - 4) * gearScale * 0.5f / PTM_RATIO;

	fixtureDef.shape = &circleShape;
	fixtureDef.density = 5.0f;
	fixtureDef.friction = 0.2f;
	_gearBody->CreateFixture(&fixtureDef);

	//GearStatic
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(gearPos.x / PTM_RATIO, gearPos.y / PTM_RATIO);
	bodyDef.userData = NULL;

	b2Body* staticBody = _b2World->CreateBody(&bodyDef);

	circleShape.m_radius = 1.0f / PTM_RATIO;

	fixtureDef.shape = &circleShape;
	fixtureDef.density = 1.0f;
	staticBody->CreateFixture(&fixtureDef);

	b2RevoluteJoint* reJoint;
	b2RevoluteJointDef revJoint;
	revJoint.Initialize(staticBody, _gearBody, _gearBody->GetWorldCenter());
	reJoint = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&revJoint));

	b2GearJointDef gearJoint;
	gearJoint.bodyA = _gearBody;
	gearJoint.bodyB = _plateBody;
	gearJoint.joint1 = reJoint;
	gearJoint.joint2 = priJoint;
	gearJoint.ratio = 1;
	_b2World->CreateJoint(&gearJoint);
}

void Level04::setFilter() {
	Color3B filterColor[3] = { Color3B::RED, Color3B::GREEN, Color3B::BLUE };

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	b2Body* body;

	b2FixtureDef fixtureDef;
	b2PolygonShape boxShape;
	fixtureDef.shape = &boxShape;

	std::ostringstream ostr;
	std::string objname;

	for (int i = 1; i <= 3; i++)
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

		if (i == 1)
			fixtureDef.filter.maskBits = 1 << 2 | 1;
		else if (i == 2)
			fixtureDef.filter.maskBits = 1 << 3 | 1;
		else if (i == 3)
			fixtureDef.filter.maskBits = 1 << 4 | 1;
		body->CreateFixture(&fixtureDef);
	}
}

void Level04::setAutoPlate() {
	b2BodyDef staticDef;
	staticDef.type = b2_staticBody;
	staticDef.userData = NULL;

	b2CircleShape staticShape;
	staticShape.m_radius = 5 / PTM_RATIO;
	b2FixtureDef staticFixtureDef;
	staticFixtureDef.shape = &staticShape;

	//pulleyA
	auto boxSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("autoMove"));
	Point pos = boxSprite->getPosition();
	Size size = boxSprite->getContentSize();
	float scaleX = boxSprite->getScaleX();
	float scaleY = boxSprite->getScaleY();
	_initPosY = pos.y;
	_stopPosY = pos.y + 564.0f;

	b2BodyDef bodyDef;
	bodyDef.type = b2_kinematicBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = boxSprite;

	_autoPlateBody = _b2World->CreateBody(&bodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox((size.width - 4) * scaleX * 0.5f / PTM_RATIO, (size.height - 4) * scaleY * 0.5f / PTM_RATIO);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 10.0f;
	fixtureDef.friction = 0.2f;
	_autoPlateBody->CreateFixture(&fixtureDef);

	staticDef.position.Set(pos.x / PTM_RATIO, (pos.y - 300) / PTM_RATIO);
	b2Body* jointBody = _b2World->CreateBody(&staticDef);
	jointBody->CreateFixture(&staticFixtureDef);

	b2PrismaticJointDef priJoint;
	priJoint.Initialize(jointBody, _autoPlateBody, _autoPlateBody->GetWorldCenter(), b2Vec2(0, 1));
	_b2World->CreateJoint(&priJoint);

	_isStop = true;
	_fTime = 0;
	_Type = 0;
}

void Level04::setPullBtn() {
	//PullBtnJoint
	auto jointSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("pullJoint"));
	Point pos = jointSprite->getPosition();
	Size size = jointSprite->getContentSize();
	float scaleX = jointSprite->getScaleX();
	float scaleY = jointSprite->getScaleY();
	float angle = jointSprite->getRotation();

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = jointSprite;
	bodyDef.angle= (-angle) * M_PI / 180.0f;

	_orangeJointBody = _b2World->CreateBody(&bodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox((size.width - 4) * scaleX * 0.5f / PTM_RATIO, (size.height - 4) * scaleY * 0.5f / PTM_RATIO);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 1.0;
	fixtureDef.friction = 0.2f;
	_orangeJointBody->CreateFixture(&fixtureDef);

	//PullBtnStatic
	Point staticPos = Point(pos.x, (pos.y + size.height * 0.5f));
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(staticPos.x / PTM_RATIO, staticPos.y / PTM_RATIO);
	bodyDef.userData = NULL;

	b2Body* staticBody = _b2World->CreateBody(&bodyDef);

	b2CircleShape circleShape;
	circleShape.m_radius = 0.5f / PTM_RATIO;
	//boxShape.SetAsBox(1 * 0.5f / PTM_RATIO, 1 * 0.5f / PTM_RATIO);

	fixtureDef.shape = &circleShape;
	fixtureDef.density = 1.0;
	fixtureDef.friction = 0.2f;
	staticBody->CreateFixture(&fixtureDef);

	b2RevoluteJointDef revJoint;
	b2Vec2 vec = b2Vec2(_orangeJointBody->GetWorldCenter().x, _orangeJointBody->GetWorldCenter().y + size.height);
	revJoint.Initialize(staticBody, _orangeJointBody, b2Vec2(_orangeJointBody->GetWorldCenter().x, (pos.y + size.height * 0.5f) / PTM_RATIO));
	_b2World->CreateJoint(&revJoint);

	//PullBtnOrange
	_orange = dynamic_cast<Sprite*>(_csbRoot->getChildByName("pullBtn"));
	pos = _orange->getPosition();
	size = _orange->getContentSize();
	float scale = _orange->getScaleX();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = _orange;

	b2Body* orangeBody = _b2World->CreateBody(&bodyDef);
	_contactListener._orangeBody = orangeBody;

	circleShape.m_radius = (size.width - 4) * scale * 0.5f / PTM_RATIO;

	fixtureDef.shape = &circleShape;
	fixtureDef.density = 1;
	fixtureDef.friction = 0.2f;
	orangeBody->CreateFixture(&fixtureDef);

	revJoint.Initialize(_orangeJointBody, orangeBody, orangeBody->GetWorldCenter());
	_b2World->CreateJoint(&revJoint);

	b2PrismaticJointDef priJoint;
	priJoint.Initialize(_orangeJointBody, orangeBody, orangeBody->GetWorldCenter(), b2Vec2(0, 0));
	_b2World->CreateJoint(&priJoint);

	//plate
	auto plateSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("pullBtn_plate"));
	Point platePos = plateSprite->getPosition();
	Size plateSize = plateSprite->getContentSize();
	float plateScaleX = plateSprite->getScaleX();
	float plateScaleY = plateSprite->getScaleY();
	_downPosY = platePos.y - 181.0f;
	_upPosY = platePos.y;

	bodyDef.type = b2_kinematicBody;
	bodyDef.position.Set(platePos.x / PTM_RATIO, platePos.y / PTM_RATIO);
	bodyDef.userData = plateSprite;

	_pullBtnPlateBody = _b2World->CreateBody(&bodyDef);

	boxShape.SetAsBox((plateSize.width - 4) * plateScaleX * 0.5f / PTM_RATIO, (plateSize.height - 4) * plateScaleY * 0.5f / PTM_RATIO);

	fixtureDef.shape = &boxShape;
	fixtureDef.density = 1.0;
	fixtureDef.friction = 0.2f;
	_pullBtnPlateBody->CreateFixture(&fixtureDef);

	priJoint.Initialize(staticBody, _pullBtnPlateBody, _pullBtnPlateBody->GetWorldCenter(), b2Vec2(0, 1));
	_b2World->CreateJoint(&priJoint);

	_bBtnTouch = false;
}

void Level04::setPullBtnSensor() {
	//down sensor
	auto sensorSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("pullsensor_down"));
	sensorSprite->setVisible(false);
	Point pos = sensorSprite->getPosition();
	Size size = sensorSprite->getContentSize();
	float scaleX = sensorSprite->getScaleX();
	float scaleY = sensorSprite->getScaleY();

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = sensorSprite;

	b2Body* sensorDown = _b2World->CreateBody(&bodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox((size.width - 4) * scaleX * 0.5f / PTM_RATIO, (size.height - 4) * scaleY * 0.5f / PTM_RATIO);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 1002.0f;
	sensorDown->CreateFixture(&fixtureDef);

	//up sensor
	sensorSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("pullsensor_up"));
	sensorSprite->setVisible(false);
	pos = sensorSprite->getPosition();
	size = sensorSprite->getContentSize();
	scaleX = sensorSprite->getScaleX();
	scaleY = sensorSprite->getScaleY();

	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = sensorSprite;

	b2Body* sensorUp = _b2World->CreateBody(&bodyDef);

	boxShape.SetAsBox((size.width - 4) * scaleX * 0.5f / PTM_RATIO, (size.height - 4) * scaleY * 0.5f / PTM_RATIO);

	fixtureDef.shape = &boxShape;
	fixtureDef.density = 1003.0f;
	sensorUp->CreateFixture(&fixtureDef);
}

bool Level04::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)//觸碰開始事件
{
	Point touchLoc = pTouch->getLocation();

	//MouseJoint
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		if (body->GetUserData() == NULL) continue; // 靜態物體不處理

		if (body->GetUserData() == _dount)
		{
			//移動剪刀
			Size size = _dount->getContentSize();
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
		else if (body->GetUserData() == _orange) {
			//移動柳丁
			Size size = _orange->getContentSize();
			float fdistX = size.width / 2.0f;
			float fdistY = size.height / 2.0f;

			float x = body->GetPosition().x * PTM_RATIO - touchLoc.x;
			float y = body->GetPosition().y * PTM_RATIO - touchLoc.y;
			float tpdist = x * x + y * y;
			if (tpdist < fdistX * fdistY) {
				_bBtnTouch = true;
				_orangeJointBody->SetType(b2_dynamicBody);
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
		else {
			_draw->onTouchBegan(touchLoc);//繪圖
			_ParticleSystem->setColor(_draw->getPenColor());
			_ParticleSystem->onTouchBegan(touchLoc);
		}
	}

	return true;
}

void  Level04::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //觸碰移動事件
{
	Point touchLoc = pTouch->getLocation();

	if (_bOnTouch) {
		_mouseJoint->SetTarget(b2Vec2(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO));
	}

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
		else {
			_draw->onTouchMoved(touchLoc);//繪圖
			_ParticleSystem->setColor(_draw->getPenColor());
			_ParticleSystem->onTouchMoved(touchLoc);
		}
	}
}

void  Level04::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //觸碰結束事件 
{
	Point touchLoc = pTouch->getLocation();

	if (_bOnTouch) {
		_bOnTouch = false;
		if (_mouseJoint->GetBodyB()->GetUserData() == _orange) {
			_bBtnTouch = false;
			_orangeJointBody->SetType(b2_staticBody);
		}
		else if (_mouseJoint != NULL) {
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

CLevel04ContactListener::CLevel04ContactListener()
{
	_carSprite = nullptr;
	_isFinish = false;

	_BtnSprite = nullptr;
	_isClickBtn = false;

	_iType = 0;
}

//
// 只要是兩個 body 的 fixtures 碰撞，就會呼叫這個函式
//
void CLevel04ContactListener::BeginContact(b2Contact* contact)
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

	//拉桿是否碰到感應
	//down
	if (BodyA->GetFixtureList()->GetDensity() == 1002.0f && BodyB == _orangeBody) {
		_iType = 1;
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 1002.0f && BodyA == _orangeBody) {
		_iType = 1;
	}
	//up
	if (BodyA->GetFixtureList()->GetDensity() == 1003.0f && BodyB == _orangeBody) {
		_iType = 2;
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 1003.0f && BodyA == _orangeBody) {
		_iType = 2;
	}

}

//碰撞結束
void CLevel04ContactListener::EndContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();
	//down
	if (BodyA->GetFixtureList()->GetDensity() == 1002.0f && BodyB == _orangeBody) {
		_iType = 0;
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 1002.0f && BodyA == _orangeBody) {
		_iType = 0;
	}
	//up
	if (BodyA->GetFixtureList()->GetDensity() == 1003.0f && BodyB == _orangeBody) {
		_iType = 0;
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 1003.0f && BodyA == _orangeBody) {
		_iType = 0;
	}
}

void CLevel04ContactListener::setCollisionTarget(cocos2d::Sprite& targetSprite) {
	_carSprite = &targetSprite;
}