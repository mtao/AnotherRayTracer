#include "art/objects/SceneNode.hpp"

#include <spdlog/spdlog.h>

namespace art::objects {

auto SceneNode::create() -> Ptr { return std::make_shared<SceneNode>(); }
void SceneNode::update_bbox() {
    geometry::BoundingBox bbox;
    for (auto&& c : _children) {
        c->update_bbox();
        spdlog::info("child {} {}", c->bbox().min(), c->bbox().max());
        bbox.expand(c->bbox());
    }
    spdlog::info("{} {}", bbox.min(), bbox.max());
    set_bbox(bbox);
}
void SceneNode::add_node(Object::Ptr node) {
    _children.push_back(node);
    set_bbox(geometry::BoundingBox(bbox()).expand(node->bbox()));
}
bool SceneNode::intersect(const geometry::Ray& ray,
                          std::optional<Intersection>& isect) const {
    if (!ray.hits_bbox(bbox(), isect)) {
        return false;
    }

    for (auto&& c : _children) {
        if (!ray.hits_bbox(c->bbox(), isect)) {
            continue;
        }
        c->intersect(ray, isect);
    }
    return isect.has_value();
}
}  // namespace art::objects
