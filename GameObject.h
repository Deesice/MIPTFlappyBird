#pragma once
#include "Component.h"
#include "Transform.h"
#include "iostream"
class GameObject
{
	Component** components;
	int componentCount;
public:
	Transform* transform;
	uint32_t color;
	char* Name;
	GameObject(const char* s = "")
	{
		components = new Component*[10];
		componentCount = 0;
		Name = new char [256];
		int i = 0;
		while (s[i] != 0)
		{
			Name[i] = s[i];
			i++;
		}
		Name[i] = 0;
		transform = AddComponent<Transform>();
	}
	~GameObject()
	{
		for (int i = 0; i < componentCount; i++)
			delete components[i];
		delete[] components;
	}
	template<class T> T* GetComponent()
	{
		T* component;
		for (int i = 0; i < componentCount; i++)
		{
			component = dynamic_cast<T>(components[i]);
			if (component)
				return component;
		}
		return component;
	}
	template<class T> T* AddComponent()
	{
		componentCount++;
		T* t = new T(this);
		components[componentCount - 1] = t;
		return t;
	}
};