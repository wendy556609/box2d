#pragma once

#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "cocostudio/CocoStudio.h"

using namespace std;

#define PTM_RATIO 32.0f

#define LINE_LENGTH 200 //�i�e�u������

class CDraw
{
private:
	// for Box2D
	b2World* _b2World;
	cocos2d::Node* _scene;

	cocos2d::Point _linepos[LINE_LENGTH];
	float _angle[LINE_LENGTH];

	cocos2d::DrawNode* _drawNode;

	int _ilineUse;
	int _ilineFree;

	b2Vec2 lineVec[2];
	b2Body* _lineBody;
	b2PolygonShape* lineShape;

	cocos2d::Point _preLoc;
	bool _bTouch;
public:
	CDraw();
	~CDraw();

	void init(b2World& world,cocos2d::Node &scene);

	bool onTouchBegan(cocos2d::Point touchLoc); //Ĳ�I�}�l�ƥ�
	void onTouchMoved(cocos2d::Point touchLoc); //Ĳ�I���ʨƥ�
	void onTouchEnded(cocos2d::Point touchLoc); //Ĳ�I�����ƥ� 
};
