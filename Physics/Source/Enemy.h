#pragma once

#include "SceneBase.h"
#include "GameObject.h"
#include "Application.h"
#include "SpriteAnimation.h"
#include "MeshBuilder.h"
#include "Mesh.h"
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
	void Update(const double dElapsedTime);
	
	static void setSpawn(float x, float y,Vector3& pos);
	void movedistancefromotherenemies(Enemy* go1);

	enum STATES
	{
		MOVINGL = 0,
		MOVINGR,
		ATTACK,
		DEADL,
		DEADR
	};

	enum TYPEOFENEMY
	{
		SLIME,
		SPIDER,
		VAMPIRE,
		SKELETON,
		GHOST,
		ZOMBIE,
		NUM_TYPEOFENEMY
	};

	//public vars
	SpriteAnimation* enemy;
	Mesh* meshList[NUM_TYPEOFENEMY];

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
	int CurrEnemyType;
	float timerforboss;
	

	std::string address;
	void dieanimation();
	void spawnBoss();
	void sethp(float Nhp);
	float gethp();
	void setaddress(std::string ad);
	int getState();
	void setState(int newState);
	void setEnemyType(int EnemyType, Mesh* CurrentMesh);
	bool Deadornot();
	void AttackPhase();
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
