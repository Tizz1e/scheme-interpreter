#include <algorithm>
#include <stdexcept>

#include "tokenizer.h"

bool SymbolToken::operator==(const SymbolToken& other) const {
    return name == other.name;
}

bool QuoteToken::operator==(const QuoteToken&) const {
    return true;
}

bool DotToken::operator==(const DotToken&) const {
    return true;
}

bool ConstantToken::operator==(const ConstantToken& other) const {
    return value == other.value;
}

Tokenizer::Tokenizer(std::istream* in) : input_(in) {
    Next();
}

bool Tokenizer::IsEnd() {
    return !current_token_;
}

void Tokenizer::Next() {
    ReadNextToken();
}

Token Tokenizer::GetToken() {
    if (current_token_) {
        return *current_token_;
    }
    throw std::runtime_error("GetToken: no token");
}

void Tokenizer::ReadNextToken() {
    current_token_.reset();
    std::ws(*input_);
    if (input_->peek() != EOF) {
        char current_char = input_->peek();
        if (current_char == '(' || current_char == ')') {
            ReadBracket();
        } else if (current_char == '.') {
            ReadDot();
        } else if (current_char == '\'') {
            ReadQuote();
        } else if (IsConstantStart()) {
            ReadConstant();
        } else if (SymbolHead(current_char)) {
            ReadSymbol();
        } else {
            throw std::runtime_error("Unknown token at " + std::to_string(input_->tellg()));
        }
    }
}

void Tokenizer::ReadBracket() {
    char current_char = input_->get();
    if (current_char == '(') {
        current_token_ = BracketToken::OPEN;
    } else {
        current_token_ = BracketToken::CLOSE;
    }
}

void Tokenizer::ReadDot() {
    input_->get();
    current_token_ = DotToken();
}

void Tokenizer::ReadQuote() {
    input_->get();
    current_token_ = QuoteToken();
}

void Tokenizer::ReadConstant() {
    std::string number;
    char current_char = input_->get();
    number += static_cast<char>(current_char);

    while (input_->peek() != EOF) {
        current_char = input_->peek();
        if (std::isdigit(current_char) == 0) {
            break;
        }
        number += input_->get();
        ;
    }
    current_token_ = ConstantToken{.value = std::stoi(number)};
}

void Tokenizer::ReadSymbol() {
    std::string symbol;
    char current_char = input_->get();
    symbol += current_char;

    static constexpr std::array kUniqueChars{'+', '-', '/'};

    if (std::ranges::contains(kUniqueChars, current_char)) {
        current_token_ = SymbolToken{symbol};
        return;
    }

    while (input_->peek() != EOF) {
        current_char = input_->peek();
        if (SymbolTail(current_char)) {
            symbol += input_->get();
        } else {
            break;
        }
    }

    current_token_ = SymbolToken{symbol};
}

bool Tokenizer::IsConstantStart() {
    char current_char = input_->peek();
    if (std::isdigit(current_char) != 0) {
        return true;
    }
    if (current_char == '-' || current_char == '+') {
        char previous_char = input_->get();
        if (input_->peek() == EOF) {
            input_->putback(previous_char);
            return false;
        }
        current_char = input_->peek();
        input_->putback(previous_char);
        if (std::isdigit(current_char) != 0) {
            return true;
        }
    }
    return false;
}

bool Tokenizer::SymbolHead(char current_char) {
    static constexpr std::array kSymbolHeadChars{'<', '=', '>', '*', '#', '+', '-', '/'};

    return std::isalpha(current_char) != 0 || std::ranges::contains(kSymbolHeadChars, current_char);
}

bool Tokenizer::SymbolTail(char current_char) {
    static constexpr std::array kSymbolTailChars{'<', '=', '>', '*', '#', '?', '!', '-'};

    return std::isalnum(current_char) != 0 || std::ranges::contains(kSymbolTailChars, current_char);
}
