# blade
a cross platform 3d engine using c++98

# Features
  1. cross platform arthitecture, easy to add support for more platforms. current supported platform is Win32(x64/x86), Android(ARM/ARM64). iOS is planned.  
  2. plugin based framework, maximum decoupling and flexiable customization.  
  3. task based parallel system. each subsystem has own tasks and cooperate using parallel state data.  
  4. multiple rendering API support: D3D9, OpenGL ES 3, Vulkan is planned.  
  5. graphics features: terrain rendering, 3DS Max importer (deprecated), FBX importer, Dual quaternion animation, IK, etc.  
  6. fast build/compilation, for fast prototyping/iteration. Win32 debug takes **1.5 min** on i7 CPU, Android takes about **5 mins**.  
  7. data binding to XML (non-intrusive); simple binary XML format.  

# How to build
  1. Build dependencies  
    Open Denpendices/build/VC14.1/ALL/All_Denpendency.sln and batch build all (MENU:Build => Batch Build..., press "Selected All" and then "Build").  
    Open All_Dpendency_Android & batch build all if you want to build Android version.  
    After that, you probably don't need to touch that solution(s) any more, exept that new dependencies were added.  
    there're VC14 (VS2015 projects) too, if you want to debug & run on Android, VC14.1 (VS2017) is recommended, since older version may have bugs on installing packages & start apps.  
    
  2. Build tools  
    Open Build/VC14.1/Tools.sln, build only Release|Win32. Blade uses win32 tools to get maximum compatibility.  
    
  3. Build Blade & Run  
    Open Build/VC14.1/Blade.sln (or Blade_Android.sln for android).  
    select the build type and build.  
    set GameStudio (AppPackage_Android for android) as startup project.  
   
   4. Editor mode  
    Supported on Windows version (Blade.sln) only.  
    Right Click "GameStudio" project => Configuration Properties => Debugging => Command Arguments: set "--mode=Editor"  
