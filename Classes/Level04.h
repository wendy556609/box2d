#pragma once

#include "common/Level.h"

using namespace std;

#define WallCount 10
#define rectWallCount 4

class CLevel04ContactListener : public b2ContactListener
{
public:
	cocos2d::Sprite* _carSprite;
	bool _isFinish;

	//按鈕
	cocos2d::Sprite* _BtnSprite;
	bool _isClickBtn;

	b2Body* _orangeBody;
	int _iType;

	CLevel04ContactListener();
	//碰撞開始
	virtual void BeginContact(b2Contact* contact);
	//碰撞結束
	virtual void EndContact(b2Contact* contact);
	void setCollisionTarget(cocos2d::Sprite& targetSprite);
};

class Level04 : public Level
{
public:
	~Level04();

	static cocos2d::Scene* createScene();

	CLevel04ContactListener _contactListener;

	//按鈕
	cocos2d::Sprite* _BtnSprite;
	b2Body* _BtnBody;
	b2Body* _doorBody;
	bool _bBtnClick;
	bool _isdoorOpen;
	float _stopPosX;

	b2MouseJoint* _mouseJoint;
	cocos2d::Sprite* _dount;
	b2Body* _plateBody;
	b2Body* _plateJoint;
	b2Body* _plateBody1;
	b2Body* _plateJoint1;
	b2Body* _gearBody;
	bool _bOnTouch;

	cocos2d::Sprite* _orange;
	b2Body* _orangeJointBody;
	b2Body* _pullBtnPlateBody;
	float _upPosY;
	float _downPosY;
	bool _bBtnTouch;

	b2Body* _autoPlateBody;
	float _stopPosY;
	float _initPosY;
	int _Type;//0往上 1往下
	bool _isStop;
	float _fTime;

	bool init();
	void update(float dt);

	void Replay();
	void NextLevel();

	void setObject();

	void setButton();
	void setPullJoint();
	void setFilter();
	void setAutoPlate();
	void setPullBtn();
	void setPullBtnSensor();

	bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰開始事件
	void onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰移動事件
	void onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰結束事件 
	// implement the "static create()" method manually
	CREATE_FUNC(Level04);
};
