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

	//public vars
	Vector3 pos;
	Vector3 vel;
	Vector3 scale;
	int type;
	float hp;
	std::string address;

	void sethp(float Nhp);
	float gethp();
	void setaddress(std::string ad);

protected:
	enum DIRECTION
	{
		LEFT = 0,
		RIGHT = 1,
		UP = 2,
		DOWN = 3,
		NUM_DIRECTIONS
	};

	enum SPAWN
	{
		NORTH = 0,
		EAST = 1,
		SOUTH = 2,
		WEST = 3,
		NUM_LOCATIONS
	};

};

