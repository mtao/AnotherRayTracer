

#include <spdlog/spdlog.h>

#include <catch2/catch_all.hpp>
#include <art/point.hpp>



using namespace art;

TEST_CASE("test_point", "[point]") {

    Point x = Point::Constant(3);

    CHECK(x.numerator()(0) == 3);
    CHECK(x.numerator()(1) == 3);
    CHECK(x.numerator()(2) == 3);
    CHECK(x.denominator() == 1);

    x.numerator()(2) = 4;
    Point y = x.numerator().eval();

    CHECK(y.numerator()(0) == 3);
    CHECK(y.numerator()(1) == 3);
    CHECK(y.numerator()(2) == 4);
    CHECK(y.denominator() == 1);

    x.numerator()(1) = 5;
    x.denominator() = 2;

    y = x;
    
    CHECK(y.numerator()(0) == 3);
    CHECK(y.numerator()(1) == 5);
    CHECK(y.numerator()(2) == 4);
    CHECK(y.denominator() == 2);
    Point z = y;

    CHECK(z.numerator()(0) == 3);
    CHECK(z.numerator()(1) == 5);
    CHECK(z.numerator()(2) == 4);
    CHECK(z.denominator() == 2);
    CHECK(z == y);


    x.numerator() = {4,5,2};
    x.denominator() = 3;

    z = x + y;

    CHECK(z.denominator() == 6);
    CHECK(z.numerator()(0) == 3*3 + 4*2);
    CHECK(z.numerator()(1) == 5*3 + 5*2);
    CHECK(z.numerator()(2) == 4*3 + 2*2);
    
}
