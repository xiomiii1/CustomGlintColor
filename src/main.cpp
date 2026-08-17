#include "core/Runtime.hpp"
#include <pl/Mod.hpp>

class CustomGlintColorMod {
public:
    static CustomGlintColorMod& instance() {
        static CustomGlintColorMod mod;
        return mod;
    }

    bool load(pl::mod::ModContext& context) { return glintmod::core::Runtime::get().load(context); }
    bool enable(pl::mod::ModContext& context) { return glintmod::core::Runtime::get().enable(context); }
    bool disable(pl::mod::ModContext& context) { return glintmod::core::Runtime::get().disable(context); }
    bool unload(pl::mod::ModContext& context) { return glintmod::core::Runtime::get().unload(context); }
};

PL_REGISTER_MOD(CustomGlintColorMod, CustomGlintColorMod::instance())
