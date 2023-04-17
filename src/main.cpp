#pragma once
#include "Speedrun.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"
#include "rendering/d3d11.hpp"
#include "rendering/d3d12.hpp"
#include <reframework/API.hpp>

Speedrun spdrun{};
HWND g_wnd{};
bool g_initialized{false};

bool initialize_imgui() {
    if (g_initialized) {
        return true;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    const auto renderer_data = reframework::API::get()->param()->renderer_data;

    DXGI_SWAP_CHAIN_DESC swap_desc{};
    auto swapchain = (IDXGISwapChain*)renderer_data->swapchain;
    swapchain->GetDesc(&swap_desc);

    g_wnd = swap_desc.OutputWindow;

    if (!ImGui_ImplWin32_Init(g_wnd)) {
        return false;
    }

    if (renderer_data->renderer_type == REFRAMEWORK_RENDERER_D3D11) {
        if (!g_d3d11.initialize()) {
            return false;
        }
    } else if (renderer_data->renderer_type == REFRAMEWORK_RENDERER_D3D12) {
        if (!g_d3d12.initialize()) {
            return false;
        }
    }

    g_initialized = true;
    return true;
}

void on_present() {
    if (!g_initialized) {
        if (!initialize_imgui()) {
            reframework::API::get()->log_info("Failed to initialize imgui");
            return;
        } else {
            reframework::API::get()->log_info("Initialized imgui");
        }
    }

    const auto renderer_data = reframework::API::get()->param()->renderer_data;

    if (renderer_data->renderer_type == REFRAMEWORK_RENDERER_D3D11) {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (spdrun.settings_open) {
            spdrun.on_draw_ui();
        }
        spdrun.draw_stats();

        ImGui::EndFrame();
        ImGui::Render();

        g_d3d11.render_imgui();
    } else if (renderer_data->renderer_type == REFRAMEWORK_RENDERER_D3D12) {

        auto command_queue = (ID3D12CommandQueue*)renderer_data->command_queue;
        if (command_queue == nullptr) {
            return;
        }

        spdrun.rebuild_fonts();
        spdrun.invalidate_device_objects();

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (spdrun.settings_open) {
            spdrun.on_draw_ui();
        }
        spdrun.draw_stats();

        ImGui::EndFrame();
        ImGui::Render();

        g_d3d12.render_imgui();
    }
}

void on_device_reset() {
    const auto renderer_data = reframework::API::get()->param()->renderer_data;

    if (renderer_data->renderer_type == REFRAMEWORK_RENDERER_D3D11) {
        ImGui_ImplDX11_Shutdown();
        g_d3d11 = {};
    }

    if (renderer_data->renderer_type == REFRAMEWORK_RENDERER_D3D12) {
        ImGui_ImplDX12_Shutdown();
        g_d3d12 = {};
    }

    g_initialized = false;
}

bool on_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
    return !ImGui::GetIO().WantCaptureMouse && !ImGui::GetIO().WantCaptureKeyboard;
}

extern "C" __declspec(dllexport) bool reframework_plugin_initialize(const REFrameworkPluginInitializeParam* param) {
    reframework::API::initialize(param);
    const auto functions = param->functions;
    functions->on_present(on_present);
    functions->on_device_reset(on_device_reset);
    functions->on_message((REFOnMessageCb)on_message);

    return true;
}