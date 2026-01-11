#pragma once

#include <cstdio>
#include <variant>
#include <istream>
#include <cctype>

struct SymbolToken {
    std::string name;

    bool operator==(const SymbolToken& other) const;
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const;
};

struct DotToken {
    bool operator==(const DotToken&) const;
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    int value;

    bool operator==(const ConstantToken& other) const;
};

using Token = std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken>;

// Интерфейс, позволяющий читать токены по одному из потока.
class Tokenizer {
    std::istream* input_;
    std::optional<Token> current_token_;

public:
    Tokenizer(std::istream* in);

    bool IsEnd();

    void Next();

    Token GetToken();

private:
    void ReadNextToken();

    void ReadBracket();

    void ReadDot();

    void ReadQuote();

    void ReadConstant();

    void ReadSymbol();

    bool IsConstantStart();

    bool SymbolHead(char current_char);

    bool SymbolTail(char current_char);
};
