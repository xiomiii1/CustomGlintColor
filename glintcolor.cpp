#include "glintcolor.hpp"

#include "core/memory/Hooks.hpp"
#include <glintmod/memory/Signatures.hpp>
#include <glintmod/sdk/Memory.hpp>
#include <glintmod/sdk/Offsets.hpp>
#include <glintmod/sdk/Types.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>

namespace {

using Color = glintmod::sdk::Color;

using SetEntityConstantsFn = void (*) (
    void*, void*, const Color*, const void*, const void*, const Color*, const Color*, const Color*, const Color*, const void*, const void*, float, float, float, float);

using SetupActorShaderParametersGlintFn = void (*) (
    void*, void*, void*, const Color*, const Color*, const Color*, const Color*, float, float, float, float, const void*, const void*, float, std::uint8_t, const void*);

using SetupFoilShaderParametersFn = void (*) (
    void*, const Color*, const Color*, const Color*, const void*);

using SetupShaderParametersGlintFn = void (*) (
    void*, const Color*, const Color*, const Color*, const Color*, float, float, float, float, const void*, const void*, float);

GlintColorModule* g_module = nullptr;
SetEntityConstantsFn g_entityOriginal = nullptr;
SetupActorShaderParametersGlintFn g_actorGlintOriginal = nullptr;
SetupFoilShaderParametersFn g_foilOriginal = nullptr;
SetupShaderParametersGlintFn g_uiOriginal = nullptr;

Color makeColor() {
    if (!g_module) return {};
    const std::uint32_t rgb = g_module->rgb();
    const float a = std::clamp(g_module->opacity(), 0.0f, 1.0f);
    return {
        static_cast<float>((rgb >> 16) & 0xFFu) / 255.0f,
        static_cast<float>((rgb >> 8) & 0xFFu) / 255.0f,
        static_cast<float>(rgb & 0xFFu) / 255.0f,
        a
    };
}

bool active() {
    return g_module && g_module->enabled;
}

void setEntityConstantsHook(
    void* entityConstants, void* renderContext, const Color* tileLightColor,
    const void* tileLightColorUV, const void* blockLightColor, const Color* overlay,
    const Color* changeColor, const Color* changeColor2, const Color* glintColor,
    const void* glintUVScale, const void* uvAnim, float uvOffset1, float uvOffset2,
    float uvRot1, float uvRot2) {
    if (!g_entityOriginal) return;
    if (active()) {
        const Color color = makeColor();
        g_entityOriginal(entityConstants, renderContext, tileLightColor, tileLightColorUV,
            blockLightColor, overlay, changeColor, changeColor2, &color,
            glintUVScale, uvAnim, uvOffset1, uvOffset2, uvRot1, uvRot2);
        return;
    }
    g_entityOriginal(entityConstants, renderContext, tileLightColor, tileLightColorUV,
        blockLightColor, overlay, changeColor, changeColor2, glintColor,
        glintUVScale, uvAnim, uvOffset1, uvOffset2, uvRot1, uvRot2);
}

void setupActorShaderParametersGlintHook(
    void* screenContext, void* entityContext, void* actor, const Color* overlay,
    const Color* changeColor, const Color* changeColor2, const Color* glintColor,
    float uvOffset1, float uvOffset2, float uvRot1, float uvRot2,
    const void* glintUVScale, const void* uvAnim, float br,
    std::uint8_t lightEmission, const void* lightEmissionColor) {
    if (!g_actorGlintOriginal) return;
    if (active()) {
        const Color color = makeColor();
        g_actorGlintOriginal(screenContext, entityContext, actor, overlay, changeColor,
            changeColor2, &color, uvOffset1, uvOffset2, uvRot1, uvRot2,
            glintUVScale, uvAnim, br, lightEmission, lightEmissionColor);
        return;
    }
    g_actorGlintOriginal(screenContext, entityContext, actor, overlay, changeColor,
        changeColor2, glintColor, uvOffset1, uvOffset2, uvRot1, uvRot2,
        glintUVScale, uvAnim, br, lightEmission, lightEmissionColor);
}

void setupFoilShaderParametersHook(
    void* screenContext, const Color* overlay, const Color* changeColor,
    const Color* changeColor2, const void* uvScale) {
    if (!g_foilOriginal) return;
    g_foilOriginal(screenContext, overlay, changeColor, changeColor2, uvScale);
    if (!active() || !screenContext) return;

    using namespace glintmod::sdk::offsets;
    const auto constants = glintmod::sdk::field<std::uintptr_t>(
        screenContext, ScreenContext::mActorShaderConstants);
    if (!constants) return;

    const auto glintConstant = glintmod::sdk::field<std::uintptr_t>(
        reinterpret_cast<void*>(constants), ActorShaderConstants::mGlintColor);
    if (!glintConstant) return;

    const auto data = glintmod::sdk::field<std::uintptr_t>(
        reinterpret_cast<void*>(glintConstant), ShaderConstant::mData);
    if (!data) return;

    *reinterpret_cast<Color*>(data) = makeColor();
    glintmod::sdk::field<std::uint8_t>(
        reinterpret_cast<void*>(glintConstant), ShaderConstant::mDirty) = 1;
}

void setupShaderParametersGlintHook(
    void* screenContext, const Color* overlay, const Color* changeColor,
    const Color* changeColor2, const Color* glintColor, float uvOffset1,
    float uvOffset2, float uvRot1, float uvRot2, const void* glintUVScale,
    const void* uvAnim, float br) {
    if (!g_uiOriginal) return;
    if (active()) {
        const Color color = makeColor();
        g_uiOriginal(screenContext, overlay, changeColor, changeColor2, &color,
            uvOffset1, uvOffset2, uvRot1, uvRot2, glintUVScale, uvAnim, br);
        return;
    }
    g_uiOriginal(screenContext, overlay, changeColor, changeColor2, glintColor,
        uvOffset1, uvOffset2, uvRot1, uvRot2, glintUVScale, uvAnim, br);
}

bool parseHexColor(std::string value, std::uint32_t& out) {
    if (!value.empty() && value.front() == '#') value.erase(value.begin());
    if (value.size() != 6) return false;
    for (const unsigned char ch : value) {
        if (!std::isxdigit(ch)) return false;
    }
    try {
        out = static_cast<std::uint32_t>(std::stoul(value, nullptr, 16));
        return true;
    } catch (...) {
        return false;
    }
}

std::string toHexColor(std::uint32_t rgb) {
    const char* digits = "0123456789ABCDEF";
    std::string result = "#RRGGBB";
    result[1] = digits[(rgb >> 20) & 0xFu];
    result[2] = digits[(rgb >> 16) & 0xFu];
    result[3] = digits[(rgb >> 12) & 0xFu];
    result[4] = digits[(rgb >> 8) & 0xFu];
    result[5] = digits[(rgb >> 4) & 0xFu];
    result[6] = digits[rgb & 0xFu];
    return result;
}

} // namespace

GlintColorModule::GlintColorModule()
    : Module("Glint Color", "Customizes the enchantment glint color and opacity.") {
    g_module = this;
}

GlintColorModule::~GlintColorModule() {
    if (g_module == this) g_module = nullptr;
}

void GlintColorModule::onInit() {
    m_entityTarget = reinterpret_cast<void*>(glintmod::memory::resolve(
        glintmod::memory::SignatureId::ActorShaderManagerSetEntityConstants));
    m_actorGlintTarget = reinterpret_cast<void*>(glintmod::memory::resolve(
        glintmod::memory::SignatureId::ActorShaderManagerSetupShaderParametersActorGlint));
    m_foilTarget = reinterpret_cast<void*>(glintmod::memory::resolve(
        glintmod::memory::SignatureId::ActorShaderManagerSetupFoilShaderParameters));
    m_uiTarget = reinterpret_cast<void*>(glintmod::memory::resolve(
        glintmod::memory::SignatureId::ActorShaderManagerSetupShaderParametersGlint));
    installHooks();
}

void GlintColorModule::installHooks() {
    if (!m_entityHooked && m_entityTarget) {
        m_entityHooked = glintmod::hooks::install(
            m_entityTarget, reinterpret_cast<void*>(&setEntityConstantsHook),
            reinterpret_cast<void**>(&g_entityOriginal)) != nullptr;
    }
    if (!m_actorGlintHooked && m_actorGlintTarget) {
        m_actorGlintHooked = glintmod::hooks::install(
            m_actorGlintTarget, reinterpret_cast<void*>(&setupActorShaderParametersGlintHook),
            reinterpret_cast<void**>(&g_actorGlintOriginal)) != nullptr;
    }
    if (!m_foilHooked && m_foilTarget) {
        m_foilHooked = glintmod::hooks::install(
            m_foilTarget, reinterpret_cast<void*>(&setupFoilShaderParametersHook),
            reinterpret_cast<void**>(&g_foilOriginal)) != nullptr;
    }
    if (!m_uiHooked && m_uiTarget) {
        m_uiHooked = glintmod::hooks::install(
            m_uiTarget, reinterpret_cast<void*>(&setupShaderParametersGlintHook),
            reinterpret_cast<void**>(&g_uiOriginal)) != nullptr;
    }
}

void GlintColorModule::onEnable() {
    installHooks();
}

void GlintColorModule::onDisable() {}

void GlintColorModule::loadConfig(const nlohmann::json& j) {
    Module::loadConfig(j);
    if (j.contains("glintColor") && j["glintColor"].is_string()) {
        std::uint32_t parsed = 0;
        if (parseHexColor(j["glintColor"].get<std::string>(), parsed)) {
            m_rgb.store(parsed, std::memory_order_relaxed);
        }
    }
    if (j.contains("glintOpacity") && j["glintOpacity"].is_number()) {
        const float value = std::clamp(j["glintOpacity"].get<float>(), 0.0f, 1.0f);
        m_opacity.store(value, std::memory_order_relaxed);
    }
}

void GlintColorModule::saveConfig(nlohmann::json& j) {
    Module::saveConfig(j);
    j["glintColor"] = toHexColor(m_rgb.load(std::memory_order_relaxed));
    j["glintOpacity"] = m_opacity.load(std::memory_order_relaxed);
}

std::uint32_t GlintColorModule::rgb() const {
    return m_rgb.load(std::memory_order_relaxed);
}

float GlintColorModule::opacity() const {
    return m_opacity.load(std::memory_order_relaxed);
}
