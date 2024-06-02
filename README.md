# template

This is a template for starting new 3DS libctru projects.

## 需要的软件、硬件
* 3DS: 刷入了Luma3DS菜单，有Homebrew启动器
* Citra模拟器
* devkitPro + 3DS Dev
* VSCode编辑器，安装C/C++，Makefile Tools扩展
* SDL2, SDL2_image, SDL2_ttf, SDL2_mixer
* ImGUI
* stb库

## 准备
检查.vscode文件夹内的JSON文件中的路径是否正确，如有需要请修改

## 实机调试方法
1. 在3DS上启动Homebrew Launcher，按Y等待网络传输
2. 按L+下+Select调出Luma3DS菜单，进入Debugger options选项，选择Enable Debugger（若未启用），再选择Force-debug next…选项，退出Luma3DS菜单
3. 在VSCode上打开该工程，点击左侧“运行和调试”按钮，点击“(gdb)Launch”开始调试，VSCode自动进入单步调试状态
4. 结束后如需再次调试，需要重复执行上述步骤

## 模拟器运行方法
在VSCode中打开后按下Ctrl+Shift+P，选择“Task: Run Task”，“run debug”或“run release”

## SDL库编译选项
* SDL2_mixer: `cmake -S. -Bbuild-3ds -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/cmake/3DS.cmake" -DCMAKE_BUILD_TYPE=Release -DSDL2MIXER_VENDORED=ON -DSDL2MIXER_DEPS_SHARED=OFF -DBUILD_SHARED_LIBS=OFF -DSDL2MIXER_WAVPACK=OFF -DSDL2MIXER_OPUS=OFF`
  （不支持WAVPACK和OPUS格式）
* SDL2_ttf: `cmake -S. -Bbuild-3ds -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/cmake/3DS.cmake" -DCMAKE_BUILD_TYPE=Release -DSDL2TTF_VENDORED=ON`
