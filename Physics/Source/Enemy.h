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
	void Update(const double dElapsedTime);
	
	static void setSpawn(float x, float y,Vector3& pos);

	enum STATES
	{
		MOVING = 0,
		ATTACK,
		IDLE,
		DEAD
	};

	//public vars
	Vector3 pos;
	Vector3 vel;
	Vector3 scale;
	Vector3 previousCoord;
	bool usePrevX;
	bool usePrevY;
	int type;
	float hp;
	float distFromPlayer;
	float expVal;
	std::string address;
	void sethp(float Nhp);
	float gethp();
	void setaddress(std::string ad);

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

	STATES sCurrentState;
	DIRECTION sFacingDirection;

};

