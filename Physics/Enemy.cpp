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
	switch (spawnLocation)
	{
	case NORTH:
		
	case EAST:

	case SOUTH:

	case WEST:

	}
}
