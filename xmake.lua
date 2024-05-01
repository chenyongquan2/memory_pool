-- vs project file
-- xmake project -k vsxmake -m "release,debug"

------------ sdk start ------------

if is_mode("debug") then
	set_runtimes("MDd")
else
	set_runtimes("MD")
end

-- set_runtimes("MD")
set_exceptions "cxx"
set_encodings "utf-8"
------------ sdk end ------------

--- 必须开启这个才能编译debug生成pdb符号文件
add_rules("mode.debug", "mode.release")
-- add_rules("mode.profile", "mode.coverage", "mode.asan", "mode.tsan", "mode.lsan", "mode.ubsan")

--- 必须开启这个clangd，才能正常更新编译数据库，方便代码补全以及代码跳转
add_rules("plugin.compile_commands.autoupdate", {outputdir = "build", lsp = "clangd"})
-- -- add_rules("plugin.vsxmake.autoupdate")
-- add_rules("utils.install.pkgconfig_importfiles")
-- add_rules("utils.install.cmake_importfiles")

add_requireconfs("*", { configs = { debug = is_mode("debug") } })
add_runenvs("PATH", "$(projectdir)/bin")

-- set_runtimes("MD")
set_exceptions "cxx"
set_encodings "utf-8"


--add_requires("conan::asio/1.24.0", { alias = "asio" })
add_requires("spdlog")
add_requires("fmt", { configs = {header_only = true}})
add_requires("gtest")


add_cxxflags ("cl::/Zc:__cplusplus")
-- temp no warning
add_cxxflags ("/w")
add_cxxflags ("/bigobj")

set_languages "c++20"

--add_packages("asio","spdlog","fmt")
add_packages("spdlog","fmt")

-- case1)单项目
target "memory_pool"
add_files("src/*.cpp")

-- case2)多项目，可定义多个项目，每个target包含不同的文件，以及可以exclude排除不相关的cpp编译单元
-- 也可以所有的project: xmake 或者 xmake -ba
-- 编译单个项目: 
-- xmake -b client 
-- target("client")
-- 	add_files("src/*.cpp")
-- 	remove_files( "src/server.cpp", "src/servermain.cpp")

-- -- xmake -b client 
-- target("server")
-- 	add_files("src/*.cpp")
-- 	remove_files( "src/clientmain.cpp")

--单元测试
add_tests("unittest", {
    remove_files = "src/main.cpp",
    packages = {"gtest"},
    files = { "test/*.cpp"},
    defines = "UNITTEST"
})
