#ifndef SCENE_COLLISION_H
#define SCENE_COLLISION_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"
#include "Player.h"
#include "Enemy.h"
class SceneCollision : public SceneBase
{
public:

	enum gameStates {
		start = 0,
		main,
		shop,
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
	void MakeThickWall(float width, float height, const Vector3& normal, const Vector3& pos);
	void spawnPowerup(Vector3 pos);
	void RenderTitleScreen();
	void RenderGronkDialogue();
	void SpawnTree();
	void shooting(double elapsedTime, int numberofshots, GameObject* Gun);
protected:
	Vector3 m_lineStart;
	//Physics
	std::vector<GameObject *> m_goList;
	std::vector<GameObject*> m_thickWallList;

	GameObject* Companion;
	GameObject* Gronk;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	
	float elapsedTime;
	float prevTime;

	int score;
	int minutes;
	float seconds;
	vector<Enemy*> enemyList;
	CPlayer2D* cPlayer2D;
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
	int thickWall;
	//Auditing
	float m1, m2;
	int flip;
	int rotationorder;
	float companionX, companionY;

	int DeathCount;
	float dialogueTime;

	GameObject* Gun;
	double GunShootingTimer;
	Mesh* CurrentGun;

	string GronkDialogue[50];

	Vector3 prevPos;
	Vector3 u1, u2, v1, v2;
	float calculateAngle(float x, float y);
	string OutputDialogue;
	bool CurrentTextWrite, TextFinished;
	int CurrentCharText;
	int randomDialogue;
	int shootonceonly;
	bool GunShoot;
	int GunFrameWhereItStarts;
	bool needtofinishanimation;
	bool xisneg;
	float zaxis;
	std::vector<double> timerforbullets;
	int numberofbullets;
	float testingexpbar, hptestingbar;
};

#endif