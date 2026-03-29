// geometry requires this header, including geometry first to guarantee a
// uniform include order
#include "Geometry.hpp"
#if !defined(ART_GEOMETRY_BOX_HPP)
#define ART_GEOMETRY_BOX_HPP
#include "art/Point.hpp"
#include "art/export.hpp"

namespace art::geometry {
class ART_API Box : public Geometry {
  public:
    Box(const Point &min, const Point &max) : _min(min), _max(max) {}
    // creates a unit box centered at the origin
    Box() = default;
    Box(const Box &other) = default;
    Box(Box &&other) = default;
    auto operator=(const Box &other) -> Box & = default;
    auto operator=(Box &&other) -> Box & = default;

    // utilities for bounding box expansion
    auto expand(const Box &bb) -> Box &;
    auto expand(const Point &p) -> Box &;
    auto contains(const Point &p) const -> bool;
    auto contains(const Box &bb) const -> bool;
    auto min() const -> const Point & { return _min; }
    auto max() const -> const Point & { return _max; }

    auto bounding_box() const -> Box override;
    auto intersect(const Ray &ray, std::optional<Intersection> &isect) const
        -> bool override;

    // an alternate intersection that doesn't fill an isect object
    auto intersect(const Ray &ray) const -> bool;

  private:
    Point _min = Point::Constant(-0.5);
    Point _max = Point::Constant(0.5);
};
ART_API auto format_as(const Box &bbox) -> std::string;
} // namespace art::geometry
#endif
