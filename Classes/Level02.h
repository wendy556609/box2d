#pragma once

#include "common/Level.h"
#include "common/CButton.h"

using namespace std;

#define WallCount 4
#define rectWallCount 9

class CLevel02ContactListener : public b2ContactListener
{
public:
	cocos2d::Sprite* _carSprite;
	bool _isFinish;

	bool _isCut;
	int ropeNum;
	int rope2Num;

	//orange
	b2Body* ropeBody[5];
	//platform
	b2Body* ropeBody2[8];

	CLevel02ContactListener();
	//碰撞開始
	virtual void BeginContact(b2Contact* contact);
	//碰撞結束
	virtual void EndContact(b2Contact* contact);
	void setCollisionTarget(cocos2d::Sprite& targetSprite);
};

class Level02 : public Level
{
public:
	~Level02();

	static cocos2d::Scene* createScene();

	CLevel02ContactListener _contactListener;

	cocos2d::Sprite* _cutSprite;
	b2MouseJoint* _mouseJoint;
	bool _bOnTouch;

	//orange
	b2RopeJoint* ropeJoint;
	b2RevoluteJoint* ropeReJoint[5];
	b2Body* ropeBody[5];
	//platform
	b2RopeJoint* ropeJoint2;
	b2RevoluteJoint* ropeReJoint2[8];
	b2Body* ropeBody2[8];

	bool init();
	void update(float dt);

	void Replay();
	void NextLevel();

	void setObject();

	void setPullJoint();
	void setMouseJoint();
	void setRopeJoint();
	void setFilter();

	bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰開始事件
	void onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰移動事件
	void onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰結束事件 
	// implement the "static create()" method manually
	CREATE_FUNC(Level02);
};
