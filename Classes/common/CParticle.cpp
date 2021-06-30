#include "CParticle.h"

USING_NS_CC;

#define NUM_PARTICLES 500

inline int INTENSITY(float f) {
	if (f >= 255) return(255);
	else if (f <= 0) return(0);
	else return((int)f);
}

CParticle::CParticle() {
	_Particle = nullptr;
}

void CParticle::setProperty(std::string pngname, cocos2d::Scene& stage) {
	_Particle = Sprite::createWithSpriteFrameName(pngname);
	_Particle->setPosition(Point(0, 0));
	_color = Color3B(255, 255, 255);
	setVisible(false);

	BlendFunc blendfunc = { GL_ONE, GL_ONE };
	//_Particle->setBlendFunc(BlendFunc::ADDITIVE);

	stage.addChild(_Particle, 3);
}

bool CParticle::update(float dt) {
	float sint, cost, sin2t;
	float size;
	if (!_bVisible && _fTime == 0) {
		_Particle->setPosition(_Pos);
		_Particle->setColor(_color);
		setVisible(true);
	}
	else if (_fTime > _fLifeTime) {
		setVisible(false);
		return true;
	}
	else {
		switch (_iType)
		{
		case CARSMOKE:
			sint = sinf(M_PI * (_fTime / _fLifeTime));
			cost = cosf(M_PI_2 * (_fTime / _fLifeTime));
			sin2t = sinf(M_PI_2 * (_fTime / _fLifeTime));
			size = _fSize * sin2t;
			_Particle->setScale(size);
			_Particle->setOpacity(_fOpacity * cost);
			
			_Pos.x += (_Dir.x * cost * _fSpeed) * dt * PIXEL_PERM;
			_Pos.y += (_Dir.y * cost * _fSpeed) * dt * PIXEL_PERM;
			_Particle->setPosition(_Pos);
			break;
		case DRAWINK:
			sint = sinf(M_PI * (_fTime / _fLifeTime));
			cost = cosf(M_PI_2 * (_fTime / _fLifeTime));
			_fSize = 0.5f * cost;
			_Particle->setScale(_fSize);
			_Particle->setOpacity(_fOpacity);

			_Pos.x += (_Dir.x * cost * _fSpeed) * dt * PIXEL_PERM;
			_Pos.y += (_Dir.y * cost * _fSpeed) * dt * PIXEL_PERM;
			_Particle->setPosition(_Pos);
			break;
		default:
			break;
		}

	}
	_fTime += dt;
	return false;
}

void CParticle::setBehavior(int type) {
	float t;
	switch (type)
	{
	case CARSMOKE:
		_fTime = 0;
		_fLifeTime = 3.0f + LIFE_NOISE(0.2f);
		_fOpacity = 64;
		_Particle->setOpacity(_fOpacity);
		_fSpeed = 1.0f + (rand() % 101 / 100.0f) - 0.5f;
		_fSize = 1.0f + (rand() % 100 / 100.0f);
		_Particle->setScale(0);
		_Dir.x = -1;
		_Dir.y = (rand() % 201 / 100.0f) - 1.0f;
		_SpriteName = "cloud.png";
		_Particle->setSpriteFrame(_SpriteName);
		_color = Color3B::GRAY;
		_iType = CARSMOKE;
		break;
	case DRAWINK:
		_fTime = 0;
		_fLifeTime = 2.0f + LIFE_NOISE(0.15f);
		_fOpacity = 255;
		_fSize = 1;
		_fSpeed = 2.0f + (rand() % 201 / 100.0f);
		t = 2.0f * M_PI * (rand() % 100 / 100.0f);
		_Dir.x = sinf(t);
		_Dir.y = cosf(t);
		_SpriteName = "flare.png";
		_Particle->setSpriteFrame(_SpriteName);
		_iType = DRAWINK;
		break;
	default:
		break;
	}
}

void CParticle::setPosition(cocos2d::Point pos) {
	_Pos = pos;
}

void CParticle::setVisible(bool visible) {
	_bVisible = visible;
	_Particle->setVisible(visible);
}

void CParticle::setOpacity(float Opacity) {
	_fOpacity = Opacity;
}

void CParticle::setSpeed(float Speed) {
	_fSpeed = Speed;
}

void CParticle::setColor(cocos2d::Color3B color) {
	_color = color;
}

void CParticle::setSprite(std::string& spriteName) {
	_SpriteName = spriteName;
	_Particle->setSpriteFrame(_SpriteName);
}


//ParticleSystem

CParticleSystem::CParticleSystem() {
	_Particle = nullptr;
}

CParticleSystem::~CParticleSystem() {
	if (_iInUsed != 0) _InUsedList.clear(); // 移除所有的 NODE
	if (_iFree != 0) _FreeList.clear();
	delete[] _Particle; // 釋放所有取得資源
	//SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
}

void CParticleSystem::init(cocos2d::Scene& stage) {
	_iFree = NUM_PARTICLES;
	_iInUsed = 0;
	//SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");
	_Particle = new CParticle[NUM_PARTICLES];

	for (int i = 0; i < NUM_PARTICLES; i++)
	{
		_Particle[i].setProperty("flare.png", stage);
		_FreeList.push_front(&_Particle[i]);
	}

	_fTime = 0;
}

void CParticleSystem::update(float dt) {
	CParticle* get;
	list <CParticle*>::iterator it;
	
	_fTime += dt;
	if (_fTime >= 0.2f) {
		_fTime = 0;
		if (_iFree != 0) {
			for (int i = 0; i < 5; i++)
			{
				get = _FreeList.front();
				get->setPosition(_bornPos);
				get->setBehavior(CARSMOKE);
				_FreeList.pop_front();
				_InUsedList.push_front(get);
				_iFree--;
				_iInUsed++;
			}
		}
	}

	if (_iInUsed != 0) {
		for (it = _InUsedList.begin(); it != _InUsedList.end();)
		{
			if ((*it)->update(dt)) {
				_FreeList.push_front((*it));
				it = _InUsedList.erase(it);
				_iFree++;
				_iInUsed--;
			}
			else it++;
		}
	}
}

void CParticleSystem::setPosition(cocos2d::Point pos) {
	_bornPos = pos;
}

void CParticleSystem::setOpacity(float Opacity) {
	_fOpacity = Opacity;
}

void CParticleSystem::setSpeed(float Speed) {
	_fSpeed = Speed;
}

void CParticleSystem::setColor(Color4F color) {
	_color = Color3B(color.r * 255, color.g * 255, color.b * 255);
}

void CParticleSystem::setSprite(std::string& spriteName) {
	_SpriteName = spriteName;
}

void CParticleSystem::onTouchBegan(cocos2d::Point touchLoc) {
	CParticle* get;
	if (_iFree != 0) {
		_fTouchTime = 0;
		get = _FreeList.front();
		get->setPosition(touchLoc);
		get->setColor(_color);
		get->setBehavior(DRAWINK);
		_FreeList.pop_front();
		_InUsedList.push_front(get);
		_iFree--;
		_iInUsed++;
	}
	else return;
}

void CParticleSystem::onTouchMoved(cocos2d::Point touchLoc) {
	CParticle* get;
	_fTouchTime += 1;
	if (_fTouchTime >= 10.0f) {
		_fTouchTime = 0;
		if (_iFree != 0) {
			get = _FreeList.front();
			get->setPosition(touchLoc);
			get->setColor(_color);
			get->setBehavior(DRAWINK);
			_FreeList.pop_front();
			_InUsedList.push_front(get);
			_iFree--;
			_iInUsed++;
		}
	}
	
	else return;
}

void CParticleSystem::onTouchEnded(cocos2d::Point touchLoc) {

}