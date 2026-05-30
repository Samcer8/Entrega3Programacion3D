#include "core_pch.h"
#include "draw.h"
#include "graphics/graphics.h"
#include "app.h"

constexpr int MAX_LIGHTS = 8;

struct {
    Vertex_Buffer vbo;
    Shader shader;
} quad;

struct {
    Camera camera;
    Free_Look editor_camera;
    s32 viewport_x = 1280;
    s32 viewport_y = 720;
    Global_Buffer gbo;
    Global_Buffer gbo_light_data;
    Texture white;

    struct {
        Mat4 projection = Mat.Identity4;
        Mat4 transform = Mat.Identity4;
        Vec2 uv_size = F32.One;
        Vec2 uv_offset = F32.Zero;
        Vec4 tint = Color.White;
        s32  tex_unit = 0;
    } global_data;

    struct
    {
        Vec4 colors[MAX_LIGHTS];
        Vec4 positions[MAX_LIGHTS];
        Vec4 view_pos;
    } light_data;
} scene;

Vertex_Buffer sprite_vbo;
Shader sprite_shader;
Shader mesh_shader;

Global_Buffer global_buffer;

struct {
    Mat4 projection = Mat.Identity4;
    Mat4 transform = Mat.Identity4;
    Vec2 uv_size = F32.One;
    Vec2 uv_offset;
    Vec4 tint = Color.White;
    s32 tex_unit = 0;
} shader_data;

void draw_init()
{
    // Quad init.
    {
        constexpr f32 verts[] = {
            -0.5f, -0.5f,  0.0f, 0.0f,
            +0.5f, -0.5f,  1.0f, 0.0f,
            +0.5f, +0.5f,  1.0f, 1.0f,
            -0.5f, +0.5f,  0.0f, 1.0f,
        };
        constexpr u32 elems[] = {
            0u, 1u, 2u, // Triangle 1
            2u, 3u, 0u, // Triangle 2
        };
        constexpr Data_Type attrs[] = {
            Data_Type::Float2,
            Data_Type::Float2,
        };

        Vertex_Buffer_Desc vbo_def = {
            { verts, sizeof(verts), /* vertex count */ 4 },
            { elems, /* element count */ 6 },
            { attrs, /* attribute count * */ 2 },
        };
        gfx_init(&quad.vbo, vbo_def);

        const char* shader_filename = "shader_sprite.glsl";
        gfx_init(&quad.shader, { shader_filename });

        gfx_init(&scene.gbo, { sizeof(scene.global_data) });
        gfx_init(&scene.gbo_light_data, { sizeof(scene.light_data) });
    }

    Texture_Desc def;
    def.image = io_image_white();
    gfx_init(&scene.white, def);

    {
        scene.camera = Camera(Camera::Perspective);
        scene.camera.pos = Vec3(0, 50, 0);
        scene.camera.rot = Vec3(-90, 0, 0);
        scene.editor_camera.init(&scene.camera);
        const char* shader_filename = "shader_mesh_lit.glsl";
        gfx_init(&mesh_shader, { shader_filename });
    }

    /* {
        Shader_Desc def;
        def.filename = "shader_mesh.glsl";
        gfx_init(&mesh_shader, def);
    }*/
}

void draw_done()
{
    gfx_done(&global_buffer);
    gfx_done(&sprite_shader);
    gfx_done(&sprite_vbo);
    gfx_done(&scene.gbo);
    gfx_done(&quad.vbo);
    gfx_done(&quad.shader);
}

void draw_update(f32 dt)
{
    bool use_editor_camera = os_key_down(Key_Code::Mouse_Right);

    for (const Input_Event& event : os_events_this_frame()) {
        if (use_editor_camera && event.type == Input_Event::Mouse_Move) {
            scene.editor_camera.rotate((f32)event.mouse_delta_x, (f32)event.mouse_delta_y);
        }
        if (event.type == Input_Event::Window) {
            scene.viewport_x = event.window_x;
            scene.viewport_y = event.window_y;
        }
    }
    if (!use_editor_camera) {  
        os_set_cursor_mode(Cursor_Mode::Default);

        if (os_key_down('W')) scene.camera.pos += Vec3(0, 0, -1) * 2 * dt;
        if (os_key_down('S')) scene.camera.pos += Vec3(0, 0, 1) * 2 * dt;
        if (os_key_down('A')) scene.camera.pos += Vec3(-1, 0, 0) * 2 * dt;
        if (os_key_down('D')) scene.camera.pos += Vec3(1, 0, 0) * 2 * dt;
    }
    else {
        os_set_cursor_mode(Cursor_Mode::Hidden);

        if (os_key_down('W')) scene.editor_camera.move_forward(dt);
        if (os_key_down('S')) scene.editor_camera.move_backward(dt);
        if (os_key_down('A')) scene.editor_camera.move_left(dt);
        if (os_key_down('D')) scene.editor_camera.move_right(dt);
        if (os_key_down(Key_Code::Space)) scene.editor_camera.move_up(dt);
        if (os_key_down(Key_Code::Shift)) scene.editor_camera.move_down(dt);
    }

    scene.camera.update_matrix(scene.viewport_x, scene.viewport_y);
    scene.global_data.projection = scene.camera.matrix();
    set_viewport(scene.viewport_x, scene.viewport_y);
}

void draw_quad(Vec3 pos, Vec3 rot, Vec3 scl)
{
    Mat4 transform = Mat.Identity4;

    shader_data.transform = Mat4::transpose(Mat4::transform(pos, rot, scl));

    gfx_update(global_buffer, &shader_data);

    gfx_use(sprite_shader);
    s32 samplers[32];
    for (s32 i = 0; i < 32; ++i) {
        samplers[i] = i;
    }

    gfx_set_param(sprite_shader, "u_samplers", 32, samplers);

    gfx_use(global_buffer);

    gfx_draw_call(sprite_vbo);
}

void draw_sprite(const Texture* tex, s32 frame, Vec4 tint, const Mat4& transform, bool isPlayer)
{
    set_blend_enabled();

    if (tex && tex->subtexs.count) {
        if (!ensuref(frame < tex->subtexs.count, "Error! subtex %i does not exist!", 20)) {
            return;
        }

        auto& subtex = tex->subtexs.data[frame];

        scene.global_data.uv_size = {
            (f32)subtex.width / (f32)tex->width,
            (f32)subtex.height / (f32)tex->height
        };

        scene.global_data.uv_offset = {
            (f32)subtex.x / (f32)tex->width,
            1.0f - ((f32)subtex.y + subtex.height) / tex->height
        };
    }

    scene.global_data.transform = Mat4::transpose(transform);
    scene.global_data.tint = tint;

    gfx_use(scene.white, 0u);

    if (tex) {
        scene.global_data.tex_unit = 1;
        gfx_use(*tex, /* unit */ 1u);
    }
    else {
        scene.global_data.tex_unit = 0;
    }

    gfx_use(quad.shader);

    s32 samplers[32];
    for (s32 i = 0; i < 32; ++i) {
        samplers[i] = i;
    }

    gfx_set_param(quad.shader, "u_samplers", 32, samplers);

    gfx_update(scene.gbo, &scene.global_data);
    gfx_use(scene.gbo);

    gfx_draw_call(quad.vbo);
}

void mesh_init(Mesh* mesh, std::string_view filename, Asset_Handle shader, bool normals_as_colors)
{
    IO_Model model;
    model.normals_as_colors = normals_as_colors;
    io_model_load(filename, &model);

    constexpr Data_Type attrs[] = {
        Data_Type::Float3,
        Data_Type::Float2,
        Data_Type::Float3,
        Data_Type::Float4,
    };

    Vertex_Buffer_Desc def;
    def.attributes.count = 4;
    def.attributes.data = attrs;
    def.vertices.count = model.vertices.size();
    def.vertices.data = model.vertices.data();
    def.vertices.size = sizeof(IO_Model_VTX) * def.vertices.count;
    def.indices.count = model.elems.size();
    def.indices.data = model.elems.data();
    gfx_init(&mesh->vbo, def);

    for (auto& shape : model.shapes) {

        Submesh& submesh = append(&mesh->submeshes);
        submesh.elem_count = shape.index_count;
        submesh.elem_offset = shape.index_offset;

        if (shader.kind == Asset_Kind_Shader && shader.value.index != 0) {

            Asset_Handle material = asset_create(Asset_Kind_Material);
            submesh.material = material;
            Material* material_data = (Material*)asset_get(material);
            material_data->shader = shader;

            auto& textures = shape.textures;
            std::string diffuse_name = textures.diffuse.empty() ? textures.ambient : textures.diffuse;

            if (!diffuse_name.empty()) {
                Asset_Handle diffuse = asset_create(Asset_Kind_Texture);
                Texture* diffuse_data = (Texture*)asset_get(diffuse);
                Texture_Desc diffuse_def;
                std::string model_dirpath = model.dirpath;
                diffuse_def.filename = model_dirpath.append("\\").append(diffuse_name);
                gfx_init(diffuse_data, diffuse_def);
                material_data->texture_diffuse = diffuse;
            }

        }
    }

    //io_model_done(&model);
    model.vertices.clear();
    model.elems.clear();
    model.shapes.clear();
    model = {};
}

void mesh_done(Mesh* mesh)
{
    reset(&mesh->submeshes);
    gfx_done(&mesh->vbo);
}

void draw_mesh(const Mesh* mesh, const Mat4& transform)
{
    scene.global_data.transform = Mat4::transpose(transform);
    scene.global_data.tex_unit = 0;

    s32 samplers[32];
    for (s32 i = 0; i < 32; ++i) {
        samplers[i] = i;
    }

    gfx_set_param(mesh_shader, "u_samplers", 32, samplers);

    gfx_update(scene.gbo, &scene.global_data);
    gfx_use(scene.gbo, 0);

    scene.light_data.colors[0] = Vec4(Color.Blue);
    scene.light_data.positions[0] = Vec4(0, 5, 30, 0);

    scene.light_data.colors[1] = Vec4(Color.Red);
    scene.light_data.positions[1] = Vec4(30, 5, 0, 0);

    scene.light_data.colors[2] = Vec4(Color.White_Faded);
    scene.light_data.positions[2] = Vec4(0, 10, 0, 0);

    scene.light_data.view_pos = Vec4(scene.camera.pos.x, scene.camera.pos.y, scene.camera.pos.z, 1.0);

    gfx_update(scene.gbo_light_data, &scene.light_data);
    gfx_use(scene.gbo_light_data, 1);

    for (auto& submesh : mesh->submeshes) {

        Material* material_data = (Material*)asset_get(submesh.material);
        Shader* shader_data = (Shader*)asset_get(material_data->shader);
        Texture* diffuse_data = (Texture*)asset_get(material_data->texture_diffuse);

        gfx_use(*diffuse_data, 0u);
        gfx_use(*shader_data);
        gfx_draw_call_complex(mesh->vbo, submesh.elem_count, submesh.elem_offset);
    }
}
