#pragma once
#include "core_pch.h"
#include "graphics.h"
#include "draw.h"
#include "os_gl.h"
#include "os_core.h"
#include "io_image.h"
#include "app.h"

struct Collision {
	float posX;
	float posY;
	float posZ;
	float width;
	float height;
	float length;

	bool CheckCollision(Collision otherCollider);
	bool CheckCollision3D(Collision otherCollider);
};

struct Entity {
	Texture tex;
	Mesh mesh;
	Vec3 pos;
	Collision collider;	

	int health;
	int maxHealth;
	int damage;
	bool isDead;
	float speed;

	void RestartDefaultValues();
	virtual void ReceiveDamage(int damage);
	virtual void Move(float dt) = 0;
};

struct Bullet : Entity {
	float lifeTime;
	float aliveTime = 0;
	Vec3 direction;

	Bullet();
	Bullet(Texture tex, int maxHealth, int damage, int speed, Vec3 pos, Vec3 dir, float lifeTime);
	Bullet(Mesh mesh, int maxHealth, int damage, int speed, Vec3 pos, Vec3 dir, float lifeTime);

	virtual void Move(float dt) override;
	void Respawn(Vec3 playerPosition, Vec3 playerDirection);
	void Despawn();
	void CheckLifeTime(float dt);
	void SetDirection(Vec3 playerDirection);
};

struct Player : Entity {
	Vec3 direction;
	float shootRate;
	float timePassed = 0;
	bool hasShot = false;

	Player();
	Player(Texture tex, int maxHealth, int damage, int speed, float shootRate, Vec3 pos = F32.Zero);
	Player(Mesh mesh, int maxHealth, int damage, int speed, float shootRate, Vec3 pos = F32.Zero);

	virtual void Move(float dt) override;
	virtual void ReceiveDamage(int damage) override;
	Bullet* Shoot(Texture tex);
	bool CanShoot(float dt);
};

struct Enemy : Entity {
	Player* target;

	Enemy();
	Enemy(Texture tex, int maxHealth, int damage, int speed, Vec3 pos = F32.Zero);
	Enemy(Mesh mesh, int maxHealth, int damage, int speed, Player& target, Vec3 pos = F32.Zero);

	virtual void Move(float dt) override;
	virtual void ReceiveDamage(int damage) override;
	void Respawn();
	Vec3 CalculateTargetDirection() const;
};

