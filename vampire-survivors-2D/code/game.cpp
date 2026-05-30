//#include "app.h"
#include "draw.h"
#include "Entities/entity.h"

fn main() -> s32 {
    App_Desc desc;
    desc.window.title = L"Survive 3D";
    app_init(desc);
    draw_init();

    Texture monk_run_texture;
    {
        Texture_Desc def;
        def.kind = Texture_Kind_Multiple;
        def.subtex_size = 192;
        def.filename = "Monk.png";
        gfx_init(&monk_run_texture, def);
    }

    s32 frame_count = monk_run_texture.subtexs.count;
    s32 anim_frames = 12;
    s32 curr_frame = 0;
    s32 last_frame = frame_count - 1;
    f32 frame_duration = 1.0f / (f32)anim_frames;
    f32 frame_timer = 0.0f;

    Mesh box_stack;
    Mesh randomGuy;
    Mesh bichito;
    Mesh bulletMesh;

    Player player = Player(randomGuy, 10, 1, 2, 1);
    //player.mesh = randomGuy;
    Asset_Handle mesh_shader = asset_create(Asset_Kind_Shader);
    Shader* mesh_shader_data = (Shader*)asset_get(mesh_shader);
    Shader_Desc mesh_shader_def{ "shader_mesh_lit.glsl" };
    gfx_init(mesh_shader_data, mesh_shader_def);

    mesh_init(&box_stack, "box_stack/box_stack.obj", mesh_shader);
    mesh_init(&player.mesh, "randomGuy.obj", mesh_shader);
    mesh_init(&bichito, "bichito.obj", mesh_shader);
    mesh_init(&bulletMesh, "bullet.obj", mesh_shader);

    Enemy enemy1 = Enemy(bichito, 5, 1, 1, Vec3(2, 0, 4));
    Enemy enemy2 = Enemy(bichito, 5, 1, 1, Vec3(2, 0, 4));
    Enemy enemy3 = Enemy(bichito, 5, 1, 1, Vec3(2, 0, 4));
    Enemy enemy4 = Enemy(bichito, 5, 1, 1, Vec3(2, 0, 4));
    Enemy enemy5 = Enemy(bichito, 5, 1, 1, Vec3(2, 0, 4));
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

    std::vector<Enemy*> enemyList;
    enemyList.push_back(&enemy1);
    enemyList.push_back(&enemy2);
    enemyList.push_back(&enemy3);
    enemyList.push_back(&enemy4);
    enemyList.push_back(&enemy5);

    std::vector<Bullet*> playerBullets;
    //playerBullets.push_back(&bullet1);

    set_depth_test_enabled();

    float box_spin = 0.f;

    while (app_running()) {

        frame_timer += os_delta_time();

        while (frame_timer >= frame_duration) {
            frame_timer -= frame_duration;
            curr_frame++;
            if (curr_frame >= frame_count) {
                curr_frame = 0;
            }
        }

        draw_update(os_delta_time());
        clear_back_buffer();
        draw_sprite(&monk_run_texture, curr_frame, Color.White, Mat4::transform(F32.Zero, F32.Zero, Vec3(F32.One) * 3.0f));

        box_spin += 30 * os_delta_time();
        player.Move(os_delta_time());
        draw_mesh(&player.mesh, Mat4::transform(player.pos, player.rot, Vec3(F32.One)));
        
        if (player.CanShoot(os_delta_time())) {

            if (playerBullets.empty()) {
                Bullet* bullet = player.Shoot(bulletMesh);
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
                    playerBullets.push_back(player.Shoot(bulletMesh));
                    playerBullets[playerBullets.size() - 1]->Respawn(player.pos, player.direction);
                }
            }
        }

        for (Enemy* enemy : enemyList) {
            enemy->Move(os_delta_time());
            
            draw_mesh(&enemy->mesh, Mat4::transform(enemy->pos, enemy->rot, Vec3(F32.One)));
        }

        if (playerBullets.size() > 0) {
            for (Bullet* bullet : playerBullets) {
                if (!bullet->isDead) {
                    bullet->Move(os_delta_time());
                    draw_mesh(&bullet->mesh, Mat4::transform(bullet->pos, F32.Zero, Vec3(F32.One)));

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

        draw_mesh(&box_stack, Mat4::transform(Vec3(0, 5, 30), Vec3(0, 0.5, 0) * box_spin, Vec3(F32.One)));
        draw_mesh(&box_stack, Mat4::transform(Vec3(30, 5, 0), Vec3(0, 0.5, 0) * box_spin, Vec3(F32.One)));
        draw_mesh(&box_stack, Mat4::transform(Vec3(-30, 5, 0), Vec3(0, 0.5, 0) * box_spin, Vec3(F32.One)));

        os_swap_buffers();
    }

    gfx_done(&monk_run_texture);
    free_all_assets();
    draw_done();
    app_done();
}