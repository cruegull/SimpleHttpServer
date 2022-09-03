# CMake 最低版本号要求
cmake_minimum_required (VERSION 3.0)

# 项目信息
project (server)
# 项目目录
aux_source_directory(./http LIST1)
# 指定生成目标
add_executable(server ./server.cc ${LIST1})