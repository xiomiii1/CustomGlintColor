#pragma once
#include <cstdint>
#include <string_view>
namespace glintmod::memory {
enum class SignatureId : std::size_t {
    ActorShaderManagerSetEntityConstants,
    ActorShaderManagerSetupShaderParametersActorGlint,
    ActorShaderManagerSetupFoilShaderParameters,
    ActorShaderManagerSetupShaderParametersGlint,
    Count
};
inline constexpr std::size_t SignatureCount = static_cast<std::size_t>(SignatureId::Count);
struct SignatureDefinition { SignatureId id; std::string_view pattern; };
bool resolveAll(std::string_view libraryName);
std::uintptr_t resolve(SignatureId id);
void clear();
}
