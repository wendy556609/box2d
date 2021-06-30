#pragma once

#include "common/Level.h"
#include "Level04.h"

using namespace std;

#define WallCount 10
#define rectWallCount 3

class CLevel03ContactListener : public b2ContactListener
{
public:
	cocos2d::Sprite* _carSprite;
	bool _isFinish;

	//按鈕
	cocos2d::Sprite* _BtnSprite;
	bool _isClickBtn;

	//剪繩子
	bool _isCut;
	//orange
	int ropeNum;
	b2Body* ropeBody[8];
	//platform
	int rope2Num;
	b2Body* ropeBody2[8];

	b2Body* gearBody[4];
	b2Body* gearSensor[4];
	bool _isOn[4];

	CLevel03ContactListener();
	//碰撞開始
	virtual void BeginContact(b2Contact* contact);
	//碰撞結束
	virtual void EndContact(b2Contact* contact);
	void setCollisionTarget(cocos2d::Sprite& targetSprite);
};

class Level03 : public Level
{
public:
	~Level03();

	static cocos2d::Scene* createScene();

	CLevel03ContactListener _contactListener;

	//剪刀
	cocos2d::Sprite* _cutSprite;
	b2MouseJoint* _mouseJoint;
	bool _bOnTouch;

	//按鈕
	cocos2d::Sprite* _BtnSprite;
	b2Body* _BtnBody;
	b2Body* _FloorBody;
	b2Body* platJoint;
	bool _bBtnClick;
	bool _isFloorUp;
	float _stopPosY;

	//繩子
	//orange
	b2RopeJoint* ropeJoint;
	b2RevoluteJoint* ropeReJoint[8];
	b2Body* ropeBody[8];
	//platform
	b2RopeJoint* ropeJoint2;
	b2RevoluteJoint* ropeReJoint2[8];
	b2Body* ropeBody2[8];

	//齒輪
	b2Body* gearBody[10];
	b2Body* gearJoint[10];
	b2Body* gearSensor[4];
	b2GearJointDef GJoint[9];
	b2PrismaticJoint* gearPriJoint[4];
	b2RevoluteJoint* RevJoint[10];
	float ratio[8];
	bool _isOn[4];
	bool _isTurn;
	/*------*/
	//gear[1]--GJoint[0] GJoint[1]
	//gear[2]--GJoint[1] GJoint[2]
	//gear[4]--GJoint[3] GJoint[4]
	//gear[7]--GJoint[6] GJoint[7]
	/*------*/

	//甜甜圈
	cocos2d::Point _dountPos;
	float  _dountScale;
	bool _dountBorn;

	bool init();
	void update(float dt);

	void Replay();
	void NextLevel();

	void setObject();

	void setButton();
	void setMouseJoint();
	void setRopeJoint();
	void setWeldJoint();
	void setGear();
	void setGearSensor();
	void setGearOn();

	bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰開始事件
	void onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰移動事件
	void onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰結束事件 
	// implement the "static create()" method manually
	CREATE_FUNC(Level03);
};
