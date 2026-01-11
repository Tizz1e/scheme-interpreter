#include "object.h"
#include <memory>

Number::Number(int value) : value_(value) {
}

int Number::GetValue() const {
    return value_;
}

Boolean::Boolean(bool value) : value_(value) {
}

bool Boolean::GetValue() const {
    return value_;
}

Symbol::Symbol(const std::string& name) : name_(name) {
}

Symbol::Symbol(std::string&& name) : name_(std::move(name)) {
}

const std::string& Symbol::GetName() const {
    return name_;
}

Cell::Cell(std::shared_ptr<Object> first, std::shared_ptr<Object> second)
    : first_(first), second_(second) {
}

std::shared_ptr<Object> Cell::GetFirst() const {
    return first_;
}

std::shared_ptr<Object> Cell::GetSecond() const {
    return second_;
}

void Cell::SetFirst(std::shared_ptr<Object> value) {
    first_ = value;
}

void Cell::SetSecond(std::shared_ptr<Object> value) {
    second_ = value;
}
