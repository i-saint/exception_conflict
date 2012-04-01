#ifndef app_h
#define app_h

namespace exception {
  enum {
    BU_EXIT = 100,
    BU_START,
    BU_RANKING,
    BU_CONFIG,

    DL_UPDATE,

    DL_START,
    BU_START_LOCAL,
    BU_START_SERVER,
    BU_START_CLIENT,
    ED_START_ADDRESS,
    ED_START_PORT,
    BU_START_CONNECT,
    BU_START_BLOWSE,
    LI_START_SERVERLIST,
    BU_START_CONNECTLIST,
    BU_START_UPDATELIST,
    BU_START_BLOWSELIST,
    BU_START_LIGHT,
    BU_START_NORMAL,
    BU_START_HEAVY,
    BU_START_EXCESS,
    BU_START_FUTURE,
    BU_START_MAP_HORDE,
    BU_START_MAP_DEATHMATCH,
    BU_START_MAP_TEAMFORTRESS,
    BU_START_EXTRA,
    CB_START_HORDE_WAVE,
    CB_START_DEATHMATCH_TIME,
    CB_START_TEAMFORTRESS_LIFE,
    BU_START_START,

    DL_RECORD,
    LI_RECORD,
    BU_RECORD,
    BU_RECORD_OPEN,
    BU_RECORD_UPLOAD,
    BU_RECORD_DELETE,
    BU_RECORD_DELETE_CONFIRM,

    DL_STATE,
    LI_STATE,
    BU_STATE,
    BU_STATE_SAVE,
    BU_STATE_LOAD,
    BU_STATE_DELETE,
    BU_STATE_DELETE_CONFIRM,

    LI_RANKING,
    BU_RANKING_OPEN,
    BU_RANKING_LIGHT,
    BU_RANKING_NORMAL,
    BU_RANKING_HEAVY,
    BU_RANKING_EXCESS,
    BU_RANKING_FUTURE,
    BU_RANKING_TEST,
    BU_RANKING_STAGE_ALL,
    BU_RANKING_STAGE_1,
    BU_RANKING_STAGE_2,
    BU_RANKING_STAGE_3,
    BU_RANKING_STAGE_4,
    BU_RANKING_STAGE_5,
    BU_RANKING_STAGE_EX,


    DL_CONFIG,
    LI_RESOLUTION,
    BU_FULLSCREEN,
    BU_VSYNC,
    BU_FRAMESKIP,
    BU_EXLIGHT,
    BU_SHADER,
    BU_VERTEX_BUFFER,
    BU_SIMPLEBG,
    BU_FPS_30,
    BU_NOBLUR,
    BU_BLOOM_INC,
    BU_BLOOM_DEC,
    BU_THREAD_INC,
    BU_THREAD_DEC,
    ED_SCORENAME,
    BU_COLOR,
    DL_COLOR,
    BU_AUTOUPDATE,
    BU_SHOW_FPS,
    BU_SHOW_OBJ,
    BU_BGM,
    BU_BGM_VOLUME_UP,
    BU_BGM_VOLUME_DOWN,
    BU_SE,
    BU_SE_VOLUME_UP,
    BU_SE_VOLUME_DOWN,

    BU_PAUSE_RESUME,
    BU_PAUSE_CONFIG,
    BU_PAUSE_EXIT,
    BU_PAUSE_EXIT_CONFIRM,

    BU_SERVER_AUTODELAY,
    ED_SERVER_DELAY,
    BU_SERVER_DRAWSKIP,
    BU_SERVER_ENABLEPAUSE,
    BU_SERVER_LIGHT,
    BU_SERVER_NORMAL,
    BU_SERVER_HEAVY,
    BU_SERVER_EXCESS,
    BU_SERVER_FUTURE,

    LI_DEBUG,
    BU_DEBUG_P,
    BU_DEBUG_DESTROY,
    BU_DEBUG_ORDER_BY_ID,
    BU_DEBUG_ORDER_BY_DRAW_PRIORITY,
  };

  void CreateStartDialog();

  void CreateConfigDialog();
  void ToggleConfigDialog();

  void CreateChatWindow();
  void CreateAndFocusChatWindow();

  void CreateTitlePanel();
  void CreateGamePanel(IGame *game);
  IResource* CreateResource();


  void ToggleDebugDialog();
  void ToggleObjectBlowserDialog();
  void ToggleProfileDialog();
  void ProfileDialogNotifyThreadCountChange();
  void AddUpdateTime(float v);
  void AddDrawTime(float v);
  void AddThreadTime(boost::thread::id tid, float v);

  void ToggleStateDialog();
  void StateDialogUpdateList();

  extern sgui::Window* g_widget_window;

} // namespace exception 

#endif
