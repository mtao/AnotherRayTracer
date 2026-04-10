// art_term — Ray-trace a mesh and display in the terminal.
//
// Usage: art_term model.obj [width] [height]
//
// Loads a triangle mesh via quiver, builds an ART scene, ray-traces it
// with headlight shading, and outputs the image to the terminal via
// Kitty graphics protocol (or half-block truecolor fallback).

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include <spdlog/spdlog.h>

#include <quiver/MeshBase.hpp>
#include <quiver/io/mesh_io.hpp>

#include "art/Camera.hpp"
#include "art/Image.hpp"
#include "art/geometry/TriangleMesh.hpp"
#include "art/io/image_io.hpp"
#include "art/objects/Object.hpp"
#include "art/zipper_types.hpp"

#include <balsa/terminal/image_output.hpp>

namespace {

/// Extract vertex positions and triangle indices from a quiver MeshBase
/// and build an art::geometry::TriangleMesh.
auto mesh_from_quiver(const quiver::MeshBase &mesh)
    -> std::shared_ptr<art::geometry::TriangleMesh> {
    // Vertex positions: stored as "vertex_positions" at dim 0.
    auto vp_handle =
        mesh.get_attribute_handle<std::array<double, 3>>("vertex_positions", 0);
    if (!vp_handle) {
        spdlog::error("Mesh has no vertex_positions attribute");
        return nullptr;
    }
    const auto &vp = vp_handle->attribute();

    std::vector<art::Vector3d> vertices;
    vertices.reserve(vp.size());
    for (size_t i = 0; i < vp.size(); ++i) {
        const auto &p = vp[i];
        vertices.emplace_back(art::Vector3d({p[0], p[1], p[2]}));
    }

    // Triangle indices: stored as "_SV" at dim 2.
    auto sv_handle =
        mesh.get_attribute_handle<std::array<int64_t, 3>>("_SV", 2);
    if (!sv_handle) {
        spdlog::error("Mesh has no triangle connectivity (_SV at dim 2)");
        return nullptr;
    }
    const auto &sv = sv_handle->attribute();

    std::vector<art::geometry::TriangleMesh::Face> faces;
    faces.reserve(sv.size());
    for (size_t i = 0; i < sv.size(); ++i) {
        const auto &tri = sv[i];
        faces.push_back({static_cast<size_t>(tri[0]),
                         static_cast<size_t>(tri[1]),
                         static_cast<size_t>(tri[2])});
    }

    return std::make_shared<art::geometry::TriangleMesh>(std::move(vertices),
                                                         std::move(faces));
}

/// Compute a bounding sphere center and radius for auto-framing the camera.
auto compute_centroid_and_radius(const art::geometry::TriangleMesh &mesh)
    -> std::pair<art::Vector3d, double> {
    auto verts = mesh.vertices();
    if (verts.empty()) { return {{}, 1.0}; }

    art::Vector3d center({0.0, 0.0, 0.0});
    for (const auto &v : verts) { center = center + v; }
    center = center / static_cast<double>(verts.size());

    double max_dist_sq = 0.0;
    for (const auto &v : verts) {
        double d = (v - center).norm_powered<2>();
        if (d > max_dist_sq) { max_dist_sq = d; }
    }
    return {center, std::sqrt(max_dist_sq)};
}

} // namespace

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: art_term <mesh_file> [width] [height]\n";
        return 1;
    }

    std::filesystem::path mesh_path = argv[1];
    size_t width = 640;
    size_t height = 480;

    if (argc >= 3) { width = std::stoul(argv[2]); }
    if (argc >= 4) { height = std::stoul(argv[3]); }

    // 1. Load mesh via quiver.
    spdlog::info("Loading mesh: {}", mesh_path.string());
    auto result = quiver::io::read_mesh(mesh_path);
    if (!result) {
        spdlog::error("Failed to load mesh: {}", mesh_path.string());
        return 1;
    }
    auto quiver_mesh = std::move(*result);

    // 2. Convert to ART TriangleMesh.
    auto tri_mesh = mesh_from_quiver(*quiver_mesh);
    if (!tri_mesh) { return 1; }
    spdlog::info("Loaded {} vertices, {} triangles",
                 tri_mesh->vertices().size(),
                 tri_mesh->faces().size());

    // 3. Build scene: wrap geometry in an Object.
    art::objects::Object obj(*tri_mesh);
    obj.update_bounding_box();

    // 4. Auto-frame camera: position at 2.5x bounding radius along +Z.
    auto [center, radius] = compute_centroid_and_radius(*tri_mesh);
    double cam_dist = radius * 2.5;
    art::Vector3d eye_vec = center + art::Vector3d({0.0, 0.0, cam_dist});

    art::Camera cam(art::Camera::lookAt(
        art::Point(eye_vec), art::Point(center), art::Point(0, 1, 0)));

    // 5. Ray trace.
    spdlog::info("Rendering {}x{} ...", width, height);
    art::Image image = cam.render(width, height, obj);

    // 6. Convert to RGBA8 and output to terminal.
    auto rgba8 = image.to_rgba8();
    balsa::terminal::emit_auto(width, height, rgba8);

    // Also save as PPM for reference.
    auto save_result = art::io::save(mesh_path.stem().string() + ".ppm", image);
    if (!save_result) {
        spdlog::warn("Failed to save PPM: {}",
                     mesh_path.stem().string() + ".ppm");
    }

    return 0;
}
