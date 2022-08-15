#pragma

#include "../Common/DesignPatterns/SingletonTemplate.h"
#include "GameObject.h"
#include <vector>
#include "SceneBase.h"
class CPlayer2D : public CSingletonTemplate<CPlayer2D> , public SceneBase {
	friend CSingletonTemplate<CPlayer2D>;
public:
	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();
	virtual void setmeshList(Mesh* player);
	float playerX;
	float playerY;
protected:
	CPlayer2D();
	~CPlayer2D();

	Mesh* playerMesh;
	float m_worldWidth;
	float m_worldHeight;
	float hp;
	float dmg;

};