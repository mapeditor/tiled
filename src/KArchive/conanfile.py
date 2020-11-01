from conans import ConanFile, CMake, tools

class KArchiveConan(ConanFile):
    name = "KArchive"
    version = "5.37.0"
    license = "LGPL-2.1"
    url = "https://api.kde.org/frameworks/karchive/html/index.html"
    settings = "os", "compiler", "build_type", "arch"

    # build this as shared library by default, but static builds are an option
    options = {"shared": [True, False]}
    default_options = "shared=True"
    generators = "cmake"
    exports_sources = "*"

    def build(self):
        cmake = CMake(self)

        # change the library install dir to just "lib" as that's what Conan expects in its packages
        args = ['-DCMAKE_INSTALL_PREFIX="%s"' % self.package_folder,
                '-DKDE_INSTALL_LIBDIR=lib']
        self.run('cmake %s %s %s' % (self.source_folder, cmake.command_line, " ".join(args)))
        self.run("cmake --build . --target install %s" % cmake.build_config)

    def package(self):
        # ideally nothing here, cmake with install takes care of it
        pass

    def package_info(self):
        self.cpp_info.libs = ["KF5Archive"]
        self.cpp_info.includedirs = ['include/KF5', 'include/KF5/KArchive']
