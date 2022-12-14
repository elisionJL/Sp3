#ifndef SCENE_COLLISION_H
#define SCENE_COLLISION_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"
#include "Player.h"
#include "Enemy.h"
#include <fstream>
// Include Keyboard controller
#include "../Common/Inputs/KeyboardController.h"
#include "../Common/Inputs/MouseController.h"
#include <iostream>
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
	bool CheckCollision(Enemy* Enemy, CPlayer2D* cPlayer2D);
	void CollisionResponse(GameObject* actor, GameObject* actee);
	void CollisionResponse(Enemy* actor, Enemy* actee, double dt);
	void spawnPowerup(Vector3 pos);
	void RenderTitleScreen();
	void ShopInteraction(double dt);
	void ShopUI();
	void RenderGronkDialogue();
	void SpawnMapObjects();
	void PlayerMapCheck();
	void MapBoundary();
	void MakeScreenShake();
	void MakeScreenShakeForMG();
	void shooting(double elapsedTime, int numberofshots, GameObject* Gun);
	void PistolShooting(double elapsedTime, int numofshots);
	bool bulletcollisioncheck(GameObject* Gun, GameObject* Bullet, Enemy* go2);
	void dobulletcollision(GameObject* Gun, GameObject* Bullet, Enemy* go2);
	void DeleteEnemy(Enemy* Enemy);
	void DamageNumbers(int damage, Enemy* Enem, bool critornot);
	void RenderDmgNum(Vector3 dmgandtime, bool yesorno);
	bool Movingofdamagenumbers(float posX, int dmg);
	void dragonshooting(int numberofshots, float strengthofproj, int piercing);
	float CalculateAdditionalForce(Enemy* Enemy, CPlayer2D* cPlayer2D);
	void MoveEnemiesToPlayer(Enemy* enemy, CPlayer2D* cPlayer2D, double dt);
	void MachineGunPewPew(double elapsedTime, int numofshots);
	void WritePlayerStats();
	void chest(Vector3 mousePos,float dt);
	void renderBossTraits(Vector3 mousePos);
	void generateTraits();
	void reset();
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
	enum traits {
		critRate = 0,
		critDamage,
		reverseShoot,
		hpUpMSDOWN,
		brokenShard,
		rateUpMSDown,
		regen
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
	traits traitsUpgrades[3];
	GameObject* Companion;
	GameObject* Gronk;
	GameObject* Shield;
	GameObject* ArrowToBoss;
	GameObject* ArrowInsideArrow;
	Enemy* Boss;
	int BossKilled;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	DIFFICULTY difficulty;
	int firerateUpgrade;
	int MSUpgrade;
	float hpScaling;
	float elapsedTime;
	float prevTime;
	float timerbeforeweaponselect;
	float timerBeforeUpgrade;
	int score;
	int minutes;
	float seconds;
	vector<Enemy*> enemyList;
	vector<Mesh*>enemyAnimationPlayed;
	vector<int> enemycurrentstate;
	//Get Player singleton Instance
	CPlayer2D* cPlayer2D;
	// Keyboard Controller singleton instance
	CKeyboardController* cKeyboardController;
	CMouseController* cMouseController;
	gameStates currentState;
	Vector3 m_gravity;
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
	bool chestOpened;
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
	std::vector<Vector3> timerfordmgnumber;
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
	bool MaxUpgrade[6];
	bool Transition;
	float SongVolumeChange;
	float timerforpistol;
	bool GunRightClickSpecial;
	float staggertimingforpistol;
	float timerfordragon;
	int acquiredGold;
	float screenShake[2];
	float PowerUsed;
	bool SuperPainPower;
	float shieldcooldowntimer;
	int killcounter;
	bool bossspawned;
	float surviveSeconds;
	double enemyspawn;
	float enemyspawnspeed;
	float enemyovertime;
};

#endif