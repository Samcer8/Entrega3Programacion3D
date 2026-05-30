#include "app.h"

fn main() -> s32 {
    logf("Hellope!");
    app_init();

    u8 image_pixels[16] = {
        42u, 45u, 121u, 255u,
        232u, 5u, 103u, 255u,
        255u, 142u, 104u, 255u,
        244u, 234u, 188u, 255u
    };

    IO_Image image;
    image.data = image_pixels;
    image.channels = 4;
    image.width = 2;
    image.height = 2;

    u32 tex = 0u;
    glCreateTextures(GL_TEXTURE_2D, 1, &tex);

    s32 storage_format = image.channels == 4 ? GL_RGBA8 : image.channels == 3 ? GL_RGB8 : 0;

    glTextureStorage2D(tex, 1, storage_format, image.width, image.height);

    glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_REPEAT);

    s32 data_format = image.channels == 4 ? GL_RGBA : image.channels == 3 ? GL_RGB : 0;

    glTextureSubImage2D(tex, 0, 0, 0, image.width, image.height, data_format, GL_UNSIGNED_BYTE, image.data);

    constexpr f32 QUAD_VTS[] = {
        -0.5f, -0.5f, 0.0f, 0.0f,
        +0.5f, -0.5f , 1.0f, 0.0f,
        +0.5f, +0.5f, 1.0f, 1.0f,
        -0.5f, +0.5f, 0.0f, 1.0f,
    };
    u32 vao = 0u;
    glCreateVertexArrays(1, &vao);

    u32 offset = 0;

    //Le voy a añadir colores luego XD
    struct
    {
        u32 index = 0;
        u32 count = 2;
        u32 size = sizeof(f32) * count;
    } constexpr pos;

    glEnableVertexArrayAttrib(vao, pos.index);
    glVertexArrayAttribFormat(vao, pos.index, pos.count, GL_FLOAT, false, offset);

    glVertexArrayAttribBinding(vao, pos.index, 0u);

    offset += pos.size;

    //texturas
    struct {
        s32 index = 1;
        s32 count = 2;
        s32 size = sizeof(f32) * count;
    } constexpr UV;

    glEnableVertexArrayAttrib(vao, UV.index);
    glVertexArrayAttribFormat(vao, UV.index, UV.count, GL_FLOAT, false, offset);
    offset += UV.size;
    glVertexArrayAttribBinding(vao, UV.index, 0u);

    u32 vbo = 0u;
    constexpr u64 QUAD_VT_SIZE = pos.size + UV.size;

    glCreateBuffers(1, &vbo);
    glNamedBufferData(vbo, sizeof(QUAD_VTS), QUAD_VTS, GL_STATIC_DRAW);

    glVertexArrayVertexBuffer(vao, 0u, vbo, 0u, QUAD_VT_SIZE);

    u32 ebo = 0u;

    constexpr u32 QUAD_ELEMS[] = {
        0u, 1u, 2u,
        2u, 3u, 0u,
    };

    glCreateBuffers(1, &ebo);
    glNamedBufferData(ebo, sizeof(QUAD_ELEMS), QUAD_ELEMS, GL_STATIC_DRAW);

    glVertexArrayElementBuffer(vao, ebo);

    std::string source = os_read_entire_file("shader_flat_color2.glsl");
    u32 shader_flat_color = os_create_gl_program(source);

    struct {
        Mat4 projection = Mat.Identity4;
        Mat4 transform = Mat.Identity4;
        u32 tex_unit = 0;
    } shader_data;

    u32 ubo = 0u;
    glCreateBuffers(1, &ubo);
    glNamedBufferData(ubo, sizeof(shader_data), nullptr, GL_DYNAMIC_DRAW);

    f32 aspect = 16.f / 9.f;
    f32 zoom = 3.0f;
    f32 nearpl = 0.0f;
    f32 farpl = 0.1f;

    Vec3 quad_pos;
    Vec3 quad_pos2 = Vec3(F32.Left) * 2.f;

    while (app_running()) {
        glClear(GL_COLOR_BUFFER_BIT);
        shader_data.transform = Mat4::transpose(Mat4::transform(quad_pos));
        shader_data.projection = Mat4::transpose(Mat4::orthographic(aspect, zoom, nearpl, farpl));

        glBindTextureUnit(0, tex);
        shader_data.tex_unit = 0;

        s32 samplers[32];

        for (s32 i = 0; i < 32; ++i) {
            samplers[i] = i;
        }

        GLint location = glGetUniformLocation(shader_flat_color, "u_samplers");
        glUniform1iv(location, 32, samplers);

        glNamedBufferSubData(ubo, 0, sizeof(shader_data), &shader_data);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0u, ubo);

        glUseProgram(shader_flat_color);
        glBindVertexArray(vao);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        /*quad_pos += Vec3(F32.Right) * os_delta_time();

        shader_data.transform = Mat4::transpose(Mat4::transform(quad_pos2));
        glNamedBufferSubData(ubo, 0, sizeof(shader_data), &shader_data);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0u, ubo);
        
        glUseProgram(shader_flat_color);
        glBindVertexArray(vao);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);*/

        os_swap_buffers();
    }

    app_done();
    //while(true);
}