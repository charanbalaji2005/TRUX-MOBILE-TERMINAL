// src/core/art_gallery.cpp
#include "art_gallery.hpp"
#include "graphics/image_decoder.hpp"
#include "graphics/ascii_art_engine.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <dirent.h>
#include <fstream>
#include <random>
#include <sstream>
#include <unistd.h>

namespace meridian::core {

namespace {

inline RgbColor blend(RgbColor c1, RgbColor c2, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    uint8_t r = static_cast<uint8_t>(c1.r * (1.0f - t) + c2.r * t);
    uint8_t g = static_cast<uint8_t>(c1.g * (1.0f - t) + c2.g * t);
    uint8_t b = static_cast<uint8_t>(c1.b * (1.0f - t) + c2.b * t);
    return RgbColor{r, g, b, 255};
}

// -----------------------------------------------------------------------------
// 1. ITACHI UCHIHA: MANGEKYŌ SHARINGAN & TSUKUYOMI BLOOD MOON
// -----------------------------------------------------------------------------
TerminalImage make_itachi_sharingan_art(int w, int h) {
    TerminalImage img(w, h);
    for (int y = 0; y < h; ++y) {
        float ny = static_cast<float>(y) / (h - 1);
        for (int x = 0; x < w; ++x) {
            float nx = static_cast<float>(x) / (w - 1);

            // Deep Obsidian void with subtle crimson ambient fog
            RgbColor col{static_cast<uint8_t>(12 + ny * 6), 4, static_cast<uint8_t>(8 + ny * 4), 255};

            // Sharingan Eye Center
            float cx = (nx - 0.50f) * 1.3f;
            float cy = (ny - 0.50f);
            float dist = std::sqrt(cx * cx + cy * cy);
            float angle = std::atan2(cy, cx);

            // Outer Sclera / Red Glow
            if (dist < 0.46f) {
                float glow = (0.46f - dist) / 0.46f;
                col = blend(col, RgbColor{160, 10, 20, 255}, glow * 0.5f);
            }

            // Iris Outer Ring (Crimson Red)
            if (dist < 0.38f) {
                float iris_t = (0.38f - dist) / 0.38f;
                RgbColor iris_col = blend(RgbColor{190, 15, 25, 255}, RgbColor{255, 45, 55, 255}, iris_t);
                col = iris_col;

                // Black Outer Iris Border
                if (dist > 0.35f) {
                    col = RgbColor{20, 2, 5, 255};
                }

                // Inner Pinwheel Blade (Itachi's 3-curved Mangekyō Blades)
                float blade_angle = std::fmod(angle + 3.14159f * 2.0f, 3.14159f * 2.0f / 3.0f) - (3.14159f / 3.0f);
                float blade_dist = dist * 2.2f;
                float curve = std::sin(blade_dist * 3.14159f) * 0.35f;

                if (dist > 0.08f && std::abs(blade_angle - curve) < 0.30f && dist < 0.33f) {
                    col = RgbColor{10, 2, 4, 255}; // Black Pinwheel Blade
                }

                // Center Pupil (Jet Black)
                if (dist < 0.09f) {
                    col = RgbColor{5, 1, 2, 255};
                }
                // Central Ring Highlight
                else if (dist > 0.18f && dist < 0.21f) {
                    col = blend(col, RgbColor{255, 100, 110, 255}, 0.5f);
                }
            }

            // Flying Tsukuyomi Crow Silhouettes (top-left and top-right)
            float cr1_x = nx - 0.18f; float cr1_y = ny - 0.22f;
            float cr2_x = nx - 0.82f; float cr2_y = ny - 0.28f;
            if ((std::abs(cr1_x * 2.0f + cr1_y) < 0.04f && std::abs(cr1_y) < 0.05f) ||
                (std::abs(cr2_x * 2.0f - cr2_y) < 0.04f && std::abs(cr2_y) < 0.05f)) {
                col = RgbColor{0, 0, 0, 255};
            }

            img.set_pixel(x, y, col);
        }
    }
    return img;
}

// -----------------------------------------------------------------------------
// 2. JUJUTSU KAISEN: GOJO SATORU — INFINITE VOID & HOLLOW PURPLE
// -----------------------------------------------------------------------------
TerminalImage make_gojo_hollow_purple_art(int w, int h) {
    TerminalImage img(w, h);
    for (int y = 0; y < h; ++y) {
        float ny = static_cast<float>(y) / (h - 1);
        for (int x = 0; x < w; ++x) {
            float nx = static_cast<float>(x) / (w - 1);

            // Infinite Void Space Background (Deep Midnight Navy)
            RgbColor col{8, 6, 24, 255};

            // Space Distortion Waves
            float wave = std::sin(nx * 12.0f + ny * 8.0f) * 0.05f;

            // Hollow Purple Singularity Sphere (Center)
            float px = (nx - 0.50f) * 1.3f;
            float py = (ny - 0.50f);
            float pdist = std::sqrt(px * px + py * py) + wave;

            // Reversal Red (Left side energy)
            float rx = (nx - 0.35f) * 1.3f;
            float rdist = std::sqrt(rx * rx + py * py);
            if (rdist < 0.28f) {
                float r_int = (0.28f - rdist) / 0.28f;
                col = blend(col, RgbColor{239, 68, 68, 255}, r_int * 0.75f);
            }

            // Lapse Blue (Right side energy)
            float bx = (nx - 0.65f) * 1.3f;
            float bdist = std::sqrt(bx * bx + py * py);
            if (bdist < 0.28f) {
                float b_int = (0.28f - bdist) / 0.28f;
                col = blend(col, RgbColor{56, 189, 248, 255}, b_int * 0.75f);
            }

            // Purple Collision Singularity Core
            if (pdist < 0.24f) {
                float p_int = (0.24f - pdist) / 0.24f;
                RgbColor purple_edge{168, 85, 247, 255};
                RgbColor purple_core{245, 230, 255, 255}; // Blinding White Core
                col = blend(purple_edge, purple_core, p_int * p_int);
            }

            // Gojo's Snow White Spiky Hair (Top Horizon)
            if (ny < 0.25f) {
                float hair_spike = std::sin(nx * 30.0f) * 0.06f;
                if (ny < (0.16f + hair_spike)) {
                    col = blend(RgbColor{240, 248, 255, 255}, RgbColor{186, 230, 253, 255}, ny * 4.0f);
                }
            }

            // Six-Eyes Cerulean Sparkle (top center)
            float ex = nx - 0.50f; float ey = ny - 0.26f;
            if (std::sqrt(ex * ex * 2.0f + ey * ey * 4.0f) < 0.04f) {
                col = RgbColor{14, 165, 233, 255};
            }

            img.set_pixel(x, y, col);
        }
    }
    return img;
}

// -----------------------------------------------------------------------------
// 3. JUJUTSU KAISEN: RYOMEN SUKUNA — MALEVOLENT SHRINE & CURSED FLAMES
// -----------------------------------------------------------------------------
TerminalImage make_sukuna_art(int w, int h) {
    TerminalImage img(w, h);
    for (int y = 0; y < h; ++y) {
        float ny = static_cast<float>(y) / (h - 1);
        for (int x = 0; x < w; ++x) {
            float nx = static_cast<float>(x) / (w - 1);

            // Blood Red & Charcoal Sky
            RgbColor col = blend(RgbColor{55, 10, 15, 255}, RgbColor{15, 5, 8, 255}, ny);

            // Malevolent Shrine Roof Silhouette (Bottom horizon)
            float sx = std::abs(nx - 0.5f) * 2.0f;
            float shrine_curve = 0.65f + std::pow(sx, 1.8f) * 0.25f;
            if (ny > shrine_curve) {
                col = RgbColor{10, 2, 4, 255}; // Deep Black Shrine Roof
            }

            // Sukuna's Cursed Tattoo Lines (Face Markings)
            float tx = std::abs(nx - 0.5f);
            if ((std::abs(tx - 0.18f) < 0.025f && ny > 0.25f && ny < 0.65f) ||
                (std::abs(ny - 0.38f) < 0.025f && tx < 0.32f)) {
                col = RgbColor{15, 5, 8, 255}; // Black Tattoo Ink
            }

            // Sukuna's 4 Demonic Glowing Crimson Eyes
            bool eye1 = (std::abs(nx - 0.40f) < 0.03f && std::abs(ny - 0.34f) < 0.02f);
            bool eye2 = (std::abs(nx - 0.60f) < 0.03f && std::abs(ny - 0.34f) < 0.02f);
            bool eye3 = (std::abs(nx - 0.38f) < 0.028f && std::abs(ny - 0.42f) < 0.018f);
            bool eye4 = (std::abs(nx - 0.62f) < 0.028f && std::abs(ny - 0.42f) < 0.018f);

            if (eye1 || eye2 || eye3 || eye4) {
                col = RgbColor{255, 30, 45, 255}; // Intense Demon Red
            }

            // Divine Flame "Fuga" Embers rising
            if ((x * 23 + y * 47) % 31 == 0 && ny < 0.65f) {
                col = RgbColor{255, 165, 0, 255}; // Golden Fire Ember
            }

            img.set_pixel(x, y, col);
        }
    }
    return img;
}

// -----------------------------------------------------------------------------
// 4. NARUTO: SAGE MODE & KURAMA RASENGAN
// -----------------------------------------------------------------------------
TerminalImage make_naruto_art(int w, int h) {
    TerminalImage img(w, h);
    for (int y = 0; y < h; ++y) {
        float ny = static_cast<float>(y) / (h - 1);
        for (int x = 0; x < w; ++x) {
            float nx = static_cast<float>(x) / (w - 1);

            // Dark Leaf Village Night Sky
            RgbColor col{12, 14, 28, 255};

            // Nine-Tails (Kurama) Golden-Orange Chakra Aura
            float k_dist = std::abs((nx - 0.5f) * 1.5f + (ny - 0.5f));
            if (k_dist < 0.60f) {
                float k_glow = (0.60f - k_dist) / 0.60f;
                RgbColor kurama_col = blend(RgbColor{255, 120, 0, 255}, RgbColor{255, 210, 0, 255}, k_glow);
                col = blend(col, kurama_col, k_glow * 0.55f);
            }

            // Swirling Rasengan Chakra Sphere (Center)
            float rx = (nx - 0.50f) * 1.3f;
            float ry = (ny - 0.50f);
            float rdist = std::sqrt(rx * rx + ry * ry);
            float r_angle = std::atan2(ry, rx);

            if (rdist < 0.28f) {
                // Spiral Chakra Vortex Lines
                float spiral = std::sin(r_angle * 4.0f + rdist * 25.0f);
                float r_int = (0.28f - rdist) / 0.28f;

                RgbColor cyan_edge{0, 180, 255, 255};
                RgbColor white_core{230, 250, 255, 255};
                RgbColor base_rasengan = blend(cyan_edge, white_core, r_int * r_int);

                if (spiral > 0.3f) {
                    base_rasengan = blend(base_rasengan, RgbColor{255, 255, 255, 255}, 0.6f);
                }
                col = base_rasengan;
            }

            // Sage Mode Orange Pigmentation & Frog Eyes (Top)
            if (std::abs(ny - 0.24f) < 0.035f && (std::abs(nx - 0.38f) < 0.06f || std::abs(nx - 0.62f) < 0.06f)) {
                col = RgbColor{255, 140, 0, 255}; // Sage Orange Eyeliner
                // Horizontal bar frog pupil
                if (std::abs(ny - 0.24f) < 0.012f && (std::abs(nx - 0.38f) < 0.035f || std::abs(nx - 0.62f) < 0.035f)) {
                    col = RgbColor{10, 5, 2, 255};
                }
            }

            img.set_pixel(x, y, col);
        }
    }
    return img;
}

// -----------------------------------------------------------------------------
// 5. DEMON SLAYER: RENGOKU — SUN BREATHING FLAME TIGER
// -----------------------------------------------------------------------------
TerminalImage make_rengoku_art(int w, int h) {
    TerminalImage img(w, h);
    for (int y = 0; y < h; ++y) {
        float ny = static_cast<float>(y) / (h - 1);
        for (int x = 0; x < w; ++x) {
            float nx = static_cast<float>(x) / (w - 1);

            // Midnight Black & Crimson Smoke
            RgbColor col = blend(RgbColor{10, 4, 8, 255}, RgbColor{40, 8, 12, 255}, ny);

            // Blazing Sun Breathing Flame Waves
            float flame_wave = std::sin(nx * 10.0f + ny * 6.0f) * 0.12f;
            float f_dist = std::abs((nx * 1.1f - ny * 0.9f) - 0.1f) + flame_wave;

            if (f_dist < 0.32f) {
                float intensity = (0.32f - f_dist) / 0.32f;
                RgbColor flame_col = blend(RgbColor{220, 20, 20, 255}, RgbColor{255, 215, 0, 255}, intensity);
                col = blend(col, flame_col, intensity * 0.90f);
            }

            // Nichirin Katana Black Blade with Glowing Red Edge
            float blade_dist = std::abs((nx * 0.85f + ny * 1.0f) - 0.70f);
            if (blade_dist < 0.035f && ny > 0.25f) {
                col = (blade_dist < 0.015f) ? RgbColor{15, 12, 15, 255} : RgbColor{255, 60, 40, 255};
            }

            // Floating Golden Embers
            if ((x * 19 + y * 29) % 23 == 0) {
                col = RgbColor{255, 220, 50, 255};
            }

            img.set_pixel(x, y, col);
        }
    }
    return img;
}

// -----------------------------------------------------------------------------
// 6. DRAGON BALL: ULTRA INSTINCT GOKU SILVER AURA
// -----------------------------------------------------------------------------
TerminalImage make_ultra_instinct_art(int w, int h) {
    TerminalImage img(w, h);
    for (int y = 0; y < h; ++y) {
        float ny = static_cast<float>(y) / (h - 1);
        for (int x = 0; x < w; ++x) {
            float nx = static_cast<float>(x) / (w - 1);

            // Deep Cobalt Space Background
            RgbColor col{6, 10, 25, 255};

            // Ultra Instinct Divine Silver-Blue Aura Pillar
            float ax = std::abs(nx - 0.50f);
            float aura_wave = std::sin(ny * 16.0f + nx * 8.0f) * 0.04f;
            float adist = ax + aura_wave;

            if (adist < 0.38f) {
                float a_int = (0.38f - adist) / 0.38f;
                RgbColor aura_blue{56, 189, 248, 255};
                RgbColor aura_silver{240, 245, 255, 255};
                RgbColor aura_col = blend(aura_blue, aura_silver, a_int);
                col = blend(col, aura_col, a_int * 0.85f);
            }

            // Ultra Instinct Silver Gaze (Eyes)
            if (std::abs(ny - 0.44f) < 0.022f && (std::abs(nx - 0.42f) < 0.035f || std::abs(nx - 0.58f) < 0.035f)) {
                col = RgbColor{245, 245, 255, 255}; // Pure Silver Iris
            }

            // Divine Aura Particles
            if ((x * 31 + y * 13) % 27 == 0) {
                col = RgbColor{220, 240, 255, 255};
            }

            img.set_pixel(x, y, col);
        }
    }
    return img;
}

// -----------------------------------------------------------------------------
// 7. CHAINSAW MAN: POWER & DENJI
// -----------------------------------------------------------------------------
TerminalImage make_chainsaw_man_art(int w, int h) {
    TerminalImage img(w, h);
    for (int y = 0; y < h; ++y) {
        float ny = static_cast<float>(y) / (h - 1);
        for (int x = 0; x < w; ++x) {
            float nx = static_cast<float>(x) / (w - 1);

            RgbColor col{static_cast<uint8_t>(18 + ny * 10), static_cast<uint8_t>(20 + ny * 8), static_cast<uint8_t>(26 + ny * 12), 255};

            float slash_dist = std::abs((nx * 1.2f - ny * 0.9f) - 0.2f);
            if (slash_dist < 0.25f) {
                float intensity = 1.0f - (slash_dist / 0.25f);
                RgbColor slash_col = blend(RgbColor{220, 20, 50, 255}, RgbColor{255, 90, 95, 255}, intensity);
                col = blend(col, slash_col, intensity * 0.85f);
            }

            float cx = nx - 0.55f;
            float cy = ny - 0.45f;
            float dist_face = std::sqrt(cx * cx * 1.5f + cy * cy * 2.0f);

            if (dist_face < 0.38f && ny < 0.65f) {
                float hair_spec = std::sin(nx * 20.0f + ny * 10.0f) * 0.15f;
                RgbColor hair_col = blend(RgbColor{235, 220, 195, 255}, RgbColor{255, 245, 230, 255}, ny + hair_spec);
                col = blend(col, hair_col, 0.90f);
            }

            float h1_x = nx - 0.48f; float h1_y = ny - 0.22f;
            float h2_x = nx - 0.62f; float h2_y = ny - 0.20f;
            if ((std::abs(h1_x + h1_y * 0.4f) < 0.04f && h1_y < 0.12f && h1_y > -0.18f) ||
                (std::abs(h2_x - h2_y * 0.4f) < 0.04f && h2_y < 0.12f && h2_y > -0.18f)) {
                col = RgbColor{185, 28, 28, 255};
            }

            if ((std::abs(nx - 0.50f) < 0.035f && std::abs(ny - 0.42f) < 0.025f) ||
                (std::abs(nx - 0.60f) < 0.035f && std::abs(ny - 0.41f) < 0.025f)) {
                col = RgbColor{255, 183, 3, 255};
            }

            img.set_pixel(x, y, col);
        }
    }
    return img;
}

// -----------------------------------------------------------------------------
// 8. CYBERPUNK EDGERUNNERS: LUCY & NIGHT CITY
// -----------------------------------------------------------------------------
TerminalImage make_cyberpunk_art(int w, int h) {
    TerminalImage img(w, h);
    for (int y = 0; y < h; ++y) {
        float ny = static_cast<float>(y) / (h - 1);
        for (int x = 0; x < w; ++x) {
            float nx = static_cast<float>(x) / (w - 1);

            RgbColor sky_top{15, 10, 35, 255};
            RgbColor sky_bot{55, 20, 90, 255};
            RgbColor col = blend(sky_top, sky_bot, ny);

            int bldg_idx = static_cast<int>(nx * 8.0f);
            float bldg_h = 0.40f + ((bldg_idx * 7) % 5) * 0.09f;
            if (ny > (1.0f - bldg_h)) {
                col = RgbColor{16, 12, 28, 255};
                if (((x % 3 == 0) && (y % 2 == 0)) && ny > (1.1f - bldg_h)) {
                    col = ((x + y) % 5 == 0) ? RgbColor{0, 240, 255, 255} : RgbColor{254, 228, 64, 255};
                }
            }

            float mx = nx - 0.72f;
            float my = ny - 0.30f;
            float mdist = std::sqrt(mx * mx + my * my);
            if (mdist < 0.16f) {
                col = blend(col, RgbColor{0, 240, 255, 255}, 0.85f);
            } else if (std::abs(mdist - 0.22f) < 0.02f) {
                col = RgbColor{255, 0, 128, 255};
            }

            img.set_pixel(x, y, col);
        }
    }
    return img;
}

// -----------------------------------------------------------------------------
// 9. SYNTHWAVE SUNSET & HORIZON
// -----------------------------------------------------------------------------
TerminalImage make_synthwave_art(int w, int h) {
    TerminalImage img(w, h);
    for (int y = 0; y < h; ++y) {
        float ny = static_cast<float>(y) / (h - 1);
        for (int x = 0; x < w; ++x) {
            float nx = static_cast<float>(x) / (w - 1);

            RgbColor sky_top{38, 12, 60, 255};
            RgbColor sky_bot{180, 25, 110, 255};
            RgbColor col = blend(sky_top, sky_bot, ny * 1.5f);

            float sx = nx - 0.5f;
            float sy = ny - 0.45f;
            float sdist = std::sqrt(sx * sx + sy * sy);
            if (sdist < 0.32f && ny < 0.65f) {
                bool scanline = (y % 3 == 0) && (ny > 0.40f);
                if (!scanline) {
                    float sun_grad = (ny - 0.15f) / 0.50f;
                    col = blend(RgbColor{255, 220, 0, 255}, RgbColor{255, 60, 90, 255}, sun_grad);
                }
            }

            if (ny >= 0.65f) {
                float grid_y = (ny - 0.65f) / 0.35f;
                col = blend(RgbColor{20, 10, 35, 255}, RgbColor{5, 2, 15, 255}, grid_y);
                float persp_x = (nx - 0.5f) / (grid_y + 0.15f);
                bool v_line = std::abs(std::fmod(persp_x * 4.0f + 10.0f, 1.0f) - 0.5f) < 0.08f;
                bool h_line = (y == static_cast<int>(h * 0.68f) || y == static_cast<int>(h * 0.74f) ||
                               y == static_cast<int>(h * 0.82f) || y == static_cast<int>(h * 0.92f));

                if (v_line || h_line) {
                    col = RgbColor{0, 245, 212, 255};
                }
            }

            img.set_pixel(x, y, col);
        }
    }
    return img;
}

// -----------------------------------------------------------------------------
// 10. STUDIO GHIBLI: ANIME MEADOW & AZURE SKY
// -----------------------------------------------------------------------------
TerminalImage make_ghibli_art(int w, int h) {
    TerminalImage img(w, h);
    for (int y = 0; y < h; ++y) {
        float ny = static_cast<float>(y) / (h - 1);
        for (int x = 0; x < w; ++x) {
            float nx = static_cast<float>(x) / (w - 1);

            RgbColor sky_top{50, 130, 220, 255};
            RgbColor sky_bot{170, 225, 250, 255};
            RgbColor col = blend(sky_top, sky_bot, ny * 1.6f);

            float cloud1 = std::sqrt(std::pow(nx - 0.35f, 2) + std::pow((ny - 0.28f) * 1.8f, 2));
            float cloud2 = std::sqrt(std::pow(nx - 0.55f, 2) + std::pow((ny - 0.22f) * 1.8f, 2));
            if (cloud1 < 0.20f || cloud2 < 0.22f) {
                col = blend(col, RgbColor{255, 255, 255, 255}, 0.92f);
            }

            float hill1 = 0.60f + std::sin(nx * 4.0f) * 0.08f;
            float hill2 = 0.72f + std::cos(nx * 5.0f) * 0.06f;

            if (ny >= hill2) {
                col = blend(RgbColor{45, 160, 80, 255}, RgbColor{30, 110, 50, 255}, (ny - hill2) * 2.5f);
                if ((x * 13 + y * 7) % 19 == 0) col = RgbColor{255, 105, 180, 255};
            } else if (ny >= hill1) {
                col = blend(RgbColor{80, 190, 100, 255}, RgbColor{50, 140, 70, 255}, (ny - hill1) * 3.0f);
            }

            img.set_pixel(x, y, col);
        }
    }
    return img;
}

TerminalImage load_gallery_file(const std::string& filename, int w, int h) {
    const char* home = std::getenv("HOME");
    std::vector<std::string> search_paths = {
        "resources/images/gallery/" + filename,
        "../resources/images/gallery/" + filename,
        std::string(home ? home : "") + "/.config/meridian/gallery/" + filename,
        "/usr/local/share/meridian/images/gallery/" + filename,
        "/usr/share/meridian/images/gallery/" + filename,
        "/home/charanbalaji/.gemini/antigravity/brain/782459be-1b65-48a0-8575-26e8a2d72710/.user_uploaded/" + filename
    };

    for (const auto& path : search_paths) {
        if (!path.empty() && access(path.c_str(), R_OK) == 0) {
            TerminalImage img;
            if (img.load_file(path)) {
                return img;
            }
        }
    }

    return make_itachi_sharingan_art(w, h);
}

} // namespace

size_t ArtGallery::theme_count() {
    return 14;
}

std::vector<std::pair<std::string, std::string>> ArtGallery::list_themes() {
    return {
        {"itachi_sharingan", "Itachi Uchiha: Mangekyō Sharingan & Crows"},
        {"shadow_swordsman", "Shadow Swordsman: Blood Moon Requiem"},
        {"ribbon_girl",      "Monochrome Ribbon Girl by Window"},
        {"gojo_six_eyes",    "Gojo Satoru: Six Eyes & Red Spark"},
        {"gojo_awakening",   "Gojo Satoru: Honored One Awakening"},
        {"sunset_girl",      "Sunset City Anime Girl on Balcony"},
        {"sharingan_eye",    "Sasuke Mangekyō Eye"},
        {"sakura_girl",      "Sakura Blossom Girl"},
        {"fan_girl",         "Anime Girl with Fan"},
        {"gojo_purple",      "Gojo: Hollow Purple (JJK)"},
        {"sukuna_shrine",    "Sukuna: Malevolent Shrine (JJK)"},
        {"naruto_rasengan",  "Naruto: Kurama Rasengan"},
        {"rengoku_flames",   "Rengoku: Sun Breathing"},
        {"ultra_instinct",   "Goku: Ultra Instinct"}
    };
}

std::string ArtGallery::get_configured_choice() {
    const char* home = std::getenv("HOME");
    if (!home) return "random";
    std::string config_file = std::string(home) + "/.config/meridian/artwork_choice";
    std::ifstream in(config_file);
    if (!in.is_open()) return "random";
    std::string choice;
    if (std::getline(in, choice)) {
        while (!choice.empty() && (choice.back() == '\r' || choice.back() == '\n' || choice.back() == ' ')) choice.pop_back();
        while (!choice.empty() && (choice.front() == ' ' || choice.front() == '\t')) choice.erase(0, 1);
        if (!choice.empty()) return choice;
    }
    return "random";
}

bool ArtGallery::set_permanent_choice(const std::string& choice) {
    const char* home = std::getenv("HOME");
    if (!home) return false;
    std::string config_dir = std::string(home) + "/.config/meridian";
    system(("mkdir -p " + config_dir).c_str());
    std::string config_file = config_dir + "/artwork_choice";
    std::ofstream out(config_file);
    if (!out.is_open()) return false;
    out << choice << "\n";
    return true;
}

ArtworkTheme ArtGallery::get_artwork_by_id_or_file(const std::string& id_or_file, int width, int height) {
    if (id_or_file == "random") {
        return get_next_artwork(width, height);
    }

    // Check if numeric index
    try {
        size_t idx = std::stoul(id_or_file);
        if (idx < theme_count()) {
            return get_artwork_by_index(idx, width, height);
        }
    } catch (...) {}

    // Check uploaded real images
    if (id_or_file == "itachi" || id_or_file == "itachi_sharingan" || id_or_file == "crows") return ArtworkTheme{"itachi_sharingan", "Itachi Uchiha: Mangekyō Sharingan & Crows", load_gallery_file("itachi_sharingan.png", width, height)};
    if (id_or_file == "swordsman" || id_or_file == "shadow_swordsman" || id_or_file == "blood_moon") return ArtworkTheme{"shadow_swordsman", "Shadow Swordsman: Blood Moon Requiem", load_gallery_file("shadow_swordsman.png", width, height)};
    if (id_or_file == "ribbon_girl" || id_or_file == "ribbon" || id_or_file == "window") return ArtworkTheme{"ribbon_girl", "Monochrome Ribbon Girl by Window", load_gallery_file("ribbon_girl.png", width, height)};
    if (id_or_file == "gojo_six_eyes" || id_or_file == "gojo" || id_or_file == "six_eyes") return ArtworkTheme{"gojo_six_eyes", "Gojo Satoru: Six Eyes & Red Spark", load_gallery_file("gojo_six_eyes.png", width, height)};
    if (id_or_file == "gojo_awakening" || id_or_file == "awakening" || id_or_file == "honored_one") return ArtworkTheme{"gojo_awakening", "Gojo Satoru: Honored One Awakening", load_gallery_file("gojo_awakening.png", width, height)};
    if (id_or_file == "sunset_girl" || id_or_file == "sunset" || id_or_file == "city_girl") return ArtworkTheme{"sunset_girl", "Sunset City Anime Girl on Balcony", load_gallery_file("sunset_girl.png", width, height)};
    if (id_or_file == "sharingan_eye" || id_or_file == "eye" || id_or_file == "sasuke") return ArtworkTheme{"sharingan_eye", "Sasuke Mangekyō Eye", load_gallery_file("sharingan_eye.png", width, height)};
    if (id_or_file == "sakura_girl" || id_or_file == "sakura") return ArtworkTheme{"sakura_girl", "Sakura Blossom Girl", load_gallery_file("sakura_girl.png", width, height)};
    if (id_or_file == "fan_girl" || id_or_file == "fan") return ArtworkTheme{"fan_girl", "Anime Girl with Fan", load_gallery_file("fan_girl.png", width, height)};

    // Check procedural themes
    if (id_or_file == "purple" || id_or_file == "gojo_purple") return ArtworkTheme{"gojo_purple", "Gojo: Hollow Purple", make_gojo_hollow_purple_art(width, height)};
    if (id_or_file == "sukuna" || id_or_file == "shrine" || id_or_file == "sukuna_shrine") return ArtworkTheme{"sukuna_shrine", "Sukuna: Malevolent Shrine", make_sukuna_art(width, height)};
    if (id_or_file == "naruto" || id_or_file == "rasengan" || id_or_file == "naruto_rasengan") return ArtworkTheme{"naruto_rasengan", "Naruto: Kurama Rasengan", make_naruto_art(width, height)};
    if (id_or_file == "rengoku" || id_or_file == "flames" || id_or_file == "rengoku_flames") return ArtworkTheme{"rengoku_flames", "Rengoku: Sun Breathing", make_rengoku_art(width, height)};
    if (id_or_file == "goku" || id_or_file == "ultra_instinct" || id_or_file == "ui") return ArtworkTheme{"ultra_instinct", "Goku: Ultra Instinct", make_ultra_instinct_art(width, height)};
    if (id_or_file == "chainsaw" || id_or_file == "chainsaw_man" || id_or_file == "denji" || id_or_file == "power") return ArtworkTheme{"chainsaw_man", "Chainsaw Man", make_chainsaw_man_art(width, height)};

    // Check if it is a custom image file on disk
    if (access(id_or_file.c_str(), R_OK) == 0) {
        TerminalImage custom_img;
        if (custom_img.load_file(id_or_file)) {
            return ArtworkTheme{"custom", "Custom File: " + id_or_file, custom_img};
        }
    }

    return get_artwork_by_index(0, width, height);
}

ArtworkTheme ArtGallery::get_active_artwork(int width, int height) {
    const char* env_art = std::getenv("MERIDIAN_ARTWORK");
    if (env_art && std::string(env_art).length() > 0) {
        return get_artwork_by_id_or_file(env_art, width, height);
    }

    std::string choice = get_configured_choice();
    if (choice.empty() || choice == "random") {
        return get_next_artwork(width, height);
    }
    return get_artwork_by_id_or_file(choice, width, height);
}

ArtworkTheme ArtGallery::get_artwork_by_index(size_t index, int width, int height) {
    size_t theme_id = index % theme_count();

    switch (theme_id) {
        case 0: return ArtworkTheme{"itachi_sharingan", "Itachi Uchiha: Mangekyō Sharingan & Crows", load_gallery_file("itachi_sharingan.png", width, height)};
        case 1: return ArtworkTheme{"shadow_swordsman", "Shadow Swordsman: Blood Moon Requiem", load_gallery_file("shadow_swordsman.png", width, height)};
        case 2: return ArtworkTheme{"ribbon_girl", "Monochrome Ribbon Girl by Window", load_gallery_file("ribbon_girl.png", width, height)};
        case 3: return ArtworkTheme{"gojo_six_eyes", "Gojo Satoru: Six Eyes & Red Spark", load_gallery_file("gojo_six_eyes.png", width, height)};
        case 4: return ArtworkTheme{"gojo_awakening", "Gojo Satoru: Honored One Awakening", load_gallery_file("gojo_awakening.png", width, height)};
        case 5: return ArtworkTheme{"sunset_girl", "Sunset City Anime Girl on Balcony", load_gallery_file("sunset_girl.png", width, height)};
        case 6: return ArtworkTheme{"sharingan_eye", "Sasuke Mangekyō Eye", load_gallery_file("sharingan_eye.png", width, height)};
        case 7: return ArtworkTheme{"sakura_girl", "Sakura Blossom Girl", load_gallery_file("sakura_girl.png", width, height)};
        case 8: return ArtworkTheme{"fan_girl", "Anime Girl with Fan", load_gallery_file("fan_girl.png", width, height)};
        case 9: return ArtworkTheme{"gojo_purple", "Gojo: Hollow Purple", make_gojo_hollow_purple_art(width, height)};
        case 10: return ArtworkTheme{"sukuna_shrine", "Sukuna: Malevolent Shrine", make_sukuna_art(width, height)};
        case 11: return ArtworkTheme{"naruto_rasengan", "Naruto: Kurama Rasengan", make_naruto_art(width, height)};
        case 12: return ArtworkTheme{"rengoku_flames", "Rengoku: Sun Breathing", make_rengoku_art(width, height)};
        case 13: default: return ArtworkTheme{"ultra_instinct", "Goku: Ultra Instinct", make_ultra_instinct_art(width, height)};
    }
}

ArtworkTheme ArtGallery::get_next_artwork(int width, int height) {
    static size_t session_counter = 0;
    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    size_t pick = (static_cast<size_t>(now) + getpid() + (session_counter++)) % theme_count();
    return get_artwork_by_index(pick, width, height);
}

std::vector<std::string> ArtGallery::render_artwork_lines(const TerminalImage& img, int target_cols, int target_rows) {
    std::vector<std::string> lines;
    int src_w = img.width();
    int src_h = img.height();
    if (src_w <= 0 || src_h <= 0 || target_cols <= 0 || target_rows <= 0) return lines;

    std::vector<uint8_t> rgba(src_w * src_h * 4);
    for (int y = 0; y < src_h; ++y) {
        for (int x = 0; x < src_w; ++x) {
            auto c = img.get_pixel(x, y);
            int idx = (y * src_w + x) * 4;
            rgba[idx + 0] = c.r;
            rgba[idx + 1] = c.g;
            rgba[idx + 2] = c.b;
            rgba[idx + 3] = c.a;
        }
    }

    int grid_w = target_cols;
    int grid_h = target_rows * 2;

    auto resampled = graphics::AsciiArtEngine::resample_lanczos3(rgba, src_w, src_h, grid_w, grid_h);
    graphics::AsciiArtEngine::apply_color_adjustments(resampled, grid_w, grid_h, 1.05f, 1.15f, 0.98f, 1.15f);
    graphics::AsciiArtEngine::apply_edge_enhancement(resampled, grid_w, grid_h, 0.35f);

    std::string ansi = graphics::AsciiArtEngine::emit_truecolor_halfblocks(resampled, grid_w, grid_h, true);

    std::stringstream ss(ansi);
    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty() || lines.size() < static_cast<size_t>(target_rows)) {
            lines.push_back(line);
        }
    }

    return lines;
}

} // namespace meridian::core
