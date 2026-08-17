#include "ModuleMenu.hpp"
#include "modules/ModuleRegistry.hpp"
#include "config/ConfigManager.hpp"
#include <pl/ModMenu.hpp>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>

static void onModuleToggle(std::string_view module_id, bool enabled) {
    auto* mod = ModuleRegistry::get().find(module_id);
    if (!mod) return;
    mod->setMasterEnabled(enabled);
    glintmod::config::ConfigManager::get().save();
}

static void onModuleConfigChanged(std::string_view module_id, std::string_view key, std::string_view value) {
    auto* mod = ModuleRegistry::get().find(module_id);
    if (!mod) return;

    nlohmann::json j;
    mod->saveConfig(j);

    const std::string safeValue(value);
    const std::string safeKey(key);
    if (!safeValue.empty()) {
        try {
            if (j.contains(safeKey)) {
                if (j[safeKey].is_boolean()) {
                    if (safeValue == "true") j[safeKey] = true;
                    else if (safeValue == "false") j[safeKey] = false;
                } else if (j[safeKey].is_number_integer()) {
                    char* end = nullptr;
                    const int val = static_cast<int>(std::strtol(safeValue.c_str(), &end, 10));
                    if (end && end != safeValue.c_str()) j[safeKey] = val;
                } else if (j[safeKey].is_number_float()) {
                    char* end = nullptr;
                    const float val = std::strtof(safeValue.c_str(), &end);
                    if (end && end != safeValue.c_str()) j[safeKey] = val;
                } else {
                    j[safeKey] = safeValue;
                }
            } else {
                j[safeKey] = safeValue;
            }
        } catch (...) {
            j[safeKey] = safeValue;
        }
    }

    mod->loadConfig(j);
    glintmod::config::ConfigManager::get().save();
}

void registerModulesWithLauncher() {
    for (auto* mod : ModuleRegistry::get().modules()) {
        if (!mod->showInMenu) continue;

        pl::modmenu::ModuleBuilder builder(mod->moduleId, mod->name);
        builder.description(mod->description)
            .defaultEnabled(mod->masterEnabled)
            .hideInHudEditor(true)
            .onToggle(onModuleToggle)
            .onConfigChanged(onModuleConfigChanged);

        nlohmann::json j;
        mod->saveConfig(j);

        for (auto& [key, value] : j.items()) {
            if (key == "keybind" || key == "keybindActive" || key == "masterEnabled") continue;

            std::string displayName;
            displayName.reserve(key.size() + 4);
            for (std::size_t i = 0; i < key.size(); ++i) {
                const unsigned char ch = static_cast<unsigned char>(key[i]);
                if (i == 0) displayName += static_cast<char>(std::toupper(ch));
                else if (std::isupper(ch)) {
                    displayName += ' ';
                    displayName += static_cast<char>(ch);
                } else {
                    displayName += static_cast<char>(ch);
                }
            }

            pl::modmenu::ConfigType type;
            std::string defaultValue;
            std::string minValue;
            std::string maxValue;

            if (value.is_boolean()) {
                type = pl::modmenu::ConfigType::Toggle;
                defaultValue = value.get<bool>() ? "true" : "false";
            } else if (value.is_number_float()) {
                type = pl::modmenu::ConfigType::SliderFloat;
                defaultValue = std::to_string(value.get<float>());
                minValue = "0.0";
                maxValue = (key.find("Opacity") != std::string::npos || key.find("opacity") != std::string::npos)
                    ? "1.0" : "100.0";
            } else if (value.is_string()) {
                const auto stringValue = value.get<std::string>();
                if (!stringValue.empty() && stringValue[0] == '#') {
                    type = pl::modmenu::ConfigType::Color;
                    defaultValue = stringValue;
                } else {
                    type = pl::modmenu::ConfigType::Text;
                    defaultValue = stringValue;
                }
            } else {
                continue;
            }

            builder.config(key, displayName, type, defaultValue, minValue, maxValue, "");
        }

        (void)builder.registerModule();
    }
}
