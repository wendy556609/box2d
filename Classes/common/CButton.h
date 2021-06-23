#pragma once

#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "cocostudio/CocoStudio.h"

class CButton
{
private:
	cocos2d::Node* _Btn;
	cocos2d::Sprite* _normal;
	cocos2d::Sprite* _touched;
	cocos2d::Sprite* _icon;

	cocos2d::Point _pos;
	cocos2d::Size _size;
	cocos2d::Rect _rect;

	bool _bTouch;
public:
	CButton();
	~CButton();

	void init(const std::string& normal, const std::string& touch, cocos2d::Point pos, cocos2d::Node& scene);
	void setIcon(const std::string& icon, bool reverse = false);

	bool onTouchBegan(cocos2d::Point touchLoc);
	bool onTouchMoved(cocos2d::Point touchLoc);
	bool onTouchEnded(cocos2d::Point touchLoc);
};
