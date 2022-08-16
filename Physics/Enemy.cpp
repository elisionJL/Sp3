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
}

void Enemy::setSpawn()
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
