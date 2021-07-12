#pragma once
#include "GameObject.h"
struct Component
{
	GameObject* gameObject;
	Component(GameObject* g)
	{
		gameObject = g;
	}
	virtual void Start() = 0;
	virtual void Update() = 0;
};