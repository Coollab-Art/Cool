#pragma once
#if defined(COOL_OPENGL)
#include "../TextureFormat.hpp"
#include "FrameBuffer.h"
#include "glpp/glpp.hpp"

namespace Cool {

class TextureFB : public FrameBuffer {
public:
    explicit TextureFB(TextureFormat format)
        : _format{format}
    {}

    void   attachTextureToSlot(unsigned int slot) const;
    GLuint textureID() const { return m_colorTextureId->id(); }

private:
    void createAttachments(img::Size size) override;
    void destroyAttachments() override;
    void attachAttachments() override;

private:
    std::optional<glpp::UniqueTexture2D> m_colorTextureId{};
    TextureFormat                        _format;
};

} // namespace Cool

#endif