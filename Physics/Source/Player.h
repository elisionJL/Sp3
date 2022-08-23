#pragma
#include "Vector3.h"
#include "../Common/DesignPatterns/SingletonTemplate.h"
#include "GameObject.h"
#include <vector>
#include "SceneBase.h"
class CPlayer2D : public CSingletonTemplate<CPlayer2D> , public SceneBase {
	friend CSingletonTemplate<CPlayer2D>;
public:
	enum STATES {
		MOVING = 0,
		IDLE,
		DODGING,
		DEAD
	};
	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	void CollisionDetectedUp(bool Collided);
	void CollisionDetectedDown(bool Collided);
	void CollisionDetectedLeft(bool Collided);
	void CollisionDetectedRight(bool Collided);
	virtual void Exit();
	virtual void setmeshList(Mesh* player);
	Vector3 pos;
	Vector3 vel;
	float xp;
	bool leveledUp;
	float hp, maxHP;
	float rollCooldown;
	float maxRollCooldown;
	int getLevel();
	void increaseLevel();
	int getState();
	void IncreaseSpd();
	void IncreaseHP();
	void DecreaseShieldCooldown();
	void IncreaseDmg();
	void IncreaseLifeCount();
	void IncreaseEXPGain();
	int GetGold();
	void UseGold(int Gold_Used);
	void IncreaseGold(float gold);
protected:
	enum DIRECTION {
		RIGHT = 0,
		LEFT
	};
	CPlayer2D();
	~CPlayer2D();
	int level;
	int Lives;
	int Gold;
	float elapsedTime;
	float prevTime;
	STATES sCurrentState;
	DIRECTION sFacingDirection;
	Mesh* playerMesh;
	float m_worldWidth;
	float m_worldHeight;
	float dmg;
	int Walk_Speed;

};