#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <vector>
#include "Vector3.h"

struct GameObject
{
	enum powerUp {
		ballUp = 0,
		rechargeUp,
		heal,
		extend
	};
	enum projectile {
		pistol = 0,
		GL,
		bow,
		sniper,
		shotgun,
		dragon
	};
	enum GAMEOBJECT_TYPE
	{
		GO_NONE = 0,
		GO_BALL,
		GO_CUBE,
		GO_WALL,
		GO_PILLAR,
		GO_POWERUP,
		GO_COMPANION,
		GO_PROJECTILE,
		GO_SHOTGUN,
		GO_SNIPER,
		GO_BOW,
		GO_PISTOL,
		GO_GL,
		GO_GRONK,
		GO_BOSS_SLIME,
		GO_SPIDER,
		GO_VAMPIRE,
		GO_SKELETON,
		GO_GHOST,
		GO_TREE,
		GO_ROCK,
		GO_SUPERPAIN,
		GO_EXPLOSION,
		GO_CHEST,
		GO_VAMPIRE,
		GO_SKELETON,
		GO_GHOST,
		GO_SPIDER,
		GO_TOTAL, //must be last
	};

	GAMEOBJECT_TYPE type; //here Zhi Kai
	powerUp PU;
	projectile proj;
	Vector3 pos;
	Vector3 vel;
	Vector3 scale;
	Vector3 normal;
	Vector3 color;

	std::vector<std::string> pier;
	int amountofpierleft;

	float angle;
	bool active;
	float mass;
	Vector3 prevpos;
	float thickWall;
	int thinWall;
	int lifetime;
	float damage;
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
	int bowdrawamount = 1;
	float prevangle;
	float angle1secago;
	void fetchReturnStatus();
	GameObject(GAMEOBJECT_TYPE typeValue = GO_BALL);
	~GameObject();
};

#endif