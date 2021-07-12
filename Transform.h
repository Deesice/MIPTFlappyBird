#pragma once
#include "Component.h"
struct Vector2
{
    float x;
    float y;
    Vector2(float xPos = 0, float yPos = 0)
    {
        x = xPos;
        y = yPos;
    }
    Vector2 operator +(Vector2 v)
    {
        return Vector2(x + v.x, y + v.y);
    }
    Vector2 operator *(Vector2 v)
    {
        return Vector2(x * v.x, y * v.y);
    }
};
struct Transform :
    public Component
{
    Vector2 localPosition;
    Vector2 localScale;
    Transform* parent;
    Transform(GameObject* g) : Component(g)
    {
        parent = 0;
        localScale = Vector2(1, 1);
    }
    Vector2 Position()
    {
        Transform* t = this;
        Vector2 globalPosition;
        while (t != 0)
        {
            globalPosition = t->localPosition + globalPosition;
            t = t->parent;
        }
        return globalPosition;
    }
    Vector2 Scale()
    {
        Transform* t = this;
        Vector2 globalScale = Vector2(1,1);
        while (t != 0)
        {
            globalScale = t->localScale * globalScale;
            t = t->parent;
        }
        return globalScale;
    }
    void Start() {}
    void Update() {}
};

