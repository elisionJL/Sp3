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
	float xp;
	bool leveledUp;
	int getLevel();
	int getState();
protected:
	enum DIRECTION {
		RIGHT = 0,
		LEFT
	};
	CPlayer2D();
	~CPlayer2D();
	int level;
	STATES sCurrentState;
	DIRECTION sFacingDirection;
	Mesh* playerMesh;
	float m_worldWidth;
	float m_worldHeight;
	float hp, maxHP;
	float dmg;
	int Walk_Speed;
	Vector3 vel;

};