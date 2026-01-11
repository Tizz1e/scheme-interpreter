#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

class Scope;

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;
};

class Number : public Object {
private:
    int value_;

public:
    Number(int value);

    int GetValue() const;
};

class Boolean : public Object {
private:
    int value_;

public:
    Boolean(bool value);

    bool GetValue() const;
};

class Symbol : public Object {
private:
    std::string name_;

public:
    Symbol(const std::string& name);
    Symbol(std::string&& name);

    const std::string& GetName() const;
};

class Cell : public Object {
private:
    std::shared_ptr<Object> first_;
    std::shared_ptr<Object> second_;

public:
    Cell(std::shared_ptr<Object> first, std::shared_ptr<Object> second);

    std::shared_ptr<Object> GetFirst() const;
    std::shared_ptr<Object> GetSecond() const;

    void SetFirst(std::shared_ptr<Object> value);
    void SetSecond(std::shared_ptr<Object> value);
};

class Function : public Object {
public:
    virtual std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                               std::shared_ptr<Scope> scope) = 0;
};

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj) {
    auto result = std::dynamic_pointer_cast<T>(obj);
    if (!result) {
        throw std::runtime_error("As: downcasting to wrong type");
    }
    return result;
}

template <class T>
bool Is(const std::shared_ptr<Object>& obj) {
    return static_cast<bool>(std::dynamic_pointer_cast<T>(obj));
}

inline const std::shared_ptr<Boolean> kTrue = std::make_shared<Boolean>(true);
inline const std::shared_ptr<Boolean> kFalse = std::make_shared<Boolean>(false);
