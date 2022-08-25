#include "GL\glew.h"

#include "Enemy.h"
#include "SpriteAnimation.h"
#include "iostream"
#include "LoadTexture.h"

Enemy::Enemy()
{
	vel = 0;
	hp = 10;
	expVal= 1;
}

Enemy::~Enemy()
{
}

bool Enemy::Init()
{
	return false;
}

void Enemy::Update(const double dElapsedTime)
{
	//Update enemy movement to chase player

	//Change enemy states in response to the direction they are moving

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

void Enemy::setEnemyType(int EnemyType, Mesh* CurrentMesh)
{
	if (EnemyType == 0)
	{
		CurrEnemyType = 3;
		//Skeleton
		meshList[SKELETON] = MeshBuilder::GenerateSpriteAnimation("Skeleton", 4, 7);
		meshList[SKELETON]->textureID = LoadTexture("Image//SkeletonSS.png", true);
		meshList[SKELETON]->material.kAmbient.Set(1, 1, 1);

		enemy = dynamic_cast<SpriteAnimation*>(meshList[SKELETON]);
		//Add the animations
		enemy->AddAnimation("Attack", 7, 13);
		enemy->AddAnimation("MoveRight", 0, 6);
		enemy->AddAnimation("MoveLeft", 21, 27);
		enemy->AddAnimation("Die", 14, 20);
		GEOTYPE = SceneBase::GEO_SKELETON;
	}
	else if (EnemyType == 1)
	{
		CurrEnemyType = 4;
		meshList[GHOST] = MeshBuilder::GenerateSpriteAnimation("Ghost", 5, 7);
		meshList[GHOST]->textureID = LoadTexture("Image//GhostSS.png", true);
		meshList[GHOST]->material.kAmbient.Set(1, 1, 1);

		enemy = dynamic_cast<SpriteAnimation*>(meshList[GHOST]);

		enemy->AddAnimation("Attack", 7, 13);
		enemy->AddAnimation("MoveRight", 0, 6);
		enemy->AddAnimation("MoveLeft", 28, 34);
		enemy->AddAnimation("Die", 21, 26);
		enemy->AddAnimation("Hurt", 14, 20);
		GEOTYPE = SceneBase::GEO_GHOST;
	}
	else if (EnemyType == 2)
	{
		CurrEnemyType = 5;
		//Zombie
		meshList[ZOMBIE] = MeshBuilder::GenerateSpriteAnimation("Zombie", 6, 9);
		meshList[ZOMBIE]->textureID = LoadTexture("Image//ZombieSS.png", true);
		meshList[ZOMBIE]->material.kAmbient.Set(1, 1, 1);
		enemy = dynamic_cast<SpriteAnimation*>(meshList[ZOMBIE]);
		//Add the animations
		enemy->AddAnimation("Attack", 5, 11);
		enemy->AddAnimation("MoveRight", 12, 20);
		enemy->AddAnimation("MoveLeft", 39, 47);
		enemy->AddAnimation("Die", 21, 27);
		GEOTYPE = SceneBase::GEO_ZOMBIE;
	}
	else if (EnemyType == 3)
	{
		CurrEnemyType = 0; //slime
		meshList[SLIME] = MeshBuilder::GenerateSpriteAnimation("Boss Slime", 5, 6);
		meshList[SLIME]->textureID = LoadTexture("Image//boss_slime_sprites.png", true);
		meshList[SLIME]->material.kAmbient.Set(1, 1, 1);
		enemy = dynamic_cast<SpriteAnimation*>(meshList[SLIME]);
		//Add the animations
		enemy->AddAnimation("Die", 0, 3);
		enemy->AddAnimation("Attack", 4, 15);
		enemy->AddAnimation("MoveRight", 16, 22);
		enemy->AddAnimation("MoveLeft", 23, 30);
		GEOTYPE = SceneBase::GEO_BOSS_SLIME;
	}
	else if (EnemyType == 4)
	{
		CurrEnemyType = 1; //spider

		//Spider
		meshList[SPIDER] = MeshBuilder::GenerateSpriteAnimation("Spider", 4, 8);
		meshList[SPIDER]->textureID = LoadTexture("Image//spiderSS.png", true);
		meshList[SPIDER]->material.kAmbient.Set(1, 1, 1);
		enemy = dynamic_cast<SpriteAnimation*>(meshList[SPIDER]);
		//Add the animations
		enemy->AddAnimation("Attack", 8, 13);
		enemy->AddAnimation("MoveRight", 0, 7);
		enemy->AddAnimation("MoveLeft", 25, 31);
		enemy->AddAnimation("Die", 17, 22);
		GEOTYPE = SceneBase::GEO_SPIDER;
	}
	else if (EnemyType == 5)
	{
		CurrEnemyType = 2; //vampire

		//Vampire
		meshList[VAMPIRE] = MeshBuilder::GenerateSpriteAnimation("Vampire", 5, 7);
		meshList[VAMPIRE]->textureID = LoadTexture("Image//vampireSS.png", true);
		meshList[VAMPIRE]->material.kAmbient.Set(1, 1, 1);
		enemy = dynamic_cast<SpriteAnimation*>(meshList[VAMPIRE]);
		//Add the animations
		enemy->AddAnimation("Attack", 7, 12);
		enemy->AddAnimation("MoveRight", 0, 5);
		enemy->AddAnimation("MoveLeft", 28, 33);
		enemy->AddAnimation("Die", 21, 27);
		enemy->AddAnimation("Hurt", 14, 19);
		GEOTYPE = SceneBase::GEO_VAMPIRE;
	}
}

bool Enemy::Deadornot()
{
	return enemy->getAnimationStatus("Die");
}

//int Enemy::getDirection()
//{
//	return sFacingDirection;
//}
