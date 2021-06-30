#include "Level.h"

Level::Level() {
	_csbRoot = nullptr;
	_b2World = nullptr;

	_car = nullptr;
	_draw = nullptr;

	_leftBtn = nullptr;
	_rightBtn = nullptr;
	_retryBtn = nullptr;
	_redBtn = nullptr;
	_greenBtn = nullptr;
	_blueBtn = nullptr;

	_stopLight = nullptr;

	_bottomBody = nullptr;

	_endCount = 0;
}

Level::~Level()
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
	CC_SAFE_DELETE(_rightBtn);
	CC_SAFE_DELETE(_retryBtn);
	CC_SAFE_DELETE(_redBtn);
	CC_SAFE_DELETE(_greenBtn);
	CC_SAFE_DELETE(_blueBtn);
}

void Level::setInitObject() {
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
	_leftBtn = new (nothrow) CButton();
	_leftBtn->init("runnormal.png", "runon.png", btn->getPosition(), *this);
	_leftBtn->setIcon("arrow.png", false);

	btn = dynamic_cast<Sprite*>(_csbRoot->getChildByName("btn_right"));
	btn->setVisible(false);
	_rightBtn = new (nothrow) CButton();
	_rightBtn->init("runnormal.png", "runon.png", btn->getPosition(), *this);
	_rightBtn->setIcon("arrow.png", true);

	btn = dynamic_cast<Sprite*>(_csbRoot->getChildByName("replaybtn"));
	btn->setVisible(false);
	_retryBtn = new (nothrow) CButton();
	_retryBtn->init("replaybtn.png", "replaybtn.png", btn->getPosition(), *this);

	_car = new (nothrow) CCar();
	_car->init(*_csbRoot, *_b2World);

	auto background = dynamic_cast<Sprite*>(_csbRoot->getChildByName("background"));
	background->setVisible(false);
	_background = Sprite::createWithSpriteFrameName("bg64.png");
	_background->setPosition(background->getPosition());
	_background->setScale(background->getScale());
	this->addChild(_background, -1);
#ifdef BOX2D_DEBUG
	_background->setVisible(false);
#endif
	

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
}

void Level::readBlocksCSBFile(const char* csbfilename)
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

void Level::readSceneFile(const char* csbfilename)
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

void Level::setGoalSensor() {
	auto sensorSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("car_sensor"));
	_goalPos = sensorSprite->getPosition();
	Size  size = sensorSprite->getContentSize();

	b2BodyDef sensorBodyDef;
	sensorBodyDef.position.Set(_goalPos.x / PTM_RATIO, _goalPos.y / PTM_RATIO);
	sensorBodyDef.type = b2_staticBody;
	sensorBodyDef.userData = sensorSprite;

	b2Body* sensorBody = _b2World->CreateBody(&sensorBodyDef);
	b2PolygonShape sensorShape;
	sensorShape.SetAsBox((size.width - 40) * 0.5f / PTM_RATIO, (size.height - 30) * 0.5f / PTM_RATIO);

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

void Level::createStaticBoundary(int wall, int rect)
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

	for (size_t i = 1; i <= wall; i++)
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

	for (size_t i = 1; i <= rect; i++)
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

void Level::setPenColor(cocos2d::Color4F color, int num) {
	_draw->setPenColor(color, num);
}

#ifdef BOX2D_DEBUG
//改寫繪製方法
void Level::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif