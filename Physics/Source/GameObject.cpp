
#include "GameObject.h"

GameObject::GameObject(GAMEOBJECT_TYPE typeValue) 
	: type(typeValue),
	scale(1, 1, 1),
	active(false),
	mass(1.f),
	thickWall(-1),
	thinWall(-1),
	bounce(false),
	prevpos ((0, 0, 0)),
	PUIFrame(0)
{
}

void GameObject::fetchReturnStatus()
{
	pos.SetZero();
	vel.SetZero();
	scale.SetZero();
	normal.SetZero();
	color.SetZero();
	angle = 0;
	active = true;
	mass = 0;
	activeTime = 1.5;
	thickWall = 0;
	thinWall = -1;
	bounce = false;
	//Rotation variables
	direction.SetZero();
	angularVelocity = 0;
	momentOfInertia = 0;
	prevpos = (0, 0, 0);
	PUIFrame = 0;
	otherWall = nullptr;
	visible = true;
	placed = false;
}

GameObject::~GameObject()
{
}