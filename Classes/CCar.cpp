#include "CCar.h"

USING_NS_CC;

CCar::CCar() {
	_fstopTime = 0;

	_isStatic = false;
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
	Point pos = _carSprite->getPosition();
	Size size = _carSprite->getContentSize();

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = _carSprite;

	carbody = _b2World->CreateBody(&bodyDef);

	b2FixtureDef fixtureDef;
	fixtureDef.density = 1.0f; fixtureDef.friction = 0.1f; fixtureDef.restitution = 0.25f;
	b2PolygonShape boxShape;
	boxShape.SetAsBox((size.width - 6) * 0.5f / PTM_RATIO, (size.height - 8) * 0.5f / PTM_RATIO);
	fixtureDef.shape = &boxShape;

	carbody->CreateFixture(&fixtureDef);

	//wheel01
	auto wheel = dynamic_cast<Sprite*>(_csbRoot->getChildByName("wheel01"));
	Point wheelposA = wheel->getPosition();
	size = wheel->getContentSize();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(wheelposA.x / PTM_RATIO, wheelposA.y / PTM_RATIO);
	bodyDef.userData = wheel;

	wheelbodyA = _b2World->CreateBody(&bodyDef);

	fixtureDef.restitution = 0.1f;
	b2CircleShape circle;
	circle.m_radius = size.width * 0.5f / PTM_RATIO;
	fixtureDef.shape = &circle;
	fixtureDef.density = 0.5f; fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	wheelbodyA->CreateFixture(&fixtureDef);

	b2RevoluteJoint* RjointA;
	b2RevoluteJointDef jointDef;
	jointDef.Initialize(carbody, wheelbodyA, wheelbodyA->GetWorldCenter());
	RjointA = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&jointDef));

	//wheel02
	wheelbodyB = _b2World->CreateBody(&bodyDef);

	wheel = dynamic_cast<Sprite*>(_csbRoot->getChildByName("wheel02"));
	Point wheelposB = wheel->getPosition();
	size = wheel->getContentSize();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(wheelposB.x / PTM_RATIO, wheelposB.y / PTM_RATIO);
	bodyDef.userData = wheel;

	wheelbodyB = _b2World->CreateBody(&bodyDef);

	circle.m_radius = size.width * 0.5f / PTM_RATIO;
	fixtureDef.shape = &circle;
	fixtureDef.density = 0.5f; fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	wheelbodyB->CreateFixture(&fixtureDef);

	b2RevoluteJoint* RjointB;
	jointDef.Initialize(carbody, wheelbodyB, wheelbodyB->GetWorldCenter());
	_b2World->CreateJoint(&jointDef);
	RjointB = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&jointDef));

	//wheels set gearjoint
	b2GearJointDef GJoint;
	GJoint.bodyA = wheelbodyA;
	GJoint.bodyB = wheelbodyB;
	GJoint.joint1 = RjointA;
	GJoint.joint2 = RjointB;
	GJoint.ratio = -1;
	_b2World->CreateJoint(&GJoint);

	//moveTarget
	pos.x = (wheelposA.x + wheelposB.x) / 2;
	pos.y = -200.0f;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x / PTM_RATIO, pos.y / PTM_RATIO);
	bodyDef.userData = nullptr;

	moveTarget = _b2World->CreateBody(&bodyDef);

	boxShape.SetAsBox((wheelposB.x - wheelposA.x) * 0.5f / PTM_RATIO, 3 / PTM_RATIO);
	fixtureDef.shape = &boxShape;

	moveTarget->CreateFixture(&fixtureDef);

	b2RevoluteJoint* Rjoint;
	jointDef.Initialize(carbody, moveTarget, moveTarget->GetWorldCenter());
	_b2World->CreateJoint(&jointDef);
	Rjoint = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&jointDef));
	/*b2RevoluteJoint *PrJoint;
	PrJoint.Initialize(carbody, moveTarget, moveTarget->GetWorldCenter(), b2Vec2(1.0f, 0));
	b2PrismaticJoint* moveJoint = dynamic_cast<b2PrismaticJoint*>(_b2World->CreateJoint(&PrJoint));*/

	GJoint.bodyA = wheelbodyA;
	GJoint.bodyB = moveTarget;
	GJoint.joint1 = RjointA;
	GJoint.joint2 = Rjoint;
	GJoint.ratio = 10;
	_b2World->CreateJoint(&GJoint);

	GJoint.bodyA = wheelbodyB;
	GJoint.bodyB = moveTarget;
	GJoint.joint1 = RjointB;
	GJoint.joint2 = Rjoint;
	GJoint.ratio = 10;
	_b2World->CreateJoint(&GJoint);
}

void CCar::update(float dt) {
	if (!_isFinish) {
		if (_iState != STOP) {
			float velocity = _fVelocity * dt;
			b2Vec2 vec = carbody->GetWorldVector(b2Vec2(1, 0));
			vec = velocity * vec;
			carbody->ApplyLinearImpulseToCenter(vec, true);
		}
		else {
			if (_isStatic) {
				_fstopTime += dt;
				if (_fstopTime >= 0.5f) {
					_fstopTime = 0;
					carbody->SetType(b2_dynamicBody);
					_isStatic = false;
				}
			}
		}
	}
}

void CCar::setState(int state) {
	switch (state)
	{
	case STOP:
		_fVelocity = 0;
		_iState = STOP;
		carbody->SetLinearVelocity(b2Vec2(0, 0));
		carbody->ApplyLinearImpulseToCenter(b2Vec2(0, 0), false);
		//carbody->SetType(b2_staticBody);
		_isStatic = true;
		break;
	case LEFT:
		_fVelocity = -100.0f;
		_iState = LEFT;
		carbody->SetType(b2_dynamicBody);
		break;
	case RIGHT:
		_fVelocity = 100.0f;
		_iState = RIGHT;
		carbody->SetType(b2_dynamicBody);
		break;
	default:
		break;
	}
}

void CCar::setFinish(cocos2d::Point goalPos) {
	if (!_isFinish) {
		log("finish");
		_isFinish = true;

		b2Vec2 pos = b2Vec2(goalPos.x / PTM_RATIO, goalPos.y / PTM_RATIO);
		carbody->SetTransform(pos, carbody->GetAngle());
	}
}

cocos2d::Sprite* CCar::getCarSprite() {
	return _carSprite;
}