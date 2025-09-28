#pragma once

namespace Cool {

class NfdFileFilter {
public:
    static const std::vector<nfdfilteritem_t> ImageSave;
    static const std::vector<nfdfilteritem_t> ImageLoad;
    static const std::vector<nfdfilteritem_t> Png;
    static const std::vector<nfdfilteritem_t> Jpeg;
    static const std::vector<nfdfilteritem_t> Tiff;
    static const std::vector<nfdfilteritem_t> Video;
    static const std::vector<nfdfilteritem_t> Audio;
    static const std::vector<nfdfilteritem_t> Shader;
    static const std::vector<nfdfilteritem_t> FragmentShader;
    static const std::vector<nfdfilteritem_t> VertexShader;
    static const std::vector<nfdfilteritem_t> ComputeShader;
    static const std::vector<nfdfilteritem_t> Mesh;
};

} // namespace Cool