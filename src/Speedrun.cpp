#pragma once
#include "Speedrun.h"
#include "enums.h"
#include <chrono>
#include <sstream>
#include <utility>

using namespace reframework;

static auto display(std::stringstream& os, std::chrono::nanoseconds ns) {
    char fill = os.fill();
    os.fill('0');
    const auto h = std::chrono::duration_cast<std::chrono::hours>(ns);
    ns -= h;
    const auto m = std::chrono::duration_cast<std::chrono::minutes>(ns);
    ns -= m;
    const auto s = std::chrono::duration_cast<std::chrono::seconds>(ns);
    ns -= s;
    const auto mill = std::chrono::duration_cast<std::chrono::milliseconds>(ns);
    os << std::setw(2) << h.count() << "h:" << std::setw(2) << m.count() << "m:" << std::setw(2) << s.count() << "s." << mill.count();
    os.fill(fill);
}

static auto frame_time = 0.0;

void Speedrun::on_draw_ui() {
    ImGui::Begin("Speedrun Settings", &settings_open, ImGuiWindowFlags_AlwaysAutoResize);

    enabled->draw("Enabled");
    locked->draw("Lock Window");
    in_game_time->draw("In Game Time");
    game_rank->draw("Game Rank");
    money->draw("Money");
    local_enemies->draw("Nearby Enemies");
    kill_count->draw("Kill Count");
    boss_only->draw("Boss Health Only");

    if (!boss_only->value()) {
        num_display->draw("Enemies to Display");
        ImGui::Text("Use 0 to display all enemies");
    }

    if (font_size->draw("Font Size")) {
        if (font_size->value() != internal_font_size && font_size->value() >= 8.0) {
            internal_font_size = font_size->value();
            m_fonts_need_updating = true;
        }
    }

    ImGui::NewLine();
    ImGui::Text("Last Frametime: : %.3fms", frame_time);

    ImGui::NewLine();
    ImGui::BulletText("Drag and drop to re-order");
    int move_from = -1, move_to = -1;
    for (int n = 0; n < COLUMNS; n++) {
        ImGui::Selectable(INFO_LABELS[info_order[n] - 1], info_order[n]);
        ImGuiDragDropFlags src_flags = ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers | ImGuiDragDropFlags_SourceNoPreviewTooltip;
        if (ImGui::BeginDragDropSource(src_flags)) {
            ImGui::SetDragDropPayload("MENU_ORDER", &n, sizeof(int));
            ImGui::EndDragDropSource();
        }
        if (ImGui::BeginDragDropTarget()) {
            ImGuiDragDropFlags target_flags = ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect;
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MENU_ORDER", target_flags)) {
                move_from = *(const int*)payload->Data;
                move_to = n;
            }
            ImGui::EndDragDropTarget();
        }
    }
    if (move_from != -1 && move_to != -1) {
        std::swap(info_order[move_from], info_order[move_to]);
        ImGui::SetDragDropPayload("MENU_ORDER", &move_to, sizeof(int));
    }
    ImGui::End();
}

void Speedrun::draw_stats() {
    if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        settings_open = !settings_open;
        save_config(config);
        API::get()->log_info("Saved Config: %s", config.file_path.data());
    }
    ImGui::GetIO().MouseDrawCursor = settings_open;
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    if (!enabled->value()) {
        return;
    }
    const auto start = std::chrono::high_resolution_clock ::now();
    ImGui::Begin("", nullptr, window_flags(locked->value()));
    for (int i : info_order) {
        switch (i) {
        case 1:
            if (in_game_time->value()) {
                draw_ingame_time();
            }
            continue;
        case 2:
            if (money->value()) {
                draw_money();
            }
            continue;
        case 3:
            if (game_rank->value()) {
                draw_game_rank();
            }
            continue;
        case 4:
            if (kill_count->value()) {
                draw_kills();
            }
            continue;
        case 5:
            if (local_enemies->value()) {
                draw_enemies(num_display->value(), boss_only->value());
            }
            continue;
        default:
            reframework::API::get()->log_info("Done 5");
            if (enabled->value() && !in_game_time->value() && !money->value() && !game_rank->value() && !local_enemies->value()) {
                ImGui::Text("You disabled everything, but left the overlay enabled.");
            }
            continue;
        }
    }

    const auto diff = std::chrono::high_resolution_clock::now() - start;
    frame_time = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(diff).count();
    ImGui::End();
}

void Speedrun::draw_money() {
    const auto inventory_manager = API::get()->get_managed_singleton(game_namespace("InventoryManager"));
    const auto ingame_shop_manager = API::get()->get_managed_singleton(game_namespace("InGameShopManager"));
    const auto ptas = inventory_manager->get_field<uint32_t>("<CurrPTAS>k__BackingField");
    const auto spinels = ingame_shop_manager->get_field<uint32_t>("<CurrSpinelCount>k__BackingField");
    if (ptas != nullptr) {
        ImGui::LabelText("Current Pitas (Yum)", "%u", *ptas);
    }
    if (spinels != nullptr) {
        ImGui::LabelText("Current Spinals", "%u", *spinels);
    }
}

void Speedrun::draw_kills() {
    const auto ctx = API::get()->get_vm_context();
    const auto game_stat_manager = API::get()->get_managed_singleton(game_namespace("GameStatsManager"));
    const auto ongoing_stats = game_stat_manager->get_field<API::ManagedObject*>("<OngoingStats>k__BackingField");
    if (ongoing_stats != nullptr && *ongoing_stats != nullptr) {
        auto kill_stats = (*ongoing_stats)->call<API::ManagedObject*>("get_Kill()", ctx, *ongoing_stats);
        if (kill_stats != nullptr) {
            const auto kills = kill_stats->call<uint32_t>("get_Count()", ctx, kill_stats);
            ImGui::LabelText("Killz", "%i", kills);
        }
    }
}

void Speedrun::draw_ingame_time() {
    const auto game_stat_manager = API::get()->get_managed_singleton(game_namespace("GameStatsManager"));
    const auto nullable = API::get()->tdb()->find_type("System.Nullable`1<System.UInt64>");
    const auto nullable_method = nullable->find_method("GetValueOrDefault");
    const auto ctx = API::get()->get_vm_context();
    if (game_stat_manager != nullptr) {
        const auto total_chapter_time = game_stat_manager->call<API::ManagedObject*>("calcTotalChapterStatsClearTime()", &nullable, ctx, game_stat_manager);
        if (total_chapter_time != nullptr && nullable_method != nullptr) {
            const auto total_time_value = nullable_method->call<uint64_t>(ctx, total_chapter_time);
            if (total_time_value == 0) {
                ImGui::LabelText("Game Time", "00h:00m:00s.000");
            } else {
                const auto m = std::chrono::duration(std::chrono::microseconds(total_time_value));
                const auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(m);
                std::stringstream os{};
                display(os, nanos);
                const auto sv = os.str();
                ImGui::LabelText("Game Time", "%s", sv.data());
            }
        }
    }
}

void Speedrun::draw_game_rank() {
    const auto rank_manager = API::get()->get_managed_singleton(game_namespace("GameRankSystem"));
    if (rank_manager != nullptr) {
        const auto current_rank = rank_manager->get_field<uint32_t>("_GameRank");
        const auto action_point = rank_manager->get_field<float>("_ActionPoint");
        const auto item_point = rank_manager->get_field<float>("_ItemPoint");
        ImGui::LabelText("Current Rank", "%i", *current_rank);
        ImGui::LabelText("Action Point", "%f", *action_point);
        ImGui::LabelText("Item Point", "%f", *item_point);
    }
}

inline auto compare_tuples = [](auto const& t1, auto const& t2) { return std::get<1>(t1) > std::get<1>(t2); };

void Speedrun::draw_enemies(const int num_to_display, const bool boss_only) {
    const auto ctx = API::get()->get_vm_context();
    const auto character_manager = API::get()->get_managed_singleton(game_namespace("CharacterManager"));
    if (character_manager != nullptr) {
        const auto enemy_context_list = character_manager->call<API::ManagedObject*>("get_EnemyContextList()", ctx, character_manager);
        if (enemy_context_list != nullptr) {
            const auto num_enemies = enemy_context_list->call<uint32_t>("get_Count()", ctx, enemy_context_list);
            ImGui::LabelText("Potential Number of Enemies", "%u", num_enemies);
            std::vector<std::tuple<float, float, int>> enemy_list{};
            for (int i = 0; i < num_enemies; i++) {
                const auto enemy_ctx = enemy_context_list->call<API::ManagedObject*>("get_Item", ctx, enemy_context_list, i);
                if (enemy_ctx != nullptr) {
                    const auto kind_id = enemy_ctx->call<uint32_t>("get_KindID()", ctx, enemy_ctx);
                    if (boss_only && !boss_map.contains(kind_id)) {
                        continue;
                    }
                    const auto hp = enemy_ctx->call<API::ManagedObject*>("get_HitPoint()", ctx, enemy_ctx);
                    if (hp != nullptr) {
                        const auto current_hp = hp->call<uint32_t>("get_CurrentHitPoint()", ctx, hp);
                        if (current_hp > 0) {
                            const auto max_hp = hp->call<uint32_t>("get_DefaultHitPoint()", ctx, hp);
                            enemy_list.emplace_back((float)current_hp, (float)max_hp, kind_id);
                        }
                    }
                }
            }
            std::sort(std::begin(enemy_list), std::end(enemy_list), compare_tuples);
            if (num_to_display == 0 || num_to_display > enemy_list.size()) {
                for (auto [current_health, max_health, kind_id] : enemy_list) {
                    make_enemy_health_bar(current_health, max_health, enemy_map.at(kind_id));
                }
            } else {
                for (int i = 0; i < num_to_display; ++i) {
                    auto elem = enemy_list[i];
                    make_enemy_health_bar(std::get<0>(elem), std::get<1>(elem), enemy_map.at(std::get<2>(elem)));
                }
            }
        }
    }
}