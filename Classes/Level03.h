#pragma once

#include "common/Level.h"

using namespace std;

#define WallCount 10
#define rectWallCount 3

class CLevel03ContactListener : public b2ContactListener
{
public:
	cocos2d::Sprite* _carSprite;
	bool _isFinish;

	//���s
	cocos2d::Sprite* _BtnSprite;
	bool _isClickBtn;

	//��÷�l
	bool _isCut;
	//orange
	int ropeNum;
	b2Body* ropeBody[8];
	//platform
	int rope2Num;
	b2Body* ropeBody2[8];

	CLevel03ContactListener();
	//�I���}�l
	virtual void BeginContact(b2Contact* contact);
	//�I������
	virtual void EndContact(b2Contact* contact);
	void setCollisionTarget(cocos2d::Sprite& targetSprite);
};

class Level03 : public Level
{
public:
	~Level03();

	static cocos2d::Scene* createScene();

	CLevel03ContactListener _contactListener;

	//�ŤM
	cocos2d::Sprite* _cutSprite;
	b2MouseJoint* _mouseJoint;
	bool _bOnTouch;

	//���s
	cocos2d::Sprite* _BtnSprite;
	b2Body* _BtnBody;
	b2Body* _FloorBody;
	bool _bBtnClick;
	bool _isFloorUp;
	float _stopPosY;

	//÷�l
	//orange
	b2RopeJoint* ropeJoint;
	b2RevoluteJoint* ropeReJoint[8];
	b2Body* ropeBody[8];
	//platform
	b2RopeJoint* ropeJoint2;
	b2RevoluteJoint* ropeReJoint2[8];
	b2Body* ropeBody2[8];

	b2Body* platJoint;
	b2Body* distanceBody;
	b2Body* disJointBody;

	b2Body* gearBody[10];
	b2GearJointDef GJoint[9];
	/*------*/
	//gear[1]--GJoint[0] GJoint[1]
	//gear[2]--GJoint[1] GJoint[2]
	//gear[4]--GJoint[3] GJoint[4]
	//gear[7]--GJoint[6] GJoint[7]
	/*------*/

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
	void setDistanceJoint();
	void setGear();

	bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //Ĳ�I�}�l�ƥ�
	void onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //Ĳ�I���ʨƥ�
	void onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //Ĳ�I�����ƥ� 
	// implement the "static create()" method manually
	CREATE_FUNC(Level03);
};
