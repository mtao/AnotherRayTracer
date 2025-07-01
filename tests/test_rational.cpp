

#include <spdlog/spdlog.h>

#include <art/Rational.hpp>
#include <catch2/catch_all.hpp>

using namespace art;

TEST_CASE("test_rational", "[rational]") {
    std::array<Rational, 4> one{
        {Rational(1, 1), Rational(1), Rational(2, 2), Rational(-2, -2)}};

    std::array<Rational, 4> negone{
        {Rational(-1, 1), Rational(-1), Rational(2, -2), Rational(-2, 2)}};

    std::array<Rational, 4> zero{
        {Rational(0, 1), Rational(0), Rational(0, 0), Rational(-0, 0)}};

    for (const auto& a : one) {
        CHECK(a.positive());
        for (const auto& b : one) {
            CHECK(a == b);
        }
    }

    for (const auto& a : negone) {
        CHECK(!a.positive());
        for (const auto& b : negone) {
            CHECK(a == b);
        }
    }

    for (const auto& a : zero) {
        CHECK(!a.positive());
        for (const auto& b : zero) {
            CHECK(a == b);
        }
    }
    for (const auto& a : one) {
        for (const auto& b : zero) {
            CHECK(a > b);
            CHECK(b < a);
        }
        for (const auto& b : negone) {
            CHECK(a > b);
            CHECK(b < a);
        }
    }
}
