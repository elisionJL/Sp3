#pragma

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
	virtual void Exit();
	virtual void setmeshList(Mesh* player);
	float playerX;
	float playerY;
	int getState();
protected:
	enum DIRECTION {
		RIGHT = 0,
		LEFT
	};
	CPlayer2D();
	~CPlayer2D();
	STATES sCurrentState;
	DIRECTION sFacingDirection;
	Mesh* playerMesh;
	float m_worldWidth;
	float m_worldHeight;
	float hp;
	float dmg;

};