#include "app.h"
#include "box2d/box2d.h"
#include "draw.h"
#include <iostream>

fn main() -> s32 {
    logf("Hellope!");

    srand(time(NULL));

    int windowSizeX = 1280;
    int windowSizeY = 720;

    app_init();

    draw_init();

    //Todo esto habría que meterlo en un struct
    Vec3 quad_pos;
    Vec3 quad_pos2 = Vec3(F32.Left) * 2.f;

    Vec3 direction = Vec3(F32.Right);
    Vec3 snakeScale = Vec3(1.f, 1.f, 0.0f);
    Vec3 appleScale = Vec3(0.3f, 0.3f, 0.0f);

    Collision appleCollision;
    appleCollision.width = appleScale.x;
    appleCollision.height = appleScale.y;
    
    float size = 1.f;
    float offsetSnake = 1.2f;
    //Posiciones actuales de los trozos de serpiente
    std::vector<Vec3> snakeLocations;

    Snake snake;
    SnakePiece head;
    head.isHead = true;
    head.currentPosition = quad_pos2;
    head.collider.posX = quad_pos2.x;
    head.collider.posY = quad_pos2.y;
    head.collider.width = snakeScale.x / 3;
    head.collider.height = snakeScale.y / 3;
    snake.pieces.push_back(head);

    int currentSnakeCount = 0;
    snakeLocations.push_back(quad_pos2);

    Direction playerDirection;

    while (app_running()) {
        glClear(GL_COLOR_BUFFER_BIT);

        if (snake.pieces[0].CheckCollision(appleCollision)) {
            logf("1 point!");

            int newX, newY;
            newX = rand() % 6;
            newY = rand() % 4;

            quad_pos = Vec3(newX, newY, 0);

            std::cout << quad_pos.x << " " << quad_pos.y << std::endl;

            appleCollision.posX = newX;
            appleCollision.posY = newY;

            for (int i = 0; i < 10; ++i) {
                SnakePiece newPiece = SnakePiece();
                newPiece.currentPosition = snake.pieces[snake.numPieces].lastPosition;
                newPiece.collider.posX = newPiece.currentPosition.x;
                newPiece.collider.posY = newPiece.currentPosition.y;
                //newPiece.collider.width = snakeScale.x / 100;
                //newPiece.collider.height = snakeScale.y / 100;

                snake.pieces.push_back(newPiece);

                ++currentSnakeCount;
                ++snake.numPieces;
            }
            
        }

        if (snake.numPieces > 1) {
            for (int i = 1; i < snake.numPieces; ++i) {
                if (snake.pieces[0].CheckCollision(snake.pieces[i].collider)) {
                    logf("You lost :(");
                }
            }
        }

        draw_quad(quad_pos, F32.Zero, appleScale);

        appleCollision.posX = quad_pos.x;
        appleCollision.posY = quad_pos.y;

        //Snake
        if (os_key_down(Key_Code::Arrow_Left)) {
            direction = Vec3(F32.Left);
        }
        else if (os_key_down(Key_Code::Arrow_Right)) {
            direction = Vec3(F32.Right);
        }
        else if (os_key_down(Key_Code::Arrow_Up)) {
            direction = Vec3(F32.Up);
        }
        else if (os_key_down(Key_Code::Arrow_Down)) {
            direction = Vec3(F32.Down);
        }

        snake.pieces[0].lastPosition = snake.pieces[0].currentPosition;
        snake.pieces[0].currentPosition += direction * os_delta_time();
        snake.pieces[0].collider.posX = snake.pieces[0].currentPosition.x;
        snake.pieces[0].collider.posY = snake.pieces[0].currentPosition.y;

        draw_quad(snake.pieces[0].currentPosition, F32.Zero, snakeScale);

        //Snake 2(?
        if (os_key_down(Key_Code::Space)) {
            SnakePiece newPiece = SnakePiece();
            newPiece.currentPosition = snake.pieces[snake.numPieces].lastPosition;

            snake.pieces.push_back(newPiece);

            ++currentSnakeCount;
            ++snake.numPieces;
        }

        if (snake.numPieces > 1) {
            for (int i = 1; i < snake.numPieces; ++i) {
                snake.pieces[i].currentPosition = snake.pieces[i - 1].lastPosition;

                draw_quad(snake.pieces[i].currentPosition, F32.Zero, snakeScale);
            }
        }

        for (int i = 0; i < snake.numPieces; ++i) {
            snake.pieces[i].lastPosition = snake.pieces[i].currentPosition;
        }

        os_swap_buffers();
    }

    draw_done();
    app_done();
}