#include "error.h"
#include "object.h"
#include "parser.h"
#include "tokenizer.h"

#include <memory>
#include <vector>
#include <variant>

namespace {
template <typename... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};

std::shared_ptr<Object> ReadImpl(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        return nullptr;
    }
    Token token = tokenizer->GetToken();
    auto visitor = Overloaded{
        [&](const ConstantToken& token) {
            tokenizer->Next();
            return std::make_shared<Number>(token.value);
        },
        [&](const SymbolToken& token) -> std::shared_ptr<Object> {
            tokenizer->Next();
            if (token.name == "#t") {
                return std::make_shared<Boolean>(true);
            }
            if (token.name == "#f") {
                return std::make_shared<Boolean>(false);
            }
            return std::make_shared<Symbol>(token.name);
        },
        [&](const QuoteToken& token) {
            tokenizer->Next();
            auto quote_arg = ReadImpl(tokenizer);
            auto wrapped_quote_arg = std::make_shared<Cell>(quote_arg, nullptr);
            return std::make_shared<Cell>(std::make_shared<Symbol>("quote"), wrapped_quote_arg);
        },
        [&](const BracketToken& token) {
            if (token == BracketToken::OPEN) {
                return ReadList(tokenizer);
            }
            throw SyntaxError("Read: not matching closing bracket");
        },
        [](const auto&) -> std::shared_ptr<Object> {
            throw SyntaxError("Read: unexpected token");
        }};
    return std::visit<std::shared_ptr<Object>>(visitor, token);
    ;
}
}  // namespace

std::shared_ptr<Object> Read(Tokenizer* tokenizer) {
    auto result = ReadImpl(tokenizer);
    if (!tokenizer->IsEnd()) {
        throw SyntaxError("expect end of programm");
    }
    return result;
}

std::shared_ptr<Object> ReadList(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("expected opening bracket");
    }

    auto check_bracket = [&](BracketToken expected_bracket) -> bool {
        const auto token = tokenizer->GetToken();
        auto bracket_token = std::get_if<BracketToken>(&token);
        return bracket_token && *bracket_token == expected_bracket;
    };

    if (!check_bracket(BracketToken::OPEN)) {
        throw SyntaxError("expected opening bracket");
    }

    tokenizer->Next();

    bool read_next = true;
    bool is_proper = true;

    std::vector<std::shared_ptr<Object>> objects;

    while (!tokenizer->IsEnd() && read_next) {
        auto visitor = Overloaded{[&](const BracketToken& token) {
                                      if (token == BracketToken::CLOSE) {
                                          read_next = false;
                                      }
                                  },
                                  [&](const DotToken& token) {
                                      if (objects.empty()) {
                                          throw SyntaxError("expect object before .");
                                      }
                                      tokenizer->Next();
                                      objects.push_back(ReadImpl(tokenizer));
                                      is_proper = false;
                                      read_next = false;
                                  },
                                  [](const auto&) {}};
        std::visit(visitor, tokenizer->GetToken());
        if (read_next) {
            objects.push_back(ReadImpl(tokenizer));
        }
    }

    if (tokenizer->IsEnd() || !check_bracket(BracketToken::CLOSE)) {
        throw SyntaxError("expect closing bracket");
    }

    tokenizer->Next();

    if (objects.empty()) {
        return nullptr;
    }

    std::shared_ptr<Object> result = is_proper ? nullptr : objects.back();
    size_t next_index = is_proper ? objects.size() - 1 : objects.size() - 2;

    while (true) {
        result = std::make_shared<Cell>(objects[next_index], result);
        if (next_index-- == 0) {
            break;
        }
    }

    return result;
}
