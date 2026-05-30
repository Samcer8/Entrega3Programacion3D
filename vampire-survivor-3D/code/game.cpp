#include "app.h"
#include "draw.h"
#include "Entities/entity.h"

fn main() -> s32 {
    logf("Hellope!");

    srand(time(NULL));

    app_init();

    draw_init();

    /*Texture red_tower_texture;
    {
        Texture_Desc def;
        def.kind = Texture_Kind_Single;
        def.subtex_size = 0;
        def.filename = "sprites/Buildings/Red Buildings/Tower.png";
        gfx_init(&red_tower_texture, def);
    }*/

    Texture monk_run_texture;
    {
        Texture_Desc def2;
        def2.kind = Texture_Kind_Multiple;
        def2.subtex_size = 192;
        def2.filename = "sprites/Units/Blue Units/Monk/Run.png";
        gfx_init(&monk_run_texture, def2);
    }

    Player player = Player(monk_run_texture, 10, 2, 2, 2);

    Texture enemy_run_texture;
    {
        Texture_Desc def3;
        def3.kind = Texture_Kind_Multiple;
        def3.subtex_size = 192;
        def3.filename = "sprites/Units/Red Units/Monk/Run.png";
        gfx_init(&enemy_run_texture, def3);
    }

    Enemy enemy1 = Enemy(enemy_run_texture, 5, 1, 1, Vec3(2, 4, 0));
    Enemy enemy2 = Enemy(enemy_run_texture, 5, 1, 1, Vec3(2, 4, 0));
    Enemy enemy3 = Enemy(enemy_run_texture, 5, 1, 1, Vec3(2, 4, 0));
    Enemy enemy4 = Enemy(enemy_run_texture, 5, 1, 1, Vec3(2, 4, 0));
    Enemy enemy5 = Enemy(enemy_run_texture, 5, 1, 1, Vec3(2, 4, 0));
    enemy1.target = &player;
    enemy2.target = &player;
    enemy3.target = &player;
    enemy4.target = &player;
    enemy5.target = &player;
    enemy1.Respawn();
    enemy2.Respawn();
    enemy3.Respawn();
    enemy4.Respawn();
    enemy5.Respawn();

    Texture bullet_texture;
    {
        Texture_Desc def4;
        def4.kind = Texture_Kind_Multiple;
        def4.subtex_size = 64;
        def4.filename = "sprites/Particle FX/Fire_02.png";
        gfx_init(&bullet_texture, def4);
    }

    Bullet bullet1 = Bullet(bullet_texture, 1, 5, 4, player.pos, player.direction, 5);

    s32 frame_count = monk_run_texture.subtexs.count;
    s32 anim_frames = 12;
    s32 curr_frame = 0;
    s32 last_frame = frame_count - 1;
    f32 frame_duration = 1.0f / (f32)anim_frames;
    f32 frame_timer = 0.0f;

    std::vector<Enemy*> enemyList;
    enemyList.push_back(&enemy1);
    enemyList.push_back(&enemy2);
    enemyList.push_back(&enemy3);
    enemyList.push_back(&enemy4);
    enemyList.push_back(&enemy5);

    std::vector<Bullet*> playerBullets;
    playerBullets.push_back(&bullet1);

    while (app_running()) {
        f32 dt = os_delta_time();
        frame_timer += os_delta_time();

        while (frame_timer >= frame_duration) {
            frame_timer -= frame_duration;
            curr_frame++;
            if (curr_frame >= frame_count) {
                curr_frame = 0;
            }
        }

        draw_update(dt);
        clear_back_buffer();

        player.Move(dt);
        draw_sprite(&player.tex, curr_frame, Color.White, Mat4::transform(player.pos, F32.Zero, Vec3(F32.One) * 3.0f));
        
        if (player.CanShoot(dt)) {
            
            if (playerBullets.empty()) {
                Bullet* bullet = player.Shoot(bullet_texture);
                playerBullets.push_back(bullet);
                playerBullets[0]->Respawn(player.pos, player.direction);
            }
            else {
                for (Bullet* bullet : playerBullets) {
                    if (bullet->isDead) {
                        bullet->Respawn(player.pos, player.direction);
                        player.hasShot = true;
                        break;
                    }
                }

                if (!player.hasShot) {
                    playerBullets.push_back(player.Shoot(bullet_texture));
                    playerBullets[playerBullets.size() - 1]->Respawn(player.pos, player.direction);
                }
            }
        }

        for (Enemy* enemy : enemyList) {
            enemy->Move(dt);
            draw_sprite(&enemy->tex, curr_frame, Color.White, Mat4::transform(enemy->pos, F32.Zero, Vec3(F32.One) * 3.0f));
        }

        //Ya sé que esto es horrible
        if (playerBullets.size() > 0) {
            for (Bullet* bullet : playerBullets) {
                if (!bullet->isDead) {
                    bullet->Move(dt);
                    draw_sprite(&bullet->tex, curr_frame, Color.White, Mat4::transform(bullet->pos, F32.Zero, Vec3(F32.One) * 3.0f));

                    for (Enemy* enemy : enemyList) {
                        if (bullet->collider.CheckCollision(enemy->collider)) {
                            logf("Disparo!");
                            enemy->ReceiveDamage(bullet->damage);
                            break;
                        }
                    }
                }
            }
        }

        //draw_sprite(&red_tower_texture, 0, Color.White, Mat4::transform(Vec3(3, 3, 0), F32.Zero, Vec3(F32.One) * 3.0f));

        os_swap_buffers();
    }

    gfx_done(&monk_run_texture);
    gfx_done(&enemy_run_texture);
    //gfx_done(&red_tower_texture);
    gfx_done(&bullet_texture);
    draw_done();
    app_done();
    //while(true);
}