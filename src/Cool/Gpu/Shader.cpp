// #include "Shader.h"
// #include <string>
// #include <string_view>
// #include "Cool/Gpu/TextureLibrary_FromFile.h"
// #include "Cool/Gpu/TextureSamplerLibrary.h"
// #include "Cool/Gpu/TextureSource.h"
// #include "Cool/Midi/MidiManager.h"
// #include "Cool/OSC/OSCManager.h"
// #include "Cool/StrongTypes/Camera2D.h"
// #include "Cool/StrongTypes/ColorAndAlpha.h"
// #include "Cool/WebGPU/Texture.h"
// #include "ShaderModule.h"

// namespace Cool {

// // static void assert_shader_is_bound(GLuint id)
// // {
// // #if DEBUG
// //     GLint current_id;
// //     glGetIntegerv(GL_CURRENT_PROGRAM, &current_id);
// //     assert(static_cast<GLuint>(current_id) == id && "The shader must be bound before setting any uniform.");
// // #else
// //     (void)id;
// // #endif
// // }

// void Shader::bind() const
// {
//     // GLDebug(glUseProgram(_shader.id()));
// }

// // GLint Shader::uniform_location(std::string_view uniform_name) const
// // {
// //     const auto name = std::string{uniform_name};
// //     const auto it   = _uniform_locations.find(name);
// //     if (it != _uniform_locations.end())
// //     {
// //         return it->second;
// //     }
// //     else
// //     {
// //         GLDebug(GLint location = glGetUniformLocation(_shader.id(), name.c_str()));
// //         _uniform_locations[name] = location;
// //         return location;
// //     }
// // }

// void Shader::set_uniform(std::string_view uniform_name, int v) const
// {
//     // assert_shader_is_bound(_shader.id());
//     // GLDebug(glUniform1i(uniform_location(uniform_name), v));
// }
// void Shader::set_uniform(std::string_view uniform_name, unsigned int v) const
// {
//     // assert_shader_is_bound(_shader.id());
//     // GLDebug(glUniform1ui(uniform_location(uniform_name), v));
// }
// void Shader::set_uniform(std::string_view uniform_name, bool v) const
// {
//     set_uniform(uniform_name, v ? 1 : 0);
// }
// void Shader::set_uniform(std::string_view uniform_name, float v) const
// {
//     // assert_shader_is_bound(_shader.id());
//     // GLDebug(glUniform1f(uniform_location(uniform_name), v));
// }
// void Shader::set_uniform(std::string_view uniform_name, const glm::vec2& v) const
// {
//     // assert_shader_is_bound(_shader.id());
//     // GLDebug(glUniform2f(uniform_location(uniform_name), v.x, v.y));
// }
// void Shader::set_uniform(std::string_view uniform_name, const glm::vec3& v) const
// {
//     // assert_shader_is_bound(_shader.id());
//     // GLDebug(glUniform3f(uniform_location(uniform_name), v.x, v.y, v.z));
// }
// void Shader::set_uniform(std::string_view uniform_name, const glm::vec4& v) const
// {
//     // assert_shader_is_bound(_shader.id());
//     // GLDebug(glUniform4f(uniform_location(uniform_name), v.x, v.y, v.z, v.w));
// }
// void Shader::set_uniform(std::string_view uniform_name, const glm::uvec2& v) const
// {
//     // assert_shader_is_bound(_shader.id());
//     // GLDebug(glUniform2ui(uniform_location(uniform_name), v.x, v.y));
// }
// void Shader::set_uniform(std::string_view uniform_name, const glm::uvec3& v) const
// {
//     // assert_shader_is_bound(_shader.id());
//     // GLDebug(glUniform3ui(uniform_location(uniform_name), v.x, v.y, v.z));
// }
// void Shader::set_uniform(std::string_view uniform_name, const glm::uvec4& v) const
// {
//     // assert_shader_is_bound(_shader.id());
//     // GLDebug(glUniform4ui(uniform_location(uniform_name), v.x, v.y, v.z, v.w));
// }
// void Shader::set_uniform(std::string_view uniform_name, const glm::mat2& mat) const
// {
//     // assert_shader_is_bound(_shader.id());
//     // GLDebug(glUniformMatrix2fv(uniform_location(uniform_name), 1, GL_FALSE, glm::value_ptr(mat)));
// }
// void Shader::set_uniform(std::string_view uniform_name, const glm::mat3& mat) const
// {
//     // assert_shader_is_bound(_shader.id());
//     // GLDebug(glUniformMatrix3fv(uniform_location(uniform_name), 1, GL_FALSE, glm::value_ptr(mat)));
// }
// void Shader::set_uniform(std::string_view uniform_name, const glm::mat4& mat) const
// {
//     // assert_shader_is_bound(_shader.id());
//     // GLDebug(glUniformMatrix4fv(uniform_location(uniform_name), 1, GL_FALSE, glm::value_ptr(mat)));
// }
// void Shader::set_uniform(std::string_view uniform_name, Angle angle) const
// {
//     set_uniform(uniform_name, angle.as_radians());
// }
// void Shader::set_uniform(std::string_view uniform_name, Color const& color) const
// {
//     set_uniform(uniform_name, color.as_sRGB());
// }
// void Shader::set_uniform(std::string_view uniform_name, ColorAndAlpha const& color) const
// {
//     set_uniform(uniform_name, color.as_sRGB_StraightA());
// }
// void Shader::set_uniform(std::string_view uniform_name, Direction2D direction) const
// {
//     set_uniform(uniform_name, direction.as_unit_vec2());
// }
// void Shader::set_uniform(std::string_view uniform_name, Hue hue) const
// {
//     set_uniform(uniform_name, hue.from_0_to_1());
// }
// void Shader::set_uniform(std::string_view uniform_name, const ColorPalette& palette) const
// {
//     int idx = 0;
//     for (auto const& color : palette.value)
//     {
//         set_uniform(fmt::format("{}[{}]", Cool::internal::color_palette_array_name(uniform_name), idx), color);
//         idx++;
//     }
// }
// void Shader::set_uniform(std::string_view uniform_name, const ImGG::ColorRGBA& v) const
// {
//     // assert_shader_is_bound(_shader.id());
//     // GLDebug(glUniform4f(uniform_location(uniform_name), v.x, v.y, v.z, v.w));
// }
// void Shader::set_uniform(std::string_view uniform_name, const Gradient& gradient) const
// {
//     int idx = 0;
//     for (ImGG::Mark const& mark : gradient.value.gradient().get_marks())
//     {
//         set_uniform(
//             fmt::format("{}[{}].pos", Cool::internal::gradient_marks_array_name(uniform_name), idx),
//             mark.position.get()
//         );
//         set_uniform(
//             fmt::format("{}[{}].col", Cool::internal::gradient_marks_array_name(uniform_name), idx),
//             ColorAndAlpha::from_srgb_straight_alpha(as_glm(mark.color)).as_Oklab_PremultipliedA()
//         );
//         idx++;
//     }
// }
// void Shader::set_uniform(std::string_view uniform_name, Point2D point2D) const
// {
//     set_uniform(uniform_name, point2D.value);
// }
// void Shader::set_uniform(std::string_view uniform_name, Camera2D const& cam) const
// {
//     set_uniform(uniform_name, cam.transform_matrix());
// }

// // static auto max_number_of_texture_slots() -> GLuint
// // {
// //     GLint res{};
// //     glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &res);
// //     return static_cast<GLuint>(res);
// // }

// // static auto get_next_texture_slot() -> GLuint
// // {
// //     static GLuint       current_slot = 0;
// //     static GLuint const max_slots    = max_number_of_texture_slots();

// //     current_slot = (current_slot + 1) % max_slots;
// //     if (current_slot == 0) // HACK Slot 0 is used for texture operations like resizing and setting the image, anyone might override the texture set here at any time. So we use all slots but the 0th one for rendering.
// //         current_slot = 1;
// //     return current_slot;
// // }

// void Shader::set_uniform(std::string_view uniform_name, Texture const& texture, TextureSamplerDescriptor const& sampler) const
// {
//     // auto const slot = get_next_texture_slot();
//     // texture.attach_to_slot(slot);
//     // GLDebug(glBindSampler(slot, TextureSamplerLibrary::instance().get(sampler).id()));
//     // set_uniform(fmt::format("{}.tex", uniform_name), static_cast<int>(slot));
//     // set_uniform(fmt::format("{}.aspect_ratio", uniform_name), texture.aspect_ratio());
//     // set_uniform(fmt::format("{}.flip_y", uniform_name), texture.need_to_flip_y());
//     // GLDebug(glActiveTexture(GL_TEXTURE0)); // HACK Slot 0 is used for texture operations like resizing and setting the image, anyone might override the texture set here at any time. So we use all slots but the 0th one for rendering.
// }

// // void Shader::set_uniform_texture(std::string_view uniform_name, GLuint texture_id, TextureSamplerDescriptor const& sampler) const
// // {
// // auto const slot = get_next_texture_slot();
// // GLDebug(glActiveTexture(GL_TEXTURE0 + slot));
// // GLDebug(glBindTexture(GL_TEXTURE_2D, texture_id));
// // GLDebug(glBindSampler(slot, TextureSamplerLibrary::instance().get(sampler).id()));
// // set_uniform(uniform_name, static_cast<int>(slot));
// // GLDebug(glActiveTexture(GL_TEXTURE0)); // HACK Slot 0 is used for texture operations like resizing and setting the image, anyone might override the texture set here at any time. So we use all slots but the 0th one for rendering.
// // }

// // void Shader::set_uniform_texture1D(std::string_view uniform_name, GLuint texture_id) const
// // {
// // auto const slot = get_next_texture_slot();
// // GLDebug(glActiveTexture(GL_TEXTURE0 + slot));
// // GLDebug(glBindTexture(GL_TEXTURE_1D, texture_id));
// // GLDebug(glBindSampler(slot, 0)); // No sampler, we use the one from the texture
// // set_uniform(uniform_name, static_cast<int>(slot));
// // GLDebug(glActiveTexture(GL_TEXTURE0)); // HACK Slot 0 is used for texture operations like resizing and setting the image, anyone might override the texture set here at any time. So we use all slots but the 0th one for rendering.
// // }

// void Shader::set_uniform(std::string_view uniform_name, TextureDescriptor const& texture_info) const
// {
//     set_uniform(uniform_name, get_texture(texture_info.source), texture_info.sampler);
// }
// void Shader::set_uniform(std::string_view uniform_name, MidiChannel const& channel) const
// {
//     set_uniform(uniform_name, midi_manager().get_value(channel));
// }
// void Shader::set_uniform(std::string_view uniform_name, OSCChannel const& channel) const
// {
//     set_uniform(uniform_name, osc_manager().get_value(channel));
// }
// void Shader::set_uniform(std::string_view uniform_name, TimeSpeed const& time_speed) const
// {
//     set_uniform(uniform_name, time_speed.value);
// }

// } // namespace Cool
