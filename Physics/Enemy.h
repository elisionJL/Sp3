#pragma once

#include "Source/SceneBase.h"
#include "Source/GameObject.h"
#include "Source/Application.h"

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
	
	void setSpawn();

	//public vars
	float x, y;
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

