#pragma once

//#define BOX2D_DEBUG 1

#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "cocostudio/CocoStudio.h"
#include <list>

#include "common/CDraw.h"

#ifdef BOX2D_DEBUG
#include "Common/GLES-Render.h"
#include "Common/GB2ShapeCache-x.h"
#endif

using namespace std;

#define PTM_RATIO 32.0f
#define RepeatCreateBallTime 3
#define AccelerateMaxNum 2
#define AccelerateRatio 1.5f

#define LINE_LENGTH 200 //�i�e�u������

class Level01 : public cocos2d::Scene
{
public:
	~Level01();
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();

	CDraw* _draw;

	Node* _csbRoot;

	// for Box2D
	b2World* _b2World;
	cocos2d::Label* _titleLabel;
	cocos2d::Size _visibleSize;

	// Box2D Examples
	void readBlocksCSBFile(const char*);
	void readSceneFile(const char*);
	void createStaticBoundary();

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	void update(float dt);

	void setCar();

	bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //Ĳ�I�}�l�ƥ�
	void onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //Ĳ�I���ʨƥ�
	void onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //Ĳ�I�����ƥ� 

#ifdef BOX2D_DEBUG
//DebugDraw
	GLESDebugDraw* _DebugDraw;
	virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags);
#endif
	// implement the "static create()" method manually
	CREATE_FUNC(Level01);
};
