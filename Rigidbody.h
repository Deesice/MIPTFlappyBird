#pragma once
#include "Component.h"
#include "Transform.h"
struct Rigidbody :
    public Component
{
    void Start() {}
    void Update()
    {
        gameObject->transform->localPosition = gameObject->transform->localPosition + Vector2(-1, -1);
    }
};

