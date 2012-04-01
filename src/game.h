#ifndef game_h
#define game_h

namespace exception {


  class Config : public IConfig
  {
  private:
    Config()
    {
      exlight = true;
      npttexture = true;
      bloom =1.0f;

      scorename = "nullpo";
      color = vector4(0.8f, 0.8f, 0.8f);
      width = 640;
      height = 480;

      fullscreen = false;
      vsync = false;
      frameskip = true;
      simplebg = false;
      shader = false;
      vertex_buffer = false;
      noblur = false;
      fps_30 = false;

      show_fps = true;
      show_obj = true;
      update = true;
#ifdef WIN32
      SYSTEM_INFO info;
      GetSystemInfo(&info);
      threads = std::max<size_t>(1, info.dwNumberOfProcessors);
#else
      threads = 2;
#endif

      sound = false;
      bgm_mute = false;
      se_mute = false;
      bgm_volume = 96;
      se_volume = 96;

      key[0]=sgui::KEY_Z;
      key[1]=sgui::KEY_X;
      key[2]=sgui::KEY_V;
      key[3]=sgui::KEY_C;
      key[4]=sgui::KEY_A;
      key[5]=sgui::KEY_S;
      key[6]=sgui::KEY_UP;
      key[7]=sgui::KEY_DOWN;
      key[8]=sgui::KEY_RIGHT;
      key[9]=sgui::KEY_LEFT;

      pad[0]=0;
      pad[1]=1;
      pad[2]=2;
      pad[3]=3;
      pad[4]=4;
      pad[5]=5;
      pad[6]=10;

      controller = 0;
      daxis1 = 0;
      daxis2 = 0;
      threshold1 = 15000;
      threshold2 = 15000;
      hat = false;

      chat_sc[0] = "よろしく！";
      chat_sc[1] = "お疲れ様！";
      chat_sc[2] = "ほげー";
      chat_sc[3] = "フレンドリーファイア、ダメ、絶対";

      last_server = "127.0.0.1";
      last_port = 10040;
      last_network_mode = NETWORK_SERVER;

      server_autostart = 2;
      server_port = 10040;
      server_max_connection = 8;
      server_allow_pause = false;
      server_autodelay = true;
      server_enable_console = true;
      server_enable_log = true;
      server_logfile = "server.log";

      enable_server_mode = false;
    }

  public:
    static Config* instance()
    {
      static Config s_inst;
      return &s_inst;
    }

    void save()
    {
      FILE *f = fopen("config", "wb");
      if(!f) {
        return;
      }

      fprintf(f, "resolution=%dx%d\n", width, height);
      fprintf(f, "fullscreen=%d\n", fullscreen);
      fprintf(f, "vsync=%d\n", vsync);
      fprintf(f, "frameskip=%d\n", frameskip);
      fprintf(f, "exlight=%d\n", exlight);
      fprintf(f, "shader=%d\n", shader);
      fprintf(f, "vertex_buffer=%d\n", vertex_buffer);
      fprintf(f, "simplebg=%d\n", simplebg);
      fprintf(f, "noblur=%d\n", noblur);
      fprintf(f, "30fps=%d\n", fps_30);
      fprintf(f, "bloom=%f\n", bloom);

      fprintf(f, "scorename=%s\n", scorename.c_str());
      fprintf(f, "color=%.2f,%.2f,%.2f\n", color.x, color.y, color.z);
      fprintf(f, "show_fps=%d\n", show_fps);
      fprintf(f, "show_obj=%d\n", show_obj);
      fprintf(f, "update=%d\n", update);
      fprintf(f, "thread=%d\n", threads);

      fprintf(f, "bgm=%d\n", !bgm_mute);
      fprintf(f, "se=%d\n", !se_mute);
      fprintf(f, "bgm_volume=%d\n", bgm_volume);
      fprintf(f, "se_volume=%d\n", se_volume);

      fprintf(f, "key0=%d\n", key[0]);
      fprintf(f, "key1=%d\n", key[1]);
      fprintf(f, "key2=%d\n", key[2]);
      fprintf(f, "key3=%d\n", key[3]);
      fprintf(f, "key4=%d\n", key[4]);
      fprintf(f, "key5=%d\n", key[5]);
      fprintf(f, "key6=%d\n", key[6]);
      fprintf(f, "key7=%d\n", key[7]);
      fprintf(f, "key8=%d\n", key[8]);
      fprintf(f, "key9=%d\n", key[9]);
      fprintf(f, "pad0=%d\n", pad[0]);
      fprintf(f, "pad1=%d\n", pad[1]);
      fprintf(f, "pad2=%d\n", pad[2]);
      fprintf(f, "pad3=%d\n", pad[3]);
      fprintf(f, "pad4=%d\n", pad[4]);
      fprintf(f, "pad5=%d\n", pad[5]);
      fprintf(f, "pad6=%d\n", pad[6]);
      fprintf(f, "controller=%d\n", controller);
      fprintf(f, "daxis1=%d\n", daxis1);
      fprintf(f, "daxis2=%d\n", daxis2);
      fprintf(f, "threshold1=%d\n", threshold1);
      fprintf(f, "threshold2=%d\n", threshold2);
      fprintf(f, "hat=%d\n", hat);

      fprintf(f, "f1=%s\n", chat_sc[0].c_str());
      fprintf(f, "f2=%s\n", chat_sc[1].c_str());
      fprintf(f, "f3=%s\n", chat_sc[2].c_str());
      fprintf(f, "f4=%s\n", chat_sc[3].c_str());
      fprintf(f, "f5=%s\n", chat_sc[4].c_str());
      fprintf(f, "f6=%s\n", chat_sc[5].c_str());
      fprintf(f, "f7=%s\n", chat_sc[6].c_str());
      fprintf(f, "f8=%s\n", chat_sc[7].c_str());

      fprintf(f, "last_server=%s\n", last_server.c_str());
      fprintf(f, "last_port=%d\n", last_port);
      fprintf(f, "last_network_mode=%d\n", last_network_mode);

      fprintf(f, "server_autostart=%d\n",      server_autostart);
      fprintf(f, "server_port=%d\n",           server_port);
      fprintf(f, "server_max_connection=%d\n", server_max_connection);
      fprintf(f, "server_allow_pause=%d\n",    server_allow_pause);
      fprintf(f, "server_autodelay=%d\n",      server_autodelay);
      fprintf(f, "server_enable_console=%d\n", server_enable_console);
      fprintf(f, "server_enable_log=%d\n",     server_enable_log);
      fprintf(f, "server_logfile=%s\n",        server_logfile.c_str());

      fprintf(f, "enable_server_mode=%d\n", enable_server_mode);
      fclose(f);
    }

    void load()
    {
      FILE *in = fopen("config", "rb");
      if(!in) {
        return;
      }

      char l[256];
      char buf[128];
      int i, j;
      float f;
      vector4 c;
      while(fgets(l, 256, in)) {
        if(sscanf(l, "resolution=%dx%d", &i, &j)==2) { width=i; height=j; }
        if(sscanf(l, "fullscreen=%d", &i))   { fullscreen = i!=0; }
        if(sscanf(l, "vsync=%d", &i))        { vsync = i!=0; }
        if(sscanf(l, "frameskip=%d", &i))    { frameskip = i!=0; }
        if(sscanf(l, "exlight=%d", &i))      { exlight = i!=0; }
        if(sscanf(l, "shader=%d", &i))       { shader = i!=0; }
        if(sscanf(l, "vertex_buffer=%d", &i)){ vertex_buffer = i!=0; }
        if(sscanf(l, "simplebg=%d", &i))     { simplebg = i!=0; }
        if(sscanf(l, "noblur=%d", &i))       { noblur = i!=0; }
        if(sscanf(l, "30fps=%d", &i))        { fps_30 = i!=0; }
        if(sscanf(l, "bloom=%f", &f))        { bloom = f; }

        if(sscanf(l, "scorename=%[^\n]", buf)) { scorename = buf; }
        if(sscanf(l, "color=%f,%f,%f", &c.x, &c.y, &c.z)) { color = c; }
        if(sscanf(l, "show_fps=%d", &i))     { show_fps = i!=0; }
        if(sscanf(l, "show_obj=%d", &i))     { show_obj = i!=0; }
        if(sscanf(l, "update=%d", &i))       { update = i!=0; }
        if(sscanf(l, "thread=%d", &i))       { threads = trim(1, i, 8); }

        if(sscanf(l, "bgm=%d", &i))          { bgm_mute = i==0; }
        if(sscanf(l, "se=%d", &i))           { se_mute = i==0; }
        if(sscanf(l, "bgm_volume=%d", &i))   { bgm_volume = i; }
        if(sscanf(l, "se_volume=%d", &i))    { se_volume = i; }

        if(sscanf(l, "key0=%d", &i)) { key[0] = i; }
        if(sscanf(l, "key1=%d", &i)) { key[1] = i; }
        if(sscanf(l, "key2=%d", &i)) { key[2] = i; }
        if(sscanf(l, "key3=%d", &i)) { key[3] = i; }
        if(sscanf(l, "key4=%d", &i)) { key[4] = i; }
        if(sscanf(l, "key5=%d", &i)) { key[5] = i; }
        if(sscanf(l, "key6=%d", &i)) { key[6] = i; }
        if(sscanf(l, "key7=%d", &i)) { key[7] = i; }
        if(sscanf(l, "key8=%d", &i)) { key[8] = i; }
        if(sscanf(l, "key9=%d", &i)) { key[9] = i; }

        if(sscanf(l, "pad0=%d", &i)) { pad[0] = i; }
        if(sscanf(l, "pad1=%d", &i)) { pad[1] = i; }
        if(sscanf(l, "pad2=%d", &i)) { pad[2] = i; }
        if(sscanf(l, "pad3=%d", &i)) { pad[3] = i; }
        if(sscanf(l, "pad4=%d", &i)) { pad[4] = i; }
        if(sscanf(l, "pad5=%d", &i)) { pad[5] = i; }
        if(sscanf(l, "pad6=%d", &i)) { pad[6] = i; }

        if(sscanf(l, "controller=%d", &i)) { controller = i; }
        if(sscanf(l, "daxis1=%d", &i))     { daxis1 = i; }
        if(sscanf(l, "daxis2=%d", &i))     { daxis2 = i; }
        if(sscanf(l, "threshold1=%d", &i)) { threshold1 = i; }
        if(sscanf(l, "threshold2=%d", &i)) { threshold2 = i; }
        if(sscanf(l, "hat=%d", &i))        { hat = i!=0; }

        if(sscanf(l, "f1=%[^\n]", buf)) { chat_sc[0]=buf; }
        if(sscanf(l, "f2=%[^\n]", buf)) { chat_sc[1]=buf; }
        if(sscanf(l, "f3=%[^\n]", buf)) { chat_sc[2]=buf; }
        if(sscanf(l, "f4=%[^\n]", buf)) { chat_sc[3]=buf; }
        if(sscanf(l, "f5=%[^\n]", buf)) { chat_sc[4]=buf; }
        if(sscanf(l, "f6=%[^\n]", buf)) { chat_sc[5]=buf; }
        if(sscanf(l, "f7=%[^\n]", buf)) { chat_sc[6]=buf; }
        if(sscanf(l, "f8=%[^\n]", buf)) { chat_sc[7]=buf; }

        if(sscanf(l, "last_server=%[^\n]", buf)) { last_server = buf; }
        if(sscanf(l, "last_port=%d", &i))        { last_port = i; }
        if(sscanf(l, "last_network_mode=%d", &i)){ last_network_mode = i; }

        if(sscanf(l, "server_autostart=%d", &i))      { server_autostart = trim<int>(0, i, 16); }
        if(sscanf(l, "port=%d", &i))                  { server_port = trim<int>(0, i, 65535); }
        if(sscanf(l, "server_port=%d", &i))           { server_port = trim<int>(0, i, 65535); }
        if(sscanf(l, "server_max_connection=%d", &i)) { server_max_connection = trim<int>(2, i, 16); }
        if(sscanf(l, "server_allow_pause=%d", &i))    { server_allow_pause = i!=0; }
        if(sscanf(l, "server_autodelay=%d", &i))      { server_autodelay = i!=0; }
        if(sscanf(l, "server_enable_console=%d", &i)) { server_enable_console = i!=0; }
        if(sscanf(l, "server_enable_log=%d", &i))     { server_enable_log = i!=0; }
        if(sscanf(l, "server_logfile=%[^\n]", buf))   { server_logfile = buf; }

        if(sscanf(l, "enable_server_mode=%d", &i)) { enable_server_mode = i!=0; }
      }
      checkScorename();

      fclose(in);
    }

    void checkScorename()
    {
      scorename = trim(scorename, 10);
      for(size_t i=0; i<scorename.size(); ++i) {
        if(scorename[i]==',') {
          scorename[i] = '.';
        }
      }
    }
  };


  struct GameOption
  {
  public:
    int level;
    int stage;
    int seed;
    int delay;
    int horde_wave;
    int deathmatch_time;
    float teamfortress_life;
    float cboost;
    float fboost;
    float eboost;

    GameOption() : level(LEVEL_HEAVY), stage(0), seed(::time(0)), delay(5),
      horde_wave(1), deathmatch_time(3), teamfortress_life(1.0f),
      cboost(1.0f), fboost(1.0f), eboost(1.0f)
    {}

    void serialize(ist::bostream& s) const
    {
      s << level << stage << seed
        << horde_wave << deathmatch_time << teamfortress_life
        << cboost << fboost << eboost;
    }

    void deserialize(ist::bistream& s)
    {
      s >> level >> stage >> seed
        >> horde_wave >> deathmatch_time >> teamfortress_life
        >> cboost >> fboost >> eboost;
    }

    void print()
    {
      string r;
      r+=sgui::Format("level: %d\n", level);
      r+=sgui::Format("map: %d\n", stage);
      r+=sgui::Format("seed: %d\n", seed);
      r+=sgui::Format("delay: %d\n", delay);
      r+=sgui::Format("horde_wave: %d\n", horde_wave);
      r+=sgui::Format("deathmatch_time: %d\n", deathmatch_time);
      r+=sgui::Format("teamfortress_life: %.1f\n", teamfortress_life);
      r+=sgui::Format("cboost: %.2f\n", cboost);
      r+=sgui::Format("fboost: %.2f\n", fboost);
      r+=sgui::Format("eboost: %.2f\n", eboost);
      printf(r.c_str());
    }
  };

  struct PadState
  {
    char button[23];
    int move_x;
    int move_y;
    int dir_x;
    int dir_y;

    PadState() : move_x(0), move_y(0), dir_x(0), dir_y(0)
    {
      for(int i=0; i<23; ++i) {
        button[i] = 0;
      }
    }
  };
  PadState GetPadState();
  ushort GetMouseInput();
  ushort GetKeyboardInput();
  ushort GetJoystickInput();

  sgui::Window* GetTitleWindow();
  sgui::Window* GetGameWindow();

#ifdef EXCEPTION_ENABLE_PROFILE
  void AddUpdateTime(float v);
  void AddDrawTime(float v);
  void AddThreadTime(boost::thread::id tid, float v);
#endif // EXCEPTION_ENABLE_PROFILE 

  void Pause();
  void WaitForNewPlayer(int sid);
  void ReleaseWaitingForNewPlayer(int sid);
  void Resume();
  void Ending();
  void FadeToTitle();
  void FadeToGame(const GameOption& opt);
  void FadeToGame(const string& replay);
  void FadeToExit();
  void CreateContinuePanel();
  IGame* CreateGame(const GameOption& opt);
  IGame* CreateGame(Deserializer& s);

  void SaveState(const string& filename);
  void LoadState(const string& filename);
  void LoadState(Deserializer& s);

  SDL_Joystick* GetJoystick();
  const vector2& GetMousePosition();

} // namespace exception 


inline ist::bostream& operator<<(ist::bostream& b, const exception::GameOption& v)
{
  v.serialize(b);
  return b;
}
inline ist::bistream& operator>>(ist::bistream& b, exception::GameOption& v)
{
  v.deserialize(b);
  return b;
}




#endif
