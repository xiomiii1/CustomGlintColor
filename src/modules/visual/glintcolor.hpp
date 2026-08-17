#pragma once

#include "../Module.hpp"
#include <atomic>
#include <cstdint>

class GlintColorModule final : public Module {
private:
    std::atomic<std::uint32_t> m_rgb{0x55FFFFu};
    std::atomic<float> m_opacity{0.80f};

    void* m_entityTarget = nullptr;
    void* m_actorGlintTarget = nullptr;
    void* m_foilTarget = nullptr;
    void* m_uiTarget = nullptr;

    bool m_entityHooked = false;
    bool m_actorGlintHooked = false;
    bool m_foilHooked = false;
    bool m_uiHooked = false;

    void installHooks();

public:
    GlintColorModule();
    ~GlintColorModule() override;

    void onInit() override;
    void onEnable() override;
    void onDisable() override;
    void loadConfig(const nlohmann::json& j) override;
    void saveConfig(nlohmann::json& j) override;

    std::uint32_t rgb() const;
    float opacity() const;
};
