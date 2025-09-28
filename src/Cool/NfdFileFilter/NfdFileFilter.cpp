#include "NfdFileFilter.h"

namespace Cool {

const std::vector<nfdfilteritem_t> NfdFileFilter::ImageSave = {
    {"Image", "png,jpg,jpeg"},
    {"png", "png"},
    {"jpeg", "jpg,jpeg"},
};
const std::vector<nfdfilteritem_t> NfdFileFilter::ImageLoad = {
    {"Image", "png,jpg,jpeg,tif,tiff"},
    {"png", "png"},
    {"jpeg", "jpg,jpeg"},
    {"tiff", "tif,tiff"},
};
const std::vector<nfdfilteritem_t> NfdFileFilter::Png  = {NfdFileFilter::ImageLoad[1]};
const std::vector<nfdfilteritem_t> NfdFileFilter::Jpeg = {NfdFileFilter::ImageLoad[2]};
const std::vector<nfdfilteritem_t> NfdFileFilter::Tiff = {NfdFileFilter::ImageLoad[3]};

const std::vector<nfdfilteritem_t> NfdFileFilter::Video = {
    {"Video", "3g2,3gp,3gp2,3gpp,amrec,amv,asf,avi,bik,bin,crf,dav,divx,drc,dv,dvr-ms,evo,f4v,flv,gif,gifv,gvi,gxf,iso,k3g,m1v,m2v,m2t,m2ts,m4v,mkv,mov,mp2,mp2v,mp4,mp4v,mpe,mpeg,mpeg1,mpeg2,mpeg4,mpg,mpv2,mts,mtv,mxf,mxg,nsv,nuv,ogg,ogm,ogv,ogx,ps,qt,rec,rm,rmvb,rpl,skm,thp,tod,tp,ts,tts,txd,vob,vp6,vro,webm,wm,wmv,wtv,xesc"},
};

const std::vector<nfdfilteritem_t> NfdFileFilter::Audio = {
    // Restricted to only the extensions that libnyquist can load.
    {"Audio", "mp3,wav,wave,wv,flac,ogg,opus,mpc,mpp"},
    {"mp3", "mp3"},
    {"wav", "wav,wave"},
    {"wv", "wv"},
    {"flac", "flac"},
    {"ogg", "ogg"},
    {"opus", "opus"},
    {"mpc", "mpc,mpp"},
};

const std::vector<nfdfilteritem_t> NfdFileFilter::Shader = {
    {"Shader", "frag,fs,vert,vs,comp,cs,glsl,hlsl"},
    {"Fragment Shader", "frag,fs,glsl,hlsl"},
    {"Vertex Shader", "vert,vs,glsl,hlsl"},
    {"Compute Shader", "comp,cs,glsl,hlsl"},
};
const std::vector<nfdfilteritem_t> NfdFileFilter::FragmentShader = {NfdFileFilter::Shader[1]};
const std::vector<nfdfilteritem_t> NfdFileFilter::VertexShader   = {NfdFileFilter::Shader[2]};
const std::vector<nfdfilteritem_t> NfdFileFilter::ComputeShader  = {NfdFileFilter::Shader[3]};

const std::vector<nfdfilteritem_t> NfdFileFilter::Mesh = {
    {"Mesh", "ply"},
    {"ply", "ply"},
};

} // namespace Cool