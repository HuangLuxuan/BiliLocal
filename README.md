# BiliLocal
## 0.？？
20220411：坏事了，发现居然不支持XP了，估计是openssl太新了的缘故，而且还不太确定会不会有其他兼容性问题，至少现在它在XP上报缺少bcrypt.dll
## 1.如何编译
首先你得有如下三个库  
libvlc-3.0.9.2-win32  
openssl-3.0.2-win32  
ffmpeg-5.0-win32  
然后到BiliLocal.pro文件末端去改这些库的路径  
接着你需要将各种dll文件和需要的资源拷贝到程序所在目录  
libvlc需要libvlc.dll, libvlccore.dll, plugins\  
openssl需要libcrypto-3.dll  
ffmpeg需要avutil-57.dll和swscale-6.dll  
如果你使用QtCreator可以在项目/构建设置/build步骤中添加如
```
cmd /c xcopy  extrafiles\* build-BiliLocal-Desktop_Qt_5_12_9_MinGW_32_bit-Release\release\ /D /E /R /K /Y
```
的命令
## 2.Qt的DLL
直接运行大概率报错缺DLL，你需要用windeployqt来拷贝Qt的DLL
```
windeployqt --help
Usage: windeployqt [options] [files]
```
QtCreator下可以点击项目/构建设置/构建环境/Open Terminal直接打开环境变量配置好的CMD
## 3.语言文件
依次点击  
QtCreator菜单栏/工具/外部/Qt语言家/更新翻译(lupdate)  
QtCreator菜单栏/工具/外部/Qt语言家/发布翻译(lrelease)  
如果没有出错的话你会在res文件夹里面找到两个qm文件  
直接拷贝到locale文件夹，然后最后把这个文件夹放到程序所在目录，可以利用如何编译中提到的命令行实现  


以下为原README.md文件内容
---
BiliLocal
=========

Add danmaku to local videos

![W](res/00.jpg)

![L](res/01.jpg)

![W](res/02.jpg)

Download: http://pan.baidu.com/s/1kV6PMF9
