#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "Vector3.h"

struct GameObject
{
	enum powerUp {
		ballUp = 0,
		rechargeUp,
		heal,
		extend
	};
	enum GAMEOBJECT_TYPE
	{
		GO_NONE = 0,
		GO_BALL,
		GO_CUBE,
		GO_WALL,
		GO_PILLAR,
		GO_POWERUP,
		GO_TOTAL, //must be last
	};
	GAMEOBJECT_TYPE type;
	powerUp PU;
	Vector3 pos;
	Vector3 vel;
	Vector3 scale;
	Vector3 normal;
	Vector3 color;
	float angle;
	bool active;
	float mass;
	Vector3 prevpos;
	int thickWall;
	int thinWall;
	//Rotation variables
	Vector3 direction;
	Vector3 linestart;
	Vector3 lineEnd;
	float momentOfInertia;
	float angularVelocity;
	bool bounce;
	float activeTime;
	float PUIFrame;
	GameObject* otherWall;
	bool visible;
	bool placed;
	void fetchReturnStatus();
	GameObject(GAMEOBJECT_TYPE typeValue = GO_BALL);
	~GameObject();
};

#endif