# YunQue-Ticker

YunQue-Ticker 是一款基于 Allwinner T113 + 嵌入式 Linux 的桌面型股票行情显示终端
面向桌面场景的“常驻可见行情看台”

# 项目展示

📺 演示视频（Bilibili）
👉 https://www.bilibili.com/video/BV1xNv4BqE4C/
<div align="center"><img src="docs/pic/logo.bmp" /></div>

# 项目简介

云雀桌面股票看台 是一款运行于嵌入式 Linux 平台的桌面级股票行情展示设备，
通过长条屏常驻显示用户关注的股票价格、涨跌幅等核心信息，
避免频繁解锁手机、打开 App 的操作成本，适用于办公桌、家庭桌面等场景。

# 项目整体架构

本项目从工程角度分为三个层级：

```text
┌──────────────┐
│   应用层     │  Qt 股票行情 App（本仓库）
├──────────────┤
│   系统层     │  Buildroot 系统适配及外设驱动适配
├──────────────┤
│   硬件层     │  T113 硬件设计
└──────────────┘
```
<div align="center"> <img src="docs/pic/frame-V020.png"> </div>


# 本仓库说明（重要）

⚠️ 请务必阅读
> 1、本仓库仅包含《云雀桌面股票看台》的应用层（APP）代码；不包含 T113 Tina5 SDK。
>
> 2、完整的硬件设计可参考 [Bilibili 小智学长桌面智慧屏项目视频教程](https://www.bilibili.com/cheese/play/ep1811686?csource=Hp_searchresult&spm_id_from=333.337.0.0) 或 [小智学长硬件设计文档](https://x509p6c8to.feishu.cn/docx/A5xfd2VVeokJrkxf8M9cvJWCnXT)

# 技术栈

平台与系统

* 芯片平台：Allwinner T113

* 系统：Buildroot

应用层

* GUI：Qt 5.15.9（Qt Widgets）

* 行情接口：[新浪财经](https://www.sinacloud.com/doc/api.html)

# 目录结构说明

## 主目录

```shell
.
├── app/            # Qt 应用源码（股票看台主程序，UI 与业务逻辑）
│
├── dev-service/    # 后端服务程序（提供基本的硬件操作接口）
│
├── build.sh        # 一键编译脚本
│                   # - 交叉编译 Qt APP
│                   # - 编译后端服务程序
│
├── deploy.sh       # 部署脚本
│                   # - 将可执行文件推送至 T113 板卡
│
├── toolchain/      # 交叉编译工具链
│
├── out/            # 可执行文件的编译输出目录
│
├── ota/            # OTA（预留）
│
├── docs/           # 项目文档
│                   # - 项目介绍
│                   # - 版本迭代
│                   # - 接口文档
│                   # - 展示图片 / 演示素材
│
├── firmware/       # 提供可烧录的固件
|
├── README.md       # 仓库入口说明（软链接至 docs/README.md）
│   -> docs/README.md
│
└── .gitignore
```

## app 目录（Qt 应用源码）

```shell
app/Ticker
├── main.cpp            # 程序入口
│
├── appcontext.h/.cpp   # 应用全局上下文
│                       # - 连接服务器
│                       # - 建立服务器与 Presenters 的连接
│
├── widget.h/.cpp/.ui   # 主窗口，框架页的实现
│                       # - 菜单栏、状态栏
│                       # - 页面切换
│
├── core/               # 核心基础模块
│   └── network         # Unix Domain Socket（UDS）
│
├── features            # 功能模块（具体页面的实现）
│   ├── homePage              # 主页面，也即是股票行情页
│   ├── pagelifecycleaware.h  # 页面生命周期
│   ├── pagemsgmanager.cpp    # 页面间通信管理
│   ├── pagemsgmanager.h
│   ├── settingPage           # 设置页实现
│   ├── sysinfoPage           # 系统信息页实现
│   └── wifiPage              # Wi-Fi 配置页实现
│
├── services/           # 服务层抽象接口
│                       # - 与 dev-service 的接口封装
│
├── utils/              # 通用工具
│   ├── confirmDialog   # 确认弹窗
│   ├── log             # 日志系统
│   ├── qssload         # qss 加载
│   └── stock           # 股票行情获取
│
├── res/                # 资源文件
│                       # - 图标
│                       # - 图片
│                       # - 字体
│
├── res.qrc             # Qt 资源文件索引
│
└── Ticker.pro          # Qt 工程文件
```

## dev-service 目录（服务器源码）
```shell
├── main.c              # 服务器入口程序
├── core                # 服务器核心
│                       # - 命令注册
├── include             # 公共头文件
├── Makefile            # 
├── modules             # 接口模块
│   ├── audio           # 音频接口
│   ├── backlight       # 背光接口
│   ├── sysinfo         # 系统信息接口
│   └── wifi            # Wi-Fi 接口
├── third_party         # 第 3 方库
│                       # - cJSON、wpa_ctrl
└── utils               # 插件
│                       # - 链表、日志系统、队列
├── test                # 客户端测试程序，用于测试服务器接口是否可以正常接收/返回数据
├── build               # 编译输出目录
```

# 系统固件下载

系统固件存放在 firmware 目录下，烧录后可直接正常使用。

下面介绍如何单独编译 APP 并替换。

# APP 编译与运行

## 编译

```shell
# build.sh 脚本使用
# Usage: ./build.sh [platform] [options]
#   Platforms:
#     -t113          Build for t113 platform
#     -linux         Build for Linux platform
#   Options:
#     -service       Build only the service
#     -client        Build only the client test
#     -qt            Build only the QT client
#     -all           Build service, client test, and QT client (default)
#     -clean         Clean build outputs for the specified platform
#     -pack          Package the built application into a tar.gz file
#     -h, --help     Show this help message

# 编译 T113 平台的服务端程序
./build.sh -t113 -service
# 编译 T113 平台的用于测试服务端程序的客户端测试程序
./build.sh -t113 -client
# 编译 T113 平台的 Qt 程序
./build.sh -t113 -qt
# 编译全部
./build.sh -t113 all
# 将可执行程序打包成可更新的 APP 压缩包
./build.sh -t113 -pack

# 编译后的可执行程序和打包后的 APP 压缩包在
YunQue-Ticker/out
├── Ticker-app          # Qt 程序
├── Ticker-service      # 服务器
├── Ticker-client-test  # 服务器测试
└── app_v0.5.0.tar      # APP 压缩包
```

# 运行
```shell
# 将打包出的 app_v0.5.0.tar 拷贝至板卡如下目录：
root@YunQue:/opt/Ticker# ls
app_v0.4.0 app_v0.5.0.tar current dict 

# 重启板卡即可完成 APP 更新
```

# 后续

持续更新...

# 联系方式
* vx：Cohen0415
* qq：1033878279
* 欢迎交流与 Issue 讨论