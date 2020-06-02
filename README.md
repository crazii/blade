# blade
A cross platform 3d engine using c++98    

Dev Wiki:  https://github.com/crazii/blade/wiki  
Roadmap:  https://github.com/crazii/blade/wiki/Roadmap  
Chat Channel: https://gitter.im/engine-blade/  
Coding Standard: [CodingStandard.md](https://github.com/crazii/blade/blob/master/Document/CodingStandard.md)

**Why re-invent another wheel and why it is open sourced?**  
This project started about in 2010 with the purpose to study the tech behind 3D engines.  
Some of its idea is learned from [Ogre](https://www.ogre3d.org/), some from [Nebula Device](https://sourceforge.net/projects/nebulaengine/), some from in-house engines through my job. Some problems solved in real-life work have also been put into it.  
Thanks to its explosive growth, Blade was open-sourced in Dec/2018 because it cannot be accomplished by any single person. Hopefully someone could help to improve it so that it may be put into practical use in the future.  
Any bug reports, suggestions, criticisms are welcomed. If anything you can learn from it, that'll be great too.    


# Features
  1. Crossplatform architecture, easy-to-add support for more platforms. The supported platforms are:  
      * Windows (x64/x86);  
      * Android (ARM/ARM64);  
      * iOS is planned;  
      
      Supported compilers:
      * MSVC 
      * GCC
      * Clang
      
  2. Plugin-based framework, maximum decoupling and flexible customization.  
  3. Task-based parallel system. Each subsystem has own tasks and cooperate using parallel state data.  
  4. Multiple rendering API support:
      * D3D9  
      * OpenGL ES 3  
      * Vulkan is planned.  
  5. Graphics features:  
      * Large world & terrains;  
      * 3DS Max importer (deprecated);  
      * FBX importer;
      * Dual Quternion animation;  
      * IK;  
      * HBAO, HDR Post Effects;  
      * CSM shadow maps;  
      * Customizable rendering pipelines, using xml as config script. (deferred shading & forward shading, and any customized pipeline you want. Deferred lighting is not maintained but any implementation is welcomed);  
  6. Editor (to be improved)  
      * Scene editing;  
      * Terrain editing, height brush & texture splatting;  
      * Mesh viewer;  
      
  7. Tool chains (CLI):  
      * Texture compressor;  
      * Custom package (BPK) packer (CLI), and package viewer (editor plugin);  
      * Shader compiler;  
      * Mesh & skinned animation importer;  
      * Language processor (in a GNU gettext way) & translation editing tool (a plugin for editor);
  8. Fast build/compilation, for fast prototyping/iteration. **Effective LOC 200k**, takes **1.5 min** on i7 CPU for Win32 debug, Android takes about **5 mins**.  
  9. Data binding to XML (non-intrusive); simple binary XML format.  
  10. Multi-language supprt.  

# How to build
You need Visual Studio 2015/2017 (latest update), with **desktop develepment**, **C++ cross platform (mobile) develeopment**, **MFC development** installed. If any project failed to load, right-click the project and click reload to check the error messge to see which feature is missing and then re-install Visual studio with required feature (only newly required features are installed, it could be fast).  

  0. Use `git submodule update --init` to init git submodule (art resources).  

  1. Build dependencies  
    Open Denpendices/build/VC14.1/ALL/All_Denpendency.sln and batch build all (MENU:Build => Batch Build..., press "Selected All" and then "Build"). At least **Release|Win32** is needed for tools.  
    Open All_Dpendency_Android & batch build all if you want to build Android version.  
    After that, you probably don't need to touch that solution(s) any more, exept that new dependencies were added.  
    There're VC14 (VS2015 projects) too, if you want to debug & run on Android, VC14.1 (VS2017) is recommended, since older VS version may have bugs on installing packages & start apps.  
    
  2. Build tools  
    Open `Build/VC14.1/Tools.sln`, build only **Release|Win32**. Blade uses win32 tools to get maximum compatibility.  
    If you don't have 3DS Max SDK installed, you can **unload the projects manually: BladeMaxExport, BladeMaxExportLoader.** they'll probably fail to load anyway; just ignore them.  
    
  3. Build Blade & Run  
      * Open `Build/VC14.1/Blade.sln` (or Blade_Android.sln for android);  
      * Select the build type and build;  
      * Set GameStudio (AppPackage_Android for android) as startup project;  
   
   4. Editor mode  
    Supported on Windows version (Blade.sln) only.  
    Right Click "GameStudio" project => Configuration Properties => Debugging => Command Arguments: set "--mode=Editor"(without quotes)  
    
# How to debug Android
  Note: currently OBB is needed to work. It is not convenient, and a network file device reading host obb is planed to be added.  
  In the Blade_Android solution , `Terminal Projects/Data/Packages_Android` will build an bpk package located at:  
  `Bin/main.1.com.blade.games.obb`.
  * run the app for the first time, and it will crash on not obb found, but it will create essential folders i.e. /sdcard/Android/obb/com.blade.games/  
  * Use ADB "adb push repo\Bin\main.1.com.blade.games.obb /sdcard/Android/obb/com.blade.games/" to push obb to device
  * Press F5 to start debugging.
  

# Screenshots
 ## Configuration panel  
 ![Config Panel](https://raw.githubusercontent.com/crazii/blade/master/Document/ScreenShots/config_panel.jpg)
 ## Editor: model viewer  
 ![Editor](https://raw.githubusercontent.com/crazii/blade/master/Document/ScreenShots/editor.jpg)
 ## Editor: scene & terrain editing (512x4CSM)  
 ![Terrain Editing](https://raw.githubusercontent.com/crazii/blade/master/Document/ScreenShots/editor_terrain.jpg)
 ## Terrain height brushing & config  
 ![Terrain Brush](https://raw.githubusercontent.com/crazii/blade/master/Document/ScreenShots/terrain_brush.jpg)
 ## Dual quaternion animation: bone visualizer & bounding visualizer  
 ![Dual Quaternion Animation](https://raw.githubusercontent.com/crazii/blade/master/Document/ScreenShots/animation.jpg)
 ## Package browser  
 ![Package Browser](https://raw.githubusercontent.com/crazii/blade/master/Document/ScreenShots/package_browser.png)
 ## Language Editor  
![Language Editor](https://raw.githubusercontent.com/crazii/blade/master/Document/ScreenShots/lang_edit.png)
