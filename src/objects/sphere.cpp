
#include "art/objects/sphere.hpp"


namespace art::objects {
void Sphere::update_bbox() {
    set_bbox({Point::Constant(-radius), Point::Constant(radius)});
}
bool Sphere::intersect(const geometry::Ray& ray,
                       std::optional<Intersection>& isect) const {
    const auto& d = ray.direction;
    //spdlog::info("{} + t [{}]", std::string(ray.origin), fmt::join(d, ","));
    Rational a = ray.direction.norm_powered<2>();
    Rational b(2 * ray.origin.numerator().dot(ray.direction),
               ray.origin.denominator());
    Rational c = ray.origin.squaredNorm() - radius * radius;

    Rational discriminant = b * b - Rational(4) * a * c;
    double disc_double = double(discriminant);
    Rational t;
    if (disc_double < 0) {
        return false;
    } else if (std::abs(disc_double) == 0) {
        t = -b / (2. * a);
    } else {  // two solutions

        Rational sd = sqrt(discriminant);
        Rational t1 = -(b + sd) / (2. * a);
        Rational t2 = -(b - sd) / (2. * a);
        // std::cout << std::string(t1) << ":" << std::string(t2) <<
        // std::endl;
        if (t1 > t2) {
            t = t2;
            // std::swap(t1, t2);
        } else {
            t = t1;
        }
        // Rational min = bool(isect) ? isect->t : Rational(0);
        // if (t1 > min) {
        //     isect.emplace();
        //     isect->t = t1;
        // } else if (t2 > min) {
        //     isect.emplace();
        //     isect->t = t2;
        // } else {
        //     return false;
        // }
    }
    if (t < Rational(0)) {
        return false;
    }
    if (!isect || isect->t > t) {
        isect.emplace();
        isect->t = t;
        isect->position = ray(t);
        isect->normal = isect->position.numerator();
        return true;
    }
    return false;
}
}  // namespace art::objects
