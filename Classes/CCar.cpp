#include "CCar.h"

USING_NS_CC;

CCar::CCar() {
	_b2World = nullptr;
	_csbRoot = nullptr;
	_carSprite = nullptr;

	_carBody = nullptr;
	_wheelBodyA = nullptr;
	_wheelBodyB = nullptr;
	_moveTarget = nullptr;

	_isFinish = false;
}

CCar::~CCar() {

}

void CCar::init(cocos2d::Node& csbroot, b2World& world) {
	_csbRoot = &csbroot;
	_b2World = &world;

	setCar();
	setState(STOP);
}

void CCar::setCar() {
	//car
	_carSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("car"));
	_carPos = _carSprite->getPosition();
	Size size = _carSprite->getContentSize();

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(_carPos.x / PTM_RATIO, _carPos.y / PTM_RATIO);
	bodyDef.userData = _carSprite;

	_carBody = _b2World->CreateBody(&bodyDef);

	b2FixtureDef fixtureDef;
	fixtureDef.density = 1.0f; fixtureDef.friction = 0.1f; fixtureDef.restitution = 0.25f;
	b2PolygonShape boxShape;
	boxShape.SetAsBox((size.width - 6) * 0.5f / PTM_RATIO, (size.height - 8) * 0.5f / PTM_RATIO);
	fixtureDef.filter.categoryBits = 1 << 1;
	fixtureDef.shape = &boxShape;

	_carBody->CreateFixture(&fixtureDef);

	_locPos = b2Vec2(-100 / PTM_RATIO, -10 / PTM_RATIO);

	//wheel01
	auto wheel = dynamic_cast<Sprite*>(_csbRoot->getChildByName("wheel01"));
	Point wheelposA = wheel->getPosition();
	size = wheel->getContentSize();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(wheelposA.x / PTM_RATIO, wheelposA.y / PTM_RATIO);
	bodyDef.userData = wheel;

	_wheelBodyA = _b2World->CreateBody(&bodyDef);

	fixtureDef.restitution = 0.1f;
	b2CircleShape circle;
	circle.m_radius = size.width * 0.5f / PTM_RATIO;
	fixtureDef.shape = &circle;
	fixtureDef.density = 0.5f; fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	fixtureDef.filter.categoryBits = 1 << 1;
	_wheelBodyA->CreateFixture(&fixtureDef);

	b2RevoluteJoint* RjointA;
	b2RevoluteJointDef jointDef;
	jointDef.Initialize(_carBody, _wheelBodyA, _wheelBodyA->GetWorldCenter());
	RjointA = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&jointDef));

	//wheel02
	_wheelBodyB = _b2World->CreateBody(&bodyDef);

	wheel = dynamic_cast<Sprite*>(_csbRoot->getChildByName("wheel02"));
	Point wheelposB = wheel->getPosition();
	size = wheel->getContentSize();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(wheelposB.x / PTM_RATIO, wheelposB.y / PTM_RATIO);
	bodyDef.userData = wheel;

	_wheelBodyB = _b2World->CreateBody(&bodyDef);

	circle.m_radius = size.width * 0.5f / PTM_RATIO;
	fixtureDef.shape = &circle;
	fixtureDef.density = 0.5f; fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	fixtureDef.filter.categoryBits = 1 << 1;
	_wheelBodyB->CreateFixture(&fixtureDef);

	b2RevoluteJoint* RjointB;
	jointDef.Initialize(_carBody, _wheelBodyB, _wheelBodyB->GetWorldCenter());
	_b2World->CreateJoint(&jointDef);
	RjointB = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&jointDef));

	//wheels set gearjoint
	b2GearJointDef GJoint;
	GJoint.bodyA = _wheelBodyA;
	GJoint.bodyB = _wheelBodyB;
	GJoint.joint1 = RjointA;
	GJoint.joint2 = RjointB;
	GJoint.ratio = -1;
	_b2World->CreateJoint(&GJoint);

	//moveTarget
	Point pos;
	pos.x = (wheelposA.x + wheelposB.x) / 2;
	pos.y = -400.0f;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = nullptr;

	_moveTarget = _b2World->CreateBody(&bodyDef);

	boxShape.SetAsBox((wheelposB.x - wheelposA.x) * 0.5f / PTM_RATIO, 3 / PTM_RATIO);
	fixtureDef.shape = &boxShape;

	_moveTarget->CreateFixture(&fixtureDef);

	b2RevoluteJoint* Rjoint;
	jointDef.Initialize(_carBody, _moveTarget, _moveTarget->GetWorldCenter());
	_b2World->CreateJoint(&jointDef);
	Rjoint = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&jointDef));

	GJoint.bodyA = _wheelBodyA;
	GJoint.bodyB = _moveTarget;
	GJoint.joint1 = RjointA;
	GJoint.joint2 = Rjoint;
	GJoint.ratio = 10;
	_b2World->CreateJoint(&GJoint);

	GJoint.bodyA = _wheelBodyB;
	GJoint.bodyB = _moveTarget;
	GJoint.joint1 = RjointB;
	GJoint.joint2 = Rjoint;
	GJoint.ratio = 10;
	_b2World->CreateJoint(&GJoint);
}

void CCar::update(float dt) {
	if (!_isFinish) {
		if (_iState != STOP) {
			float velocity = _fVelocity * dt;
			b2Vec2 vec = _carBody->GetWorldVector(b2Vec2(1, 0));
			vec = velocity * vec;
			_carBody->ApplyLinearImpulseToCenter(vec, true);
		}
	}
}

void CCar::setState(int state) {
	switch (state)
	{
	case STOP:
		_fVelocity = 0;
		_iState = STOP;
		_carBody->SetLinearVelocity(b2Vec2(0, 0));
		_carBody->ApplyLinearImpulseToCenter(b2Vec2(0, 0), false);
		break;
	case LEFT:
		_fVelocity = -100.0f;
		_iState = LEFT;
		break;
	case RIGHT:
		_fVelocity = 100.0f;
		_iState = RIGHT;
		break;
	default:
		break;
	}
}

void CCar::setFinish(cocos2d::Point goalPos) {
	if (!_isFinish) {
		_isFinish = true;
		_carBody->SetLinearVelocity(b2Vec2(0, 0));
		b2Vec2 pos = b2Vec2(goalPos.x / PTM_RATIO, goalPos.y / PTM_RATIO);
		_carBody->SetTransform(pos, _carBody->GetAngle());
	}
}

cocos2d::Sprite* CCar::getCarSprite() {
	return _carSprite;
}

cocos2d::Point CCar::getCarPos() {
	return _carPos;
}

b2Body* CCar::getCarBody() {
	return _carBody;
}

b2Vec2 CCar::getCarLocPos() {
	return _locPos;
}