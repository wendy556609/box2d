#pragma once

//#define BOX2D_DEBUG 1

#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

#include "common/CDraw.h"
#include "common/CButton.h"
#include "CCar.h"

#ifdef BOX2D_DEBUG
#include "Common/GLES-Render.h"
#include "Common/GB2ShapeCache-x.h"
#endif

using namespace std;
//using namespace cocostudio::timeline;

USING_NS_CC;

#define MAX_2(X,Y) (X)>(Y) ? (X) : (Y)

#define PTM_RATIO 32.0f
#define RepeatCreateBallTime 3
#define AccelerateMaxNum 2
#define AccelerateRatio 1.5f

#define ENDTIME 3.0f

class Level : public cocos2d::Scene
{
public:
	Level();
	~Level();

	int wallCount;
	int rectCount;

	float _endCount;

	Node* _csbRoot;

	CCar* _car;

	CDraw* _draw;
	CButton* _leftBtn;
	CButton* _rightBtn;
	CButton* _retryBtn;

	CButton* _redBtn;
	CButton* _greenBtn;
	CButton* _blueBtn;

	b2Body* _bottomBody;

	cocos2d::Point _goalPos;
	cocos2d::Sprite* _stopLight;

	// for Box2D
	b2World* _b2World;
	cocos2d::Label* _titleLabel;
	cocos2d::Size _visibleSize;

	// Box2D Examples
	void readBlocksCSBFile(const char*);
	void readSceneFile(const char*);

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	void setInitObject();
	virtual bool init() = 0;
	virtual void update(float dt) = 0;

	virtual void createStaticBoundary(int wall, int rect);
	virtual void setGoalSensor();

	virtual void Replay() = 0;
	virtual void NextLevel() = 0;

	void setPenColor(cocos2d::Color4F color, int num = 1);

	virtual bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) = 0; //觸碰開始事件
	virtual void onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) = 0; //觸碰移動事件
	virtual void onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) = 0; //觸碰結束事件 

#ifdef BOX2D_DEBUG
//DebugDraw
	GLESDebugDraw* _DebugDraw;
	virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags);
#endif
};
