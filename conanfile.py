from conan import ConanFile
from conan.tools.meson import MesonToolchain
from conan.tools.gnu import PkgConfigDeps

class AnotherRayTracer(ConanFile):
    requires = [
        "spdlog/1.15.1",
        "catch2/3.8.1",
        "mdspan/0.6.0",
    ]
    settings = "os", "compiler", "build_type", "arch"
    generators = "PkgConfigDeps"

    def generate(self):
        meson = MesonToolchain(self)
        meson.generate()
