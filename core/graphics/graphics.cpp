#include "core_pch.h"
#include "graphics.h"
#include "os_gl.h"
#include "os_core.h"
#include "io_image.h"
#include "app.h"

bool gfx_init(Vertex_Buffer* buffer, Vertex_Buffer_Desc ds)
{
    auto& [vao, vbo, ebo, count] = *buffer;
    glCreateVertexArrays(1, &buffer->vao);

    u32 offset = 0;

    for (s32 i = 0; i < ds.attributes.count; ++i) {
        Data_Type attr = ds.attributes.data[i];
        if (os_to_gl(attr) == 0) {
            continue;
        }
        glEnableVertexArrayAttrib(vao, i);
        if (is_integer_type(attr)) {
            glVertexArrayAttribIFormat(vao, i, get_count(attr), os_to_gl(attr), offset);
        }
        else {
            glVertexArrayAttribFormat(vao, i, get_count(attr), os_to_gl(attr), false, offset);
        }
        glVertexArrayAttribBinding(vao, i, 0);
        offset += get_size(attr);
    }

    glCreateBuffers(1, &vbo);
    GLenum usage = ds.vertices.data ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
    glNamedBufferData(vbo, ds.vertices.size, ds.vertices.data, usage);
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, (ds.vertices.size / ds.vertices.count));

    glCreateBuffers(1, &ebo);
    glNamedBufferData(ebo, (sizeof(u32) * ds.indices.count), ds.indices.data, GL_STATIC_DRAW);
    glVertexArrayElementBuffer(vao, ebo);
    buffer->count = ds.indices.count;

    /*glEnableVertexArrayAttrib(buffer->vao, *ds.indices.data);
    glVertexArrayAttribFormat(buffer->vao, *ds.indices.data, ds.attributes.count, os_to_gl(*ds.attributes.data), false, offset);

    glVertexArrayAttribBinding(buffer->vao, *ds.indices.data, 0u);

    glCreateBuffers(1, &buffer->vbo);*/

    return true;
}

void gfx_done(Vertex_Buffer* buffer)
{
    auto& [vao, vbo, ebo, count] = *buffer;
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    *buffer = {};
}

void gfx_draw_call(Vertex_Buffer buffer)
{
    glBindVertexArray(buffer.vao);
    glDrawElements(GL_TRIANGLES, buffer.count, GL_UNSIGNED_INT, nullptr);
}

void gfx_draw_call_complex(Vertex_Buffer obj, u32 count, u32 index_offset)
{
    checkf(obj.vao != 0u, "Error! This is not a valid Vertex Array!");
    glBindVertexArray(obj.vao);
    glDrawElements(GL_TRIANGLES, count == 0 ? obj.count : count, GL_UNSIGNED_INT, (void*)(index_offset * sizeof(u32)));
}

bool gfx_init(Global_Buffer* buffer, Global_Buffer_Desc ds)
{
    glCreateBuffers(1, &buffer->ubo);
    glNamedBufferData(buffer->ubo, ds.size, nullptr, GL_DYNAMIC_DRAW);
    buffer->size = ds.size;
    return true;
}

void gfx_done(Global_Buffer* buffer)
{
    glDeleteBuffers(1, &buffer->ubo);
    *buffer = {};
}

void gfx_update(Global_Buffer buffer, void* data)
{
    glNamedBufferSubData(buffer.ubo, 0, buffer.size, data);
}

void gfx_use(Global_Buffer buffer)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, /*index*/ 0, buffer.ubo);
}

void gfx_use(Global_Buffer buffer, u32 index)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, index, buffer.ubo);
}

bool gfx_init(Shader* shader, Shader_Desc ds)
{
    //Revisar, no sé si me hace falta la primera función aqui
    std::string source = os_read_entire_file(ds.filename);
    checkf(!source.empty(), "Error! This is not a valid Vertex Array!");
    shader->pgm = os_create_gl_program(source);
    return true;
}

void gfx_done(Shader* shader)
{
    glDeleteProgram(shader->pgm);
    *shader = {};
}

void gfx_use(Shader shader)
{
    checkf(shader.pgm != 0, "Error! This is not a valid Shader!");
    glUseProgram(shader.pgm);
}

void gfx_set_param(Shader shader, std::string_view param, u32 count, s32* values)
{
    checkf(shader.pgm != 0, "Error! This is not a valid Shader!");
    GLint location = glGetUniformLocation(shader.pgm, param.data());
    glUniform1iv(location, count, values);
}

void gfx_init(Texture* texture, Texture_Desc def)
{
    IO_Image image_buff;
    const IO_Image* image = nullptr;
    if (def.image) {
        image = def.image;
    }
    else {
        io_image_load(def.filename, &image_buff);
        image = &image_buff;
    }
    checkf(io_image_valid(*image), "Error! This is not a valid Image!");

    u32& tex = texture->tex;
    texture->width = image->width;
    texture->height = image->height;

    glCreateTextures(GL_TEXTURE_2D, 1, &tex);

    // Check if as RGB or RGBA.
    s32 storage_format = image->channels == 4 ? GL_RGBA8
        : image->channels == 3 ? GL_RGB8 : 0;

    // Reserve the storage.    
    glTextureStorage2D(tex, 1, storage_format, image->width, image->height);

    // Texture config.
    GLenum filter = def.filter == Texture_Filter::Nearest ? GL_NEAREST :
        def.filter == Texture_Filter::Linear ? GL_LINEAR : 0;

    glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, filter);
    glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, filter);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Check if as RGB or RGBA. (Again :S)
    s32 data_format = image->channels == 4 ? GL_RGBA
        : image->channels == 3 ? GL_RGB : 0;

    // Send the texture data to the gpu.
    glTextureSubImage2D(tex, 0, 0, 0, image->width, image->height, data_format, GL_UNSIGNED_BYTE, image->data);

    if (io_image_valid(image_buff)) {
        io_image_free(&image_buff);
    }

    // Build the subtex info
    switch (def.kind) {
    case Texture_Kind_Multiple: {
        if (!ensuref(def.subtex_size <= texture->width && def.subtex_size <= texture->height, "Error! Invalid subtex_size!")) {
            break;
        }
        s32 x_count = texture->width / def.subtex_size;
        s32 y_count = texture->height / def.subtex_size;
        s32 subtex_count = x_count * y_count;

        if (!ensuref(subtex_count <= texture->subtexs.max, "Error! Subtex count is %i. Max allowed is %i", subtex_count, texture->subtexs.max)) {
            break;
        }

        texture->subtexs.count = subtex_count;

        for (s32 i = 0; i < subtex_count; ++i) {
            s32 x = i % x_count;
            s32 y = i / x_count;

            auto& subtex = texture->subtexs.data[i];
            subtex.width = def.subtex_size;
            subtex.height = def.subtex_size;
            subtex.x = def.subtex_size * x;
            subtex.y = def.subtex_size * y;
        }
    }
    case Texture_Kind_Single:
    default: {
        // We don't need to do anything here.
        }
    }
}

void gfx_done(Texture* texture) 
{
    glDeleteTextures(1, &texture->tex);
    *texture = {};
}

void gfx_use(Texture texture, u32 unit)
{
    checkf(texture.tex != 0, "Error! This is not a valid Texture!");
    glBindTextureUnit(unit, texture.tex);
}

void clear_back_buffer(Vec4 color)
{
    os_clear_color_gl(color);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void set_blend_enabled(bool enabled)
{
    if (enabled) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else {
        glDisable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
}

void set_viewport(s32 viewport_x, s32 viewport_y)
{
    glViewport(0, 0, viewport_x, viewport_y);
}

void set_depth_test_enabled(bool enabled)
{
    if (enabled) {
        glEnable(GL_DEPTH_TEST);
    }
    else {
        glDisable(GL_DEPTH_TEST);
    }
}