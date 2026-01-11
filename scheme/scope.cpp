#include "scope.h"
#include "error.h"
#include "object.h"

std::shared_ptr<Object> Scope::Get(const std::string& key) const {
    if (auto it = mapping_.find(key); it != mapping_.end()) {
        return it->second;
    }
    if (parent_ != nullptr) {
        return parent_->Get(key);
    }
    throw NameError(key);
}

// void Scope::Define(const std::string& key, std::shared_ptr<Object> value) {
//     mapping_[key] = value;
// }

void Scope::Set(const std::string& key, std::shared_ptr<Object> value) {
    // if (mapping_.contains(key)) {
    //     mapping_[key] = value;
    // }
    // if (parent_) {
    //     parent_->Set(key, value);
    // } else {
    //     throw RuntimeError("No such key: " + key);
    // }
    mapping_[key] = value;
}
