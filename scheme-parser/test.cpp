#include "parser.h"
#include "error.h"
#include "utils.h"

#include <sstream>
#include <string>
#include <random>
#include <memory>

#include <catch2/catch_test_macros.hpp>

namespace {

std::shared_ptr<Object> ReadFull(std::string str) {
    std::stringstream ss{std::move(str)};
    Tokenizer tokenizer{&ss};

    auto obj = Read(&tokenizer);
    REQUIRE(tokenizer.IsEnd());
    return obj;
}

void CheckNumber(const std::shared_ptr<Object>& obj, int value) {
    REQUIRE(Is<Number>(obj));
    REQUIRE(As<Number>(obj)->GetValue() == value);
}

void CheckSymbol(const std::shared_ptr<Object>& obj, const std::string& name) {
    REQUIRE(Is<Symbol>(obj));
    REQUIRE(As<Symbol>(obj)->GetName() == name);
}

std::shared_ptr<Cell> CheckCell(const std::shared_ptr<Object>& obj) {
    REQUIRE(Is<Cell>(obj));
    return As<Cell>(obj);
}

}  // namespace

TEST_CASE("Read number") {
    CheckNumber(ReadFull("5"), 5);
    CheckNumber(ReadFull("-5"), -5);
    CheckNumber(ReadFull(" 5001"), 5'001);
    CheckNumber(ReadFull(" -90 "), -90);
}

TEST_CASE("Read symbol") {
    SECTION("Plus") {
        CheckSymbol(ReadFull("+"), "+");
    }

    SECTION("Symbol") {
        CheckSymbol(ReadFull(" #foo"), "#foo");
        CheckSymbol(ReadFull("bar! "), "bar!");
        CheckSymbol(ReadFull(" baz-baz "), "baz-baz");
    }

    SECTION("Random symbol") {
        RandomGenerator gen{568'234};
        for (auto i = 0; i < 10; ++i) {
            auto name = gen.GenString(64);
            CheckSymbol(ReadFull(name), name);
        }
    }
}

TEST_CASE("Lists") {
    SECTION("Empty list") {
        REQUIRE_FALSE(ReadFull("()"));
    }

    SECTION("Pair") {
        auto cell = CheckCell(ReadFull("(1 . 2)"));
        CheckNumber(cell->GetFirst(), 1);
        CheckNumber(cell->GetSecond(), 2);
    }

    SECTION("Simple list") {
        auto cell = CheckCell(ReadFull("(1 2)"));
        CheckNumber(cell->GetFirst(), 1);

        cell = CheckCell(cell->GetSecond());
        CheckNumber(cell->GetFirst(), 2);
        REQUIRE_FALSE(cell->GetSecond());
    }

    SECTION("List with operator") {
        auto cell = CheckCell(ReadFull("(+ 1 29)"));
        CheckSymbol(cell->GetFirst(), "+");

        cell = CheckCell(cell->GetSecond());
        CheckNumber(cell->GetFirst(), 1);

        cell = CheckCell(cell->GetSecond());
        CheckNumber(cell->GetFirst(), 29);
        REQUIRE_FALSE(cell->GetSecond());
    }

    SECTION("List with funny end") {
        auto cell = CheckCell(ReadFull("(1 -2 . +3)"));
        CheckNumber(cell->GetFirst(), 1);

        cell = CheckCell(cell->GetSecond());
        CheckNumber(cell->GetFirst(), -2);
        CheckNumber(cell->GetSecond(), 3);
    }

    SECTION("List in list") {
        auto obj = ReadFull("(1 (abacaba 4 -22) . 3)");
        auto cell = CheckCell(obj);
        CheckNumber(cell->GetFirst(), 1);
        cell = CheckCell(cell->GetSecond());
        CheckNumber(cell->GetSecond(), 3);

        auto inner = CheckCell(cell->GetFirst());
        CheckSymbol(inner->GetFirst(), "abacaba");
        inner = CheckCell(inner->GetSecond());
        CheckNumber(inner->GetFirst(), 4);
        inner = CheckCell(inner->GetSecond());
        CheckNumber(inner->GetFirst(), -22);
        REQUIRE_FALSE(inner->GetSecond());
    }

    SECTION("Complex lists") {
        ReadFull("(1 . ())");
        ReadFull("(1 -2 . ())");
        ReadFull("(1 . (-2 . ()))");
        ReadFull("(-14 25 (3 41) (()))");
        ReadFull("(+ 1 -2 (- 31 +4))");
        ReadFull("(() () ())");
        ReadFull("(() () . ())");
        ReadFull("(() (hello-world) -42)");
        ReadFull("(aba! (#caba 2 (1) . 4) (() 3 2 ()))");
    }

    SECTION("Invalid lists") {
        CHECK_THROWS_AS(ReadFull("("), SyntaxError);
        CHECK_THROWS_AS(ReadFull(")"), SyntaxError);
        CHECK_THROWS_AS(ReadFull(" . "), SyntaxError);
        CHECK_THROWS_AS(ReadFull("(1"), SyntaxError);
        CHECK_THROWS_AS(ReadFull("(1 ."), SyntaxError);
        CHECK_THROWS_AS(ReadFull("( ."), SyntaxError);
        CHECK_THROWS_AS(ReadFull("(1 . ()"), SyntaxError);
        CHECK_THROWS_AS(ReadFull("(1 . )"), SyntaxError);
        CHECK_THROWS_AS(ReadFull("(- 3 (+ 2 .))"), SyntaxError);
        CHECK_THROWS_AS(ReadFull("(1 . 2 3)"), SyntaxError);
        CHECK_THROWS_AS(ReadFull("(. 3)"), SyntaxError);
        CHECK_THROWS_AS(ReadFull("- 5"), SyntaxError);
    }
}
