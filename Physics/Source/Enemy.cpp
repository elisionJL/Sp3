#include "Enemy.h"

Enemy::Enemy()
{
	x = 0;
	y = 0;
}

Enemy::~Enemy()
{
}

bool Enemy::Init(void)
{

	return false;
}

void Enemy::Update(const double dElapsedTime)
{
	//Change enemy states in response to the direction they are moving
	
	//Update enemy movement to chase player

}

void Enemy::setSpawn(float x, float y,Vector3& pos)
{
	int spawnLocation = Math::RandIntMinMax(0, 3);
	float w, h;
	w = Application::GetWindowWidth();
	h = Application::GetWindowHeight();
	switch (spawnLocation)
	{
	case NORTH:
		x = Math::RandFloatMinMax(0, w);
		y = h + (h / 4);
	case EAST:
		x = w + (w / 4);
		y = Math::RandFloatMinMax(0, h);
	case SOUTH:
		x = Math::RandFloatMinMax(0, w);
		y = 0 - (h / 4);
	case WEST:
		x = 0 - (w / 4);
		y = Math::RandFloatMinMax(0, h);
	}
}
