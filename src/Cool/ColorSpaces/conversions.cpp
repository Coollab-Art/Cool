#include "conversions.h"
#include <algorithm>

namespace Cool {

static auto keep_above_0(glm::vec3 const& v) -> glm::vec3
{
    return {
        std::max(v.x, 0.f),
        std::max(v.y, 0.f),
        std::max(v.z, 0.f),
    };
}

// Start of [Block1]
// From https://entropymine.com/imageworsener/srgbformula/

static auto LinearRGB_from_sRGB_impl(float x) -> float
{
    return x < 0.04045f
               ? x / 12.92f
               : std::pow((x + 0.055f) / 1.055f, 2.4f);
}

static auto sRGB_from_LinearRGB_impl(float x) -> float
{
    return x < 0.0031308f
               ? x * 12.92f
               : 1.055f * std::pow(x, 1.f / 2.4f) - 0.055f;
}

auto LinearRGB_from_sRGB(glm::vec3 const& srgb) -> glm::vec3
{
    auto const clamped = keep_above_0(srgb);
    return {
        LinearRGB_from_sRGB_impl(clamped.x),
        LinearRGB_from_sRGB_impl(clamped.y),
        LinearRGB_from_sRGB_impl(clamped.z),
    };
}

auto sRGB_from_LinearRGB(glm::vec3 const& rgb) -> glm::vec3
{
    auto const clamped = keep_above_0(rgb);
    return {
        sRGB_from_LinearRGB_impl(clamped.x),
        sRGB_from_LinearRGB_impl(clamped.y),
        sRGB_from_LinearRGB_impl(clamped.z),
    };
}
// End of [Block1]

// Start of [Block2]
// From http://www.easyrgb.com/en/math.php

auto XYZ_from_LinearRGB(glm::vec3 const& c) -> glm::vec3
{
    return c * glm::mat3(0.4124f, 0.3576f, 0.1805f, 0.2126f, 0.7152f, 0.0722f, 0.0193f, 0.1192f, 0.9505f);
}
auto LinearRGB_from_XYZ(glm::vec3 const& c) -> glm::vec3
{
    return c * glm::mat3(3.2406f, -1.5372f, -0.4986f, -0.9689f, 1.8758f, 0.0415f, 0.0557f, -0.2040f, 1.0570f);
}

auto XYZ_from_sRGB(glm::vec3 const& c) -> glm::vec3
{
    return XYZ_from_LinearRGB(LinearRGB_from_sRGB(c));
}
auto sRGB_from_XYZ(glm::vec3 const& c) -> glm::vec3
{
    return sRGB_from_LinearRGB(LinearRGB_from_XYZ(c));
}

// End of [Block2]

// Start of [Block3]
// https://bottosson.github.io/posts/colorpicker/
// Copyright(c) 2021 Bj�rn Ottosson
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions : The above copyright
// noticeand this permission notice shall be included in all copies or
// substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS",
// WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
// FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// Finds the maximum saturation possible for a given hue that fits in sRGB
// Saturation here is defined as S = C/L
// a and b must be normalized so a^2 + b^2 == 1
static auto compute_max_saturation(float a, float b) -> float
{
    // Max saturation will be when one of r, g or b goes below zero.

    // Select different coefficients depending on which component goes below zero first
    float k0, k1, k2, k3, k4, wl, wm, ws; // NOLINT(cppcoreguidelines-init-variables, readability-isolate-declaration)

    if (-1.88170328f * a - 0.80936493f * b > 1)
    {
        // Red component
        k0 = +1.19086277f;
        k1 = +1.76576728f;
        k2 = +0.59662641f;
        k3 = +0.75515197f;
        k4 = +0.56771245f;
        wl = +4.0767416621f;
        wm = -3.3077115913f;
        ws = +0.2309699292f;
    }
    else if (1.81444104f * a - 1.19445276f * b > 1)
    {
        // Green component
        k0 = +0.73956515f;
        k1 = -0.45954404f;
        k2 = +0.08285427f;
        k3 = +0.12541070f;
        k4 = +0.14503204f;
        wl = -1.2684380046f;
        wm = +2.6097574011f;
        ws = -0.3413193965f;
    }
    else
    {
        // Blue component
        k0 = +1.35733652f;
        k1 = -0.00915799f;
        k2 = -1.15130210f;
        k3 = -0.50559606f;
        k4 = +0.00692167f;
        wl = -0.0041960863f;
        wm = -0.7034186147f;
        ws = +1.7076147010f;
    }

    // Approximate max saturation using a polynomial:
    float S = k0 + k1 * a + k2 * b + k3 * a * a + k4 * a * b;

    // Do one step Halley's method to get closer
    // this gives an error less than 10e6, except for some blue hues where the dS/dh is close to infinite
    // this should be sufficient for most applications, otherwise do two/three steps

    float k_l = +0.3963377774f * a + 0.2158037573f * b;
    float k_m = -0.1055613458f * a - 0.0638541728f * b;
    float k_s = -0.0894841775f * a - 1.2914855480f * b;

    {
        float l_ = 1.f + S * k_l;
        float m_ = 1.f + S * k_m;
        float s_ = 1.f + S * k_s;

        float l = l_ * l_ * l_;
        float m = m_ * m_ * m_;
        float s = s_ * s_ * s_;

        float l_dS = 3.f * k_l * l_ * l_;
        float m_dS = 3.f * k_m * m_ * m_;
        float s_dS = 3.f * k_s * s_ * s_;

        float l_dS2 = 6.f * k_l * k_l * l_;
        float m_dS2 = 6.f * k_m * k_m * m_;
        float s_dS2 = 6.f * k_s * k_s * s_;

        float f  = wl * l + wm * m + ws * s;
        float f1 = wl * l_dS + wm * m_dS + ws * s_dS;
        float f2 = wl * l_dS2 + wm * m_dS2 + ws * s_dS2;

        S = S - f * f1 / (f1 * f1 - 0.5f * f * f2);
    }

    return S;
}

struct LC {
    float L;
    float C;
};

// Alternative representation of (L_cusp, C_cusp)
// Encoded so S = C_cusp/L_cusp and T = C_cusp/(1-L_cusp)
// The maximum value for C in the triangle is then found as std::min(S*L, T*(1-L)),
// for a given L
struct ST {
    float S;
    float T;
};

// finds L_cusp and C_cusp for a given hue
// a and b must be normalized so a^2 + b^2 == 1
static auto find_cusp(float a, float b) -> LC
{
    // First, find the maximum saturation (saturation S = C/L)
    float S_cusp = compute_max_saturation(a, b);

    // Convert to linear sRGB to find the first point where at least one of r,g or b >= 1:
    glm::vec3 const rgb_at_max = LinearRGB_from_Oklab({1, S_cusp * a, S_cusp * b});
    float           L_cusp     = cbrtf(1.f / std::max(std::max(rgb_at_max.r, rgb_at_max.g), rgb_at_max.b));
    float           C_cusp     = L_cusp * S_cusp;

    return {L_cusp, C_cusp};
}

// Finds intersection of the line defined by
// L = L0 * (1 - t) + t * L1;
// C = t * C1;
// a and b must be normalized so a^2 + b^2 == 1
static auto find_gamut_intersection(float a, float b, float L1, float C1, float L0, LC cusp) -> float // NOLINT(bugprone-easily-swappable-parameters)
{
    // Find the intersection for upper and lower half seprately
    float t; // NOLINT(cppcoreguidelines-init-variables)
    if (((L1 - L0) * cusp.C - (cusp.L - L0) * C1) <= 0.f)
    {
        // Lower half

        t = cusp.C * L0 / (C1 * cusp.L + cusp.C * (L0 - L1));
    }
    else
    {
        // Upper half

        // First intersect with triangle
        t = cusp.C * (L0 - 1.f) / (C1 * (cusp.L - 1.f) + cusp.C * (L0 - L1));

        // Then one step Halley's method
        {
            float dL = L1 - L0;
            float dC = C1;

            float k_l = +0.3963377774f * a + 0.2158037573f * b;
            float k_m = -0.1055613458f * a - 0.0638541728f * b;
            float k_s = -0.0894841775f * a - 1.2914855480f * b;

            float l_dt = dL + dC * k_l;
            float m_dt = dL + dC * k_m;
            float s_dt = dL + dC * k_s;

            // If higher accuracy is required, 2 or 3 iterations of the following block can be used:
            {
                float L = L0 * (1.f - t) + t * L1;
                float C = t * C1;

                float l_ = L + C * k_l;
                float m_ = L + C * k_m;
                float s_ = L + C * k_s;

                float l = l_ * l_ * l_;
                float m = m_ * m_ * m_;
                float s = s_ * s_ * s_;

                float ldt = 3 * l_dt * l_ * l_;
                float mdt = 3 * m_dt * m_ * m_;
                float sdt = 3 * s_dt * s_ * s_;

                float ldt2 = 6 * l_dt * l_dt * l_;
                float mdt2 = 6 * m_dt * m_dt * m_;
                float sdt2 = 6 * s_dt * s_dt * s_;

                float r  = 4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s - 1;
                float r1 = 4.0767416621f * ldt - 3.3077115913f * mdt + 0.2309699292f * sdt;
                float r2 = 4.0767416621f * ldt2 - 3.3077115913f * mdt2 + 0.2309699292f * sdt2;

                float u_r = r1 / (r1 * r1 - 0.5f * r * r2);
                float t_r = -r * u_r;

                float g  = -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s - 1;
                float g1 = -1.2684380046f * ldt + 2.6097574011f * mdt - 0.3413193965f * sdt;
                float g2 = -1.2684380046f * ldt2 + 2.6097574011f * mdt2 - 0.3413193965f * sdt2;

                float u_g = g1 / (g1 * g1 - 0.5f * g * g2);
                float t_g = -g * u_g;

                float b_ = -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s - 1;
                float b1 = -0.0041960863f * ldt - 0.7034186147f * mdt + 1.7076147010f * sdt;
                float b2 = -0.0041960863f * ldt2 - 0.7034186147f * mdt2 + 1.7076147010f * sdt2;

                float u_b = b1 / (b1 * b1 - 0.5f * b_ * b2);
                float t_b = -b_ * u_b;

                t_r = u_r >= 0.f ? t_r : FLT_MAX;
                t_g = u_g >= 0.f ? t_g : FLT_MAX;
                t_b = u_b >= 0.f ? t_b : FLT_MAX;

                t += std::min(t_r, std::min(t_g, t_b));
            }
        }
    }

    return t;
}

static auto toe(float x) -> float
{
    constexpr float k_1 = 0.206f;
    constexpr float k_2 = 0.03f;
    constexpr float k_3 = (1.f + k_1) / (1.f + k_2);
    return 0.5f * (k_3 * x - k_1 + sqrtf((k_3 * x - k_1) * (k_3 * x - k_1) + 4 * k_2 * k_3 * x));
}

static auto toe_inv(float x) -> float
{
    constexpr float k_1 = 0.206f;
    constexpr float k_2 = 0.03f;
    constexpr float k_3 = (1.f + k_1) / (1.f + k_2);
    return (x * x + k_1 * x) / (k_3 * (x + k_2));
}

static auto to_ST(LC cusp) -> ST
{
    float L = cusp.L;
    float C = cusp.C;
    return {C / L, C / (1 - L)};
}

// Returns a smooth approximation of the location of the cusp
// This polynomial was created by an optimization process
// It has been designed so that S_mid < S_max and T_mid < T_max
static auto get_ST_mid(float a_, float b_) -> ST
{
    float S = 0.11516993f + 1.f / (+7.44778970f + 4.15901240f * b_ + a_ * (-2.19557347f + 1.75198401f * b_ + a_ * (-2.13704948f - 10.02301043f * b_ + a_ * (-4.24894561f + 5.38770819f * b_ + 4.69891013f * a_))));

    float T = 0.11239642f + 1.f / (+1.61320320f - 0.68124379f * b_ + a_ * (+0.40370612f + 0.90148123f * b_ + a_ * (-0.27087943f + 0.61223990f * b_ + a_ * (+0.00299215f - 0.45399568f * b_ - 0.14661872f * a_))));

    return {S, T};
}

struct Cs {
    float C_0;
    float C_mid;
    float C_max;
};

static auto get_Cs(float L, float a_, float b_) -> Cs
{
    LC cusp = find_cusp(a_, b_);

    float C_max  = find_gamut_intersection(a_, b_, L, 1, L, cusp);
    ST    ST_max = to_ST(cusp);

    // Scale factor to compensate for the curved part of gamut shape:
    float k = C_max / std::min((L * ST_max.S), (1 - L) * ST_max.T);

    float C_mid; // NOLINT(cppcoreguidelines-init-variables)
    {
        ST ST_mid = get_ST_mid(a_, b_);

        // Use a soft minimum function, instead of a sharp triangle shape to get a smooth value for chroma.
        float C_a = L * ST_mid.S;
        float C_b = (1.f - L) * ST_mid.T;
        C_mid     = 0.9f * k * sqrtf(sqrtf(1.f / (1.f / (C_a * C_a * C_a * C_a) + 1.f / (C_b * C_b * C_b * C_b))));
    }

    float C_0; // NOLINT(cppcoreguidelines-init-variables)
    {
        // for C_0, the shape is independent of hue, so ST are constant. Values picked to roughly be the average values of ST.
        float C_a = L * 0.4f;
        float C_b = (1.f - L) * 0.8f;

        // Use a soft minimum function, instead of a sharp triangle shape to get a smooth value for chroma.
        C_0 = sqrtf(1.f / (1.f / (C_a * C_a) + 1.f / (C_b * C_b)));
    }

    return {C_0, C_mid, C_max};
}

auto LinearRGB_from_Okhsl(glm::vec3 const& hsl) -> glm::vec3
{
    float h = hsl.x;
    float s = hsl.y;
    float l = hsl.z;

    if (l == 1.0f)
        return {1.f, 1.f, 1.f};
    if (l == 0.f)
        return {0.f, 0.f, 0.f};

    float a_ = cosf(2.f * glm::pi<float>() * h);
    float b_ = sinf(2.f * glm::pi<float>() * h);
    float L  = toe_inv(l);

    Cs    cs    = get_Cs(L, a_, b_);
    float C_0   = cs.C_0;
    float C_mid = cs.C_mid;
    float C_max = cs.C_max;

    float mid     = 0.8f;
    float mid_inv = 1.25f;

    float C, t, k_0, k_1, k_2; // NOLINT(cppcoreguidelines-init-variables, readability-isolate-declaration)

    if (s < mid)
    {
        t = mid_inv * s;

        k_1 = mid * C_0;
        k_2 = (1.f - k_1 / C_mid);

        C = t * k_1 / (1.f - k_2 * t);
    }
    else
    {
        t = (s - mid) / (1 - mid);

        k_0 = C_mid;
        k_1 = (1.f - mid) * C_mid * C_mid * mid_inv * mid_inv / C_0;
        k_2 = (1.f - (k_1) / (C_max - C_mid));

        C = k_0 + t * k_1 / (1.f - k_2 * t);
    }

    return LinearRGB_from_Oklab({L, C * a_, C * b_});
}

auto Okhsl_from_LinearRGB(glm::vec3 const& rgb) -> glm::vec3
{
    glm::vec3 const lab = Oklab_from_LinearRGB(rgb);

    float C  = sqrtf(lab.y * lab.y + lab.z * lab.z);
    float a_ = lab.y / C;
    float b_ = lab.z / C;

    float L = lab.x;
    float h = 0.5f + 0.5f * atan2f(-lab.z, -lab.y) / glm::pi<float>();

    Cs    cs    = get_Cs(L, a_, b_);
    float C_0   = cs.C_0;
    float C_mid = cs.C_mid;
    float C_max = cs.C_max;

    // Inverse of the interpolation in okhsl_to_srgb:

    float mid     = 0.8f;
    float mid_inv = 1.25f;

    float s; // NOLINT(cppcoreguidelines-init-variables)
    if (C < C_mid)
    {
        float k_1 = mid * C_0;
        float k_2 = (1.f - k_1 / C_mid);

        float t = C / (k_1 + k_2 * C);
        s       = t * mid;
    }
    else
    {
        float k_0 = C_mid;
        float k_1 = (1.f - mid) * C_mid * C_mid * mid_inv * mid_inv / C_0;
        float k_2 = (1.f - (k_1) / (C_max - C_mid));

        float t = (C - k_0) / (k_1 + k_2 * (C - k_0));
        s       = mid + (1.f - mid) * t;
    }

    float l = toe(L);
    return {h, s, l};
}

auto sRGB_from_Okhsl(glm::vec3 const& hsl) -> glm::vec3
{
    return sRGB_from_LinearRGB(LinearRGB_from_Okhsl(hsl));
}

auto Okhsl_from_sRGB(glm::vec3 const& rgb) -> glm::vec3
{
    return Okhsl_from_LinearRGB(LinearRGB_from_sRGB(rgb));
}

auto XYZ_from_Okhsl(glm::vec3 const& col) -> glm::vec3
{
    return XYZ_from_LinearRGB(LinearRGB_from_Okhsl(col));
}

auto Okhsl_from_XYZ(glm::vec3 const& col) -> glm::vec3
{
    return Okhsl_from_LinearRGB(LinearRGB_from_XYZ(col));
}

// End of [Block3]

// Start of [Block4]
// https://bottosson.github.io/posts/oklab/
// Copyright(c) 2021 Bj�rn Ottosson
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions : The above copyright
// noticeand this permission notice shall be included in all copies or
// substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS",
// WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
// FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.

auto Oklab_from_XYZ(glm::vec3 const& xyz) -> glm::vec3
{
    glm::vec3 const lms = glm::mat3{
                              +0.8189330101f, +0.0329845436f, +0.0482003018f,
                              +0.3618667424f, +0.9293118715f, +0.2643662691f,
                              -0.1288597137f, +0.0361456387f, +0.6338517070f
                          }
                          * xyz;
    return glm::mat3{
               +0.2104542553f, +1.9779984951f, +0.0259040371f,
               +0.7936177850f, -2.4285922050f, +0.7827717662f,
               +0.0040720468f, +0.4505937099f, -0.8086757660f
           }
           * glm::pow(lms, glm::vec3{1.f / 3.f});
}

auto XYZ_from_Oklab(glm::vec3 const& lab) -> glm::vec3
{
    glm::vec3 const lms = glm::mat3{
                              +1.000000000f, +1.000000000f, +1.000000000f,
                              +0.396337792f, -0.105561342f, -0.089484182f,
                              +0.215803758f, -0.063854175f, -1.291485538f
                          }
                          * lab;
    return glm::mat3{
               +1.227013851f, -0.040580178f, -0.076381285f,
               -0.557799981f, +1.112256870f, -0.421481978f,
               +0.281256149f, -0.071676679f, +1.586163220f
           }
           * lms * lms * lms;
}
// End of [Block4]

#include "generated/conversions_cpp_definition.inl"

} // namespace Cool
