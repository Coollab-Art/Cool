#pragma once
#if defined(COOL_OPENGL)

namespace Cool {

/// <summary>
/// A SSBO (Shader Storage Buffer Object) is a buffer that lives on the GPU and that you can access from your shaders, either to read or write.
/// </summary>
/// <typeparam name="T">The type of the elements stored in the buffer, like float.</typeparam>
template<typename T>
class SSBO {
public:
    /// <summary>
    ///
    /// </summary>
    /// <param name="binding">The "id" that the buffer will be referenced by in the shaders. Make sure that it matches the "binding=xxx" when you declare the buffer in the shader.
    /// To declare your SSBO in a shader, simply write "layout(std430, binding=xxx) buffer myBuffer { float data[]; };"
    /// After that, data will be available as a regular variable in your shader.
    /// Note that myBuffer and data are arbitrary names and you can change them as you please.
    /// Also, the layout doesn't need to use floats, you can basically use any type you want (kind of, vec3 doesn't work) : float, vec2, vec4, a struct, etc.
    /// </param>
    explicit SSBO(unsigned int binding)
        : _binding(binding)
    {
        GLDebug(glGenBuffers(1, &_id));
        GLDebug(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, _binding, _id));
    }
    ~SSBO()
    {
        GLDebug(glDeleteBuffers(1, &_id));
    }
    SSBO(SSBO const&)            = delete;
    SSBO& operator=(SSBO const&) = delete;
    SSBO(SSBO&& other) noexcept
        : _binding{other._binding}
        , _id{other._id}
    {
        other._id = 0;
    }
    auto operator=(SSBO&& other) noexcept -> SSBO&
    {
        _binding = other._binding;
        if (this != &other)
        {
            GLDebug(glDeleteBuffers(1, &_id));
            _id       = other._id;
            other._id = 0;
        }
        return *this;
    }

    /// Raw OpenGL name of the buffer. Useful to also bind it to another target, e.g. as a
    /// GL_ARRAY_BUFFER to read it as a vertex attribute.
    auto id() const -> GLuint { return _id; }

    void bind()
    {
        GLDebug(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, _binding, _id));
        GLDebug(glBindBuffer(GL_SHADER_STORAGE_BUFFER, _id));
    }

    /// <summary>
    /// Sends some data (an array of T) from the CPU (your C++ code) to the GPU (your shader code).
    /// </summary>
    /// <param name="nb_of_elements">Number of elements in the array</param>
    /// <param name="data">Pointer to the beginning of the array</param>
    /// <param name="usage">A hint that allows OpenGL to optimize the SSBO. You can see all possible values here : http://docs.gl/gl4/glBufferData</param>
    void upload_data(size_t nb_of_elements, T* data, GLenum usage = GL_STREAM_READ)
    {
        bind();
        GLDebug(glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(nb_of_elements * sizeof(T)), data, usage));
    }

    /// <summary>
    /// Sends some data from the CPU (your C++ code) to the GPU (your shader code).
    /// </summary>
    /// <param name="v">The vector containing the data.</param>
    /// <param name="usage">A hint that allows OpenGL to optimize the SSBO. You can see all possible values here : http://docs.gl/gl4/glBufferData</param>
    void upload_data(std::vector<T>& v, GLenum usage = GL_STREAM_READ)
    {
        upload_data(v.size(), v.data(), usage);
    }

    /// <summary>
    /// Retrieves the data from the GPU (your shader code) back to the CPU (your C++ code) and writes it in the given array.
    /// </summary>
    /// <param name="nb_of_elements">Number of elements in the array</param>
    /// <param name="data">Pointer to the beginning of the array</param>
    void download_data(size_t nb_of_elements, T* data) const
    {
        GLDebug(glBindBuffer(GL_SHADER_STORAGE_BUFFER, _id));
        GLvoid* p{};
        GLDebug(p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY));
        memcpy(data, p, nb_of_elements * sizeof(T));
        GLDebug(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));
    }

    /// <summary>
    /// Retrieves the data from the GPU (your shader code) back to the CPU (your C++ code) and writes it in the given std::vector.
    /// </summary>
    /// <param name="v">The vector that will receive the data. Please make sure that it already has the right size !</param>
    void download_data(std::vector<T>& v) const
    {
        download_data(v.size(), v.data());
    }

private:
    unsigned int _binding{};
    unsigned int _id{};
};

} // namespace Cool

#endif
