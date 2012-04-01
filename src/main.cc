#include "stdafx.h"
#include "ist/ist_sys.h"
#include "system.h"

#ifdef WIN32
  #pragma comment(lib,"SDL.lib")
  #pragma comment(lib,"SDLmain.lib")
  #pragma comment(lib,"SDL_mixer.lib")
  #pragma comment(lib,"opengl32.lib")
  #pragma comment(lib,"glu32.lib")
  #pragma comment(lib,"glew32.lib")
  #pragma comment(lib,"ftgl.lib")
  #pragma comment(lib,"zlib.lib")
  #pragma comment(lib,"libpng.lib")
  #pragma comment(lib,"imm32.lib")
#endif


namespace exception {
  sgui::App* CreateApp(int argc, char *argv[]);
#ifdef EXCEPTION_ENABLE_RUNTIME_CHECK
  void PrintLeakObject();
#endif

  void DumpReplay(const string& path);
  void ExecuteServer(int argc, char *argv[]);
}


int main(int argc, char *argv[])
{
#ifdef EXCEPTION_CHECK_LEAK
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  char *hoge = new char[128];
#endif


  while(exception::FindProcess("updater.exe")) {
    sgui::Sleep(100);
  }
  while(exception::FindProcess("exception_conflict_config.exe")) {
    sgui::Sleep(100);
  }

  if(ist::IsFile("updater.exe.tmp")) {
    if(ist::IsFile("updater.exe")) {
      remove("updater.exe");
    }
    rename("updater.exe.tmp", "updater.exe");
  }


  std::string command = argc>=2 ? argv[1] : "";
#ifdef EXCEPTION_ENABLE_REPLAY_DUMP
  if(command=="dump_replay") {
    if(argc>=3) {
      exception::DumpReplay(argv[2]);
    }
    return 0;
  }
#endif // EXCEPTION_ENABLE_REPLAY_DUMP 
#ifdef EXCEPTION_ENABLE_DEDICATED_SERVER
  if(command=="server") {
    exception::ExecuteServer(argc, argv);
    return 0;
  }
#endif // EXCEPTION_ENABLE_DEDICATED_SERVER 


  try {
    exception::CreateApp(argc, argv)->exec();
  }
  catch(const std::exception& e) {
#ifdef WIN32
    MessageBox(NULL, e.what(), "error", MB_OK);
#else
    puts(e.what());
#endif
  }

#ifdef EXCEPTION_ENABLE_RUNTIME_CHECK
  exception::PrintLeakObject();
#endif

  return 0;
}
