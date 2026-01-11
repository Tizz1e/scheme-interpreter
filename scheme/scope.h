#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "error.h"
#include "object.h"

class Scope {
public:
    Scope() = default;
    Scope(const std::unordered_map<std::string, std::shared_ptr<Object>>& mapping)
        : mapping_(mapping) {
    }

    Scope(std::unordered_map<std::string, std::shared_ptr<Object>>&& mapping)
        : mapping_(std::move(mapping)) {
    }

    Scope(std::shared_ptr<Scope> parent) : parent_(parent) {
    }

    std::shared_ptr<Object> Get(const std::string& key) const;
    void Set(const std::string& key, std::shared_ptr<Object> value);

private:
    std::shared_ptr<Scope> parent_;
    std::unordered_map<std::string, std::shared_ptr<Object>> mapping_;
};
