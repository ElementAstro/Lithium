set_project("lithium.cxxtools.test")
set_version("1.0.0")
set_xmakever("2.5.1")

-- Add gtest dependency
add_requires("gtest")

-- Test Executable
target("lithium.cxxtools.test")
    set_kind("binary")
    add_files("**.cpp")
    add_packages("gtest", "lithium.cxxtools", "loguru")
    set_targetdir("$(buildir)/bin")
target_end()
