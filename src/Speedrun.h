#pragma once

#include "imgui/font_robotomedium.hpp"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_dx12.h"
#include "utilities/Mod.h"
#include <chrono>
#include <format>
#include <reframework/API.hpp>

static inline std::string game_namespace(std::string_view base_name) {
    return std::string{"chainsaw."} + base_name.data();
}

class Speedrun {
public:
    Speedrun() { load_config(config); }
    void on_draw_ui();

    void draw_stats();

    static void draw_ingame_time();

    static void draw_game_rank();

    static void draw_enemies(int num_to_display, bool boss_only);

    static void draw_money();

    static void draw_spinels();

    static void draw_kills();

    static void draw_companion();

    bool settings_open{true};
    float internal_font_size = 16.0;
    bool m_wants_device_object_cleanup{false};
    bool m_fonts_need_updating{true};
    utility::Config config{"speedrun.txt"};

    constexpr static inline const float sc = 1.0f / 255.0f;
    constexpr static inline const ImVec4 fine = ImVec4(37 * sc, 97 * sc, 68 * sc, 1.0);
    constexpr static inline const ImVec4 kinda_fine = ImVec4(51 * sc, 72 * sc, 24 * sc, 1.0);
    constexpr static inline const ImVec4 caution = ImVec4(94 * sc, 72 * sc, 27 * sc, 1.0);
    constexpr static inline const ImVec4 extra_caution = ImVec4(136 * sc, 1 * sc, 27 * sc, 1.0);
    constexpr static inline const ImVec4 danger = ImVec4(136 * sc, 1 * sc, 27 * sc, 1.0);

    static ImVec4 create_color(const float i) {
        if (i >= 1.0f)
            return fine; // Fine
        if (i < 1.0f && i >= 0.66f)
            return kinda_fine; // That kinda fine, but not color
        if (i < 0.66f && i >= 0.30f)
            return caution; // Caution
        if (i < 0.30f)
            return extra_caution; // Extra Caution
        return danger;            // Danger
    }

    static void make_enemy_health_bar(const float current_health, const float max_health, const std::string_view kind_id) {
        const auto ratio = current_health / max_health;
        const auto color = create_color(ratio);
        const auto fmt_str = std::vformat("{} - ({} / {}) {:.0f}%", std::make_format_args(kind_id, current_health, max_health, ratio * 100.0));
        const auto y_region = ImGui::GetFontSize() > 20.0f ? ImGui::GetFontSize() + 5.0f : 20.0f;
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
        ImGui::ProgressBar(ratio, ImVec2(ImGui::GetContentRegionAvail().x, y_region), fmt_str.data());
        ImGui::PopStyleColor(1);
    }

    void load_config(const utility::Config& cfg) {
        locked->config_load(cfg);
        enabled->config_load(cfg);
        in_game_time->config_load(cfg);
        game_rank->config_load(cfg);
        money->config_load(cfg);
        spinels->config_load(cfg);
        local_enemies->config_load(cfg);
        kill_count->config_load(cfg);
        companion->config_load(cfg);
        font_size->config_load(cfg);
        if (font_size->value() != internal_font_size) {
            internal_font_size = font_size->value();
        }
        num_display->config_load(cfg);
        boss_only->config_load(cfg);
    }

    void save_config(utility::Config& cfg) {
        locked->config_save(cfg);
        enabled->config_save(cfg);
        in_game_time->config_save(cfg);
        game_rank->config_save(cfg);
        money->config_save(cfg);
        spinels->config_save(cfg);
        local_enemies->config_save(cfg);
        kill_count->config_save(cfg);
        companion->config_save(cfg);
        font_size->config_save(cfg);
        num_display->config_save(cfg);
        boss_only->config_save(cfg);
        cfg.save("speedrun.txt");
    }

    void rebuild_fonts() {
        if (!m_fonts_need_updating) {
            return;
        }
        m_fonts_need_updating = false;
        auto& fonts = ImGui::GetIO().Fonts;
        fonts->Clear();
        fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, internal_font_size);
        fonts->Build();
        m_wants_device_object_cleanup = true;
    }

    void invalidate_device_objects() {
        if (!m_wants_device_object_cleanup) {
            return;
        }

        if (reframework::API::get()->param()->renderer_data->renderer_type == REFRAMEWORK_RENDERER_D3D11) {
            ImGui_ImplDX11_InvalidateDeviceObjects();
        } else if (reframework::API::get()->param()->renderer_data->renderer_type == REFRAMEWORK_RENDERER_D3D12) {
            ImGui_ImplDX12_InvalidateDeviceObjects();
        }

        m_wants_device_object_cleanup = false;
    }

private:
    constexpr static const int COLUMNS = 7;
    constexpr static const char* INFO_LABELS[COLUMNS] = {"IGT", "Money", "Spinels", "Rank", "Kill Count", "Companion", "Enemies"};

    int info_order[COLUMNS] = {1, 2, 3, 4, 5, 6, 7};

    static int window_flags(const bool locked) {
        auto base_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus |
                          ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        return (locked) ? base_flags | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs : base_flags;
    }

    [[nodiscard]] static std::string generate_name(std::string_view name) { return std::string{"Speedrun_"} + name.data(); }

    const ModToggle::Ptr locked{ModToggle::create(generate_name("Lock Window"), true)};
    const ModToggle::Ptr enabled{ModToggle::create(generate_name("Enabled"), true)};
    const ModToggle::Ptr in_game_time{ModToggle::create(generate_name("In Game Time"), true)};
    const ModToggle::Ptr game_rank{ModToggle::create(generate_name("Rank"), true)};
    const ModToggle::Ptr money{ModToggle::create(generate_name("Money"), true)};
    const ModToggle::Ptr spinels{ModToggle::create(generate_name("Spinels"), true)};
    const ModToggle::Ptr local_enemies{ModToggle::create(generate_name("Local Enemies"), true)};
    const ModToggle::Ptr kill_count{ModToggle::create(generate_name("Kill Count"), true)};
    const ModToggle::Ptr companion{ModToggle::create(generate_name("Companion Distance"), true)};
    const ModToggle::Ptr boss_only{ModToggle::create(generate_name("Boss Health Only"), true)};
    const ModInt32::Ptr num_display{ModInt32::create(generate_name("Number of Enemies to Display"), 0)};
    const ModFloat::Ptr font_size{ModFloat::create(generate_name("Font Size"), 16.0)};
};