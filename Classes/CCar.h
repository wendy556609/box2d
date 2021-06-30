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
private:
	cocos2d::Sprite* _carSprite;

	b2Body* _carBody;
	b2Body* _wheelBodyA;
	b2Body* _wheelBodyB;
	b2Body* _moveTarget;

	cocos2d::Point _carPos;
	b2Vec2 _locPos;

	float _fVelocity;

	bool _isFinish;

	int _iState;
public:
	CCar();
	~CCar();

	cocos2d::Node* _csbRoot;

	// for Box2D
	b2World* _b2World;

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	void init(cocos2d::Node& csbroot, b2World& world);
	void update(float dt);

	void setCar();
	void setState(int state);
	void setFinish(cocos2d::Point goalPos);

	cocos2d::Sprite* getCarSprite();
	cocos2d::Point getCarPos();
	b2Body* getCarBody();
	b2Vec2 getCarLocPos();

#ifdef BOX2D_DEBUG
//DebugDraw
	GLESDebugDraw* _DebugDraw;
	virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags);
#endif
};
