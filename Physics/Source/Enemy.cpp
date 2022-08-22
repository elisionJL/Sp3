#include "Enemy.h"
#include "SpriteAnimation.h"
#include "iostream"
Enemy::Enemy()
{
	vel = 0;
	hp = 10;
	expVal= 1;
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
	//SpriteAnimation* bs = dynamic_cast<SpriteAnimation*>(boss_slime);
	//if (hp > 0)
	//{
	//	if (vel.x < 0)
	//	{
	//		bs->PlayAnimation("Move Left", -1, 2.f);
	//	}
	//	else if (vel.x > 0)
	//	{
	//		bs->PlayAnimation("Move Right", -1, 2.f);
	//	}
	//	else
	//	{
	//		bs->PlayAnimation("Idle", -1, 2.f);
	//	}
	//	if (vel.y < 0)
	//	{
	//		bs->PlayAnimation("Idle", -1, 2.f);
	//	}
	//	else
	//	{
	//		bs->PlayAnimation("Idle", -1, 2.f);
	//	}
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

void Enemy::movedistancefromotherenemies(Enemy* go1)
{
	vel = Vector3(0, 0, 0);
	if (!enemytoleft)
	{
		vel.x = -scale.x;
	}
	else if (!enemytoright)
	{
		vel.x = scale.x;
	}
	else if (!enemyup)
	{
		vel.x = scale.y;
	}
	else if (!enemydown)
	{
		vel.x = scale.y;
	}
	else
	{
		return;
		cout << "enemies everywhere" << endl;
	}
}

void Enemy::sethp(float Nhp)
{
	hp = Nhp;
}

float Enemy::gethp()
{
	return hp;
}

void Enemy::setaddress(std::string ad)
{
}

//int Enemy::getDirection()
//{
//	return sFacingDirection;
//}
