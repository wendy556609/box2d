#include "CDraw.h"

USING_NS_CC;

CDraw::CDraw() {
	_b2World = nullptr;
	_scene = nullptr;
	_drawNode = nullptr;

	_lineBody = nullptr;
	lineShape = nullptr;

	_bTouch = false;
}

CDraw::~CDraw() {

}

void CDraw::init(b2World& world, cocos2d::Node& scene) {
	_b2World = &world;
	_scene = &scene;

	for (int i = 0; i < LINE_LENGTH; i++)
	{
		_linepos[i] = cocos2d::Point(0, 0);
		_angle[i] = 0;
	}

	lineShape = new b2PolygonShape();

	_ilineFree = LINE_LENGTH;
	_ilineUse = 0;

	setPenColor(Color4F::BLACK);
}

bool CDraw::onTouchBegan(cocos2d::Point touchLoc) {
	if (_ilineFree > 0) {
		_preLoc = touchLoc;
		_bTouch = true;

		_linepos[_ilineUse] = _preLoc;
		_ilineUse++; _ilineFree--;

		_drawNode = DrawNode::create();
		_scene->addChild(_drawNode, 5);
	}
	return true;
}

void CDraw::onTouchMoved(cocos2d::Point touchLoc) {

	Point _currentLoc;

	if (_bTouch) {
		_currentLoc = touchLoc;
		if (_ilineFree > 0) {
			_linepos[_ilineUse] = _currentLoc;

			_drawNode->drawLine(_preLoc, _currentLoc, _color);
			_angle[_ilineUse] = -ccpAngleSigned(ccpSub(_currentLoc, _preLoc), ccp(1.0f, 0.0f));
			_ilineUse++; _ilineFree--;
		}
		_preLoc = _currentLoc;
	}

}

void CDraw::onTouchEnded(cocos2d::Point touchLoc) {

	if (_bTouch) {
		if (_ilineUse > 2) {
			b2FixtureDef lineFixtureDef;

			Point pos = _drawNode->getPosition();
			Point preLoc, curLoc, startLoc;
			b2Vec2 center;

			b2BodyDef lineDef;
			lineDef.type = b2_dynamicBody;
			lineDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
			lineDef.userData = _drawNode;
			_lineBody = _b2World->CreateBody(&lineDef);

			for (int i = 0; i < (_ilineUse - 1); i++)
			{
				preLoc = _linepos[i]; curLoc = _linepos[i + 1];

				float32 segLength = ccpDistance(preLoc, curLoc) * 0.5f;

				lineVec[0] = b2Vec2(preLoc.x / PTM_RATIO, preLoc.y / PTM_RATIO);
				lineVec[1] = b2Vec2(curLoc.x / PTM_RATIO, curLoc.y / PTM_RATIO);

				center = b2Vec2((lineVec[0] + lineVec[1]).x / 2, (lineVec[0] + lineVec[1]).y / 2);

				lineShape->SetAsBox(segLength / PTM_RATIO, 0.5f / PTM_RATIO, center, _angle[i]);
				lineFixtureDef.shape = lineShape;
				lineFixtureDef.filter.categoryBits = 1 << colorNum;
				_lineBody->CreateFixture(&lineFixtureDef);
			}
		}
		_ilineUse = 0;
		_ilineFree = LINE_LENGTH;
		_bTouch = false;
	}
}

void CDraw::setPenColor(cocos2d::Color4F color, int num) {
	_color = color;
	colorNum = num;
}

cocos2d::Color4F CDraw::getPenColor() {
	return _color;
}