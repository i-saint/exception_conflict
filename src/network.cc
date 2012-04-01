#include "stdafx.h"
#include "network.h"

namespace exception {

  iserver_ptr g_iserver;
  iclient_ptr g_iclient;

  IInputServer* GetInputServer() { return g_iserver.get(); }
  IInputClient* GetInputClient() { return g_iclient.get(); }

  void SendNMessage(const NMessage& t)
  {
    if(g_iclient) {
      g_iclient->push(t);
    }
  }

  bool IsLocalMode()
  {
    return g_iclient && typeid(*g_iclient)==typeid(InputClientLocal&);
  }

  bool IsServerMode()
  {
    return !!g_iserver;
  }

  bool IsClientMode()
  {
#ifdef EXCEPTION_ENABLE_NETPLAY
    return !g_iserver && g_iclient && typeid(*g_iclient)==typeid(InputClientIP&);
#else
    return false;
#endif // EXCEPTION_ENABLE_NETPLAY 
  }

  bool IsReplayMode()
  {
    return g_iclient && typeid(*g_iclient)==typeid(InputClientReplay&);
  }


  size_t GetSessionCount()
  {
    return g_iclient ? g_iclient->getSessionCount() : 0;
  }

  size_t GetSessionID()
  {
    return g_iclient ? g_iclient->getSessionID() : 0;
  }

  session_ptr GetSession(size_t i)
  {
    return g_iclient->getSession(i);
  }

  session_ptr GetSessionByID(size_t id)
  {
    return g_iclient->getSessionByID(id);
  }


  void UpdateServerInfomation(const string& comment)
  {
    SendNMessage(NMessage::ServerStatus(comment));
  }

  void CreateConfigDialog();
  bool ParseChatCommand(const wstring& s)
  {
    if(s.empty() || s[0]!=L'/') {
      return false;
    }

    string cl = _S(s);
    boost::smatch m;
    if(regex_search(cl, m, regex("/(leave|l|exit|bye)"))) {
      if(GetGame()) {
        SendNMessage(NMessage::End());
      }
    }
    else if(regex_search(cl, m, regex("/(config|c)"))) {
      CreateConfigDialog();
    }
    else if(regex_search(cl, m, regex("/(browse|b)"))) {
      if(g_iclient) {
        g_iclient->browse();
      }
    }
    else if(regex_search(cl, m, regex("/f([1-8]) (.*)"))) {
      int i = lexical_cast<int>(m.str(1));
      string message = m.str(2);
      GetConfig()->chat_sc[i-1] = message;
      PushChatText(sgui::Format("# F%d=%s", i, message.c_str()));
    }
    return true;
  }



  void PrintSTDOUT(const string& comment);
  namespace {
    FILE *g_logfile = 0;
    void (*g_print)(const string& comment) = PrintSTDOUT;
    GameOption g_gopt;
    int g_start_on = 1;
    bool g_end_flag = false;
    bool g_end = false;
  }

  void PrintSTDOUT(const string& comment)
  {
    printf(comment.c_str());
  }

  void PrintConsole(const string& comment)
  {
    sgui::WriteConsole(comment.c_str());
    if(g_logfile) {
      fprintf(g_logfile, "%s", comment.c_str());
      fflush(g_logfile);
    }
  }



  void Print(const string& comment)
  {
    g_print(comment);
  }



  void Server_ParseCommandLine(const string& cl)
  {
    IConfig& c = *GetConfig();
    boost::smatch m;
    if(regex_search(cl, m, regex("name=(.+)"))) {
      c.scorename = m.str(1);
      c.checkScorename();
    }
    else if(regex_search(cl, m, regex("autostart=(\\d+)"))) {
      int i = lexical_cast<int>(m.str(1));
      if(i>=0 || i<=16) {
        c.server_autostart = i;
      }
    }
    else if(regex_search(cl, m, regex("port=(\\d+)"))) {
      int i = lexical_cast<int>(m.str(1));
      if(i>=0 || i<=65535) {
        c.server_port = i;
      }
    }
    else if(regex_search(cl, m, regex("max_connection=(\\d+)"))) {
      int i = lexical_cast<int>(m.str(1));
      if(i>=2 || i<=16) {
        c.server_max_connection = trim<int>(2, i, 16);
      }
    }
    else if(regex_search(cl, m, regex("allow_pause=(\\w+)"))) {
      c.server_allow_pause = m.str(1)=="true";
    }
    else if(regex_search(cl, m, regex("enable_console=(\\w+)"))) {
      c.server_enable_console = m.str(1)=="true";
    }
    else if(regex_search(cl, m, regex("enable_log=(\\w+)"))) {
      c.server_enable_log = m.str(1)=="true";
    }
    else if(regex_search(cl, m, regex("logfile=(.+)"))) {
      c.server_logfile = m.str(1);
    }
    else if(regex_search(cl, m, regex("level=(.+)"))) {
      int v = GetLevelFromString(m.str(1));
      if(v!=-1) {
        g_gopt.level = v;
      }
    }
    else if(regex_search(cl, m, regex("map=(.+)"))) {
      int v = GetMapFromString(m.str(1));
      if(v!=-1) {
        g_gopt.stage = v;
      }
    }
    else if(regex_search(cl, m, regex("(h_wave|horde_wave)=([0-9]+)"))) {
      int v = lexical_cast<int>(m.str(2));
      if(v>=1 && v<=30) {
        g_gopt.horde_wave = v;
      }
    }
    else if(regex_search(cl, m, regex("(dm_time|deathmatch_time)=([0-9]+)"))) {
      int v = lexical_cast<int>(m.str(2));
      if(v==3 || v==5 || v==8) {
        g_gopt.deathmatch_time = v;
      }
    }
    else if(regex_search(cl, m, regex("(tf_life|teamfortress_life)=([0-9]+)"))) {
      float v = -1.0f;
      try { v = lexical_cast<float>(m.str(2)); } catch(...) {}
      if(v>=0.5f && v<=5.0f) {
        g_gopt.teamfortress_life = v;
      }
    }
    else if(regex_search(cl, m, regex("(cboost|catapult_boost)=([0-9.]+)"))) {
      float v = -1.0f;
      try { v = lexical_cast<float>(m.str(2)); } catch(...) {}
      if(v>=0.0f && v<=3.0f) {
        g_gopt.cboost = v;
      }
    }
    else if(regex_search(cl, m, regex("(fboost|fraction_boost)=([0-9.]+)"))) {
      float v = -1.0f;
      try { v = lexical_cast<float>(m.str(2)); } catch(...) {}
      if(v>=0.0f && v<=2.0f) {
        g_gopt.fboost = v;
      }
    }
    else if(regex_search(cl, m, regex("(eboost|enemy_boost)=([0-9.]+)"))) {
      float v = -1.0f;
      try { v = lexical_cast<float>(m.str(2)); } catch(...) {}
      if(v>=0.1f && v<=3.0f) {
        g_gopt.eboost = v;
      }
    }
    else if(regex_search(cl, m, regex("autoreply=(.+)"))) {
      std::ifstream in(m.str(1).c_str());
      string l;
      while(std::getline(in, l)) {
        if(!l.empty() && l[0]!='#') {
          l+="\n";
          c.server_autoreply+=l;
        }
      }
    }
  }

  void Server_PrintConfiguration()
  {
    IConfig& c = *GetConfig();
    Print("configuration:\n");
    Print(sgui::Format("name=%s\n", c.scorename.c_str()));
    Print(sgui::Format("autostart=%d\n", c.server_autostart));
    Print(sgui::Format("port=%d\n", c.server_port));
    Print(sgui::Format("max_connection=%d\n", c.server_max_connection));
    Print(sgui::Format("allow_pause=%s\n", (c.server_allow_pause ? "true" : "false")));
    Print(sgui::Format("enable_console=%s\n", (c.server_enable_console ? "true" : "false")));
    Print(sgui::Format("enable_log=%s\n", (c.server_enable_log ? "true" : "false")));
    if(c.server_enable_log) {
      Print(sgui::Format("log_path=%s\n", c.server_logfile.c_str()));
    }
    Print(sgui::Format("level=%s\n", GetLevelString(g_gopt.level).c_str()));
    Print(sgui::Format("map=%s\n", GetMapString(g_gopt.stage).c_str()));
    if(g_gopt.stage==MAP_HORDE) {
      Print(sgui::Format("horde_wave=%d\n", g_gopt.horde_wave));
    }
    if(g_gopt.stage==MAP_DEATHMATCH) {
      Print(sgui::Format("deathmatch_time=%d\n", g_gopt.deathmatch_time));
    }
    if(g_gopt.stage==MAP_TEAMFORTRESS) {
      Print(sgui::Format("teamfortress_life=%d\n", g_gopt.teamfortress_life));
    }
    Print(sgui::Format("catapult_boost=%.2f\n", g_gopt.cboost));
    Print(sgui::Format("fraction_boost=%.2f\n", g_gopt.fboost));
    Print(sgui::Format("enemy_boost=%.2f\n", g_gopt.eboost));
    Print("\n");
  }

#ifdef WIN32
  BOOL WINAPI Server_OnShutdown(DWORD ctrl)
  {
    if(ctrl==CTRL_C_EVENT || ctrl==CTRL_CLOSE_EVENT || ctrl==CTRL_SHUTDOWN_EVENT) {
      g_end_flag = true;
      while(!g_end) {
        ::Sleep(100);
      }
      return TRUE;
    }
    return FALSE;
  }
#endif // WIN32 

  void ExecuteServer(int argc, char *argv[])
  {
    IConfig& c = *GetConfig();
    c.load();
    for(int i=2; i<argc; ++i) {
      Server_ParseCommandLine(argv[i]);
    }

    if(c.server_enable_log) {
      g_logfile = fopen(c.server_logfile.c_str(), "a");
    }
    if(c.server_enable_console) {
      sgui::OpenConsole();
#ifdef WIN32
      ::SetConsoleCtrlHandler(Server_OnShutdown, TRUE);
#endif // WIN32 
    }
    g_print = PrintConsole;

    Print(sgui::Format("exception conflict server %.2f\n\n", float(EXCEPTION_VERSION)/100.0f).c_str());
    Server_PrintConfiguration();

    try {
      while(!g_end_flag) {
        boost::shared_ptr<InputServer> is(new InputServer());
        is->setGameOption(g_gopt);
        is->pushMessage(NMessage::ServerStatus(""));
        is->run();
        while(!is->isAccepting()) {
          if(!is->getError().empty()) {
            throw std::runtime_error(is->getError());
          }
          sgui::Sleep(1);
        }

        Print("server start\n");
        while(is && !is->isComplete()) {
          sgui::Sleep(100);
          if(c.server_autostart>0 && !is->isGameStarted()) {
            if(is->getClientCount()>=c.server_autostart) {
              is->startGame();
            }
          }
          if(!is->getError().empty()) {
            throw std::runtime_error(is->getError());
          }
          if(g_end_flag) {
            break;
          }
        }
        Print("server end\n\n");
      }
      g_end = true;
    }
    catch(const std::runtime_error& e) {
      Print(string("caught an exception: ")+e.what()+"\n");
    }

    if(c.server_enable_console) {
      sgui::CloseConsole();
    }
    if(g_logfile) {
      fclose(g_logfile);
    }
  }


} // namespace exception 
