# CMake最低版本要求
cmake_minimum_required(VERSION 3.0)

# 项目名称
project(ConnectionPool_Linux)

# 设置C++标准为C++17，以使用现代特性
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

pkg_search_module(MYSQLCLIENT REQUIRED mariadb mysqlclient)


# 添加头文件目录
# include_directories() 会将目录添加到所有目标的包含路径中
include_directories(
    include # 项目自身的头文件目录
    ${MYSQLCLIENT_INCLUDE_DIRS} # pkg-config 找到的MySQL/MariaDB头文件目录
)

# 添加可执行文件及其源文件
add_executable(
    main
    src/main.cpp
    src/Connection.cpp
    src/ConnectionPool.cpp
)

# 链接所需的库

target_link_libraries(
    main 
    PRIVATE 
    ${MYSQLCLIENT_LIBRARIES} # pkg-config 找到的MySQL/MariaDB库
    pthread                  
)

# 设置可执行文件输出目录，方便管理
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
