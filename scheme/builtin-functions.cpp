#include "builtin-functions.h"
#include <memory>
#include "error.h"
#include "object.h"

std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> obj, std::shared_ptr<Scope> scope) {
    if (Is<Number>(obj) || Is<Boolean>(obj)) {
        return obj;
    }
    if (Is<Symbol>(obj)) {
        return scope->Get(As<Symbol>(obj)->GetName());
    }
    if (obj == nullptr || !Is<Cell>(obj)) {
        throw RuntimeError("can't evaluate list");
    }
    std::shared_ptr<Cell> func_list = As<Cell>(obj);
    auto func_expr = func_list->GetFirst();
    auto args = func_list->GetSecond();

    auto functor = Evaluate(func_expr, scope);
    if (!Is<Function>(functor)) {
        throw RuntimeError("Expected function applying");
    }
    return (*As<Function>(functor))(args, scope);
}

namespace {
std::vector<std::shared_ptr<Object>> CellToVector(std::shared_ptr<Object> arg, bool proper = true) {
    if (arg == nullptr) {
        return {};
    }
    if (!Is<Cell>(arg)) {
        if (proper) {
            throw SyntaxError("Expected proper list");
        }
        return {arg};
    }
    auto cell = As<Cell>(arg);
    std::vector<std::shared_ptr<Object>> result;
    while (cell) {
        result.push_back(cell->GetFirst());
        auto second = cell->GetSecond();
        if (second == nullptr) {
            break;
        }
        if (Is<Cell>(second)) {
            cell = As<Cell>(second);
        } else {
            if (proper) {
                throw SyntaxError("Expected proper list");
            }
            result.push_back(second);
            break;
        }
    }
    return result;
}

std::vector<std::shared_ptr<Object>> Evaluate(const std::vector<std::shared_ptr<Object>>& objs,
                                              std::shared_ptr<Scope> scope) {
    std::vector<std::shared_ptr<Object>> result;
    for (auto obj : objs) {
        result.push_back(Evaluate(obj, scope));
    }
    return result;
}

void CheckNonEmpty(const std::vector<std::shared_ptr<Object>>& args, const std::string& name) {
    if (args.empty()) {
        throw RuntimeError("\"" + name + "\" must have arguments");
    }
}

void CheckAllNumbers(const std::vector<std::shared_ptr<Object>>& args, const std::string& name) {
    for (auto arg : args) {
        if (!Is<Number>(arg)) {
            throw RuntimeError("\"" + name + "\" arguments must be numbers");
        }
    }
}
}  // namespace

std::shared_ptr<Object> Define::operator()(std::shared_ptr<Object> args,
                                           std::shared_ptr<Scope> scope) {
    auto flatten_args = CellToVector(args);
    if (flatten_args.empty()) {
        throw SyntaxError("\"define\" takes 2 arguments");
    }
    if (Is<Symbol>(flatten_args[0])) {
        if (flatten_args.size() != 2) {
            throw SyntaxError("\"define\" takes 2 arguments");
        }
        scope->Set(As<Symbol>(flatten_args[0])->GetName(), Evaluate(flatten_args[1], scope));
        return flatten_args[0];
    }
    if (Is<Cell>(flatten_args[0])) {
        if (flatten_args.size() < 2) {
            throw SyntaxError("\"define\" with lambda-sugar takes at least 2 arguments");
        }
        auto flatten_func = CellToVector(flatten_args[0]);
        if (!Is<Symbol>(flatten_func[0])) {
            throw SyntaxError("\"define\" 1st argument must be symbol or list");
        }
        auto lmbd = Lambda()(std::make_shared<Cell>(As<Cell>(flatten_args[0])->GetSecond(),
                                                    As<Cell>(args)->GetSecond()),
                             scope);
        scope->Set(As<Symbol>(flatten_func[0])->GetName(), lmbd);
        return flatten_func[0];
    }
    throw SyntaxError("\"define\" 1st argument must be symbol or list");
}

std::shared_ptr<Object> Quote::operator()(std::shared_ptr<Object> args,
                                          std::shared_ptr<Scope> scope) {
    if (!Is<Cell>(args) || As<Cell>(args)->GetSecond() != nullptr) {
        throw RuntimeError("Wrong structure for quote function");
    }
    // std::cout << Scheme::ToString(As<Cell>(args)->GetFirst()) << std::endl;
    return As<Cell>(args)->GetFirst();
}

std::shared_ptr<Object> Set::operator()(std::shared_ptr<Object> args,
                                        std::shared_ptr<Scope> scope) {
    auto flatten_args = CellToVector(args);
    if (flatten_args.size() != 2) {
        throw SyntaxError("\"set!\" takes 2 arguments");
    }
    if (!Is<Symbol>(flatten_args[0])) {
        throw SyntaxError("\"set!\" 1st argument must be symbol");
    }
    const std::string& name = As<Symbol>(flatten_args[0])->GetName();
    scope->Get(name);
    scope->Set(name, Evaluate(flatten_args[1], scope));
    return nullptr;
}

std::shared_ptr<Object> IsBoolean::operator()(std::shared_ptr<Object> args,
                                              std::shared_ptr<Scope> scope) {
    auto flatten_args = CellToVector(args);
    if (flatten_args.size() != 1) {
        throw RuntimeError("bool? expects one argument");
    }
    if (Is<Boolean>(Evaluate(flatten_args.front(), scope))) {
        return kTrue;
    }
    return kFalse;
}

std::shared_ptr<Object> Not::operator()(std::shared_ptr<Object> args,
                                        std::shared_ptr<Scope> scope) {
    auto flatten_args = CellToVector(args);
    if (flatten_args.size() != 1) {
        throw RuntimeError("\"not\" expects 1 argument");
    }
    auto evaluated = Evaluate(flatten_args.front(), scope);
    if (Is<Boolean>(evaluated) && As<Boolean>(evaluated)->GetValue() == false) {
        return kTrue;
    }
    return kFalse;
}

std::shared_ptr<Object> And::operator()(std::shared_ptr<Object> args,
                                        std::shared_ptr<Scope> scope) {
    auto flatten_args = CellToVector(args);
    std::shared_ptr<Object> evaluated = kTrue;
    for (auto arg : flatten_args) {
        evaluated = Evaluate(arg, scope);
        if (Is<Boolean>(evaluated) && As<Boolean>(evaluated)->GetValue() == false) {
            return kFalse;
        }
    }
    return evaluated;
}

std::shared_ptr<Object> Or::operator()(std::shared_ptr<Object> args, std::shared_ptr<Scope> scope) {
    auto flatten_args = CellToVector(args);
    for (auto arg : flatten_args) {
        auto evaluated = Evaluate(arg, scope);
        if (!Is<Boolean>(evaluated) || As<Boolean>(evaluated)->GetValue() == true) {
            return evaluated;
        }
    }
    return kFalse;
}

std::shared_ptr<Object> Add::operator()(std::shared_ptr<Object> args,
                                        std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args), scope);
    CheckAllNumbers(evaluated, "+");
    int result = 0;
    for (auto arg : evaluated) {
        result += As<Number>(arg)->GetValue();
    }
    return std::make_shared<Number>(result);
}

std::shared_ptr<Object> Sub::operator()(std::shared_ptr<Object> args,
                                        std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args), scope);
    if (evaluated.empty()) {
        throw RuntimeError("\"-\" must have argument");
    }
    CheckAllNumbers(evaluated, "-");
    int result = As<Number>(evaluated[0])->GetValue();
    if (evaluated.size() == 1) {
        return std::make_shared<Number>(-result);
    }
    for (size_t i = 1; i < evaluated.size(); ++i) {
        result -= As<Number>(evaluated[i])->GetValue();
    }
    return std::make_shared<Number>(result);
}

std::shared_ptr<Object> Mul::operator()(std::shared_ptr<Object> args,
                                        std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args), scope);
    CheckAllNumbers(evaluated, "*");
    int result = 1;
    for (auto arg : evaluated) {
        result *= As<Number>(arg)->GetValue();
    }
    return std::make_shared<Number>(result);
}

std::shared_ptr<Object> Div::operator()(std::shared_ptr<Object> args,
                                        std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args), scope);
    if (evaluated.empty()) {
        throw RuntimeError("\"/\" must have argument");
    }
    CheckAllNumbers(evaluated, "/");
    int result = As<Number>(evaluated[0])->GetValue();
    if (evaluated.size() == 1) {
        return std::make_shared<Number>(result == 1 ? 1 : 0);
    }
    for (size_t i = 1; i < evaluated.size(); ++i) {
        result /= As<Number>(evaluated[i])->GetValue();
    }
    return std::make_shared<Number>(result);
}

std::shared_ptr<Object> Less::operator()(std::shared_ptr<Object> args,
                                         std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args), scope);
    if (evaluated.empty()) {
        return kTrue;
    }
    CheckAllNumbers(evaluated, "<");
    int first = As<Number>(evaluated[0])->GetValue();
    for (size_t i = 1; i < evaluated.size(); ++i) {
        int next = As<Number>(evaluated[i])->GetValue();
        if (first >= next) {
            return kFalse;
        }
        first = next;
    }
    return kTrue;
}

std::shared_ptr<Object> LessOrEqual::operator()(std::shared_ptr<Object> args,
                                                std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args), scope);
    if (evaluated.empty()) {
        return kTrue;
    }
    CheckAllNumbers(evaluated, "<=");
    int first = As<Number>(evaluated[0])->GetValue();
    for (size_t i = 1; i < evaluated.size(); ++i) {
        int next = As<Number>(evaluated[i])->GetValue();
        if (first > next) {
            return kFalse;
        }
        first = next;
    }
    return kTrue;
}

std::shared_ptr<Object> Greater::operator()(std::shared_ptr<Object> args,
                                            std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args), scope);
    if (evaluated.empty()) {
        return kTrue;
    }
    CheckAllNumbers(evaluated, ">");
    int first = As<Number>(evaluated[0])->GetValue();
    for (size_t i = 1; i < evaluated.size(); ++i) {
        int next = As<Number>(evaluated[i])->GetValue();
        if (first <= next) {
            return kFalse;
        }
        first = next;
    }
    return kTrue;
}

std::shared_ptr<Object> GreaterOrEqual::operator()(std::shared_ptr<Object> args,
                                                   std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args), scope);
    if (evaluated.empty()) {
        return kTrue;
    }
    CheckAllNumbers(evaluated, ">=");
    int first = As<Number>(evaluated[0])->GetValue();
    for (size_t i = 1; i < evaluated.size(); ++i) {
        int next = As<Number>(evaluated[i])->GetValue();
        if (first < next) {
            return kFalse;
        }
        first = next;
    }
    return kTrue;
}

std::shared_ptr<Object> Equal::operator()(std::shared_ptr<Object> args,
                                          std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args), scope);
    if (evaluated.empty()) {
        return kTrue;
    }
    CheckAllNumbers(evaluated, "=");
    int first = As<Number>(evaluated[0])->GetValue();
    for (size_t i = 1; i < evaluated.size(); ++i) {
        int next = As<Number>(evaluated[i])->GetValue();
        if (first != next) {
            return kFalse;
        }
    }
    return kTrue;
}

std::shared_ptr<Object> IsNumber::operator()(std::shared_ptr<Object> args,
                                             std::shared_ptr<Scope> scope) {
    auto flatten_args = CellToVector(args);
    if (flatten_args.size() != 1) {
        throw RuntimeError("\"number?\" expects one argument");
    }
    if (Is<Number>(Evaluate(flatten_args.front(), scope))) {
        return kTrue;
    }
    return kFalse;
}

std::shared_ptr<Object> Min::operator()(std::shared_ptr<Object> args,
                                        std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args), scope);
    CheckNonEmpty(evaluated, "min");
    CheckAllNumbers(evaluated, "min");
    int result = As<Number>(evaluated[0])->GetValue();
    for (auto arg : evaluated) {
        result = std::min(result, As<Number>(arg)->GetValue());
    }
    return std::make_shared<Number>(result);
}

std::shared_ptr<Object> Max::operator()(std::shared_ptr<Object> args,
                                        std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args), scope);
    CheckNonEmpty(evaluated, "max");
    CheckAllNumbers(evaluated, "max");
    int result = As<Number>(evaluated[0])->GetValue();
    for (auto arg : evaluated) {
        result = std::max(result, As<Number>(arg)->GetValue());
    }
    return std::make_shared<Number>(result);
}

std::shared_ptr<Object> Abs::operator()(std::shared_ptr<Object> args,
                                        std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args), scope);
    if (evaluated.size() != 1) {
        throw RuntimeError("\"abs\" must have 1 argument");
    }
    CheckAllNumbers(evaluated, "min");
    return std::make_shared<Number>(abs(As<Number>(evaluated[0])->GetValue()));
}

std::shared_ptr<Object> IsPair::operator()(std::shared_ptr<Object> args,
                                           std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args), scope);
    if (evaluated.size() != 1) {
        throw RuntimeError("\"pair?\" must have 1 argument");
    }
    if (evaluated[0] == nullptr || !Is<Cell>(evaluated[0])) {
        return kFalse;
    }
    return kTrue;
}

std::shared_ptr<Object> IsNull::operator()(std::shared_ptr<Object> args,
                                           std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args), scope);
    if (evaluated.size() != 1) {
        throw RuntimeError("\"null?\" must have 1 argument");
    }
    if (evaluated[0] == nullptr) {
        return kTrue;
    }
    return kFalse;
}

std::shared_ptr<Object> IsList::operator()(std::shared_ptr<Object> args,
                                           std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args, false), scope);
    if (evaluated.size() != 1) {
        throw RuntimeError("\"list?\" must have 1 argument");
    }
    if (evaluated[0] != nullptr && !Is<Cell>(evaluated[0])) {
        return kFalse;
    }
    try {
        CellToVector(evaluated[0], true);
    } catch (...) {
        return kFalse;
    }

    return kTrue;
}

std::shared_ptr<Object> Cons::operator()(std::shared_ptr<Object> args,
                                         std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args, false), scope);
    if (evaluated.size() != 2) {
        throw RuntimeError("\"cons\" must have 2 argument");
    }

    return std::make_shared<Cell>(evaluated[0], evaluated[1]);
}

std::shared_ptr<Object> Car::operator()(std::shared_ptr<Object> args,
                                        std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args, false), scope);
    if (evaluated.size() != 1) {
        throw RuntimeError("\"car\" must have 1 argument");
    }
    if (!Is<Cell>(evaluated[0])) {
        throw RuntimeError("\"car\" argument must be pair-like structure");
    }
    if (evaluated[0] == nullptr) {
        throw RuntimeError("\"cdr\" on nil");
    }
    auto head = As<Cell>(evaluated[0])->GetFirst();
    return head;
}

std::shared_ptr<Object> Cdr::operator()(std::shared_ptr<Object> args,
                                        std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args, false), scope);
    if (evaluated.size() != 1) {
        throw RuntimeError("\"cdr\" must have 1 argument");
    }
    if (!Is<Cell>(evaluated[0])) {
        throw RuntimeError("\"cdr\" argument must be pair-like structure");
    }
    if (evaluated[0] == nullptr) {
        throw RuntimeError("\"cdr\" on nil");
    }
    auto tail = As<Cell>(evaluated[0])->GetSecond();
    return tail;
}

std::shared_ptr<Object> SetCar::operator()(std::shared_ptr<Object> args,
                                           std::shared_ptr<Scope> scope) {
    auto flatten_args = CellToVector(args);
    if (flatten_args.size() != 2) {
        throw RuntimeError("\"set-car!\" must have 2 arguments");
    }
    if (!Is<Symbol>(flatten_args[0])) {
        throw RuntimeError("\"set-car!\" 1st argument must be symbol");
    }
    auto evaluated = Evaluate(flatten_args[1], scope);
    auto val = scope->Get(As<Symbol>(flatten_args[0])->GetName());
    if (!Is<Cell>(val)) {
        throw RuntimeError("\"set-car!\" argument is not list");
    }
    if (val == nullptr) {
        throw RuntimeError("\"set-car!\" argument can't be nil");
    }
    scope->Set(As<Symbol>(flatten_args[0])->GetName(),
               std::make_shared<Cell>(evaluated, As<Cell>(val)->GetSecond()));
    return nullptr;
}

std::shared_ptr<Object> SetCdr::operator()(std::shared_ptr<Object> args,
                                           std::shared_ptr<Scope> scope) {
    auto flatten_args = CellToVector(args);
    if (flatten_args.size() != 2) {
        throw RuntimeError("\"set-cdr!\" must have 2 arguments");
    }
    if (!Is<Symbol>(flatten_args[0])) {
        throw RuntimeError("\"set-cdr!\" 1st argument must be symbol");
    }
    auto evaluated = Evaluate(flatten_args[1], scope);
    auto val = scope->Get(As<Symbol>(flatten_args[0])->GetName());
    if (!Is<Cell>(val)) {
        throw RuntimeError("\"set-cdr!\" argument is not list");
    }
    if (val == nullptr) {
        throw RuntimeError("\"set-cdr!\" argument can't be nil");
    }
    scope->Set(As<Symbol>(flatten_args[0])->GetName(),
               std::make_shared<Cell>(As<Cell>(val)->GetFirst(), evaluated));
    return nullptr;
}

std::shared_ptr<Object> List::operator()(std::shared_ptr<Object> args,
                                         std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args), scope);
    std::shared_ptr<Cell> result = nullptr;
    size_t last_idx = evaluated.size();
    while (last_idx) {
        --last_idx;
        result = std::make_shared<Cell>(evaluated[last_idx], result);
    }
    return result;
}

std::shared_ptr<Object> ListRef::operator()(std::shared_ptr<Object> args,
                                            std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args), scope);
    if (evaluated.size() != 2) {
        throw RuntimeError("\"list-ref\" must have 2 args");
    }
    if (!Is<Number>(evaluated[1])) {
        throw RuntimeError("\"list-ref\" 2nd argument must be number");
    }
    if (evaluated[0] == nullptr || !Is<Cell>(evaluated[0])) {
        throw RuntimeError("\"list-ref\" 1st argument must be list");
    }
    int idx = As<Number>(evaluated[1])->GetValue();
    auto elements = CellToVector(evaluated[0]);
    if (elements.size() <= idx) {
        throw RuntimeError("\"list-ref\": index out of range");
    }
    return elements[idx];
}

std::shared_ptr<Object> ListTail::operator()(std::shared_ptr<Object> args,
                                             std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args), scope);
    if (evaluated.size() != 2) {
        throw RuntimeError("\"list-ref\" must have 2 args");
    }
    if (!Is<Number>(evaluated[1])) {
        throw RuntimeError("\"list-ref\" 2nd argument must be number");
    }
    if (evaluated[0] == nullptr || !Is<Cell>(evaluated[0])) {
        throw RuntimeError("\"list-ref\" 1st argument must be list");
    }
    int idx = As<Number>(evaluated[1])->GetValue();
    auto cell = As<Cell>(evaluated[0]);
    for (int i = 0; i < idx; ++i) {
        if (cell == nullptr) {
            throw RuntimeError("\"list-tail\": index out of range");
        }
        if (!Is<Cell>(cell->GetSecond())) {
            if (i + 1 == idx) {
                return cell->GetSecond();
            }
            throw RuntimeError("\"list-tail\": index out of range");
        }
        cell = As<Cell>(cell->GetSecond());
    }
    return cell ? cell : nullptr;
}

std::shared_ptr<Object> If::operator()(std::shared_ptr<Object> args, std::shared_ptr<Scope> scope) {
    auto flatten_args = CellToVector(args);
    if (flatten_args.empty() || flatten_args.size() > 3) {
        throw SyntaxError("\"if\" must have at least 1 argument and at most 3 arguments");
    }
    auto predicate = Evaluate(flatten_args[0], scope);
    if (Is<Boolean>(predicate) && As<Boolean>(predicate)->GetValue() == false) {
        return flatten_args.size() < 3 ? nullptr : Evaluate(flatten_args[2], scope);
    } else {
        return flatten_args.size() < 2 ? nullptr : Evaluate(flatten_args[1], scope);
    }
}

class LambdaHelper : public Function {
    std::vector<std::string> arg_names_;
    std::vector<std::shared_ptr<Object>> evaluation_;
    std::shared_ptr<Scope> scope_;

public:
    LambdaHelper(std::vector<std::string> arg_names, std::vector<std::shared_ptr<Object>> eval,
                 std::shared_ptr<Scope> lambda_scope)
        : arg_names_(arg_names), evaluation_(eval), scope_(lambda_scope) {
    }

    std::shared_ptr<Object> operator()(std::shared_ptr<Object> args,
                                       std::shared_ptr<Scope> scope) override {
        auto evaluated = Evaluate(CellToVector(args), scope);
        if (evaluated.size() != arg_names_.size()) {
            throw RuntimeError("\"lambda\": not equal amount of arguments");
        }
        for (size_t i = 0; i < arg_names_.size(); ++i) {
            scope_->Set(arg_names_[i], evaluated[i]);
        }
        std::shared_ptr<Object> last_eval;
        for (auto e : evaluation_) {
            last_eval = Evaluate(e, scope_);
        }
        return last_eval;
    }
};

std::shared_ptr<Object> Lambda::operator()(std::shared_ptr<Object> args,
                                           std::shared_ptr<Scope> scope) {
    auto flatten_args = CellToVector(args);
    if (flatten_args.size() < 2) {
        throw SyntaxError("\"lambda\" must have at least 2 arguments");
    }

    std::vector<std::string> names;

    for (auto name : CellToVector(flatten_args[0])) {
        if (!Is<Symbol>(name)) {
            throw SyntaxError("lambda args names must be symbols");
        }
        names.push_back(As<Symbol>(name)->GetName());
    }

    flatten_args.erase(flatten_args.begin());

    return std::make_shared<LambdaHelper>(names, flatten_args, std::make_shared<Scope>(scope));
}

std::shared_ptr<Object> IsSymbol::operator()(std::shared_ptr<Object> args,
                                             std::shared_ptr<Scope> scope) {
    auto evaluated = Evaluate(CellToVector(args), scope);
    if (evaluated.size() != 1) {
        throw RuntimeError("\"symbol?\" mush have 1 argument");
    }
    if (Is<Symbol>(evaluated[0])) {
        return kTrue;
    }
    return kFalse;
}
