#include "Level03.h"

Level03::~Level03()
{
	CC_SAFE_DELETE(_dropBtn);
}

Scene* Level03::createScene()
{
	return Level03::create();
}

// on "init" you need to initialize your instance
bool Level03::init()
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

	//���D : ��ܥثe BOX2D �Ҥ��Ъ��\��
	_titleLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 48);
	_titleLabel->setPosition(_visibleSize.width / 2, _visibleSize.height * 0.9f);
	this->addChild(_titleLabel, 1);

	_csbRoot = CSLoader::createNode("Level03.csb");

	addChild(_csbRoot, 1);

	setObject();

	_b2World->SetContactListener(&_contactListener);

	auto listener = EventListenerTouchOneByOne::create();	//�Ыؤ@�Ӥ@��@���ƥ��ť��
	listener->onTouchBegan = CC_CALLBACK_2(Level03::onTouchBegan, this);		//�[�JĲ�I�}�l�ƥ�
	listener->onTouchMoved = CC_CALLBACK_2(Level03::onTouchMoved, this);		//�[�JĲ�I���ʨƥ�
	listener->onTouchEnded = CC_CALLBACK_2(Level03::onTouchEnded, this);		//�[�JĲ�I���}�ƥ�

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//�[�J��Ыت��ƥ��ť��
	this->schedule(CC_SCHEDULE_SELECTOR(Level03::update));

	return true;
}

void Level03::update(float dt)
{
	int velocityIterations = 8;	// �t�׭��N����
	int positionIterations = 1; // ��m���N����
	// Instruct the world to perform a single step of simulation.
	// It is generally best to keep the time step and iterations fixed.
	_b2World->Step(dt, velocityIterations, positionIterations);

	_car->update(dt);

	if (_contactListener._isFinish) {
		_car->setFinish(_goalPos);
		_stopLight->setSpriteFrame("orange05.png");
	}
	// ���o _b2World ���Ҧ��� body �i��B�z
	// �̥D�n�O�ھڥثe�B�⪺���G�A��s���ݦb body �� sprite ����m
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		// �H�U�O�H Body ���]�t Sprite ��ܬ���
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
		else if (!_isFloorUp) {
			_FloorBody->SetType(b2_kinematicBody);
			_FloorBody->SetLinearVelocity(b2Vec2(0, 1));
			float curY = _FloorBody->GetWorldCenter().y * PTM_RATIO;
			if (curY >= _stopPosY) {
				_isFloorUp = true;
				_FloorBody->SetLinearVelocity(b2Vec2(0, 0));
				_FloorBody->SetTransform(b2Vec2(_FloorBody->GetWorldCenter().x, _stopPosY / PTM_RATIO), 0);
				_FloorBody->SetType(b2_kinematicBody);
			}
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

	platNode->clear();
	if (distanceBody != nullptr && disJointBody != nullptr) {
		Vec2 vec1 = Vec2(distanceBody->GetPosition().x * PTM_RATIO, distanceBody->GetPosition().y * PTM_RATIO);
		Vec2 vec2 = Vec2(disJointBody->GetPosition().x * PTM_RATIO, disJointBody->GetPosition().y * PTM_RATIO);
		platNode->drawLine(vec1, vec2, Color4F::BLACK);
	}
	if (platJoint != nullptr && _FloorBody != nullptr) {
		Vec2 vec1 = Vec2(platJoint->GetPosition().x * PTM_RATIO, platJoint->GetPosition().y * PTM_RATIO);
		Vec2 vec2 = Vec2(_FloorBody->GetPosition().x * PTM_RATIO, _FloorBody->GetPosition().y * PTM_RATIO);
		platNode->drawLine(vec1, vec2, Color4F::BLACK);
	}
	
	if (gearBody[0] != nullptr) {
		float angle = (-30) * M_PI / 180.0f;
		gearBody[0]->SetAngularVelocity(angle);
	}
}

void Level03::setObject()
{
	wallCount = WallCount;
	rectCount = rectWallCount;

	setInitObject();
	setGoalSensor();
	createStaticBoundary(wallCount, rectCount);

	_contactListener.setCollisionTarget(*_car->getCarSprite());

	auto btn = dynamic_cast<Sprite*>(_csbRoot->getChildByName("born"));
	btn->setVisible(false);
	_dropBtn = new(nothrow)CButton();
	_dropBtn->init("dnarrow.png", "dnarrowon.png", btn->getPosition(), *this, oneClick);
	_dropBtn->setScale(btn->getScale());

	platNode = DrawNode::create();
	this->addChild(platNode, 2);

	setMouseJoint();
	setRopeJoint();
	setWeldJoint();
	setButton();
	setDistanceJoint();
	setGear();
}

void Level03::Replay() {
	// ���N�o�� SCENE �� update  �q schedule update �����X
	this->unschedule(schedule_selector(Level03::update));
	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
	//  �]�w�����������S��
	TransitionFade* pageTurn = TransitionFade::create(1.0F, Level03::createScene());
	Director::getInstance()->replaceScene(pageTurn);
}

void Level03::NextLevel() {

}

void Level03::setButton() {
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
	staticDef.position.Set((pos.x - 10.0f) / PTM_RATIO, pos.y / PTM_RATIO);

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
	fixtureDef.density = 1002.0f;
	sensorBody->CreateFixture(&fixtureDef);

	//floorJoint
	auto jointSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("platJoint"));
	pos = jointSprite->getPosition();
	size = jointSprite->getContentSize();
	scaleX = jointSprite->getScaleX();
	scaleY = jointSprite->getScaleY();

	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = jointSprite;

	platJoint = _b2World->CreateBody(&bodyDef);

	boxShape.SetAsBox((size.width - 4) * scaleX * 0.5f / PTM_RATIO, (size.height - 4) * scaleY * 0.5f / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	platJoint->CreateFixture(&fixtureDef);

	//floor
	auto doorSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("btn_platform"));
	pos = doorSprite->getPosition();
	size = doorSprite->getContentSize();
	scaleX = doorSprite->getScaleX();
	scaleY = doorSprite->getScaleY();
	_stopPosY = pos.y + 226.0f;

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = doorSprite;

	_FloorBody = _b2World->CreateBody(&bodyDef);

	boxShape.SetAsBox((size.width - 4) * scaleX * 0.5f / PTM_RATIO, (size.height - 4) * scaleY * 0.5f / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 10.0f;
	fixtureDef.friction = 0.25f;
	fixtureDef.restitution = 0;
	_FloorBody->CreateFixture(&fixtureDef);

	PriJoint.Initialize(ClickJoint, _FloorBody, _FloorBody->GetWorldCenter(), b2Vec2(0, 1.0f));
	_b2World->CreateJoint(&PriJoint);

	_bBtnClick = false;
	_isFloorUp = false;
}

void Level03::setMouseJoint() {
	//�ŤM
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

void Level03::setRopeJoint() {
	/*------------rope01-----------------*/
	//�R�A����
	auto staticSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("ropeJoint1"));
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

	//÷�l���`
	b2RopeJointDef jointDef;
	jointDef.bodyA = staticBody;
	jointDef.bodyB = _car->getCarBody();
	jointDef.localAnchorA = b2Vec2(0, -0.5f);
	jointDef.localAnchorB = b2Vec2(0.7f, 0.85f);
	jointDef.maxLength = (staticPos.y - _car->getCarPos().y) / PTM_RATIO;
	jointDef.collideConnected = true;
	ropeJoint = dynamic_cast<b2RopeJoint*>(_b2World->CreateJoint(&jointDef));

	//÷�l�u�q
	Sprite* ropeSprite[8] = { nullptr };
	Point pos[8];
	Size size[8];
	ropeBody[8] = { nullptr };

	bodyDef.type = b2_dynamicBody;
	fixtureDef.density = 0.01f;  fixtureDef.friction = 1.0f; fixtureDef.restitution = 0.0f;
	fixtureDef.shape = &boxShape;

	for (int i = 0; i < 8; i++)
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
	//�s��staticBody�MropeBody[0]
	float locAnchor = 0.5f * (size[0].height - 5) / PTM_RATIO;
	b2RevoluteJointDef reJoint;
	reJoint.bodyA = staticBody;
	reJoint.localAnchorA.Set(0, -0.7f);
	reJoint.bodyB = ropeBody[0];
	reJoint.localAnchorB.Set(0, locAnchor);
	ropeReJoint[0] = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&reJoint));

	for (int i = 0; i < 7; i++)
	{
		reJoint.bodyA = ropeBody[i];
		reJoint.localAnchorA.Set(0, -locAnchor);
		reJoint.bodyB = ropeBody[i + 1];
		reJoint.localAnchorB.Set(0, locAnchor);
		ropeReJoint[i + 1] = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&reJoint));
	}
	reJoint.bodyA = ropeBody[7];
	reJoint.localAnchorA.Set(0, -locAnchor);
	reJoint.bodyB = _car->getCarBody();
	reJoint.localAnchorB.Set(0.3f, 0.7f);
	ropeReJoint[7] = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&reJoint));

	/*------------rope02-----------------*/
	//�R�A����
	staticSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("ropeJoint2"));
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

	//÷�l���`
	jointDef.bodyA = staticBody;
	jointDef.bodyB = _car->getCarBody();
	jointDef.localAnchorA = b2Vec2(0, -0.7f);
	jointDef.localAnchorB = b2Vec2(-0.7f, 0.85f);
	jointDef.maxLength = (staticPos.y - _car->getCarPos().y) / PTM_RATIO;
	jointDef.collideConnected = true;
	ropeJoint2 = dynamic_cast<b2RopeJoint*>(_b2World->CreateJoint(&jointDef));

	//÷�l�u�q
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
	//�s��staticBody�MropeBody[0]
	locAnchor = 0.5f * (size[0].height - 5) / PTM_RATIO;

	reJoint.bodyA = staticBody;
	reJoint.localAnchorA.Set(0, -0.7f);
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
	reJoint.bodyB = _car->getCarBody();
	reJoint.localAnchorB.Set(-0.7f, 0.7f);
	ropeReJoint2[7] = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&reJoint));


}

void Level03::setWeldJoint() {
	/*--------------dount-----------------*/
	auto dount = dynamic_cast<Sprite*>(_csbRoot->getChildByName("dount_born"));
	_dountPos = dount->getPosition();
	_dountScale = dount->getScale();

	_dountBorn = false;

	/*--------------weldJoint2-----------------*/
	//weldJoint
	auto staticSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("weld_joint1"));
	Point staticPos = staticSprite->getPosition();
	Size staticSize = staticSprite->getContentSize();
	float staticScale = staticSprite->getScale();

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(staticPos.x / PTM_RATIO, staticPos.y / PTM_RATIO);
	bodyDef.userData = staticSprite;

	b2Body* staticBody = _b2World->CreateBody(&bodyDef);
	b2CircleShape circleShape;
	circleShape.m_radius = staticSize.width * staticScale * 0.5f / PTM_RATIO;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 1.0f; fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	staticBody->CreateFixture(&fixtureDef);

	//weldPlatform
	auto platform = dynamic_cast<Sprite*>(_csbRoot->getChildByName("weld_platform1"));
	Point platPos = platform->getPosition();
	Size platSize = platform->getContentSize();
	float platScaleX = platform->getScaleX();
	float platScaleY = platform->getScaleY();
	float angle = platform->getRotation();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(platPos.x / PTM_RATIO, platPos.y / PTM_RATIO);
	bodyDef.angle = (-angle) * M_PI / 180.0f;
	bodyDef.userData = platform;

	b2Body* platBody = _b2World->CreateBody(&bodyDef);
	b2PolygonShape rectShape;
	rectShape.SetAsBox(platSize.width * platScaleX * 0.5f / PTM_RATIO, platSize.height * platScaleY * 0.5f / PTM_RATIO);
	fixtureDef.shape = &rectShape;
	fixtureDef.density = 2.0f;  fixtureDef.friction = 0; fixtureDef.restitution = 1.0f;
	platBody->CreateFixture(&fixtureDef);

	b2WeldJointDef jointDef;
	jointDef.Initialize(staticBody, platBody, staticBody->GetPosition() + b2Vec2(10 / PTM_RATIO, 10 / PTM_RATIO));
	jointDef.frequencyHz = 3.0f;
	jointDef.dampingRatio = 0.05f;
	_b2World->CreateJoint(&jointDef);
}

void Level03::setGear() {
	for (int i = 0; i < 10; i++)
	{
		gearBody[i] = nullptr;
	}
	Sprite* gearSprite[10] = { nullptr };
	Point pos[10];
	Size size[10];
	float scale[9];
	float scaleX, scaleY;
	b2Body* staticBody[10] = { nullptr };
	b2RevoluteJoint* RevJoint[9] = { nullptr };
	b2PrismaticJoint* PriJoint = nullptr;
	float ratio[8];

	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;

	b2CircleShape staticShape;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &staticShape;

	for (int i = 0; i < 10; i++)
	{
		std::ostringstream ostr;
		std::string objname;
		ostr << "gear";
		ostr << i + 1; objname = ostr.str();

		gearSprite[i] = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		pos[i] = gearSprite[i]->getPosition();
		size[i] = gearSprite[i]->getContentSize();

		if (i < 9) {
			scale[i] = gearSprite[i]->getScale();
			ostr.str("");
			if (i == 6) {
				ostr << "gear";
				ostr << 6;
				ostr << "_joint"; objname = ostr.str();
			}
			else {
				ostr << "gear";
				ostr << i + 1; 
				ostr << "_joint"; objname = ostr.str();
			}
			auto staticSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
			Size circleSize = staticSprite->getContentSize();
			float circleScale = staticSprite->getScale();

			staticBodyDef.userData = staticSprite;
			staticBodyDef.position.Set(pos[i].x / PTM_RATIO, pos[i].y / PTM_RATIO);
			staticBody[i] = _b2World->CreateBody(&staticBodyDef);
			staticShape.m_radius = circleSize.width * circleScale * 0.5f / PTM_RATIO;
			staticBody[i]->CreateFixture(&fixtureDef);
		}
		else {
			scaleX = gearSprite[i]->getScaleX();
			scaleY = gearSprite[i]->getScaleY();

			staticBodyDef.userData = NULL;
			staticBodyDef.position.Set(pos[i].x / PTM_RATIO, pos[i].y / PTM_RATIO);
			staticBody[i] = _b2World->CreateBody(&staticBodyDef);
			staticShape.m_radius = 5 * 0.5f / PTM_RATIO;
			staticBody[i]->CreateFixture(&fixtureDef);
		}
	}
	
	b2BodyDef gearBodyDef;
	gearBodyDef.type = b2_dynamicBody;

	b2CircleShape gearShape;
	b2PolygonShape rectShape;

	fixtureDef.shape = &gearShape;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.2f;
	fixtureDef.restitution = 0.25f;

	float radius[9];

	for (int i = 0; i < 10; i++)
	{
		if (i < 9) {
			gearShape.m_radius = (size[i].width - 4) * 0.5f * scale[i] / PTM_RATIO;
			radius[i] = gearShape.m_radius;
		}
		else {
			fixtureDef.shape = &rectShape;
			rectShape.SetAsBox((size[i].width - 4) * 0.5f * scaleX / PTM_RATIO, (size[i].height - 4) * 0.5f * scaleY / PTM_RATIO);
		}

		gearBodyDef.userData = gearSprite[i];
		gearBodyDef.position.Set(pos[i].x / PTM_RATIO, pos[i].y / PTM_RATIO);
		gearBody[i] = _b2World->CreateBody(&gearBodyDef);
		gearBody[i]->CreateFixture(&fixtureDef);
	}

	b2RevoluteJointDef RJoint;
	b2PrismaticJointDef PrJoint; // �������`

	int count = 0;
	for (int i = 0; i < 10; i++)
	{
		if (i < 9) {
			RJoint.Initialize(staticBody[i], gearBody[i], gearBody[i]->GetWorldCenter());
			RevJoint[i] = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&RJoint));
			if (i < 8)
				ratio[i] = radius[i + 1] / radius[i];
		}
		else {
			PrJoint.Initialize(staticBody[i], gearBody[i], gearBody[i]->GetWorldCenter(), b2Vec2(0, 1));
			PriJoint = dynamic_cast<b2PrismaticJoint*>(_b2World->CreateJoint(&PrJoint));
		}
	}

	for (int i = 0; i < 9; i++)
	{
		GJoint[i].bodyA = gearBody[i];
		GJoint[i].bodyB = gearBody[i + 1];
		if (i < 8) {
			GJoint[i].joint1 = RevJoint[i];
			GJoint[i].joint2 = RevJoint[i + 1];
		}
		else {
			GJoint[i].joint1 = RevJoint[i];
			GJoint[i].joint2 = PriJoint;
		}

		if (i == 1 || i == 5 || i == 6)
			GJoint[i].ratio = -ratio[i];
		else if(i == 8)
			GJoint[i].ratio = -2;
		else
			GJoint[i].ratio = ratio[i];
		//_b2World->CreateJoint(&GJoint[i]);
	}
	_b2World->CreateJoint(&GJoint[5]);
	_b2World->CreateJoint(&GJoint[8]);
}

void Level03::setDistanceJoint() {
	//platform
	auto distance = dynamic_cast<Sprite*>(_csbRoot->getChildByName("distance"));
	Point pos = distance->getPosition();
	Size size = distance->getContentSize();
	float scaleX = distance->getScaleX();
	float scaleY = distance->getScaleY();

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = distance;

	distanceBody = _b2World->CreateBody(&bodyDef);

	b2PolygonShape rectShape;
	rectShape.SetAsBox((size.width - 4) * scaleX * 0.5f / PTM_RATIO, (size.height - 4) * scaleY * 0.5f / PTM_RATIO);
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &rectShape;
	fixtureDef.density = 3.0f; fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	distanceBody->CreateFixture(&fixtureDef);

	//static
	auto distanceJoint = dynamic_cast<Sprite*>(_csbRoot->getChildByName("distanceJoint"));
	pos = distanceJoint->getPosition();
	size = distanceJoint->getContentSize();
	scaleX = distanceJoint->getScaleX();
	scaleY = distanceJoint->getScaleY();

	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = distanceJoint;

	disJointBody = _b2World->CreateBody(&bodyDef);

	rectShape.SetAsBox((size.width - 4) * scaleX * 0.5f / PTM_RATIO, (size.height - 4) * scaleY * 0.5f / PTM_RATIO);
	fixtureDef.shape = &rectShape;
	fixtureDef.density = 3.0f; fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	disJointBody->CreateFixture(&fixtureDef);

	//DistanceJoint
	b2DistanceJointDef disJoint;
	disJoint.Initialize(distanceBody, disJointBody, distanceBody->GetWorldCenter(), disJointBody->GetWorldCenter());
	_b2World->CreateJoint(&disJoint);
}

bool Level03::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)//Ĳ�I�}�l�ƥ�
{
	Point touchLoc = pTouch->getLocation();

	//MouseJoint
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		if (body->GetUserData() == NULL) continue; // �R�A���餣�B�z

		if (body->GetUserData() == _cutSprite)
		 {
			//���ʰŤM
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
		else if (_dropBtn->onTouchBegan(touchLoc)) {
		}
		else
			_draw->onTouchBegan(touchLoc);//ø��
	}

	return true;
}

void  Level03::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //Ĳ�I���ʨƥ�
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
		else if (_dropBtn->onTouchMoved(touchLoc)) {
		}
		else
			_draw->onTouchMoved(touchLoc);
	}
}

void  Level03::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //Ĳ�I�����ƥ� 
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
		else if (_dropBtn->onTouchEnded(touchLoc)) {
			//�ͦ�������
			if (!_dountBorn) {
				auto dountSprite = Sprite::createWithSpriteFrameName("dount02.png");
				dountSprite->setScale(_dountScale);
				this->addChild(dountSprite, 2);

				b2BodyDef bodyDef;
				bodyDef.type = b2_dynamicBody;
				bodyDef.position.Set(_dountPos.x / PTM_RATIO, _dountPos.y / PTM_RATIO);
				bodyDef.userData = dountSprite;
				b2Body* body = _b2World->CreateBody(&bodyDef);

				// Define poly shape for our dynamic body.
				b2CircleShape circleShape;
				Size size = dountSprite->getContentSize();
				circleShape.m_radius = size.width * _dountScale * 0.5f / PTM_RATIO;
				// Define the dynamic body fixture.
				b2FixtureDef fixtureDef;
				fixtureDef.shape = &circleShape;
				fixtureDef.restitution = 0.1f;
				fixtureDef.density = 1.0f;
				fixtureDef.friction = 0.25f;

				// �Ҧ� BOX2D ���� filter.categoryBits �w�]���O 1
				body->CreateFixture(&fixtureDef);

				_dountBorn = true;
			}
		}
		else
			_draw->onTouchEnded(touchLoc);
	}
}

CLevel03ContactListener::CLevel03ContactListener()
{
	_carSprite = nullptr;
	_isFinish = false;
	
	_BtnSprite = nullptr;
	_isClickBtn = false;

	_isCut = false;
	ropeNum = 99;
	rope2Num = 99;
}

//
// �u�n�O��� body �� fixtures �I���A�N�|�I�s�o�Ө禡
//
void CLevel03ContactListener::BeginContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();

	// ���l�O�_�I��ت��a
	if (BodyA->GetFixtureList()->GetDensity() == 1000.0f && BodyB->GetUserData() == _carSprite) {
		_isFinish = true;
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 1000.0f && BodyA->GetUserData() == _carSprite) {
		_isFinish = true;
	}

	//�ŤM�I��÷�l
	if (BodyA->GetFixtureList()->GetDensity() == 1001.0f && BodyB->GetFixtureList()->GetDensity() == 0.01f) {
		for (int i = 0; i < 8; i++)
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
		for (int i = 0; i < 8; i++)
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

	//�O�_���U���s
	if (BodyA->GetFixtureList()->GetDensity() == 1002.0f && BodyB->GetUserData() == _BtnSprite) {
		_isClickBtn = true;
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 1002.0f && BodyA->GetUserData() == _BtnSprite) {
		_isClickBtn = true;
	}

}

//�I������
void CLevel03ContactListener::EndContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();

}

void CLevel03ContactListener::setCollisionTarget(cocos2d::Sprite& targetSprite) {
	_carSprite = &targetSprite;
}