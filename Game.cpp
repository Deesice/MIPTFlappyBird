#include "Engine.h"
#include <stdlib.h>
#include <memory.h>
#include <ctime>
#include <cstdlib>
bool reset;
int points = 0;
struct Component;
class GameObject;
struct Transform;
struct Rigidbody;
template<class T> T Clamp(T value, T min, T max)
{
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}
template<class T> bool Between(T value, T min, T max)
{
	if (value < min)
		return false;
	if (value > max)
		return false;
	return true;
}
int Random(int minInclusive, int maxExlusive)
{
	return std::rand() % maxExlusive + minInclusive;
}
struct Component
{
	GameObject* gameObject;
	Component(GameObject* g)
	{
		gameObject = g;
	}
	virtual void Update(float dt) = 0;
	virtual void OnCollisionEnter(GameObject* collision) = 0;
	virtual void OnCollisionExit(GameObject* collision) = 0;
};
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
	Vector2 operator *(float f)
	{
		return Vector2(x * f, y * f);
	}
};
bool IsOverlapse(Vector2 pos1, Vector2 scale1, Vector2 pos2, Vector2 scale2)
{
	float minAx = pos1.x - scale1.x / 2;
	float maxAx = minAx + scale1.x;
	float minAy = pos1.y - scale1.y / 2;
	float maxAy = minAy + scale1.y;
	float minBx = pos2.x - scale2.x / 2;
	float maxBx = minBx + scale2.x;
	float minBy = pos2.y - scale2.y / 2;
	float maxBy = minBy + scale2.y;

	return maxAx >= minBx && minAx <= maxBx && minAy <= maxBy && maxAy >= minBy;
}
class Transform :
	public Component
{
	Transform* parent;
public:
	Vector2 localPosition;
	Vector2 localScale;
	Transform** children;
	int childrenCount;
	Transform(GameObject* g) : Component(g)
	{
		parent = 0;
		children = new Transform*[10];
		childrenCount = 0;
		localScale = Vector2(1, 1);
	}
	void SetParent(Transform* newParent)
	{
		if (parent)
		{
			bool shift = false;
			for(int i = 0; i < parent->childrenCount; i++)
			{
				if (parent->children[i] == this)
					shift = true;
				if (shift)
					parent->children[i] = parent->children[i + 1];
			}
		}
		parent = newParent;
		newParent->childrenCount++;
		newParent->children[newParent->childrenCount] = this;
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
		Vector2 globalScale = Vector2(1, 1);
		while (t != 0)
		{
			globalScale = t->localScale * globalScale;
			t = t->parent;
		}
		return globalScale;
	}
	void Update(float dt) {}
	void OnCollisionEnter(GameObject* collision) {}
	void OnCollisionExit(GameObject* collision) {}
	~Transform()
	{
		delete[] children;
	}
};
class GameObject
{
	Component** components;
	int componentCount;
public:
	Transform* transform;
	uint32_t color;
	bool render;
	char* Name;
	GameObject(const char* s)
	{
		render = true;
		components = new Component * [10];
		componentCount = 0;
		Name = new char[256];
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
	void Update(float dt)
	{
		for (int i = 0; i < componentCount; i++)
			components[i]->Update(dt);
	}
	void OnCollisionEnter(GameObject* collision)
	{
		for (int i = 0; i < componentCount; i++)
			components[i]->OnCollisionEnter(collision);
	}
	void OnCollisionExit(GameObject* collision)
	{
		for (int i = 0; i < componentCount; i++)
			components[i]->OnCollisionExit(collision);
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
struct Rigidbody : public Component
{
	float gravity = 1500;
	Vector2 velocity;
	Rigidbody(GameObject* g) : Component(g) {}
	void Update(float dt)
	{
		velocity = velocity + Vector2(0, -1) * dt * gravity;
		gameObject->transform->localPosition = gameObject->transform->localPosition + velocity * dt;
	}
	void OnCollisionEnter(GameObject* collision) {}
	void OnCollisionExit(GameObject* collision) {}
};
struct Input : public Component
{
	float force = 500;
	Rigidbody* rigidbody;
	bool flag = false;
	Input(GameObject* g) : Component(g) {}
	void Update(float dt)
	{
		if (is_key_pressed(VK_SPACE))
		{
			if (!flag)
			{
				flag = true;
				rigidbody->velocity = Vector2(0, force);
			}
		}
		else
		{
			flag = false;
		}
		if (gameObject->transform->localPosition.y < 0)
			reset = true;
	}
	void OnCollisionEnter(GameObject* collision) {}
	void OnCollisionExit(GameObject* collision) {}
};
struct Trigger : public Component
{
	GameObject* target;
	Trigger(GameObject* g) : Component(g) {}
	void Update(float dt){}
	void OnCollisionEnter(GameObject* collision)
	{
		if (target == collision)
			reset = true;
	}
	void OnCollisionExit(GameObject* collision)
	{
	}
};
struct PointZone : public Component
{
	char string[10];
	GameObject* target;
	PointZone(GameObject* g) : Component(g) {}
	void Update(float dt) {}
	void OnCollisionEnter(GameObject* collision)
	{
	}
	void OnCollisionExit(GameObject* collision)
	{
		if (target == collision)
		{
			points++;
			set_score(points);
		}
	}
};
struct Pipe : public Component
{
	float speed = 100;
	Pipe(GameObject* g) : Component(g) {}
	void Update(float dt)
	{
		gameObject->transform->localPosition = gameObject->transform->localPosition + Vector2(-1, 0) * dt * speed;
		if ((gameObject->transform->localPosition.x + gameObject->transform->localScale.x/2) <= 0)
			gameObject->transform->localPosition =
			Vector2(SCREEN_WIDTH + gameObject->transform->localScale.x / 2, Random(300, SCREEN_HEIGHT + 1 - 300));
	}
	void OnCollisionEnter(GameObject* collision)
	{
	}
	void OnCollisionExit(GameObject* collision)
	{
	}
};

//  is_key_pressed(int button_vk_code) - check if a key is pressed,
//                                       use keycodes (VK_SPACE, VK_RIGHT, VK_LEFT, VK_UP, VK_DOWN, 'A', 'B')
//
//  get_cursor_x(), get_cursor_y() - get mouse cursor position
//  is_mouse_button_pressed(int button) - check if mouse button is pressed (0 - left button, 1 - right button)
//  clear_buffer() - set all pixels in buffer to 'black'
//  is_window_active() - returns true if window is active
//  schedule_quit_game() - quit game after act()
GameObject** objects;
int objectCount;
bool** collisionTable;
GameObject* instantiate(const char* name = "")
{
	objectCount++;
	auto n = new GameObject * [objectCount];
	for (int i = 0; i < objectCount - 1; i++)
		n[i] = objects[i];
	n[objectCount - 1] = new GameObject(name);
	if (objectCount > 1)
		delete[] objects;
	objects = n;

	auto b = new bool* [objectCount];
	for (int i = 0; i < objectCount; i++)
	{
		b[i] = new bool[objectCount];
		if (i >= objectCount - 1)
		{
			for (int j = 0; j < objectCount; j++)
				b[i][j] = false;
			continue;
		}

		for (int j = 0; j < objectCount - 1; j++)
			b[i][j] = collisionTable[i][j];
		b[i][objectCount - 1] = false;
	}
	if (objectCount > 1)
	{
		for (int i = 0; i < objectCount - 1; i++)
			delete[] collisionTable[i];
		delete[] collisionTable;
	}
	collisionTable = b;

	return objects[objectCount - 1];
}
void CreatePipe(float xPos, GameObject* bird, float holeSize = 100, float pipeSize = 100)
{
	auto root = instantiate("Pipe");
	root->render = false;
	root->AddComponent<Pipe>();
	root->transform->localPosition = Vector2(xPos, Random(300, SCREEN_HEIGHT + 1 - 300));
	root->transform->localScale = Vector2(pipeSize, 1);

	auto pipe1 = instantiate("WallUpper");
	pipe1->color = 0xff00ffff;
	pipe1->transform->localPosition = Vector2(0, (SCREEN_HEIGHT * 100 + holeSize) / 2);
	pipe1->transform->localScale = Vector2(1, SCREEN_HEIGHT * 100);
	pipe1->transform->SetParent(root->transform);
	pipe1->AddComponent<Trigger>()->target = bird;

	auto pipe2= instantiate("WallLower");
	pipe2->color = 0xff00ffff;
	pipe2->transform->localPosition = Vector2(0, -(SCREEN_HEIGHT * 100 + holeSize) / 2);
	pipe2->transform->localScale = Vector2(1, SCREEN_HEIGHT * 100);
	pipe2->transform->SetParent(root->transform);
	pipe2->AddComponent<Trigger>()->target = bird;
	//return;
	auto pointZone = instantiate("PointZone");
	pointZone->AddComponent<PointZone>()->target = bird;
	pointZone->transform->SetParent(root->transform);
	pointZone->color = 0xffffffff;
	pointZone->transform->localPosition = Vector2(0, 0);
	pointZone->transform->localScale = Vector2(1, holeSize);
}
void initialize()
{
	std::srand(std::time(nullptr));
	points = 0;
	set_score(0);
    auto player = instantiate("Player");
	player->color = 0xffff00ff;
	player->transform->localScale = Vector2(50,50);
	player->transform->localPosition = Vector2(SCREEN_WIDTH / 6, SCREEN_HEIGHT / 2);
	player->AddComponent<Input>()->rigidbody = objects[0]->AddComponent<Rigidbody>();
	//////////////////
	for (int i = 0; i < 3; i++)
		CreatePipe(SCREEN_WIDTH + SCREEN_WIDTH * ((i + 0.5f)/ 3), objects[0], 300);
}
// this function is called to update game data,
// dt - time elapsed since the previous update (in seconds)
void act(float dt)
{
	for (int i = 0; i < objectCount; i++)
		objects[i]->Update(dt);

	Vector2 posA;
	Vector2 scaleA;
	Vector2 posB;
	Vector2 scaleB;

	for (int i = 0; i < objectCount; i++)
	{
		posA = objects[i]->transform->Position();
		scaleA = objects[i]->transform->Scale();
		for (int j = 0; j < objectCount; j++)
		{
			if (j == i)
				continue;
			posB = objects[j]->transform->Position();
			scaleB = objects[j]->transform->Scale();
			if (IsOverlapse(posA, scaleA, posB, scaleB) != collisionTable[i][j])
			{
				collisionTable[i][j] = !collisionTable[i][j];
				if (collisionTable[i][j])
					objects[i]->OnCollisionEnter(objects[j]);
				else
					objects[i]->OnCollisionExit(objects[j]);
			}
		}
	}

	if (is_key_pressed(VK_ESCAPE))
		schedule_quit_game();

	if (reset)
	{
		reset = false;		
		finalize();
		initialize();
	}
}

// fill buffer in this function
// uint32_t buffer[SCREEN_HEIGHT][SCREEN_WIDTH] - is an array of 32-bit colors (8 bits per R, G, B)
void draw()
{
  // clear backbuffer
  memset(buffer, 0, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t));
  Vector2 pos;
  Vector2 scale;
  int posX;
  int posY;
  for (int i = 0; i < objectCount; i++)
  {
	  if (!objects[i]->render)
		  continue;

      pos = objects[i]->transform->Position();
      scale = objects[i]->transform->Scale();
	  posX = pos.x - scale.x / 2;
	  posY = pos.y + scale.y / 2;
	  posY = SCREEN_HEIGHT - posY;
      for (int x = Clamp(posX, 0, SCREEN_WIDTH - 1); x < Clamp((int)scale.x + posX, 0, SCREEN_WIDTH - 1); x++)
		  for (int y = Clamp(posY, 0, SCREEN_HEIGHT - 1); y < Clamp((int)scale.y + posY, 0, SCREEN_HEIGHT - 1); y++)
		  {
				buffer[y][x] = objects[i]->color;
		  }
  }
}

// free game data in this function
void finalize()
{
    for (int i = 0; i < objectCount; i++)
        delete objects[i];
    delete[] objects;
	for (int i = 0; i < objectCount; i++)
		delete[] collisionTable[i];
	delete[] collisionTable;
	objectCount = 0;
}

