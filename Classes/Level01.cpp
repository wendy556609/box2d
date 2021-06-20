#include "Level01.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

#define StaticAndDynamicBodyExample 1

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

	CC_SAFE_DELETE(_draw);
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

	// 建立 Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//重力方向
	bool AllowSleep = true;			//允許睡著
	_b2World = new (std::nothrow) b2World(Gravity);	//創建世界
	_b2World->SetAllowSleeping(AllowSleep);	//設定物件允許睡著

	//繪圖
	_draw = new CDraw();
	_draw->init(*_b2World, *this);

	/*auto box = dynamic_cast<Sprite*>(_csbRoot->getChildByName("square01"));
	Point pos = box->getPosition();
	Size size = box->getContentSize();
	
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = box;

	b2Body* body = _b2World->CreateBody(&bodyDef);

	b2FixtureDef fixtureDef;
	fixtureDef.density = 1.0f; fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	b2PolygonShape boxShape;
	boxShape.SetAsBox(size.width * 0.5f / PTM_RATIO, size.height * 0.5f / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	body->CreateFixture(&fixtureDef);*/

	setCar();

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

	// 取得 _b2World 中所有的 body 進行處理
	// 最主要是根據目前運算的結果，更新附屬在 body 中 sprite 的位置
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		// 以下是以 Body 有包含 Sprite 顯示為例
		if (body->GetUserData() != NULL)
		{
			Point move;
			move = Point((body->GetPosition().x + 0.05f), body->GetPosition().y);
			//body->SetTransform(b2Vec2(move.x, move.y), body->GetAngle());
			Sprite* bodyData = static_cast<Sprite*>(body->GetUserData());
			bodyData->setPosition(body->GetPosition().x * PTM_RATIO, body->GetPosition().y * PTM_RATIO);
			bodyData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
		}
	}
}

void Level01::setCar() {
	//car
	auto box = dynamic_cast<Sprite*>(_csbRoot->getChildByName("car"));
	Point pos = box->getPosition();
	Size size = box->getContentSize();

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = box;

	b2Body* body = _b2World->CreateBody(&bodyDef);

	b2FixtureDef fixtureDef;
	fixtureDef.density = 1.0f; fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	b2PolygonShape boxShape;
	boxShape.SetAsBox((size.width - 4) * 0.5f / PTM_RATIO, (size.height - 4) * 0.5f / PTM_RATIO);
	fixtureDef.shape = &boxShape;

	body->CreateFixture(&fixtureDef);

	//wheel01
	box = dynamic_cast<Sprite*>(_csbRoot->getChildByName("wheel01"));
	pos = box->getPosition();
	size = box->getContentSize();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = box;

	b2Body* wheelbody = _b2World->CreateBody(&bodyDef);

	fixtureDef.restitution = 0.1f;
	b2CircleShape circle;
	circle.m_radius = size.width * 0.5f / PTM_RATIO;
	fixtureDef.shape = &circle;
	wheelbody->CreateFixture(&fixtureDef);

	b2RevoluteJointDef jointDef;
	jointDef.Initialize(body, wheelbody, wheelbody->GetWorldCenter());
	_b2World->CreateJoint(&jointDef);

	//wheel02
	box = dynamic_cast<Sprite*>(_csbRoot->getChildByName("wheel02"));
	pos = box->getPosition();
	size = box->getContentSize();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = box;

	wheelbody = _b2World->CreateBody(&bodyDef);

	circle.m_radius = size.width * 0.5f / PTM_RATIO;
	fixtureDef.shape = &circle;
	wheelbody->CreateFixture(&fixtureDef);

	jointDef.Initialize(body, wheelbody, wheelbody->GetWorldCenter());
	_b2World->CreateJoint(&jointDef);
}

bool Level01::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)//觸碰開始事件
{
	Point touchLoc = pTouch->getLocation();

	_draw->onTouchBegan(touchLoc);

	return true;
}

void  Level01::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //觸碰移動事件
{
	Point touchLoc = pTouch->getLocation();

	_draw->onTouchMoved(touchLoc);
}

void  Level01::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //觸碰結束事件 
{
	Point touchLoc = pTouch->getLocation();

	_draw->onTouchEnded(touchLoc);
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

	//bottom edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, 0.0f / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	//left edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(0.0f / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	//top edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, _visibleSize.height / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	//right edge
	edgeShape.Set(b2Vec2(_visibleSize.width / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	auto wall= dynamic_cast<Sprite*>(_csbRoot->getChildByName("wall1_01"));
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
	wep1.y= lep1.x * modelMatrix.m[4] + lep1.y * modelMatrix.m[5] + modelMatrix.m[7];
	wep2.x = lep2.x * modelMatrix.m[0] + lep2.y * modelMatrix.m[1] + modelMatrix.m[3];
	wep2.y = lep2.x * modelMatrix.m[4] + lep2.y * modelMatrix.m[5] + modelMatrix.m[7];

	edgeShape.Set(b2Vec2(wep1.x / PTM_RATIO, wep1.y / PTM_RATIO), b2Vec2(wep2.x / PTM_RATIO, wep2.y / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);
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