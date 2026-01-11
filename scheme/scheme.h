#pragma once

#include <string>
#include "object.h"
#include "scope.h"

class Scheme {
    std::shared_ptr<Scope> scope_;

public:
    Scheme();

    std::string Evaluate(const std::string& expression);

private:
    // std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> obj);

    static std::string ToString(std::shared_ptr<Object> obj);
};
