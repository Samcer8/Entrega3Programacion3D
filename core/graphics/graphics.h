#pragma once

// Here we add the OpenGL guard, just in case 
// we implement another graphics API later. 
// These structs would have different members
// variables depending on the API.

// --------------------------------------------

// *** Objects ***

#ifdef GAME_GL 

// Vertex Buffer.
struct Vertex_Buffer {
    u32 vao = 0u;
    u32 vbo = 0u;
    u32 ebo = 0u;
    u32 count = 0u; // Triangle count.
};

// Global Buffer (Also called Uniform Buffer in OpenGL).
struct Global_Buffer {
    u32 ubo = 0u;
    u32 size = 0u;
};

// Shader.
struct Shader {
    u32 pgm = 0u;
};

#else
#   error "Unsupported graphics API!"
#endif 

// --------------------------------------------

// *** Descriptors ***

// Vertex Data Array.
struct Verts_View {
    const void* data = nullptr;
    u32 size = 0u;
    u32 count = 0u;
};

// Element Data Array
struct Elems_View {
    const u32* data = nullptr;
    u32 count = 0u;
};

// Vertex Attributes Array.
struct Attrs_View {
    const Data_Type* data = nullptr; 
    u32 count = 0u;    
};

// Vertex Buffer Descriptor:
// A Vertex Buffer needs to this be initialized
struct Vertex_Buffer_Desc {
    Verts_View vertices;
    Elems_View indices;
    Attrs_View attributes;
};

// Global Buffer Descriptor:
// A Global Buffer needs this to be initialized.
struct Global_Buffer_Desc {
    u32 size = 0u;
};

// Shader Buffer Descriptor:
// A Shader needs this to be initialized.
struct Shader_Desc {
    std::string_view filename;
    std::string_view source;
};

enum Texture_Filter {
    Linear = 0,
    Nearest,
};

enum Texture_Kind : u8 {
    Texture_Kind_Single = 0,
    Texture_Kind_Multiple
};

struct Texture_Desc {
    Texture_Filter filter = Texture_Filter::Nearest;
    Texture_Kind kind = Texture_Kind_Single;
    const struct IO_Image* image = nullptr;
    std::string_view filename;
    s32 subtex_size = 0;
};

struct Subtex {
    s32 x = 0;
    s32 y = 0;
    s32 width = 0;
    s32 height = 0;
};

struct Subtex_Array {
    static constexpr s32 max = 25;
    Subtex data[max];
    s32 count = 0;
};

struct Texture {
    u32 tex = 0u;
    s32 width = 0;
    s32 height = 0;
    Subtex_Array subtexs;
};

// --------------------------------------------

// *** Functions ***

fn gfx_init(Vertex_Buffer* buffer, Vertex_Buffer_Desc ds) -> bool;
fn gfx_done(Vertex_Buffer* buffer) -> void;
fn gfx_draw_call(Vertex_Buffer buffer) -> void;
fn gfx_draw_call_complex(Vertex_Buffer obj, u32 count = 0, u32 index_offset = 0) -> void;

fn gfx_init(Global_Buffer* buffer, Global_Buffer_Desc ds) -> bool;
fn gfx_done(Global_Buffer* buffer) -> void;
fn gfx_update(Global_Buffer buffer, void* data) -> void;
fn gfx_use(Global_Buffer buffer) -> void;
fn gfx_use(Global_Buffer buffer, u32 index) -> void;

fn gfx_init(Shader* shader, Shader_Desc ds) -> bool;
fn gfx_done(Shader* shader) -> void;
fn gfx_use(Shader shader) -> void;
fn gfx_set_param(Shader shader, std::string_view param, u32 count, s32* values) -> void;

// --------------------------------------------

fn gfx_init(Texture* texture, Texture_Desc def) -> void;
fn gfx_done(Texture* texture) -> void;
fn gfx_use(Texture texture, u32 unit = 0) -> void;

fn clear_back_buffer(Vec4 color = Color.Corn_Flower_Blue) -> void;
fn set_blend_enabled(bool enabled = true) -> void;
fn set_viewport(s32 viewport_x, s32 viewport_y) -> void;
fn set_depth_test_enabled(bool enabled = true) -> void;