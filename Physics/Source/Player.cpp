#include "player.h"
#include "Application.h"
#include "SpriteAnimation.h"
#include "iostream"
void CPlayer2D::Init()
{
	maxHP = hp = 30;
	Walk_Speed = maxWalk_Speed= 30;
	dmg = maxDamage = Lives = maxLives = 1;
	Gold = 0;
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();
	pos.Set(m_worldWidth * 0.5, m_worldHeight * 0.5, 1);
	vel.Set(0, 0, 0);
	sFacingDirection = RIGHT;
	sCurrentState = IDLE;
	rollCooldown = 0;
	maxRollCooldown = 5;
	ShieldCountDown = 0;
	xp = 0;
	xpBooster = 1;
	level = 1;
	prevTime = elapsedTime = 0;
	leveledUp = false;
	inVuln = 0;
}

void CPlayer2D::Update(double dt)
{
	SpriteAnimation* sa = dynamic_cast<SpriteAnimation*>(playerMesh);
	if (Application::IsKeyPressed('L')) {
		hp = 0;
	}
	if (Application::IsKeyPressed('K')) {
		xp += 1;
	}
	if (Application::IsKeyPressed('J')) {
		hp += 1;
	}
	if (Application::IsKeyPressed('H')) {
		hp -= 1;
	}

	if (hp < maxHP) {
		if (elapsedTime - prevTime > 1) {
			hp += 0.01 * maxHP;
			prevTime = elapsedTime;
		}
		else {
			elapsedTime += dt;
		}
	}
	if (hp <= 0) {
		if (Lives - 1 > 0)
		{
			hp = maxHP;
			Lives -= 1;
		}

		else
		{
			sa->PlayAnimation("death", 0, 2.f);
			sa->Update(dt);
			if (sa->getAnimationStatus("death") == false) {
				sCurrentState = DEAD;
			}
		}
	}
	else if (hp > 0) {
		rollCooldown += dt;
		if (Application::IsKeyPressed('Q') && sCurrentState != DODGING && rollCooldown > maxRollCooldown) {
			sCurrentState = DODGING;
			rollCooldown = 0;
		}
		if (Application::IsKeyPressed('W')) {
			vel.y = Walk_Speed;
			if (sCurrentState != DODGING)
				sCurrentState = MOVING;
		}
		else if (Application::IsKeyPressed('S')) {
			vel.y = -Walk_Speed;
			if (sCurrentState != DODGING)
				sCurrentState = MOVING;
		}
		else {
			vel.y = 0;
			if (sCurrentState != DODGING)
				sCurrentState = IDLE;
		}
		
		if (Application::IsKeyPressed('D')) {
			sFacingDirection = RIGHT;
			if (sCurrentState != DODGING)
				sCurrentState = MOVING;
			vel.x = Walk_Speed;
		}
		else if (Application::IsKeyPressed('A')) {
			sFacingDirection = LEFT;
			if (sCurrentState != DODGING)
				sCurrentState = MOVING;
			vel.x = -Walk_Speed;
		}
		else {
			vel.x = 0;
			if (sCurrentState == IDLE && sCurrentState != DODGING) {
				sCurrentState = IDLE;
			}
		}

		switch (sCurrentState) {
		case MOVING:
			switch (sFacingDirection) {
			case RIGHT:
				sa->PlayAnimation("walkR", -1, 1.f);
				break;
			case LEFT:
				sa->PlayAnimation("walkL", -1, 1.f); 
				break;
			}
			break;
		case IDLE:
			switch (sFacingDirection) {
			case RIGHT:
				sa->PlayAnimation("idleR", -1, 1.f);
				break;
			case LEFT:
				sa->PlayAnimation("idleL", -1, 1.f);
				break;
			}
			break;
		case DODGING:
			switch (sFacingDirection) {
			case RIGHT:
				sa->PlayAnimation("rollR", 0, 1.5f);
				break;
			case LEFT:
				sa->PlayAnimation("rollL", 0, 1.5f);
				break;
			}
			break;
		}
		sa->Update(dt);
		if (sCurrentState == DODGING) {
			pos += vel * 2 * dt;
			inVuln += elapsedTime + 100;
			switch (sFacingDirection) {
			case RIGHT:
				if (sa->getAnimationStatus("rollR") == false) {
					sCurrentState = IDLE;
					inVuln = elapsedTime;
				}
				break;
			case LEFT:
				if (sa->getAnimationStatus("rollL") == false) {
					sCurrentState = IDLE;
					inVuln = elapsedTime;
				}
				break;
			}
		}
		else {
			pos += vel * dt;
		}
	}

}
void CPlayer2D::Render()
{

}

void CPlayer2D::Exit()
{
}

void CPlayer2D::setmeshList(Mesh* meshlist)
{
	playerMesh = meshlist;
}

void CPlayer2D::reset()
{
	maxHP = hp = 30;
	Walk_Speed = maxWalk_Speed = 30;
	dmg = maxDamage = Lives = maxLives = 1;
	Gold = 0;
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();
	pos.Set(m_worldWidth * 0.5, m_worldHeight * 0.5, 1);
	vel.Set(0, 0, 0);
	sFacingDirection = RIGHT;
	sCurrentState = IDLE;
	rollCooldown = 0;
	maxRollCooldown = 5;
	ShieldCountDown = 0;
	xp = 0;
	xpBooster = 1;
	level = 1;
	prevTime = elapsedTime = 0;
	leveledUp = false;
}

int CPlayer2D::getLevel()
{
	return level;
}

void CPlayer2D::increaseLevel()
{
	xp -= ((level - 1) * 10) + 5;
	level += 1;
	leveledUp = false;
}

int CPlayer2D::getState()
{
	return sCurrentState;
}

void CPlayer2D::IncreaseSpd()
{
	maxWalk_Speed *= 1.15f;
	Walk_Speed = maxWalk_Speed;
}

void CPlayer2D::IncreaseHP()
{
	maxHP += 5;
	hp = maxHP;
}

void CPlayer2D::DecreaseShieldCooldown()
{
	ShieldCountDown += 1;
}

void CPlayer2D::IncreaseDmg()
{
	maxDamage += 1;
	dmg = maxDamage;
}

void CPlayer2D::IncreaseLifeCount()
{
	maxLives += 1;
	Lives = maxLives;
}

void CPlayer2D::IncreaseEXPGain()
{
	if (xpBooster == 1)
		xpBooster = 1.1f;
	else if (xpBooster == 1.1f)
		xpBooster = 1.2f;
	else if (xpBooster == 1.2f)
		xpBooster = 1.3f;
	else if (xpBooster == 1.3f)
		xpBooster = 1.4f;
	else if (xpBooster == 1.4f)
		xpBooster = 1.5f;
}

void CPlayer2D::setStats()
{
	hp = maxHP;
	dmg = maxDamage;
	Lives = maxLives;
	Walk_Speed = maxWalk_Speed;
}

float CPlayer2D::getlowerShieldTime()
{
	return ShieldCountDown;
}

float CPlayer2D::getExpBooster()
{
	return xpBooster;
}

int CPlayer2D::GetGold()
{
	return Gold;
}

void CPlayer2D::UseGold(int Gold_Used)
{
	Gold -= Gold_Used;
}

void CPlayer2D::IncreaseGold(int gold)
{
	this->Gold += gold;
}

CPlayer2D::CPlayer2D() :
	hp(0), maxHP(0), dmg(0), maxDamage(0),
	Walk_Speed(0), maxWalk_Speed(0),
	Lives(0), maxLives(0), Gold(0),
	xp(0), xpBooster(1), level(0),
	ShieldCountDown(0)
{
}

CPlayer2D::~CPlayer2D()
{
}
