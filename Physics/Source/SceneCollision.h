#ifndef SCENE_COLLISION_H
#define SCENE_COLLISION_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"
class SceneCollision : public SceneBase
{
public:

	enum gameStates {
		start = 0,
		main,
		win,
		lose
	};
	SceneCollision();
	~SceneCollision();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();
	
	void RenderGO(GameObject *go);

	GameObject* FetchGO();
	void ReturnGO(GameObject *go);
	bool CheckCollision(GameObject* go1, GameObject* go2);
	void CollisionResponse(GameObject* actor, GameObject* actee);
	void MakeThinWall(float width, float height, const Vector3& normal, const Vector3& pos, bool real);
	void MakeBounceWall(float width, float height, const Vector3& normal, const Vector3& pos, Vector3& start, Vector3& end);
	void MakeThickWall(float width, float height, const Vector3& normal, const Vector3& pos);
	void spawnPowerup(Vector3 pos);
	void renderBricks();
protected:
	Vector3 m_lineStart;
	//Physics
	std::vector<GameObject *> m_goList;
	std::vector<GameObject*> m_thickWallList;
	std::vector<GameObject*> m_thinWallList;
	std::vector<GameObject*> m_bouncerList;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	int hp;
	int score;
	int minutes;
	float seconds;
	gameStates currentState;
	Vector3 m_gravity;
	GameObject* m_timeGO;
	int m_objectCount;
	float m_timeEstimated1;
	float m_timeTaken1;
	float m_timeEstimated2;
	float m_timeTaken2;
	float m_heightEstimated;
	float m_heightMax;
	float m_DistanceEstimated;
	float waveTime;
	int maxBalls;
	int ballcount;
	float rechargeBall;
	int activeWalls;
	int thickWall;
	int thinWall;
	float rechargeMulti;
	int extendMulti;
	float rechargeTime;
	float extendTime;
	//Auditing
	float m1, m2;
	Vector3 prevPos;
	Vector3 u1, u2, v1, v2;
};

#endif