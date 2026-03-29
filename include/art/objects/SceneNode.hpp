#pragma once
#include <memory>

#include "art/export.hpp"
#include "art/utils/AffineTransform.hpp"

namespace art::objects {
class ART_API SceneNode {
  public:
    using Ptr = std::shared_ptr<SceneNode>;
    virtual ~SceneNode() = default;

    auto transform() const -> const utils::AffineTransform & {
        return _transform;
    }
    auto transform() -> utils::AffineTransform & { return _transform; }

  private:
    utils::AffineTransform _transform;
};
} // namespace art::objects
