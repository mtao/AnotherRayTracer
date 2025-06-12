
#include "art/objects/sphere.hpp"

#include <iostream>

namespace art::objects {
void Sphere::update_bbox() {
    set_bbox({Point::Constant(-radius), Point::Constant(radius)});
}
bool Sphere::intersect(const geometry::Ray& ray,
                       std::optional<Intersection>& isect) const {
    Rational a = ray.direction.squaredNorm();
    Rational b(2 * ray.origin.numerator().dot(ray.direction),
               ray.origin.denominator());
    Rational c = ray.origin.squaredNorm() - radius * radius;

    Rational discriminant = b * b - Rational(4) * a * c;
    double disc_double = double(discriminant);
    if (std::abs(disc_double) < 1e-8) {
        Rational t = -b / (2. * a);
        if (!isect || isect->t) {
            isect.emplace();
            isect->t = t;
            isect->position = ray(t);
            isect->normal = isect->position.numerator();
        }
        return true;
    } else if (disc_double < 0) {
        return false;
    } else {  // two solutions

        Rational sd = sqrt(discriminant);
        Rational t1 = -(b + sd) / (2. * a);
        Rational t2 = -(b - sd) / (2. * a);
        // std::cout << std::string(t1) << ":" << std::string(t2) <<
        // std::endl;
        if (t1 > t2) {
            std::swap(t1, t2);
        }
        Rational min = bool(isect) ? isect->t : Rational(0);
        if (t1 > min) {
            isect.emplace();
            isect->t = t1;
        } else if (t2 > min) {
            isect.emplace();
            isect->t = t2;
        } else {
            return false;
        }
        isect->position = ray(isect->t);
        isect->normal = isect->position.numerator();
        return true;
    }
}
}  // namespace art::objects
