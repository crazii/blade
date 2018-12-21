# in general  
Blade uses lower camel case naming conventions i.e. "isValid()" not "IsValid()", class members have '**m**' prefix (not 'm_'), static members have 'ms' prefix.  


# structures, classes, enums, typedefs  
  1. class names have no prefix.  
  2. interface class names have '**I**' prefix, i.e. IDevice.  
  3. structure names usually have '**S**' prefix, but not a MUST.  
  4. enum names come with '**E**' prefix.  
  5. enums always use **UPPER CASE**. to avoid conflict, enum constant have a **prefix with abbreviate** of enum type name. i.e.  
  enum ECustomType { CT_NONE, }; //'CT_' prefix is abbreviate for CustomType.  
  6. functor names have '**Fn**' prefix.  
  7. typedefs always use **UPPER CASE**.  
  8. static/global variables uses **UPPER CASE** (recommended).  
  9. macro defines is recommonded in **UPPER CASE**. to avoid conflict with other sources/libs, use **BLADE_** prefix. i.e. "BLADE_COMPILER". macros with args may use lower cases.  
  10. **readability**: prefer explicit sized memebers for structures that needs serialization(IO read/write). i.e. use int8/uint8, int16/uint16, int32/uint32, int64/uint64 instead of char/long/int.  
      **portability**: always serialize utf8 encoding for string (auto done in IStream). Alwyas serialize uint8 for bool.  
  
# scopes: namespace, class members  
  1. do not using namespace in public headers to avoid name polution  
  2. using **::** to access global symbols recommmended, i.e. ::Sleep()  
  3. using **std::** for C function is recommended (unless it is macro), i.e. std::memcpy()  
  4. use **this** in member function is recommended. i.e. this->doit();  
  
# other restrictions
  1. **DO NOT use STL objects as public class members. DO NOT use STL objects as public function parameters.**  
      (especially for MSVC) different CRT lib object may have different memory layouts (size/data layouts etc.),  
      thus if client dll/exe have different CRT runtime, the object datas and corresponding code will mismatch if it has STL objects.
      function parameters are the same. this usually causes crashes.  
        
      by this restriction, Blade can link against different version MSVC CRT libs,  
      i.e. perform a release build (using /MD) of the projects with one single project with opmization disabled(/Od) and linking agaist debug CRT (/MDd),
      so that the single project can be debugged, without bearing the low overall performance of debug build.  
      also, by doing this, blade can establish a universal SDK, not multiple VS2015 sdk & VS2017 sdk.  
      
  2. hide implementation details using **interface** or **Pimpl**.
      the purpose is to reduece depedency & also can improve C++ compling speed.  
      
  3. **DO NOT include platform dependent headers in public headers**  
      to avoid header polution, and directly use platform API by mistake, don't include them.  
      i.e. don't include <Windows.h> in public headers, otherwise someone at sometime (with bad coding status)  
      may use DWORD, INT_PTR, ::Sleep() or ::MesssageBox() directly by mistake, then it will break other platform compilation.  
      one exception is "PlatformData.h", alghough it is public & shared among some modules, you shall not include it  
      unless it is really necessary(and you need to define 'BLADE_USE_PLATFORM_DATA_EXPLICIT' to use it.  
      
  4. headers: visibility and dependency  
      to control (de)coupling, blade has strict rules on dependency and doesn't set all source locations as include paths for all modules.  
      generally, blade doesn't allow mutual dependency between modules, only single direction dependency allowed. i.e. foundation libraries shall not include framework library headers, nor plugin headers.  
      each project/module's source folder contains source/ and header/. header files in source/ are private and only visible to project/moduel itself.  
      headers files in header/ are public but only dependent modules can include them. i.e. headers of subsystem BladeGraphics should be only visible to graphics plugins. BladeTerrain headers are only visible to TerrainEditor.  
      header/interface vs. header/interface/public: public interfaces are used for client inheritances, although there may be default impelemntations for public interfaces. common interface are abstraction for internal implementations and usually don't need/allow to inherit.  
      
# to be continued.
