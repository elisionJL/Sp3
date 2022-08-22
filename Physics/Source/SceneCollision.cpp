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
	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();
	currentState = start;
	//Physics code here
	m_speed = 1.f;
	score = 0;
	thickWall = 0;
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
	}

	dmgofgun = 0;

	velocityofbullet = 20;

	bowframe = 0;

	MaxUpgrade = false;

	Transition = false;

	SongVolumeChange = 0;

	timerforpistol = 0;

	timerfordragon = 0;
	shootpistolspecial = false;
	staggertimingforpistol = 0;
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
				go->damage = Gun->thickWall / 100;
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

	switch (go2->type)
	{
	case GameObject::GO_BOSS_SLIME:
	{
		float dmg = dmgofgun * Bullet->bowdrawamount;

		if (Bullet->type == GameObject::GO_EXPLOSION)
			dmg = dmgofgun * 3.333333f;
		else if (Bullet->proj == GameObject::dragon)
			dmg = Bullet->damage;
		else if (Bullet->proj == GameObject::sniper)
			dmg = dmgofgun + Bullet->damage;

		go2->sethp(go2->gethp() - dmg);
		DamageNumbers(dmg, go2);


		Bullet->amountofpierleft -= 1;

		if (go2->gethp() <= 0)
		{
			DeleteEnemy(go2);
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
				break;
			}
		}
		break;
	}
	default:
		break;
	}
}

void SceneCollision::DeleteEnemy(Enemy* Enemy)
{
	for (int i = 0; i < enemyList.size(); ++i)
	{
		if (enemyList[i] == Enemy)
		{
			cPlayer2D->xp += Enemy->expVal;
			enemyList.erase(enemyList.begin() + i);
			score += 10;
			if (Enemy->type == GameObject::GO_BOSS_SLIME)
			{
				GameObject* go = FetchGO();
				go->pos = Enemy->pos;
				go->scale.Set(4, 4, 1);
				go->type = GameObject::GO_CHEST;
				go->vel.SetZero();
			}
		}
	}
}

void SceneCollision::DamageNumbers(int damage, Enemy* Enem)
{
	for (int arraynumber = 0; arraynumber < dmgandtimefordmgnumber.size(); ++arraynumber)
	{
		if (dmgandtimefordmgnumber[arraynumber] != 0)
		{
			continue;
		}
		dmgandtimefordmgnumber[arraynumber] = Vector3(Enem->pos.x, Enem->pos.y, damage);
		timerfordmgnumber[arraynumber] = elapsedTime + 0.5f;
		break;
	}
	dmgandtimefordmgnumber.push_back(Vector3(Enem->pos.x, Enem->pos.y, damage));
	timerfordmgnumber.push_back(elapsedTime + 0.5f);
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
			go->damage = 10;
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
	{
		float sign = 1;
		Vector3 dir = sign * (cPlayer2D->pos - enemy->pos).Normalized();
		float force = CalculateAdditionalForce(enemy, cPlayer2D);
		enemy->vel += 1.f / enemy->mass * dir * force * dt;
	}
}

void SceneCollision::RenderDmgNum(Vector3 posanddmg)
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
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0), 2, posX, posY);
	coordinatesofdamagenumbers.push_back(posX);
}

void SceneCollision::Update(double dt)
{
	SceneBase::Update(dt);
	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	if (Application::IsKeyPressed('9'))
	{
		currentState = lose;
	}
	if (Application::IsKeyPressed('0'))
	{
		currentState = win;
	}
	if (Application::IsKeyPressed('G'))
	{
		cPlayer2D->UseGold(-9999);
	}
	//Mouse Section
	double x, y, windowwidth, windowheight;
	Application::GetCursorPos(&x, &y);
	windowwidth = Application::GetWindowWidth();
	windowheight = Application::GetWindowHeight();
	Vector3 mousePos = Vector3((x / windowwidth) * m_worldWidth, ((windowheight - y) / windowheight) * m_worldHeight, 0);
	static bool bLButtonState = false;

	if (currentState == main)
		camera.Update(dt, cPlayer2D->pos, m_worldWidth, m_worldHeight);
	else {
		camera.position.Set(0, 0, 1);
		camera.target.Set(0, 0, 0);
		camera.up.Set(0, 1, 0);
	}
	switch (currentState) {
	case start:
	{
		if ((!bLButtonState && Application::IsMousePressed(0)) || Application::IsKeyPressed(VK_SPACE)) {
			bLButtonState = true;
			if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.2 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.2) && (mousePos.y <= (m_worldHeight * 0.4) + 4.75 && mousePos.y >= (m_worldHeight * 0.4) - 4.75)) {
				currentState = weaponselection;
				timerbeforeweaponselect = 1.0f;
				timerBeforeUpgrade = 1.0f;
				elapsedTime = 0;
				prevTime = 0;
				m_objectCount = 0;
				minutes = 2;
				seconds = 30;
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
		static bool leftclick = false;
		if ((!leftclick && Application::IsMousePressed(0))) {
			leftclick = true;
		}
		else if (leftclick && !Application::IsMousePressed(0)) {
			leftclick = false;
			if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.2 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.2) &&
				(mousePos.y <= (m_worldHeight * 0.4) + 4.75 && mousePos.y >= (m_worldHeight * 0.4) - 4.75)) {
				difficulty = easy;
			}
			if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.2 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.2) &&
				(mousePos.y <= (m_worldHeight * 0.4) + 4.75 && mousePos.y >= (m_worldHeight * 0.4) - 4.75)) {
				difficulty = hard;
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


				Gun->mass = 2;
				GunFrameWhereItStarts = 6;
				if (Gun->type == GameObject::GO_GL)
				{
					Gun->scale.Set(5, 2, 1);
					CurrentGun = meshList[GEO_GL];
					numberofbullets = 1;
					dmgofgun = 3; //explosion does 5
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
				}
				else if (Gun->type == GameObject::GO_SHOTGUN)
				{
					Gun->scale.Set(5, 2, 1);
					CurrentGun = meshList[GEO_SHOTGUN];
					numberofbullets = 4;
					dmgofgun = 3;
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
					firerate = 1.5f;
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
					dmgofgun = 4;
					pierceforbullet = 1;
					firerate = 1;
				}
				Gun->pos.Set(cPlayer2D->pos.x, cPlayer2D->pos.y, 3);
				Gun->vel.SetZero();
				if (Startgame)
				{
					cSoundController->PlaySoundByID(15);
					Transition = true;
					elapsedTime = 2;
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
		seconds -= dt;

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
		}

		else if (Transition == false)
		{
			if (minutes == 0 && seconds < 0) {
				currentState = win;
				break;
			}
			else if (seconds <= 0) {
				minutes -= 1;
				seconds += 60;
			}

			if (cPlayer2D->xp >= ((cPlayer2D->getLevel() - 1) * 10) + 5 && !cPlayer2D->leveledUp)
			{
				cPlayer2D->leveledUp = true;
				//generate 3 random upgrades for the player to choose
				for (int i = 0; i < 3; ++i) {
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
						break;
					case 6:
						levelUpgrades[i] = fireRate;
						break;
					case 7:
						levelUpgrades[i] = dragon;
						break;
					}

				}

			}
			if (cPlayer2D->leveledUp == false && pause == false) {
				cPlayer2D->Update(dt);
				//cPlayer2D->xp++;

				elapsedTime += dt;
				static bool BPressed = false;
				if (Application::IsKeyPressed('B') && BPressed == false) {
					BPressed = true;
				}
				else if (!Application::IsKeyPressed('B') && BPressed == true) {
					BPressed = false;
					pause = true;
					break;
				}


				//Enemy Spawn
				static bool blMButtonState = false;
				if (Application::IsKeyPressed('M') && blMButtonState == false)
				{
					Vector3 Epos;
					//GameObject* enemy = FetchGO();
					Enemy* go = new Enemy();

					int whichEnemytoSpawn = Math::RandIntMinMax(0, 5);
					switch (whichEnemytoSpawn)
					{
					case 0:
						go->type = GameObject::GO_BOSS_SLIME;
						break;
					case 1:
						go->type = GameObject::GO_SPIDER;
						break;
					case 2:
						go->type = GameObject::GO_VAMPIRE;
						break;
					case 3:
						go->type = GameObject::GO_SKELETON;
						break;
					case 4:
						go->type = GameObject::GO_GHOST;
						break;
					}

					Enemy::setSpawn(cPlayer2D->pos.x, cPlayer2D->pos.y, Epos);
					/*go->type = GameObject::GO_BOSS_SLIME;*/ //dont need this anymore
					go->scale.Set(10, 10, 1);
					go->pos = Epos;
					go->mass = 10;

					cout << Epos.x << endl;
					cout << Epos.y << endl;

					enemyList.push_back(go);

					const void* address = static_cast<const void*>(go);
					std::stringstream ss;
					ss << address;
					go->address = ss.str();

					blMButtonState = true;
				}
				else if (!Application::IsKeyPressed('M') && blMButtonState)
				{
					blMButtonState = false;
				}


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
								if (G->getcurrentanimationframe("Shoot") == 0)
								{
									SceneCollision::shooting(elapsedTime, numberofbullets, Gun);
								}
								else if (G->getcurrentanimationframe("ShootR") == GunFrameWhereItStarts)
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
					if (!bRButtonState && Application::IsMousePressed(1) && Gun->type == GameObject::GO_PISTOL && timerforpistol < elapsedTime)
					{
						timerforpistol = elapsedTime + firerate * 2;
						staggertimingforpistol = elapsedTime;
						bRButtonState = true;
						shootpistolspecial = true;
						Gun->mass = 0;
					}
					else if (bRButtonState && !Application::IsMousePressed(1))
					{
						bRButtonState = false;
					}
					if (shootpistolspecial)
					{
						if (staggertimingforpistol < elapsedTime && Gun->mass < 6) //for pistol special ability, 1 mass means 1 bullet
						{
							SceneCollision::PistolShooting(elapsedTime, numberofbullets);
							staggertimingforpistol = elapsedTime + 0.08f;
							G->PlayAnimation("Shoot", 0, 0.08f);
							G->Update(dt);
							Gun->mass++;
							if (Gun->mass == 6)
								shootpistolspecial = false;
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
								Companion->PlayAnimation("ShootL", -1, 2.0f);
								Companion->Update(dt);
								if (Companion->getcurrentanimationframe("ShootR") == 31 && go->bounce)
								{
									SceneCollision::dragonshooting(numberofbullets, 35, 2);
									go->bounce = false;
								}
								else if (Companion->getcurrentanimationframe("ShootR") == 34 && !go->bounce)
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
							else if (flip == 1 && mousePos.x > go->pos.x)
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

							if (!shootpistolspecial)
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
						else if (go->type == GameObject::GO_CHEST)
						{
							SpriteAnimation* Chest = dynamic_cast<SpriteAnimation*>(meshList[GEO_CHEST]);
							Chest->PlayAnimation("Opening", -1, 1.0f);
							Chest->Update(dt);
						}


						GameObject* go2 = nullptr;
						for (unsigned j = i + 1; j < size; ++j)
						{
							go2 = m_goList[j];
							GameObject* actor(go);
							GameObject* actee(go2);
							if (go->type != GameObject::GO_BALL)
							{
								actor = go2;
								actee = go;
							}
							if (actee->placed == false && actee->thinWall == 0 && actee->bounce == false) {
								continue;
							}
							if (go2->active && CheckCollision(actor, actee))
							{
								CollisionResponse(actor, actee);
							}
						}
						if (cPlayer2D->getState() == cPlayer2D->DEAD) {
							currentState = lose;
						}
					}
				}

				//Enemy List
				//pulling in of enemies			
				for (unsigned i = 0; i < enemyList.size(); ++i)
				{
					Enemy* go1 = enemyList[i];
					MoveEnemiesToPlayer(go1, cPlayer2D, dt);
					SpriteAnimation* enemy = dynamic_cast<SpriteAnimation*>(meshList[go1->type]);
					enemy->PlayAnimation("MoveRight", -1, 2.0f);
					enemy->Update(dt);
					go1->pos += go1->vel * dt;

					for (unsigned x = i; x < enemyList.size(); ++x)
					{
						Enemy* go2 = enemyList[x];

						if (go2->gethp() > 0 && go2 != go1)
						{
							if (CheckCollision(go1, go2))
							{
								go1->pos -= go1->vel * dt;
							}
						}
					}
				}
			}
			else if (cPlayer2D->leveledUp == true) {
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
								break;
							case velocity:
								velocityofbullet += 5;
								break;
							case fireRate:
								firerate *= 0.95;
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
								}
								break;
							}
						}
					}
				}
			}
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
					else if ((mousePos.x >= x - (m_worldWidth * 0.1) && mousePos.x <= x + (m_worldWidth * 0.1) &&
						mousePos.y <= m_worldHeight * 0.67 && mousePos.y >= m_worldHeight * 0.53)) {
						pause = false;
						currentState = lose;
					}
				}
			}
		}

		break;
	}
	case win:
	{
		static bool bLButtonState = false;
		if (!bLButtonState && Application::IsMousePressed(0)) {
			bLButtonState = true;
			if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.25 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.25) && (mousePos.y <= (m_worldHeight * 0.6) + 7.5 && mousePos.y >= (m_worldHeight * 0.6) - 7.5)) {
				currentState = main;
				m_objectCount = 0;
				minutes = 2;
				seconds = 30;;
				m_goList.clear();
				m_thickWallList.clear();
			}
			else if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.25 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.25) && (mousePos.y <= (m_worldHeight * 0.3) + 7.5 && mousePos.y >= (m_worldHeight * 0.3) - 7.5)) {
				quit = true;
			}
		}
		break;
	}
	case lose:
	{
		static bool bLButtonState = false;
		if (!bLButtonState && Application::IsMousePressed(0)) {
			bLButtonState = true;
			if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.25 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.25) && (mousePos.y <= (m_worldHeight * 0.6) + 7.5 && mousePos.y >= (m_worldHeight * 0.6) - 7.5)) {
				currentState = main;
				m_objectCount = 0;
				minutes = 2;
				seconds = 30;
				m_goList.clear();
				m_thickWallList.clear();
			}
			else if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.25 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.25) && (mousePos.y <= (m_worldHeight * 0.3) + 7.5 && mousePos.y >= (m_worldHeight * 0.3) - 7.5)) {
				quit = true;
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
		case GameObject::GO_PILLAR:
		case GameObject::GO_BALL:
		{
			Vector3 relativeVel = go1->vel - go2->vel;
			Vector3 disDiff = go2->pos - go1->pos;

			if (relativeVel.Dot(disDiff) <= 0) {
				return false;
			}
			return disDiff.LengthSquared() <= (go1->scale.x + go2->scale.x) * (go1->scale.x + go2->scale.x);
		}
		break;
		case GameObject::GO_WALL:
		{
			Vector3 diff = go1->pos - go2->pos;
			Vector3 axisX = go2->normal;
			Vector3 axisY = Vector3(-go2->normal.y, go2->normal.x, 0);


			float ProjectedDist = diff.Dot(axisX);

			//if its a thickwall
			if (go2->otherWall != nullptr)
			{
				if (Math::FAbs(ProjectedDist) / go2->scale.x < Math::FAbs(diff.Dot(axisY)) / go2->otherWall->scale.x)
				{
					return false;
				}
			}

			if (ProjectedDist > 0)
			{
				axisX = -axisX;
			}

			return go1->vel.Dot(axisX) >= 0 &&  //if traveling towards wall
				go2->scale.x * 0.5 + go1->scale.x > -diff.Dot(axisX) && //Radius + Thickness vs Distance away from ball
				go2->scale.y * 0.5 > fabs(diff.Dot(axisY)); //Length check
		}
		break;
		case GameObject::GO_POWERUP:
		{
			if (go2->PUIFrame > 3.5) {
				return false;
			}
			Vector3 relativeVel = go1->vel - go2->vel;
			Vector3 disDiff = go2->pos - go1->pos;
			if (relativeVel.Dot(disDiff) <= 0) {
				return false;
			}
			return disDiff.LengthSquared() <= (go1->scale.x + go2->scale.x) * (go1->scale.x + go2->scale.x);
		}
		break;
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

bool SceneCollision::CheckCollision(Enemy * enemy1, Enemy * enemy2)
{

	Vector3 relativeVel = enemy1->vel - enemy2->vel;

	Vector3 disDiff = enemy2->pos - enemy1->pos;

	if (enemy1->pos.y > enemy2->pos.y)
	{
		disDiff -= Vector3(0, enemy1->scale.y / 2, 0);
	}
	else
	{
		disDiff += Vector3(0, enemy1->scale.y / 2, 0);
	}

	if (enemy1->pos.x > enemy2->pos.x)
	{
		disDiff -= Vector3(enemy1->scale.x / 2, 0, 0);
	}
	else
	{
		disDiff += Vector3(enemy1->scale.x / 2, 0, 0);
	}


	if (relativeVel.Dot(disDiff) <= 0) {
		return false;
	}
	if (enemy1->pos.y > enemy2->pos.y)
	{
		if (enemy1->pos.y - enemy1->scale.y <= enemy2->pos.y)
		{
			enemy1->usePrevY = true;
			enemy1->enemyup = true;
		}
		else
		{
			enemy1->usePrevY = false;
			enemy1->enemyup = false;
		}
	}
	else
	{
		if (enemy1->pos.y + enemy1->scale.y >= enemy2->pos.y)
		{
			enemy1->usePrevY = true;
			enemy1->enemyup = true;
		}
		else
		{
			enemy1->usePrevY = false;
			enemy1->enemyup = false;
		}
	}

	if (enemy1->pos.x > enemy2->pos.x)
	{
		if (enemy1->pos.x - enemy1->scale.x <= enemy2->pos.x)
		{
			enemy1->usePrevX = true;
			enemy1->enemytoright = true;
		}
		else
		{
			enemy1->usePrevX = false;
			enemy1->enemytoright = false;
		}
	}
	else
	{
		if (enemy1->pos.x + enemy1->scale.x >= enemy2->pos.x)
		{
			enemy1->usePrevX = true;
			enemy1->enemytoleft = true;
		}
		else
		{
			enemy1->usePrevX = false;
			enemy1->enemytoleft = false;
		}
	}

	return disDiff.LengthSquared() <= (enemy1->scale.x + enemy2->scale.x) * (enemy1->scale.x + enemy2->scale.x);

}

bool SceneCollision::CheckCollision(Enemy * enemy, GameObject * go)
{
	return false;
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

void SceneCollision::MakeThickWall(float width, float height, const Vector3 & normal, const Vector3 & pos)
{
	Vector3 tangent(-normal.y, normal.x);
	thickWall++;
	//first wall
	GameObject* wall1 = FetchGO();
	float size = 0.1f;
	wall1->type = GameObject::GO_WALL;
	wall1->scale.Set(width, height, 1.0f);
	wall1->pos = pos;
	wall1->normal = normal;
	wall1->vel = Vector3(0, -1, 0);
	wall1->placed = true;
	wall1->thickWall = thickWall;
	wall1->mass = 5;
	m_thickWallList.push_back(wall1);

	//second wall
	GameObject* wall2 = FetchGO();
	wall2->type = GameObject::GO_WALL;
	wall2->scale.Set(height, width, 1.0f);
	wall2->pos = pos;
	wall2->normal = tangent;
	wall2->vel = Vector3(0, -1, 0);
	wall2->visible = false;
	wall2->thickWall = thickWall;
	wall2->placed = true;
	wall2->mass = 5;
	m_thickWallList.push_back(wall2);

	wall1->otherWall = wall2;
	wall2->otherWall = wall1;

	GameObject* pillar = FetchGO();
	pillar->type = GameObject::GO_PILLAR;
	pillar->color.Set(1, 1, 0);
	pillar->scale.Set(size, size, 1.f);
	pillar->pos = pos + height * 0.5f * tangent + width * 0.5f * normal;
	pillar->thickWall = thickWall;
	pillar->placed = true;
	pillar->vel = Vector3(0, -1, 0);
	pillar->mass = 5;
	m_thickWallList.push_back(pillar);

	pillar = FetchGO();
	pillar->type = GameObject::GO_PILLAR;
	pillar->color.Set(1, 1, 0);
	pillar->scale.Set(size, size, 1.f);
	pillar->pos = pos + height * 0.5f * tangent - width * 0.5f * normal;
	pillar->thickWall = thickWall;
	pillar->placed = true;
	pillar->vel = Vector3(0, -1, 0);
	pillar->mass = 5;
	m_thickWallList.push_back(pillar);

	pillar = FetchGO();
	pillar->type = GameObject::GO_PILLAR;
	pillar->color.Set(1, 1, 0);
	pillar->scale.Set(size, size, 1.f);
	pillar->pos = pos - height * 0.5f * tangent - width * 0.5f * normal;
	pillar->thickWall = thickWall;
	pillar->placed = true;
	pillar->vel = Vector3(0, -1, 0);
	pillar->mass = 5;
	m_thickWallList.push_back(pillar);

	pillar = FetchGO();
	pillar->type = GameObject::GO_PILLAR;
	pillar->color.Set(1, 1, 0);
	pillar->scale.Set(size, size, 1.f);
	pillar->pos = pos - height * 0.5f * tangent + width * 0.5f * normal;
	pillar->thickWall = thickWall;
	pillar->placed = true;
	pillar->vel = Vector3(0, -1, 0);
	pillar->mass = 5;
	m_thickWallList.push_back(pillar);
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
			if (cPlayer2D->GetGold() < 200 * pow(1.75, ShopUpgrades[0]) || ShopUpgrades[0] >= 5)
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
			if (cPlayer2D->GetGold() < 100 * pow(1.3, ShopUpgrades[1]) || ShopUpgrades[1] >= 10)
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
			if (cPlayer2D->GetGold() < 150 * pow(2.0, ShopUpgrades[2]) || ShopUpgrades[2] >= 5)
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
			if (cPlayer2D->GetGold() < 100 * pow(1.5, ShopUpgrades[3]) || ShopUpgrades[3] >= 4)
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
			if (cPlayer2D->GetGold() < 500 * pow(1.1, ShopUpgrades[5]) || ShopUpgrades[5] >= 5)
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
	modelStack.PushMatrix();
	modelStack.Translate(m_worldWidth * 0.4, m_worldHeight * 0.45f, 4);
	modelStack.Scale(m_worldWidth * 0.6, m_worldHeight * 0.4, 0);
	RenderMesh(meshList[GEO_SHOP_UI_BG], false);
	modelStack.PopMatrix();

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
				MaxUpgrade = false;
			if (MaxUpgrade == true)
				randomDialogue = rand() % 3 + 25;
			else if (cPlayer2D->GetGold() > 200 * pow(1.75, ShopUpgrades[0]))
				randomDialogue = rand() % 4 + 17;
			else
				randomDialogue = rand() % 4 + 21;
			if (ShopUpgrades[0] >= 5)
				MaxUpgrade = true;
			PlayerBuy = true;
			currentlyHovering = false;
		}
		else if ((mousePos.x >= (m_worldWidth * 0.4f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.4f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.45f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.45f))) {
			OutputDialogue = "";
			CurrentCharText = 0;
			if (ShopUpgrades[1] < 10)
				MaxUpgrade = false;
			if (MaxUpgrade == true)
				randomDialogue = rand() % 3 + 25;
			else if (cPlayer2D->GetGold() > 100 * pow(1.3, ShopUpgrades[1]))
				randomDialogue = rand() % 4 + 17;
			else
				randomDialogue = rand() % 4 + 21;
			if (ShopUpgrades[1] >= 10)
				MaxUpgrade = true;
			PlayerBuy = true;
			currentlyHovering = false;
		}
		else if ((mousePos.x >= (m_worldWidth * 0.55f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.55f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.45f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.45f))) {
			OutputDialogue = "";
			CurrentCharText = 0;
			if (ShopUpgrades[2] < 5)
				MaxUpgrade = false;
			if (MaxUpgrade == true)
				randomDialogue = rand() % 3 + 25;
			else if (cPlayer2D->GetGold() > 150 * pow(2.0, ShopUpgrades[2]))
				randomDialogue = rand() % 4 + 17;
			else
				randomDialogue = rand() % 4 + 21;
			if (ShopUpgrades[2] >= 5)
				MaxUpgrade = true;
			PlayerBuy = true;
			currentlyHovering = false;
		}
		else if ((mousePos.x >= (m_worldWidth * 0.25f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.25f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.325f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.325f))) {
			OutputDialogue = "";
			CurrentCharText = 0;
			if (ShopUpgrades[3] < 4)
				MaxUpgrade = false;
			if (MaxUpgrade == true)
				randomDialogue = rand() % 3 + 25;
			else if (cPlayer2D->GetGold() > 100 * pow(1.5, ShopUpgrades[3]))
				randomDialogue = rand() % 4 + 17;
			else
				randomDialogue = rand() % 4 + 21;
			if (ShopUpgrades[3] >= 4)
				MaxUpgrade = true;
			PlayerBuy = true;
			currentlyHovering = false;
		}
		else if ((mousePos.x >= (m_worldWidth * 0.4f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.4f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.325f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.325f))) {
			OutputDialogue = "";
			CurrentCharText = 0;
			if (ShopUpgrades[4] <= 1) {
				MaxUpgrade = false;
				ShopUpgrades[4] += 1;
			}
			if (MaxUpgrade == true)
				randomDialogue = rand() % 3 + 25;
			else if (cPlayer2D->GetGold() > 2000)
				randomDialogue = rand() % 4 + 17;
			else
				randomDialogue = rand() % 4 + 21;
			if (ShopUpgrades[4] > 1)
				MaxUpgrade = true;
			PlayerBuy = true;
			currentlyHovering = false;
		}
		else if ((mousePos.x >= (m_worldWidth * 0.55f) - m_worldWidth * 0.05 && mousePos.x <= (m_worldWidth * 0.55f) + m_worldWidth * 0.05) && (mousePos.y <= (m_worldHeight * 0.325f) + m_worldHeight * 0.1 && mousePos.y >= (m_worldHeight * 0.325f))) {
			OutputDialogue = "";
			CurrentCharText = 0;
			if (ShopUpgrades[5] < 5)
				MaxUpgrade = false;
			if (MaxUpgrade == true)
				randomDialogue = rand() % 3 + 25;
			else if (cPlayer2D->GetGold() > 500 * pow(1.1, ShopUpgrades[5]))
				randomDialogue = rand() % 4 + 17;
			else
				randomDialogue = rand() % 4 + 21;
			if (ShopUpgrades[5] >= 5)
				MaxUpgrade = true;
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
				CurrentDialogue = GronkDialogue[randomDialogue][CurrentCharText];
				OutputDialogue += CurrentDialogue;

				CurrentCharText += 1;
				dialogueTime = 0;
			}

			else
			{
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

	while (ObjectCount < 200)
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

		SpawnX = ((-m_worldWidth) + (-m_worldWidth) + (-m_worldWidth)) + static_cast<float>(rand()) * static_cast<float>(((m_worldWidth + m_worldWidth + m_worldWidth)) - ((-m_worldWidth) + (-m_worldWidth) + (-m_worldWidth))) / RAND_MAX;
		SpawnY = ((-m_worldHeight) + (-m_worldHeight) + (-m_worldHeight)) + static_cast<float>(rand()) * static_cast<float>(((m_worldHeight + m_worldHeight + m_worldHeight)) - ((-m_worldHeight) + (-m_worldHeight) + (-m_worldHeight))) / RAND_MAX;

		//TreeSpawnPos.Set(SpawnX, SpawnY, 4.f);
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
		}
	}
}

void SceneCollision::MapBoundary()
{
	if (cPlayer2D->pos.y >= 295.f)
		cPlayer2D->pos.y = Math::Clamp(cPlayer2D->pos.y, 295.f, 295.f);
	if (cPlayer2D->pos.y <= -295.f)
		cPlayer2D->pos.y = Math::Clamp(cPlayer2D->pos.y, -295.f, -295.f);
	if (cPlayer2D->pos.x >= 455.f)
		cPlayer2D->pos.x = Math::Clamp(cPlayer2D->pos.x, 455.f, 455.f);
	if (cPlayer2D->pos.x <= -455.f)
		cPlayer2D->pos.x = Math::Clamp(cPlayer2D->pos.x, -455.f, -455.f);
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
	////Play the animation �ROW1� that is looping infinitely and
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
		case GameObject::heal:
			RenderMesh(meshList[GEO_HEAL], false);
			break;
		case GameObject::rechargeUp:
			RenderMesh(meshList[GEO_RECHARGEUP], false);
			break;
		case GameObject::ballUp:
			RenderMesh(meshList[GEO_BALLUP], false);
			break;
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
		case GameObject::pistol:
			meshList[GEO_PROJECTILE]->textureID = LoadTexture("Image//bullet.png", true);
			break;
		case GameObject::shotgun:
			meshList[GEO_PROJECTILE]->textureID = LoadTexture("Image//shotgunBullet.png", true);
			break;
		case GameObject::sniper:
			meshList[GEO_PROJECTILE]->textureID = LoadTexture("Image//sniperBullet.png", true);
			break;
		case GameObject::GL:
			meshList[GEO_PROJECTILE]->textureID = LoadTexture("Image//bullet.png", true);
			break;
		case GameObject::bow:
			modelStack.Scale(1, 0.5, 1);
			meshList[GEO_PROJECTILE]->textureID = LoadTexture("Image//arrow.png", true);
			break;
		case GameObject::dragon:
			modelStack.Scale(1, 0.5, 1);
			meshList[GEO_PROJECTILE]->textureID = LoadTexture("Image//firestatus.png", true);
			break;
		}

		RenderMesh(meshList[GEO_PROJECTILE], false);
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
		if (Transition == true)
		{
			if (elapsedTime >= 3.f)
				elapsedTime = 3.f;
			modelStack.PushMatrix();
			modelStack.Translate(m_worldWidth / 2, elapsedTime * m_worldHeight, 10);
			modelStack.Scale(200, 200, 1);
			RenderMesh(meshList[GEO_TRANSITION], false);
			modelStack.PopMatrix();
		}
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

		modelStack.PushMatrix();
		modelStack.Translate(cPlayer2D->pos.x, cPlayer2D->pos.y, 1.0001f);
		modelStack.Scale(10, 10, 1);
		RenderMesh(meshList[GEO_PLAYER], false);
		modelStack.PopMatrix();

		//Render Boundary
		float RenderDistance;
		RenderDistance = 0;
		for (int y = 0; y < 6; ++y)
		{
			for (int x = -60; x < 60; ++x)
			{
				RenderDistance = cPlayer2D->pos.Length() - Vector3((m_worldWidth / 2) + (9.8f * x), (m_worldHeight * 3.05) + (9.8 * y), zaxis).Length();

				if (RenderDistance < 0)
					RenderDistance = -RenderDistance;

				if (RenderDistance < 100)
				{
					modelStack.PushMatrix();
					modelStack.Translate((m_worldWidth / 2) + (9.8f * x), (m_worldHeight * 3.05) + (9.8 * y), 3);
					modelStack.Scale(10, 10, 10);
					RenderMesh(meshList[GEO_BOUNDARY], false);
					modelStack.PopMatrix();
				}
			}
		}
		for (int y = 0; y < 6; ++y)
		{
			for (int x = -60; x < 60; ++x)
			{
				RenderDistance = cPlayer2D->pos.Length() - Vector3((m_worldWidth / 2) + (9.8f * x), (m_worldHeight * -3.05) - (9.8 * y), zaxis).Length();

				if (RenderDistance < 0)
					RenderDistance = -RenderDistance;

				if (RenderDistance < 100)
				{
					modelStack.PushMatrix();
					modelStack.Translate((m_worldWidth / 2) + (9.8f * x), (m_worldHeight * -3.05) - (9.8 * y), 3);
					modelStack.Scale(10, 10, 10);
					RenderMesh(meshList[GEO_BOUNDARY], false);
					modelStack.PopMatrix();
				}
			}
		}
		for (int x = 0; x < 9; ++x)
		{
			for (int y = -60; y < 60; ++y)
			{
				RenderDistance = cPlayer2D->pos.Length() - Vector3((m_worldWidth * -2.55) - (9.8f * x), (m_worldHeight / 2) + (9.8 * y), zaxis).Length();

				if (RenderDistance < 0)
					RenderDistance = -RenderDistance;

				if (RenderDistance < 100)
				{
					modelStack.PushMatrix();
					modelStack.Translate((m_worldWidth * -2.55) - (9.8f * x), (m_worldHeight / 2) + (9.8 * y), 3);
					modelStack.Scale(10, 10, 10);
					RenderMesh(meshList[GEO_BOUNDARY], false);
					modelStack.PopMatrix();
				}
			}
		}
		for (int x = 0; x < 9; ++x)
		{
			for (int y = -60; y < 60; ++y)
			{
				RenderDistance = cPlayer2D->pos.Length() - Vector3((m_worldWidth * 2.55) + (9.8f * x), (m_worldHeight / 2) + (9.8 * y), zaxis).Length();

				if (RenderDistance < 0)
					RenderDistance = -RenderDistance;

				if (RenderDistance < 100)
				{
					modelStack.PushMatrix();
					modelStack.Translate((m_worldWidth * 2.55) + (9.8f * x), (m_worldHeight / 2) + (9.8 * y), 3);
					modelStack.Scale(10, 10, 10);
					RenderMesh(meshList[GEO_BOUNDARY], false);
					modelStack.PopMatrix();
				}
			}
		}

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
				RenderMesh(meshList[GEO_BOSS_SLIME], false);
				modelStack.PopMatrix();
			}
		}

		for (int i = 0; i < timerfordmgnumber.size(); ++i)
		{
			if (elapsedTime < timerfordmgnumber[i])
			{
				SceneCollision::RenderDmgNum(dmgandtimefordmgnumber[i]);
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
		modelStack.Translate(expX, expY, 4);
		modelStack.Scale(expScaleX, expScaleY, 1);
		RenderMesh(meshList[GEO_EXPBG], false);
		modelStack.PopMatrix();


		expScaleX = Math::Min((float)(m_worldWidth * 0.75), m_worldWidth * (float)0.75 * (cPlayer2D->xp / ((cPlayer2D->getLevel() - 1) * 10 + 5)));

		modelStack.PushMatrix();
		modelStack.Translate(expScaleX / 2 + m_worldWidth * 0.12 + camera.position.x, expY, 4.1f);
		modelStack.Scale(expScaleX, expScaleY, 1);
		RenderMesh(meshList[GEO_EXP], false);
		modelStack.PopMatrix();


		//hp
		float hpX = m_worldWidth * 0.16 + camera.position.x, hpY = m_worldHeight * 0.8 + camera.position.y;
		float hpScaleX = m_worldWidth * 0.3, hpScaleY = 10;
		modelStack.PushMatrix();
		modelStack.Translate(hpX, hpY, 4);
		modelStack.Scale(hpScaleX, hpScaleY, 1);
		RenderMesh(meshList[GEO_HEALTHBG], false);
		modelStack.PopMatrix();


		hpScaleX = Math::Min(m_worldWidth * 0.3 * 0.73684210526, m_worldWidth * 0.3 * 0.73684210526 * (cPlayer2D->hp / cPlayer2D->maxHP));

		modelStack.PushMatrix();
		modelStack.Translate(hpScaleX / 2 + hpX - m_worldWidth * 0.075, hpY, 4.1f);
		modelStack.Scale(hpScaleX, hpScaleY, 1);
		RenderMesh(meshList[GEO_HEALTH], false);
		modelStack.PopMatrix();

		//On screen text
		std::ostringstream ss;
		ss.precision(5);
		ss << "FPS: " << fps;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 3, 0, 3);


		ss.str("");
		ss.precision(2);
		ss << minutes << ":" << seconds;
		if (seconds < 10) {
			ss.precision(1);
			ss.str("");
			ss << minutes << ":0" << seconds;
		}
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 3, 3, 56);

		//render if player has leveled up
		if (cPlayer2D->leveledUp == true) {
			modelStack.PushMatrix();
			modelStack.Translate(camera.position.x, camera.position.y, 4.3f);
			modelStack.Scale(1000, 1000, 1);
			RenderMesh(meshList[GEO_LVLUPBG], false);
			modelStack.PopMatrix();

			for (int i = 1; i < 4; ++i) {
				float x = (i * 0.04 * m_worldWidth) + ((i - 1) * 0.28 * m_worldWidth) + (m_worldWidth * 0.14);
				float cameramoveX = cPlayer2D->pos.x - m_worldWidth * 0.5;
				float cameramoveY = cPlayer2D->pos.y - m_worldHeight * 0.5;
				float textx;
				Vector3 color = Vector3(1, 0, 0);
				switch (i) {
				case 1:
					textx = 8;
					break;
				case 2:
					textx = 34;
					break;
				case 3:
					textx = 60;
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
				modelStack.Translate(x + cameramoveX, m_worldHeight * 0.6 + cameramoveY, 6.02);
				modelStack.Scale(m_worldWidth * 0.14, m_worldHeight * 0.14, 1);
				ss.str("");

				switch (levelUpgrades[i - 1]) {
				case pierce:
					meshList[GEO_UPGRADEICON]->textureID = LoadTexture("Image//upgrades//pierceUp.png", true);
					ss << "pierce +1";
					RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 1, textx + 2, 20);
					break;
				case atk:
					meshList[GEO_UPGRADEICON]->textureID = LoadTexture("Image//upgrades//atkUp.png", true);
					ss << "damage +10%";
					RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 1, textx, 20);
					break;
				case hp:
					meshList[GEO_UPGRADEICON]->textureID = LoadTexture("Image//upgrades//hpUp.png", true);
					ss << "110% hp";
					RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 1, textx + 2, 20);
					break;
				case multishot:
					meshList[GEO_UPGRADEICON]->textureID = LoadTexture("Image//upgrades//multishot.png", true);
					ss << "bulletcount +1";
					RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 1, textx - 1, 20);
					break;
				case moveSpeed:
					meshList[GEO_UPGRADEICON]->textureID = LoadTexture("Image//upgrades//moveSpeedUp.png", true);
					ss << "105% movespeed";
					RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 1, textx - 1, 20);
					break;
				case velocity:
					meshList[GEO_UPGRADEICON]->textureID = LoadTexture("Image//upgrades//velUp.png", true);
					ss << "velocity +5";
					RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 1, textx, 20);
					break;
				case fireRate:
					meshList[GEO_UPGRADEICON]->textureID = LoadTexture("Image//upgrades//fireRateUp.png", true);
					ss << "110% firerate";
					RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 1, textx - 1, 20);
					break;
				case dragon:
					meshList[GEO_UPGRADEICON]->textureID = LoadTexture("Image//upgrades//companion.png", true);
					ss << "grants a dragon companion";
					RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 1, textx, 20);
				}
				RenderMesh(meshList[GEO_UPGRADEICON], false);

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
			if (Gun->type == GameObject::GO_BOW) {
				ss << "RoF:" << firerate << "s";
			}
			else
				ss << "RoF:" << firerate << "s";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2, 11, 8);

			ss.str("");
			ss << "hp:" << cPlayer2D->maxHP;
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2, 25, 14);

			ss.str("");
			ss << "Shots:" << numberofbullets;
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2, 25, 11);

			ss.str("");
			if (Gun->type == GameObject::GO_BOW) {
				ss << "pierce:" << pierceforbullet + 12;
			}
			else
				ss << "pierce:" << pierceforbullet;
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2, 25, 8);

			ss.str("");
			ss << "MS:" << moveSpeed;
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2, 50, 14);
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
		RenderMesh(meshList[GEO_RETRY], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth / 2, m_worldHeight * 0.3, 1);
		modelStack.Scale(m_worldWidth * 0.5, 15, 1);
		RenderMesh(meshList[GEO_QUIT], false);
		modelStack.PopMatrix();

		std::ostringstream ss;
		ss.str("");
		ss << "score: " << score;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 3, 25, 43);
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
		RenderMesh(meshList[GEO_RETRY], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth / 2, m_worldHeight * 0.3, 1);
		modelStack.Scale(m_worldWidth * 0.5, 15, 1);
		RenderMesh(meshList[GEO_QUIT], false);
		modelStack.PopMatrix();

		std::ostringstream ss;
		ss.str("");
		ss << "score: " << score;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 3, 25, 43);
		break;
	}
	}
}

void SceneCollision::Exit()
{
	SceneBase::Exit();
	//Cleanup GameObjects
	while (m_goList.size() > 0)
	{
		GameObject* go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
	while (enemyList.size() > 0)
	{
		Enemy* go = enemyList.back();
		delete go;
		enemyList.pop_back();
	}
	cPlayer2D->Destroy();


}
