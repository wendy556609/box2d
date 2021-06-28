#pragma once

#include "common/Level.h"
#include "Level02.h"

using namespace std;

#define WallCount 4
#define rectWallCount 8

class CLevel01ContactListener : public b2ContactListener
{
public:
	cocos2d::Sprite* _carSprite;
	bool _isFinish;

	//按鈕
	cocos2d::Sprite* _BtnSprite;
	bool _isClickBtn;

	CLevel01ContactListener();
	//碰撞開始
	virtual void BeginContact(b2Contact* contact);
	//碰撞結束
	virtual void EndContact(b2Contact* contact);
	void setCollisionTarget(cocos2d::Sprite& targetSprite);
};

class Level01 : public Level
{
public:
	~Level01();

	static cocos2d::Scene* createScene();

	CLevel01ContactListener _contactListener;

	cocos2d::Sprite* _BtnSprite;
	b2Body* _BtnBody;
	b2Body* _doorBody;
	bool _bBtnClick;
	bool _isDoorOpen;
	float _stopPosY;

	cocos2d::Sprite* _frameSprite;
	b2MouseJoint* _mouseJoint;
	bool _bOnTouch;

	bool init();
	void update(float dt);

	void Replay();
	void NextLevel();

	void setObject();

	void setPullJoint();
	void setSeesaw();
	void setButton();
	void setMouseJoint();

	bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰開始事件
	void onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰移動事件
	void onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰結束事件 
	// implement the "static create()" method manually
	CREATE_FUNC(Level01);
};
