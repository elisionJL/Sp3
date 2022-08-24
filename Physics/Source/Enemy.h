#pragma once

#include "SceneBase.h"
#include "GameObject.h"
#include "Application.h"

class Enemy
{
public:
	//constructor
	Enemy();
	//destructor
	~Enemy();
	//init
	bool Init(void);
	//update
	void Update(const double dElapsedTime, Mesh* CurrentMesh);
	
	static void setSpawn(float x, float y,Vector3& pos);
	void movedistancefromotherenemies(Enemy* go1);

	enum STATES
	{
		MOVINGL = 0,
		MOVINGR,
		ATTACK,
		DEAD
	};

	enum TYPEOFENEMY
	{
		SLIME = 0,
		SPIDER = 1,
		VAMPIRE = 2,
		SKELETON = 3,
		GHOST = 4,
		NUM_TYPEOFENEMY
	};

	//public vars
	SceneBase::GEOMETRY_TYPE GEOTYPE;
	Vector3 pos;
	Vector3 vel;
	Vector3 scale;
	Vector3 previousCoord;
	bool usePrevX;
	bool usePrevY;
	int type;
	float hp;
	int amountoftimesitstayed;
	bool enemytoleft, enemytoright, enemyup, enemydown;
	float distFromPlayer;
	float expVal;
	int mass;
	double rangedcooldown;
	
	std::string address;
	void spawnBoss();
	void sethp(float Nhp);
	float gethp();
	void setaddress(std::string ad);
	int getState();
	void setState(int newState);
	Enemy::STATES getstate();
	//int getDirection();

protected:
	enum SPAWN
	{
		NORTH = 0,
		EAST = 1,
		SOUTH = 2,
		WEST = 3,
		NUM_LOCATIONS
	};

	enum DIRECTION
	{
		LEFT = 0,
		RIGHT = 1,
		UP = 2,
		DOWN = 3,
		NUM_DIRECTIONS
	};

	//DIRECTION sFacingDirection;

	STATES sCurrentState;
	Mesh* boss_slime;

};
