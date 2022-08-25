#include "SceneBase.h"
#include "GL\glew.h"

#include "shader.hpp"
#include "MeshBuilder.h"
#include "Application.h"
#include "Utility.h"
#include "LoadTGA.h"
#include "LoadTexture.h"
#include <sstream>

#include "../Common/System/filesystem.h"

SceneBase::SceneBase() : cSoundController(NULL)
{
}

SceneBase::~SceneBase()
{
	if (cSoundController)
	{
		cSoundController = NULL;
	}
}

void SceneBase::Init()
{
	quit = false;
	// Black background
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 
	
	glEnable(GL_CULL_FACE);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenVertexArrays(1, &m_vertexArrayID);
	glBindVertexArray(m_vertexArrayID);

	m_programID = LoadShaders( "Shader//comg.vertexshader", "Shader//comg.fragmentshader" );
	
	// Get a handle for our uniform
	m_parameters[U_MVP] = glGetUniformLocation(m_programID, "MVP");
	//m_parameters[U_MODEL] = glGetUniformLocation(m_programID, "M");
	//m_parameters[U_VIEW] = glGetUniformLocation(m_programID, "V");
	m_parameters[U_MODELVIEW] = glGetUniformLocation(m_programID, "MV");
	m_parameters[U_MODELVIEW_INVERSE_TRANSPOSE] = glGetUniformLocation(m_programID, "MV_inverse_transpose");
	m_parameters[U_MATERIAL_AMBIENT] = glGetUniformLocation(m_programID, "material.kAmbient");
	m_parameters[U_MATERIAL_DIFFUSE] = glGetUniformLocation(m_programID, "material.kDiffuse");
	m_parameters[U_MATERIAL_SPECULAR] = glGetUniformLocation(m_programID, "material.kSpecular");
	m_parameters[U_MATERIAL_SHININESS] = glGetUniformLocation(m_programID, "material.kShininess");
	m_parameters[U_LIGHTENABLED] = glGetUniformLocation(m_programID, "lightEnabled");
	m_parameters[U_NUMLIGHTS] = glGetUniformLocation(m_programID, "numLights");
	m_parameters[U_LIGHT0_TYPE] = glGetUniformLocation(m_programID, "lights[0].type");
	m_parameters[U_LIGHT0_POSITION] = glGetUniformLocation(m_programID, "lights[0].position_cameraspace");
	m_parameters[U_LIGHT0_COLOR] = glGetUniformLocation(m_programID, "lights[0].color");
	m_parameters[U_LIGHT0_POWER] = glGetUniformLocation(m_programID, "lights[0].power");
	m_parameters[U_LIGHT0_KC] = glGetUniformLocation(m_programID, "lights[0].kC");
	m_parameters[U_LIGHT0_KL] = glGetUniformLocation(m_programID, "lights[0].kL");
	m_parameters[U_LIGHT0_KQ] = glGetUniformLocation(m_programID, "lights[0].kQ");
	m_parameters[U_LIGHT0_SPOTDIRECTION] = glGetUniformLocation(m_programID, "lights[0].spotDirection");
	m_parameters[U_LIGHT0_COSCUTOFF] = glGetUniformLocation(m_programID, "lights[0].cosCutoff");
	m_parameters[U_LIGHT0_COSINNER] = glGetUniformLocation(m_programID, "lights[0].cosInner");
	m_parameters[U_LIGHT0_EXPONENT] = glGetUniformLocation(m_programID, "lights[0].exponent");
	// Get a handle for our "colorTexture" uniform
	m_parameters[U_COLOR_TEXTURE_ENABLED] = glGetUniformLocation(m_programID, "colorTextureEnabled");
	m_parameters[U_COLOR_TEXTURE] = glGetUniformLocation(m_programID, "colorTexture");
	// Get a handle for our "textColor" uniform
	m_parameters[U_TEXT_ENABLED] = glGetUniformLocation(m_programID, "textEnabled");
	m_parameters[U_TEXT_COLOR] = glGetUniformLocation(m_programID, "textColor");
	
	// Use our shader
	glUseProgram(m_programID);

	lights[0].type = Light::LIGHT_DIRECTIONAL;
	lights[0].position.Set(0, 20, 0);
	lights[0].color.Set(1, 1, 1);
	lights[0].power = 1;
	lights[0].kC = 1.f;
	lights[0].kL = 0.01f;
	lights[0].kQ = 0.001f;
	lights[0].cosCutoff = cos(Math::DegreeToRadian(45));
	lights[0].cosInner = cos(Math::DegreeToRadian(30));
	lights[0].exponent = 3.f;
	lights[0].spotDirection.Set(0.f, 1.f, 0.f);
	
	glUniform1i(m_parameters[U_NUMLIGHTS], 0);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 0);

	glUniform1i(m_parameters[U_LIGHT0_TYPE], lights[0].type);
	glUniform3fv(m_parameters[U_LIGHT0_COLOR], 1, &lights[0].color.r);
	glUniform1f(m_parameters[U_LIGHT0_POWER], lights[0].power);
	glUniform1f(m_parameters[U_LIGHT0_KC], lights[0].kC);
	glUniform1f(m_parameters[U_LIGHT0_KL], lights[0].kL);
	glUniform1f(m_parameters[U_LIGHT0_KQ], lights[0].kQ);
	glUniform1f(m_parameters[U_LIGHT0_COSCUTOFF], lights[0].cosCutoff);
	glUniform1f(m_parameters[U_LIGHT0_COSINNER], lights[0].cosInner);
	glUniform1f(m_parameters[U_LIGHT0_EXPONENT], lights[0].exponent);

	camera.Init(Vector3(0, 0, 1), Vector3(0, 0, 0), Vector3(0, 1, 0));

	for(int i = 0; i < NUM_GEOMETRY; ++i)
	{
		meshList[i] = NULL;
	}
	//to add animation do this

	//SpriteAnimation* sa = dynamic_cast<SpriteAnimation*>(meshList[GEO_BALL]);
	////Add the animation �ROW1� that start at 0 with 4 frameshjb
	//sa->AddAnimation("ROW1", 0, 4);

	meshList[GEO_AXES] = MeshBuilder::GenerateAxes("reference", 1000, 1000, 1000);
	meshList[GEO_BALL] = MeshBuilder::GenerateSphere("ball", Color(1, 1, 1), 10, 10, 1.f);
	meshList[GEO_CUBE] = MeshBuilder::GenerateCube("cube", Color(1, 1, 1), 1.f);

	meshList[GEO_WALL] = MeshBuilder::GenerateQuad("wall", Color(1, 1, 1), 1.f);
	meshList[GEO_WALL]->textureID = LoadTexture("Image//brick.png", true);

	meshList[GEO_PLAYER] = MeshBuilder::GenerateSpriteAnimation("player", 7, 10);
	meshList[GEO_PLAYER]->textureID = LoadTexture("Image//playerSpriteSheet.psd", true);
	meshList[GEO_PLAYER]->material.kAmbient.Set(1, 1, 1);
	SpriteAnimation* sa = dynamic_cast<SpriteAnimation*>(meshList[GEO_PLAYER]);
	sa->AddAnimation("idleR", 0, 7);
	sa->AddAnimation("idleL", 10, 17);
	sa->AddAnimation("walkR", 30, 39);
	sa->AddAnimation("walkL", 20, 29);
	sa->AddAnimation("rollL", 40, 48);
	sa->AddAnimation("rollR", 50, 58);
	sa->AddAnimation("death", 60, 67);

	meshList[GEO_CARD] = MeshBuilder::GenerateQuad("card", Color(1, 1, 1), 1.f);
	meshList[GEO_CARD]->textureID = LoadTexture("Image//upgrades//card.png", false);

	meshList[GEO_UPGRADESELECT] = MeshBuilder::GenerateQuad("upgradeSelect", Color(1, 1, 1), 1.f);
	meshList[GEO_UPGRADESELECT]->textureID = LoadTexture("Image//upgrades//upgradeSelect.png", true);

	meshList[GEO_DIFFICULTY] = MeshBuilder::GenerateQuad("difficulty", Color(1, 1, 1), 1.f);

	meshList[GEO_diffSelect] = MeshBuilder::GenerateQuad("diffSelect", Color(1, 1, 1), 1.f);
	meshList[GEO_diffSelect]->textureID = LoadTexture("Image//diffSelect.psd", true);

	meshList[GEO_ROLLBAR] = MeshBuilder::GenerateQuad("bar", Color(1,1,1),1.f);

	meshList[GEO_ROLL] = MeshBuilder::GenerateQuad("roll", Color(1, 1, 1), 1.f);
	meshList[GEO_ROLL]->textureID = LoadTexture("Image//roll.png", true);

	meshList[GEO_PORTAL] = MeshBuilder::GenerateQuad("portal", Color(1, 1, 1), 1.f);
	meshList[GEO_PORTAL]->textureID = LoadTexture("Image//portal.png", true);

	meshList[GEO_PAUSEPANEL] = MeshBuilder::GenerateQuad("pausePanel", Color(1, 1, 1), 1.f);
	meshList[GEO_PAUSEPANEL]->textureID = LoadTexture("Image//pause//panel.png", true);

	meshList[GEO_PISTOLBULLET] = MeshBuilder::GenerateQuad("pistolBullet", Color(1, 1, 1), 1.f);
	meshList[GEO_PISTOLBULLET]->textureID = LoadTexture("Image//bullet.png", true);

	meshList[GEO_SHOTGUNBULLET] = MeshBuilder::GenerateQuad("shotgunbul", Color(1, 1, 1), 1.f);
	meshList[GEO_SHOTGUNBULLET]->textureID = LoadTexture("Image//shotgunBullet.png", true);

	meshList[GEO_SNIPERBULLET] = MeshBuilder::GenerateQuad("SniperBullet", Color(1, 1, 1), 1.f);
	meshList[GEO_SNIPERBULLET]->textureID = LoadTexture("Image//sniperBullet.png", true);

	meshList[GEO_ARROW] = MeshBuilder::GenerateQuad("Arrow", Color(1, 1, 1), 1.f);
	meshList[GEO_ARROW]->textureID = LoadTexture("Image//arrow.png", true);

	meshList[GEO_LMGBULLET] = MeshBuilder::GenerateQuad("LMGBullet", Color(1, 1, 1), 1.f);
	meshList[GEO_LMGBULLET]->textureID = LoadTexture("Image//50CalBullet.png", true);

	meshList[GEO_DRAGONFIRE] = MeshBuilder::GenerateQuad("DragonFire", Color(1, 1, 1), 1.f);
	meshList[GEO_DRAGONFIRE]->textureID = LoadTexture("Image//firestatus.png", true);

	meshList[GEO_CRTRATE] = MeshBuilder::GenerateQuad("critRate", Color(1, 1, 1), 1.f);
	meshList[GEO_CRTRATE]->textureID = LoadTexture("Image//upgrades//traits//crtRate.png", true);

	meshList[GEO_CRITDMG] = MeshBuilder::GenerateQuad("critDamage", Color(1, 1, 1), 1.f);
	meshList[GEO_CRITDMG]->textureID = LoadTexture("Image//upgrades//traits//crtDmg.png", true);

	meshList[GEO_REGEN] = MeshBuilder::GenerateQuad("regen", Color(1, 1, 1), 1.f);
	meshList[GEO_REGEN]->textureID = LoadTexture("Image//upgrades//traits//regen.png", true);

	meshList[GEO_JUGG] = MeshBuilder::GenerateQuad("jugg", Color(1, 1, 1), 1.f);
	meshList[GEO_JUGG]->textureID = LoadTexture("Image//upgrades//traits//jugg.png", true);

	meshList[GEO_REVERSESHOT] = MeshBuilder::GenerateQuad("reverseShot", Color(1, 1, 1), 1.f);
	meshList[GEO_REVERSESHOT]->textureID = LoadTexture("Image//upgrades//traits//reverseShot.png", true);

	meshList[GEO_SHAPEDGLASS] = MeshBuilder::GenerateQuad("shapedGlass", Color(1, 1, 1), 1.f);
	meshList[GEO_SHAPEDGLASS]->textureID = LoadTexture("Image//upgrades//traits//shapedGlass.png", true);

	meshList[GEO_RATEUPMSDOWN] = MeshBuilder::GenerateQuad("rateupMSdown", Color(1, 1, 1), 1.f);
	meshList[GEO_RATEUPMSDOWN]->textureID = LoadTexture("Image//upgrades//traits//rateUpMSDown2.png", true);

	meshList[GEO_PIERCE] = MeshBuilder::GenerateQuad("pieceUpgrade", Color(1, 1, 1), 1.f);
	meshList[GEO_PIERCE]->textureID = LoadTexture("Image//upgrades//pierceUp.png", true);

	meshList[GEO_ATKUP] = MeshBuilder::GenerateQuad("atkUpgrade", Color(1, 1, 1), 1.f);
	meshList[GEO_ATKUP]->textureID = LoadTexture("Image//upgrades//atkUp.png", true);

	meshList[GEO_HPUP] = MeshBuilder::GenerateQuad("hpUpgrade", Color(1, 1, 1), 1.f);
	meshList[GEO_HPUP]->textureID = LoadTexture("Image//upgrades//hpUp.png", true);

	meshList[GEO_MULTISHOT] = MeshBuilder::GenerateQuad("multiShotUpgrade", Color(1, 1, 1), 1.f);
	meshList[GEO_MULTISHOT]->textureID = LoadTexture("Image//upgrades//multishot.png", true);

	meshList[GEO_MOVESPEED] = MeshBuilder::GenerateQuad("MoveSpeedUpgrade", Color(1, 1, 1), 1.f);
	meshList[GEO_MOVESPEED]->textureID = LoadTexture("Image//upgrades//moveSpeedUp.png", true);

	meshList[GEO_VELUP] = MeshBuilder::GenerateQuad("velUpgrade", Color(1, 1, 1), 1.f);
	meshList[GEO_VELUP]->textureID = LoadTexture("Image//upgrades//velUp.png", true);
	 
	meshList[GEO_FIRERATE] = MeshBuilder::GenerateQuad("fireUpgrade", Color(1, 1, 1), 1.f);
	meshList[GEO_FIRERATE]->textureID = LoadTexture("Image//upgrades//fireRateUp.png", true);

	meshList[GEO_COMPANIONUPGRADE] = MeshBuilder::GenerateQuad("companionUpgrade", Color(1, 1, 1), 1.f);
	meshList[GEO_COMPANIONUPGRADE]->textureID = LoadTexture("Image//upgrades//companion.png", true);

	meshList[GEO_COMPANIONDAMAGE] = MeshBuilder::GenerateQuad("CompanionDamageUpgrade", Color(1, 1, 1), 1.f);
	meshList[GEO_COMPANIONDAMAGE]->textureID = LoadTexture("Image//upgrades//companion2.png", true);

	meshList[GEO_PAUSERESUME] = MeshBuilder::GenerateQuad("pausePanel", Color(1, 1, 1), 1.f);
	meshList[GEO_PAUSERESUME]->textureID = LoadTexture("Image//pause//resume.psd", true);

	meshList[GEO_PAUSEQUIT] = MeshBuilder::GenerateQuad("pausePanel", Color(1, 1, 1), 1.f);
	meshList[GEO_PAUSEQUIT]->textureID = LoadTexture("Image//pause//quit.psd", true);

	meshList[GEO_STATPANEL] = MeshBuilder::GenerateQuad("pausePanel", Color(1, 1, 1), 1.f);
	meshList[GEO_STATPANEL]->textureID = LoadTexture("Image//pause//statPanel.png", true);

	meshList[GEO_BOUNCE] = MeshBuilder::GenerateQuad("bounce", Color(1, 1, 1), 1.f);
	meshList[GEO_BOUNCE]->textureID = LoadTexture("Image//bounceWall.png", true);

	meshList[GEO_THIN] = MeshBuilder::GenerateQuad("wall", Color(1, 1, 1), 1.f);
	meshList[GEO_THIN]->textureID = LoadTexture("Image//thinWall.png", true);

	meshList[GEO_START] = MeshBuilder::GenerateQuad("start", Color(1, 1, 1), 1.f);
	meshList[GEO_START]->textureID = LoadTexture("Image//Start_Game.png", true);

	meshList[GEO_SUPERPAIN] = MeshBuilder::GenerateQuad("superpain", Color(1, 1, 1), 1.f);
	meshList[GEO_SUPERPAIN]->textureID = LoadTexture("Image//Power_Up.png", true);

	meshList[GEO_RETRY] = MeshBuilder::GenerateQuad("retry", Color(1, 1, 1), 1.f);
	meshList[GEO_RETRY]->textureID = LoadTexture("Image//endScreens//retry.png", true);

	meshList[GEO_RETURN] = MeshBuilder::GenerateQuad("retry", Color(1, 1, 1), 1.f);
	meshList[GEO_RETURN]->textureID = LoadTexture("Image//endScreens//return.png", true);

	meshList[GEO_MENU] = MeshBuilder::GenerateQuad("menu", Color(1, 1, 1), 1.f);
	meshList[GEO_MENU]->textureID = LoadTexture("Image//Super_Pain.png", true);

	meshList[GEO_WIN] = MeshBuilder::GenerateQuad("win", Color(1, 1, 1), 1.f);
	meshList[GEO_WIN]->textureID = LoadTexture("Image//endScreens//winScreen.png", true);

	meshList[GEO_BUTTONBG] = MeshBuilder::GenerateQuad("button", Color(1, 1, 1), 1.f);
	meshList[GEO_BUTTONBG]->textureID = LoadTexture("Image//endScreens//button.png", true);

	meshList[GEO_LVLUPBG] = MeshBuilder::GenerateQuad("LVLUPBG", Color(1, 1, 1), 1.f);
	meshList[GEO_LVLUPBG]->textureID = LoadTexture("Image//Tree2.png", true);

	meshList[GEO_LOSE] = MeshBuilder::GenerateQuad("lose", Color(1, 1, 1), 1.f);
	meshList[GEO_LOSE]->textureID = LoadTexture("Image//endScreens//loseScreen.png", true);


	meshList[GEO_QUIT] = MeshBuilder::GenerateQuad("quit", Color(1, 1, 1), 1.f);
	meshList[GEO_QUIT]->textureID = LoadTexture("Image//Quit_Game.png", true);

	meshList[GEO_RETURN] = MeshBuilder::GenerateQuad("return", Color(1, 1, 1), 1.f);
	meshList[GEO_RETURN]->textureID = LoadTexture("Image//endScreens//return.png", true);

	meshList[GEO_SHOP] = MeshBuilder::GenerateQuad("shop", Color(1, 1, 1), 1.f);
	meshList[GEO_SHOP]->textureID = LoadTexture("Image//Shop.png", true);

	meshList[GEO_SHOP_BG] = MeshBuilder::GenerateQuad("shopbg", Color(1, 1, 1), 1.f);
	meshList[GEO_SHOP_BG]->textureID = LoadTexture("Image//Shop_BG.png", true);

	meshList[GEO_SHOP_UI_BG] = MeshBuilder::GenerateQuad("shopuibg", Color(1, 1, 1), 1.f);
	meshList[GEO_SHOP_UI_BG]->textureID = LoadTexture("Image//Shop_UI_BG.png", true);

	meshList[GEO_SPEED_UPGRADE] = MeshBuilder::GenerateQuad("upgradespd", Color(1, 1, 1), 1.f);
	meshList[GEO_SPEED_UPGRADE]->textureID = LoadTexture("Image//Shoe_Upgrade_UI.png", true);

	meshList[GEO_MAXHP_UPGRADE] = MeshBuilder::GenerateQuad("upgradehp", Color(1, 1, 1), 1.f);
	meshList[GEO_MAXHP_UPGRADE]->textureID = LoadTexture("Image//Heart_Upgrade_UI.png", true);

	meshList[GEO_SHIELD_UPGRADE] = MeshBuilder::GenerateQuad("shield", Color(1, 1, 1), 1.f);
	meshList[GEO_SHIELD_UPGRADE]->textureID = LoadTexture("Image//Shield_Upgrade_UI.png", true);

	meshList[GEO_ATTACK_UPGRADE] = MeshBuilder::GenerateQuad("attack", Color(1, 1, 1), 1.f);
	meshList[GEO_ATTACK_UPGRADE]->textureID = LoadTexture("Image//Attack_Upgrade_UI.png", true);

	meshList[GEO_EXTRALIFE_UPGRADE] = MeshBuilder::GenerateQuad("extralife", Color(1, 1, 1), 1.f);
	meshList[GEO_EXTRALIFE_UPGRADE]->textureID = LoadTexture("Image//ExtraLife_Upgrade_UI.png", true);

	meshList[GEO_EXPGAIN_UPGRADE] = MeshBuilder::GenerateQuad("expgainup", Color(1, 1, 1), 1.f);
	meshList[GEO_EXPGAIN_UPGRADE]->textureID = LoadTexture("Image//EXPGain_Upgrade_UI.png", true);

	meshList[GEO_GOLD] = MeshBuilder::GenerateQuad("gold", Color(1, 1, 1), 1.f);
	meshList[GEO_GOLD]->textureID = LoadTexture("Image//Gold.png", true);

	meshList[GEO_GOLD_BG] = MeshBuilder::GenerateQuad("goldbg", Color(1, 1, 1), 1.f);
	meshList[GEO_GOLD_BG]->textureID = LoadTexture("Image//Gold_BG.png", true);

	meshList[GEO_SHOP_SIGN] = MeshBuilder::GenerateQuad("signboard", Color(1, 1, 1), 1.f);
	meshList[GEO_SHOP_SIGN]->textureID = LoadTexture("Image//Wooden_Sign.png", true);

	meshList[GEO_DIALOGUE_BOX] = MeshBuilder::GenerateQuad("dialoguebox", Color(1, 1, 1), 1.f);
	meshList[GEO_DIALOGUE_BOX]->textureID = LoadTexture("Image//DialogueBox.png", true);

	meshList[GEO_GRONK_BACK_BUTTON] = MeshBuilder::GenerateQuad("gronkbackbutton", Color(1, 1, 1), 1.f);
	meshList[GEO_GRONK_BACK_BUTTON]->textureID = LoadTexture("Image//Gronk_Back_Button.png", true);

	meshList[GEO_EXPLOSION] = MeshBuilder::GenerateQuad("explosion", Color(1, 1, 1), 1.f);
	meshList[GEO_EXPLOSION]->textureID = LoadTexture("Image//Explosion.png", true);

	meshList[GEO_BG] = MeshBuilder::GenerateQuad("bg", Color(1, 1, 1), 1.f);
	meshList[GEO_BG]->textureID = LoadTexture("Image//Grass_BG.png", true);

	meshList[GEO_EXPBG] = MeshBuilder::GenerateQuad("expBarBG", Color(1, 1, 1), 1.f);
	meshList[GEO_EXPBG]->textureID = LoadTexture("Image//expBarBG.png", true);

	meshList[GEO_EXP] = MeshBuilder::GenerateQuad("expBar", Color(1, 1, 1), 1.f);
	meshList[GEO_EXP]->textureID = LoadTexture("Image//expBar.png", true);

	meshList[GEO_HEALTHBG] = MeshBuilder::GenerateQuad("healthBG", Color(1, 1, 1), 1.f);
	meshList[GEO_HEALTHBG]->textureID = LoadTexture("Image//healthBarBG.png", true);

	meshList[GEO_HEALTH] = MeshBuilder::GenerateQuad("Health", Color(1, 1, 1), 1.f);
	meshList[GEO_HEALTH]->textureID = LoadTexture("Image//healthBar.png", true);

	meshList[GEO_TREE] = MeshBuilder::GenerateQuad("tree", Color(1, 1, 1), 1.f);
	meshList[GEO_TREE]->textureID = LoadTexture("Image//Tree.png", true);

	meshList[GEO_ROCK] = MeshBuilder::GenerateQuad("rock", Color(1, 1, 1), 1.f);
	meshList[GEO_ROCK]->textureID = LoadTexture("Image//Rock.png", true);

	meshList[GEO_SELECTED] = MeshBuilder::GenerateQuad("selected", Color(1, 1, 1), 1.f);
	meshList[GEO_SELECTED]->textureID = LoadTexture("Image//Selected_UI.png", true);

	meshList[GEO_TEXT] = MeshBuilder::GenerateText("text", 16, 16);
	meshList[GEO_TEXT]->textureID = LoadTGA("Image//calibri.tga");
	meshList[GEO_TEXT]->material.kAmbient.Set(1, 0, 0);

	meshList[GEO_TRANSITION] = MeshBuilder::GenerateQuad("transition", Color(1, 1, 1), 1.f);
	meshList[GEO_TRANSITION]->textureID = LoadTexture("Image//Black.png", true);


	meshList[GEO_SHIELD] = MeshBuilder::GenerateQuad("shield", Color(1, 1, 1), 1.f);
	meshList[GEO_SHIELD]->textureID = LoadTexture("Image//Shield.png", true);

	meshList[GEO_SKELETONATTACK] = MeshBuilder::GenerateQuad("skeleattack", Color(1, 1, 1), 1.f);
	meshList[GEO_SKELETONATTACK]->textureID = LoadTexture("Image//Shield.png", true);
	meshList[GEO_SKELETONATTACK]->material.kAmbient.Set(1, 1, 0);

	meshList[GEO_WHEREBOSS] = MeshBuilder::GenerateQuad("arrow", Color(1, 1, 1), 1.f);
	meshList[GEO_WHEREBOSS]->textureID = LoadTexture("Image//WhereBoss.png", true);




	cSoundController = CSoundController::GetInstance();
	cSoundController->Init();

	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\Main_Menu.ogg"), 1, true, true); //Main Menu
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\Character_Select.ogg"), 2, true, true); //Character Select Music
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\Boss_Music.ogg"), 3, true, true); //Boss Music
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\Shop_Menu.ogg"), 4, true, true); //Shop Music
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\Battle_Music.ogg"), 5, true, true); //Battle Music
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\Buying.ogg"), 6, true); //Buy SFX
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\CannotBuy.ogg"), 7, true); //Cannot Buy SFX
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\GLSFX.ogg"), 8, true); //Cannot Buy SFX
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\BowSFX.ogg"), 9, true); //Cannot Buy SFX
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\ArrowSFX.ogg"), 10, true); //Cannot Buy SFX
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\ShotgunSFX.ogg"), 11, true); //Cannot Buy SFX
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\SniperSFX.ogg"), 12, true); //Cannot Buy SFX
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\RevolverSFX.ogg"), 13, true); //Cannot Buy SFX
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\Highlight.ogg"), 14, true); //Hover over button
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\Selected.ogg"), 15, true); //Button selected
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\SuperPainBomb.wav"), 16, true); //Button selected
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\MachineGun.ogg"), 17, true); //Machine Gun BRRRRR
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\FullerAuto.ogg"), 18, true); //Mini Gun BRRRRRR
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\GronkSpeak1.ogg"), 19, true); //Gronk Spek 1
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\GronkSpeak2.ogg"), 20, true); //Gronk Spek 2
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\GronkSpeak3.ogg"), 21, true); //Gronk Spek 3
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\DifficultyButtonClick.ogg"), 22, true); //Difficulty Select
	cSoundController->LoadSound(FileSystem::getPath("Music_SFX\\Death.ogg"), 23, true); //Difficulty Select

	meshList[GEO_COMPANION] = MeshBuilder::GenerateSpriteAnimation("Dragon", 5, 7);
	meshList[GEO_COMPANION]->textureID = LoadTexture("Image//Dragon.png", true);
	meshList[GEO_COMPANION]->material.kAmbient.Set(1, 1, 1);
	SpriteAnimation* Companion = dynamic_cast<SpriteAnimation*>(meshList[GEO_COMPANION]);
	//Add the animation �ROW1� that start at 0 with 4 frames
	Companion->AddAnimation("RunningR", 0, 7);
	Companion->AddAnimation("RunningL", 8, 14);
	Companion->AddAnimation("ShootR", 22, 28);
	Companion->AddAnimation("ShootL", 29, 35);

	//Enemy
	meshList[GEO_SKELETON] = MeshBuilder::GenerateQuad("skeleton", Color(1, 1, 1), 1.f);
	meshList[GEO_SKELETON]->textureID = LoadTexture("Image//SkeletonSS.png", true);

	meshList[GEO_GHOST] = MeshBuilder::GenerateQuad("ghost", Color(1, 1, 1), 1.f);
	meshList[GEO_GHOST]->textureID = LoadTexture("Image//GhostSS.png", true);

	meshList[GEO_ZOMBIE] = MeshBuilder::GenerateQuad("zombie", Color(1, 1, 1), 1.f);
	meshList[GEO_ZOMBIE]->textureID = LoadTexture("Image//ZombieSS.png", true);

	meshList[GEO_BOSS_SLIME] = MeshBuilder::GenerateQuad("slime", Color(1, 1, 1), 1.f);
	meshList[GEO_BOSS_SLIME]->textureID = LoadTexture("Image//boss_slime_sprites.png", true);

	meshList[GEO_SPIDER] = MeshBuilder::GenerateQuad("spider", Color(1, 1, 1), 1.f);
	meshList[GEO_SPIDER]->textureID = LoadTexture("Image//spiderSS.png", true);

	meshList[GEO_VAMPIRE] = MeshBuilder::GenerateQuad("vampire", Color(1, 1, 1), 1.f);
	meshList[GEO_VAMPIRE]->textureID = LoadTexture("Image//vampireSS.png", true);

	meshList[GEO_CHEST] = MeshBuilder::GenerateSpriteAnimation("Chest", 1, 4);
	meshList[GEO_CHEST]->textureID = LoadTexture("Image//Chest.png", true);
	meshList[GEO_CHEST]->material.kAmbient.Set(1, 1, 1);
	SpriteAnimation* Chest = dynamic_cast<SpriteAnimation*>(meshList[GEO_CHEST]);
	//Add the animation �ROW1� that start at 0 with 4 frames
	Chest->AddAnimation("Opening", 0, 4);

	meshList[GEO_BOW] = MeshBuilder::GenerateSpriteAnimation("Bow", 3, 6);
	meshList[GEO_BOW]->textureID = LoadTexture("Image//bow.png", true);
	meshList[GEO_BOW]->material.kAmbient.Set(1, 0, 0);
	SpriteAnimation* Bow = dynamic_cast<SpriteAnimation*>(meshList[GEO_BOW]);
	//Add the animation �ROW1� that start at 0 with 4 frames
	Bow->AddAnimation("Shoot", 0, 13);

	meshList[GEO_GL] = MeshBuilder::GenerateSpriteAnimation("GL", 2, 6);
	meshList[GEO_GL]->textureID = LoadTexture("Image//GL.png", true);
	meshList[GEO_GL]->material.kAmbient.Set(1, 1, 1);
	SpriteAnimation* GL = dynamic_cast<SpriteAnimation*>(meshList[GEO_GL]);
	//Add the animation �ROW1� that start at 0 with 4 frames
	GL->AddAnimation("Shoot", 0, 6);
	GL->AddAnimation("ShootR", 6, 12);

	meshList[GEO_MACHINEGUN] = MeshBuilder::GenerateSpriteAnimation("MG", 2, 6);
	meshList[GEO_MACHINEGUN]->textureID = LoadTexture("Image//MACHINEGUN.png", true);
	meshList[GEO_MACHINEGUN]->material.kAmbient.Set(1, 1, 1);
	SpriteAnimation* MG = dynamic_cast<SpriteAnimation*>(meshList[GEO_MACHINEGUN]);
	//Add the animation �ROW1� that start at 0 with 4 frames
	MG->AddAnimation("Shoot", 0, 6);
	MG->AddAnimation("ShootR", 6, 12);

	meshList[GEO_SHOTGUN] = MeshBuilder::GenerateSpriteAnimation("Shotgun", 2, 6);
	meshList[GEO_SHOTGUN]->textureID = LoadTexture("Image//Shotgun.png", true);
	meshList[GEO_SHOTGUN]->material.kAmbient.Set(1, 1, 1);
	SpriteAnimation* Shotgun = dynamic_cast<SpriteAnimation*>(meshList[GEO_SHOTGUN]);
	//Add the animation �ROW1� that start at 0 with 4 frames
	Shotgun->AddAnimation("Shoot", 0, 6);
	Shotgun->AddAnimation("ShootR", 6, 12);

	meshList[GEO_PISTOL] = MeshBuilder::GenerateSpriteAnimation("Revolver", 2, 6);
	meshList[GEO_PISTOL]->textureID = LoadTexture("Image//Revolver.png", true);
	meshList[GEO_PISTOL]->material.kAmbient.Set(1, 1, 1);
	SpriteAnimation* Pistol = dynamic_cast<SpriteAnimation*>(meshList[GEO_PISTOL]);
	//Add the animation �ROW1� that start at 0 with 4 frames
	Pistol->AddAnimation("Shoot", 0, 5);
	Pistol->AddAnimation("ShootR", 6, 12);

	meshList[GEO_SNIPER] = MeshBuilder::GenerateSpriteAnimation("Sniper", 2, 6);
	meshList[GEO_SNIPER]->textureID = LoadTexture("Image//Sniper.png", true);
	meshList[GEO_SNIPER]->material.kAmbient.Set(1, 1, 1);
	SpriteAnimation* Sniper = dynamic_cast<SpriteAnimation*>(meshList[GEO_SNIPER]);
	//Add the animation �ROW1� that start at 0 with 4 frames
	Sniper->AddAnimation("Shoot", 0, 5);
	Sniper->AddAnimation("ShootR", 6, 12);

	//Shopkeeper
	meshList[GEO_GRONK] = MeshBuilder::GenerateSpriteAnimation("Gronk", 1, 12);
	meshList[GEO_GRONK]->textureID = LoadTexture("Image//Gronk.png", true);
	meshList[GEO_GRONK]->material.kAmbient.Set(1, 1, 1);
	SpriteAnimation* gronk = dynamic_cast<SpriteAnimation*>(meshList[GEO_GRONK]);

	//Add the animation for gronk
	gronk->AddAnimation("Idle", 0, 12);

	//Boundary
	meshList[GEO_BOUNDARY] = MeshBuilder::GenerateSpriteAnimation("boundary", 2, 8);
	meshList[GEO_BOUNDARY]->textureID = LoadTexture("Image//Ocean.png", true);
	meshList[GEO_BOUNDARY]->material.kAmbient.Set(1, 1, 1);
	SpriteAnimation* ocean = dynamic_cast<SpriteAnimation*>(meshList[GEO_BOUNDARY]);
	//Add the animation for ocean boundary
	ocean->AddAnimation("Waves", 0, 16);

	bLightEnabled = true;
}

void SceneBase::Update(double dt)
{
	//Keyboard Section
	if(Application::IsKeyPressed('1'))
		glEnable(GL_CULL_FACE);
	if(Application::IsKeyPressed('2'))
		glDisable(GL_CULL_FACE);
	if(Application::IsKeyPressed('3'))
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if(Application::IsKeyPressed('4'))
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	fps = (float)(1.f / dt);
}

void SceneBase::RenderText(Mesh* mesh, std::string text, Color color)
{
	if(!mesh || mesh->textureID <= 0)
		return;
	
	glDisable(GL_DEPTH_TEST);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 1);
	glUniform3fv(m_parameters[U_TEXT_COLOR], 1, &color.r);
	glUniform1i(m_parameters[U_LIGHTENABLED], 0);
	glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mesh->textureID);
	glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);
	for(unsigned i = 0; i < text.length(); ++i)
	{
		Mtx44 characterSpacing;
		characterSpacing.SetToTranslation(i * 1.0f, 0, 0); //1.0f is the spacing of each character, you may change this value
		Mtx44 MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top() * characterSpacing;
		glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, &MVP.a[0]);
	
		mesh->Render((unsigned)text[i] * 6, 6);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 0);
	glEnable(GL_DEPTH_TEST);
}

void SceneBase::RenderTextOnScreen(Mesh* mesh, std::string text, Color color, float size, float x, float y)
{
	if(!mesh || mesh->textureID <= 0)
		return;

	glDisable(GL_DEPTH_TEST);
	Mtx44 ortho;
	ortho.SetToOrtho(0, 80, 0, 60, -10, 10);
	projectionStack.PushMatrix();
	projectionStack.LoadMatrix(ortho);
	viewStack.PushMatrix();
	viewStack.LoadIdentity();
	modelStack.PushMatrix();
	modelStack.LoadIdentity();
	modelStack.Translate(x, y, 0);
	modelStack.Scale(size, size, size);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 1);
	glUniform3fv(m_parameters[U_TEXT_COLOR], 1, &color.r);
	glUniform1i(m_parameters[U_LIGHTENABLED], 0);
	glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mesh->textureID);
	glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);
	for(unsigned i = 0; i < text.length(); ++i)
	{
		Mtx44 characterSpacing;
		characterSpacing.SetToTranslation(i * 1.0f + 0.5f, 0.5f, 0); //1.0f is the spacing of each character, you may change this value
		Mtx44 MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top() * characterSpacing;
		glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, &MVP.a[0]);
		mesh->Render((unsigned)text[i] * 6, 6);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 0);
	modelStack.PopMatrix();
	viewStack.PopMatrix();
	projectionStack.PopMatrix();
	glEnable(GL_DEPTH_TEST);
}

void SceneBase::RenderDialogueOnScreen(Mesh* mesh, std::string text, Color color, float size, float x, float y)
{
	if (!mesh || mesh->textureID <= 0)
		return;

	glDisable(GL_DEPTH_TEST);
	Mtx44 ortho;
	ortho.SetToOrtho(0, 80, 0, 60, -10, 10);
	projectionStack.PushMatrix();
	projectionStack.LoadMatrix(ortho);
	viewStack.PushMatrix();
	viewStack.LoadIdentity();
	modelStack.PushMatrix();
	modelStack.LoadIdentity();
	modelStack.Translate(x, y, 0);
	modelStack.Scale(size, size, size);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 1);
	glUniform3fv(m_parameters[U_TEXT_COLOR], 1, &color.r);
	glUniform1i(m_parameters[U_LIGHTENABLED], 0);
	glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mesh->textureID);
	glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);
	int NextLine;
	bool LineOne, LineTwo, LineThree, LineFour;
	LineOne = LineTwo = LineThree = LineFour = false;
	for (unsigned i = 0; i < text.length(); ++i)
	{
		Mtx44 characterSpacing;
		if (i < 20) {
			LineOne = true;
			characterSpacing.SetToTranslation(i * 1.0f + 0.5f, 0.5f, 0); //1.0f is the spacing of each character, you may change this value
		}
		else if (i >= 20 && text[i] != ' ' && LineTwo != true) {
			characterSpacing.SetToTranslation(i * 1.0f + 0.5f, 0.5f, 0); //1.0f is the spacing of each character, you may change this value
		}
		else if (i >= 20 && text[i] == ' ' && LineTwo != true) {
			NextLine = i + 1;
			LineTwo = true;
		}
		else if ((i >= 20 && i < 45) && LineTwo == true) {
			characterSpacing.SetToTranslation((i - NextLine) * 1.0f + 0.5f, -1.f, 0);
		}
		else if (i >= 45 && text[i] != ' ' && LineThree != true) {
			characterSpacing.SetToTranslation((i - NextLine) * 1.0f + 0.5f, -1.f, 0);
		}
		else if (i >= 45 && text[i] == ' ' && LineThree != true) {
			NextLine = i + 1;
			LineThree = true;
		}
		else if ((i >= 45 && i < 70) && LineThree == true) {
			characterSpacing.SetToTranslation((i - NextLine) * 1.0f + 0.5f, -2.5f, 0);
		}
		else if (i >= 70 && text[i] != ' ' && LineFour != true) {
			characterSpacing.SetToTranslation((i - NextLine) * 1.0f + 0.5f, -2.5f, 0);
		}
		else if (i >= 70 && text[i] == ' ' && LineFour != true) {
			NextLine = i + 1;
			LineFour = true;
		}
		//else if (text[i] == ' ') {
		//	NextLine = i;
		//}
		//else if ((i >= 40 && i < 60) && text[i] != ' ') {
		//	characterSpacing.SetToTranslation((i - NextLine) * 1.0f + 0.5f, -1.5f, 0);
		//}

		Mtx44 MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top() * characterSpacing;
		glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, &MVP.a[0]);
		mesh->Render((unsigned)text[i] * 6, 6);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 0);
	modelStack.PopMatrix();
	viewStack.PopMatrix();
	projectionStack.PopMatrix();
	glEnable(GL_DEPTH_TEST);
}

void SceneBase::RenderMesh(Mesh* mesh, bool enableLight)
{
	Mtx44 MVP, modelView, modelView_inverse_transpose;
	MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top();
	glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, &MVP.a[0]);
	if (enableLight && bLightEnabled)
	{
		glUniform1i(m_parameters[U_LIGHTENABLED], 1);
		modelView = viewStack.Top() * modelStack.Top();
		glUniformMatrix4fv(m_parameters[U_MODELVIEW], 1, GL_FALSE, &modelView.a[0]);
		modelView_inverse_transpose = modelView.GetInverse().GetTranspose();
		glUniformMatrix4fv(m_parameters[U_MODELVIEW_INVERSE_TRANSPOSE], 1, GL_FALSE, &modelView.a[0]);

		//load material
		glUniform3fv(m_parameters[U_MATERIAL_AMBIENT], 1, &mesh->material.kAmbient.r);
		glUniform3fv(m_parameters[U_MATERIAL_DIFFUSE], 1, &mesh->material.kDiffuse.r);
		glUniform3fv(m_parameters[U_MATERIAL_SPECULAR], 1, &mesh->material.kSpecular.r);
		glUniform1f(m_parameters[U_MATERIAL_SHININESS], mesh->material.kShininess);
	}
	else
	{
		glUniform1i(m_parameters[U_LIGHTENABLED], 0);
	}
	if (mesh->textureID > 0)
	{
		glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mesh->textureID);
		glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);
	}
	else
	{
		glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 0);
	}
	mesh->Render();
	if (mesh->textureID > 0)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void SceneBase::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SceneBase::Exit()
{
	// Cleanup VBO
	for(int i = 0; i < NUM_GEOMETRY; ++i)
	{
		if(meshList[i])
			delete meshList[i];
	}
	glDeleteProgram(m_programID);
	glDeleteVertexArrays(1, &m_vertexArrayID);
}
