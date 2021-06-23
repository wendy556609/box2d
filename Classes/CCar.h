#pragma once

#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "cocostudio/CocoStudio.h"

#ifdef BOX2D_DEBUG
#include "Common/GLES-Render.h"
#include "Common/GB2ShapeCache-x.h"
#endif

using namespace std;

#define PTM_RATIO 32.0f
#define RepeatCreateBallTime 3
#define AccelerateMaxNum 2
#define AccelerateRatio 1.5f

enum State {
	STOP = 0,
	LEFT,
	RIGHT
};

class CCar
{
public:
	CCar();
	~CCar();

	cocos2d::Node* _csbRoot;

	// for Box2D
	b2World* _b2World;

	cocos2d::Sprite* _carSprite;

	b2Body* carbody;
	b2Body* wheelbodyA;
	b2Body* wheelbodyB;
	b2Body* moveTarget;

	float _fVelocity;
	float _fstopTime;

	bool _isStatic;
	bool _isFinish;

	int _iState;

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	void init(cocos2d::Node& csbroot, b2World& world);
	void update(float dt);

	void setCar();
	void setState(int state);
	void setFinish(cocos2d::Point goalPos);

	cocos2d::Sprite* getCarSprite();

#ifdef BOX2D_DEBUG
//DebugDraw
	GLESDebugDraw* _DebugDraw;
	virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags);
#endif
};
