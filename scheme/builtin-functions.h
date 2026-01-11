#pragma once

#include <memory>
#include "object.h"
#include "scope.h"

std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> obj, std::shared_ptr<Scope> scope);

class Define : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class Quote : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class Set : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class Equal : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class IsBoolean : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class Not : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class And : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class Or : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class IsNumber : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class Less : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class LessOrEqual : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class Greater : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class GreaterOrEqual : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class Add : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class Sub : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class Mul : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class Div : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class Min : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class Max : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class Abs : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class IsPair : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class IsNull : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class IsList : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class Cons : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class Car : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class Cdr : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class SetCar : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class SetCdr : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class List : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class ListRef : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class ListTail : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class If : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class Lambda : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};

class IsSymbol : public Function {
public:
    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override;
};
