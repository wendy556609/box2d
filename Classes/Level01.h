#pragma once

//#define BOX2D_DEBUG 1

#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "cocostudio/CocoStudio.h"

#include "common/CDraw.h"
#include "common/CButton.h"
#include "CCar.h"

#ifdef BOX2D_DEBUG
#include "Common/GLES-Render.h"
#include "Common/GB2ShapeCache-x.h"
#endif

using namespace std;

#define PTM_RATIO 32.0f
#define RepeatCreateBallTime 3
#define AccelerateMaxNum 2
#define AccelerateRatio 1.5f

#define WallCount 4
#define rectWallCount 8

class CContactListener : public b2ContactListener
{
public:
	cocos2d::Sprite* _carSprite;
	cocos2d::Sprite* _BtnSprite;

	bool _isFinish;
	bool _isClickBtn;

	CContactListener();
	//碰撞開始
	virtual void BeginContact(b2Contact* contact);
	//碰撞結束
	virtual void EndContact(b2Contact* contact);
	void setCollisionTarget(cocos2d::Sprite& targetSprite);
};

class Level01 : public cocos2d::Scene
{
public:
	~Level01();
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();

	CContactListener _contactListener;
	int wallCount;

	Node* _csbRoot;

	CCar* _car;

	CDraw* _draw;
	CButton* _leftBtn;
	CButton* _rightBtn;

	cocos2d::Point _goalPos;
	cocos2d::Sprite* _stopLight;

	// for Box2D
	b2World* _b2World;
	cocos2d::Label* _titleLabel;
	cocos2d::Size _visibleSize;

	cocos2d::Sprite* _BtnSprite;
	b2Body* _BtnBody;
	b2Body* _doorBody;
	bool _bBtnClick;
	bool _isDoorOpen;
	float _stopPosY;

	b2Body* _bottomBody;
	cocos2d::Sprite* _frameSprite;
	b2MouseJoint* _mouseJoint;
	bool _bOnTouch;

	// Box2D Examples
	void readBlocksCSBFile(const char*);
	void readSceneFile(const char*);
	void createStaticBoundary();

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	void update(float dt);

	void setGoalSensor();
	void setPullJoint();
	void setSeesaw();
	void setButton();
	void setMouseJoint();

	bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰開始事件
	void onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰移動事件
	void onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰結束事件 

#ifdef BOX2D_DEBUG
//DebugDraw
	GLESDebugDraw* _DebugDraw;
	virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags);
#endif
	// implement the "static create()" method manually
	CREATE_FUNC(Level01);
};
