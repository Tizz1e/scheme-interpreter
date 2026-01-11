#include "scheme.h"

#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include "object.h"
#include "parser.h"
#include "tokenizer.h"
#include "builtin-functions.h"

Scheme::Scheme() {
    scope_ = std::make_shared<Scope>(std::unordered_map<std::string, std::shared_ptr<Object>>{
        {{"quote", std::make_shared<Quote>()},
         {"define", std::make_shared<Define>()},
         {"set!", std::make_shared<Set>()},
         {"boolean?", std::make_shared<IsBoolean>()},
         {"not", std::make_shared<Not>()},
         {"and", std::make_shared<And>()},
         {"or", std::make_shared<Or>()},
         {"number?", std::make_shared<IsNumber>()},
         {"<", std::make_shared<Less>()},
         {"<=", std::make_shared<LessOrEqual>()},
         {">", std::make_shared<Greater>()},
         {">=", std::make_shared<GreaterOrEqual>()},
         {"=", std::make_shared<Equal>()},
         {"+", std::make_shared<Add>()},
         {"-", std::make_shared<Sub>()},
         {"*", std::make_shared<Mul>()},
         {"/", std::make_shared<Div>()},
         {"min", std::make_shared<Min>()},
         {"max", std::make_shared<Max>()},
         {"abs", std::make_shared<Abs>()},
         {"pair?", std::make_shared<IsPair>()},
         {"null?", std::make_shared<IsNull>()},
         {"list?", std::make_shared<IsList>()},
         {"cons", std::make_shared<Cons>()},
         {"car", std::make_shared<Car>()},
         {"cdr", std::make_shared<Cdr>()},
         {"set-car!", std::make_shared<SetCar>()},
         {"set-cdr!", std::make_shared<SetCdr>()},
         {"list", std::make_shared<List>()},
         {"list-ref", std::make_shared<ListRef>()},
         {"list-tail", std::make_shared<ListTail>()},
         {"if", std::make_shared<If>()},
         {"lambda", std::make_shared<Lambda>()},
         {"symbol?", std::make_shared<IsSymbol>()}}});
}

std::string Scheme::Evaluate(const std::string& expression) {
    std::stringstream ss{expression};
    Tokenizer tokenizer(&ss);
    auto expr = Read(&tokenizer);
    auto evaluated = ::Evaluate(expr, scope_);
    return ToString(evaluated);
}

// std::shared_ptr<Object> Scheme::Evaluate(std::shared_ptr<Object> arg) {
//     if (Is<Number>(arg)) {
//         return arg;
//     }
//     throw std::runtime_error("Not implemented");

// }

std::string Scheme::ToString(std::shared_ptr<Object> obj) {
    if (obj == nullptr) {
        return "()";
    }
    if (Is<Number>(obj)) {
        return std::to_string(As<Number>(obj)->GetValue());
    }
    if (Is<Boolean>(obj)) {
        return As<Boolean>(obj)->GetValue() ? "#t" : "#f";
    }
    if (Is<Symbol>(obj)) {
        return As<Symbol>(obj)->GetName();
    }
    std::vector<std::string> inner_strings;
    bool proper = true;
    auto cell = As<Cell>(obj);
    while (cell) {
        inner_strings.push_back(ToString(cell->GetFirst()));
        if (cell->GetSecond() == nullptr) {
            break;
        }
        if (Is<Cell>(cell->GetSecond())) {
            cell = As<Cell>(cell->GetSecond());
        } else {
            proper = false;
            inner_strings.push_back(ToString(cell->GetSecond()));
            break;
        }
    }

    std::string res = "(";
    for (size_t i = 0; i + 1 < inner_strings.size(); ++i) {
        res += inner_strings[i];
        res += ' ';
    }
    if (proper) {
        res += inner_strings.back();
    } else {
        res += ". ";
        res += inner_strings.back();
    }
    res += ')';
    return res;
}
