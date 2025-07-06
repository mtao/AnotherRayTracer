// geometry requires this header, including geometry first to guarantee a
// uniform include order
#include "Geometry.hpp"
#if !defined(ART_GEOMETRY_BOX_HPP)
#define ART_GEOMETRY_BOX_HPP
#include "art/Point.hpp"

namespace art::geometry {
class Box : public Geometry {
   public:
    Box(const Point& min, const Point& max) : _min(min), _max(max) {}
    // creates a unit box centered at the origin
    Box() = default;
    Box(const Box& other) = default;
    Box(Box&& other) = default;
    Box& operator=(const Box& other) = default;
    Box& operator=(Box&& other) = default;

    // utilities for bounding box expansion
    Box& expand(const Box& bb);
    Box& expand(const Point& p);
    bool contains(const Point& p) const;
    bool contains(const Box& bb) const;
    const Point& min() const { return _min; }
    const Point& max() const { return _max; }

    Box bounding_box() const override;
    bool intersect(const Ray& ray,
                   std::optional<Intersection>& isect) const override;

    // an alternate intersection that doesn't fill an isect object
    bool intersect(const Ray& ray) const;

   private:
    Point _min = Point::Constant(-0.5);
    Point _max = Point::Constant(0.5);
};
std::string format_as(const Box& bbox);
}  // namespace art::geometry
#endif
