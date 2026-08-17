#pragma once

#include <nlohmann/json.hpp>
#include <string>

class Module {
public:
    const char* name;
    const char* description;
    std::string moduleId;
    bool masterEnabled = false;
    bool keybindActive = true;
    bool enabled = false;
    bool showInMenu = true;
    bool hideInHudEditor = true;
    int keybind = 0;

    Module(const char* n, const char* d)
        : name(n), description(d), moduleId(std::string("customglintcolor.") + n) {}
    virtual ~Module() = default;

    virtual void onInit() {}
    virtual void onEnable() {}
    virtual void onDisable() {}

    virtual void onKeybindEvent(const std::string& key, bool isDown) {
        if (key == "keybind" && isDown) setKeybindActive(!keybindActive);
    }

    void setMasterEnabled(bool state) {
        if (masterEnabled == state) return;
        masterEnabled = state;
        updateEnabledState();
    }

    void setKeybindActive(bool state) {
        if (keybindActive == state) return;
        keybindActive = state;
        updateEnabledState();
    }

    void updateEnabledState() {
        const bool newState = masterEnabled && keybindActive;
        if (newState == enabled) return;
        enabled = newState;
        if (enabled) onEnable();
        else onDisable();
    }

    virtual void loadConfig(const nlohmann::json& j) {
        if (j.contains("keybind") && j["keybind"].is_number_integer()) keybind = j["keybind"].get<int>();
        if (j.contains("keybindActive") && j["keybindActive"].is_boolean()) {
            keybindActive = j["keybindActive"].get<bool>();
        }
        if (j.contains("masterEnabled") && j["masterEnabled"].is_boolean()) {
            masterEnabled = j["masterEnabled"].get<bool>();
        }
        updateEnabledState();
    }

    virtual void saveConfig(nlohmann::json& j) {
        j["keybind"] = keybind;
        j["keybindActive"] = keybindActive;
        j["masterEnabled"] = masterEnabled;
    }
};
