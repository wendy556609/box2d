#pragma once

#include "cocos2d.h"
#include "ui/CocosGUI.h" // For Cocos Studio 控制項元件
#include "cocostudio/CocoStudio.h"
#include <list>

using namespace std;

#define FALLING_TIME 2.5f
#define MAX_HEIGHT 720.0f
#define PIXEL_PERM (2.0f*MAX_HEIGHT/(9.8f*FALLING_TIME*FALLING_TIME))
#define GRAVITY_Y(t,dt,g) ((g)*((t)+0.5f*(dt)))
#define LIFE_NOISE(f) ((f)*(1.0f-(rand()%2001/1000.0f)))

//particleType
#define CARSMOKE 0
#define DRAWINK 1

class CParticle
{
private:
	cocos2d::Sprite* _Particle;
	cocos2d::Point _Pos;
	cocos2d::Point _Dir;

	float _fLifeTime;
	float _fTime;

	float _fOpacity;
	float _fSpeed;
	float _fSize;

	cocos2d::Color3B _color;
	std::string _SpriteName;

	bool _bVisible;

	int _iType;
public:

	CParticle();

	void setProperty(std::string pngname, cocos2d::Scene& stage);
	bool update(float dt);
	void setBehavior(int type);

	void setPosition(cocos2d::Point pos);
	void setVisible(bool visible);

	void setOpacity(float Opacity);
	void setSpeed(float Speed);
	void setColor(cocos2d::Color3B color);

	void setSprite(std::string& spriteName);
};

class CParticleSystem
{
private:
	CParticle* _Particle;
	list<CParticle*> _FreeList;
	list<CParticle*> _InUsedList;

	int _iInUsed;
	int _iFree;

	cocos2d::Point _bornPos;
	float _fOpacity;
	float _fSpeed;

	cocos2d::Color3B _color;
	std::string _SpriteName;

	bool _bVisible;
public:
	float _fTime;
	float _fTouchTime;

	CParticleSystem();
	~CParticleSystem();

	void init(cocos2d::Scene& stage); 
	void update(float dt);

	void setPosition(cocos2d::Point pos);

	void setOpacity(float Opacity);
	void setSpeed(float Speed);
	void setColor(cocos2d::Color4F color);

	void setSprite(std::string& spriteName);

	void onTouchBegan(cocos2d::Point touchLoc);
	void onTouchMoved(cocos2d::Point touchLoc);
	void onTouchEnded(cocos2d::Point touchLoc);
};