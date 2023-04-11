#pragma once

#include "../imgui/imgui.h"
#include "Config.h"
#include <memory>
#include <string>

class IModValue {
public:
    using Ptr = std::unique_ptr<IModValue>;

    virtual ~IModValue() = default;;
    virtual bool draw(std::string_view name) = 0;
    virtual void draw_value(std::string_view name) = 0;
    virtual void config_load(const utility::Config& cfg) = 0;
    virtual void config_save(utility::Config& cfg) = 0;
};

// Convenience classes for imgui
template <typename T>
class ModValue : public IModValue {
public:
    using Ptr = std::unique_ptr<ModValue<T>>;

    static auto create(std::string_view config_name, T default_value = T{}) {
        return std::make_unique<ModValue<T>>(config_name, default_value);
    }

    ModValue(std::string_view config_name, T default_value)
        : m_config_name{ config_name },
        m_value{ default_value },
        m_default_value{ default_value }
    {
    }

    ~ModValue() override = default;;

    void config_load(const utility::Config& cfg) override {
        auto v = cfg.get<T>(m_config_name);

        if (v) {
            m_value = *v;
        }
    };

    void config_save(utility::Config& cfg) override {
        cfg.set<T>(m_config_name, m_value);
    };

    explicit operator T&() {
        return m_value;
    }

    T& value() {
        return m_value;
    }

    [[nodiscard]] const T& default_value() const {
        return m_default_value;
    }

    [[nodiscard]] const auto& get_config_name() const {
        return m_config_name;
    }

protected:
    T m_value{};
    T m_default_value{};
    std::string m_config_name{ "Default_ModValue" };
};

class ModToggle : public ModValue<bool> {
public:
    using Ptr = std::unique_ptr<ModToggle>;

    ModToggle(std::string_view config_name, bool default_value)
        : ModValue<bool>{config_name, default_value} {}

    static auto create(std::string_view config_name, bool default_value = false) { return std::make_unique<ModToggle>(config_name, default_value); }

    bool draw(std::string_view name) override {
        ImGui::PushID(this);
        auto ret = ImGui::Checkbox(name.data(), &m_value);
        ImGui::PopID();

        return ret;
    }

    void draw_value(std::string_view name) override { ImGui::Text("%s: %i", name.data(), m_value); }

    bool toggle() { return m_value = !m_value; }
};

class ModFloat : public ModValue<float> {
public:
    using Ptr = std::unique_ptr<ModFloat>;

    ModFloat(std::string_view config_name, float default_value)
        : ModValue<float>{config_name, default_value} {}

    static auto create(std::string_view config_name, float default_value = 0.0f) { return std::make_unique<ModFloat>(config_name, default_value); }

    bool draw(std::string_view name) override {
        ImGui::PushID(this);
        auto ret = ImGui::InputFloat(name.data(), &m_value, 1.0, 1.0, "%.0f");
        ImGui::PopID();

        return ret;
    }

    void draw_value(std::string_view name) override { ImGui::Text("%s: %f", name.data(), m_value); }
};

class ModInt32 : public ModValue<int32_t> {
public:
    using Ptr = std::unique_ptr<ModInt32>;

    static auto create(std::string_view config_name, uint32_t default_value = 0) { return std::make_unique<ModInt32>(config_name, default_value); }

    explicit ModInt32(std::string_view config_name, uint32_t default_value = 0)
        : ModValue{config_name, static_cast<int>(default_value)} {}

    bool draw(std::string_view name) override {
        ImGui::PushID(this);
        auto ret = ImGui::InputInt(name.data(), &m_value);
        ImGui::PopID();

        return ret;
    }

    void draw_value(std::string_view name) override { ImGui::Text("%s: %i", name.data(), m_value); }
};