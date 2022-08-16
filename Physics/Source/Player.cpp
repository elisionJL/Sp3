#include "player.h"
#include "Application.h"
#include "SpriteAnimation.h"

void CPlayer2D::Init()
{
	hp = 30;
	dmg = 2;
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();
	playerX = m_worldWidth * 0.5;
	playerY = m_worldHeight * 0.5;
	sFacingDirection = RIGHT;
	sCurrentState = IDLE;
}

void CPlayer2D::Update(double dt)
{
	SpriteAnimation* sa = dynamic_cast<SpriteAnimation*>(playerMesh);
	if (Application::IsKeyPressed('L')) {
		hp = 0;
	}
	if (hp <= 0) {
		sa->PlayAnimation("death", 0, 2.f);
		sa->Update(dt);
		if (sa->getAnimationStatus("death") == true) {
			sCurrentState = DEAD;
		}
	}
	else if (hp > 0) {
		if (Application::IsKeyPressed('W')) {
			playerY += 10 * dt;
			sCurrentState = MOVING;
		}
		else if (Application::IsKeyPressed('S')) {
			playerY -= 10 * dt;
			sCurrentState = MOVING;
		}
		else {
			sCurrentState = IDLE;
		}
		if (Application::IsKeyPressed('D')) {
			sFacingDirection = RIGHT;
			sCurrentState = MOVING;
			playerX += 10 * dt;
		}
		else if (Application::IsKeyPressed('A')) {
			sFacingDirection = LEFT;
			sCurrentState = MOVING;
			playerX -= 10 * dt;
		}
		else {
			if (sCurrentState == IDLE) {
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
		}
	}

	sa->Update(dt);
}

void CPlayer2D::Render()
{
	modelStack.PushMatrix();
	modelStack.Translate(playerX, playerY, 1);
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
