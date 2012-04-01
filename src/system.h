#ifndef system_h
#define system_h

#include <windows.h>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#ifdef WIN32
  #include <shellapi.h>
  #include <tlhelp32.h>
#endif // WIN32 

namespace exception {
  struct DisplaySize
  {
    int width;
    int height;

    DisplaySize(int w, int h) : width(w), height(h)
    {}

    bool operator==(const DisplaySize& v) const
    {
      return width==v.width && height==v.height;
    }

    bool operator<(const DisplaySize& v) const
    {
      return width!=v.width ? width<v.width : height<v.height;
    }
  };
  typedef std::vector<DisplaySize> DisplaySizeList;

  inline const DisplaySizeList& EnumDisplaySize()
  {
    static DisplaySizeList s_dsize;
    if(s_dsize.empty()) {
#ifdef WIN32
      ::DEVMODE dm;
      for(int i=0; ::EnumDisplaySettings(0, i, &dm); ++i) {
        if(dm.dmBitsPerPel>16) {
          DisplaySize t(dm.dmPelsWidth, dm.dmPelsHeight);
          if(std::find(s_dsize.begin(), s_dsize.end(), t)==s_dsize.end()) {
            s_dsize.push_back(t);
          }
        }
      }
      std::sort(s_dsize.begin(), s_dsize.end(), std::less<DisplaySize>());
#else // WIN32 
      DisplaySize dm[] = {
        DisplaySize(640, 480),
        DisplaySize(800, 600),
        DisplaySize(1024, 768),
        DisplaySize(1280, 800),
        DisplaySize(1280, 960),
        DisplaySize(1280, 1024),
      };
#endif // WIN32 
    }
    return s_dsize;
  }

  void OpenURL(const std::string& url);
  bool FindProcess(const char *exe);
}

#endif // system_h
