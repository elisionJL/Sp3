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

void Enemy::Update(const double dElapsedTime, Mesh* CurrentMesh)
{
	//Update enemy movement to chase player

	//Change enemy states in response to the direction they are moving
	SpriteAnimation* enemy = dynamic_cast<SpriteAnimation*>(CurrentMesh); //here Zhi Kai
	//enemy->PlayAnimation("MoveRight", -1, 2.0f);

	if (sCurrentState != DEAD)
	{
		if (vel.x < 0)
		{
			enemy->PlayAnimation("MoveLeft", -1, 2.f);
		}
		else if (vel.x > 0)
		{
			enemy->PlayAnimation("MoveRight", -1, 2.f);
		}
	}
	else
	{
		enemy->PlayAnimation("Die", 0, 2.f);
	}
	enemy->Update(dElapsedTime);



	//if (sCurrentState == MOVING)
	//{
	//	if (vel.x < 0)
	//	{
	//		enemy->PlayAnimation("MoveLeft", -1, 2.f);
	//	}
	//	else if (vel.x > 0)
	//	{
	//		enemy->PlayAnimation("MoveRight", -1, 2.f);
	//	}
	//}
	//else if (sCurrentState == ATTACK)
	//{
	//	enemy->PlayAnimation("Attack", -1, 2.f);
	//}
	//else if (sCurrentState == DEAD)
	//{
	//	enemy->PlayAnimation("Die", -1, 2.f);
	//}
	

}

void Enemy::setSpawn(float playerX, float playerY, Vector3& pos)
{
	int spawnLocation = Math::RandIntMinMax(0, 3);
	
	float w, h;
	w = 100.f;
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

void Enemy::spawnBoss()
{
	int spawnLocation = Math::RandIntMinMax(0, 3);
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

int Enemy::getState()
{
	return sCurrentState;
}


void Enemy::setState(int newState)
{
	switch (newState)
	{
	case 0:
		sCurrentState = MOVINGL;
		break;
	case 1:
		sCurrentState = MOVINGR;
		break;
	case 2:
		sCurrentState = ATTACK;
		break;
	case 3:
		sCurrentState = DEAD;
		break;
	}
}

Enemy::STATES Enemy::getstate()
{
	return sCurrentState;
}


//int Enemy::getDirection()
//{
//	return sFacingDirection;
//}
