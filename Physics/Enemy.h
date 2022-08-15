#pragma once

#include "Source/SceneBase.h"
#include "Source/GameObject.h"

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
protected:

};

