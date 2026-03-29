#pragma once
#include <memory>

#include "art/export.hpp"
#include "art/objects/SceneNode.hpp"

namespace art::geometry {
class Geometry;
}
namespace art::objects {
class ART_API Object : public SceneNode {
  public:
    using Ptr = std::shared_ptr<Object>;
    Object(const geometry::Geometry &geometry);
    ~Object();

    auto geometry() const -> const geometry::Geometry & { return *_geometry; }
    auto geometry_ptr() const -> std::shared_ptr<const geometry::Geometry> {
        return _geometry;
    }

  private:
    std::shared_ptr<const geometry::Geometry> _geometry;
};
} // namespace art::objects
