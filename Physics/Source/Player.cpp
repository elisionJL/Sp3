#include "player.h"
#include "Application.h"
#include "SpriteAnimation.h"
#include "iostream"
void CPlayer2D::Init()
{
	maxHP = hp = 30;
	Walk_Speed = 10;
	dmg = 2;
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();
	pos.Set(m_worldWidth * 0.5, m_worldHeight * 0.5, 1);
	vel.Set(0, 0, 0);
	sFacingDirection = RIGHT;
	sCurrentState = IDLE;
	xp = 0;
	level = 1;
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
	if (xp >= ((level - 1) * 10) + 5)
	{
		xp -= ((level - 1) * 10) + 5;
		level += 1;
	}
	if (hp <= 0) {
		sa->PlayAnimation("death", 0, 2.f);
		sa->Update(dt);
		if (sa->getAnimationStatus("death") == true) {
			sCurrentState = DEAD;
		}
	}
	else if (hp > 0) {
		if (Application::IsKeyPressed('Q') && sCurrentState != DODGING) {
			sCurrentState = DODGING;
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
				sa->PlayAnimation("rollR", 0, 2.f);
				break;
			case LEFT:
				sa->PlayAnimation("rollL", 0, 2.f);
				break;
			}
			break;
		}
		sa->Update(dt);
		if (sCurrentState == DODGING) {
			pos += vel * 2 * dt;
			switch (sFacingDirection) {
			case RIGHT:
				if (sa->getAnimationStatus("rollR") == false) {
					sCurrentState = IDLE;
				}
				break;
			case LEFT:
				if (sa->getAnimationStatus("rollL") == false) {
					sCurrentState = IDLE;
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
	modelStack.PushMatrix();
	modelStack.Translate(pos.x, pos.y, 1);
	modelStack.Scale(10, 10, 1);
	RenderMesh(playerMesh, false);
	modelStack.PopMatrix();
}

void CPlayer2D::Exit()
{
}

void CPlayer2D::setmeshList(Mesh* meshlist)
{
	playerMesh = meshlist;
}

int CPlayer2D::getLevel()
{
	return level;
}

int CPlayer2D::getState()
{
	return sCurrentState;
}

CPlayer2D::CPlayer2D():
	hp(0),
	dmg(0)
{
}

CPlayer2D::~CPlayer2D()
{
}
