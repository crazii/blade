# blade
a cross platform 3d engine using c++98

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
