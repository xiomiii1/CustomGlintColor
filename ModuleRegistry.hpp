#pragma once

#include "Module.hpp"
#include <memory>
#include <utility>
#include <string_view>
#include <unordered_map>
#include <vector>

class ModuleRegistry {
private:
    std::vector<std::unique_ptr<Module>> mStorage;
    std::vector<Module*> mView;
    std::unordered_map<std::string_view, Module*> mById;
    bool mInitialized = false;

public:
    static ModuleRegistry& get();

    template <typename T, typename... Args>
    T& emplace(Args&&... args) {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        auto* raw = ptr.get();
        mById.emplace(raw->moduleId, raw);
        mView.push_back(raw);
        mStorage.push_back(std::move(ptr));
        return *raw;
    }

    Module* find(std::string_view id) const;
    const std::vector<Module*>& modules() const;
    void initialize();
};

void registerAllModules();
