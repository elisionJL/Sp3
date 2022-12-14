#include "SceneCollision.h"
#include "GL\glew.h"
#include "Application.h"
#include "LoadTexture.h"
#include <sstream>
#include "SpriteAnimation.h"
#include "LoadTexture.h"
//Include GLFW
#include <GLFW/glfw3.h>


const float SceneCollision::GRAVITY_CONSTANT = 20.0f;
using namespace std;

SceneCollision::SceneCollision()
{
}

SceneCollision::~SceneCollision()
{

}

void SceneCollision::Init()
{
	SceneBase::Init();
	srand(time(0));
	cPlayer2D = CPlayer2D::GetInstance();
	cPlayer2D->Init();
	
	cPlayer2D->setmeshList(meshList[GEO_PLAYER]);
	cKeyboardController = CKeyboardController::GetInstance();
	cMouseController = CMouseController::GetInstance();
	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();
	currentState = start;
	//Physics code here
	m_speed = 1.f;
	score = 0;
	acquiredGold = 0;
	thickWall = 0;
	BossKilled = 0;
	surviveSeconds = 60;
	Math::InitRNG();
	m_objectCount = 0;
	cSoundController->PlaySoundByID(1);
	dialogueTime = 0;
	//Companion->mass = 1;
	Companion = FetchGO();
	Gun = FetchGO();
	Companion->mass = 1;
	flip = 1;
	OutputDialogue = "";
	CurrentTextWrite = false, TextFinished = false;
	CurrentCharText = 0;
	randomDialogue = 0;

	companionX = 9;
	companionY = 9;

	GunShootingTimer = 0;

	rotationorder = 1;

	shootonceonly = 1;

	GunShoot = false;

	needtofinishanimation = false;

	PlayerBuy = false;

	currentlyHovering = false;
	shopClick = 0;
	zaxis = 1;
	pause = false;
	for (int i = 0; i < 6; ++i)
	{
		ShopUpgrades[i] = 0;
		MaxUpgrade[i] = false;
	}
	string line;
	int GetStat = 0;
	ifstream CurrentStats("Player_Stats.txt");
	if (CurrentStats.is_open())
	{
		while (getline(CurrentStats, line))
		{
			if (GetStat < 6)
				ShopUpgrades[GetStat] = stoi(line);
			else
				cPlayer2D->UseGold(-stoi(line));
			GetStat += 1;
		}
		CurrentStats.close();
	}


	if (ShopUpgrades[0] > 0)
		for (int Upgrade = 0; Upgrade < ShopUpgrades[0]; ++Upgrade)
			cPlayer2D->IncreaseSpd();
	if (ShopUpgrades[1] > 0)
		for (int Upgrade = 0; Upgrade < ShopUpgrades[1]; ++Upgrade)
			cPlayer2D->IncreaseHP();
	if (ShopUpgrades[2] > 0)
		for (int Upgrade = 0; Upgrade < ShopUpgrades[2]; ++Upgrade)
			cPlayer2D->DecreaseShieldCooldown();
	if (ShopUpgrades[3] > 0)
		for (int Upgrade = 0; Upgrade < ShopUpgrades[3]; ++Upgrade)
			cPlayer2D->IncreaseDmg();
	if (ShopUpgrades[4] > 0)
		for (int Upgrade = 0; Upgrade < ShopUpgrades[4]; ++Upgrade)
			cPlayer2D->IncreaseLifeCount();
	if (ShopUpgrades[5] > 0)
		for (int Upgrade = 0; Upgrade < ShopUpgrades[5]; ++Upgrade)
			cPlayer2D->IncreaseEXPGain();

	if (ShopUpgrades[0] >= 5)
		MaxUpgrade[0] = true;
	if (ShopUpgrades[1] >= 10)
		MaxUpgrade[1] = true;
	if (ShopUpgrades[2] >= 5)
		MaxUpgrade[2] = true;
	if (ShopUpgrades[3] >= 4)
		MaxUpgrade[3] = true;
	if (ShopUpgrades[4] >= 1)
		MaxUpgrade[4] = true;
	if (ShopUpgrades[5] >= 5)
		MaxUpgrade[5] = true;
	dmgofgun = 0;

	velocityofbullet = 20;

	bowframe = 0;

	Transition = false;

	timerforpistol = 0;
	timerfordragon = 0;
	GunRightClickSpecial = false;
	staggertimingforpistol = 0;

	Shield = FetchGO();
	Shield->type = GameObject::GO_SHIELD;
	Shield->scale = Vector3(15, 15, 1);
	Shield->pos = Vector3(0, 0, 0);

	enemyspawn = 0;
	enemyspawnspeed = 0.5;
	enemyovertime = 0;

	screenShake[0] = 0;
	screenShake[1] = 0;
	SuperPainPower = false;
	PowerUsed = 0;

	SongVolumeChange = 0;
}

GameObject* SceneCollision::FetchGO()
{
	for (auto i = m_goList.begin(); i != m_goList.end(); i++)
	{
		GameObject* go = (GameObject*)*i;
		if (go->active == true)
		{
			continue;
		}
		go->fetchReturnStatus();
		return go;
	}
	int prevSize = m_goList.size();
	for (int i = 0; i < 10; ++i) {
		m_goList.push_back(new GameObject(GameObject::GO_BALL));
	}
	m_goList.at(prevSize)->fetchReturnStatus();
	return m_goList.at(prevSize);
}

void SceneCollision::ReturnGO(GameObject* go)
{
	if (go->active == true)
	{
		go->active = false;
		m_objectCount--;
	}
}

void SceneCollision::shooting(double elapsedTime, int numberofshots, GameObject* Gun)
{
	double x = 0, y = 0;
	if (numberofshots > 0)
	{
		Application::GetCursorPos(&x, &y);
		unsigned w = Application::GetWindowWidth();
		unsigned h = Application::GetWindowHeight();
		float posX = (x / w * m_worldWidth) + camera.position.x;
		float posY = m_worldHeight - (y / h * m_worldHeight) + camera.position.y;
		Vector3 center = Vector3(posX, posY, 0) - cPlayer2D->pos;
		float angle = calculateAngle(center.x, center.y);
		float magnitude = center.Length();


		float startingamount = (numberofshots - 1) * 5;
		if (startingamount < 1)
		{
			startingamount = 1;
		}

		for (int i = -startingamount; i <= startingamount; i += 10)
		{
			GameObject* go = FetchGO();
			go->pos = cPlayer2D->pos;
			go->scale.Set(4, 2, 1);
			go->type = GameObject::GO_PROJECTILE;
			go->angle = angle + i;
			switch (Gun->type) {
			case GameObject::GO_PISTOL:
				go->proj = GameObject::pistol;
				cSoundController->StopPlayByID(13);
				cSoundController->PlaySoundByID(13);
				break;
			case GameObject::GO_BOW:
				go->proj = GameObject::bow;
				cSoundController->StopPlayByID(10);
				cSoundController->PlaySoundByID(10);
				break;
			case GameObject::GO_GL:
				go->proj = GameObject::GL;
				cSoundController->StopPlayByID(8);
				cSoundController->PlaySoundByID(8);
				break;
			case GameObject::GO_SHOTGUN:
				go->proj = GameObject::shotgun;
				cSoundController->StopPlayByID(11);
				cSoundController->PlaySoundByID(11);
				break;
			case GameObject::GO_SNIPER:
				go->proj = GameObject::sniper;
				cSoundController->StopPlayByID(12);
				cSoundController->PlaySoundByID(12);
				break;
			case GameObject::GO_MACHINEGUN:
				go->proj = GameObject::machinegun;
				cSoundController->StopPlayByID(17);
				cSoundController->PlaySoundByID(17);
				break;
			}
			if (go->angle > 360) {
				go->angle -= 360;
			}
			go->vel.x = cos(Math::DegreeToRadian(go->angle)) * magnitude;
			go->vel.y = sin(Math::DegreeToRadian(go->angle)) * magnitude;

			if (Gun->type == GameObject::GO_BOW)
			{
				go->vel.Normalize() *= velocityofbullet * (0.5 * bowframe);
				go->bowdrawamount = bowframe;
				go->amountofpierleft = pierceforbullet + bowframe;
			}
			else if (Gun->type == GameObject::GO_SNIPER)
			{
				go->vel.Normalize() *= velocityofbullet + (Gun->thickWall / 10);
				go->amountofpierleft = pierceforbullet + (Gun->thickWall / 50);
				go->damage = Gun->thickWall / 50;
			}
			else
			{
				go->vel.Normalize() *= velocityofbullet;
				go->amountofpierleft = pierceforbullet;
			}
			go->pier.clear();

			for (int arraynumber = 0; arraynumber < timerforbullets.size(); ++arraynumber)
			{
				if (timerforbullets[arraynumber] != 0)
				{
					continue;
				}
				timerforbullets[arraynumber] = elapsedTime + 2.0f;
				go->lifetime = arraynumber;
				break;
			}
			timerforbullets.push_back(elapsedTime + 2.0f);
			go->lifetime = timerforbullets.size() - 1;

		}
		if (Gun->reverseShoot)
		{
			GameObject* go = FetchGO();
			go->pos = cPlayer2D->pos;
			go->scale.Set(4, 2, 1);
			go->type = GameObject::GO_PROJECTILE;
			go->angle = angle + 180;
			switch (Gun->type) {
			case GameObject::GO_PISTOL:
				go->proj = GameObject::pistol;
				cSoundController->StopPlayByID(13);
				cSoundController->PlaySoundByID(13);
				break;
			case GameObject::GO_BOW:
				go->proj = GameObject::bow;
				cSoundController->StopPlayByID(10);
				cSoundController->PlaySoundByID(10);
				break;
			case GameObject::GO_GL:
				go->proj = GameObject::GL;
				cSoundController->StopPlayByID(8);
				cSoundController->PlaySoundByID(8);
				break;
			case GameObject::GO_SHOTGUN:
				go->proj = GameObject::shotgun;
				cSoundController->StopPlayByID(11);
				cSoundController->PlaySoundByID(11);
				break;
			case GameObject::GO_SNIPER:
				go->proj = GameObject::sniper;
				cSoundController->StopPlayByID(12);
				cSoundController->PlaySoundByID(12);
				break;
			case GameObject::GO_MACHINEGUN:
				go->proj = GameObject::machinegun;
				cSoundController->StopPlayByID(17);
				cSoundController->PlaySoundByID(17);
				break;
			}
			if (go->angle > 360) {
				go->angle -= 360;
			}
			go->vel.x = cos(Math::DegreeToRadian(go->angle)) * magnitude;
			go->vel.y = sin(Math::DegreeToRadian(go->angle)) * magnitude;

			if (Gun->type == GameObject::GO_BOW)
			{
				go->vel.Normalize() *= velocityofbullet * (0.5 * bowframe);
				go->bowdrawamount = bowframe;
				go->amountofpierleft = pierceforbullet + bowframe;
			}
			else if (Gun->type == GameObject::GO_SNIPER)
			{
				go->vel.Normalize() *= velocityofbullet + (Gun->thickWall / 10);
				go->amountofpierleft = pierceforbullet + (Gun->thickWall / 50);
				go->damage = Gun->thickWall / 50;
			}
			else
			{
				go->vel.Normalize() *= velocityofbullet;
				go->amountofpierleft = pierceforbullet;
			}
			go->pier.clear();

			for (int arraynumber = 0; arraynumber < timerforbullets.size(); ++arraynumber)
			{
				if (timerforbullets[arraynumber] != 0)
				{
					continue;
				}
				timerforbullets[arraynumber] = elapsedTime + 2.0f;
				go->lifetime = arraynumber;
				break;
			}
			timerforbullets.push_back(elapsedTime + 2.0f);
			go->lifetime = timerforbullets.size() - 1;
		}
	}
}

void SceneCollision::PistolShooting(double elapsedTime, int numofshots)
{
	float angle = Math::RandFloatMinMax(0, 360);

	cSoundController->PlayOverlappingSoundByID(13);

	float startingamount = (numofshots - 1) * 5;
	if (startingamount < 1)
	{
		startingamount = 1;
	}

	for (int i = -startingamount; i <= startingamount; i += 10)
	{
		GameObject* go = FetchGO();
		go->pos = cPlayer2D->pos;
		go->scale.Set(4, 2, 1);
		go->type = GameObject::GO_PROJECTILE;
		go->angle = angle + i;
		go->proj = GameObject::pistol;
		if (go->angle > 360) {
			go->angle -= 360;
		}
		go->vel.x = cos(Math::DegreeToRadian(go->angle));
		go->vel.y = sin(Math::DegreeToRadian(go->angle));
		Gun->angle = angle;

		go->vel.Normalize() *= velocityofbullet;
		go->amountofpierleft = pierceforbullet;
		go->pier.clear();

		for (int arraynumber = 0; arraynumber < timerforbullets.size(); ++arraynumber)
		{
			if (timerforbullets[arraynumber] != 0)
			{
				continue;
			}
			timerforbullets[arraynumber] = elapsedTime + 2.0f;
			go->lifetime = arraynumber;
			break;
		}
		timerforbullets.push_back(elapsedTime + 2.0f);
		go->lifetime = timerforbullets.size() - 1;
	}
	if (Gun->reverseShoot)
	{
		GameObject* go = FetchGO();
		go->pos = cPlayer2D->pos;
		go->scale.Set(4, 2, 1);
		go->type = GameObject::GO_PROJECTILE;
		go->angle = angle + 180;
		go->proj = GameObject::pistol;
		if (go->angle > 360) {
			go->angle -= 360;
		}
		go->vel.x = cos(Math::DegreeToRadian(go->angle));
		go->vel.y = sin(Math::DegreeToRadian(go->angle));
		Gun->angle = angle;

		go->vel.Normalize() *= velocityofbullet;
		go->amountofpierleft = pierceforbullet;
		go->pier.clear();

		for (int arraynumber = 0; arraynumber < timerforbullets.size(); ++arraynumber)
		{
			if (timerforbullets[arraynumber] != 0)
			{
				continue;
			}
			timerforbullets[arraynumber] = elapsedTime + 2.0f;
			go->lifetime = arraynumber;
			break;
		}
		timerforbullets.push_back(elapsedTime + 2.0f);
		go->lifetime = timerforbullets.size() - 1;
	}
}

bool SceneCollision::bulletcollisioncheck(GameObject* Gun, GameObject* Bullet, Enemy* go2)
{
	for (int i = 0; i < Bullet->pier.size(); ++i)
	{
		if (Bullet->pier[i] == go2->address)
		{
			return false;
		}
	}

	{
		Vector3 relativeVel = Bullet->vel - go2->vel;

		Vector3 disDiff = go2->pos - Bullet->pos;

		if (Bullet->pos.y > go2->pos.y)
		{
			disDiff -= Vector3(0, go2->scale.y / 2, 0);
		}
		else
		{
			disDiff += Vector3(0, go2->scale.y / 2, 0);
		}

		if (Bullet->pos.x > go2->pos.x)
		{
			disDiff -= Vector3(go2->scale.x / 2, 0, 0);
		}
		else
		{
			disDiff += Vector3(go2->scale.x / 2, 0, 0);
		}


		if (relativeVel.Dot(disDiff) <= 0) {
			return false;
		}
		return disDiff.LengthSquared() <= (Bullet->scale.x + go2->scale.x) * (Bullet->scale.x + go2->scale.x);
	}
}

void SceneCollision::dobulletcollision(GameObject* Gun, GameObject* Bullet, Enemy* go2)
{
	u1 = Bullet->vel;
	u2 = go2->vel;
	m1 = go2->mass;
	m2 = go2->mass;

	/*switch (go2->type)
	{
	case GameObject::GO_BOSS_SLIME:*/

	if (go2->getState() == 3 || go2->getState() == 4)
		return;

	{
		float dmg = dmgofgun * Bullet->bowdrawamount + cPlayer2D->GetDmg();

		if (Bullet->type == GameObject::GO_EXPLOSION)
			dmg = dmgofgun * 2.f;
		else if (Bullet->proj == GameObject::dragon)
			dmg = Bullet->damage;
		else if (Bullet->proj == GameObject::sniper)
			dmg = dmgofgun + Bullet->damage + cPlayer2D->GetDmg();
		else if (Bullet->proj == GameObject::machinegun)
		{
			dmg = dmgofgun + (meshList[GEO_MACHINEGUN]->material.kAmbient.b / (meshList[GEO_MACHINEGUN]->material.kAmbient.b * meshList[GEO_MACHINEGUN]->material.kAmbient.b));
		}

		float critornot = Math::RandFloatMinMax(0, 100);
		bool yesorno = false;
		if (Gun->critchance >= critornot)
		{
			dmg *= Gun->critdamage;
			yesorno = true;
		}

		go2->sethp(go2->gethp() - dmg);
		DamageNumbers(dmg, go2, yesorno);


		Bullet->amountofpierleft -= 1;

		if (go2->gethp() <= 0)
		{
			go2->dieanimation();
		}


		if (Bullet->amountofpierleft <= 0 && Bullet->type != GameObject::GO_EXPLOSION)
		{
			if (Bullet->proj == GameObject::GL)
			{
				GameObject* Explosion = FetchGO();
				Explosion->type = GameObject::GO_EXPLOSION;
				Explosion->pos = Bullet->pos;
				Explosion->scale.Set(8, 8, 1);
				for (int arraynumber = 0; arraynumber < timerforbullets.size(); ++arraynumber)
				{
					if (timerforbullets[arraynumber] != 0)
					{
						continue;
					}
					timerforbullets[arraynumber] = elapsedTime + 0.5f;
					Explosion->lifetime = arraynumber;
					break;
				}
				timerforbullets.push_back(elapsedTime + 0.5f);
				Explosion->lifetime = timerforbullets.size() - 1;
			}
			Bullet->pier.clear();
			ReturnGO(Bullet);
		}
		else
		{
			const void* address = static_cast<const void*>(go2);
			std::stringstream ss;
			ss << address;
			Bullet->pier.push_back(ss.str());
			if (Bullet->proj == GameObject::shotgun)
			{
				Vector3 n = Bullet->pos - go2->pos;
				Vector3 vec = (u1 - u2).Dot(n) / (n).LengthSquared() * n;
				Bullet->vel = u1 - (2 * m2 / (m1 + m2)) * vec;
				go2->vel = u2 - (2 * m1 / (m2 + m1)) * -vec;
				Bullet->angle = calculateAngle(Bullet->vel.x, Bullet->vel.y);
				//break;
			}
		}
		//break;
	}
	/*	default:
		break;
	}*/
}

void SceneCollision::DeleteEnemy(Enemy* enemy)
{
	for (int i = 0; i < enemyList.size(); ++i)
	{
		if (enemyList[i] == enemy)
		{
			if(SuperPainPower == false)
				cPlayer2D->xp += enemy->expVal * cPlayer2D->getExpBooster();
			if (enemy->GEOTYPE == GEO_ZOMBIE)
				acquiredGold += 10;
			else
				acquiredGold += 2;

			enemyList.erase(enemyList.begin() + i);
			score += 10;
			if (enemy->GEOTYPE == GEO_BOSS_SLIME || enemy->GEOTYPE == GEO_SPIDER || enemy->GEOTYPE == GEO_VAMPIRE)
			{
				acquiredGold += 50;
				GameObject* go = FetchGO();
				go->pos = enemy->pos;
				go->scale.Set(4, 4, 1);
				go->type = GameObject::GO_CHEST;
				go->vel.SetZero();
				go->placed = true;
				bossspawned = false;
				++BossKilled;
				if (BossKilled < 3)
				{
					ArrowToBoss->visible = false;
					ArrowInsideArrow->visible = false;
				}
				else
					Boss->timerforboss = 60;
			}
			else
				killcounter++;

			if (killcounter >= 100 && !bossspawned)
			{
				Vector3 Epos;
				Enemy* go = new Enemy();

				int typeOfEnemy = Math::RandIntMinMax(0, 2);
				switch (typeOfEnemy)
				{
				case 0:
					go->setEnemyType(3, meshList[GEO_BOSS_SLIME]);
					break;
				case 1:
					go->setEnemyType(4, meshList[GEO_SPIDER]);
					break;
				case 2:
					go->setEnemyType(5, meshList[GEO_VAMPIRE]);
					break;
				}

				int locationOfEnemy = Math::RandIntMinMax(0, 3);
				switch (locationOfEnemy)
				{
				case 0:
					go->pos = Vector3(m_worldWidth * 2.5, m_worldHeight * 2.5, 1);
					break;
				case 1:
					go->pos = Vector3(m_worldWidth * 2.5, -m_worldHeight * 2.5, 1);
					break;
				case 2:
					go->pos = Vector3(-m_worldWidth * 2.5, m_worldHeight * 2.5, 1);
					break;
				case 3:
					go->pos = Vector3(-m_worldWidth * 2.5, -m_worldHeight * 2.5, 1);
					break;
				}
				bossspawned = true;
				killcounter -= 100;
				Boss = go;

				go->scale.Set(20, 20, 1);
				go->mass = 10;
				go->hp = 200 * pow(hpScaling, minutes);

				enemyList.push_back(go);

				const void* address = static_cast<const void*>(go);
				std::stringstream ss;
				ss << address;
				go->address = ss.str();
				Boss->timerforboss = 90;

				
				ArrowToBoss->pos = cPlayer2D->pos;
				ArrowToBoss->scale.Set(4, 4, 1);
				ArrowToBoss->type = GameObject::GO_WHEREBOSS;
				ArrowToBoss->vel.SetZero();
				ArrowToBoss->visible = true;
				
				ArrowInsideArrow->pos = cPlayer2D->pos;
				ArrowInsideArrow->scale.Set(4, 4, 1);
				ArrowInsideArrow->type = GameObject::GO_WHEREBOSS;
				ArrowInsideArrow->vel.SetZero();
				ArrowInsideArrow->visible = true;

				enemyspawn = elapsedTime;
			}
		}
	}
}

void SceneCollision::DamageNumbers(int damage, Enemy* Enem, bool critornot)
{
	for (int arraynumber = 0; arraynumber < dmgandtimefordmgnumber.size(); ++arraynumber)
	{
		if (dmgandtimefordmgnumber[arraynumber] != 0)
		{
			continue;
		}
		dmgandtimefordmgnumber[arraynumber] = Vector3(Enem->pos.x, Enem->pos.y, damage);
		timerfordmgnumber[arraynumber] = Vector3(elapsedTime + 0.5f, critornot, 0);
		break;
	}
	dmgandtimefordmgnumber.push_back(Vector3(Enem->pos.x, Enem->pos.y, damage));
	timerfordmgnumber.push_back(Vector3(elapsedTime + 0.5f, critornot, 0));
}

bool SceneCollision::Movingofdamagenumbers(float posX, int dmg)
{
	if (switchdmgnum % 2 == 0)
	{
		return false;
	}
	for (int i = 0; i < coordinatesofdamagenumbers.size(); ++i)
	{
		if (posX > coordinatesofdamagenumbers[i].x - 2 && posX < coordinatesofdamagenumbers[i].x + 2)
		{
			displaynumberoffset += 2;
			if (dmg > 9)
			{
				displaynumberoffsety += 3;
				displaynumberoffset++;
			}
			return true;
		}
	}
	return false;
}

void SceneCollision::dragonshooting(int numberofshots, float velofproj, int piercing)
{
	double x = 0, y = 0;
	if (numberofshots > 0)
	{
		Application::GetCursorPos(&x, &y);
		unsigned w = Application::GetWindowWidth();
		unsigned h = Application::GetWindowHeight();
		float posX = (x / w * m_worldWidth) + camera.position.x;
		float posY = m_worldHeight - (y / h * m_worldHeight) + camera.position.y;
		Vector3 center = Vector3(posX, posY, 0) - (cPlayer2D->pos + Vector3(companionX, companionY, 0));
		float angle = calculateAngle(center.x, center.y);
		float magnitude = center.Length();


		float startingamount = (numberofshots - 1) * 5;
		if (startingamount < 1)
		{
			startingamount = 1;
		}

		for (int i = -startingamount; i <= startingamount; i += 10)
		{
			GameObject* go = FetchGO();
			go->pos = (cPlayer2D->pos + Vector3(companionX, companionY, 0));
			go->scale.Set(4, 4, 1);
			go->type = GameObject::GO_PROJECTILE;
			go->angle = angle + i;
			go->proj = GameObject::dragon;
			if (go->angle > 360) {
				go->angle -= 360;
			}
			go->vel.x = cos(Math::DegreeToRadian(go->angle)) * magnitude;
			go->vel.y = sin(Math::DegreeToRadian(go->angle)) * magnitude;
			go->vel.Normalize() *= velofproj;
			go->amountofpierleft = piercing;
			go->damage = Companion->damage;
			go->pier.clear();

			for (int arraynumber = 0; arraynumber < timerforbullets.size(); ++arraynumber)
			{
				if (timerforbullets[arraynumber] != 0)
				{
					continue;
				}
				timerforbullets[arraynumber] = elapsedTime + 2.0f;
				go->lifetime = arraynumber;
				break;
			}
			timerforbullets.push_back(elapsedTime + 2.0f);
			go->lifetime = timerforbullets.size() - 1;
		}
	}
}

float SceneCollision::CalculateAdditionalForce(Enemy* Enemy, CPlayer2D* cPlayer2D)
{
	float radiusSquared = 50;
	return (GRAVITY_CONSTANT * 50 * Enemy->mass) / radiusSquared;
}

void SceneCollision::MoveEnemiesToPlayer(Enemy* enemy, CPlayer2D* cPlayer2D, double dt)
{

	//Vector3 relativeVel = cPlayer2D->vel - enemy->vel;
	//Vector3 disDiff = enemy->pos - cPlayer2D->pos;

	//if (relativeVel.Dot(disDiff) <= 0) {
	//	/*enemy->vel += (cPlayer2D->pos - enemy->pos).Normalized() * 20;*/
	//	return;
	//}

	/*if (disDiff.LengthSquared() <= (10 + enemy->scale.x) * (10 + enemy->scale.x))
	{

	}
	else*/
	/*{
		float sign = 1;
		Vector3 dir = sign * (cPlayer2D->pos - enemy->pos).Normalized();
		float force = CalculateAdditionalForce(enemy, cPlayer2D);
		enemy->vel += 1.f / enemy->mass * dir * force * dt;
	}*/
}

void SceneCollision::MachineGunPewPew(double elapsedTime, int numofshots)
{
	double x = 0, y = 0;
	if (numofshots > 0)
	{
		Application::GetCursorPos(&x, &y);
		unsigned w = Application::GetWindowWidth();
		unsigned h = Application::GetWindowHeight();
		float posX = (x / w * m_worldWidth) + camera.position.x;
		float posY = m_worldHeight - (y / h * m_worldHeight) + camera.position.y;
		Vector3 center = Vector3(posX, posY, 0) - cPlayer2D->pos;
		float angle = calculateAngle(center.x, center.y);
		float magnitude = center.Length();


		float startingamount = (numofshots - 1) * 5;
		if (startingamount < 1)
		{
			startingamount = 1;
		}

		for (int i = -startingamount; i <= startingamount; i += 10)
		{
			GameObject* go = FetchGO();
			go->pos = cPlayer2D->pos;
			go->scale.Set(4, 2, 1);
			go->type = GameObject::GO_PROJECTILE;
			go->angle = angle + i + Math::RandFloatMinMax(0, 30);
			go->proj = GameObject::machinegun;
			cSoundController->PlaySoundByID(18);

			if (go->angle > 360) {
				go->angle -= 360;
			}
			go->vel.x = cos(Math::DegreeToRadian(go->angle)) * magnitude;
			go->vel.y = sin(Math::DegreeToRadian(go->angle)) * magnitude;

			go->vel.Normalize() *= (velocityofbullet + 30);
			go->amountofpierleft = 1;
			
			go->pier.clear();

			for (int arraynumber = 0; arraynumber < timerforbullets.size(); ++arraynumber)
			{
				if (timerforbullets[arraynumber] != 0)
				{
					continue;
				}
				timerforbullets[arraynumber] = elapsedTime + 1.0f;
				go->lifetime = arraynumber;
				break;
			}
			timerforbullets.push_back(elapsedTime + 2.0f);
			go->lifetime = timerforbullets.size() - 1;
		}
	}
}

void SceneCollision::WritePlayerStats()
{
	ofstream savedStats("Player_Stats.txt");
	if (savedStats.is_open())
	{
		savedStats << char16_t(ShopUpgrades[0]) << endl << char16_t(ShopUpgrades[1]) << endl << char16_t(ShopUpgrades[2]) << endl
			<< char16_t(ShopUpgrades[3]) << endl << char16_t(ShopUpgrades[4]) << endl << char16_t(ShopUpgrades[5]) << endl
			<< char16_t(cPlayer2D->GetGold()) << endl;
		savedStats.flush();
		savedStats.close();
	}
}

void SceneCollision::chest(Vector3 mousePos,float dt)
{

	elapsedTime += dt;
	if (elapsedTime > timerBeforeUpgrade) {
		static bool LMPressed = false;
		if (Application::IsMousePressed(0) && !LMPressed) {
			LMPressed = true;
		}
		else if (!Application::IsMousePressed(0) && LMPressed) {
			LMPressed = false;
			chestOpened = false;
			for (int i = 1; i < 4; ++i) {
				float x = (i * 0.04 * m_worldWidth) + ((i - 1) * 0.28 * m_worldWidth) + (m_worldWidth * 0.14);
				float cameramoveX = cPlayer2D->pos.x - m_worldWidth * 0.5;

				if ((mousePos.x >= (i * 0.04 * m_worldWidth) + ((i - 1) * 0.28 * m_worldWidth) && mousePos.x <= (i * 0.04 * m_worldWidth) + ((i - 1) * 0.28 * m_worldWidth) + m_worldWidth * 0.28) &&
					(mousePos.y <= m_worldHeight * 0.73 && mousePos.y >= m_worldHeight * 0.17)) {

					cPlayer2D->increaseLevel();

					switch (traitsUpgrades[i - 1]) {
					case critRate:
						Gun->critchance += 10;
						break;
					case critDamage:
						Gun->critdamage += .25;
						break;
					case reverseShoot:
						Gun->reverseShoot = true;
						dmgofgun *= 1.5;
						break;
					case hpUpMSDOWN:
						cPlayer2D->maxWalk_Speed *= 0.8;
						cPlayer2D->Walk_Speed = cPlayer2D->maxWalk_Speed;
						cPlayer2D->maxHP *= 1.5;
						cPlayer2D->hp *= 1.5;
						break;
					case brokenShard:

						dmgofgun *= 2;
						//cPlayer2D->dmg *= 2;
						cPlayer2D->maxHP *= .5;
						cPlayer2D->hp *= .5;
						break;
					case rateUpMSDown:
						firerate *= 0.7;
						cPlayer2D->maxWalk_Speed *= 0.8;
						cPlayer2D->Walk_Speed = cPlayer2D->maxWalk_Speed;
						break;
					case regen:
						cPlayer2D->regenMulti *= 2;
						break;
					}
				}
			}
		}
	}
}


void SceneCollision::renderBossTraits(Vector3 mousePos)
{
	modelStack.PushMatrix();
	modelStack.Translate(camera.position.x, camera.position.y, zaxis += 0.001f);
	modelStack.Scale(1000, 1000, 1);
	RenderMesh(meshList[GEO_LVLUPBG], false);
	modelStack.PopMatrix();
	float cameramoveX = cPlayer2D->pos.x - m_worldWidth * 0.5;
	float cameramoveY = cPlayer2D->pos.y - m_worldHeight * 0.5;

	modelStack.PushMatrix();
	modelStack.Translate((m_worldWidth / 2) + cameramoveX, (m_worldHeight * 0.9) + cameramoveY, zaxis += 0.001f);
	modelStack.Scale(m_worldWidth * 0.6, m_worldHeight * 0.1, 1);
	RenderMesh(meshList[GEO_UPGRADESELECT], false);
	modelStack.PopMatrix();

	for (int i = 1; i < 4; ++i) {
		float x = (i * 0.04 * m_worldWidth) + ((i - 1) * 0.28 * m_worldWidth) + (m_worldWidth * 0.14);
		float textx;
		Vector3 color = Vector3(1, 0, 0);
		switch (i) {
		case 1:
			textx = 14;
			break;
		case 2:
			textx = 40;
			break;
		case 3:
			textx = 65;
			break;
		}
		modelStack.PushMatrix();
		modelStack.Translate(x + cameramoveX, m_worldHeight * 0.45 + cameramoveY, 6.01);
		modelStack.Scale(m_worldWidth * 0.28, m_worldHeight * 0.56, 1);
		if ((mousePos.x >= (i * 0.04 * m_worldWidth) + ((i - 1) * 0.28 * m_worldWidth) && mousePos.x <= (i * 0.04 * m_worldWidth) + ((i - 1) * 0.28 * m_worldWidth) + m_worldWidth * 0.28) &&
			(mousePos.y <= m_worldHeight * 0.73 && mousePos.y >= m_worldHeight * 0.17)) {
			meshList[GEO_CARD]->material.kAmbient.Set(1, 1, 0);
		}
		else {
			meshList[GEO_CARD]->material.kAmbient.Set(1, 1, 1);
		}
		RenderMesh(meshList[GEO_CARD], true);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(x + cameramoveX, m_worldHeight * 0.45 + cameramoveY, 6.02);
		modelStack.Scale(m_worldWidth * 0.28, m_worldHeight * 0.56, 1);

		switch (traitsUpgrades[i - 1]) {
		case critRate:
			RenderMesh(meshList[GEO_CRTRATE], false);
			break;
		case critDamage:
			RenderMesh(meshList[GEO_CRITDMG], false);
			break;
		case hpUpMSDOWN:
			RenderMesh(meshList[GEO_JUGG], false);
			break;
		case rateUpMSDown:
			RenderMesh(meshList[GEO_RATEUPMSDOWN], false);
			break;
		case reverseShoot:
			RenderMesh(meshList[GEO_REVERSESHOT], false);
			break;
		case brokenShard:
			RenderMesh(meshList[GEO_SHAPEDGLASS], false);
			break;
		case regen:
			RenderMesh(meshList[GEO_REGEN], false);
			break;
		}

		modelStack.PopMatrix();
	}
}

void SceneCollision::generateTraits()
{
	timerBeforeUpgrade = elapsedTime + 1;
	//generate 3 random traits for the player to choose
	for (int i = 0; i < 3; ++i) {
		if (i == 0) {
			switch (Math::RandIntMinMax(0, 6) ){
			case 0:
				traitsUpgrades[i] = critRate;
				break;
			case 1:
				traitsUpgrades[i] = critDamage;
				break;
			case 2:
				traitsUpgrades[i] = reverseShoot;
				break;
			case 3:
				traitsUpgrades[i] = hpUpMSDOWN;
				break;
			case 4:
				traitsUpgrades[i] = brokenShard;
				break;
			case 5:
				traitsUpgrades[i] = rateUpMSDown;
				if (MSUpgrade != 5) {
					break;
				}
			case 6:
				traitsUpgrades[i] = regen;
				break;
			}
		}
		else if (i == 1) {
			do {
				switch (Math::RandIntMinMax(0, 6)) {
				case 0:
					traitsUpgrades[i] = critRate;
					break;
				case 1:
					traitsUpgrades[i] = critDamage;
					break;
				case 2:
					traitsUpgrades[i] = reverseShoot;
					break;
				case 3:
					traitsUpgrades[i] = hpUpMSDOWN;
					break;
				case 4:
					traitsUpgrades[i] = brokenShard;
					break;
				case 5:
					traitsUpgrades[i] = rateUpMSDown;
					break;
					
				case 6:
					traitsUpgrades[i] = regen;
					break;
				}
			} while (traitsUpgrades[1] == traitsUpgrades[0]);
		}
		else if (i == 2) {
			do {
				switch (Math::RandIntMinMax(0, 6) ){
				case 0:
					traitsUpgrades[i] = critRate;
					break;
				case 1:
					traitsUpgrades[i] = critDamage;
					break;
				case 2:
					traitsUpgrades[i] = reverseShoot;
					break;
				case 3:
					traitsUpgrades[i] = hpUpMSDOWN;
					break;
				case 4:
					traitsUpgrades[i] = brokenShard;
					break;
				case 5:
					traitsUpgrades[i] = rateUpMSDown;
					break;
				case 6:
					traitsUpgrades[i] = regen;
					break;
				}
			} while (traitsUpgrades[2] == traitsUpgrades[1] || traitsUpgrades[2] == traitsUpgrades[0]);
		}

	}
}

void SceneCollision::reset()
{
	m_goList.clear();
	enemyList.clear();
	timerforbullets.clear();
	dmgandtimefordmgnumber.clear();
	timerfordmgnumber.clear();
	coordinatesofdamagenumbers.clear();
	acquiredGold = 0;
	timerbeforeweaponselect = 1.0f;
	timerBeforeUpgrade = 1.f;
	elapsedTime = 0;
	prevTime = 0;
	m_objectCount = 0;
	minutes = 0;
	seconds = 0;
	chestOpened = false;
	firerateUpgrade = 0;
	Companion = FetchGO();
	Gun = FetchGO();
	Companion->mass = 1;
	flip = 1;
	OutputDialogue = "";
	CurrentTextWrite = false, TextFinished = false;
	CurrentCharText = 0;
	randomDialogue = 0;
	companionX = 9;
	companionY = 9;
	GunShootingTimer = 0;
	SpriteAnimation* G = dynamic_cast<SpriteAnimation*>(CurrentGun);
	G->Reset();
	rotationorder = 1;
	shootonceonly = 1;
	GunShoot = false;
	needtofinishanimation = false;
	PlayerBuy = false;
	currentlyHovering = false;
	shopClick = 0;
	zaxis = 1;
	pause = false;
	MSUpgrade = 0;
	cPlayer2D->reset();
	if (ShopUpgrades[0] > 0)
		for (int Upgrade = 0; Upgrade < ShopUpgrades[0]; ++Upgrade)
			cPlayer2D->IncreaseSpd();
	if (ShopUpgrades[1] > 0)
		for (int Upgrade = 0; Upgrade < ShopUpgrades[1]; ++Upgrade)
			cPlayer2D->IncreaseHP();
	if (ShopUpgrades[2] > 0)
		for (int Upgrade = 0; Upgrade < ShopUpgrades[2]; ++Upgrade)
			cPlayer2D->DecreaseShieldCooldown();
	if (ShopUpgrades[3] > 0)
		for (int Upgrade = 0; Upgrade < ShopUpgrades[3]; ++Upgrade)
			cPlayer2D->IncreaseDmg();
	if (ShopUpgrades[4] > 0)
		for (int Upgrade = 0; Upgrade < ShopUpgrades[4]; ++Upgrade)
			cPlayer2D->IncreaseLifeCount();
	if (ShopUpgrades[5] > 0)
		for (int Upgrade = 0; Upgrade < ShopUpgrades[5]; ++Upgrade)
			cPlayer2D->IncreaseEXPGain();
	dmgofgun = 0;
	velocityofbullet = 20;
	bowframe = 0;
	Transition = false;
	timerforpistol = 0;
	timerfordragon = 0;
	GunRightClickSpecial = false;
	staggertimingforpistol = 0;
	BossKilled = 0;
	surviveSeconds = 60;
	Shield = FetchGO();
	Shield->type = GameObject::GO_SHIELD;
	Shield->scale = Vector3(15, 15, 1);

	enemyspawn = 0;
	enemyspawnspeed = 0.5;
	enemyovertime = 0;

	screenShake[0] = 0;
	screenShake[1] = 0;
	SuperPainPower = false;
	PowerUsed = 0;

	cSoundController->StopAllSound();
}

void SceneCollision::RenderDmgNum(Vector3 posanddmg, bool yesorno)
{
	/*unsigned w = Application::GetWindowWidth();
	unsigned h = Application::GetWindowHeight();
	float posX = (x / w * m_worldWidth) + camera.position.x;
	float posY = m_worldHeight - (y / h * m_worldHeight) + camera.position.y;*/

	unsigned w = Application::GetWindowWidth();
	unsigned h = Application::GetWindowHeight();
	float posX = (posanddmg.x - camera.position.x) / m_worldWidth * 80;
	float posY = (posanddmg.y - camera.position.y) / m_worldHeight * 60;

	while (Movingofdamagenumbers(posX, posanddmg.z))
	{
		if (switchdmgnum % 2 == 0)
		{
			posX -= displaynumberoffset;
			switchdmgnum++;
		}
		else
		{
			posX += displaynumberoffset;
			switchdmgnum++;
		}
	}

	if (posanddmg.z > 9)
	{
		if (switchdmgnum % 2 == 0)
		{
			posY -= displaynumberoffsety;
		}
		else
			posY += displaynumberoffsety;
	}

	std::ostringstream ss;
	ss << posanddmg.z;
	if (yesorno == true)
	{
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 4, posX, posY);
	}
	else
	{
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2, posX, posY);
	}
	coordinatesofdamagenumbers.push_back(posX);
}

void SceneCollision::Update(double dt)
{
	SceneBase::Update(dt);
	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	if (cKeyboardController->IsKeyDown('9'))
	{
		cPlayer2D->leveledUp = true;
		//elapsedTime++;
		//currentState = lose;
	}
	if (cKeyboardController->IsKeyDown('0'))
	{
		currentState = win;
	}
	if (cKeyboardController->IsKeyDown('G'))
	{
		cPlayer2D->UseGold(-9999);
	}
	if (cKeyboardController->IsKeyDown('N'))
	{
		if (Boss != nullptr) {
			surviveSeconds = 5;
			BossKilled = 3;
		}
	}
	//Mouse Section
	double x, y, windowwidth, windowheight;
	Application::GetCursorPos(&x, &y);
	windowwidth = Application::GetWindowWidth();
	windowheight = Application::GetWindowHeight();
	Vector3 mousePos = Vector3((x / windowwidth) * m_worldWidth, ((windowheight - y) / windowheight) * m_worldHeight, 0);
	static bool bLButtonState = false;

	if (currentState == main)
		camera.Update(dt, Vector3(cPlayer2D->pos.x + screenShake[0], cPlayer2D->pos.y + screenShake[1], cPlayer2D->pos.z), m_worldWidth, m_worldHeight);
	else {
		camera.position.Set(0, 0, 1);
		camera.target.Set(0, 0, 0);
		camera.up.Set(0, 1, 0);
	}
	switch (currentState) {
	case start:
	{
		if ((!bLButtonState && Application::IsMousePressed(0)) || cKeyboardController->IsKeyDown(VK_SPACE)) {
			bLButtonState = true;
			if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.2 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.2) && (mousePos.y <= (m_worldHeight * 0.4) + 4.75 && mousePos.y >= (m_worldHeight * 0.4) - 4.75)) {
				
				currentState = difficultySelection;
				chestOpened = false;
				timerbeforeweaponselect = 1.0f;
				timerBeforeUpgrade = 1.f;
				elapsedTime = 0;
				prevTime = 0;
				m_objectCount = 0;
				minutes = 0;
				seconds = 0;
				firerateUpgrade = 0;
				MSUpgrade = 0;
				shieldcooldowntimer = 10;
				cPlayer2D->setStats();
				cSoundController->StopAllSound();
				cSoundController->PlaySoundByID(2);
				SpawnMapObjects();
			}
			else if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.075 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.075) && (mousePos.y <= (m_worldHeight * 0.25) + 4.75 && mousePos.y >= (m_worldHeight * 0.25) - 4.75)) {
				cSoundController->StopAllSound();
				cSoundController->PlaySoundByID(4);

				Gronk = FetchGO();
				Gronk->mass = 1;
				Gronk->type = GameObject::GO_GRONK;
				Gronk->mass = 5;
				Gronk->scale.Set(1, 1, 1);
				Gronk->pos.Set(m_worldWidth / 2, m_worldHeight * 0.4, 1);
				Gronk->vel.SetZero();

				currentState = shop;
			}
			else if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.17 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.17) && (mousePos.y <= (m_worldHeight * 0.1) + 4.75 && mousePos.y >= (m_worldHeight * 0.1) - 4.75)) {
				WritePlayerStats();
				quit = true;
			}
		}
		else if (bLButtonState && !Application::IsMousePressed(0))
		{
			bLButtonState = false;
		}
		break;
	}
	case difficultySelection:
	{
		elapsedTime += dt;
		if (elapsedTime > 1) {
			static bool leftclick = false;
			if ((!leftclick && Application::IsMousePressed(0))) {
				leftclick = true;
			}
			else if (leftclick && !Application::IsMousePressed(0)) {
				leftclick = false;
				if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.1 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.1) &&
					(mousePos.y <= (m_worldHeight * 0.5) + m_worldHeight * 0.075 && mousePos.y >= (m_worldHeight * 0.5) - m_worldHeight * 0.075)) {
					cSoundController->PlaySoundByID(22);
					difficulty = easy;
					hpScaling = 1.1f;
				}
				if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.1 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.1) &&
					(mousePos.y <= (m_worldHeight * 0.3) + m_worldHeight * 0.075 && mousePos.y >= (m_worldHeight * 0.3) - m_worldHeight * 0.075)) {
					cSoundController->PlaySoundByID(22);
					difficulty = hard;
					hpScaling = 1.3;
				}
				currentState = weaponselection;
				elapsedTime = 0;
			}
		}
		break;
	}
	case weaponselection:
	{
		if (Transition == false)
		{
			elapsedTime += dt;
			float scalingthegun = m_worldWidth * 0.01;
			bool Startgame = false;

			if (elapsedTime > timerbeforeweaponselect)
			{
				static bool bLButtonState = false;
				if (!bLButtonState && Application::IsMousePressed(0))
				{
					bLButtonState = true;

				}
				else if (bLButtonState && !Application::IsMousePressed(0))
				{
					bLButtonState = false;
					if ((mousePos.x >= (m_worldWidth * 0.2) - scalingthegun * 2.5 && mousePos.x <= (m_worldWidth * 0.2) + scalingthegun * 2.5) &&
						(mousePos.y <= (m_worldHeight * 0.6) + scalingthegun && mousePos.y >= (m_worldHeight * 0.6) - scalingthegun))
					{
						Startgame = true;
						Gun->type = GameObject::GO_GL;
					}
					else if ((mousePos.x >= (m_worldWidth * 0.5) - scalingthegun * 2.5 && mousePos.x <= (m_worldWidth * 0.5) + scalingthegun * 2.5) &&
						(mousePos.y <= (m_worldHeight * 0.6) + scalingthegun * 2.5 && mousePos.y >= (m_worldHeight * 0.6) - scalingthegun * 2.5))
					{
						Startgame = true;
						Gun->type = GameObject::GO_BOW;
					}
					else if ((mousePos.x >= (m_worldWidth * 0.8) - scalingthegun * 2.5 && mousePos.x <= (m_worldWidth * 0.8) + scalingthegun * 2.5) &&
						(mousePos.y <= (m_worldHeight * 0.6) + scalingthegun && mousePos.y >= (m_worldHeight * 0.6) - scalingthegun))
					{
						Startgame = true;
						Gun->type = GameObject::GO_SHOTGUN;
					}
					else if ((mousePos.x >= (m_worldWidth * 0.4) - scalingthegun * 3.5 && mousePos.x <= (m_worldWidth * 0.4) + scalingthegun * 3.5) &&
						(mousePos.y <= (m_worldHeight * 0.4) + scalingthegun * 0.72282608695 && mousePos.y >= (m_worldHeight * 0.4) - scalingthegun * 0.72282608695))
					{
						Startgame = true;
						Gun->type = GameObject::GO_SNIPER;
					}
					else if ((mousePos.x >= (m_worldWidth * 0.6) - scalingthegun * 2.5 && mousePos.x <= (m_worldWidth * 0.6) + scalingthegun * 2.5) &&
						(mousePos.y <= (m_worldHeight * 0.4) + scalingthegun && mousePos.y >= (m_worldHeight * 0.4) - scalingthegun))
					{
						Startgame = true;
						Gun->type = GameObject::GO_PISTOL;
					}
				}

				static bool hLButtonState = false;
				if (!hLButtonState && Application::IsKeyPressed('H'))
				{
					Startgame = true;
					Gun->type = GameObject::GO_MACHINEGUN;
					hLButtonState = true;
				}

				Gun->mass = 2;
				GunFrameWhereItStarts = 6;
				if (Gun->type == GameObject::GO_GL)
				{
					Gun->scale.Set(5, 2, 1);
					CurrentGun = meshList[GEO_GL];
					numberofbullets = 1;
					dmgofgun = 5; //explosion does 5
					pierceforbullet = 1;
					firerate = 1.2f;
				}
				else if (Gun->type == GameObject::GO_BOW)
				{
					Gun->scale.Set(5, 5, 1);
					CurrentGun = meshList[GEO_BOW];
					numberofbullets = 1;
					dmgofgun = 1;
					pierceforbullet = 1;
					firerate = 2.0f;
					Gun->activeTime = 0;
				}
				else if (Gun->type == GameObject::GO_SHOTGUN)
				{
					Gun->scale.Set(5, 2, 1);
					CurrentGun = meshList[GEO_SHOTGUN];
					numberofbullets = 4;
					dmgofgun = 6;
					pierceforbullet = 1;
					firerate = 1.2f;
				}
				else if (Gun->type == GameObject::GO_SNIPER)
				{
					Gun->scale.Set(7, 1.44565217391, 1);
					CurrentGun = meshList[GEO_SNIPER];
					numberofbullets = 1;
					dmgofgun = 10;
					pierceforbullet = 3;
					velocityofbullet = 50;
					firerate = 1.3f;
					Gun->thickWall = 0;
					Gun->prevangle = 0;
					Gun->activeTime = 0;
					Gun->angle1secago = 0;
				}
				else if (Gun->type == GameObject::GO_PISTOL)
				{
					Gun->scale.Set(5, 2, 1);
					CurrentGun = meshList[GEO_PISTOL];
					numberofbullets = 1;
					dmgofgun = 6;
					pierceforbullet = 1;
					firerate = 1;
					Gun->mass = 6 + numberofbullets;
				}
				else if (Gun->type == GameObject::GO_MACHINEGUN)
				{
					Gun->scale.Set(5, 2, 1);
					CurrentGun = meshList[GEO_MACHINEGUN];
					numberofbullets = 1;
					dmgofgun = 1;
					pierceforbullet = 1;
					firerate = 0.5f;
					Gun->activeTime = 0;
					Gun->mass = 0;
				}
				Gun->pos.Set(cPlayer2D->pos.x, cPlayer2D->pos.y, 3);
				Gun->vel.SetZero();
				if (Startgame)
				{
					killcounter = 0;
					cSoundController->PlaySoundByID(15);
					Transition = true;
					elapsedTime = 2;
					bossspawned = false;
					ArrowToBoss = FetchGO();
					ArrowInsideArrow = FetchGO();
					Shield->pos = cPlayer2D->pos;
					Gun->critchance = 10;
					Gun->critdamage = 1.5;
					Gun->reverseShoot = false;
					GunRightClickSpecial = false;
				}
			}
		}

		else if (Transition == true)
		{
			elapsedTime -= 1 * dt;

			SongVolumeChange += 1.f * dt;
			if (SongVolumeChange >= .35f)
			{
				SongVolumeChange = 0;
				cSoundController->MasterVolumeDecrease();
			}

			if (elapsedTime <= 0.f) {
				SongVolumeChange = 0;
				currentState = main;
				cSoundController->StopPlayByID(2);
				cSoundController->PlaySoundByID(5);
			}
		}
		break;
	}
	case shop:
	{
		zaxis = 1;
		SpriteAnimation* gronk = dynamic_cast<SpriteAnimation*>(meshList[GEO_GRONK]);
		gronk->PlayAnimation("Idle", -1, 2.f);
		gronk->Update(dt);

		dialogueTime += 1 * dt;
		if (dialogueTime > 5.f && currentlyHovering != true)
		{
			CurrentTextWrite = true;
			dialogueTime = 0;
		}
		
		ShopInteraction(dt);
		break;
	}
	case main:
	{
		PlayerMapCheck();
		MapBoundary();
		zaxis = 1;
		displaynumberoffset = 0;
		displaynumberoffsety = 0;
		switchdmgnum = 1;
		coordinatesofdamagenumbers.clear();
		SpriteAnimation* ocean = dynamic_cast<SpriteAnimation*>(meshList[GEO_BOUNDARY]);
		//Add the animation for ocean boundary
		ocean->PlayAnimation("Waves", -1, 5.f);
		ocean->Update(dt);

		if (Transition == true) {
			elapsedTime += 1 * dt;

			SongVolumeChange += 1.f * dt;
			if (SongVolumeChange >= 0.6f)
			{
				SongVolumeChange = 0;
				if (cSoundController->GetMasterVolume() <= 0.5f)
					cSoundController->MasterVolumeIncrease();
			}

			if (elapsedTime >= 3) {
				SongVolumeChange = 0;
				Transition = false;
				elapsedTime = 0;
			}

			unsigned size = m_goList.size();

			for (unsigned i = 0; i < size; ++i)
			{
				GameObject* go = m_goList[i];
				if (go->type == GameObject::GO_SHIELD)
				{
					go->pos = cPlayer2D->pos;
					if (go->visible)
					{
						for (unsigned i = 0; i < enemyList.size(); ++i)
						{
							Enemy* go1 = enemyList[i];
							Vector3 relativeVel = go->vel - go1->vel;

							Vector3 disDiff = go1->pos - go->pos;

							if (go->pos.y > go1->pos.y)
							{
								disDiff -= Vector3(0, go1->scale.y / 2, 0);
							}
							else
							{
								disDiff += Vector3(0, go1->scale.y / 2, 0);
							}

							if (go->pos.x > go1->pos.x)
							{
								disDiff -= Vector3(go1->scale.x / 2, 0, 0);
							}
							else
							{
								disDiff += Vector3(go1->scale.x / 2, 0, 0);
							}


							if (relativeVel.Dot(disDiff) <= 0) {
								continue;
							}
							if (disDiff.LengthSquared() <= (go->scale.x + go1->scale.x) * (go->scale.x + go1->scale.x))
							{
								go->visible = false;
								go->activeTime = elapsedTime + (shieldcooldowntimer - cPlayer2D->getlowerShieldTime());
							}
						}
					}
					else if (go->activeTime < elapsedTime)
					{
						go->visible = true;
					}
				}
			}
		}
		else if (Transition == false)
		{
			//picked up super pain power up
			if (SuperPainPower == true)
			{
				PowerUsed += 1 * dt;
				MakeScreenShake();
				for (int i = 0; i < enemyList.size(); ++i)
				{
					Enemy* enemy = enemyList[i];

					float Distance = cPlayer2D->pos.Length() - enemy->pos.Length();

					if (Distance < 0)
						Distance = -Distance;

					if (Distance < 100) {
						if (enemy->GEOTYPE != GEO_BOSS_SLIME && enemy->GEOTYPE != GEO_SPIDER && enemy->GEOTYPE != GEO_VAMPIRE)
							enemy->dieanimation();
					}
				}
			}
			if (PowerUsed > 2.f && SuperPainPower == true)
			{
				screenShake[0] = screenShake[1] = 0;
				SuperPainPower = false;
				PowerUsed = 0;
			}
			//update time
			if (seconds >= 60) {
				minutes += 1;
				seconds -= 60;
			}
			//check if player has leveled up;
			if (cPlayer2D->xpToLevel() == true && !cPlayer2D->leveledUp)
			{
				cPlayer2D->leveledUp = true;
				timerBeforeUpgrade = elapsedTime + 1;
				//generate 3 random upgrades for the player to choose
				for (int i = 0; i < 3; ++i) {
					if (i == 0) {
						switch (Math::RandIntMinMax(0, 7)) {
						case 0:
							levelUpgrades[i] = pierce;
							break;
						case 1:
							levelUpgrades[i] = multishot;
							break;
						case 2:
							levelUpgrades[i] = atk;
							break;
						case 3:
							levelUpgrades[i] = hp;
							break;
						case 4:
							levelUpgrades[i] = velocity;
							break;
						case 5:
							levelUpgrades[i] = moveSpeed;
							if (MSUpgrade != 5) {
								break;
							}
						case 6:
							levelUpgrades[i] = fireRate;
							if (firerateUpgrade != 5) {
								break;
							}
						case 7:
							levelUpgrades[i] = dragon;
							break;
						}
					}
					else if (i == 1) {
					do {
						switch (Math::RandIntMinMax(0, 7)) {
						case 0:
							levelUpgrades[i] = pierce;
							break;
						case 1:
							levelUpgrades[i] = multishot;
							break;
						case 2:
							levelUpgrades[i] = atk;
							break;
						case 3:
							levelUpgrades[i] = hp;
							break;
						case 4:
							levelUpgrades[i] = velocity;
							break;
						case 5:
							levelUpgrades[i] = moveSpeed;
							if (MSUpgrade != 5) {
								break;
							}
						case 6:
							levelUpgrades[i] = fireRate;
							if (firerateUpgrade != 5) {
								break;
							}
						case 7:
							levelUpgrades[i] = dragon;
							break;
						}
					} while (levelUpgrades[1] == levelUpgrades[0]);
					}
					else if (i == 2) {
					do {
						switch (Math::RandIntMinMax(0, 7)) {
						case 0:
							levelUpgrades[i] = pierce;
							break;
						case 1:
							levelUpgrades[i] = multishot;
							break;
						case 2:
							levelUpgrades[i] = atk;
							break;
						case 3:
							levelUpgrades[i] = hp;
							break;
						case 4:
							levelUpgrades[i] = velocity;
							break;
						case 5:
							levelUpgrades[i] = moveSpeed;
							if (MSUpgrade != 5) {
								break;
							}
						case 6:
							levelUpgrades[i] = fireRate;
							if (firerateUpgrade != 5) {
								break;
							}
						case 7:
							levelUpgrades[i] = dragon;
							break;
						}
					} while (levelUpgrades[2] == levelUpgrades[1] || levelUpgrades[2] == levelUpgrades[0]);
					}

				}

			}
			//main gameplay loop
			if (cPlayer2D->leveledUp == false && pause == false && cPlayer2D->xpToLevel() == false && chestOpened == false)
			{
				cPlayer2D->Update(dt);
				seconds += dt;
				elapsedTime += dt;
				if (surviveSeconds <= 0) {
					if (cPlayer2D->pos.x > (m_worldWidth * 0.5) - 2.5 && cPlayer2D->pos.x < (m_worldWidth * 0.5) + 2.5 &&
						cPlayer2D->pos.y >(m_worldHeight * 0.5) - 5 && cPlayer2D->pos.y < (m_worldHeight * 0.5) + 5) {
						currentState = win;
					}
				}
				else if (BossKilled >= 3) {
					surviveSeconds -= dt;
				}
				if (cKeyboardController->IsKeyPressed('B') ){
					pause = true;
					break;
				}
				//Enemy Spawn
				if (cKeyboardController->IsKeyPressed('M'))
				{
					killcounter += 100;
				}
				//satisfy win conditions
				if (cKeyboardController->IsKeyPressed('N'))
				{
					BossKilled = 3;
					surviveSeconds = 10;

				}

				//enemy spawn over time
				enemyovertime += dt;
				if (enemyovertime)
				{
					double spawn = elapsedTime - enemyspawn;
					if (spawn > enemyspawnspeed)
					{
						Vector3 Epos;
						Enemy* go = new Enemy();

						int typeOfEnemy = Math::RandIntMinMax(0, 20);
						switch (typeOfEnemy)
						{
						default:
						{
							int whichEnemytoSpawn = Math::RandIntMinMax(0, 20);
							switch (whichEnemytoSpawn)
							{
							case 0:
								go->setEnemyType(0, meshList[GEO_SKELETON]); //Set enemy type, 0 for Skeleton
								break;
							default:
								go->setEnemyType(1, meshList[GEO_GHOST]); //Set enemy type, 1 for Ghost
								break;
							}
							go->sethp(20 * pow(hpScaling, minutes));
							break;
						}
						case 20:
							go->setEnemyType(2, meshList[GEO_ZOMBIE]); //Set enemy type, 2 for ZOMBIE
							go->sethp(40 * pow(hpScaling, minutes));
							break;
						}

						Enemy::setSpawn(cPlayer2D->pos.x, cPlayer2D->pos.y, Epos);
						/*go->type = GameObject::GO_BOSS_SLIME;*/ //dont need this anymore
						go->scale.Set(10, 10, 1);
						go->pos = Epos;
						go->mass = 10;

						std::cout << Epos.x << endl;
						std::cout << Epos.y << endl;

						enemyList.push_back(go);

						const void* address = static_cast<const void*>(go);
						std::stringstream ss;
						ss << address;
						go->address = ss.str();

						enemyspawn = elapsedTime;
					}
				}

				//shooting
				SpriteAnimation* G = dynamic_cast<SpriteAnimation*>(CurrentGun);
				bool shooting = true;
				{
					if (Application::IsMousePressed(0) && shooting && !needtofinishanimation)
					{
						if (Gun->type == GameObject::GO_BOW)
						{
							G->PlayAnimation("Shoot", 0, firerate);

							G->Update(dt);

							if (G->getcurrentanimationframe("Shoot") == 0)
							{
								cSoundController->PlaySoundByID(9);
							}

							if (G->getAnimationStatus("Shoot"))
							{
								shooting = false;
								bowframe = G->getcurrentanimationframe("Shoot");
							}
						}
						else
						{
							float Xaxis = mousePos.x - Gun->pos.x;
							if (Xaxis >= 0)
								G->PlayAnimation("Shoot", 0, firerate);
							else
								G->PlayAnimation("ShootR", 0, firerate);

							if (GunShoot == false)
							{
								if (G->getcurrentanimationframe("Shoot") == 0 && (Gun->mass < elapsedTime || Gun->type == GameObject::GO_PISTOL))
								{
									SceneCollision::shooting(elapsedTime, numberofbullets, Gun);
								}
								else if (G->getcurrentanimationframe("ShootR") == GunFrameWhereItStarts && (Gun->mass < elapsedTime || Gun->type == GameObject::GO_PISTOL))
								{
									SceneCollision::shooting(elapsedTime, numberofbullets, Gun);
								}

								GunShoot = true;
							}

							shooting = false;

							G->Update(dt);
						}
					}

					if (G->getAnimationStatus("Shoot") == false && Gun->type == GameObject::GO_BOW && shootonceonly == 1)
					{
						bowframe = 12;
						SceneCollision::shooting(elapsedTime, numberofbullets, Gun);
						shootonceonly = 0;
					}
					else if (GunShoot == true)
					{
						float Xaxis = mousePos.x - Gun->pos.x;
						if ((Xaxis >= 0 && !G->getAnimationStatus("Shoot")) || (Xaxis < 0 && !G->getAnimationStatus("ShootR")))
						{
							GunShoot = false;

							if (Xaxis >= 0)
								G->PlayAnimation("Shoot", -1, firerate);
							else
								G->PlayAnimation("ShootR", -1, firerate);

							G->truereset();
						}
					}
					if (shooting && needtofinishanimation && (G->getAnimationStatus("Shoot") || G->getAnimationStatus("ShootR")))
					{
						if (!xisneg)
						{
							float Xaxis = mousePos.x - Gun->pos.x;
							if (Xaxis < 0)
							{
								G->PlayAnimation("ShootR", 0, firerate);
								xisneg = true;
							}
						}
						else
						{
							float Xaxis = mousePos.x - Gun->pos.x;
							if (Xaxis >= 0)
							{
								G->PlayAnimation("Shoot", 0, firerate);
								xisneg = false;
							}
						}

						G->Update(dt);

						if (!xisneg)
						{
							if (!G->getAnimationStatus("Shoot"))
								needtofinishanimation = false;
						}
						else
						{
							if (!G->getAnimationStatus("ShootR"))
								needtofinishanimation = false;
						}
					}
					else if (shooting && !needtofinishanimation)
					{
						float Xaxis = mousePos.x - Gun->pos.x;
						if (Xaxis < 0)
						{
							G->PlayAnimation("ShootR", 0, firerate);
						}
						else
							G->PlayAnimation("Shoot", 0, firerate);

						G->Reset();
					}


					static bool bLButtonState = false;
					if (!bLButtonState && Application::IsMousePressed(0))
					{
						bLButtonState = true;
					}
					else if (bLButtonState && !Application::IsMousePressed(0))
					{
						bLButtonState = false;
						if (Gun->type == GameObject::GO_BOW)
						{
							if (shootonceonly == 1 && bowframe > 0)
							{
								SceneCollision::shooting(elapsedTime, numberofbullets, Gun);
							}
							shooting = false;
							G->PlayAnimation("Shoot", 0, firerate);
							G->truereset();
							shootonceonly = 1;
						}
						else if (Gun->type != GameObject::GO_BOW)
						{
							float Xaxis = mousePos.x - Gun->pos.x;

							if (G->getcurrentanimationframe("Shoot") != 0)
							{
								needtofinishanimation = true;
								xisneg = false;
							}
							else if (G->getcurrentanimationframe("ShootR") != GunFrameWhereItStarts)
							{
								needtofinishanimation = true;
								xisneg = true;
							}
						}
					}

					static bool bRButtonState = false;
					if (!bRButtonState && Application::IsMousePressed(1))
					{
						if (Gun->type == GameObject::GO_PISTOL && timerforpistol < elapsedTime)
						{
							timerforpistol = elapsedTime + firerate * 2;
							staggertimingforpistol = elapsedTime;
							GunRightClickSpecial = true;
							Gun->mass = 0;
						}
						else if (Gun->type == GameObject::GO_MACHINEGUN && Gun->activeTime < elapsedTime && GunRightClickSpecial == false)
						{
							Gun->mass = elapsedTime + 3.8f;
							Gun->activeTime = elapsedTime + 0.5f;
							cSoundController->PlaySoundByID(18);
							GunRightClickSpecial = true;
						}
						else if (Gun->type == GameObject::GO_GL && Gun->activeTime < elapsedTime)
						{
							Gun->mass = elapsedTime + 3.8f;
							Gun->activeTime = elapsedTime + 0.5f;
							cSoundController->PlaySoundByID(18);
							GunRightClickSpecial = true;
						}
						bRButtonState = true;
					}
					else if (bRButtonState && !Application::IsMousePressed(1))
					{
						bRButtonState = false;
					}
					if (GunRightClickSpecial)
					{
						if (staggertimingforpistol < elapsedTime && Gun->mass < 6 + numberofbullets && Gun->type == GameObject::GO_PISTOL) //for pistol special ability, 1 mass means 1 bullet
						{
							SceneCollision::PistolShooting(elapsedTime, 1);
							staggertimingforpistol = elapsedTime + 0.08f;
							G->PlayAnimation("Shoot", 0, 0.08f);
							G->Update(dt);
							Gun->mass++;
							if (Gun->mass == 6 + numberofbullets)
								GunRightClickSpecial = false;
						}
						else if (Gun->activeTime < elapsedTime && Gun->mass > elapsedTime && Gun->type == GameObject::GO_MACHINEGUN) //activetime is skill cooldown and gun mass would be firing duration
						{
							MakeScreenShakeForMG();
							MachineGunPewPew(elapsedTime, numberofbullets);
							if (Gun->mass > elapsedTime)
							{
								Gun->activeTime = elapsedTime + 0.08f;

								if (meshList[GEO_MACHINEGUN]->material.kAmbient.b > 0.1f)
								{
									meshList[GEO_MACHINEGUN]->material.kAmbient.b -= 0.05;
									meshList[GEO_MACHINEGUN]->material.kAmbient.g -= 0.05;
									meshList[GEO_LMGBULLET]->material.kAmbient.b -= 0.05;
									meshList[GEO_LMGBULLET]->material.kAmbient.g -= 0.05;
								}
							}
						}
						else if (Gun->mass < elapsedTime && Gun->type == GameObject::GO_MACHINEGUN)
						{
							Gun->activeTime = elapsedTime + firerate * 10;
							GunRightClickSpecial = false;
						}
					}
				}

				unsigned size = m_goList.size();

				//Physics Simulation Section
				for (unsigned i = 0; i < size; ++i)
				{
					GameObject* go = m_goList[i];
					if (go->active)
					{
						go->pos += go->vel * dt * m_speed;

						if (go->type == GameObject::GO_COMPANION)
						{
							float moveXby;
							float moveYby;
							{
								if (rotationorder == 1)
								{
									moveXby = -0.5;
									moveYby = 0.2;
									if (companionX <= 0)
										rotationorder++;
								}
								else if (rotationorder == 2)
								{
									moveXby = -0.5;
									moveYby = -0.2;
									if (companionX <= -9)
										rotationorder++;
								}
								else if (rotationorder == 3)
								{
									moveXby = -0.2;
									moveYby = -0.5;
									if (companionY <= 0)
										rotationorder++;
								}
								else if (rotationorder == 4)
								{
									moveXby = 0.2;
									moveYby = -0.5;
									if (companionY <= -9)
										rotationorder++;
								}
								else if (rotationorder == 5)
								{
									moveXby = 0.5;
									moveYby = -0.2;
									if (companionX >= 0)
										rotationorder++;
								}
								else if (rotationorder == 6)
								{
									moveXby = 0.5;
									moveYby = 0.2;
									if (companionX >= 9)
										rotationorder++;
								}
								else if (rotationorder == 7)
								{
									moveXby = 0.2;
									moveYby = 0.5;
									if (companionY >= 0)
										rotationorder++;
								}
								else
								{
									moveXby = -0.2;
									moveYby = 0.5;
									if (companionY >= 9)
										rotationorder = 1;
								}
							}
							companionX += moveXby;
							companionY += moveYby;


							SpriteAnimation* Companion = dynamic_cast<SpriteAnimation*>(meshList[GEO_COMPANION]);

							if (timerfordragon < elapsedTime && mousePos.x < go->pos.x)
							{
								Companion->PlayAnimation("ShootL", -1, 2.0f); //dragon
								Companion->Update(dt);
								if (Companion->getcurrentanimationframe("ShootL") == 31 && go->bounce)
								{
									SceneCollision::dragonshooting(numberofbullets, 35, 2);
									go->bounce = false;
								}
								else if (Companion->getcurrentanimationframe("ShootL") == 34 && !go->bounce)
								{
									timerfordragon = elapsedTime + 5;
									go->bounce = true;
								}
							}
							else if (timerfordragon < elapsedTime)
							{
								Companion->PlayAnimation("ShootR", -1, 2.0f);
								Companion->Update(dt);
								if (Companion->getcurrentanimationframe("ShootR") == 24 && go->bounce)
								{
									SceneCollision::dragonshooting(numberofbullets, 35, 2);
									go->bounce = false;
								}
								else if (Companion->getcurrentanimationframe("ShootR") == 27 && !go->bounce)
								{
									timerfordragon = elapsedTime + 5;
									go->bounce = true;
								}
							}
							else if (mousePos.x > go->pos.x)
							{
								Companion->PlayAnimation("RunningR", -1, 2.0f);
								Companion->Update(dt);
							}
							else
							{
								Companion->PlayAnimation("RunningL", -1, 2.0f);
								Companion->Update(dt);
							}

						}
						else if (go == Gun)
						{
							Application::GetCursorPos(&x, &y);
							unsigned w = Application::GetWindowWidth();
							unsigned h = Application::GetWindowHeight();
							float posX = (x / w * m_worldWidth) + camera.position.x;
							float posY = m_worldHeight - (y / h * m_worldHeight) + camera.position.y;
							Vector3 center = Vector3(posX, posY, 0) - cPlayer2D->pos;
							float angle = calculateAngle(center.x, center.y);

							float Xaxis = mousePos.x - cPlayer2D->pos.x;
							float Yaxis = mousePos.y - cPlayer2D->pos.y;

							if (Gun->type != GameObject::GO_PISTOL)
								go->angle = angle;
							else if (Gun->mass >= 6 + numberofbullets)
								go->angle = angle;

							if (Gun->type == GameObject::GO_SNIPER)
							{
								float calibrateangle = 0;
								if ((Xaxis > 0 && Yaxis < 0 && go->prevangle < 90 && go->angle > 270))
								{
									calibrateangle = go->angle - 360;
								}
								else if (Xaxis >= 0 && Yaxis >= 0 && go->prevangle > 270 && go->angle < 90)
								{
									calibrateangle = go->angle + 360;
								}
								else
									calibrateangle = go->angle;

								float changeofangle = calibrateangle - go->prevangle;

								if (changeofangle < 0)
									changeofangle = -changeofangle;

								go->thickWall += changeofangle;

								if (go->thickWall > 360)
									go->thickWall = 360;

								if (go->activeTime < elapsedTime)
								{
									go->activeTime = elapsedTime + 0.3f;

									float changewithin1sec = go->angle1secago - go->angle;

									if (changewithin1sec < 0)
										changewithin1sec = -changewithin1sec;

									if (changewithin1sec < 10)
										go->thickWall = 0;

									go->angle1secago = go->angle;
								}


								go->prevangle = go->angle;
							}
							else if (Gun->type == GameObject::GO_MACHINEGUN)
							{
								if (Gun->mass < elapsedTime)
								{
									GunRightClickSpecial = false;
									if (meshList[GEO_MACHINEGUN]->material.kAmbient.b < 1)
									{
										meshList[GEO_MACHINEGUN]->material.kAmbient.b += 0.01;
										meshList[GEO_MACHINEGUN]->material.kAmbient.g += 0.01;
										meshList[GEO_LMGBULLET]->material.kAmbient.b += 0.01;
										meshList[GEO_LMGBULLET]->material.kAmbient.g += 0.01;
									}
								}
							}
						}
						else if (go->type == GameObject::GO_PROJECTILE)
						{
							if (elapsedTime > timerforbullets[go->lifetime])
							{
								if (go->proj == GameObject::GL)
								{
									GameObject* Explosion = FetchGO();
									Explosion->type = GameObject::GO_EXPLOSION;
									Explosion->pos = go->pos;
									Explosion->scale.Set(8, 8, 1);
									for (int arraynumber = 0; arraynumber < timerforbullets.size(); ++arraynumber)
									{
										if (timerforbullets[arraynumber] != 0)
										{
											continue;
										}
										timerforbullets[arraynumber] = elapsedTime + 0.5f;
										Explosion->lifetime = arraynumber;
										break;
									}
									timerforbullets.push_back(elapsedTime + 0.5f);
									Explosion->lifetime = timerforbullets.size() - 1;
								}

								ReturnGO(go);
								timerforbullets[go->lifetime] = 0;
							}
							else if (go->pos.x > camera.position.x + m_worldWidth || go->pos.x - camera.position.x < 0 || go->pos.y > camera.position.y + m_worldHeight || go->pos.y - camera.position.y < 0)
							{
								ReturnGO(go);
							}

							//collision check and response
							{
								for (unsigned x = 0; x < enemyList.size(); ++x)
								{
									Enemy* go2 = enemyList[x];
									if (go2->gethp() > 0)
									{
										if (SceneCollision::bulletcollisioncheck(Gun, go, go2))
										{
											SceneCollision::dobulletcollision(Gun, go, go2);
										}
									}
								}
							}
						}
						else if (go->type == GameObject::GO_EXPLOSION)
						{
							if (elapsedTime > timerforbullets[go->lifetime])
								ReturnGO(go);

							for (unsigned x = 0; x < enemyList.size(); ++x)
							{
								Enemy* go2 = enemyList[x];
								if (go2->gethp() > 0)
								{
									if (SceneCollision::bulletcollisioncheck(Gun, go, go2))
									{
										SceneCollision::dobulletcollision(Gun, go, go2);
									}
								}
							}
						}
						else if (go->type == GameObject::GO_SHIELD)
						{
							go->pos = cPlayer2D->pos;
							if (go->activeTime < elapsedTime)
							{
								go->visible = true;
							}
						}
						else if (go->type == GameObject::GO_SKELETONATTACK)
						{
							if (go->pos.x > camera.position.x + m_worldWidth || go->pos.x - camera.position.x < 0 || go->pos.y > camera.position.y + m_worldHeight || go->pos.y - camera.position.y < 0)
							{
								ReturnGO(go);
							}
							Vector3 disDiff = cPlayer2D->pos - go->pos;

							if (disDiff.LengthSquared() <= (go->scale.x + 4) * (go->scale.x + 4))
							{
								if (Shield->visible)
								{
									Shield->visible = false;
									Shield->activeTime = elapsedTime + (shieldcooldowntimer - cPlayer2D->getlowerShieldTime());
								}
								else if (cPlayer2D->inVuln < elapsedTime)
								{
									cPlayer2D->hp -= 1;
									cPlayer2D->inVuln = elapsedTime + 0.5f;
								}
							}
						}
						else if (go->type == GameObject::GO_CHEST)
						{
							if (go->placed)
							{
								Vector3 relativeVel = cPlayer2D->vel - go->vel;

								Vector3 disDiff = go->pos - cPlayer2D->pos;

								if (cPlayer2D->pos.y > go->pos.y)
								{
									disDiff -= Vector3(0, go->scale.y / 2, 0);
								}
								else
								{
									disDiff += Vector3(0, go->scale.y / 2, 0);
								}

								if (cPlayer2D->pos.x > go->pos.x)
								{
									disDiff -= Vector3(go->scale.x / 2, 0, 0);
								}
								else
								{
									disDiff += Vector3(go->scale.x / 2, 0, 0);
								}


								if (relativeVel.Dot(disDiff) <= 0) {
									continue;
								}
								if (disDiff.LengthSquared() <= (10 + go->scale.x) * (10 + go->scale.x))
								{
									go->placed = false;
								}
							}
							else
							{
								SpriteAnimation* chest = dynamic_cast<SpriteAnimation*>(meshList[GEO_CHEST]);
								chest->PlayAnimation("Opening", 0, 1.0f);
								chest->Update(dt);
								if (!chest->getAnimationStatus("Opening"))
								{
									ReturnGO(go);
									chestOpened = true;
									generateTraits();
									chest->truereset();
									break;
								}
							}
						}
						

						if (cPlayer2D->getState() == cPlayer2D->DEAD) {
							currentState = lose;
						}
					}
				}

				//arrow to boss
				
				if (BossKilled >= 3)
				{
					ArrowToBoss->pos = cPlayer2D->pos + Vector3(0, -10, 0);
					Vector3 center = Vector3(ArrowToBoss->pos.x, ArrowToBoss->pos.y, 0) - Vector3(m_worldWidth * 0.5, m_worldHeight * 0.5, 0);
					ArrowToBoss->angle = calculateAngle(center.x, center.y);
					ArrowInsideArrow->pos = ArrowToBoss->pos;
					ArrowInsideArrow->scale = ArrowToBoss->scale;
				}
				else if (bossspawned)
				{
					ArrowToBoss->pos = cPlayer2D->pos + Vector3(0, -10, 0);
					Vector3 center = Vector3(ArrowToBoss->pos.x, ArrowToBoss->pos.y, 0) - Boss->pos;
					ArrowToBoss->angle = calculateAngle(center.x, center.y);
					ArrowInsideArrow->pos = ArrowToBoss->pos;
					Boss->timerforboss -= dt;
					ArrowInsideArrow->scale.x = Boss->timerforboss / 90 * 4;
					ArrowInsideArrow->scale.y = ArrowInsideArrow->scale.x;
					if (Boss->timerforboss <= 0)
					{
						cPlayer2D->hp -= 999;
					}
				}

				//Enemy List

				enemyAnimationPlayed.clear();
				enemycurrentstate.clear();
				for (unsigned i = 0; i < enemyList.size(); ++i)
				{
					Enemy* go1 = enemyList[i];
					
					bool runanimation = true;
					go1->Update(dt);

					//Boss only chases player if they are in screen
					if ((go1->getState() == 3 || go1->getState() == 4) && !go1->Deadornot())
					{
						DeleteEnemy(go1);
					}
					else if (go1->getState() != 3 && go1->getState() != 4)
					{
						if (go1->GEOTYPE == GEO_BOSS_SLIME || go1->GEOTYPE == GEO_VAMPIRE || go1->GEOTYPE == GEO_SPIDER)
						{
							float distFromPlayerX = go1->pos.x - cPlayer2D->pos.x;
							float distFromPlayerY = go1->pos.y - cPlayer2D->pos.y;
							if (Vector3(distFromPlayerX, distFromPlayerY, 0).Length() < 100)
							{
								go1->vel = cPlayer2D->pos - go1->pos;
								go1->vel = go1->vel.Normalized();
								go1->vel = go1->vel * 20;
								go1->pos += go1->vel * dt;

								//if (Vector3(distFromPlayerX, distFromPlayerY, 0).Length() < 50)
								//{
								//	
								//}
							}
							else
							{
								go1->vel.SetZero();
							}
						}
						else
						{
							go1->vel = cPlayer2D->pos - go1->pos;
							go1->vel = go1->vel.Normalized();
							go1->vel = go1->vel * 20;
							//MoveEnemiesToPlayer(go1, cPlayer2D, dt);
							//go1->pos += go1->vel * dt;

							go1->pos += go1->vel * dt;
							if (go1->GEOTYPE == GEO_SKELETON)
							{
								float Distance = cPlayer2D->pos.Length() - go1->pos.Length();
								if (Distance < 75 && go1->rangedcooldown < elapsedTime)
								{
									Vector3 center = cPlayer2D->pos - go1->pos;
									float angle = calculateAngle(center.x, center.y);
									float magnitude = center.Length();
									GameObject* go = FetchGO();
									go->pos = go1->pos;
									go->scale.Set(4, 4, 1);
									go->type = GameObject::GO_SKELETONATTACK;
									go->angle = angle;
									if (go->angle > 360) {
										go->angle -= 360;
									}
									go->vel.x = cos(Math::DegreeToRadian(go->angle)) * magnitude;
									go->vel.y = sin(Math::DegreeToRadian(go->angle)) * magnitude;
									go->vel.Normalize() *= 30;
									go->damage = 10;
									go1->rangedcooldown = elapsedTime + 5.f;
								}
							}
						}
						if (CheckCollision(go1, cPlayer2D))
						{
							if (Shield->visible)
							{
								Shield->visible = false;
								Shield->activeTime = elapsedTime + (shieldcooldowntimer - cPlayer2D->getlowerShieldTime());
							}
							else if (cPlayer2D->inVuln < elapsedTime)
							{
								if (go1->GEOTYPE == GEO_BOSS_SLIME || go1->GEOTYPE == GEO_VAMPIRE || go1->GEOTYPE == GEO_SPIDER)
								{
									cPlayer2D->hp -= 10;
									cPlayer2D->inVuln = elapsedTime + 0.5f;
								}
								else
								{
									cPlayer2D->hp -= 2;
									cPlayer2D->inVuln = elapsedTime + 0.5f;
								}
							}
						}

						for (unsigned j = 0; j < enemyList.size(); ++j)
						{
							Enemy* go2 = enemyList[j];
							if (go1 == go2) {
								continue;
							}

							if (go2->gethp() > 0)
							{
								CheckCollision(go1, go2, dt);
							}
						}
					}
				}
			}
			//leveled up
			else if (cPlayer2D->leveledUp == true && cPlayer2D->xpToLevel() == true) {
				elapsedTime += dt;
				if (elapsedTime > timerBeforeUpgrade) {
					static bool LMPressed = false;
					if (Application::IsMousePressed(0) && !LMPressed) {
						LMPressed = true;
					}
					else if (!Application::IsMousePressed(0) && LMPressed) {
						LMPressed = false;
						for (int i = 1; i < 4; ++i) {
							float x = (i * 0.04 * m_worldWidth) + ((i - 1) * 0.28 * m_worldWidth) + (m_worldWidth * 0.14);
							float cameramoveX = cPlayer2D->pos.x - m_worldWidth * 0.5;

							if ((mousePos.x >= (i * 0.04 * m_worldWidth) + ((i - 1) * 0.28 * m_worldWidth) && mousePos.x <= (i * 0.04 * m_worldWidth) + ((i - 1) * 0.28 * m_worldWidth) + m_worldWidth * 0.28) &&
								(mousePos.y <= m_worldHeight * 0.73 && mousePos.y >= m_worldHeight * 0.17)) {

								cPlayer2D->increaseLevel();
								cPlayer2D->leveledUp = false;
								switch (levelUpgrades[i - 1]) {
								case pierce:
									pierceforbullet += 1;
									break;
								case atk:
									dmgofgun *= 1.1;
									break;
								case hp:
									cPlayer2D->IncreaseHP();
									break;
								case multishot:
									numberofbullets++;
									break;
								case moveSpeed:
									cPlayer2D->IncreaseSpd();
									MSUpgrade += 1;
									break;
								case velocity:
									velocityofbullet += 5;
									break;
								case fireRate:
									firerate *= 0.95;
									firerateUpgrade += 1;
									break;
								case dragon:
									if (Companion->mass == 1)
									{
										Companion->type = GameObject::GO_COMPANION;
										Companion->mass = 5;
										Companion->scale.Set(7, 7, 1);
										Companion->pos.Set(cPlayer2D->pos.x, cPlayer2D->pos.y, 1);
										Companion->vel.SetZero();
										timerfordragon = elapsedTime;
										Companion->bounce = true;
										Companion->damage = 10;
									}
									else
									{
										Companion->damage *= 1.1;
									}
									break;
								}
							}
						}
					}
				}
			}
			//pause
			else if (pause == true) {
				static bool LMPressed = false;
				if (Application::IsMousePressed(0) && !LMPressed) {
					LMPressed = true;
				}
				else if (!Application::IsMousePressed(0) && LMPressed) {
					LMPressed = false;
					float x = m_worldWidth * 0.5;
					if ((mousePos.x >= x - (m_worldWidth * 0.1) && mousePos.x <= x + (m_worldWidth * 0.1) &&
						mousePos.y <= m_worldHeight * 0.84 && mousePos.y >= m_worldHeight * 0.7)) {
						pause = false;

					}
					else if ((mousePos.x >= x - (m_worldWidth * 0.1) && mousePos.x <= x + (m_worldWidth * 0.1) && mousePos.y <= m_worldHeight * 0.67 && mousePos.y >= m_worldHeight * 0.53))
					{
						pause = false;
						currentState = lose;
					}
				}
			}
			else if (chestOpened == true) {
				chest(mousePos,dt);
			}
		}
		break;
	}
	case win:
	{
		static bool bLButtonState = false;
		if (!bLButtonState && Application::IsMousePressed(0)) {
			bLButtonState = true;
		}
		else if (bLButtonState && !Application::IsMousePressed(0)) {
			bLButtonState = false;
			if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.25 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.25) &&
				(mousePos.y <= (m_worldHeight * 0.6) + 6 && mousePos.y >= (m_worldHeight * 0.6) - 6)) 
			{
				cPlayer2D->IncreaseGold(acquiredGold* difficulty);
				reset();
				cSoundController->PlaySoundByID(2);
				SpawnMapObjects();
				currentState = difficultySelection;
			}
			else if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.25 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.25) &&
				(mousePos.y <= (m_worldHeight * 0.3) + 6 && mousePos.y >= (m_worldHeight * 0.3) - 6)) 
			{
				cPlayer2D->IncreaseGold(acquiredGold * difficulty);
				reset();
				cSoundController->PlaySoundByID(1);
				currentState = start;
			}
		}
		break;
	}
	case lose:
	{
		static bool bLButtonState = false;
		if (!bLButtonState && Application::IsMousePressed(0)) {
			bLButtonState = true;
		}
		else if (bLButtonState && !Application::IsMousePressed(0)) {
			bLButtonState = false;
			if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.25 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.25) && (mousePos.y <= (m_worldHeight * 0.6) + 7.5 && mousePos.y >= (m_worldHeight * 0.6) - 7.5)) {
				cPlayer2D->IncreaseGold(acquiredGold * difficulty);
				reset();
				cSoundController->PlaySoundByID(2);
				SpawnMapObjects();
				currentState = difficultySelection;
			}
			else if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.25 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.25) && (mousePos.y <= (m_worldHeight * 0.3) + 7.5 && mousePos.y >= (m_worldHeight * 0.3) - 7.5)) {
				cPlayer2D->IncreaseGold(acquiredGold * difficulty);
				reset();
				cSoundController->PlaySoundByID(1);
				currentState = start;
			}
		}
		break;
	}
	default:
		break;
	}
}

bool SceneCollision::CheckCollision(GameObject* go1, GameObject* go2)
{
	if (go1->type == GameObject::GO_WALL && go2->type == GameObject::GO_PILLAR)
		switch (go2->type) {
		case GameObject::GO_BOSS_SLIME:
		{
			Vector3 distdiff = go2->pos - go1->pos;
			Vector3 minimum = (0, 0, 1);
			if (distdiff.x <= minimum.x || distdiff.y <= minimum.y)
			{
				return false;
			}
		}
		break;
		default:
			return false;
			break;
		}
}

bool SceneCollision::CheckCollision(Enemy* enemy1, Enemy* enemy2, double dt)
{
	Vector3 disDiff = enemy2->pos - enemy1->pos;
	//check if they are near each other
	if (disDiff.Length() <= enemy1->scale.x + (enemy1->vel.x * dt) - 5) {
		//response
		//check if enemy1 is moving away from enemy 2
		if ((disDiff.x > 0 && enemy1->vel.x > 0) ||
			(disDiff.x < 0 && enemy1->vel.x < 0)) {
			enemy1->pos.x -= enemy1 ->vel.x * dt;
		}
		if ((disDiff.y > 0 && enemy1->vel.y > 0) ||
			(disDiff.y < 0 && enemy1->vel.y < 0)) {
			enemy1->pos.y -= enemy1->vel.y * dt;
		}
	}
	return 0;

}

bool SceneCollision::CheckCollision(Enemy* Enemy, CPlayer2D* cPlayer2D)
{
	Vector3 relativeVel = Enemy->vel - cPlayer2D->vel;
	Vector3 disDiff = cPlayer2D->pos - Enemy->pos;

	float playeroffset = -5;

	SpriteAnimation* pa = dynamic_cast<SpriteAnimation*>(meshList[GEO_PLAYER]);
	if (pa->getAnimationStatus("walkL") == false || pa->getAnimationStatus("idleL") == false) {
		playeroffset = 5;
	}

	if (cPlayer2D->pos.y < Enemy->pos.y)
	{
		disDiff -= Vector3(0, Enemy->scale.y / 2 + 5, 0);
	}
	else
	{
		disDiff += Vector3(0, Enemy->scale.y / 2 + 5, 0);
	}

	if (cPlayer2D->pos.x < Enemy->pos.x)
	{
		disDiff -= Vector3(Enemy->scale.x / 2 - playeroffset, 0, 0);
	}
	else
	{
		disDiff += Vector3(Enemy->scale.x / 2 - playeroffset, 0, 0);
	}

	if (relativeVel.Dot(disDiff) <= 0) {
		return false;
	}
	return disDiff.LengthSquared() <= (Enemy->scale.x + 10) * (Enemy->scale.x + 10);
}

void SceneCollision::CollisionResponse(GameObject * go1, GameObject * go2)
{
	u1 = go1->vel;
	u2 = go2->vel;
	m1 = go1->mass;
	m2 = go2->mass;

	switch (go2->type)
	{
	case GameObject::GO_BALL:
	{
		// 2D Version 2
		Vector3 n = go1->pos - go2->pos;
		Vector3 vec = (u1 - u2).Dot(n) / (n).LengthSquared() * n;
		go1->vel = u1 - (2 * m2 / (m1 + m2)) * vec;
		go2->vel = u2 - (2 * m1 / (m2 + m1)) * -vec;
		break;
	}
	case GameObject::GO_WALL:
	{
		if (go2->bounce == true) {

			Vector3 projectedVec = Vector3::project(go1->pos - go2->linestart, go2->lineEnd);
			float dist = projectedVec.Length();
			float force = Math::Clamp((go2->pos - go2->prevpos).Length(), 0.f, 5.f);
			Vector3 resultanVel = ((go1->pos - go2->linestart - projectedVec).Normalize() * dist * force);
			if (resultanVel.Length() < go1->vel.Length()) {
				go1->vel = -(resultanVel + (go1->vel));
			}
			else {
				go1->vel = (resultanVel + (go1->vel));
			}
			break;
		}
		go1->vel = u1 - (2.0 * u1.Dot(go2->normal)) * go2->normal;
		break;
	}
	case GameObject::GO_PILLAR:
	{
		if (go2->bounce == true) {
			Vector3 projectedVec = Vector3::project(go1->pos - go2->linestart, go2->lineEnd);
			float dist = projectedVec.Length() / (go1->pos - go2->linestart).Length();
			float force = Math::Clamp((go2->pos - go2->prevpos).Length(), 0.f, 5.f);
			Vector3 resultanVel = ((go1->pos - go2->linestart - projectedVec).Normalize() * dist * force);
			if (resultanVel.Length() < go1->vel.Length()) {
				go1->vel = -(resultanVel + (go1->vel));
			}
			else {
				go1->vel = (resultanVel + (go1->vel));
			}
			break;
		}
		Vector3 n = (go2->pos - go1->pos).Normalize();
		go1->vel = u1 - (2.f * u1.Dot(n)) * n;
		break;
	}
	case GameObject::GO_POWERUP:
		go2->active = false;
		break;
		//case GameObject::GO_BOSS_SLIME:
		//	Vector3 n = go1->pos - go2->pos;
		//	Vector3 vec = (u1 - u2).Dot(n) / (n).LengthSquared() * n;
		//	go1->vel = u1 - (2 * m2 / (m1 + m2)) * vec;
		//	go2->vel = u2 - (2 * m1 / (m2 + m1)) * -vec;
		//	break;
	}
}

void SceneCollision::CollisionResponse(Enemy * go1, Enemy * go2, double dt)
{

}

void SceneCollision::spawnPowerup(Vector3 pos)
{
	bool spawn = false;
	switch (Math::RandIntMinMax(0, 10)) {
	case 1:
	case 2:
	case 5:
	case 6:
		spawn = true;
		break;
	}
	if (spawn == true) {
		GameObject* go = FetchGO();
		go->type = GameObject::GO_POWERUP;
		go->scale.Set(5, 5, 1.0f);
		go->pos = pos;
		go->normal = Vector3(1, 0, 0);
		go->vel = (0, -2, 0);
		go->PUIFrame = 5.f;
		switch (Math::RandIntMinMax(0, 3)) {
		case 0:
			go->PU = GameObject::heal;
			break;
		case 1:
			go->PU = GameObject::rechargeUp;
			break;
		case 2:
			go->PU = GameObject::ballUp;
			break;
		case 3:
			go->PU = GameObject::extend;
			break;
		}
	}
}

void SceneCollision::RenderTitleScreen()
{
	double x, y, windowwidth, windowheight;
	Application::GetCursorPos(&x, &y);
	windowwidth = Application::GetWindowWidth();
	windowheight = Application::GetWindowHeight();
	Vector3 mousePos = Vector3((x / windowwidth) * m_worldWidth, ((windowheight - y) / windowheight) * m_worldHeight, 0);

	modelStack.PushMatrix();
	modelStack.Translate(m_worldWidth * 0.5, m_worldHeight * 0.7f, 0);
	modelStack.Scale(m_worldWidth * 0.9, m_worldHeight * 0.5, 0);
	RenderMesh(meshList[GEO_MENU], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(m_worldWidth / 2, m_worldHeight * 0.4, 1);
	modelStack.Scale(m_worldWidth * 0.4, 12.5, 1);
	RenderMesh(meshList[GEO_START], true);
	if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.2 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.2) && (mousePos.y <= (m_worldHeight * 0.4) + 4.75 && mousePos.y >= (m_worldHeight * 0.4) - 4.75))
		meshList[GEO_START]->material.kAmbient.Set(1, 1, 0);
	else
		meshList[GEO_START]->material.kAmbient.Set(1, 1, 1);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(m_worldWidth / 2, m_worldHeight * 0.25, 1);
	modelStack.Scale(m_worldWidth * 0.4, 12.5, 1);
	RenderMesh(meshList[GEO_SHOP], true);
	if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.075 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.075) && (mousePos.y <= (m_worldHeight * 0.25) + 4.75 && mousePos.y >= (m_worldHeight * 0.25) - 4.75))
		meshList[GEO_SHOP]->material.kAmbient.Set(1, 1, 0);
	else
		meshList[GEO_SHOP]->material.kAmbient.Set(1, 1, 1);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(m_worldWidth / 2, m_worldHeight * 0.1, 1);
	modelStack.Scale(m_worldWidth * 0.4, 12.5, 1);
	RenderMesh(meshList[GEO_QUIT], true);
	if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.17 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.17) && (mousePos.y <= (m_worldHeight * 0.1) + 4.75 && mousePos.y >= (m_worldHeight * 0.1) - 4.75))
		meshList[GEO_QUIT]->material.kAmbient.Set(1, 1, 0);
	else
		meshList[GEO_QUIT]->material.kAmbient.Set(1, 1, 1);
	modelStack.PopMatrix();
}
void SceneCollision::ShopInteraction(double dt)
{
	double x, y, windowwidth, windowheight;
	Application::GetCursorPos(&x, &y);
	windowwidth = Application::GetWindowWidth();
	windowheight = Application::GetWindowHeight();
	Vector3 mousePos = Vector3((x / windowwidth) * m_worldWidth, ((windowheight - y) / windowheight) * m_worldHeight, 0);
	static bool bLButtonState = false;

	if (bLButtonState == true)
		shopClick = 1;
	else
		shopClick = 0;

	if (!bLButtonState && Application::IsMousePressed(0))
	{
		if ((mousePos.x >= ((m_worldWidth / 3) * 2.4f) - m_worldWidth * 0.2 && mousePos.x <= (m_worldWidth / 3 * 2.4f) + m_worldWidth * 0.2) && (mousePos.y <= (m_worldHeight * 0.1) + 6.25 && mousePos.y >= (m_worldHeight * 0.1) - 6.25))
		{
			OutputDialogue = "";
			CurrentTextWrite = false, TextFinished = false;
			CurrentCharText = 0;
			randomDialogue = 0;

			for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
			{
				GameObject* go = (GameObject*)*it;
				if (go->active)
					if (go->type == GameObject::GO_GRONK)
						ReturnGO(go);
			}

			cSoundController->StopAllSound();
			cSoundController->PlaySoundByID(1);
			currentState = start;
		}

		else if ((mousePos.x >= (m_worldWidth * 0.25f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.25f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.45f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.45f)))
		{
			if (cPlayer2D->GetGold() < 500 * pow(1.75, ShopUpgrades[0]) || ShopUpgrades[0] >= 5)
			{
				cSoundController->StopPlayByID(7);
				cSoundController->PlaySoundByID(7);
			}

			else //Increase Speed
			{
				cSoundController->StopPlayByID(6);
				cSoundController->PlaySoundByID(6);
				cPlayer2D->IncreaseSpd();
				cPlayer2D->UseGold(200 * pow(1.1, ShopUpgrades[0]));
				ShopUpgrades[0] += 1;
			}
		}

		else if ((mousePos.x >= (m_worldWidth * 0.4f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.4f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.45f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.45f)))
		{
			if (cPlayer2D->GetGold() < 750 * pow(1.3, ShopUpgrades[1]) || ShopUpgrades[1] >= 10)
			{
				cSoundController->StopPlayByID(7);
				cSoundController->PlaySoundByID(7);
			}

			else //Increase Health
			{
				cSoundController->StopPlayByID(6);
				cSoundController->PlaySoundByID(6);
				cPlayer2D->IncreaseHP();
				cPlayer2D->UseGold(100 * pow(1.1, ShopUpgrades[1]));
				ShopUpgrades[1] += 1;
			}
		}

		else if ((mousePos.x >= (m_worldWidth * 0.55f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.55f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.45f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.45f)))
		{
			if (cPlayer2D->GetGold() < 600 * pow(2.0, ShopUpgrades[2]) || ShopUpgrades[2] >= 5)
			{
				cSoundController->StopPlayByID(7);
				cSoundController->PlaySoundByID(7);
			}

			else //Increase Defence
			{
				cSoundController->StopPlayByID(6);
				cSoundController->PlaySoundByID(6);
				cPlayer2D->DecreaseShieldCooldown();
				cPlayer2D->UseGold(150 * pow(2.0, ShopUpgrades[2]));
				ShopUpgrades[2] += 1;
			}
		}

		else if ((mousePos.x >= (m_worldWidth * 0.25f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.25f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.325f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.325f)))
		{
			if (cPlayer2D->GetGold() < 300 * pow(1.5, ShopUpgrades[3]) || ShopUpgrades[3] >= 4)
			{
				cSoundController->StopPlayByID(7);
				cSoundController->PlaySoundByID(7);
			}

			else //Increase Dmg
			{
				cSoundController->StopPlayByID(6);
				cSoundController->PlaySoundByID(6);
				cPlayer2D->IncreaseDmg();
				cPlayer2D->UseGold(100 * pow(1.5, ShopUpgrades[3]));
				ShopUpgrades[3] += 1;
			}
		}

		else if ((mousePos.x >= (m_worldWidth * 0.4f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.4f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.325f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.325f)))
		{
			if (cPlayer2D->GetGold() < 2000 || ShopUpgrades[4] >= 1)
			{
				cSoundController->StopPlayByID(7);
				cSoundController->PlaySoundByID(7);
			}

			else //Increase Lives count
			{
				cSoundController->StopPlayByID(6);
				cSoundController->PlaySoundByID(6);
				cPlayer2D->IncreaseLifeCount();
				cPlayer2D->UseGold(2000);
				ShopUpgrades[4] += 1;
			}

		}

		else if ((mousePos.x >= (m_worldWidth * 0.55f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.55f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.325f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.325f)))
		{
			if (cPlayer2D->GetGold() < 1250 * pow(1.2, ShopUpgrades[5]) || ShopUpgrades[5] >= 5)
			{
				cSoundController->StopPlayByID(7);
				cSoundController->PlaySoundByID(7);
			}

			else//Increase exp earned
			{
				cSoundController->StopPlayByID(6);
				cSoundController->PlaySoundByID(6);
				cPlayer2D->IncreaseEXPGain();
				cPlayer2D->UseGold(500 * pow(1.5, ShopUpgrades[5]));
				ShopUpgrades[5] += 1;
			}
		}

		bLButtonState = true;
	}

	else if (bLButtonState && !Application::IsMousePressed(0))
		bLButtonState = false;
}
void SceneCollision::ShopUI()
{
	double x, y, windowwidth, windowheight;
	Application::GetCursorPos(&x, &y);
	windowwidth = Application::GetWindowWidth();
	windowheight = Application::GetWindowHeight();
	Vector3 mousePos = Vector3((x / windowwidth) * m_worldWidth, ((windowheight - y) / windowheight) * m_worldHeight, 0);

	modelStack.PushMatrix();
	modelStack.Translate(m_worldWidth * 0.4, m_worldHeight * 0.45f, 4);
	modelStack.Scale(m_worldWidth * 0.6, m_worldHeight * 0.4, 0);
	RenderMesh(meshList[GEO_SHOP_UI_BG], false);
	modelStack.PopMatrix();

	int shopUpgrade;

	if ((mousePos.x >= (m_worldWidth * 0.25f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.25f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.45f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.45f)))
	{
		shopUpgrade = (500 * pow(1.75, ShopUpgrades[0]));
		if(ShopUpgrades[0] >= 5)
			RenderTextOnScreen(meshList[GEO_TEXT], std::to_string(ShopUpgrades[0]) + "/5 [Maxed]", Color(1, 1, 1), 2, 22.5, 36);
		else
			RenderTextOnScreen(meshList[GEO_TEXT], std::to_string(ShopUpgrades[0]) + "/5 [" + std::to_string(shopUpgrade) + "]", Color(1, 1, 1), 2, 22.5, 36);
	}
	else if ((mousePos.x >= (m_worldWidth * 0.4f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.4f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.45f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.45f)))
	{
		shopUpgrade = (750 * pow(1.3, ShopUpgrades[1]));
		if (ShopUpgrades[1] >= 10)
			RenderTextOnScreen(meshList[GEO_TEXT], std::to_string(ShopUpgrades[1]) + "/10 [Maxed]", Color(1, 1, 1), 2, 22.5, 36);
		else
			RenderTextOnScreen(meshList[GEO_TEXT], std::to_string(ShopUpgrades[1]) + "/10 [" + std::to_string(shopUpgrade) + "]", Color(1, 1, 1), 2, 22.5, 36);
	}
	else if ((mousePos.x >= (m_worldWidth * 0.55f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.55f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.45f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.45f)))
	{
		shopUpgrade = (600 * pow(2.0, ShopUpgrades[2]));
		if (ShopUpgrades[2] >= 5)
			RenderTextOnScreen(meshList[GEO_TEXT], std::to_string(ShopUpgrades[2]) + "/5 [Maxed]", Color(1, 1, 1), 2, 22.5, 36);
		else
			RenderTextOnScreen(meshList[GEO_TEXT], std::to_string(ShopUpgrades[2]) + "/5 [" + std::to_string(shopUpgrade) + "]", Color(1, 1, 1), 2, 22.5, 36);
	}
	else if ((mousePos.x >= (m_worldWidth * 0.25f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.25f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.325f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.325f)))
	{
		shopUpgrade = (300 * pow(1.5, ShopUpgrades[3]));
		if (ShopUpgrades[3] >= 4)
			RenderTextOnScreen(meshList[GEO_TEXT], std::to_string(ShopUpgrades[3]) + "/4 [Maxed]", Color(1, 1, 1), 2, 22.5, 36);
		else
			RenderTextOnScreen(meshList[GEO_TEXT], std::to_string(ShopUpgrades[3]) + "/4 [" + std::to_string(shopUpgrade) + "]", Color(1, 1, 1), 2, 22.5, 36);
	}
	else if ((mousePos.x >= (m_worldWidth * 0.4f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.4f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.325f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.325f)))
	{
		shopUpgrade = 2000;
		if (ShopUpgrades[4] >= 1)
			RenderTextOnScreen(meshList[GEO_TEXT], std::to_string(ShopUpgrades[4]) + "/1 [Maxed]", Color(1, 1, 1), 2, 22.5, 36);
		else
			RenderTextOnScreen(meshList[GEO_TEXT], std::to_string(ShopUpgrades[4]) + "/1 [" + std::to_string(shopUpgrade) + "]", Color(1, 1, 1), 2, 22.5, 36);
	}
	else if ((mousePos.x >= (m_worldWidth * 0.55f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.55f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.325f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.325f)))
	{
		shopUpgrade = 1250 * pow(1.2, ShopUpgrades[5]);
		if (ShopUpgrades[5] >= 5)
			RenderTextOnScreen(meshList[GEO_TEXT], std::to_string(ShopUpgrades[5]) + "/5 [Maxed]", Color(1, 1, 1), 2, 22.5, 36);
		else
			RenderTextOnScreen(meshList[GEO_TEXT], std::to_string(ShopUpgrades[5]) + "/5 [" + std::to_string(shopUpgrade) + "]", Color(1, 1, 1), 2, 22.5, 36);
	}
	for (int y = 0; y < 2; ++y)
	{
		for (int x = 0; x < 3; ++x)
		{
			modelStack.PushMatrix();

			modelStack.Translate(m_worldWidth * (0.25f + (0.15f * x)), m_worldHeight * (0.5f - (0.125f * y)), 5);
			modelStack.Scale(m_worldWidth * 0.1f, m_worldHeight * 0.1f, 0);

			if (x == 0 && y == 0)
				RenderMesh(meshList[GEO_SPEED_UPGRADE], false);
			else if (x == 1 && y == 0)
				RenderMesh(meshList[GEO_MAXHP_UPGRADE], false);
			else if (x == 2 && y == 0)
				RenderMesh(meshList[GEO_SHIELD_UPGRADE], false);
			else if (x == 0 && y == 1)
				RenderMesh(meshList[GEO_ATTACK_UPGRADE], false);
			else if (x == 1 && y == 1)
				RenderMesh(meshList[GEO_EXTRALIFE_UPGRADE], false);
			else if (x == 2 && y == 1)
				RenderMesh(meshList[GEO_EXPGAIN_UPGRADE], false);

			modelStack.PopMatrix();
		}
	}
}

void SceneCollision::RenderGronkDialogue()
{
	double x, y, windowwidth, windowheight;
	Application::GetCursorPos(&x, &y);
	windowwidth = Application::GetWindowWidth();
	windowheight = Application::GetWindowHeight();
	Vector3 mousePos = Vector3((x / windowwidth) * m_worldWidth, ((windowheight - y) / windowheight) * m_worldHeight, 0);
	static bool bLButtonState = false;

	//Random Dialogue
	string GronkDialogue[50] = { "Welcome to Gronk Shop!", "Gronk has many stuff!", "What human want? Gronk can help find!",
									"Human died " + std::to_string(DeathCount) + " times! Gronk thinks human should work harder.", "Human not dead, human die soon?", "Gronk give human hint! No hit mean safe!",
									"Gronk first time make shop, Gronk hope can get what Gronk want!", "Gronk no travel much, human travel a lot?",
									"This place dangerous for human, Gronk thinks human should leave.", "Human say Gronk is goblin? Gronk is Gronk, Gronk no Goblin! Human stupid.",
									"Human want to know how gronk get item? Because Gronk find item!",

		//Buying upgrade //11, 12, 14, 15, 15, 16
		"Speed is speed human! Walk faster much better, slow big no no.", "Human survive better, Human no want dead yet yes?",
		"Human want shield? Ah! Human no like pain pain, human scared yes?", "Human need better weapon? Gronk has better weapon! No use on Gronk okay?",
		"Gronk know this item! Gronk hear feather used a lot on dead", "Gronk hear this used to make Human stronger faster, Gronk no idea but recommend!",

		//Bought Upgrade //17, 18, 19, 20
		"Gronk thank human! Human help Gronk shop grow!", "Gronk will use money to make better shop!", "Gronk like money, money make Gronk happy",
		"Gronk make bigger shop soon, human will see!",

		//No Money // 21, 22, 23, 24
		"Human no money! No cheating Gronk!", "Human cannot buy item, no money, human poor!", "Gronk thinks human should kill kill before buy buy.",
		"No wasting Gronk time! Human get money first!",

		//Max upgrade // 25, 26, 27
		"Human already buy it all! Don't put Gronk out of cave, Gronk like cave!", "Gronk no more of item, pick another!",
		"Item hard to find human, Gronk no have item anymore" };

	modelStack.PushMatrix();
	modelStack.Translate(m_worldWidth * 0.3, m_worldHeight * 0.125f, 1);
	modelStack.Scale(m_worldWidth * 0.6, m_worldHeight * 0.25, 0);

	if (!bLButtonState && Application::IsMousePressed(0) && shopClick < 1)
	{
		//Change to random value
		if ((mousePos.x >= (m_worldWidth * 0.25f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.25f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.45f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.45f))) {
			OutputDialogue = "";
			CurrentCharText = 0;
			if (ShopUpgrades[0] < 5)
				MaxUpgrade[0] = false;
			if (MaxUpgrade[0] == true)
				randomDialogue = rand() % 3 + 25;
			else if (cPlayer2D->GetGold() > 500 * pow(1.75, ShopUpgrades[0]))
				randomDialogue = rand() % 4 + 17;
			else
				randomDialogue = rand() % 4 + 21;
			if (ShopUpgrades[0] >= 5)
				MaxUpgrade[0] = true;
			PlayerBuy = true;
			currentlyHovering = false;
		}
		else if ((mousePos.x >= (m_worldWidth * 0.4f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.4f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.45f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.45f))) {
			OutputDialogue = "";
			CurrentCharText = 0;
			if (ShopUpgrades[1] < 10)
				MaxUpgrade[1] = false;
			if (MaxUpgrade[1] == true)
				randomDialogue = rand() % 3 + 25;
			else if (cPlayer2D->GetGold() > 750 * pow(1.3, ShopUpgrades[1]))
				randomDialogue = rand() % 4 + 17;
			else
				randomDialogue = rand() % 4 + 21;
			if (ShopUpgrades[1] >= 10)
				MaxUpgrade[1] = true;
			PlayerBuy = true;
			currentlyHovering = false;
		}
		else if ((mousePos.x >= (m_worldWidth * 0.55f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.55f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.45f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.45f))) {
			OutputDialogue = "";
			CurrentCharText = 0;
			if (ShopUpgrades[2] < 5)
				MaxUpgrade[2] = false;
			if (MaxUpgrade[2] == true)
				randomDialogue = rand() % 3 + 25;
			else if (cPlayer2D->GetGold() > 600 * pow(2.0, ShopUpgrades[2]))
				randomDialogue = rand() % 4 + 17;
			else
				randomDialogue = rand() % 4 + 21;
			if (ShopUpgrades[2] >= 5)
				MaxUpgrade[2] = true;
			PlayerBuy = true;
			currentlyHovering = false;
		}
		else if ((mousePos.x >= (m_worldWidth * 0.25f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.25f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.325f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.325f))) {
			OutputDialogue = "";
			CurrentCharText = 0;
			if (ShopUpgrades[3] < 4)
				MaxUpgrade[3] = false;
			if (MaxUpgrade[3] == true)
				randomDialogue = rand() % 3 + 25;
			else if (cPlayer2D->GetGold() > 300 * pow(1.5, ShopUpgrades[3]))
				randomDialogue = rand() % 4 + 17;
			else
				randomDialogue = rand() % 4 + 21;
			if (ShopUpgrades[3] >= 4)
				MaxUpgrade[3] = true;
			PlayerBuy = true;
			currentlyHovering = false;
		}
		else if ((mousePos.x >= (m_worldWidth * 0.4f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.4f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.325f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.325f))) {
			OutputDialogue = "";
			CurrentCharText = 0;
			if (MaxUpgrade[4] == true)
				randomDialogue = rand() % 3 + 25;
			else if (cPlayer2D->GetGold() > 2000)
				randomDialogue = rand() % 4 + 17;
			else
				randomDialogue = rand() % 4 + 21;
			if (ShopUpgrades[4] >= 1)
				MaxUpgrade[4] = true;
			PlayerBuy = true;
			currentlyHovering = false;
		}
		else if ((mousePos.x >= (m_worldWidth * 0.55f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.55f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.325f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.325f))) {
			OutputDialogue = "";
			CurrentCharText = 0;
			if (ShopUpgrades[5] < 5)
				MaxUpgrade[5] = false;
			if (MaxUpgrade[5] == true)
				randomDialogue = rand() % 3 + 25;
			else if (cPlayer2D->GetGold() > 1250 * pow(1.2, ShopUpgrades[5]))
				randomDialogue = rand() % 4 + 17;
			else
				randomDialogue = rand() % 4 + 21;
			if (ShopUpgrades[5] >= 5)
				MaxUpgrade[5] = true;
			PlayerBuy = true;
			currentlyHovering = false;
		}

		bLButtonState = true;
	}
	else
		bLButtonState = false;

	if (!Application::IsMousePressed(0))
	{
		if ((mousePos.x >= (m_worldWidth * 0.25f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.25f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.45f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.45f)) && PlayerBuy == false) {
			if (randomDialogue <= 10 || randomDialogue != 11) {
				OutputDialogue = "";
				CurrentCharText = 0;
				CurrentTextWrite = false;
				TextFinished = false;
			}
			randomDialogue = 11;
			PlayerHover = true;
		}
		else if ((mousePos.x >= (m_worldWidth * 0.4f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.4f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.45f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.45f)) && PlayerBuy == false) {
			if ((randomDialogue <= 10 || randomDialogue != 12) && PlayerBuy == false) {
				OutputDialogue = "";
				CurrentCharText = 0;
				CurrentTextWrite = false;
				TextFinished = false;
			}
			randomDialogue = 12;
			PlayerHover = true;
		}
		else if ((mousePos.x >= (m_worldWidth * 0.55f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.55f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.45f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.45f)) && PlayerBuy == false) {
			if (randomDialogue <= 10 || randomDialogue != 13) {
				OutputDialogue = "";
				CurrentCharText = 0;
				CurrentTextWrite = false;
				TextFinished = false;
			}
			randomDialogue = 13;
			PlayerHover = true;
		}
		else if ((mousePos.x >= (m_worldWidth * 0.25f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.25f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.325f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.325f)) && PlayerBuy == false) {
			if (randomDialogue <= 10 || randomDialogue != 14) {
				OutputDialogue = "";
				CurrentCharText = 0;
				CurrentTextWrite = false;
				TextFinished = false;
			}
			randomDialogue = 14;
			PlayerHover = true;
		}
		else if ((mousePos.x >= (m_worldWidth * 0.4f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.4f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.325f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.325f)) && PlayerBuy == false) {
			if (randomDialogue <= 10 || randomDialogue != 15) {
				OutputDialogue = "";
				CurrentCharText = 0;
				CurrentTextWrite = false;
				TextFinished = false;
			}
			randomDialogue = 15;
			PlayerHover = true;
		}
		else if ((mousePos.x >= (m_worldWidth * 0.55f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.55f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.325f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.325f)) && PlayerBuy == false) {
			if (randomDialogue <= 10 || randomDialogue != 16) {
				OutputDialogue = "";
				CurrentCharText = 0;
				CurrentTextWrite = false;
				TextFinished = false;
			}
			randomDialogue = 16;
			PlayerHover = true;
		}
		else {
			PlayerHover = false;
			currentlyHovering = false;
		}
	}

	if ((PlayerHover == true && currentlyHovering != true) || PlayerBuy == true)
		CurrentTextWrite = true;

	if (CurrentTextWrite == true && TextFinished == false)
	{
		if (PlayerHover != true && PlayerBuy != true)
			randomDialogue = rand() % 11;

		if (randomDialogue == 3 && DeathCount <= 0)
		{
			randomDialogue = 4;
			OutputDialogue = "";
			CurrentCharText = 0;
			TextFinished = true;
		}
		else
		{
			OutputDialogue = "";
			CurrentCharText = 0;
			TextFinished = true;
		}
	}

	if (TextFinished == true)
	{
		char CurrentDialogue;

		if (dialogueTime > 0.05f)
		{
			if (CurrentCharText < GronkDialogue[randomDialogue].length())
			{
				int randSFX = rand() % 3 + 1;
				if(randSFX == 1)
					cSoundController->PlaySoundByID(19);
				else if(randSFX == 2)
					cSoundController->PlaySoundByID(20);
				else
					cSoundController->PlaySoundByID(21);
				CurrentDialogue = GronkDialogue[randomDialogue][CurrentCharText];
				OutputDialogue += CurrentDialogue;

				CurrentCharText += 1;
				dialogueTime = 0;
			}

			else
			{
				cSoundController->StopPlayByID(19);
				cSoundController->StopPlayByID(20);
				cSoundController->StopPlayByID(21);
				if (PlayerBuy != true && PlayerHover != true)
				{
					CurrentTextWrite = false;
					TextFinished = false;
				}

				else if (dialogueTime > 2.f)
				{
					if (PlayerHover == true && PlayerBuy != true)
						currentlyHovering = true;
					CurrentTextWrite = false;
					TextFinished = false;
					PlayerBuy = false;
				}
			}
		}
	}
	RenderDialogueOnScreen(meshList[GEO_TEXT], OutputDialogue, Color(0, 0, 0), 1.5, 2.5, 8);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(m_worldWidth * 0.3, m_worldHeight * 0.125f, 1);
	modelStack.Scale(m_worldWidth * 0.6, m_worldHeight * 0.25, 0);
	RenderTextOnScreen(meshList[GEO_TEXT], "Gronk", Color(0, 0.75f, 0), 2.5f, 9, 11);
	modelStack.PopMatrix();
}

void SceneCollision::SpawnMapObjects()
{
	int ObjectCount;
	ObjectCount = 0;
	int ObjectType;

	while (ObjectCount < 100)
	{
		ObjectType = rand() % 100 + 1; //Value between 1 and 100
		bool ObjectCollided;
		ObjectCollided = false;
		float CheckSpawn;
		CheckSpawn = 0;
		float SpawnX, SpawnY;
		Vector3 TreeSpawnPos;
		TreeSpawnPos = { 0, 0, 0 };

		unsigned size = m_goList.size();

		SpawnX = (-m_worldWidth * 2.4f) + static_cast<float>(rand()) * static_cast<float>((m_worldWidth * 2.4f) - (-m_worldWidth * 2.4f)) / RAND_MAX;
		SpawnY = (-m_worldHeight * 2.9f) + static_cast<float>(rand()) * static_cast<float>((m_worldHeight * 2.9f) - (-m_worldHeight * 2.9f)) / RAND_MAX;

		for (unsigned i = 0; i < size; ++i)
		{
			GameObject* go = m_goList[i];

			if (go->type == GameObject::GO_TREE)
			{
				float NegX = go->pos.x - 9;
				float PosX = go->pos.x + 9;
				float NegY = go->pos.y - 12;
				float PosY = go->pos.y + 7.5f;

				if (SpawnX + 9 >= NegX && SpawnX - 9 <= PosX)
				{
					if (SpawnY + 7.5f >= NegY && SpawnY - 12 <= PosY)
						ObjectCollided = true;
				}
				else if (SpawnY + 7.5f >= NegY && SpawnY - 12 <= PosY)
				{
					if (SpawnX + 9 >= NegX && SpawnX - 9 <= PosX)
						ObjectCollided = true;
				}
			}

			else if (go->type == GameObject::GO_ROCK)
			{
				float NegX = go->pos.x - 7.5f;
				float PosX = go->pos.x + 7.5f;
				float NegY = go->pos.y - 5.5f;
				float PosY = go->pos.y + 3.5f;

				if (SpawnX + 7.5f >= NegX && SpawnX - 7.5f <= PosX)
				{
					if (SpawnY + 3.5f >= NegY && SpawnY - 5.5f <= PosY)
						ObjectCollided = true;
				}
				else if (SpawnY + 3.5f >= NegY && SpawnY - 5.5f <= PosY)
				{
					if (SpawnX + 7.5f >= NegX && SpawnX - 7.5f <= PosX)
						ObjectCollided = true;
				}
			}

			else if (go->type == GameObject::GO_SUPERPAIN)
			{
				float NegX = go->pos.x - 9.f;
				float PosX = go->pos.x + 9.f;
				float NegY = go->pos.y - 7.5f;
				float PosY = go->pos.y + 7.5f;

				if (SpawnX + 9.f >= NegX && SpawnX - 9.f <= PosX)
				{
					if (SpawnY + 7.5f >= NegY && SpawnY - 7.5f <= PosY)
						ObjectCollided = true;
				}
				else if (SpawnY + 7.5f >= NegY && SpawnY - 7.5f <= PosY)
				{
					if (SpawnX + 9.f >= NegX && SpawnX - 9.f <= PosX)
						ObjectCollided = true;
				}
			}

			if (i == size - 1)
			{
				if (ObjectCount == 0)
				{
					if (ObjectType >= 1 && ObjectType <= 60) //Tree
					{
						GameObject* Tree = FetchGO();
						Tree->type = GameObject::GO_TREE;
						Tree->scale.Set(10, 20, 1.0f);
						Tree->pos.Set(SpawnX, SpawnY, 4.f);
						Tree->normal.Set(0, 1, 0);
						Tree->vel.SetZero();
					}

					else if (ObjectType > 60 && ObjectType <= 100) //Rock
					{
						GameObject* Tree = FetchGO();
						Tree->type = GameObject::GO_ROCK;
						Tree->scale.Set(5, 5, 1.0f);
						Tree->pos.Set(SpawnX, SpawnY, 4.f);
						Tree->normal.Set(0, 1, 0);
						Tree->vel.SetZero();
					}
				}

				else if (ObjectCount > 0)
				{
					if (ObjectCollided == false)
					{
						if (ObjectType >= 1 && ObjectType <= 55) //Tree
						{
							GameObject* Tree = FetchGO();
							Tree->type = GameObject::GO_TREE;
							Tree->scale.Set(10, 20, 1.0f);
							Tree->pos.Set(SpawnX, SpawnY, 4.f);
							Tree->normal.Set(0, 1, 0);
							Tree->vel.SetZero();
						}

						else if (ObjectType > 55 && ObjectType <= 95) //Rock
						{
							GameObject* Tree = FetchGO();
							Tree->type = GameObject::GO_ROCK;
							Tree->scale.Set(5, 5, 1.0f);
							Tree->pos.Set(SpawnX, SpawnY, 4.f);
							Tree->normal.Set(0, 1, 0);
							Tree->vel.SetZero();
						}

						else if (ObjectType > 95) //Destroy all enemy
						{
							GameObject* superPain = FetchGO();
							superPain->type = GameObject::GO_SUPERPAIN;
							superPain->scale.Set(10, 10, 1.0f);
							superPain->pos.Set(SpawnX, SpawnY, 4.f);
							superPain->normal.Set(0, 1, 0);
							superPain->vel.SetZero();
						}
					}
				}
				break;
			}
		}

		ObjectCount++;
	}
}

void SceneCollision::PlayerMapCheck()
{
	unsigned size = m_goList.size();

	//Player collision
	for (unsigned i = 0; i < size; ++i)
	{
		GameObject* go = m_goList[i];
		if (go->active)
		{
			if (go->type == GameObject::GO_TREE)
			{
				if (cPlayer2D->pos.x >= go->pos.x - 9 && cPlayer2D->pos.x <= go->pos.x + 9)
				{
					if (cPlayer2D->pos.y >= go->pos.y - 12 && cPlayer2D->pos.y <= go->pos.y + 7.5)
					{
						Vector3 TempPos;
						TempPos = { go->pos.x - 9, go->pos.y, go->pos.z };
						Vector3 NegativeX = TempPos - cPlayer2D->pos;
						TempPos = { go->pos.x + 9, go->pos.y, go->pos.z };
						Vector3 PositiveX = TempPos - cPlayer2D->pos;
						TempPos = { go->pos.x, go->pos.y - 12, go->pos.z };
						Vector3 NegativeY = TempPos - cPlayer2D->pos;
						TempPos = { go->pos.x, go->pos.y + 7.5f, go->pos.z };
						Vector3 PositiveY = TempPos - cPlayer2D->pos;

						if (PositiveX.Length() < NegativeX.Length() && PositiveX.Length() < NegativeY.Length() && PositiveX.Length() < PositiveY.Length())
							cPlayer2D->pos.x = Math::Clamp(cPlayer2D->pos.x, go->pos.x + 9, go->pos.x + 9);
						else if (NegativeX.Length() < PositiveX.Length() && NegativeX.Length() < NegativeY.Length() && NegativeX.Length() < PositiveY.Length())
							cPlayer2D->pos.x = Math::Clamp(cPlayer2D->pos.x, go->pos.x - 9, go->pos.x - 9);
						else if (PositiveY.Length() < NegativeX.Length() && PositiveY.Length() < NegativeY.Length() && PositiveY.Length() < PositiveX.Length())
							cPlayer2D->pos.y = Math::Clamp(cPlayer2D->pos.z, go->pos.y + 7.5f, go->pos.y + 7.5f);
						else if (NegativeY.Length() < NegativeX.Length() && NegativeY.Length() < PositiveX.Length() && NegativeY.Length() < PositiveY.Length())
							cPlayer2D->pos.y = Math::Clamp(cPlayer2D->pos.z, go->pos.y - 12, go->pos.y - 12);
					}
				}
			}

			if (go->type == GameObject::GO_ROCK)
			{
				if (cPlayer2D->pos.x >= go->pos.x - 7.5f && cPlayer2D->pos.x <= go->pos.x + 7.5f)
				{
					if (cPlayer2D->pos.y >= go->pos.y - 5.5f && cPlayer2D->pos.y <= go->pos.y + 3.5f)
					{
						Vector3 TempPos;
						TempPos = { go->pos.x - 7.5f, go->pos.y, go->pos.z };
						Vector3 NegativeX = TempPos - cPlayer2D->pos;
						TempPos = { go->pos.x + 7.5f, go->pos.y, go->pos.z };
						Vector3 PositiveX = TempPos - cPlayer2D->pos;
						TempPos = { go->pos.x, go->pos.y - 5.5f, go->pos.z };
						Vector3 NegativeY = TempPos - cPlayer2D->pos;
						TempPos = { go->pos.x, go->pos.y + 3.5f, go->pos.z };
						Vector3 PositiveY = TempPos - cPlayer2D->pos;

						if (PositiveX.Length() < NegativeX.Length() && PositiveX.Length() < NegativeY.Length() && PositiveX.Length() < PositiveY.Length())
							cPlayer2D->pos.x = Math::Clamp(cPlayer2D->pos.x, go->pos.x + 7.5f, go->pos.x + 7.5f);
						else if (NegativeX.Length() < PositiveX.Length() && NegativeX.Length() < NegativeY.Length() && NegativeX.Length() < PositiveY.Length())
							cPlayer2D->pos.x = Math::Clamp(cPlayer2D->pos.x, go->pos.x - 7.5f, go->pos.x - 7.5f);
						else if (PositiveY.Length() < NegativeX.Length() && PositiveY.Length() < NegativeY.Length() && PositiveY.Length() < PositiveX.Length())
							cPlayer2D->pos.y = Math::Clamp(cPlayer2D->pos.z, go->pos.y + 3.5f, go->pos.y + 3.5f);
						else if (NegativeY.Length() < NegativeX.Length() && NegativeY.Length() < PositiveX.Length() && NegativeY.Length() < PositiveY.Length())
							cPlayer2D->pos.y = Math::Clamp(cPlayer2D->pos.z, go->pos.y - 5.5f, go->pos.y - 5.5f);
					}
				}
			}

			if (go->type == GameObject::GO_SUPERPAIN)
			{
				if (cPlayer2D->pos.x >= go->pos.x - 9.f && cPlayer2D->pos.x <= go->pos.x + 9.f)
				{
					if (cPlayer2D->pos.y >= go->pos.y - 7.5f && cPlayer2D->pos.y <= go->pos.y + 7.5f)
					{
						Vector3 TempPos;
						TempPos = { go->pos.x - 9.f, go->pos.y, go->pos.z };
						Vector3 NegativeX = TempPos - cPlayer2D->pos;
						TempPos = { go->pos.x + 9.f, go->pos.y, go->pos.z };
						Vector3 PositiveX = TempPos - cPlayer2D->pos;
						TempPos = { go->pos.x, go->pos.y - 7.5f, go->pos.z };
						Vector3 NegativeY = TempPos - cPlayer2D->pos;
						TempPos = { go->pos.x, go->pos.y + 7.5f, go->pos.z };
						Vector3 PositiveY = TempPos - cPlayer2D->pos;

						if (PositiveX.Length() < NegativeX.Length() && PositiveX.Length() < NegativeY.Length() && PositiveX.Length() < PositiveY.Length()) 
						{
							SuperPainPower = true;
							ReturnGO(go);
							cSoundController->PlaySoundByID(16);
						}
						else if (NegativeX.Length() < PositiveX.Length() && NegativeX.Length() < NegativeY.Length() && NegativeX.Length() < PositiveY.Length())
						{
							SuperPainPower = true;
							ReturnGO(go);
							cSoundController->PlaySoundByID(16);
						}
						else if (PositiveY.Length() < NegativeX.Length() && PositiveY.Length() < NegativeY.Length() && PositiveY.Length() < PositiveX.Length())
						{
							SuperPainPower = true;
							ReturnGO(go);
							cSoundController->PlaySoundByID(16);
						}
						else if (NegativeY.Length() < NegativeX.Length() && NegativeY.Length() < PositiveX.Length() && NegativeY.Length() < PositiveY.Length())
						{
							SuperPainPower = true;
							ReturnGO(go);
							cSoundController->PlaySoundByID(16);
						}
					}
				}
			}
		}
	}
}

void SceneCollision::MapBoundary()
{
	if (cPlayer2D->pos.y >= m_worldHeight * 2.9f)
		cPlayer2D->pos.y = Math::Clamp(cPlayer2D->pos.y, m_worldHeight * 2.9f, m_worldHeight * 2.9f);
	if (cPlayer2D->pos.y <= -m_worldHeight * 2.9f)
		cPlayer2D->pos.y = Math::Clamp(cPlayer2D->pos.y, -m_worldHeight * 2.9f, -m_worldHeight * 2.9f);
	if (cPlayer2D->pos.x >= m_worldWidth * 2.4f)
		cPlayer2D->pos.x = Math::Clamp(cPlayer2D->pos.x, m_worldWidth * 2.4f, m_worldWidth * 2.4f);
	if (cPlayer2D->pos.x <= -m_worldWidth * 2.4f)
		cPlayer2D->pos.x = Math::Clamp(cPlayer2D->pos.x, -m_worldWidth * 2.4f, -m_worldWidth * 2.4f);
}

void SceneCollision::MakeScreenShake()
{
	screenShake[0] = -5 + static_cast<float>(rand()) * static_cast<float>(5 - -5) / RAND_MAX;
	screenShake[1] = -5 + static_cast<float>(rand()) * static_cast<float>(5 - -5) / RAND_MAX;
}
void SceneCollision::MakeScreenShakeForMG()
{
	screenShake[0] = -2 + static_cast<float>(rand()) * static_cast<float>(2 - -2) / RAND_MAX;
	screenShake[1] = -2 + static_cast<float>(rand()) * static_cast<float>(2 - -2) / RAND_MAX;
}

float SceneCollision::calculateAngle(float x, float y)
{
	float angle;
	if (x <= 0 && y <= 0) {
		angle = Math::RadianToDegree(atan(y / x)) + 180.0f;
	}
	else if (x < 0 && y > 0) {
		angle = Math::RadianToDegree(atan(y / x)) + 180.0f;
	}
	else if (x > 0 && y > 0) {
		angle = Math::RadianToDegree(atan(y / x));
	}
	else if (x > 0 && y < 0) {
		angle = 360 + Math::RadianToDegree(atan(y / x));
	}
	else {
		angle = 0;
	}
	return angle;
}
void SceneCollision::RenderGO(GameObject * go)
{
	//this is to render the animations for the mesh
	//In Update
	//sa = dynamic_cast<SpriteAnimation*>(meshList[GEO_BALL]);
	////Play the animation ???ROW1??? that is looping infinitely and
	////each animation completes in 2 sec
	//sa->PlayAnimation("ROW1", -1, 2.0f);
	//sa->Update(dt);
	zaxis += 0.01;
	switch (go->type)
	{
	case GameObject::GO_PILLAR:
	case GameObject::GO_BALL:
		//render normal asteroid
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zaxis);
		modelStack.Scale(go->scale.x, go->scale.y, 1);
		RenderMesh(meshList[GEO_BALL], false);
		modelStack.PopMatrix();
		//Exercise 11: think of a way to give balls different colors
		break;
	case GameObject::GO_TREE:
	{
		float Distance = cPlayer2D->pos.Length() - go->pos.Length();

		if (Distance < 0)
			Distance = -Distance;

		if (Distance > 75) {
			//Do nothing
		}
		else {
			modelStack.PushMatrix();
			modelStack.Translate(go->pos.x, go->pos.y, zaxis);
			modelStack.Scale(go->scale.x, go->scale.y, 1);
			RenderMesh(meshList[GEO_TREE], false);
			modelStack.PopMatrix();
		}
	}
	break;
	case GameObject::GO_ROCK:
	{
		float Distance = cPlayer2D->pos.Length() - go->pos.Length();

		if (Distance < 0)
			Distance = -Distance;

		if (Distance > 70) {
			//Do nothing
		}
		else {
			modelStack.PushMatrix();
			modelStack.Translate(go->pos.x, go->pos.y, zaxis);
			modelStack.Scale(go->scale.x, go->scale.y, 1);
			RenderMesh(meshList[GEO_ROCK], false);
			modelStack.PopMatrix();
		}
	}
	break;
	case GameObject::GO_SUPERPAIN:
	{
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zaxis);
		modelStack.Scale(go->scale.x, go->scale.y, 1);
		RenderMesh(meshList[GEO_SUPERPAIN], false);
		modelStack.PopMatrix();
	}
	break;
	case GameObject::GO_SKELETONATTACK:
	{
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zaxis);
		modelStack.Scale(go->scale.x, go->scale.y, 1);
		RenderMesh(meshList[GEO_SKELETONATTACK], false);
		modelStack.PopMatrix();
	}
	break;
	case GameObject::GO_COMPANION:
		modelStack.PushMatrix();
		modelStack.Translate(cPlayer2D->pos.x + companionX, cPlayer2D->pos.y + companionY, zaxis);
		modelStack.Scale(go->scale.x * 2.0, go->scale.y * 2.0, go->scale.z);
		RenderMesh(meshList[GEO_COMPANION], true);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_CHEST:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zaxis);
		modelStack.Scale(go->scale.x * 2.0, go->scale.y * 2.0, go->scale.z);
		RenderMesh(meshList[GEO_CHEST], true);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_BOW:
		modelStack.PushMatrix();
		modelStack.Translate(cPlayer2D->pos.x, cPlayer2D->pos.y, zaxis);
		modelStack.Rotate(go->angle, 0, 0, 1);
		modelStack.Scale(go->scale.x * 2.0, go->scale.y * 2.0, go->scale.z);
		RenderMesh(meshList[GEO_BOW], false);
		//meshList[GEO_BOW]->material.kAmbient.Set(1, 0, 0);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_GL:
		modelStack.PushMatrix();
		modelStack.Translate(cPlayer2D->pos.x, cPlayer2D->pos.y, zaxis);
		modelStack.Rotate(go->angle, 0, 0, 1);
		modelStack.Scale(go->scale.x * 2.0, go->scale.y * 2.0, go->scale.z);
		RenderMesh(meshList[GEO_GL], true);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_SHOTGUN:
		modelStack.PushMatrix();
		modelStack.Translate(cPlayer2D->pos.x, cPlayer2D->pos.y, zaxis);
		modelStack.Rotate(go->angle, 0, 0, 1);
		modelStack.Scale(go->scale.x * 2.0, go->scale.y * 2.0, go->scale.z);
		RenderMesh(meshList[GEO_SHOTGUN], true);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_SNIPER:
		modelStack.PushMatrix();
		modelStack.Translate(cPlayer2D->pos.x, cPlayer2D->pos.y, zaxis);
		modelStack.Rotate(go->angle, 0, 0, 1);
		modelStack.Scale(go->scale.x * 2.0, go->scale.y * 2.0, go->scale.z);
		RenderMesh(meshList[GEO_SNIPER], true);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_MACHINEGUN:
		modelStack.PushMatrix();
		modelStack.Translate(cPlayer2D->pos.x, cPlayer2D->pos.y, zaxis);
		modelStack.Rotate(go->angle, 0, 0, 1);
		modelStack.Scale(go->scale.x * 2.0, go->scale.y * 2.0, go->scale.z);
		RenderMesh(meshList[GEO_MACHINEGUN], true);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_PISTOL:
		modelStack.PushMatrix();
		modelStack.Translate(cPlayer2D->pos.x, cPlayer2D->pos.y, zaxis);
		modelStack.Rotate(go->angle, 0, 0, 1);
		modelStack.Scale(go->scale.x * 2.0, go->scale.y * 2.0, go->scale.z);
		RenderMesh(meshList[GEO_PISTOL], true);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_GRONK:
		modelStack.PushMatrix();
		modelStack.Translate((m_worldWidth / 3) * 2.5f, m_worldHeight * 0.4, zaxis);
		modelStack.Scale(20, 20, 1);
		RenderMesh(meshList[GEO_GRONK], true);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_WALL:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zaxis);
		modelStack.Rotate(Math::RadianToDegree(atan2f(go->normal.y, go->normal.x)), 0, 0, 1);
		//meshList[GEO_CUBE]->material.kAmbient(go->color.x, go->colora.y, go->color.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		if (go->thickWall > 0) {
			RenderMesh(meshList[GEO_WALL], false);
		}
		else if (go->bounce == true) {
			RenderMesh(meshList[GEO_BOUNCE], false);
		}
		else if (go->thinWall >= 0) {
			RenderMesh(meshList[GEO_THIN], false);
		}
		modelStack.PopMatrix();
		break;
	case GameObject::GO_POWERUP:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zaxis);
		modelStack.Rotate(Math::RadianToDegree(atan2f(go->normal.y, go->normal.x)), 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		switch (go->PU) {
		case GameObject::extend:
			RenderMesh(meshList[GEO_EXTEND], false);
			break;
		}
		modelStack.PopMatrix();
		break;
	case GameObject::GO_PROJECTILE:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zaxis);
		modelStack.Rotate(go->angle, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		switch (go->proj) {
		case GameObject::GL:
		case GameObject::pistol:
			RenderMesh(meshList[GEO_PISTOLBULLET], false);
			break;
		case GameObject::shotgun:
			RenderMesh(meshList[GEO_SHOTGUNBULLET], false);
			break;
		case GameObject::sniper:
			RenderMesh(meshList[GEO_SNIPERBULLET], false);
			break;
		case GameObject::bow:
			modelStack.Scale(1, 0.5, 1);
			RenderMesh(meshList[GEO_ARROW], false);
			break;
		case GameObject::machinegun:
			modelStack.Scale(0.5, 0.25, 1);
			RenderMesh(meshList[GEO_LMGBULLET], false);
			break;
		case GameObject::dragon:
			modelStack.Scale(1, 0.5, 1);
			RenderMesh(meshList[GEO_DRAGONFIRE], false);
			break;
		}

		modelStack.PopMatrix();
		break;
		//case GameObject::GO_BOSS_SLIME:
		//	modelStack.PushMatrix();
		//	modelStack.Translate(go->pos.x, go->pos.y, zaxis);
		//	modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		//	RenderMesh(meshList[GEO_BOSS_SLIME], false);
		//	modelStack.PopMatrix();
		//	break;
	case GameObject::GO_EXPLOSION:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, zaxis);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_EXPLOSION], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_SHIELD:
		if (go->visible)
		{
			modelStack.PushMatrix();
			modelStack.Translate(go->pos.x, go->pos.y, zaxis);
			modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
			RenderMesh(meshList[GEO_SHIELD], false);
			modelStack.PopMatrix();
		}
		break;
	}
}

void SceneCollision::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Projection matrix : Orthographic Projection
	Mtx44 projection;
	projection.SetToOrtho(0, m_worldWidth, 0, m_worldHeight, -10, 10);
	projectionStack.LoadMatrix(projection);


	// Camera matrix
	viewStack.LoadIdentity();
	viewStack.LookAt(
		camera.position.x, camera.position.y, camera.position.z,
		camera.target.x, camera.target.y, camera.target.z,
		camera.up.x, camera.up.y, camera.up.z
	);


	double x, y, windowwidth, windowheight;
	Application::GetCursorPos(&x, &y);
	windowwidth = Application::GetWindowWidth();
	windowheight = Application::GetWindowHeight();
	Vector3 mousePos = Vector3((x / windowwidth) * m_worldWidth, ((windowheight - y) / windowheight) * m_worldHeight, 0);
	static bool bLButtonState = false;

	// Model matrix : an identity matrix (model will be at the origin)
	modelStack.LoadIdentity();

	switch (currentState) {
	case start:
		RenderTitleScreen();
		break;
	case difficultySelection:
	{
		modelStack.PushMatrix();
		modelStack.Translate((m_worldWidth / 2) + camera.position.x, (m_worldHeight * 0.8) + camera.position.y, zaxis += 0.001f);
		modelStack.Scale(m_worldWidth * 0.6, m_worldHeight * 0.2, 1);
		RenderMesh(meshList[GEO_diffSelect], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate((m_worldWidth / 2) + camera.position.x, (m_worldHeight * 0.5) + camera.position.y, zaxis += 0.001f);
		modelStack.Scale(m_worldWidth * 0.2, m_worldHeight * 0.15, 1);
		meshList[GEO_DIFFICULTY]->textureID = LoadTexture("Image//easy.psd",true);
		RenderMesh(meshList[GEO_DIFFICULTY], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate((m_worldWidth / 2) + camera.position.x, (m_worldHeight * 0.3) + camera.position.y, zaxis += 0.001f);
		modelStack.Scale(m_worldWidth * 0.2, m_worldHeight * 0.15, 1);
		meshList[GEO_DIFFICULTY]->textureID = LoadTexture("Image//hard.psd",true);
		RenderMesh(meshList[GEO_DIFFICULTY], false);
		modelStack.PopMatrix();
	}
	break;
	case weaponselection:
	{
		float scalingthegun = m_worldWidth * 0.01;

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth * 0.2, m_worldHeight * 0.6, 0);
		modelStack.Scale(scalingthegun * 5, scalingthegun * 2, 0);
		RenderMesh(meshList[GEO_GL], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth * 0.5, m_worldHeight * 0.6, 0);
		modelStack.Scale(scalingthegun * 5, scalingthegun * 5, 0);
		RenderMesh(meshList[GEO_BOW], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth * 0.8, m_worldHeight * 0.6, 0);
		modelStack.Scale(scalingthegun * 5, scalingthegun * 2, 0);
		RenderMesh(meshList[GEO_SHOTGUN], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth * 0.4, m_worldHeight * 0.4, 0);
		modelStack.Scale(scalingthegun * 7, scalingthegun * 1.44565217391, 0);
		RenderMesh(meshList[GEO_SNIPER], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth * 0.6, m_worldHeight * 0.4, 0);
		modelStack.Scale(scalingthegun * 5, scalingthegun * 2, 0);
		RenderMesh(meshList[GEO_PISTOL], false);
		modelStack.PopMatrix();

		if (Transition == false)
		{
			if (!Application::IsMousePressed(0))
			{
				if ((mousePos.x >= (m_worldWidth * 0.2) - scalingthegun * 2.5 && mousePos.x <= (m_worldWidth * 0.2) + scalingthegun * 2.5) &&
					(mousePos.y <= (m_worldHeight * 0.6) + scalingthegun && mousePos.y >= (m_worldHeight * 0.6) - scalingthegun) ||
					(mousePos.x >= (m_worldWidth * 0.5) - scalingthegun * 2.5 && mousePos.x <= (m_worldWidth * 0.5) + scalingthegun * 2.5) &&
					(mousePos.y <= (m_worldHeight * 0.6) + scalingthegun * 2.5 && mousePos.y >= (m_worldHeight * 0.6) - scalingthegun * 2.5) ||
					(mousePos.x >= (m_worldWidth * 0.8) - scalingthegun * 2.5 && mousePos.x <= (m_worldWidth * 0.8) + scalingthegun * 2.5) &&
					(mousePos.y <= (m_worldHeight * 0.6) + scalingthegun && mousePos.y >= (m_worldHeight * 0.6) - scalingthegun) ||
					(mousePos.x >= (m_worldWidth * 0.4) - scalingthegun * 3.5 && mousePos.x <= (m_worldWidth * 0.4) + scalingthegun * 3.5) &&
					(mousePos.y <= (m_worldHeight * 0.4) + scalingthegun * 0.72282608695 && mousePos.y >= (m_worldHeight * 0.4) - scalingthegun * 0.72282608695) ||
					(mousePos.x >= (m_worldWidth * 0.6) - scalingthegun * 2.5 && mousePos.x <= (m_worldWidth * 0.6) + scalingthegun * 2.5) &&
					(mousePos.y <= (m_worldHeight * 0.4) + scalingthegun && mousePos.y >= (m_worldHeight * 0.4) - scalingthegun))
				{
					if (bLButtonState == false)
					{
						cSoundController->StopPlayByID(14);
						cSoundController->PlaySoundByID(14);
					}

					bLButtonState = true;

					modelStack.PushMatrix();
					modelStack.Translate(m_worldWidth * 0.5, m_worldHeight * 0.825, 1);
					modelStack.Scale(60, 30, 0);
					RenderMesh(meshList[GEO_STATPANEL], false);
					modelStack.PopMatrix();

					modelStack.PushMatrix();
					modelStack.Translate(m_worldWidth * 0.5, m_worldHeight * 0.14, 1);
					modelStack.Scale(90, 25, 0);
					RenderMesh(meshList[GEO_STATPANEL], false);
					modelStack.PopMatrix();
				}

				else {
					bLButtonState = false;
				}
				if ((mousePos.x >= (m_worldWidth * 0.2) - scalingthegun * 2.5 && mousePos.x <= (m_worldWidth * 0.2) + scalingthegun * 2.5) &&
					(mousePos.y <= (m_worldHeight * 0.6) + scalingthegun && mousePos.y >= (m_worldHeight * 0.6) - scalingthegun))
				{
					modelStack.PushMatrix();
					modelStack.Translate(m_worldWidth * 0.1925, m_worldHeight * 0.6, 1);
					modelStack.Scale(scalingthegun * 5, scalingthegun * 5, 0);
					RenderMesh(meshList[GEO_SELECTED], false);
					modelStack.PopMatrix();

					RenderTextOnScreen(meshList[GEO_TEXT], "Grenade Launcher", Color(1, 1, 0), 1.5, 28.5, 56);
					RenderTextOnScreen(meshList[GEO_TEXT], "Bullet Count: 1", Color(1, 1, 1), 1.5, 28.5, 51);
					RenderTextOnScreen(meshList[GEO_TEXT], "Damage: 3", Color(1, 1, 1), 1.5, 28.5, 48);
					RenderTextOnScreen(meshList[GEO_TEXT], "Piercing: 1", Color(1, 1, 1), 1.5, 28.5, 45);
					RenderTextOnScreen(meshList[GEO_TEXT], "Firerate: 1.2f", Color(1, 1, 1), 1.5, 28.5, 42);
					RenderTextOnScreen(meshList[GEO_TEXT], "Bullets explode on ", Color(1, 1, 1), 2, 22, 12);
					RenderTextOnScreen(meshList[GEO_TEXT], "impact but beware", Color(1, 1, 1), 2, 22, 8);
					RenderTextOnScreen(meshList[GEO_TEXT], "of self-harm", Color(1, 1, 1), 2, 22, 4);
				}

				else if ((mousePos.x >= (m_worldWidth * 0.5) - scalingthegun * 2.5 && mousePos.x <= (m_worldWidth * 0.5) + scalingthegun * 2.5) &&
					(mousePos.y <= (m_worldHeight * 0.6) + scalingthegun * 2.5 && mousePos.y >= (m_worldHeight * 0.6) - scalingthegun * 2.5))
				{
					modelStack.PushMatrix();
					modelStack.Translate(m_worldWidth * 0.5, m_worldHeight * 0.6, 1);
					modelStack.Scale(scalingthegun * 5, scalingthegun * 5, 0);
					RenderMesh(meshList[GEO_SELECTED], false);
					modelStack.PopMatrix();

					RenderTextOnScreen(meshList[GEO_TEXT], "Bow", Color(1, 1, 0), 1.5, 38, 56);
					RenderTextOnScreen(meshList[GEO_TEXT], "Arrow Count: 1", Color(1, 1, 1), 1.5, 28.5, 51);
					RenderTextOnScreen(meshList[GEO_TEXT], "Damage: 1", Color(1, 1, 1), 1.5, 28.5, 48);
					RenderTextOnScreen(meshList[GEO_TEXT], "Piercing: 1", Color(1, 1, 1), 1.5, 28.5, 45);
					RenderTextOnScreen(meshList[GEO_TEXT], "Firerate: 2.f", Color(1, 1, 1), 1.5, 28.5, 42);
					RenderTextOnScreen(meshList[GEO_TEXT], "Charge up for more", Color(1, 1, 1), 2, 22, 12);
					RenderTextOnScreen(meshList[GEO_TEXT], "damage but fire", Color(1, 1, 1), 2, 22, 8);
					RenderTextOnScreen(meshList[GEO_TEXT], "slower", Color(1, 1, 1), 2, 22, 4);
				}

				else if ((mousePos.x >= (m_worldWidth * 0.8) - scalingthegun * 2.5 && mousePos.x <= (m_worldWidth * 0.8) + scalingthegun * 2.5) &&
					(mousePos.y <= (m_worldHeight * 0.6) + scalingthegun && mousePos.y >= (m_worldHeight * 0.6) - scalingthegun))
				{
					modelStack.PushMatrix();
					modelStack.Translate(m_worldWidth * 0.7925, m_worldHeight * 0.6, 1);
					modelStack.Scale(scalingthegun * 5, scalingthegun * 5, 0);
					RenderMesh(meshList[GEO_SELECTED], false);
					modelStack.PopMatrix();

					RenderTextOnScreen(meshList[GEO_TEXT], "Shotgun", Color(1, 1, 0), 1.5, 35, 56);
					RenderTextOnScreen(meshList[GEO_TEXT], "Bullet Count: 4", Color(1, 1, 1), 1.5, 28.5, 51);
					RenderTextOnScreen(meshList[GEO_TEXT], "Damage: 3", Color(1, 1, 1), 1.5, 28.5, 48);
					RenderTextOnScreen(meshList[GEO_TEXT], "Piercing: 1", Color(1, 1, 1), 1.5, 28.5, 45);
					RenderTextOnScreen(meshList[GEO_TEXT], "Firerate: 1.2f", Color(1, 1, 1), 1.5, 28.5, 42);
					RenderTextOnScreen(meshList[GEO_TEXT], "Starts with multiple", Color(1, 1, 1), 1.5, 22, 12);
					RenderTextOnScreen(meshList[GEO_TEXT], "pallets and will ", Color(1, 1, 1), 1.5, 22, 8);
					RenderTextOnScreen(meshList[GEO_TEXT], "bounce instead of pierce", Color(1, 1, 1), 1.5, 22, 4);
				}

				else if ((mousePos.x >= (m_worldWidth * 0.4) - scalingthegun * 3.5 && mousePos.x <= (m_worldWidth * 0.4) + scalingthegun * 3.5) &&
					(mousePos.y <= (m_worldHeight * 0.4) + scalingthegun * 0.72282608695 && mousePos.y >= (m_worldHeight * 0.4) - scalingthegun * 0.72282608695))
				{
					modelStack.PushMatrix();
					modelStack.Translate(m_worldWidth * 0.39, m_worldHeight * 0.4, 1);
					modelStack.Scale(scalingthegun * 5, scalingthegun * 5, 0);
					RenderMesh(meshList[GEO_SELECTED], false);
					modelStack.PopMatrix();

					RenderTextOnScreen(meshList[GEO_TEXT], "Sniper", Color(1, 1, 0), 1.5, 36, 56);
					RenderTextOnScreen(meshList[GEO_TEXT], "Bullet Count: 1", Color(1, 1, 1), 1.5, 28.5, 51);
					RenderTextOnScreen(meshList[GEO_TEXT], "Damage: 10", Color(1, 1, 1), 1.5, 28.5, 48);
					RenderTextOnScreen(meshList[GEO_TEXT], "Piercing: 3", Color(1, 1, 1), 1.5, 28.5, 45);
					RenderTextOnScreen(meshList[GEO_TEXT], "Firerate: 1.5f", Color(1, 1, 1), 1.5, 28.5, 42);
					RenderTextOnScreen(meshList[GEO_TEXT], "Not for the light", Color(1, 1, 1), 2, 22, 12);
					RenderTextOnScreen(meshList[GEO_TEXT], "hearted, Spin2Win", Color(1, 1, 1), 2, 22, 8);
					RenderTextOnScreen(meshList[GEO_TEXT], "is the definition", Color(1, 1, 1), 2, 22, 4);
		
				}

				else if ((mousePos.x >= (m_worldWidth * 0.6) - scalingthegun * 2.5 && mousePos.x <= (m_worldWidth * 0.6) + scalingthegun * 2.5) &&
					(mousePos.y <= (m_worldHeight * 0.4) + scalingthegun && mousePos.y >= (m_worldHeight * 0.4) - scalingthegun))
				{
					modelStack.PushMatrix();
					modelStack.Translate(m_worldWidth * 0.5955, m_worldHeight * 0.4, 1);
					modelStack.Scale(scalingthegun * 5, scalingthegun * 5, 0);
					RenderMesh(meshList[GEO_SELECTED], false);
					modelStack.PopMatrix();

					RenderTextOnScreen(meshList[GEO_TEXT], "Revolver", Color(1, 1, 0), 1.5, 34.5, 56);
					RenderTextOnScreen(meshList[GEO_TEXT], "Bullet Count: 1", Color(1, 1, 1), 1.5, 28.5, 51);
					RenderTextOnScreen(meshList[GEO_TEXT], "Damage: 4", Color(1, 1, 1), 1.5, 28.5, 48);
					RenderTextOnScreen(meshList[GEO_TEXT], "Piercing: 1", Color(1, 1, 1), 1.5, 28.5, 45);
					RenderTextOnScreen(meshList[GEO_TEXT], "Firerate: 1.f", Color(1, 1, 1), 1.5, 28.5, 42);
					RenderTextOnScreen(meshList[GEO_TEXT], "A gun for the", Color(1, 1, 1), 1.9, 22, 12);
					RenderTextOnScreen(meshList[GEO_TEXT], "fastest in the west,", Color(1, 1, 1), 1.9, 22, 8);
					RenderTextOnScreen(meshList[GEO_TEXT], "aiming is overrated", Color(1, 1, 1), 1.9, 22, 4);
				}
			}
		}

		if (Transition == true)
		{
			if (elapsedTime <= 0.f)
				elapsedTime = 0.f;

			modelStack.PushMatrix();
			modelStack.Translate(m_worldWidth / 2, elapsedTime * m_worldHeight, 3);
			modelStack.Scale(200, 200, 1);
			RenderMesh(meshList[GEO_TRANSITION], false);
			modelStack.PopMatrix();
		}
		break;
	}
	case shop:
	{
		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth / 2, m_worldHeight * 0.5f, 0);
		modelStack.Scale(m_worldWidth, m_worldHeight, 0);
		RenderMesh(meshList[GEO_SHOP_BG], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth * 0.5, m_worldHeight * 0.9f, 1);
		modelStack.Scale(m_worldWidth * 0.6, m_worldHeight * 0.5, 0);
		RenderMesh(meshList[GEO_SHOP_SIGN], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth * 0.3, m_worldHeight * 0.125f, 1);
		modelStack.Scale(m_worldWidth * 0.6, m_worldHeight * 0.25, 0);
		RenderMesh(meshList[GEO_DIALOGUE_BOX], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth * 0.15, m_worldHeight * 0.95, 1.1f);
		modelStack.Scale(m_worldWidth * 0.35, m_worldHeight * 0.1, 0);
		RenderMesh(meshList[GEO_GOLD_BG], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth * 0.05, m_worldHeight * 0.95, 1.2);
		modelStack.Scale(m_worldWidth * 0.05, m_worldHeight * 0.05, 0);
		RenderMesh(meshList[GEO_GOLD], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth * 0.3, m_worldHeight * 0.125f, 1.3);
		modelStack.Scale(m_worldWidth * 0.6, m_worldHeight * 0.25, 0);
		RenderTextOnScreen(meshList[GEO_TEXT], "X" + std::to_string(cPlayer2D->GetGold()), Color(1, 1, 0), 2.5, 7.5, 55.75);
		modelStack.PopMatrix();

		RenderGronkDialogue();

		modelStack.PushMatrix();
		modelStack.Translate((m_worldWidth / 3) * 2.4f, m_worldHeight * 0.1, 2);
		modelStack.Scale(m_worldWidth * 0.4, 12.5, 1);
		RenderMesh(meshList[GEO_GRONK_BACK_BUTTON], true);
		if ((mousePos.x >= ((m_worldWidth / 3) * 2.4f) - m_worldWidth * 0.2 && mousePos.x <= (m_worldWidth / 3 * 2.4f) + m_worldWidth * 0.2) && (mousePos.y <= (m_worldHeight * 0.1) + 6.25 && mousePos.y >= (m_worldHeight * 0.1) - 6.25))
			meshList[GEO_GRONK_BACK_BUTTON]->material.kAmbient.Set(1, 1, 1);
		else
			meshList[GEO_GRONK_BACK_BUTTON]->material.kAmbient.Set(0.4, 0.4, 0.4);
		modelStack.PopMatrix();

		ShopUI();

		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (go->active)
				if (go->type == GameObject::GO_GRONK)
					RenderGO(go);
		}
		break;
	}
	case main:
	{
		//Render Background
		for (int x = 1; x <= 5; ++x)
		{
			for (int y = 1; y <= 6; ++y)
			{
				modelStack.PushMatrix();
				modelStack.Translate((m_worldWidth / 2) * x, (m_worldHeight * 0.5f) * y, 0.5f);
				modelStack.Scale(m_worldWidth, m_worldHeight, 1);
				RenderMesh(meshList[GEO_BG], false);
				modelStack.PopMatrix();
			}

			for (int y = -1; y >= -6; --y)
			{
				modelStack.PushMatrix();
				modelStack.Translate((m_worldWidth / 2) * x, (m_worldHeight * 0.5f) * y, 0.5f);
				modelStack.Scale(m_worldWidth, m_worldHeight, 1);
				RenderMesh(meshList[GEO_BG], false);
				modelStack.PopMatrix();
			}
		}
		for (int x = -1; x >= -5; --x)
		{
			for (int y = -1; y >= -6; --y)
			{
				modelStack.PushMatrix();
				modelStack.Translate((m_worldWidth / 2) * x, (m_worldHeight * 0.5f) * y, 0.5f);
				modelStack.Scale(m_worldWidth, m_worldHeight, 1);
				RenderMesh(meshList[GEO_BG], false);
				modelStack.PopMatrix();
			}

			for (int y = 1; y <= 6; ++y)
			{
				modelStack.PushMatrix();
				modelStack.Translate((m_worldWidth / 2) * x, (m_worldHeight * 0.5f) * y, 0.5f);
				modelStack.Scale(m_worldWidth, m_worldHeight, 1);
				RenderMesh(meshList[GEO_BG], false);
				modelStack.PopMatrix();
			}
		}

		//Render Boundary
		float RenderDistance;
		RenderDistance = 0;
		//render bottom
		for (int y = 0; y < 1; ++y)
		{
			for (int x = -59; x < 40; ++x)
			{
				RenderDistance = cPlayer2D->pos.Length() - Vector3((m_worldWidth / 2) + (9.6f * x), (m_worldHeight * 3.05) + (9.8 * y), zaxis).Length();

				if (RenderDistance < 0)
					RenderDistance = -RenderDistance;

				if (RenderDistance < 100)
				{
					modelStack.PushMatrix();
					modelStack.Translate((m_worldWidth / 2) + (((m_worldWidth * 4.95) / 99) * x), (m_worldHeight * 3.05), zaxis);
					modelStack.Scale((m_worldWidth * 4.95) / 99, (m_worldHeight * 6.175) / 75, 10);
					RenderMesh(meshList[GEO_BOUNDARY], false);
					modelStack.PopMatrix();
				}
			}
		}
		//render top
		for (int y = 0; y < 1; ++y)
		{
			for (int x = -59; x < 40; ++x)
			{
				RenderDistance = cPlayer2D->pos.Length() - Vector3((m_worldWidth / 2) + (9.6f * x), (m_worldHeight * -3.05) - (9.8 * y), zaxis).Length();

				if (RenderDistance < 0)
					RenderDistance = -RenderDistance;

				if (RenderDistance < 100)
				{
					modelStack.PushMatrix();
					modelStack.Translate((m_worldWidth / 2) + (((m_worldWidth * 4.95) / 99) * x), -(m_worldHeight * 3.05), zaxis);
					modelStack.Scale((m_worldWidth * 4.95) / 99, (m_worldHeight * 6.175) / 75, 10);
					RenderMesh(meshList[GEO_BOUNDARY], false);
					modelStack.PopMatrix();
				}
			}
		}
		//render left
		for (int x = 0; x < 1; ++x)
		{
			for (int y = -43; y < 32; ++y)
			{
				RenderDistance = cPlayer2D->pos.Length() - Vector3((m_worldWidth * -2.55) - (9.6f * x), (m_worldHeight / 2) + (9.8 * y), zaxis).Length();

				if (RenderDistance < 0)
					RenderDistance = -RenderDistance;

				if (RenderDistance < 100)
				{
					modelStack.PushMatrix();
					modelStack.Translate(-(m_worldWidth * 2.5), (m_worldHeight / 2) + (((m_worldHeight * 6.175) / 75) * y), zaxis);
					modelStack.Scale((m_worldWidth * 4.95) / 99, (m_worldHeight * 6.175) / 75, 10);
					RenderMesh(meshList[GEO_BOUNDARY], false);
					modelStack.PopMatrix();
				}
			}
		}
		//render right
		for (int x = 0; x < 1; ++x)
		{
			for (int y = -43; y < 32; ++y)
			{
				RenderDistance = cPlayer2D->pos.Length() - Vector3((m_worldWidth * 2.55) + (9.6f * x), (m_worldHeight / 2) + (9.8 * y), zaxis).Length();

				if (RenderDistance < 0)
					RenderDistance = -RenderDistance;

				if (RenderDistance < 100)
				{
					modelStack.PushMatrix();
					modelStack.Translate((m_worldWidth * 2.5), (m_worldHeight / 2) + (((m_worldHeight * 6.175) / 75) * y), zaxis);
					modelStack.Scale((m_worldWidth * 4.95) / 99, (m_worldHeight * 6.175) / 75, 10);
					RenderMesh(meshList[GEO_BOUNDARY], false);
					modelStack.PopMatrix();
				}
			}
		}

		modelStack.PushMatrix();
		modelStack.Translate(cPlayer2D->pos.x, cPlayer2D->pos.y, 1.0001f);
		modelStack.Scale(10, 10, 1);
		RenderMesh(meshList[GEO_PLAYER], false);
		modelStack.PopMatrix();

		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (go->active)
			{
				RenderGO(go);
			}
		}
		for (std::vector<Enemy*>::iterator it = enemyList.begin(); it != enemyList.end(); ++it)
		{
			Enemy* go = (Enemy*)*it;

			float Distance = cPlayer2D->pos.Length() - go->pos.Length();

			if (Distance < 0)
				Distance = -Distance;

			if (Distance > 100) {
				//Do nothing
			}
			else {
				modelStack.PushMatrix();
				modelStack.Translate(go->pos.x, go->pos.y, zaxis += 0.001f);
				modelStack.Scale(go->scale.x, go->scale.y, 1);
				RenderMesh(go->meshList[go->CurrEnemyType], false); //here Zhi Kai
				modelStack.PopMatrix();
			}
		}

		modelStack.PushMatrix();
		modelStack.Translate((m_worldWidth/2) + camera.position.x , (m_worldHeight*0.1)+ camera.position.y ,zaxis += 0.001f);
		modelStack.Scale(m_worldWidth*0.3, 3, 1);
		meshList[GEO_ROLLBAR]->material.kAmbient.Set(1, 0, 0);
		RenderMesh(meshList[GEO_ROLLBAR], true);
		modelStack.PopMatrix();

		float rollScaleX =Math::Clamp(Math::Min(cPlayer2D->rollCooldown / cPlayer2D->maxRollCooldown, 1.f),0.01f, 1.f);

		modelStack.PushMatrix();
		modelStack.Translate((m_worldWidth / 2) + camera.position.x, (m_worldHeight * 0.1) + camera.position.y, zaxis += 0.001f);
		modelStack.Scale(m_worldWidth * 0.3 * rollScaleX, 3, 1);
		meshList[GEO_ROLLBAR]->material.kAmbient.Set(0,1, 0);
		RenderMesh(meshList[GEO_ROLLBAR], true);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate((m_worldWidth / 2) + camera.position.x, (m_worldHeight * 0.1) + camera.position.y, zaxis += 0.001f);
		modelStack.Scale(m_worldWidth * 0.1, 3, 1);
		RenderMesh(meshList[GEO_ROLL], true);
		modelStack.PopMatrix();

		for (int i = 0; i < timerfordmgnumber.size(); ++i)
		{
			if (elapsedTime < timerfordmgnumber[i].x)
			{
				SceneCollision::RenderDmgNum(dmgandtimefordmgnumber[i], timerfordmgnumber[i].y);
			}
		}

		if (Gun->type == GameObject::GO_SNIPER)
		{
			int angle = Gun->thickWall;
			std::ostringstream ss;
			ss << angle; //360 feature
			if (Gun->thickWall < 90)
				RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 3, mousePos.x / m_worldWidth * 80, mousePos.y / m_worldHeight * 60);
			else if (Gun->thickWall < 180)
				RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0.7, 0, 1), 3, mousePos.x / m_worldWidth * 80, mousePos.y / m_worldHeight * 60);
			else if (Gun->thickWall < 270)
				RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 1), 3, mousePos.x / m_worldWidth * 80, mousePos.y / m_worldHeight * 60);
			else
				RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 3, mousePos.x / m_worldWidth * 80, mousePos.y / m_worldHeight * 60);
		}


		float expX = cPlayer2D->pos.x, expY = cPlayer2D->pos.y - (m_worldHeight * 0.45);
		float expScaleX = m_worldWidth * 0.95, expScaleY = 2;
		modelStack.PushMatrix();
		modelStack.Translate((m_worldWidth / 2) + camera.position.x, (m_worldHeight * 0.05) + camera.position.y, zaxis += 0.001f);
		modelStack.Scale(expScaleX, expScaleY, 1);
		RenderMesh(meshList[GEO_EXPBG], false);
		modelStack.PopMatrix();


		expScaleX = Math::Min((float)(m_worldWidth * 0.75), m_worldWidth * (float)0.75 * (cPlayer2D->xp / ((cPlayer2D->getLevel() - 1) * 10 + 5)));

		modelStack.PushMatrix();
		modelStack.Translate(expScaleX / 2 + m_worldWidth * 0.12 + camera.position.x, (m_worldHeight * 0.05) + camera.position.y, zaxis += 0.001f);
		modelStack.Scale(expScaleX, expScaleY, 1);
		RenderMesh(meshList[GEO_EXP], false);
		modelStack.PopMatrix();

		//hp
		float hpX = m_worldWidth * 0.16 + camera.position.x, hpY = m_worldHeight * 0.8 + camera.position.y;
		float hpScaleX = m_worldWidth * 0.3, hpScaleY = 10;
		modelStack.PushMatrix();
		modelStack.Translate(hpX, hpY, zaxis += 0.001f);
		modelStack.Scale(hpScaleX, hpScaleY, 1);
		RenderMesh(meshList[GEO_HEALTHBG], false);
		modelStack.PopMatrix();

		hpScaleX = Math::Min(m_worldWidth * 0.3 * 0.73684210526, m_worldWidth * 0.3 * 0.73684210526 * (cPlayer2D->hp / cPlayer2D->maxHP));

		modelStack.PushMatrix();
		modelStack.Translate(hpScaleX / 2 + hpX - m_worldWidth * 0.075, hpY, zaxis += 0.001f);
		modelStack.Scale(hpScaleX, hpScaleY, 1);
		RenderMesh(meshList[GEO_HEALTH], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth * 0.045 + camera.position.x, camera.position.y + m_worldHeight * 0.7, zaxis += 0.001f);
		modelStack.Scale(m_worldWidth * 0.02, m_worldHeight * 0.02, 0);
		RenderMesh(meshList[GEO_GOLD], false);
		modelStack.PopMatrix();

		if (ArrowToBoss->visible)
		{
			modelStack.PushMatrix();
			modelStack.Translate(ArrowToBoss->pos.x, ArrowToBoss->pos.y, zaxis += 0.001f);
			modelStack.Rotate(ArrowToBoss->angle, 0, 0, 1);
			modelStack.Scale(ArrowToBoss->scale.x, ArrowToBoss->scale.y, 1);
			RenderMesh(meshList[GEO_WHEREBOSS], false);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(ArrowInsideArrow->pos.x, ArrowInsideArrow->pos.y, zaxis += 0.001f);
			modelStack.Rotate(ArrowToBoss->angle, 0, 0, 1);
			modelStack.Scale(ArrowInsideArrow->scale.x, ArrowInsideArrow->scale.y, 1);
			RenderMesh(meshList[GEO_ARROWTOBOSS], false);
			modelStack.PopMatrix();
		}

		//On screen text
		std::ostringstream ss;
		ss.precision(5);
		ss << "FPS: " << fps;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 3, 0, 3);

		ss.str("");
		ss.precision(1);
		ss << "Gold Earned: " << acquiredGold;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 0), 2, 7, 41);

		if (BossKilled < 3) {
			ss.str("");
			ss.precision(1);
			ss << "Kills:" << killcounter << "/100";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 1.5, 2, 38);
			ss.str("");
			ss << "boss Killed:" << BossKilled << "/3";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 1.5, 2, 35);
		}
		else if(surviveSeconds > 0){
			ss.str("");
			ss.precision(2);
			ss << "Survive:" << surviveSeconds;
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 1.5, 2, 38);
		}
		else {
			ss.str("");
			ss << "Enter the portal";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 1.5, 2, 38);
		}
		ss.str("");
		ss.precision(2);
		ss << minutes << ":" << seconds;
		if (seconds < 10) {
			ss.precision(1);
			ss.str("");
			ss << minutes << ":0" << seconds;
		}
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 3, 3, 56);
		
		if (surviveSeconds <= 0) {
			modelStack.PushMatrix();
			modelStack.Translate(m_worldWidth* 0.5, m_worldHeight * 0.5 , zaxis += 1);
			modelStack.Scale(5, 10, 1);
			RenderMesh(meshList[GEO_PORTAL], false);
			modelStack.PopMatrix();
		}
		//render if player has leveled up
		if (cPlayer2D->leveledUp == true && cPlayer2D->xpToLevel() == true) {
			modelStack.PushMatrix();
			modelStack.Translate(camera.position.x, camera.position.y, zaxis += 0.001f);
			modelStack.Scale(1000, 1000, 1);
			RenderMesh(meshList[GEO_LVLUPBG], false);
			modelStack.PopMatrix();
			float cameramoveX = cPlayer2D->pos.x - m_worldWidth * 0.5;
			float cameramoveY = cPlayer2D->pos.y - m_worldHeight * 0.5;

			modelStack.PushMatrix();
			modelStack.Translate((m_worldWidth / 2) + cameramoveX, (m_worldHeight * 0.9) + cameramoveY, zaxis += 0.001f);
			modelStack.Scale(m_worldWidth * 0.6, m_worldHeight * 0.1, 1);
			RenderMesh(meshList[GEO_UPGRADESELECT], false);
			modelStack.PopMatrix();

			for (int i = 1; i < 4; ++i) {
				float x = (i * 0.04 * m_worldWidth) + ((i - 1) * 0.28 * m_worldWidth) + (m_worldWidth * 0.14);
				float textx;
				Vector3 color = Vector3(1, 0, 0);
				switch (i) {
				case 1:
					textx = 14;
					break;
				case 2:
					textx = 40;
					break;
				case 3:
					textx = 65;
					break;
				}
				modelStack.PushMatrix();
				modelStack.Translate(x + cameramoveX, m_worldHeight * 0.45 + cameramoveY, 6.01);
				modelStack.Scale(m_worldWidth * 0.28, m_worldHeight * 0.56, 1);
				if ((mousePos.x >= (i * 0.04 * m_worldWidth) + ((i - 1) * 0.28 * m_worldWidth) && mousePos.x <= (i * 0.04 * m_worldWidth) + ((i - 1) * 0.28 * m_worldWidth) + m_worldWidth * 0.28) &&
					(mousePos.y <= m_worldHeight * 0.73 && mousePos.y >= m_worldHeight * 0.17)) {
					meshList[GEO_CARD]->material.kAmbient.Set(1, 1, 0);
				}
				else {
					meshList[GEO_CARD]->material.kAmbient.Set(1, 1, 1);
				}
				RenderMesh(meshList[GEO_CARD], true);
				modelStack.PopMatrix();

				modelStack.PushMatrix();
				modelStack.Translate(x + cameramoveX, m_worldHeight * 0.45 + cameramoveY, 6.02);
				modelStack.Scale(m_worldWidth * 0.28, m_worldHeight * 0.56, 1);
				ss.str("");

				switch (levelUpgrades[i - 1]) {
				case pierce:
					RenderMesh(meshList[GEO_PIERCE], false);
					break;
				case atk:
					RenderMesh(meshList[GEO_ATKUP], false);
					break;
				case hp:
					RenderMesh(meshList[GEO_HPUP], false);
					break;
				case multishot:
					RenderMesh(meshList[GEO_MULTISHOT], false);
					break;
				case moveSpeed:
					RenderMesh(meshList[GEO_MOVESPEED], false);
					ss << MSUpgrade << "/5";
					RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 1, textx - 1, 20);
					break;
				case velocity:
					RenderMesh(meshList[GEO_VELUP], false);
					break;
				case fireRate:
					RenderMesh(meshList[GEO_FIRERATE], false);
					ss << firerateUpgrade << "/5";
					RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 1, textx - 1, 20);
					break;
				case dragon:
					if (Companion->mass == 1) {
						//upgrade to spawn dragon
						RenderMesh(meshList[GEO_COMPANIONUPGRADE], false);
					}
					else {
						//increases dragon damage
						RenderMesh(meshList[GEO_COMPANIONDAMAGE], false);
					}
				}
				modelStack.PopMatrix();
			}
		}
		else if (pause == true) {
			modelStack.PushMatrix();
			modelStack.Translate(camera.position.x, camera.position.y, 4.3f);
			modelStack.Scale(1000, 1000, 1);
			RenderMesh(meshList[GEO_LVLUPBG], false);
			modelStack.PopMatrix();

			float x = m_worldWidth * 0.5;
			float cameramoveX = cPlayer2D->pos.x - m_worldWidth * 0.5;
			float cameramoveY = cPlayer2D->pos.y - m_worldHeight * 0.5;

			modelStack.PushMatrix();
			modelStack.Translate(x + cameramoveX, m_worldHeight * 0.7 + cameramoveY, 4.4f);
			modelStack.Scale(m_worldWidth * 0.3, m_worldHeight * 0.5, 1);
			RenderMesh(meshList[GEO_PAUSEPANEL], false);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(x + cameramoveX, m_worldHeight * 0.77 + cameramoveY, 4.41f);
			modelStack.Scale(m_worldWidth * 0.2, m_worldHeight * 0.14, 1);
			RenderMesh(meshList[GEO_PAUSERESUME], false);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(x + cameramoveX, m_worldHeight * 0.60 + cameramoveY, 4.42f);
			modelStack.Scale(m_worldWidth * 0.2, m_worldHeight * 0.14, 1);
			RenderMesh(meshList[GEO_PAUSEQUIT], false);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(x + cameramoveX, m_worldHeight * 0.2 + cameramoveY, 4.42f);
			modelStack.Scale(m_worldWidth * 0.8, m_worldHeight * 0.2, 1);
			RenderMesh(meshList[GEO_STATPANEL], false);
			modelStack.PopMatrix();

			ss.str("");
			ss.precision(4);
			if (Gun->type == GameObject::GO_BOW) {
				ss << "dmg:" << dmgofgun * 12;
			}
			else
				ss << "dmg:" << dmgofgun;
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2, 11, 14);

			ss.str("");
			if (Gun->type == GameObject::GO_BOW) {
				ss << "vel:" << velocityofbullet;
			}
			else
				ss << "vel:" << velocityofbullet * 6;
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2, 11, 11);

			ss.str("");
			ss.precision(2);
			if (Gun->type == GameObject::GO_BOW) {
				ss << "RoF:" << firerate << "s";
			}
			else
				ss << "RoF:" << firerate << "s";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2, 11, 8);

			ss.str("");
			ss << "hp:" << cPlayer2D->maxHP;
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2, 29, 14);

			ss.str("");
			ss << "Shots:" << numberofbullets;
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2, 29, 11);

			ss.str("");
			if (Gun->type == GameObject::GO_BOW) {
				ss << "pierce:" << pierceforbullet + 12;
			}
			else
				ss << "pierce:" << pierceforbullet;
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2, 29, 8);

			ss.str("");
			ss << "MS:" << moveSpeed;
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2, 50, 14);
		}
		else if (chestOpened == true) {
			renderBossTraits(mousePos);
		}
		if (Transition == true)
		{
			if (elapsedTime >= 3.f)
				elapsedTime = 3.f;
			modelStack.PushMatrix();
			modelStack.Translate(m_worldWidth / 2, elapsedTime * m_worldHeight, 10);
			modelStack.Scale(250, 200, 1);
			RenderMesh(meshList[GEO_TRANSITION], false);
			modelStack.PopMatrix();
		}
		break;
	}
	case win:
	{
		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth * 0.5, m_worldHeight * 0.5f, 0);
		modelStack.Scale(m_worldWidth * 0.9, m_worldHeight * 0.9, 0);
		RenderMesh(meshList[GEO_WIN], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth / 2, m_worldHeight * 0.6, 1);
		modelStack.Scale(m_worldWidth * 0.5, 15, 1);
		RenderMesh(meshList[GEO_BUTTONBG], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth / 2, m_worldHeight * 0.6, 2);
		modelStack.Scale(m_worldWidth * 0.5, 12, 1);
		if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.25 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.25) && 
			(mousePos.y <= (m_worldHeight * 0.6) + 6 && mousePos.y >= (m_worldHeight * 0.6) - 6)) {
			meshList[GEO_RETRY]->material.kAmbient.Set(1, 1, 0);
		}
		else {
			meshList[GEO_RETRY]->material.kAmbient.Set(1, 1, 1);
		}
		RenderMesh(meshList[GEO_RETRY], true);
		modelStack.PopMatrix();
		
		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth / 2, m_worldHeight * 0.3, 1);
		modelStack.Scale(m_worldWidth * 0.5, 15, 1);
		RenderMesh(meshList[GEO_BUTTONBG], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth / 2, m_worldHeight * 0.3, 2);
		modelStack.Scale(m_worldWidth * 0.5, 12, 1);
		if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.25 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.25) &&
			(mousePos.y <= (m_worldHeight * 0.3) + 6 && mousePos.y >= (m_worldHeight * 0.3) - 6)) {
			meshList[GEO_RETURN]->material.kAmbient.Set(1, 1, 0);
		}
		else {
			meshList[GEO_RETURN]->material.kAmbient.Set(1, 1, 1);
		}
		RenderMesh(meshList[GEO_RETURN], true);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth * 0.6, m_worldHeight * 0.73, 1.2);
		modelStack.Scale(m_worldWidth * 0.05, m_worldHeight * 0.05, 2);
		RenderMesh(meshList[GEO_GOLD], false);
		modelStack.PopMatrix();

		std::ostringstream ss;
		ss.str("");
		ss << "X" <<acquiredGold;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 0), 2.5, 51, 42.4);


		ss.str("");
		ss << "score:" << score;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 3, 10, 43);
		break;
	}
	case lose:
	{
		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth * 0.5, m_worldHeight * 0.5f, 0);
		modelStack.Scale(m_worldWidth * 0.9, m_worldHeight * 0.9, 0);
		RenderMesh(meshList[GEO_LOSE], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth / 2, m_worldHeight * 0.6, 1);
		modelStack.Scale(m_worldWidth * 0.5, 15, 1);
		RenderMesh(meshList[GEO_BUTTONBG], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth / 2, m_worldHeight * 0.6, 2);
		modelStack.Scale(m_worldWidth * 0.5, 12, 1);
		if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.25 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.25) &&
			(mousePos.y <= (m_worldHeight * 0.6) + 6 && mousePos.y >= (m_worldHeight * 0.6) - 6)) {
			meshList[GEO_RETRY]->material.kAmbient.Set(1, 1, 0);
		}
		else {
			meshList[GEO_RETRY]->material.kAmbient.Set(1, 1, 1);
		}
		RenderMesh(meshList[GEO_RETRY], true);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth / 2, m_worldHeight * 0.3, 1);
		modelStack.Scale(m_worldWidth * 0.5, 15, 1);
		RenderMesh(meshList[GEO_BUTTONBG], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth / 2, m_worldHeight * 0.3, 2);
		modelStack.Scale(m_worldWidth * 0.5, 12, 1);
		if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.25 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.25) &&
			(mousePos.y <= (m_worldHeight * 0.3) + 6 && mousePos.y >= (m_worldHeight * 0.3) - 6)) {
			meshList[GEO_RETURN]->material.kAmbient.Set(1, 1, 0);
		}
		else {
			meshList[GEO_RETURN]->material.kAmbient.Set(1, 1, 1);
		}
		RenderMesh(meshList[GEO_RETURN], true);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth * 0.6, m_worldHeight * 0.73, 1.2);
		modelStack.Scale(m_worldWidth * 0.05, m_worldHeight * 0.05, 2);
		RenderMesh(meshList[GEO_GOLD], false);
		modelStack.PopMatrix();

		std::ostringstream ss;
		ss.str("");
		ss << "X" << acquiredGold;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 0), 2.5, 51, 42.4);


		ss.str("");
		ss << "score:" << score;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 3, 10, 43);
		break;
	}
	}
}

void SceneCollision::Exit()
{
	SceneBase::Exit();
	if (cKeyboardController)
	{
		// We won't delete this since it was created elsewhere
		cKeyboardController = NULL;
	}
	if (cMouseController)
	{
		// We won't delete this since it was created elsewhere
		cMouseController = NULL;
	}
	//Cleanup GameObjects
	while (m_goList.size() > 0)
	{
		GameObject* go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
	while (enemyAnimationPlayed.size() > 0)
	{
		Mesh* go = enemyAnimationPlayed.back();
		delete go;
		enemyAnimationPlayed.pop_back();
	}
	while (enemyList.size() > 0)
	{
		Enemy* go = enemyList.back();
		delete go;
		enemyList.pop_back();
	}
	if (cPlayer2D) {
		cPlayer2D->Destroy();
		cPlayer2D = NULL;
	}


}
