#pragma once

#include "core_pch.h"

//Tendré que mover esto de aqui luego
enum Direction {
	Up,
	Down,
	Right,
	Left,
};

struct Collision {
	float posX;
	float posY;
	float width;
	float height;
};

struct PlayerMove {
	Direction moveDirection;
	Vec3 position;
};

struct SnakePiece {
	Vec3 lastPosition;
	Vec3 currentPosition;
	Direction direction;
	bool isHead;
	Collision collider;

	SnakePiece::SnakePiece() {
		isHead = false;
	};

	SnakePiece::SnakePiece(Vec3 lastPosition, Vec3 currentPosition, Direction direction, bool isHead = false) {
		this->lastPosition = lastPosition;
		this->currentPosition = currentPosition;
		this->direction = direction;
		this->isHead = isHead;
	};

	bool SnakePiece::CheckCollision(Collision otherCollider) {
		bool collisionX = this->collider.posX + this->collider.width >= otherCollider.posX && otherCollider.posX + otherCollider.width >= this->collider.posX;
		bool collisionY = this->collider.posY + this->collider.height >= otherCollider.posY && otherCollider.posY + otherCollider.height >= this->collider.posY;

		return collisionX && collisionY;
	}
};

struct Snake {
	std::vector<SnakePiece> pieces;
	int numPieces = 0;
	std::vector<PlayerMove> moves;
};
