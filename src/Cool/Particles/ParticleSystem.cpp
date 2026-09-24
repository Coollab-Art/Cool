#include "ParticleSystem.h"

namespace Cool {

ParticleSystem::ParticleSystem(int dimension, ParticlesShadersCode const& shader_code, size_t particles_count)
    : _particles_count{particles_count}
    , _dimension(dimension)
#if !defined(COOL_PARTICLES_DISABLED_REASON)
    , _render_shader{
          Cool::OpenGL::ShaderModule{Cool::ShaderDescription{
              .kind        = Cool::ShaderKind::Vertex,
              .source_code = shader_code.vertex,
          }},
          Cool::OpenGL::ShaderModule{Cool::ShaderDescription{
              .kind        = Cool::ShaderKind::Fragment,
              .source_code = shader_code.fragment,
          }}
      }
    , _simulation_shader{64, shader_code.simulation}
    , _init_shader{64, shader_code.init}
#endif
{
    set_particles_count(_particles_count); // Will init all the particles attributes (by calling the init shader)
#if !defined(COOL_PARTICLES_DISABLED_REASON)
    glpp::bind_vertex_array(_render_vao);
    glpp::bind_vertex_buffer(_render_vbo);
    glpp::set_vertex_buffer_attribute(_render_vbo, 0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);                          // Vertices positions
    glpp::set_vertex_buffer_attribute(_render_vbo, 1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))); // Vertices UVs
    glpp::set_vertex_buffer_data(
        _render_vbo, glpp::DataAccessFrequency::Static,
        std::array{
            -1.f, -1.f, 0.0f, 0.0f,
            +1.f, -1.f, 1.0f, 0.0f,
            +1.f, +1.f, 1.0f, 1.0f,

            -1.f, -1.f, 0.0f, 0.0f,
            +1.f, +1.f, 1.0f, 1.0f,
            -1.f, +1.f, 0.0f, 1.0f
        }
    );
    setup_instanced_vertex_attributes();
#else
    Cool::Log::error("Particles", "Particles are not supported on MacOS for now.");
    std::ignore = shader_code;
#endif
}

void ParticleSystem::render()
{
#if !defined(COOL_PARTICLES_DISABLED_REASON)
    _render_shader.bind(); // No need to bind the SSBOs: the render shader reads the particles as instanced vertex attributes, which are part of the VAO's state. See setup_instanced_vertex_attributes().
    glpp::bind_vertex_array(_render_vao);
    glpp::draw_arrays_instanced(_render_vao, glpp::PrimitiveDrawMode::Triangles, 0, 6, static_cast<GLsizei>(_particles_count));
#endif
}

void ParticleSystem::update()
{
#if !defined(COOL_PARTICLES_DISABLED_REASON)
    bind_SSBOs();
    _simulation_shader.bind();
    _simulation_shader.compute({_particles_count, 1, 1});
#endif
}

void ParticleSystem::set_simulation_shader(std::string const& shader_code)
{
#if !defined(COOL_PARTICLES_DISABLED_REASON)
    _simulation_shader = OpenGL::ComputeShader{64, shader_code};
#else
    std::ignore = shader_code;
#endif
}

void ParticleSystem::reset()
{
#if !defined(COOL_PARTICLES_DISABLED_REASON)
    bind_SSBOs();
    _init_shader.bind();
    _init_shader.compute({_particles_count, 1, 1});
#endif
}

void ParticleSystem::set_particles_count(size_t particles_count)
{
    _particles_count = particles_count;
#if !defined(COOL_PARTICLES_DISABLED_REASON)
    bind_SSBOs();
    // GL_DYNAMIC_COPY because these buffers are written by the GPU (compute shader) and read by the GPU (vertex shader).
    // (The default, GL_STREAM_READ, would tell the driver that the application reads them back, which we never do, and some drivers would then place them in host-visible memory.)
    // NB: reallocating these buffers doesn't invalidate the vertex attributes set up in setup_instanced_vertex_attributes(),
    // because a VAO stores the buffer's name and the offset within it, not the allocation itself.
    _positions.upload_data(_particles_count * static_cast<size_t>(_dimension), nullptr, GL_DYNAMIC_COPY);
    _velocities.upload_data(_particles_count * static_cast<size_t>(_dimension), nullptr, GL_DYNAMIC_COPY);
    _sizes.upload_data(_particles_count, nullptr, GL_DYNAMIC_COPY);
    _lifetimes.upload_data(_particles_count, nullptr, GL_DYNAMIC_COPY);
    _lifetime_maxs.upload_data(_particles_count, nullptr, GL_DYNAMIC_COPY);
    _colors.upload_data(_particles_count * 4, nullptr, GL_DYNAMIC_COPY);
    _init_shader.bind();
    _init_shader.compute({_particles_count, 1, 1});
#else
    std::ignore = particles_count;
#endif
}

void ParticleSystem::bind_SSBOs()
{
#if !defined(COOL_PARTICLES_DISABLED_REASON)
    _positions.bind();
    _velocities.bind();
    _sizes.bind();
    _lifetimes.bind();
    _lifetime_maxs.bind();
    _colors.bind();
#endif
}

void ParticleSystem::setup_instanced_vertex_attributes()
{
#if !defined(COOL_PARTICLES_DISABLED_REASON)
    // Bind the particle buffers as instanced vertex attributes, so that the render shader can read
    // them without using any SSBO. This matters: SSBO support in the vertex and fragment stages is
    // *optional* in OpenGL 4.3 (GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS and
    // GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS are allowed to be 0, only the compute stage is
    // guaranteed to support them), so on some drivers the render shader would simply fail to link,
    // and particles wouldn't show up at all. Instanced vertex attributes have been core since
    // OpenGL 3.3 and are available everywhere.
    // The same buffers stay bound as SSBOs for the compute shaders: a buffer object can be bound to
    // several targets at once.
    // This only needs to be done once, even though set_particles_count() reallocates the buffers,
    // because a VAO stores the buffer's name and the offset within it, not the allocation itself.
    auto const setup = [](GLuint location, GLuint buffer_id, GLint components) {
        GLDebug(glBindBuffer(GL_ARRAY_BUFFER, buffer_id));
        GLDebug(glEnableVertexAttribArray(location));
        GLDebug(glVertexAttribPointer(location, components, GL_FLOAT, GL_FALSE, components * static_cast<GLsizei>(sizeof(float)), nullptr));
        GLDebug(glVertexAttribDivisor(location, 1)); // Advance once per particle instead of once per vertex
    };
    // Locations 0 and 1 are the quad's position and uv, they come from _render_vbo.
    setup(2, _positions.id(), _dimension);
    setup(3, _sizes.id(), 1);
    setup(4, _lifetimes.id(), 1);
    setup(5, _lifetime_maxs.id(), 1);
    setup(6, _colors.id(), 4);
    GLDebug(glBindBuffer(GL_ARRAY_BUFFER, 0));
#endif
}

} // namespace Cool
