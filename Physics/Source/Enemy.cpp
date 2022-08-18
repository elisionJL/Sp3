#include "Enemy.h"
#include "iostream"
Enemy::Enemy()
{
	vel = 0;
	hp = 1;
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
	//Update enemy movement to chase player
	
	//Change enemy states in response to the direction they are moving
	//int direction = 0;
	//switch (direction)
	//{
	//case LEFT:
	//	break;
	//case RIGHT:
	//	break;
	//case UP:
	//	break;
	//case DOWN:
	//	break;
	//}

}

void Enemy::setSpawn(float playerX, float playerY, Vector3& pos)
{
	int spawnLocation = Math::RandIntMinMax(0, 3);
	float w, h;
	w = 100.f;;
	h = w * (float)Application::GetWindowWidth() / Application::GetWindowHeight();
	switch (spawnLocation)
	{
	case NORTH:
		pos.x = playerX + Math::RandFloatMinMax(-(w / 2), (w / 2));
		pos.y = playerY + (h / 2);
		std::cout << "USING NORTH" << std::endl;
		break;
	case EAST:
		pos.x = playerX + (w + (w / 2));
		pos.y = playerY + Math::RandFloatMinMax(-(h / 2), (h / 2));
		std::cout << "USING EAST" << std::endl;
		break;
	case SOUTH:
		pos.x = playerX + Math::RandFloatMinMax(-(w / 2), (w / 2));
		pos.y = playerY - (h / 2);
		std::cout << "USING SOUTH" << std::endl;
		break;
	case WEST:
		pos.x = playerX - (w + (w / 2));
		pos.y = playerY + Math::RandFloatMinMax(-(h / 2), (h / 2));
		std::cout << "USING WEST" << std::endl;
		break;
	}

	std::cout << "pos.x : " << pos.x << " pos.y: " << pos.y << std::endl;
}
