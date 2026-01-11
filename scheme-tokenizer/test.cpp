#include "tokenizer.h"

#include <sstream>
#include <string>
#include <variant>

#include <catch2/catch_test_macros.hpp>

namespace {

template <class T>
void RequireEquals(const Token& token, const T& value) {
    if (const auto* p = std::get_if<T>(&token)) {
        REQUIRE(*p == value);
    } else {
        FAIL("Wrong token type: " << typeid(T).name());
    }
}

void CheckTokens(std::string str, const auto&... tokens) {
    std::stringstream ss{std::move(str)};
    Tokenizer tokenizer{&ss};
    [[maybe_unused]] auto check_token = [&tokenizer](const auto& token) {
        REQUIRE_FALSE(tokenizer.IsEnd());
        RequireEquals(tokenizer.GetToken(), token);
        tokenizer.Next();
    };
    (check_token(tokens), ...);
    REQUIRE(tokenizer.IsEnd());
}

}  // namespace

TEST_CASE("Tokenizer works on simple case") {
    CheckTokens("2", ConstantToken{2});
    CheckTokens("34", ConstantToken{34});
    CheckTokens("  123 ", ConstantToken{123});
    CheckTokens("hello", SymbolToken{"hello"});
    CheckTokens(" world ", SymbolToken{"world"});
    CheckTokens("4+)'.", ConstantToken{4}, SymbolToken{"+"}, BracketToken::CLOSE, QuoteToken{},
                DotToken{});
}

TEST_CASE("Negative numbers") {
    CheckTokens("-2", ConstantToken{-2});
    CheckTokens("-2345", ConstantToken{-2345});
    CheckTokens("-53 - -123", ConstantToken{-53}, SymbolToken{"-"}, ConstantToken{-123});
    CheckTokens("-2 - 2", ConstantToken{-2}, SymbolToken{"-"}, ConstantToken{2});
}

TEST_CASE("Spaces are handled correctly") {
    CheckTokens("      ");
    CheckTokens("  4 +  ", ConstantToken{4}, SymbolToken{"+"});
    CheckTokens(" aba  foo caba   ", SymbolToken{"aba"}, SymbolToken{"foo"}, SymbolToken{"caba"});
}

TEST_CASE("Unary plus and minus") {
    CheckTokens("+4", ConstantToken{4});
    CheckTokens("+67", ConstantToken{67});
    CheckTokens("+1 -  -2 ++3", ConstantToken{1}, SymbolToken{"-"}, ConstantToken{-2},
                SymbolToken{"+"}, ConstantToken{3});
}

TEST_CASE("Symbol names") {
    CheckTokens("foo bar zog-zog?", SymbolToken{"foo"}, SymbolToken{"bar"},
                SymbolToken{"zog-zog?"});
    CheckTokens("baz-15 foo+15", SymbolToken{"baz-15"}, SymbolToken{"foo"}, ConstantToken{15});
    CheckTokens("<=> *42. #hash-tag' 'hi!.##", SymbolToken{"<=>"}, SymbolToken{"*42"}, DotToken{},
                SymbolToken{"#hash-tag"}, QuoteToken{}, QuoteToken{}, SymbolToken{"hi!"},
                DotToken{}, SymbolToken{"##"});
}

TEST_CASE("Brackets") {
    CheckTokens("( ()", BracketToken::OPEN, BracketToken::OPEN, BracketToken::CLOSE);
    CheckTokens("(-2 3 aba)", BracketToken::OPEN, ConstantToken{-2}, ConstantToken{3},
                SymbolToken{"aba"}, BracketToken::CLOSE);
    CheckTokens(")aba(caba)", BracketToken::CLOSE, SymbolToken{"aba"}, BracketToken::OPEN,
                SymbolToken{"caba"}, BracketToken::CLOSE);
    CheckTokens("(.-25##-15)'-8", BracketToken::OPEN, DotToken{}, ConstantToken{-25},
                SymbolToken{"##-15"}, BracketToken::CLOSE, QuoteToken{}, ConstantToken{-8});
}

TEST_CASE("GetToken is not moving") {
    std::stringstream ss{"1234+4"};
    Tokenizer tokenizer{&ss};

    REQUIRE_FALSE(tokenizer.IsEnd());
    RequireEquals(tokenizer.GetToken(), ConstantToken{1234});
    REQUIRE_FALSE(tokenizer.IsEnd());
    RequireEquals(tokenizer.GetToken(), ConstantToken{1234});
}

TEST_CASE("Tokenizer is streaming") {
    std::stringstream ss;
    ss << "2 ";

    Tokenizer tokenizer{&ss};
    REQUIRE_FALSE(tokenizer.IsEnd());
    RequireEquals(tokenizer.GetToken(), ConstantToken{2});

    ss << "* ";
    tokenizer.Next();
    REQUIRE_FALSE(tokenizer.IsEnd());
    RequireEquals(tokenizer.GetToken(), SymbolToken{"*"});

    ss << "2";
    tokenizer.Next();
    REQUIRE_FALSE(tokenizer.IsEnd());
    RequireEquals(tokenizer.GetToken(), ConstantToken{2});

    tokenizer.Next();
    REQUIRE(tokenizer.IsEnd());
}

TEST_CASE("Literal strings are handled correctly") {
    CheckTokens(R"EOF(
                                )EOF");
    CheckTokens(R"EOF(
                4 +
                )EOF",
                ConstantToken{4}, SymbolToken{"+"});
}

TEST_CASE("Empty string handled correctly") {
    std::stringstream ss;
    Tokenizer tokenizer{&ss};
    REQUIRE(tokenizer.IsEnd());
}
