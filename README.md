# RISC-V Simulator

这是一个简单的RISC-V模拟器项目，用于模拟RISC-V 64位指令集的一部分指令。该项目使用C语言编写，并采用CMake构建系统。

## 项目结构

risc_simulator/
├── include/
│ ├── cpu.h
│ ├── memory.h
│ └── riscv_defs.h
├── src/
│ ├── cpu.c
│ ├── memory.c
│ └── main.c
├── build/
└── CMakeLists.txt

shell
复制代码

## 依赖

- CMake 3.23 或更高版本
- GCC 编译器

## 构建和运行

### 1. 克隆项目

```bash
git clone <repository_url>
cd risc_simulator
2. 创建构建目录并运行CMake
bash
复制代码
mkdir build
cd build
cmake ..
3. 编译项目
bash
复制代码
make
4. 运行模拟器
bash
复制代码
./risc_simulator
