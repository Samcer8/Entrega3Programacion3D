#include "entity.h"
#include <iostream>

bool Collision::CheckCollision(Collision otherCollider) {
	bool collisionX = this->posX + this->width >= otherCollider.posX && otherCollider.posX + otherCollider.width >= this->posX;
	bool collisionY = this->posY + this->height >= otherCollider.posY && otherCollider.posY + otherCollider.height >= this->posY;

	return collisionX && collisionY;
}

bool Collision::CheckCollision3D(Collision otherCollider) {
	bool collision2D = CheckCollision();
	bool collisionZ = this->posZ + this->length >= otherCollider.posZ && otherCollider.posZ + otherCollider.length >= this->posZ;

	return collision2D && collisionZ;
}

void Entity::RestartDefaultValues()
{
	health = maxHealth;
	isDead = false;
}

void Entity::ReceiveDamage(int damage)
{
	health -= damage;

	if (health <= 0) {
		isDead = true;
	}
}

Player::Player()
{
	tex = Texture();
	maxHealth = 0;
	health = maxHealth;
	damage = 0;
	speed = 0;
	pos = F32.Zero;
}

Player::Player(Texture tex, int maxHealth, int damage, int speed, float shootRate, Vec3 pos)
{
	this->tex = tex;
	this->maxHealth = maxHealth;
	health = maxHealth;
	this->damage = damage;
	this->speed = speed;
	this->pos = pos;
	this->shootRate = shootRate;

	collider.width = 0.5;
	collider.height = 0.5;
	collider.posX = pos.x;
	collider.posY = pos.y;
}

Player::Player(Mesh mesh, int maxHealth, int damage, int speed, float shootRate, Vec3 pos)
{
	this->mesh = mesh;
	this->maxHealth = maxHealth;
	health = maxHealth;
	this->damage = damage;
	this->speed = speed;
	this->pos = pos;
	this->shootRate = shootRate;

	collider.width = 0.5;
	collider.height = 0.5;
	collider.length = 0.5;
	collider.posX = pos.x;
	collider.posY = pos.y;
	collider.posZ = pos.z;
}

void Player::Move(float dt)
{
	direction = Vec3(0, 0, 0);

	if (os_key_down('W')) {
		pos += Vec3(0, 0, 1) * speed * dt;
		direction += Vec3(0, 0, 1);
	}
	if (os_key_down('S')) {
		pos += Vec3(0, 0, -1) * speed * dt;
		direction += Vec3(0, 0, -1);
	}
	if (os_key_down('A')) {
		pos += Vec3(-1, 0, 0) * speed * dt;
		direction += Vec3(-1, 0, 0);
	}
	if (os_key_down('D')) {
		pos += Vec3(1, 0, 0) * speed * dt;
		direction += Vec3(1, 0, 0);
	}

	collider.posX = pos.x;
	collider.posY = pos.y;
	collider.posZ = pos.z;

	Vec3 normalized = direction.normalized();

	if (_isnanf(normalized.x) || _isnanf(normalized.y)) {
		direction = Vec3(1, 0, 0);
	}
	else {
		direction = normalized;
	}

	//std::cout << direction.x << " " << direction.y << std::endl;
}

void Player::ReceiveDamage(int damage)
{
	Entity::ReceiveDamage(damage);

	if (health <= 0) {
		logf("Game Over");
	}
	else {
		logf("I received damage");
	}
}

Bullet* Player::Shoot(Texture tex)
{
	Bullet* bullet = new Bullet(tex, 1, 5, 2, pos, direction, 5);
	hasShot = true;
	return bullet;
}

bool Player::CanShoot(float dt)
{
	timePassed += dt;

	if(timePassed >= shootRate){
		timePassed = 0;
		hasShot = false;
		return true;
	}

	return false;
}

Enemy::Enemy()
{
	tex = Texture();
	maxHealth = 0;
	health = maxHealth;
	damage = 0;
	speed = 0;
	pos = F32.Zero;
}

Enemy::Enemy(Texture tex, int maxHealth, int damage, int speed, Vec3 pos)
{
	this->tex = tex;
	this->maxHealth = maxHealth;
	health = maxHealth;
	this->damage = damage;
	this->speed = speed;
	this->pos = pos;

	collider.width = 1;
	collider.height = 1;
	collider.posX = pos.x;
	collider.posY = pos.y;
}

Enemy::Enemy(Mesh mesh, int maxHealth, int damage, int speed, Vec3 pos)
{
	this->mesh = mesh;
	this->maxHealth = maxHealth;
	health = maxHealth;
	this->damage = damage;
	this->speed = speed;
	this->pos = pos;

	collider.width = 1;
	collider.height = 1;
	collider.length = 1;
	collider.posX = pos.x;
	collider.posY = pos.y;
	collider.posZ = pos.z;
}

Enemy::Enemy(Texture tex, int maxHealth, int damage, int speed, Player& target, Vec3 pos)
{
	this->tex = tex;
	this->maxHealth = maxHealth;
	health = maxHealth;
	this->damage = damage;
	this->speed = speed;
	this->pos = pos;

	collider.width = 1;
	collider.height = 1;
	collider.posX = pos.x;
	collider.posY = pos.y;

	this->target = &target;
}

Enemy::Enemy(Mesh mesh, int maxHealth, int damage, int speed, Player& target, Vec3 pos)
{
	this->mesh = mesh;
	this->maxHealth = maxHealth;
	health = maxHealth;
	this->damage = damage;
	this->speed = speed;
	this->pos = pos;

	collider.width = 1;
	collider.height = 1;
	collider.length = 1;
	collider.posX = pos.x;
	collider.posY = pos.y;
	collider.posZ = pos.z;

	this->target = &target;
}

void Enemy::Move(float dt)
{
	Vec3 dir = CalculateTargetDirection();

	pos += dir * speed * dt;
	collider.posX = pos.x;
	collider.posY = pos.y;
	collider.posZ = pos.z;

	if (collider.CheckCollision(target->collider)) {
		target->ReceiveDamage(damage);
		Respawn();
	}
}

void Enemy::ReceiveDamage(int damage)
{
	Entity::ReceiveDamage(damage);
	Respawn();
}

void Enemy::Respawn()
{
	RestartDefaultValues();
	int randX = rand() % 10 - 5;
	//std::cout << "Valor X" << randX << std::endl;
	int randZ = rand() % 10 - 5;
	//std::cout << "Valor Y" << randY << std::endl;

	randX >= 0 ? pos.x = randX + 10 + target->pos.x : pos.x = randX - 10 + target->pos.x;
	randZ >= 0 ? pos.z = randZ + 10 + target->pos.z : pos.z = randZ - 10 + target->pos.z;
}

Vec3 Enemy::CalculateTargetDirection() const
{
	Vec3 dir = target->pos - pos;
	Vec3 normalizedDir = dir.normalized();

	return normalizedDir;
}

Bullet::Bullet()
{
	tex = Texture();
	maxHealth = 0;
	health = maxHealth;
	damage = 0;
	speed = 0;
	pos = F32.Zero;
}

Bullet::Bullet(Texture tex, int maxHealth, int damage, int speed, Vec3 pos, Vec3 dir, float lifeTime)
{
	this->tex = tex;
	this->maxHealth = maxHealth;
	health = maxHealth;
	this->damage = damage;
	this->speed = speed;
	this->lifeTime = lifeTime;
	Respawn(pos, dir);

	collider.width = 0.5;
	collider.height = 0.5;
	collider.posX = pos.x;
	collider.posY = pos.y;
}

Bullet::Bullet(Mesh mesh, int maxHealth, int damage, int speed, Vec3 pos, Vec3 dir, float lifeTime)
{
	this->mesh = mesh;
	this->maxHealth = maxHealth;
	health = maxHealth;
	this->damage = damage;
	this->speed = speed;
	this->lifeTime = lifeTime;
	Respawn(pos, dir);

	collider.width = 0.5;
	collider.height = 0.5;
	collider.length = 0.5;
	collider.posX = pos.x;
	collider.posY = pos.y;
	collider.posZ = pos.z;
}

void Bullet::Move(float dt)
{
	
	pos += direction * speed * dt;
	collider.posX = pos.x;
	collider.posY = pos.y;
	collider.posZ = pos.z;
	CheckLifeTime(dt);
}

void Bullet::Respawn(Vec3 playerPosition, Vec3 playerDirection) {
	isDead = false;
	SetDirection(playerDirection);
	pos = playerPosition;
	//pos += direction * 2;
}

void Bullet::Despawn()
{
	isDead = true;
	aliveTime = 0;
}

void Bullet::CheckLifeTime(float dt)
{
	aliveTime += dt;

	if (aliveTime >= lifeTime) {
		Despawn();
	}
}

void Bullet::SetDirection(Vec3 playerDirection)
{
	direction = playerDirection;
}


