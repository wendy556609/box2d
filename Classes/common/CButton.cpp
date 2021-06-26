#include "CButton.h"

USING_NS_CC;

CButton::CButton() {
	_Btn = nullptr;
	_normal = nullptr;
	_touched = nullptr;
	_icon = nullptr;

	_bTouch = false;
	_bswitchOn = false;
}

CButton::~CButton() {

}

void CButton::init(const std::string& normal, const std::string& touch, cocos2d::Point pos, cocos2d::Node& scene, int type) {
	_Btn = Node::create();
	
	_normal = Sprite::createWithSpriteFrameName(normal);
	_touched = Sprite::createWithSpriteFrameName(touch);

	_pos = pos;
	_size = _normal->getContentSize();
	_rect = Rect(_pos.x - (_size.width / 2), _pos.y - (_size.height / 2), _size.width, _size.height);

	_Btn->setPosition(_pos);
	_normal->setPosition(Point(0, 0));
	_touched->setPosition(Point(0, 0));

	_normal->setVisible(true);
	_touched->setVisible(false);
	_Btn->setScale(1.0f);

	scene.addChild(_Btn, 5);

	_Btn->addChild(_normal, 5);
	_Btn->addChild(_touched, 5);

	_iType = type;
}

void CButton::setIcon(const std::string& icon, bool reverse) {
	_icon = Sprite::createWithSpriteFrameName(icon);
	_icon->setPosition(Point(0, 0));

	if (reverse)
		_icon->setScaleX(-1);

	_Btn->addChild(_icon);
}

void CButton::setColor(const cocos2d::Color3B color) {
	_normal->setColor(color);
	_touched->setColor(color);
}

void CButton::setScale(float scale) {
	_size.width *= scale;
	_size.height *= scale;
	_rect = Rect(_pos.x - (_size.width / 2), _pos.y - (_size.height / 2), _size.width, _size.height);
	
	_normal->setScale(scale);
	_touched->setScale(scale);
}

void CButton::switchBtn() {
	_bTouch = !_bTouch;
	_bswitchOn = !_bswitchOn;
	if (_bswitchOn) {
		_touched->setVisible(true);
		_normal->setVisible(false);

		_Btn->setScale(1.25f);
	}
	else {
		_touched->setVisible(false);
		_normal->setVisible(true);

		_Btn->setScale(1.0f);
	}
}

bool CButton::getState() {
	return _bswitchOn;
}

bool CButton::onTouchBegan(cocos2d::Point touchLoc) {
	if (_rect.containsPoint(touchLoc)) {
		_normal->setVisible(false);
		_touched->setVisible(true);
		_Btn->setScale(1.25f);
		_bTouch = true;
		return true;
	}
	
	return false;
}

bool CButton::onTouchMoved(cocos2d::Point touchLoc) {
	switch (_iType)
	{
	case oneClick:
		if (_bTouch) {
			if (!_rect.containsPoint(touchLoc)) {
				_normal->setVisible(true);
				_touched->setVisible(false);
				_Btn->setScale(1.0f);
				_bTouch = false;
				return false;
			}
			return true;
		}
		break;
	case Switch:
		if (_bTouch) {
			if (!_rect.containsPoint(touchLoc)) {
				if (_bswitchOn) {
					_normal->setVisible(false);
					_touched->setVisible(true);
					_Btn->setScale(1.25f);
				}
				else {
					_normal->setVisible(true);
					_touched->setVisible(false);
					_Btn->setScale(1.0f);
				}
				_bTouch = false;
				return false;
			}
			return true;
		}
		break;
	default:
		break;
	}
	
	return false;
}

bool CButton::onTouchEnded(cocos2d::Point touchLoc) {
	switch (_iType)
	{
	case oneClick:
		if (_rect.containsPoint(touchLoc)) {
			_normal->setVisible(true);
			_touched->setVisible(false);
			_Btn->setScale(1.0f);
			_bTouch = false;
			return true;
		}
		break;
	case Switch:
		if (_rect.containsPoint(touchLoc)) {
			_bswitchOn = !_bswitchOn;
			if (_bswitchOn) {
				_normal->setVisible(false);
				_touched->setVisible(true);
				_Btn->setScale(1.25f);
			}
			else {
				_normal->setVisible(true);
				_touched->setVisible(false);
				_Btn->setScale(1.0f);
			}
			_bTouch = false;
			return true;
		}
		break;
	default:
		break;
	}
	
	return false;
}