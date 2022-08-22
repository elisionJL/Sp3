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
	static const float GRAVITY_CONSTANT;

	enum gameStates {
		start = 0,
		difficultySelection,
		weaponselection,
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
	bool CheckCollision(Enemy* enemy1, Enemy* enemy2, double dt);
	bool CheckCollision(Enemy* enemy, GameObject* go);
	void CollisionResponse(GameObject* actor, GameObject* actee);
	void CollisionResponse(Enemy* actor, Enemy* actee, double dt);
	void MakeThickWall(float width, float height, const Vector3& normal, const Vector3& pos);
	void spawnPowerup(Vector3 pos);
	void RenderTitleScreen();
	void ShopInteraction(double dt);
	void ShopUI();
	void RenderGronkDialogue();
	void SpawnMapObjects();
	void PlayerMapCheck();
	void MapBoundary();
	void shooting(double elapsedTime, int numberofshots, GameObject* Gun);
	void PistolShooting(double elapsedTime, int numofshots);
	bool bulletcollisioncheck(GameObject* Gun, GameObject* Bullet, Enemy* go2);
	void dobulletcollision(GameObject* Gun, GameObject* Bullet, Enemy* go2);
	void DeleteEnemy(Enemy* Enemy);
	void DamageNumbers(int damage, Enemy* Enem);
	void RenderDmgNum(Vector3 dmgandtime);
	bool Movingofdamagenumbers(float posX, int dmg);
	void dragonshooting(int numberofshots, float strengthofproj, int piercing);
	float CalculateAdditionalForce(Enemy* Enemy, CPlayer2D* cPlayer2D);
	void MoveEnemiesToPlayer(Enemy* enemy, CPlayer2D* cPlayer2D, double dt);
protected:
	enum upgrades {
		atk = 0,
		hp,
		fireRate,
		pierce,
		moveSpeed,
		multishot,
		velocity,
		dragon
	};
	enum DIFFICULTY {
		easy = 1,
		hard = 2
	};
	Vector3 m_lineStart;
	//Physics
	std::vector<GameObject *> m_goList;
	std::vector<GameObject*> m_thickWallList;
	upgrades levelUpgrades[3];
	GameObject* Companion;
	GameObject* Gronk;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	DIFFICULTY difficulty;
	float elapsedTime;
	float prevTime;
	float timerbeforeweaponselect;
	float timerBeforeUpgrade;
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
	std::vector<Vector3> dmgandtimefordmgnumber;
	std::vector<double> timerfordmgnumber;
	int numberofbullets;
	int ShopUpgrades[6];
	bool PlayerBuy;
	bool PlayerHover, currentlyHovering;
	float shopClick;
	float testingexpbar, hptestingbar;
	float dmgofgun;
	int pierceforbullet;
	float velocityofbullet;
	float bowdrawstring;
	int displaynumberoffset;
	int displaynumberoffsety;
	int switchdmgnum;
	std::vector<Vector3> coordinatesofdamagenumbers;
	int bowframe;
	float firerate;
	bool MaxUpgrade;
	bool Transition;
	float SongVolumeChange;
	float timerforpistol;
	bool shootpistolspecial;
	float staggertimingforpistol;
	float timerfordragon;
};

#endif