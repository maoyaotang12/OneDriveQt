OneDriveQt 完整项目 README.md
☁️ OneDriveQt — Linux 完美 OneDrive 托盘客户端
基于 Qt6 + onedrive-abraunegg 实现的 Linux 原生 OneDrive 托盘工具
解决 Linux 无官方 OneDrive 客户端问题，支持实时文件监控、状态图标自动切换、中文日志、托盘常驻、开机自启，是目前体验最接近 Windows 官方 OneDrive 的 Linux 客户端。

---
✨ 项目特色
- 实时文件监控：基于 --monitor 常驻监控，新增/删除/修改文件自动同步，无变动不刷屏
- 智能托盘图标切换
  - 空闲待命 → 默认云朵图标
  - 同步进行中 → 同步动画图标
  - 同步完成瞬间 → 完成提示图标（2秒自动切回云朵）
  - 停止/错误 对应专属状态图标
- 全程中文日志翻译：原生英文日志全部汉化，同步状态一目了然
- 完整托盘右键菜单：显示窗口、手动同步、启停同步、打开目录、设置、开机自启、关于、退出
- 状态栏文字同步更新：跟随同步状态显示「同步中/同步完成/等待中」
- Arch 打包支持：提供完整 PKGBUILD，一键编译安装
- 桌面快捷方式：内置 .desktop 文件，应用菜单可搜索、可固定任务栏

---
📦 依赖环境
本项目依赖开源版 OneDrive 同步核心 onedrive-abraunegg，必须提前安装
- 运行依赖：qt6-base、onedrive-abraunegg
- 编译依赖：cmake、gcc、make
Arch 安装依赖：
sudo pacman -S qt6-base cmake gcc make onedrive

---
🛠️ 手动编译安装
# 克隆/进入项目目录
cd OneDriveQt

# 创建编译目录
mkdir build && cd build

# 编译配置
cmake ..

# 编译
make -j4

# 运行
./OneDriveQt


---
📥 Arch 一键打包安装（PKGBUILD）
项目内置完整 PKGBUILD + onedriveqt.desktop
makepkg -si
安装完成后：
- 终端直接运行：onedriveqt
- 应用菜单搜索 OneDriveQt 启动
- 支持系统开机自启、任务栏固定

---
🎯 核心运行逻辑
1. 程序启动后自动启用 WebSocket 实时监控云端文件变更
2. 本地增删改文件 → 自动触发同步，托盘切换同步图标
3. 同步完成后短暂显示完成图标 + 完成文字
4. 2秒后自动切回默认云朵空闲图标，常驻后台待命
5. 全程无轮询、无多余日志、性能极低

---
🖱️ 托盘右键菜单功能
- 显示主窗口 / 隐藏主窗口
- 开始同步 / 停止同步 / 手动单次同步
- 打开 OneDrive 同步文件夹
- 程序设置配置
- 开启/关闭开机自启
- 关于（程序信息弹窗）
- 退出程序

---
📁 项目文件结构
OneDriveQt/
├── CMakeLists.txt       # 编译配置文件
├── PKGBUILD             # Arch 打包脚本
├── onedriveqt.desktop   # 桌面快捷方式
├── main.cpp
├── mainwindow.h/cpp     # 主窗口、托盘、状态逻辑
├── settingsdialog       # 设置窗口
├── logindialog          # 登录提示窗口
├── *.svg                # 全套状态图标
└── README.md            # 项目说明


---
💡 常见问题
1. 首次使用无法同步？
需要先在终端执行 onedrive 完成微软账号授权，授权后重启程序即可正常同步。
2. 图标不显示？
确保 svg 图标文件完整，打包版本已自动部署兼容系统图标路径。
3. 同步无反应？
检查 onedrive-abraunegg 是否正常工作、同步目录配置是否正确。

---
📄 开源协议
MIT License，自由使用、修改、二次分发。
⭐ 完全自制、极致适配 Linux 桌面体验的 OneDrive 托盘客户端
