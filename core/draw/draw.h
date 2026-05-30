#pragma once
#include "graphics.h"
#include "asset/asset.h"

void draw_init();
void draw_done();

void draw_update(f32 dt);

void draw_quad(Vec3 pos, Vec3 rot, Vec3 scl);
void draw_sprite(const Texture* tex, s32 frame, Vec4 tint, const Mat4& transform, bool isPlayer = false);

struct Submesh {
    Asset_Handle material;
    u32 elem_offset = 0u;
    u32 elem_count = 0u;
};

struct Mesh {
    Vertex_Buffer vbo;
    Array<Submesh> submeshes;
};

struct Material {
    Asset_Handle shader;
    Asset_Handle texture_diffuse;
};

void mesh_init(Mesh* mesh, std::string_view filename, Asset_Handle shader, bool normals_as_colors = false);
void mesh_done(Mesh* mesh);
void draw_mesh(const Mesh* mesh, const Mat4& transform);