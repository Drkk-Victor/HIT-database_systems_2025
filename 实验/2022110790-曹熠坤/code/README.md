# RucBase
```
 ____  _   _  ____ ____    _    ____  _____ 
|  _ \| | | |/ ___| __ )  / \  / ___|| ____|
| |_) | | | | |   |  _ \ / _ \ \___ \|  _|  
|  _ <| |_| | |___| |_) / ___ \ ___) | |___ 
|_| \_ \___/ \____|____/_/   \_\____/|_____|
```
[RUCBase](https://github.com/ruc-deke/rucbase-lab)是由中国人民大学数据库教学团队开发，配套教育部“101计划”计算机核心教材《数据库管理系统原理与实现》建设的教学用数据库管理系统，旨在支撑面向本科数据库零基础学生的数据库系统课程实验教学。RUCBase系统框架部分参考和借鉴了CMU15-445课程的[BusTub](https://github.com/cmu-db/bustub) 和Standford CS346课程的[Redbase](https://web.stanford.edu/class/cs346/2015/redbase.html)，目前中国人民大学、哈尔滨工业大学、华中科技大学、西安电子科技大学等高校使用该教学系统进行数据库内核实验。

本代码仓库是面向哈尔滨工业大学数据库系统课程教学需要定制的RucBase关系数据库管理系统内核实验代码框架。

## 实验环境：
- 操作系统：Ubuntu 18.04 及以上(64位)
- 编译器：GCC
- 编程语言：C++17
- 管理工具：cmake
- 推荐编辑器：VScode

### 依赖环境库配置：
- gcc 7.1及以上版本（要求完全支持C++17）
- cmake 3.16及以上版本
- flex
- bison
- readline

欲查看有关依赖运行库和编译工具的更多信息，以及如何运行的说明，请查阅[Rucbase使用文档](docs/Rucbase使用文档.md)

欲了解如何在非Linux系统PC上部署实验环境的指导，请查阅[Rucbase环境配置文档](docs/Rucbase环境配置文档.md)

## 实验文档索引

> 这里给出目前公开的文档分类索引

### 开发规范文档

- [Rucbase开发文档](docs/Rucbase开发文档.md)

### 项目说明文档

- [Rucbase环境配置文档](docs/Rucbase环境配置文档.md)
- [Rucbase使用文档](docs/Rucbase使用文档.md)
- [Rucbase项目结构](docs/Rucbase项目结构.pdf)
- [框架图](docs/框架图.pdf)
- [Rucbase学生实验操作说明示例](docs/Rucbase学生实验操作说明示例.md)

### 学生实验指导书

> 请使用命令git pull来拉取最新的实验文档

- [Lab0-磁盘管理器实现](docs/hit-db-class/lab0.pdf)
- [Lab1-缓冲池管理器实现](docs/hit-db-class/lab1.pdf)
- [Lab2-记录管理器实现](docs/hit-db-class/lab2.pdf)
- [Lab3-数据定义的实现](docs/hit-db-class/lab3.pdf)
- [Lab4-数据操纵的实现](docs/hit-db-class/lab4.pdf)

### 时间安排和工作量估计

| **实验**     | **发布时间**      | **截止时间**      | **工作量估计** | **难度系数** |
| ------------ | ----------------- | ----------------- | -------------- | ------------ |
| Lab1 | 第9周 | 第10周 | 12h            |          |
| Lab2 | 第11周 | 第12周 | 10h            |          |
| Lab3 | 第13周 | 第14周 | 8h         |          |
| Lab4 | 第15周 | 第16周 | 12h         |          |
