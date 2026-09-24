#pragma once

#if defined(COOL_OPENGL)
#if DEBUG

namespace Cool {
/// Wrap all your OpenGL calls in this macro : it will add debug checks. Something like : GLDebug(glDrawArrays(GL_TRIANGLES, 0, 3))
/// It is not strictly necessary though because modern OpenGL debugging is enabled too. It's only to help those who don't have the advandced debugging available on their machine.
/// It expands to a single statement, so it is safe to use as the body of a braceless if / for.
/// It can't be used to declare a variable: declare it on the line before, and only wrap the assignment.
#define GLDebug(x)                                                    \
    do                                                                \
    {                                                                 \
        CoolGlDebug::clearFromPreviousErrors();                       \
        x;                                                            \
        assert(!CoolGlDebug::checkForErrors(#x, __FILE__, __LINE__)); \
    } while (0)
} // namespace Cool

namespace CoolGlDebug {
void        clearFromPreviousErrors();
bool        checkForErrors(const char* functionName, const char* filename, int line);
char const* glErrorString(GLenum const err);

/// Must be called just after you destroy your OpenGL context during the program's shutdown
/// This prevents the error checking from freezing if you destroy OpenGL objects after the context has been destroyed
void shut_down();
} // namespace CoolGlDebug

#else
namespace Cool {
#define GLDebug(x) \
    do             \
    {              \
        x;         \
    } while (0)
} // namespace Cool
#endif
#endif
