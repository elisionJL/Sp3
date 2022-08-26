#ifndef SCENE_BASE_H
#define SCENE_BASE_H

#include "Scene.h"
#include "Mtx44.h"
#include "Camera.h"
#include "Mesh.h"
#include "MatrixStack.h"
#include "Light.h"
#include "GameObject.h"
#include <vector>

#include "../Physics/Source/SoundController/SoundController.h"

class SceneBase : public Scene
{
	enum UNIFORM_TYPE
	{
		U_MVP = 0,
		U_MODELVIEW,
		U_MODELVIEW_INVERSE_TRANSPOSE,
		U_MATERIAL_AMBIENT,
		U_MATERIAL_DIFFUSE,
		U_MATERIAL_SPECULAR,
		U_MATERIAL_SHININESS,
		U_LIGHTENABLED,
		U_NUMLIGHTS,
		U_LIGHT0_TYPE,
		U_LIGHT0_POSITION,
		U_LIGHT0_COLOR,
		U_LIGHT0_POWER,
		U_LIGHT0_KC,
		U_LIGHT0_KL,
		U_LIGHT0_KQ,
		U_LIGHT0_SPOTDIRECTION,
		U_LIGHT0_COSCUTOFF,
		U_LIGHT0_COSINNER,
		U_LIGHT0_EXPONENT,
		U_COLOR_TEXTURE_ENABLED,
		U_COLOR_TEXTURE,
		U_TEXT_ENABLED,
		U_TEXT_COLOR,
		U_TOTAL,
	};
public:
	enum GEOMETRY_TYPE
	{
		GEO_AXES,
		GEO_TEXT,
		GEO_BALL,
		GEO_CUBE,
		GEO_PLAYER,
		GEO_MENU,
		GEO_WIN,
		GEO_LOSE,
		GEO_START,
		GEO_RETURN,
		GEO_SHOP,
		GEO_SHOP_UI_BG,
		GEO_SHOP_SIGN,
		GEO_DIALOGUE_BOX,
		GEO_GRONK_BACK_BUTTON,
		GEO_TREE,
		GEO_ROCK,
		GEO_SPEED_UPGRADE,
		GEO_MAXHP_UPGRADE,
		GEO_SHIELD_UPGRADE,
		GEO_ATTACK_UPGRADE,
		GEO_EXTRALIFE_UPGRADE,
		GEO_EXPGAIN_UPGRADE,
		GEO_GOLD,
		GEO_GOLD_BG,
		GEO_RETRY,
		GEO_QUIT,
		GEO_BG,
		GEO_SHOP_BG,
		GEO_ATKUP,
		GEO_HPUP,
		GEO_MOVESPEED,
		GEO_PIERCE,
		GEO_MULTISHOT,
		GEO_FIRERATE,
		GEO_PISTOLBULLET,
		GEO_SNIPERBULLET,
		GEO_SHOTGUNBULLET,
		GEO_ARROW,
		GEO_LMGBULLET,
		GEO_DRAGONFIRE,
		GEO_COMPANIONUPGRADE,
		GEO_COMPANIONDAMAGE,
		GEO_VELUP,
		GEO_CRITDMG,
		GEO_CRTRATE,
		GEO_REGEN,
		GEO_RATEUPMSDOWN,
		GEO_JUGG,
		GEO_REVERSESHOT,
		GEO_SHAPEDGLASS,
		GEO_PORTAL,
		GEO_THIN,
		GEO_BOUNCE,
		GEO_EXTEND,
		GEO_WALL,
		GEO_COMPANION,
		GEO_SHOTGUN,
		GEO_SNIPER,
		GEO_BOW,
		GEO_PISTOL,
		GEO_SELECTED,
		GEO_GL,
		GEO_PAUSEPANEL,
		GEO_PAUSERESUME,
		GEO_PAUSEQUIT,
		GEO_STATPANEL,
		GEO_EXP,
		GEO_EXPBG,
		GEO_HEALTH,
		GEO_HEALTHBG,
		GEO_MANA,
		GEO_TRANSLUCENT,
		GEO_CARD,
		GEO_GRONK,
		GEO_BOSS_SLIME,
		GEO_EXPLOSION,
		GEO_LVLUPBG,
		GEO_BOUNDARY,
		GEO_TRANSITION,
		GEO_SUPERPAIN,
		GEO_diffSelect,
		GEO_CHEST,
		GEO_VAMPIRE,
		GEO_SKELETON,
		GEO_GHOST,
		GEO_SPIDER,
		GEO_ZOMBIE,
		GEO_SHIELD,
		GEO_ROLLBAR,
		GEO_MACHINEGUN,
		GEO_ROLL,
		GEO_BUTTONBG,
		GEO_DIFFICULTY,
		GEO_UPGRADESELECT,
		GEO_WHEREBOSS,
		GEO_SKELETONATTACK,
		GEO_ARROWTOBOSS,
		NUM_GEOMETRY,
	};
public:
	SceneBase();
	~SceneBase();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void RenderText(Mesh* mesh, std::string text, Color color);
	void RenderTextOnScreen(Mesh* mesh, std::string text, Color color, float size, float x, float y);
	void RenderDialogueOnScreen(Mesh* mesh, std::string text, Color color, float size, float x, float y);
	void RenderMesh(Mesh *mesh, bool enableLight);
	void RenderGO(GameObject *go);

	GameObject* FetchGO();
protected:
	unsigned m_vertexArrayID;
	Mesh* meshList[NUM_GEOMETRY];
	unsigned m_programID;
	unsigned m_parameters[U_TOTAL];

	CSoundController* cSoundController;
	Camera camera;

	MS modelStack;
	MS viewStack;
	MS projectionStack;

	Light lights[1];
	bool pause;
	bool bLightEnabled;

	float fps;
};

#endif