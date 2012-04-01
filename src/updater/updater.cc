#pragma warning(disable : 4996)

#include "../version.h"

#include "ist/ist_net.h"
#include "ist/ist_sys.h"
#include <tlhelp32.h>
#include <shellapi.h>

using std::string;
namespace {
  boost::asio::io_service g_io_service;
}


class App
{
public:

  struct PatchInfo
  {
    int version;
    string filename;

    PatchInfo(int v, const string& f) :
      version(v), filename(f)
    {}
  };

private:
  int m_version;

public:
  App() : m_version(EXCEPTION_VERSION)
  {
  }

  bool igzExtract(const string& path)
  {
    std::fstream file(path.c_str(), std::ios::in | std::ios::binary);
    ist::biostream bio(file);
    ist::IGZExtracter extract(bio);
    for(size_t i=0; i<extract.getFileCount(); ++i) {
      const string& filename = extract.getFileName(i);
      extract.extractToFile(filename);
      printf("  %s\n", filename.c_str());
    }
    return extract.extractAllFiles()>0;
  }

  void doUpdate()
  {
    ist::HTTPRequest req(g_io_service);
    if(req.get("i-saint.skr.jp", "/exception_conflict/update/")) {
      std::vector<PatchInfo> patchinfo;
      std::istream in(&req.getBuf());
      string l;
      while(std::getline(in, l)) {
        int version;
        char file[32];
        if(sscanf(l.c_str(), "%d, %s", &version, file)==2 && version>m_version) {
          printf("version %.2f found\n", float(version)/100.0f+0.001f);
          patchinfo.push_back(PatchInfo(version, file));
        }
      }

      for(size_t i=0; i<patchinfo.size(); ++i) {
        char buf[256];
        sprintf(buf, "version %.2f へのアップデートを行いますか？", float(patchinfo[i].version)/100.0f+0.001f);
        int ret = MessageBox(0 , buf , "update" , MB_YESNO | MB_ICONQUESTION);
        if(ret==IDNO) {
          break;
        }


        ist::HTTPRequest req(g_io_service);
        string path;
        path = "/exception_conflict/update/";
        path+=patchinfo[i].filename;

        printf("downloading: %s\n", patchinfo[i].filename.c_str());
        if(req.get("i-saint.skr.jp", path)) {
          std::ofstream of(patchinfo[i].filename.c_str(), std::ios::binary);
          while(!req.eof()) {
            int r = req.read(buf, 256);
            of.write(buf, r);
          }
        }

        printf("extracting: %s\n", patchinfo[i].filename.c_str());
        if(igzExtract(patchinfo[i].filename)) {
          m_version = patchinfo[i].version;
          remove(patchinfo[i].filename.c_str());
          printf("version %.2f complete\n\n", float(patchinfo[i].version)/100.0f+0.001f);
        }
        else {
          remove(patchinfo[i].filename.c_str());
          throw std::runtime_error("アップデートに失敗しました。ダウンロードに失敗したと思われます。\r\n何度もこのメッセージが出る場合、サーバー側に問題が出ていると思われます。しばらくお待ちください。");
        }
      }

      if(patchinfo.empty()) {
        MessageBox(0 , "既に最新の状態です。" , "update" , MB_OK | MB_ICONINFORMATION);
      }
      else if(m_version==patchinfo.back().version) {
        MessageBox(0 , "アップデート完了。" , "update" , MB_OK | MB_ICONINFORMATION);
      }
    }
  }
};


bool FindProcess(const char *exe)
{
  HANDLE snap = (HANDLE)-1;
  snap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
  if(snap==(HANDLE)-1) {
    return false;
  }

  bool res = false;
  PROCESSENTRY32 pe;
  pe.dwSize = sizeof(pe);
  BOOL n = Process32First(snap, &pe);
  while(n) {
    if(lstrcmpi(exe, pe.szExeFile)==0) {
      res = true;
      break;
    }
    n = Process32Next(snap, &pe);
  }
  CloseHandle(snap);
  return res;
}


int main()
{
  while(FindProcess("exception_conflict.exe") || FindProcess("exception_conflict_config.exe")) {
    Sleep(100);
  }

  try {
    App *app = new App();
    app->doUpdate();
    delete app;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    CreateProcess(NULL, "exception_conflict.exe", NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
  }
  catch(std::exception& e) {
    MessageBox(NULL, e.what(), "error", MB_OK);
  }

  return 0;
}
