#include "SceneCollision.h"
#include "GL\glew.h"
#include "Application.h"
#include "LoadTexture.h"
#include <sstream>

SceneCollision::SceneCollision()
{
}

SceneCollision::~SceneCollision()
{
}

void SceneCollision::Init()
{
	SceneBase::Init();
	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();
	currentState = start;
	//Physics code here
	m_speed = 1.f;
	score = 0;
	thickWall = 0;
	thinWall = 0;
	activeWalls = 0;
	Math::InitRNG();
	m_objectCount = 0;
	waveTime = 11;
	maxBalls = 2;
	ballcount = 0;
	rechargeBall = 5;

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

void SceneCollision::ReturnGO(GameObject *go)
{
	if (go->active == true)
	{
		go->active = false;
		m_objectCount--;
	}
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

	//Mouse Section
	double x, y, windowwidth, windowheight;
	Application::GetCursorPos(&x, &y);
	windowwidth = Application::GetWindowWidth();
	windowheight = Application::GetWindowHeight();
	Vector3 mousePos = Vector3((x / windowwidth) * m_worldWidth, ((windowheight - y) / windowheight) * m_worldHeight, 0);
	switch (currentState) {
	case start:
	{
		static bool bLButtonState = false;
		if (!bLButtonState && Application::IsMousePressed(0)) {
			bLButtonState = true;
			if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.25 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.25) && (mousePos.y <= (m_worldHeight * 0.6) + 7.5 && mousePos.y >= (m_worldHeight * 0.6) - 7.5)) {
				currentState = main;
				hp = 10;
				score = 0;
				m_objectCount = 0;
				waveTime = 11;
				maxBalls = 2;
				ballcount = 0;
				rechargeBall = 5;
				thickWall = 0;
				thinWall = 0;
				activeWalls = 0;
				minutes = 2;
				seconds = 30;
				rechargeMulti = 1;
				rechargeTime = 0;
				extendTime = 0;
				extendMulti = 1;
			}
			else if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.25 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.25) && (mousePos.y <= (m_worldHeight * 0.3) + 7.5 && mousePos.y >= (m_worldHeight * 0.3) - 7.5)) {
				quit = true;
			}
		}
		else if (bLButtonState && !Application::IsMousePressed(0))
		{
			bLButtonState = false;
		}
		break;
	}
	case main:
	{
		seconds -= dt;
		if (minutes == 0 && seconds < 0) {
			currentState = win;
			break;
		}
		else if (seconds <= 0) {
			minutes -= 1;
			seconds += 60;
		}
		static bool bLButtonState = false;
		if (activeWalls < 15) {
			if (!bLButtonState && Application::IsMousePressed(0))
			{
				bLButtonState = true;
				std::cout << "LBUTTON DOWN" << std::endl;

				if (mousePos.y > m_worldHeight * 0.4f) {
					mousePos.y = m_worldHeight * 0.4f;
				}
				m_lineStart = mousePos;
			}
			else if (bLButtonState && !Application::IsMousePressed(0))
			{
				bLButtonState = false;
				std::cout << "LBUTTON UP" << std::endl;
				for (unsigned i = 0; i < m_goList.size(); ++i)
				{
					GameObject* go = m_goList[i];
					if (go->thinWall == 0) {
						if (go->placed == false) {
							go->active = false;
						}
					}
				}
				if (mousePos.y > m_worldHeight * 0.4f) {
					mousePos.y = m_worldHeight * 0.4f;
				}
				Vector3 vecLength = (mousePos - m_lineStart);
				if (vecLength.Length() > (15*extendMulti)) {
					vecLength *= ((15 * extendMulti) / vecLength.Length());
				}
				float length = vecLength.Length();
				float width = 3;
				if (vecLength.Length() > 0) {
					Vector3 normal = Vector3(-vecLength.y, vecLength.x, 0).Normalize();
					MakeThinWall(width, length, normal, m_lineStart + (vecLength * 0.5), true);
				}
			}

			//left mouse button held down
			if (Application::IsMousePressed(0)) {
				for (unsigned i = 0; i < m_goList.size(); ++i)
				{
					GameObject* go = m_goList[i];
					if (go->thinWall == 0) {
						if (go->placed == false) {
							go->active = false;
						}
					}
				}
				if (mousePos.y > m_worldHeight * 0.4f) {
					mousePos.y = m_worldHeight * 0.4f;
				}
				Vector3 vecLength = (mousePos - m_lineStart);
				if (vecLength.Length() > (15 * extendMulti)) {
					vecLength *= ((15 * extendMulti) / vecLength.Length());
				}
				float length = vecLength.Length();
				float width = 3;
				if (vecLength.Length() > 0) {
					Vector3 normal = Vector3(-vecLength.y, vecLength.x, 0).Normalize();
					MakeThinWall(width, length, normal, m_lineStart + (vecLength * 0.5), false);
				}
			}
			//right click to make bouncy walls
			static bool bRButtonState = false;
			if (!bRButtonState && Application::IsMousePressed(1)) {
				bRButtonState = true;
				if (mousePos.y > m_worldHeight * 0.4f) {
					mousePos.y = m_worldHeight * 0.4f;
				}
				m_lineStart = mousePos;
				prevPos = mousePos;
			}
			else if (bRButtonState && !Application::IsMousePressed(1)) {
				bRButtonState = false;
				for (unsigned i = 0; i < m_bouncerList.size(); ++i)
				{
					GameObject* go = m_bouncerList[i];
					go->active = false;
					go->linestart.SetZero();
					go->lineEnd.SetZero();
				}
				int size = m_bouncerList.size();
				for (unsigned i = 0; i < size; ++i)
				{
					m_bouncerList.erase(m_bouncerList.begin());
				}


			}
			if (Application::IsMousePressed(1)) {
				for (unsigned i = 0; i < m_bouncerList.size(); ++i)
				{
					GameObject* go = m_bouncerList[i];
					go->active = false;
					go->linestart.SetZero();
					go->lineEnd.SetZero();

				}
				int size = m_bouncerList.size();
				for (unsigned i = 0; i < size; ++i)
				{
					m_bouncerList.erase(m_bouncerList.begin());
				}
				if (mousePos.y > m_worldHeight * 0.4f) {
					mousePos.y = m_worldHeight * 0.4f;
				}
				Vector3 vecLength = (mousePos - m_lineStart);
				if (vecLength.Length() > 15) {
					vecLength *= (15 / vecLength.Length());
				}
				float length = vecLength.Length();
				float width = 3;
				if (vecLength.Length() > 0) {
					Vector3 normal = Vector3(-vecLength.y, vecLength.x, 0).Normalize();
					MakeBounceWall(width, length, normal, m_lineStart + (vecLength * 0.5), m_lineStart, mousePos);
					prevPos = mousePos;
				}
			}
		}
		if (rechargeBall < (5 * float((1 / rechargeMulti)))) {
			rechargeBall += dt;
		}
		static bool bSpaceButtonState = false;
		if (!bSpaceButtonState && Application::IsKeyPressed(VK_SPACE)) {
			bSpaceButtonState = true;
			if (ballcount < maxBalls && rechargeBall >= (5 * (1 / rechargeMulti))) {
				GameObject* ball = FetchGO();	
				ball->type = GameObject::GO_BALL;
				ball->mass = 5;
				ball->scale.Set(2, 2, 1);
				ball->vel.SetZero();
				ball->pos.Set(m_worldWidth * 0.5, m_worldHeight * 0.2, 0);
				++ballcount;
				rechargeBall = 0;
			}
		}
		else if (bSpaceButtonState && !Application::IsKeyPressed(VK_SPACE)) {
			bSpaceButtonState = false;
		}
		if (Application::IsKeyPressed(VK_F9)) {
			extendTime = 10;
			extendMulti = 2;
		}
		if (Application::IsKeyPressed(VK_F10)) {
			rechargeMulti = 2;
			rechargeTime = 15;
		}
		waveTime += dt;
		if (waveTime > 10) {
			waveTime = 0;
			renderBricks();
		}
		if (extendTime > 0) {
			extendTime -= dt;
			if (extendTime <= 0) {
				extendMulti = 1;
			}
		}
		if (rechargeTime > 0) {
			rechargeTime -= dt;
			if (rechargeTime <= 0) {
				rechargeMulti = 1;
			}
		}
		//Physics Simulation Section
		unsigned size = m_goList.size();
		ballcount = 0;
		for (unsigned i = 0; i < size; ++i)
		{
			GameObject* go = m_goList[i];
			if (go->active)
			{
				if (go->PUIFrame > 0) {
					go->PUIFrame -= dt;
					if (go->PUIFrame <= 0) {
						go->active = false;
						continue;
					}
				}
				if (go->placed == true && go->activeTime > 0 && go->thinWall > 0) {
					go->activeTime -= dt;
				}
				else if (go->placed == true && go->activeTime < 0 && go->thinWall > 0) {
					go->active = false;
					activeWalls--;
					continue;
				}
				go->pos += go->vel * dt * m_speed;
				if (go->pos.y < m_worldHeight * 0.4 && go->thickWall > 0) {
					int target = go->thickWall;
					int destroyed = 0;
					std::vector<GameObject*>::iterator prev = m_thickWallList.begin();
					std::vector<GameObject*>::iterator i = m_thickWallList.begin();
					while (destroyed != 6) {
						GameObject* go = (GameObject*)*i;
						if (go->thickWall == target) {
							go->active = false;
							if (i == prev) {
								m_thickWallList.erase(i);
								i = m_thickWallList.begin();
								prev = m_thickWallList.begin();
								++destroyed;
								continue;
							}
							else {
								m_thickWallList.erase(i);
								i = prev;
								++i;
								++destroyed;
							}
						}
						else {
							++i;
						}
						if (destroyed == 6) {
							break;
						}
						if (i == m_thickWallList.end()) {
							break;
						}
						prev = i;
					}
					hp -= 1;
				}
				// Handle X-Axis Bound
				if (((go->pos.x - go->scale.x < m_worldWidth*0.03) && (go->vel.x < 0)) ||
					((go->pos.x + go->scale.x > m_worldWidth* 0.97) && (go->vel.x > 0)))
				{
					go->vel.x = -go->vel.x;
				}

				if (go->pos.x < 0 || go->pos.x > m_worldWidth) {

					if (go->type = GameObject::GO_BALL) {
						if (ballcount > 0) {
							--ballcount;
						}
					}
					ReturnGO(go);
					continue;
				}

				// Handle Y-Axis Bound
				if (go->thickWall == 0) {
					if (((go->pos.y + go->scale.y > m_worldHeight) && go->vel.y > 0))
					{
						go->vel.y = -go->vel.y;
					}

					if (go->pos.y < 0 || go->pos.y > m_worldHeight)
					{
						if (go->type = GameObject::GO_BALL) {
							if (ballcount > 0) {
								--ballcount;
							}
						}
						ReturnGO(go);
						continue;
					}
				}
				else {
					if (go->pos.y < 0)
					{
						ReturnGO(go);
						continue;
					}
				}
				if (go->type == GameObject::GO_BALL) {
					++ballcount;
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
				
			}
		}
		if (hp <= 0) {
			currentState = lose;
		}
	}
	break;
	case win:
	{
		static bool bLButtonState = false;
		if (!bLButtonState && Application::IsMousePressed(0)) {
			bLButtonState = true;
			if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.25 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.25) && (mousePos.y <= (m_worldHeight * 0.6) + 7.5 && mousePos.y >= (m_worldHeight * 0.6) - 7.5)) {
				currentState = main;
				hp = 10;
				score = 0;
				m_objectCount = 0;
				waveTime = 11;
				maxBalls = 2;
				ballcount = 0;
				rechargeBall = 5;
				thickWall = 0;
				thinWall = 0;
				activeWalls = 0;
				minutes = 2;
				seconds = 30;
				rechargeMulti = 1;
				rechargeTime = 0;
				extendTime = 0;
				extendMulti = 1;
				m_goList.clear();
				m_bouncerList.clear();
				m_thinWallList.clear();
				m_thickWallList.clear();
			}
			else if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.25 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.25) && (mousePos.y <= (m_worldHeight * 0.3) + 7.5 && mousePos.y >= (m_worldHeight * 0.3) - 7.5)) {
				quit = true;
			}
		}
	}
		break;
	case lose:
	{
		static bool bLButtonState = false;
		if (!bLButtonState && Application::IsMousePressed(0)) {
			bLButtonState = true;
			if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.25 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.25) && (mousePos.y <= (m_worldHeight * 0.6) + 7.5 && mousePos.y >= (m_worldHeight * 0.6) - 7.5)) {
				currentState = main;
				hp = 10;
				score = 0;
				m_objectCount = 0;
				waveTime = 11;
				maxBalls = 2;
				ballcount = 0;
				rechargeBall = 5;
				thickWall = 0;
				thinWall = 0;
				activeWalls = 0;
				minutes = 2;
				seconds = 30;
				rechargeMulti = 1;
				rechargeTime = 0;
				extendTime = 0;
				extendMulti = 1;
				m_goList.clear();
				m_bouncerList.clear();
				m_thinWallList.clear();
				m_thickWallList.clear();
			}
			else if ((mousePos.x >= (m_worldWidth / 2) - m_worldWidth * 0.25 && mousePos.x <= (m_worldWidth / 2) + m_worldWidth * 0.25) && (mousePos.y <= (m_worldHeight * 0.3) + 7.5 && mousePos.y >= (m_worldHeight * 0.3) - 7.5)) {
				quit = true;
			}
		}
	}
		break;
	}
}

bool SceneCollision::CheckCollision(GameObject* go1, GameObject* go2) {
	if (go1->type != GameObject::GO_BALL) {
		return false;
	}
	if (go1->type == GameObject::GO_WALL && go2->type == GameObject::GO_PILLAR)
		std::cout << "collide" <<std::endl;
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
	default:
		return false;
		break;
	}
}
void SceneCollision::CollisionResponse(GameObject* go1, GameObject* go2)
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
		if (go2->thickWall != 0) {
			int target = go2->thickWall;
			int destroyed = 0;
			std::vector<GameObject*>::iterator prev = m_thickWallList.begin();
			std::vector<GameObject*>::iterator i = m_thickWallList.begin();
			while (destroyed != 6) {
				GameObject* go = (GameObject*)*i;
				if (go->thickWall == target) {
					go->active = false;
					if (i == prev) {
						m_thickWallList.erase(i);
						i = m_thickWallList.begin();
						prev = m_thickWallList.begin();
						++destroyed;
						if (destroyed == 6) {
							score += 100;
							spawnPowerup(go2->pos);
							break;
						}
						continue;
					}
					else {
						m_thickWallList.erase(i);
						i = prev;
						++i;
						++destroyed;
						if (destroyed == 6) {
							score += 100;
							spawnPowerup(go2->pos);
							break;
						}
					}
				}
				else {
					++i;
				}
				if (i == m_thickWallList.end()) {
					break;
				}
				prev = i;
			}
		}
		std::cout << "collided wall" << std::endl;
		break;
	}
	case GameObject::GO_PILLAR:
	{
		if (go2->bounce == true) {
			Vector3 projectedVec = Vector3::project(go1->pos - go2->linestart, go2->lineEnd);
			float dist = projectedVec.Length()/(go1->pos - go2->linestart).Length();
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
		if (go2->thickWall != 0) {
			int target = go2->thickWall;
			int destroyed = 0;
			std::vector<GameObject*>::iterator prev = m_thickWallList.begin();
			std::vector<GameObject*>::iterator i = m_thickWallList.begin();
			while (destroyed != 6) {
				GameObject* go = (GameObject*)*i;
				if (go->thickWall == target) {
					go->active = false;
					if (i == prev) {
						m_thickWallList.erase(i);
						i = m_thickWallList.begin();
						prev = m_thickWallList.begin();
						++destroyed;
						if (destroyed == 6) {
							score += 100;
							spawnPowerup(go2->pos);
							break;
						}
						continue;
					}
					else {
						m_thickWallList.erase(i);
						i = prev;
						++i;
						++destroyed;
						if (destroyed == 6) {
							score += 100;
							spawnPowerup(go2->pos);
							break;
						}
					}
				}
				else {
					++i;
				}
				if (i == m_thickWallList.end()) {
					break;
				}
				prev = i;
			}
			std::cout << "collided pillar" << std::endl;
		}
		break;
	}
	case GameObject::GO_POWERUP:
		switch (go2->PU) {
		case GameObject::heal:
			hp = 10;
			break;
		case GameObject::rechargeUp:
			rechargeMulti = 2;
			rechargeTime = 15.f;
			break;
		case GameObject::ballUp:
			++maxBalls;
			break;
		case GameObject::extend:
			extendTime = 10;
			extendMulti = 2;
			break;
		}
		go2->active = false;
		break;
	}
}
void SceneCollision::MakeBounceWall(float width, float height, const Vector3& normal, const Vector3& pos, Vector3& start, Vector3& end) {
	GameObject* go = FetchGO();
	go->type = GameObject::GO_WALL;
	go->scale.Set(width, height, 1.0f);
	go->pos = pos;
	std::cout << "x: " << normal.x << " y: " << normal.y << std::endl;
	go->normal = normal;
	go->vel.SetZero();
	go->linestart = start;
	go->lineEnd = end;
	go->prevpos = prevPos;
	go->mass = 5;

	Vector3 tangent(-normal.y, normal.x);
	GameObject* pillar1 = FetchGO();
	pillar1->type = GameObject::GO_PILLAR;
	pillar1->color.Set(0, 1, 0);
	pillar1->scale.Set(width * 0.5f, width * 0.5f, 1.f);
	pillar1->pos = pos + height * 0.5 * tangent;
	pillar1->vel.SetZero();
	pillar1->linestart = start;
	pillar1->prevpos = prevPos;
	pillar1->lineEnd = end;
	pillar1->mass = 5;

	//pillar 2
	GameObject* pillar2 = FetchGO();
	pillar2->type = GameObject::GO_PILLAR;
	pillar2->color.Set(0, 1, 0);
	pillar2->scale.Set(width * 0.5f, width * 0.5f, 1.f);
	pillar2->pos = pos - height * 0.5 * tangent;
	pillar2->vel.SetZero();
	pillar2->linestart = start;
	pillar2->prevpos = prevPos;
	pillar2->lineEnd = end;
	pillar2->mass = 5;

	go->thinWall = 0;
	pillar1->thinWall = 0;
	pillar2->thinWall = 0;
	go->placed = true;
	m_bouncerList.push_back(go);
	pillar1->placed = true;
	pillar2->placed = true;
	m_bouncerList.push_back(pillar1);
	m_bouncerList.push_back(pillar2);
	go->bounce = true;
	pillar1->bounce = true;
	pillar2->bounce = true;
}
void SceneCollision::MakeThinWall(float width, float height, const Vector3& normal, const Vector3& pos, bool real)
{
	GameObject* go = FetchGO();
	go->type = GameObject::GO_WALL;
	go->scale.Set(width, height, 1.0f);
	go->pos = pos;
	std::cout << "x: " << normal.x << " y: " << normal.y << std::endl;
	go->normal = normal;
	go->vel.SetZero();
	go->mass = 5;

	Vector3 tangent(-normal.y, normal.x);
	GameObject* pillar1 = FetchGO();
	pillar1->type = GameObject::GO_PILLAR;
	pillar1->color.Set(0, 1, 0);
	pillar1->scale.Set(width * 0.5f, width * 0.5f, 1.f);
	pillar1->pos = pos + height * 0.5 * tangent;
	pillar1->vel.SetZero();
	pillar1->mass = 5;

	//pillar 2
	GameObject* pillar2 = FetchGO();
	pillar2->type = GameObject::GO_PILLAR;
	pillar2->color.Set(0, 1, 0);
	pillar2->scale.Set(width * 0.5f, width * 0.5f, 1.f);
	pillar2->pos = pos - height* 0.5 * tangent;
	pillar2->vel.SetZero();
	pillar2->mass = 5;


	if (real == true) {
		if (thinWall == 11) {
			thinWall = 0;
		}
		thinWall++;
		go->thinWall = thinWall;
		pillar1->thinWall = thinWall;
		pillar2->thinWall = thinWall;
		go->placed = true;
		m_thinWallList.push_back(go);
		pillar1->placed = true;
		pillar2->placed = true;
		m_thinWallList.push_back(pillar1);
		m_thinWallList.push_back(pillar2);
		activeWalls += 3;
	}
	else
	{
		go->thinWall = 0;
		pillar1->thinWall = 0;
		pillar2->thinWall = 0;
		go->placed = false;
		pillar1->placed = false;
		pillar2->placed = false;
	}
}

void SceneCollision::MakeThickWall(float width, float height, const Vector3& normal, const Vector3& pos)
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
	wall1->vel = Vector3(0,-1,0);
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
	wall2->vel = Vector3(0,-1,0);
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
	pillar->scale.Set(size,size, 1.f);
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
void SceneCollision::renderBricks()
{
	float width = 13;
	float height = 5;
	//float maxWidth = width * asin(Math::DegreeToRadian(45)) + height * (asin(Math::DegreeToRadian(45)));
	int column = floorf((m_worldWidth * 0.9) / width);
	float gap = ((m_worldWidth * 0.9f) - (column * width)) / (column - 1);
	Vector3 brickpos;
	Vector3 normal;
	for (int i = 0; i < column; ++i) {
		float x = i * width + m_worldWidth * 0.05f + i * gap + width / 2;
		brickpos = Vector3(x, width / 2 + m_worldHeight, 0);
		//Vector3 normal = Vector3(Math::RandFloatMinMax(0, 0.8), Math::RandFloatMinMax(0, 0.8), 0);
		switch (Math::RandIntMinMax(0, 3)) {
		case 0:
			normal = Vector3(0, 1, 0);
			break;
		case 1:
			normal = Vector3(1, 0, 0);
			break;
		}
		std::cout << normal.x << "y: " << normal.y << std::endl;
		MakeThickWall(height, width, normal, brickpos);
	}

}
void SceneCollision::RenderGO(GameObject *go)
{
	//this is to render the animations for the mesh
	//In Update
	//sa = dynamic_cast<SpriteAnimation*>(meshList[GEO_BALL]);
	////Play the animation “ROW1” that is looping infinitely and
	////each animation completes in 2 sec
	//sa->PlayAnimation("ROW1", -1, 2.0f);
	//sa->Update(dt);

	switch(go->type)
	{
	case GameObject::GO_PILLAR:
	case GameObject::GO_BALL:
		//render normal asteroid
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, 0);
		modelStack.Scale(go->scale.x, go->scale.y, 1);
		RenderMesh(meshList[GEO_BALL], false);
		modelStack.PopMatrix();

		//Exercise 11: think of a way to give balls different colors
		break;
	case GameObject::GO_WALL:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, 2);
		modelStack.Rotate(Math::RadianToDegree(atan2f(go->normal.y, go->normal.x)), 0, 0, 1);
		//meshList[GEO_CUBE]->material.kAmbient(go->color.x, go->color.y, go->color.z);
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
		modelStack.Translate(go->pos.x, go->pos.y, 1);
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
	// Model matrix : an identity matrix (model will be at the origin)
	modelStack.LoadIdentity();

	RenderMesh(meshList[GEO_AXES], false);
	switch (currentState) {
	case start:
		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth * 0.5, m_worldHeight * 0.5f, 0);
		modelStack.Scale(m_worldWidth*0.9, m_worldHeight*0.9, 0);
		RenderMesh(meshList[GEO_MENU], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth / 2, m_worldHeight * 0.6, 1);
		modelStack.Scale(m_worldWidth * 0.5, 15, 1);
		RenderMesh(meshList[GEO_START], false);
		modelStack.PopMatrix();
		
		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth / 2, m_worldHeight * 0.3, 1);
		modelStack.Scale(m_worldWidth * 0.5, 15, 1);
		RenderMesh(meshList[GEO_QUIT], false);
		modelStack.PopMatrix();
		break;
	case main:
	{
		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth / 2, m_worldHeight * 0.5f, 0);
		modelStack.Scale(m_worldWidth, m_worldHeight, 0);
		RenderMesh(meshList[GEO_BG], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth / 2, m_worldHeight * 0.4f, 1);
		modelStack.Scale(m_worldWidth, 1, 0);
		RenderMesh(meshList[GEO_LINE], false);
		modelStack.PopMatrix();
		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (go->active)
			{
				RenderGO(go);
			}
		}
		float X = m_worldWidth * 0.3;
		float Y = m_worldHeight * 0.05;
		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth * 0.8, Y, 10);
		modelStack.Scale(X, 5, 10);
		RenderMesh(meshList[GEO_RED], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(m_worldWidth * 0.8, Y, 10.1);
		X = X * (rechargeBall / (5*(1/rechargeMulti)));
		if (X > m_worldWidth * 0.3) {
			X = m_worldWidth * 0.3;
		}
		modelStack.Scale(X, 5, 10);
		RenderMesh(meshList[GEO_GREEN], false);
		modelStack.PopMatrix();
		//On screen text
		std::ostringstream ss;
		ss.str("");
		ss << "hp: " << hp << "/" <<"10";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 2.5, 28, 0);

		ss.str("");
		ss << "balls:" << ballcount <<"/" << maxBalls;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1,0, 0), 2.5, 3, 0);
		ss.str("");
		ss.precision(5);
		ss << "FPS: " << fps;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 3, 0, 3);

		ss.str("");
		ss << "score: " << score;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 2.5, 3, 53);

		ss.str("");
		ss.precision(2);
		ss << minutes << ":" << seconds;
		if (seconds < 10) {
			ss.precision(1);
			ss.str("");
			ss << minutes << ":0" << seconds;
		}
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1,0,0), 3, 3, 56);
	}
	break;
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
	while(m_goList.size() > 0)
	{
		GameObject *go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
	
}
