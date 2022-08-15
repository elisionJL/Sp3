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
}

void CPlayer2D::Update(double dt)
{
	//if(Application::IsKeyPressed(VK_KEY_W))
	if (Application::IsKeyPressed('W')) {
		playerY += 10 * dt;
	}
	if (Application::IsKeyPressed('S')) {
		playerY -= 10 * dt;
	}
	if (Application::IsKeyPressed('D')) {
		playerX += 10 * dt;
	}
	if (Application::IsKeyPressed('A')) {
		playerX -= 10 * dt;
	}
	SpriteAnimation* sa = dynamic_cast<SpriteAnimation*>(playerMesh);
	sa->PlayAnimation("walkR", -1,1.f);
	sa->Update(dt);
}

void CPlayer2D::Render()
{
	modelStack.PushMatrix();
	modelStack.Translate(playerX, playerY, 0);
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

CPlayer2D::CPlayer2D():
	hp(0),
	dmg(0)
{
}

CPlayer2D::~CPlayer2D()
{
}
