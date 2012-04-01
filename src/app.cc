#include "stdafx.h"
#ifdef EXCEPTION_ENABLE_DATA_RESCUE
  #include <eh.h>
#endif // EXCEPTION_ENABLE_DATA_RESCUE 

#include <SDL/SDL_mixer.h>

#include "input.h"
#include "game.h"
#include "network.h"
#include "character/creater.h"
#include "app.h"
#include "system.h"

namespace exception {

  class Selecter;
  class FadeoutWindow;

  class DelayPanel;
  class ChatWindow;
  class ConfigDialog;
  class ServerConfigDialog;
  class StartDialog;
  class RecordDialog;

  class TitlePanel;
  class GamePanel;
  class PausePanel;
  class WaitForNewPlayerPanel;


  sgui::Window *g_background_window = 0;
  sgui::Window *g_backwidget_window = 0;
  sgui::Window *g_widget_window = 0;
  Selecter *g_selecter = 0;
  FadeoutWindow *g_fadeout_window = 0;

  DelayPanel *g_delay_panel = 0;
  ChatWindow *g_chat_window = 0;
  sgui::Edit *g_chat_edit = 0;
  ConfigDialog *g_config_dialog = 0;
  ServerConfigDialog *g_server_config_dialog;
  StartDialog *g_start_dialog = 0;
  RecordDialog *g_record_dialog = 0;

  TitlePanel *g_title_panel = 0;
  GamePanel *g_game_panel = 0;
  PausePanel *g_pause_panel = 0;
  WaitForNewPlayerPanel *g_wait_for_new_player_panel = 0;

#if defined(EXCEPTION_ENABLE_NETRANKING) || defined(EXCEPTION_ENABLE_NETUPDATE) || defined(EXCEPTION_ENABLE_NETPLAY)
  boost::asio::io_service g_io_service;
#endif


  void PrintScreen(size_t width, size_t height)
  {
    std::vector<ist::bRGBA> buf(width*height);
    glReadBuffer(GL_FRONT);
    glReadPixels(0,0, width,height, GL_RGBA, GL_UNSIGNED_BYTE, &buf[0][0] );
    glReadBuffer(GL_BACK);

    ist::Bitmap bmp;
    bmp.resize(width, height);
    for(int i=0; i<height; ++i) {
      for(int j=0; j<width; ++j) {
        bmp[i][j] = buf[width*height - (i+1)*width + j];
        bmp[i][j].a = 255;
      }
    }
    char filename[128];
    sprintf(filename, "%d%d.png", ::time(0), sgui::GetTicks());
    bmp.save(filename);
  }


  PadState GetPadState()
  {
    PadState r;
    IConfig *conf = GetConfig();
    SDL_Joystick *joy = GetJoystick();
    if(!joy) {
      return r;
    }

    for(int i=0; i<16; ++i) {
      r.button[i] = SDL_JoystickGetButton(joy, i);
    }
    for(int i=0; i<3; ++i) {
      r.button[16+i*2+0] = SDL_JoystickGetAxis(joy, i+2) > conf->threshold1;
      r.button[16+i*2+1] = SDL_JoystickGetAxis(joy, i+2) <-conf->threshold1;
    }

    r.move_x = SDL_JoystickGetAxis(joy, 0);
    r.move_y = SDL_JoystickGetAxis(joy, 1);
    if(conf->daxis1>=2) { r.dir_x = SDL_JoystickGetAxis(joy, conf->daxis1); }
    if(conf->daxis2>=2) { r.dir_y = SDL_JoystickGetAxis(joy, conf->daxis2); }

    if(conf->hat) {
      Uint8 hat = SDL_JoystickGetHat(joy, 0);
      if     (hat==SDL_HAT_UP)       { r.move_y =-32767; }
      else if(hat==SDL_HAT_RIGHT)    { r.move_x = 32767; }
      else if(hat==SDL_HAT_DOWN)     { r.move_y = 32767; }
      else if(hat==SDL_HAT_LEFT)     { r.move_x =-32767; }
      else if(hat==SDL_HAT_RIGHTUP)  { r.move_x = 32767; r.move_y =-32767; }
      else if(hat==SDL_HAT_RIGHTDOWN){ r.move_x = 32767; r.move_y = 32767; }
      else if(hat==SDL_HAT_LEFTUP)   { r.move_x =-32767; r.move_y =-32767; }
      else if(hat==SDL_HAT_LEFTDOWN) { r.move_x =-32767; r.move_y = 32767; }
    }

    return r;
  }


  // Radeon様特別調査隊 
  bool IsValidDriver()
  {
    string card((char*)glGetString(GL_RENDERER));
    string version((char*)glGetString(GL_VERSION));
    boost::smatch m;
    if(regex_search(card, m, regex("Radeon"))) {
      if(regex_search(version, m, regex("(\\d+)\\.(\\d+)\\.(\\d+)"))) {
        int major = atoi(m.str(1).c_str());
        int minor = atoi(m.str(2).c_str());
        int patch = atoi(m.str(3).c_str());
        // 2.0.6645以上なら合格 
        if(major>2 || (major==2 && minor>0) || (major==2 && minor==0 && patch>=6645)) {
          return true;
        }
        else {
          return false;
        }
      }
    }
    return true;
  }

  bool IsVBOAvailable()
  {
    return GLEW_VERSION_1_5 && GLEW_ARB_vertex_buffer_object;
  }

  bool IsNPTTextureAvailable()
  {
    return IsValidDriver() &&
           GLEW_ARB_texture_non_power_of_two;
  }

  bool IsGLSLAvailable()
  {
    return IsValidDriver() &&
           GLEW_ARB_shading_language_100 &&
           GLEW_ARB_shader_objects &&
           GLEW_ARB_vertex_shader &&
           GLEW_ARB_fragment_shader &&
           GLEW_EXT_framebuffer_object;
  }






  class FPSCounter : public sgui::Window
  {
  private:
    size_t m_pre_sec;
    size_t m_cur_fps;
    size_t m_fps;
    sgui::Label *m_label;

  public:
    FPSCounter(sgui::Window *parent) :
      sgui::Window(parent),
      m_pre_sec(::time(0)),
      m_cur_fps(0),
      m_fps(0),
      m_label(0)
    {
      setPosition(sgui::Point(580,5));
      setSize(sgui::Size(90,16));
    }

    void draw()
    {
      ++m_cur_fps;
      size_t sec = size_t(::time(0));
      if(m_pre_sec!=sec) {
        m_fps = m_cur_fps;
        m_pre_sec = sec;
        m_cur_fps = 0;
      }

      if(GetConfig()->show_fps) {
        wchar_t buf[32];
        swprintf(buf, 32, L"fps: %d", getFPS());
        drawText(buf, sgui::Rect(getSize()));
      }
    }

    size_t getFPS() const { return m_fps; }
  };

  class ObjCounter : public sgui::Window
  {
  public:
    ObjCounter(sgui::Window *parent) :
      sgui::Window(parent)
    {
      setPosition(sgui::Point(580,20));
      setSize(sgui::Size(90,16));
    }

    void draw()
    {
      if(GetConfig()->show_obj) {
        wchar_t buf[32];
        swprintf(buf, 32, L"obj: %d", exception::Object::getCount());
        drawText(buf, sgui::Rect(getSize()));
      }
    }
  };


  class DelayPanel : public sgui::Panel
  {
  typedef sgui::Panel Super;
  private:
    sgui::ToggleButton *m_auto_delay;
    sgui::Edit *m_delay;

  public:
    DelayPanel(sgui::Window *parent, const sgui::Point& pos) : Super(parent, pos, sgui::Size(39,36))
    {
      m_auto_delay = new sgui::ToggleButton(this, sgui::Point(2,2), sgui::Size(35, 16), L"auto", BU_SERVER_AUTODELAY);
      m_auto_delay->setButtonState(sgui::Button::DOWN);
      m_delay = new sgui::Edit(this, sgui::Point(2,18), sgui::Size(35, 16), L"5", ED_SERVER_DELAY);
      m_delay->setVisible(false);

      sgui::Color bg(0, 0, 0, 0.1f);
      sgui::Color border(1, 1, 1, 0.5f);
      setBackgroundColor(bg);
      setBorderColor(border);
      m_auto_delay->setBackgroundColor(bg);
      m_auto_delay->setBorderColor(border);
      m_delay->setBackgroundColor(bg);
      m_delay->setBorderColor(border);

      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_BUTTON_DOWN);
      listenEvent(sgui::EVT_EDIT_ENTER);
    }

    bool handleEvent(const sgui::Event& evt)
    {
      if(evt.getType()==sgui::EVT_BUTTON_UP) {
        if(evt.getSrc()==m_auto_delay) {
          GetConfig()->server_autodelay = false;

          wchar_t buf[16];
          swprintf(buf, 16, L"%d", GetInputClient()->getDelay());
          m_delay->setText(buf);
          m_delay->setVisible(true);
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_BUTTON_DOWN) {
        if(evt.getSrc()==m_auto_delay) {
          GetConfig()->server_autodelay = true;
          m_delay->setVisible(false);
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_EDIT_ENTER) {
        if(evt.getSrc()==m_delay) {
          int delay = GetInputClient()->getDelay();
          try {
            delay = boost::lexical_cast<int>(m_delay->getText());
            delay = std::min<int>(20, std::max<int>(1, delay));
            SendNMessage(NMessage::Delay(delay));
          }
          catch(...) {
          }
          wchar_t buf[16];
          swprintf(buf, 16, L"%d", delay);
          m_delay->setText(buf);
          return true;
        }
      }
      return Super::handleEvent(evt);
    }
  };

  class DelayCounter : public sgui::Window
  {
  typedef sgui::Window Super;
  private:
    sgui::ToggleButton *m_toggle;
    DelayPanel *m_delay_panel;

  public:
    DelayCounter(sgui::Window *parent) : sgui::Window(parent)
    {
      setPosition(sgui::Point(580,35));
      setSize(sgui::Size(90,80));

      m_toggle = new sgui::ToggleButton(this, sgui::Point(), sgui::Size(32,16), L"delay");
      m_toggle->setBackgroundColor(sgui::Color(0, 0, 0, 0));
      m_toggle->setBorderColor(sgui::Color(1, 1, 1, 0.4f));
      m_delay_panel = new DelayPanel(this, sgui::Point(0,16));
      m_delay_panel->setVisible(false);

      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_BUTTON_DOWN);
    }

    void update()
    {
      Super::update();
      if(IsServerMode()) {
        setVisible(true);
        m_toggle->setBorderColor(sgui::Color(1, 1, 1, 0.4f));
      }
      else if(IsClientMode()) {
        setVisible(true);
        m_toggle->setBorderColor(sgui::Color(1, 1, 1, 0.0f));
      }
      else {
        setVisible(false);
        m_toggle->setButtonState(sgui::Button::UP);
        m_delay_panel->setVisible(false);
      }
    }

    void draw()
    {
      wchar_t buf[32];
      swprintf(buf, 32, L": %d", GetInputClient()->getDelay());
      drawText(buf, sgui::Rect(getSize())+sgui::Point(32,0));
    }

    bool handleEvent(const sgui::Event& evt)
    {
      if(evt.getType()==sgui::EVT_BUTTON_UP) {
        if(evt.getSrc()==m_toggle) {
          m_delay_panel->setVisible(false);
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_BUTTON_DOWN) {
        if(evt.getSrc()==m_toggle) {
          if(!IsServerMode()) {
            m_toggle->setButtonState(sgui::Button::UP);
          }
          else {
            m_delay_panel->setVisible(true);
          }
          return true;
        }
      }
      return Super::handleEvent(evt);
    }
  };







  class View : public sgui::View
  {
  typedef sgui::View Super;
  private:
    FPSCounter *m_fps;
    ObjCounter *m_obj;
    DelayCounter *m_delay;
    sgui::Button *m_sbutton;

  public:
    View(const string& title, const sgui::Size& size, const sgui::Size& gsize=sgui::Size(), bool full=false) :
      sgui::View(title, size, gsize, full)
    {
      listenEvent(sgui::EVT_KEYDOWN);
      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_BUTTON_DOWN);
      listenEvent(sgui::EVT_EDIT_ENTER);
      listenEvent(sgui::EVT_LIST_DOUBLECLICK);
      listenEvent(sgui::EVT_COMBO);
      listenEvent(sgui::EVT_DD_RECIEVE);
      setBackgroundColor(sgui::Color(0.0f, 0.0f, 0.0f, 0.0f));

      g_background_window = new sgui::Window(this, getPosition(), getSize());
      g_backwidget_window = new sgui::Window(this, getPosition(), getSize());
      g_widget_window = new sgui::Window(this, getPosition(), getSize());
      m_fps = new FPSCounter(this);
      m_obj = new ObjCounter(this);
      m_delay = new DelayCounter(this);
    }

    size_t getFPS() { return m_fps->getFPS(); }

    bool handleEvent(const sgui::Event& evt)
    {
      if(evt.getType()==sgui::EVT_KEYDOWN) {
        const sgui::KeyboardEvent& e = dynamic_cast<const sgui::KeyboardEvent&>(evt);
        if(e.getKey()==sgui::KEY_PRINT) {
          sgui::Size size = getWindowSize();
          PrintScreen(size_t(size.getWidth()), size_t(size.getHeight()));
          return true;
        }
        else if(e.getKey()==sgui::KEY_RETURN && !evt.isHandled()) {
          CreateAndFocusChatWindow();
          return true;
        }
        else if(e.getKey()>=sgui::KEY_F1 && e.getKey()<=sgui::KEY_F8 && !evt.isHandled()) {
          int i = e.getKey()-sgui::KEY_F1;
          SendNMessage(NMessage::Text(GetConfig()->chat_sc[i]));
          return true;
        }
      }
      return Super::handleEvent(evt);
    }
  };







  struct less_x_coord
  {
    bool operator()(sgui::Window *l, sgui::Window *r) {
      return l->getGlobalPosition().getX() < r->getGlobalPosition().getX();
    }
  };

  struct less_y_coord
  {
    bool operator()(sgui::Window *l, sgui::Window *r) {
      return l->getGlobalPosition().getY() < r->getGlobalPosition().getY();
    }
  };


  class Selecter : public sgui::Window
  {
  private:
    static Selecter *s_inst;
    typedef std::vector<sgui::Window*> cont;
    cont m_targets;
    cont m_targets_x;
    cont m_targets_y;
    sgui::Window *m_panel;
    sgui::Window *m_focus;
    int m_past;
    bool m_refresh;

  public:
    Selecter(sgui::Window *parent) :
      sgui::Window(parent), m_panel(0), m_focus(0), m_past(0), m_refresh(true)
    {
      g_selecter = this;

      listenEvent(sgui::EVT_CONSTRUCT);
      listenEvent(sgui::EVT_APP_DESTROY_WINDOW);
      listenEvent(sgui::EVT_KEYUP);
      listenEvent(sgui::EVT_KEYDOWN);
      listenEvent(sgui::EVT_JOY_AXIS);
      listenEvent(sgui::EVT_JOY_BUTTONUP);
      listenEvent(sgui::EVT_JOY_BUTTONDOWN);
      setDrawPriority(1.5f);

      setPosition(sgui::Point(260, 300));
      setSize(sgui::Size(120, 20));
    }

    ~Selecter()
    {
      g_selecter = 0;
    }

    void setRefreshFlag(bool v) { m_refresh=v; }
    void setFocus(sgui::Window *v) { m_focus=v; }
    sgui::Window* getFocus() { return m_focus; }

    void updateTargetsR(sgui::Window *w, sgui::Window *exclude)
    {
      if(!w || w==exclude) {
        return;
      }
      sgui::Window::window_cont& wc = w->getChildren();
      for(sgui::Window::window_cont::iterator p=wc.begin(); p!=wc.end(); ++p) {
        sgui::Window *c = *p;
        if(c==exclude || dynamic_cast<sgui::IMEWindow*>(c)) {
          continue;
        }

        if(dynamic_cast<sgui::Button*>(c) || dynamic_cast<sgui::Edit*>(c)) {
          m_targets.push_back(c);
        }
        else if(sgui::Combo *cb=dynamic_cast<sgui::Combo*>(c)) {
          m_targets.push_back(c);
          sgui::List *l = cb->getList();
          for(size_t i=0; i<l->getItemCount(); ++i) {
            m_targets.push_back(l->getItem(i));
          }
        }
        else if(sgui::List *l=dynamic_cast<sgui::List*>(c)) {
          for(size_t i=0; i<l->getItemCount(); ++i) {
            m_targets.push_back(l->getItem(i));
          }
        }
        else if(sgui::Panel *pn=dynamic_cast<sgui::Panel*>(c)) {
          if(!pn->getChildren().empty()) {
            m_panel = c;
            m_targets.clear();
            updateTargetsR(c, exclude);
          }
        }
        else if(c==g_widget_window) {
          m_panel = c;
          m_targets.clear();
          updateTargetsR(c, exclude);
        }
        else if(typeid(*c)==typeid(sgui::Window&)) {
          updateTargetsR(c, exclude);
        }
      }
    }

    struct invisible {
      bool operator()(sgui::Window *v) {
        return !v->isVisible();
      }
    };

    void alignTargets()
    {
      m_targets_x = m_targets;
      m_targets_x.erase(std::remove_if(m_targets_x.begin(), m_targets_x.end(), invisible()), m_targets_x.end());
      m_targets_y = m_targets_x;
      std::stable_sort(m_targets_x.begin(), m_targets_x.end(), less_x_coord());
      std::stable_sort(m_targets_y.begin(), m_targets_y.end(), less_y_coord());
    }

    void updateTargets(sgui::Window *w, sgui::Window *exclude=0)
    {
      m_targets.clear();
      updateTargetsR(w, exclude);
      alignTargets();
      if(!m_focus) {
        m_focus = m_targets.empty() ? 0 : m_targets.front();
      }
    }


    sgui::Point GetCenter(sgui::Window *w)
    {
      if(!w) {
        return sgui::Point();
      }
      sgui::Size size = w->getSize();
      return w->getGlobalPosition()+sgui::Point(size.getWidth()/2.0f, size.getHeight()/2.0f);
    }

    void up()
    {
      if(m_targets.empty()) { return; }
      if(m_targets_y.front()==m_focus) { m_focus=m_targets_y.back(); return; }

      int i = 0;
      for(; i<m_targets_y.size(); ++i) { if(m_targets_y[i]==m_focus) break;}

      float dist = 0;
      sgui::Window *n = 0;
      sgui::Point cpos = GetCenter(m_focus);
      for(i=i-1; i>=0; --i) {
        sgui::Point tpos = GetCenter(m_targets_y[i]);
        if(cpos.getY()>tpos.getY()) {
          float d = fabsf(cpos.getX()-tpos.getX()) + fabsf(cpos.getY()-tpos.getY());
          if(!n || d < dist) {
            dist = d;
            n = m_targets_y[i];
          }
        }
      }
      m_focus = n ? n : m_targets_y.back();
    }

    void down()
    {
      if(m_targets.empty()) { return; }
      if(m_targets_y.back()==m_focus) { m_focus=m_targets_y.front(); return; }

      int i = 0;
      for(; i<m_targets_y.size(); ++i) { if(m_targets_y[i]==m_focus) break;}

      float dist = 0;
      sgui::Window *n = 0;
      sgui::Point cpos = GetCenter(m_focus);
      for(i=i+1; i<m_targets_y.size(); ++i) {
        sgui::Point tpos = GetCenter(m_targets_y[i]);
        if(cpos.getY()<tpos.getY()) {
          float d = fabsf(cpos.getX()-tpos.getX()) + fabsf(cpos.getY()-tpos.getY());
          if(!n || d < dist) {
            dist = d;
            n = m_targets_y[i];
          }
        }
      }
      m_focus = n ? n : m_targets_y.front();
    }

    void left()
    {
      if(m_targets.empty()) { return; }
      if(m_targets_x.front()==m_focus) { m_focus=m_targets_x.back(); return; }

      int i = 0;
      for(; i<m_targets_x.size(); ++i) { if(m_targets_x[i]==m_focus) break;}

      float dist = 0;
      sgui::Window *n = 0;
      sgui::Point cpos = GetCenter(m_focus);
      for(i=i-1; i>=0; --i) {
        sgui::Point tpos = GetCenter(m_targets_x[i]);
        if(cpos.getX()>tpos.getX()) {
          float d = fabsf(cpos.getX()-tpos.getX()) + fabsf(cpos.getY()-tpos.getY());
          if(!n || d < dist) {
            dist = d;
            n = m_targets_x[i];
          }
        }
      }
      m_focus = n ? n : m_targets_x.back();
    }

    void right()
    {
      if(m_targets.empty()) { return; }
      if(m_targets_x.back()==m_focus) { m_focus=m_targets_x.front(); return; }

      int i = 0;
      for(; i<m_targets_x.size(); ++i) { if(m_targets_x[i]==m_focus) break;}

      float dist = 0;
      sgui::Window *n = 0;
      sgui::Point cpos = GetCenter(m_focus);
      for(i=i+1; i<m_targets_x.size(); ++i) {
        sgui::Point tpos = GetCenter(m_targets_x[i]);
        if(cpos.getX()<tpos.getX()) {
          float d = fabsf(cpos.getX()-tpos.getX()) + fabsf(cpos.getY()-tpos.getY());
          if(!n || d < dist) {
            dist = d;
            n = m_targets_x[i];
          }
        }
      }
      m_focus = n ? n : m_targets_x.front();
    }


    enum {
      SELECTER_UP,
      SELECTER_DOWN
    };

    void action(int t)
    {
      if(!m_focus) {
        return;
      }

      sgui::EventType type = t==SELECTER_UP ? sgui::EVT_MOUSE_BUTTONUP : sgui::EVT_MOUSE_BUTTONDOWN;
      sgui::Point cpos = sgui::View::instance()->toWindowCoord(GetCenter(m_focus));

      queueEvent(new sgui::MouseEvent(
        type, this, sgui::App::instance()->getMouseFocus(),
        cpos, sgui::Point(), sgui::MOUSE_LEFT));
    }

    void update()
    {
      sgui::Window::update();

      ++m_past;
      if(m_refresh) {
        if(m_panel) {
          sgui::Window *p = m_panel->getParent();
          updateTargets(p ? p : m_panel);
        }
        else {
          updateTargets(getParent());
        }
        m_refresh = false;
      }
      if(m_focus) {
        sgui::Point pos = getPosition();
        pos+=(m_focus->getGlobalPosition()-pos)*0.4f;
        setPosition(pos);

        sgui::Size size = getSize();
        size+=(m_focus->getSize()-size)*0.4f;
        setSize(size);
      }


      if(g_chat_edit && g_chat_edit->isFocused()) {
        return;
      }
      static ushort s_input;
      ushort input = GetKeyboardInput() | GetJoystickInput();
      sgui::Window *prev_focus = m_focus;
      if     (!(s_input&(1<<0)) && (input&(1<<0))) { alignTargets(); up(); }
      else if(!(s_input&(1<<1)) && (input&(1<<1))) { alignTargets(); down(); }
      if     (!(s_input&(1<<3)) && (input&(1<<3))) { alignTargets(); right(); }
      else if(!(s_input&(1<<2)) && (input&(1<<2))) { alignTargets(); left(); }
      if     (!(s_input&(1<<4)) && (input&(1<<4))) { action(SELECTER_DOWN); }
      else if( (s_input&(1<<4)) &&!(input&(1<<4))) { action(SELECTER_UP); }
      s_input = input;

      if(m_focus!=prev_focus) {
        if(sgui::ListItem* li = dynamic_cast<sgui::ListItem*>(m_focus)) {
          sgui::List *ls = dynamic_cast<sgui::List*>(li->getParent());
          ls->setScrollPosition(sgui::Point(0, (ls->getScrollPosition()-li->getPosition()).getY()));
        }
        sgui::Point cpos = sgui::View::instance()->toWindowCoord(GetCenter(m_focus));
        queueEvent(new sgui::MouseEvent(sgui::EVT_MOUSE_MOVE, this, m_focus, cpos, sgui::Point(), sgui::MOUSE_NONE));
      }
    }

    void draw()
    {
      if(!m_focus) {
        return;
      }
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      glColor4fv(float4(0.4f, 0.4f, 0.8f, (sinf(float(m_past)*ist::radian*5.0f)+1.0f)*0.3f+0.4f).v);
      sgui::DrawRect(sgui::Rect(getSize()));
      glColor4fv(float4(1.0f, 1.0f, 1.0f, 1.0f).v);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }


    bool handleEvent(const sgui::Event& evt)
    {
      const IConfig& conf = *GetConfig();
      if(evt.getType()==sgui::EVT_CONSTRUCT) {
        if(dynamic_cast<sgui::Panel*>(evt.getSrc())) {
          m_focus = 0;
        }
        m_refresh = true;
        return true;
      }
      else if(evt.getType()==sgui::EVT_APP_DESTROY_WINDOW) {
        if(evt.getDst() && evt.getDst()->isInclude(m_focus)) {
          m_focus = 0;
        }
        while(evt.getDst() && evt.getDst()->isInclude(m_panel)) {
          m_panel = m_panel->getParent();
        }
        updateTargets(m_panel ? m_panel : getParent(), evt.getDst());
        return true;
      }

      return sgui::Window::handleEvent(evt);
    }
  };



#ifdef EXCEPTION_ENABLE_NETUPDATE
  namespace updater {
    int g_patch_version = EXCEPTION_VERSION;
  }

  class UpdaterThread : public Thread
  {
  public:
    UpdaterThread()
    {}

    void checkUpdate()
    {
      bool newer = false;
      ist::HTTPRequest req(g_io_service);
      if(req.get(EXCEPTION_HOST, EXCEPTION_HOST_PATH "update/")) {
        std::istream in(&req.getBuf());
        string l;
        while(std::getline(in, l)) {
          int version;
          char file[32];
          if(sscanf(l.c_str(), "%d, %s", &version, file)==2 && version>EXCEPTION_VERSION) {
            updater::g_patch_version = version;
          }
        }
      }
    }

    void exec()
    {
      try {
        checkUpdate();
      }
      catch(...) {
      }
    }
  };

  void ExecUpdater()
  {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ::CreateProcess(NULL, "updater.exe", NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS, NULL,NULL, &si, &pi);
    sgui::App::instance()->exit();
  }
#endif // EXCEPTION_ENABLE_NETUPDATE 


  class App : public sgui::App
  {
  typedef sgui::App Super;
  public:

    class AbnormalFrameRate : public std::exception {};
    class ChangeFrameRate : public std::exception {};

#ifdef EXCEPTION_ENABLE_DATA_RESCUE
    static void se_handler(unsigned int code, struct _EXCEPTION_POINTERS* ep)
    {
      throw Win32Exception(ep);
    }
#endif

  private:
    boost::intrusive_ptr<IResource> m_res;
    boost::shared_ptr<ist::Scheduler> m_scheduler;
    SDL_Joystick *m_joy;
    vector2 m_mousepos;

#ifdef EXCEPTION_ENABLE_NETUPDATE
    UpdaterThread *m_updater;
#endif // EXCEPTION_ENABLE_NETUPDATE 

  public:
    App(int argc, char **argv) : sgui::App(argc, argv), m_joy(0)
    {
#ifdef EXCEPTION_ENABLE_DATA_RESCUE
      _set_se_translator(&App::se_handler);
#endif // EXCEPTION_ENABLE_DATA_RESCUE 

      Config *conf = dynamic_cast<Config*>(GetConfig());
      conf->load();

      updateScheduler();


#ifdef EXCEPTION_ENABLE_NETUPDATE
      m_updater = 0;
      if(conf->update) {
        m_updater = new UpdaterThread();
        m_updater->run();
      }
#endif // EXCEPTION_ENABLE_NETUPDATE 

      if(SDL_Init(SDL_INIT_AUDIO)<0) {
        throw Error(SDL_GetError());
      }

      if(Mix_OpenAudio(44100, AUDIO_S16, 2, 4096)<0) {
      }
      else {
        conf->sound = true;
        Mix_VolumeMusic(conf->bgm_mute ? 0 : conf->bgm_volume);
        Mix_Volume(-1, conf->se_mute ? 0 : conf->se_volume);
      }

      m_joy = SDL_JoystickOpen(conf->controller);

      setDefaultFont("resource/VL-Gothic-Regular.ttf");
      SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, int(conf->vsync && !conf->fps_30));
      setView(new View("exception conflict", sgui::Size(float(conf->width), float(conf->height)), sgui::Size(640, 480), conf->fullscreen));

      glewInit();
      printSystemInfo();

      if(!IsGLSLAvailable() || SDL_GetVideoInfo()->vfmt->BitsPerPixel!=32) {
        conf->shader = false;
      }
      if(!IsNPTTextureAvailable()) {
        conf->npttexture = false;
      }
      if(!IsVBOAvailable()) {
        conf->vertex_buffer = false;
      }

      m_res = CreateResource();

      setDefaultBackgroundColor(sgui::Color(0.0f, 0.0f, 0.0f, 0.8f));
      new Selecter(sgui::View::instance());
      CreateTitlePanel();
    }

    ~App()
    {
#ifdef EXCEPTION_ENABLE_NETUPDATE
      delete m_updater;
#endif // EXCEPTION_ENABLE_NETUPDATE 

      View::instance()->releaseChildren();
      m_res = 0;

      g_iclient.reset();
      g_iserver.reset();

      save();

      m_scheduler.reset();

      if(SDL_JoystickOpened(0)) {
        SDL_JoystickClose(m_joy);
      }
      Mix_CloseAudio();
    }

    void save()
    {
      GetConfig()->save();
      if(IGame *game = GetGame()) { game->write(); }
    }

    void printSystemInfo()
    {
      ist::GLInfo glinfo;
      glinfo.print(std::cout);

      printf("joystick\n");
      int joy = SDL_NumJoysticks();
      for(int i=0; i<joy; ++i) {
        printf("Joystick %d: %s\n", i, SDL_JoystickName(i));
      }
      printf("\n");
    }

    void updateScheduler()
    {
      if(m_scheduler) {
        m_scheduler->waitForAll();
        m_scheduler.reset();
      }
      m_scheduler.reset(new ist::Scheduler(GetConfig()->threads));
      if(IGame *g = GetGame()) {
        g->onThreadCountChange();
      }
#ifdef EXCEPTION_ENABLE_PROFILE
      ProfileDialogNotifyThreadCountChange();
#endif // EXCEPTION_ENABLE_PROFILE 
    }


    SDL_Joystick* getJoystick() { return m_joy; }

    void update()
    {
      if(g_iclient && !GetGame()) {
        g_iclient->sync();
      }
      Super::update();
    }


    void BusySleep(size_t ms)
    {
      Uint32 now = sgui::GetTicks();
      while(sgui::GetTicks()<now+ms) { /* busy loop */ }
    }

    void loopVSYNC()
    {
      while(!m_end_flag) {
        Uint32 start = sgui::GetTicks();
        update();
        draw();
        Uint32 elapsed = sgui::GetTicks()-start;
        if(elapsed>17 && GetConfig()->frameskip) {
          size_t frame = 1;
          for(int i=0; i<2; ++i) {
            update();
            ++frame;
            Uint32 t = sgui::GetTicks();
            if((t-start) < (frame*17)) {
              BusySleep((frame*17)-(t-start));
              break;
            }
          }
        }

        View *v = static_cast<View*>(sgui::View::instance());
        if(GetConfig()->vsync && v->getFPS()>100) {
          throw AbnormalFrameRate();
        }
        else if(GetConfig()->fps_30) {
          throw ChangeFrameRate();
        }
      }
    }

    void loop30FPS()
    {
      Uint32 frame = 0;
      Uint32 interval = 100;
      Uint32 pre_frame = sgui::GetTicks()*3;
      while(!m_end_flag) {
        bool wait = false;
        while(sgui::GetTicks()*3-pre_frame < interval) { wait=true; /* busy loop */ }
        if(wait) {
          pre_frame+=interval;
        }
        else {
          pre_frame = sgui::GetTicks()*3; // 処理落ちしている場合 
        }

        Uint32 start = sgui::GetTicks();
        update();
        update();
        draw();
        Uint32 elapsed = sgui::GetTicks()-start;
        if(elapsed>34 && GetConfig()->frameskip) {
          size_t frame = 2;
          for(int i=0; i<2; ++i) {
            update();
            ++frame;
            Uint32 t = sgui::GetTicks();
            if((t-start) < (frame*17)) {
              BusySleep((frame*17)-(t-start));
              break;
            }
          }
        }


        if(!GetConfig()->fps_30) {
          throw ChangeFrameRate();
        }
      }
    }

    void loop60FPS()
    {
      Uint32 interval = 50;
      Uint32 pre_frame = sgui::GetTicks()*3;
      while(!m_end_flag) {
        bool wait = false;
        while(sgui::GetTicks()*3-pre_frame < interval) { wait=true; /* busy loop */ }
        if(wait) {
          pre_frame+=interval;
        }
        else {
          pre_frame = sgui::GetTicks()*3; // 処理落ちしている場合 
        }

        Uint32 start = sgui::GetTicks();
        update();
        draw();
        Uint32 elapsed = sgui::GetTicks()-start;
        if(elapsed>17 && GetConfig()->frameskip) {
          size_t frame = 1;
          for(int i=0; i<2; ++i) {
            update();
            ++frame;
            Uint32 t = sgui::GetTicks();
            if((t-start) < (frame*17)) {
              BusySleep((frame*17)-(t-start));
              break;
            }
          }
        }

        if(GetConfig()->fps_30) {
          throw ChangeFrameRate();
        }
      }
    }


    void loop()
    {
      for(;;) {
        try {
          if(GetConfig()->vsync && !GetConfig()->fps_30) {
            loopVSYNC();
          }
          else if(GetConfig()->fps_30) {
            loop30FPS();
          }
          else {
            loop60FPS();
          }
          break;
        }
        catch(const AbnormalFrameRate&) {
          GetConfig()->vsync = false;
        }
        catch(const ChangeFrameRate&) {
        }
#ifdef EXCEPTION_ENABLE_DATA_RESCUE
        catch(const Win32Exception& e) {
          printf("win32 exception: %s", e.what());
          save();
          throw;
        }
#endif // EXCEPTION_ENABLE_DATA_RESCUE 
      }
    }

    bool handleEvent(const sgui::Event& e)
    {
      if(e.getType()==sgui::EVT_MOUSE_MOVE) {
        const sgui::MouseEvent& m = dynamic_cast<const sgui::MouseEvent&>(e);
        sgui::Point p = m.getPosition();
        m_mousepos = vector2(p.getX(), p.getY());
        return true;
      }
      return Super::handleEvent(e);
    }

    const vector2& getMousePosition() { return m_mousepos; }
    IResource* getResource() { return m_res.get(); }
  };

  App* GetApp() { return static_cast<App*>(sgui::App::instance()); }
  const vector2& GetMousePosition() { return GetApp()->getMousePosition(); }
  IResource* GetResource() { return GetApp()->getResource(); }
  SDL_Joystick* GetJoystick() { return GetApp()->getJoystick(); }

  IConfig* GetConfig() { return Config::instance(); }

  sgui::App* CreateApp(int argc, char *argv[]) { return new App(argc, argv); }



  class ChatWindow : public sgui::Window
  {
  typedef sgui::Window Super;
  public:
    struct Text
    {
      sgui::Color color;
      wstring text;

      Text(const wstring& t, const sgui::Color& c) : text(t), color(c) {}
    };

    typedef std::list<Text> text_cont;

    class TextField : public sgui::Panel
    {
    typedef sgui::Panel Super;
    private:
      text_cont m_log;

    public:
      TextField(sgui::Window *parent) :
        Super(parent, sgui::Point(), parent->getSize())
      {
        setBackgroundColor(sgui::Color(0, 0, 0, 0.1f));
        setBorderColor(sgui::Color(0,0,0,0));
      }

      void pushText(const string& mes, const sgui::Color& c)
      {
        puts(mes.c_str());
        m_log.push_front(Text(_L(mes), c));
        if(m_log.size()>15) {
          m_log.pop_back();
        }
      }

      void draw()
      {
        Super::draw();

        sgui::Point base;
        sgui::Size size = getSize();
        base = sgui::Point(5, size.getHeight()-40);
        for(text_cont::iterator p=m_log.begin(); p!=m_log.end(); ++p) {
          sgui::AssignColor(p->color);
          DrawText(p->text, base);
          base.setY(base.getY()-14.0f);
        }
        sgui::AssignColor(sgui::Color());
      }
    };

    class UserNameField : public sgui::ScrolledWindow
    {
    typedef sgui::ScrolledWindow Super;
    public:
      UserNameField(sgui::Window *parent, const sgui::Point& pos, const sgui::Size& size) :
        Super(parent, pos, size)
      {
        setBackgroundColor(sgui::Color(0, 0, 0, 0.0f));
        setBorderColor(sgui::Color(0,0,0,0));

        sgui::Color border(1,1,1,0.1f);
        sgui::Color bar(1,1,1,0.1f);
        sgui::Color button(0,0,0,0.1f);
        sgui::ScrollBar *sb = getVScrollBar();
        sb->setPosition(sgui::Point(0,0));
        sb->setBorderColor(border);
        sb->setBarColor(bar);
        sb->getScrollButtonTopLeft()->setBackgroundColor(button);
        sb->getScrollButtonTopLeft()->setBorderColor(border);
        sb->getScrollButtonBottomRight()->setBackgroundColor(button);
        sb->getScrollButtonBottomRight()->setBorderColor(border);
      }

      void update()
      {
        Super::update();
        setScrollSize(sgui::Size(0, 14*GetSessionCount()));
        getVScrollBar()->setVisible(GetSessionCount()>8);
      }

      void draw()
      {
        Super::draw();

        sgui::AssignColor(this->getForegroundColor());
        char buf[64];
        sgui::Point base;
        base = sgui::Point(13, getScrollPosition().getY());
        for(int i=0; i<GetSessionCount(); ++i) {
          session_ptr s = GetSession(i);
          sprintf(buf, "%s: ping %d", s->getName().c_str(), s->getPing());
          DrawText(_L(buf), base);
          base.setY(base.getY()+14.0f);
        }
        sgui::AssignColor(sgui::Color());
      }
    };

  private:
    TextField *m_textfield;
    UserNameField *m_users;
    sgui::Edit *m_textbox;
    sgui::ToggleButton *m_toggle;
    bool m_checked;
    float m_freq;

  public:
    ChatWindow() :
        Super(g_backwidget_window, sgui::Point(-1,121), sgui::Size(250, 360)),
        m_checked(true), m_freq(0.0f)
    {
      if(g_chat_window) {
        throw Error("ChatWindow::ChatWindow()");
      }
      g_chat_window = this;

      listenEvent(sgui::EVT_BUTTON_DOWN);
      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_EDIT_ENTER);
      listenEvent(sgui::EVT_KEYDOWN);

      sgui::Color bg(0, 0, 0, 0.2f);
      sgui::Color border(1, 1, 1, 0.5f);
      sgui::Color font(1, 1, 1, 0.9f);

      m_textfield = new TextField(this);
      m_textfield->setForegroundColor(font);

      m_users = new UserNameField(m_textfield, sgui::Point(0,0), sgui::Size(250,112));
      m_users->setForegroundColor(font);

      sgui::Size size = getSize();
      m_textbox = new sgui::Edit(m_textfield, sgui::Point(85, size.getHeight()-20), sgui::Size(160, 16));
      m_textbox->setBackgroundColor(bg);
      m_textbox->setBorderColor(border);
      g_chat_edit = m_textbox;

      sgui::Label *l = new sgui::Label(
        m_textfield, sgui::Point(15, size.getHeight()-20), sgui::Size(90, 16), _L(GetConfig()->scorename+":"));
      l->setForegroundColor(font);

      m_toggle = new sgui::ToggleButton(this, sgui::Point(0, size.getHeight()-20), sgui::Size(10, 15));
      m_toggle->setBackgroundColor(bg);
      m_toggle->setBorderColor(border);
      m_toggle->setButtonState(sgui::Button::DOWN);

      sgui::Color c(1,1,1,0.7f);
      pushText("tab key: show|hide chat window", c);
      pushText("type '/leave': leave the game", c);
      pushText("type '/config': call config dialog", c);
      pushText("type '/browse': browse server info", c);
    }

    ~ChatWindow()
    {
      g_chat_window = 0;
      g_chat_edit = 0;
    }

    void focus()
    {
      toggleTextField(true);
      m_textbox->focus();
    }

    void update()
    {
      Super::update();
      if(!m_checked) {
        if(m_textfield->isVisible()) {
          m_checked = true;
          m_freq = 0.0f;
          m_toggle->setBackgroundColor(sgui::Color(0, 0, 0, 0.1f));
        }
        else {
          m_freq+=6.0f;
          float s = sinf(m_freq*ist::radian);
          m_toggle->setBackgroundColor(sgui::Color(s*0.5f, s*0.5f, s, 0.2f+s*0.8f));
        }
      }
    }

    void pushText(const string& mes, const sgui::Color& c)
    {
      m_textfield->pushText(mes, c);
      if(!m_textfield->isVisible()) {
        m_checked = false;
      }
    }

    void toggleTextField(bool v)
    {
      if(v) {
        m_toggle->setButtonState(sgui::Button::DOWN);
        m_textfield->setVisible(true);
      }
      else {
        m_toggle->setButtonState(sgui::Button::UP);
        if(!IsServerMode() && !IsClientMode() && !GetGame()) {
          destroy();
        }
        else {
          m_textfield->setVisible(false);
        }
      }
    }

    bool handleEvent(const sgui::Event &evt)
    {
      if(evt.getType()==sgui::EVT_BUTTON_DOWN && evt.getSrc()==m_toggle) {
        toggleTextField(true);
        return true;
      }
      else if(evt.getType()==sgui::EVT_BUTTON_UP && evt.getSrc()==m_toggle) {
        toggleTextField(false);
        return true;
      }
      else if(evt.getType()==sgui::EVT_EDIT_ENTER && evt.getSrc()==m_textbox) {
        const sgui::EditEvent& e = dynamic_cast<const sgui::EditEvent&>(evt);
        if(!m_textbox->getText().empty()) {
          if(!ParseChatCommand(m_textbox->getText())) {
            SendNMessage(NMessage::Text(_S(m_textbox->getText())));
          }
          m_textbox->setText(L"");
        }
        m_textbox->defocus();
        return true;
      }
      else if(evt.getType()==sgui::EVT_KEYDOWN) {
        if(dynamic_cast<const sgui::KeyboardEvent&>(evt).getKey()==sgui::KEY_TAB) {
          toggleTextField(!m_toggle->getButtonState()==sgui::Button::DOWN);
          return true;
        }
      }
      return Super::handleEvent(evt);
    }
  };

  void CreateChatWindow()
  {
    if(!g_chat_window) {
      new ChatWindow();
    }
  }

  void CreateAndFocusChatWindow()
  {
    CreateChatWindow();
    g_chat_window->focus();
  }

  void PushChatText(const string& t, const sgui::Color& c)
  {
    if(!g_chat_window) {
      new ChatWindow();
    }
    g_chat_window->pushText(t,c);
  }




  void show_error(const std::string& mes)
  {
    fprintf(stderr, mes.c_str());
    new sgui::MessageDialog(_L("exception caught"), _L(mes), g_widget_window, sgui::Point(120, 90), sgui::Size(400, 300));
  }




  class ConfigDialog : public sgui::Dialog
  {
  typedef sgui::Dialog Super;
  private:
    sgui::Label *m_bloom;
    sgui::Label *m_thread;
    sgui::Label *m_bgm_vol;
    sgui::Label *m_se_vol;
    sgui::Panel *m_color_sample;

  public:
    ConfigDialog() :
      sgui::Dialog(g_widget_window, sgui::Point(30,30), sgui::Size(360,295), L"config", DL_CONFIG)
    {
      if(g_config_dialog) {
        throw Error("ConfigDialog::ConfigDialog()");
      }
      g_config_dialog = this;

      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_BUTTON_DOWN);
      listenEvent(sgui::EVT_LIST_MOVE_SELECTION);
      listenEvent(sgui::EVT_LIST_SELECT);
      listenEvent(sgui::EVT_EDIT);
      listenEvent(sgui::EVT_COLORDIALOG);

      sgui::Label *label;
      sgui::ToggleButton *tb;
      sgui::Button *b;

      sgui::Point base(5, 30);

      label = new sgui::Label(this, base, sgui::Size(90, 16), L"画面解像度");
      new sgui::ToolTip(label, L"画面解像度を指定します。\nゲーム再起動後反映されます。");
      base+=sgui::Point(0, 20);
      sgui::List *list = new sgui::List(this, base, sgui::Size(90,100), LI_RESOLUTION);
      Config& conf = *dynamic_cast<Config*>(GetConfig());
      const DisplaySizeList& dsl = EnumDisplaySize();
      wchar_t buf[32];
      for(int i=0; i<dsl.size(); ++i) {
        int w = size_t(dsl[i].width);
        int h = size_t(dsl[i].height);
        swprintf(buf, 32, L"%dx%d", w, h);
        list->addItem(new sgui::ListItem(buf));
        if(w==conf.width && h==conf.height) {
          list->setSelection(i);
        }
      }

      base+=sgui::Point(0, 110);
      tb = new sgui::ToggleButton(this, base, sgui::Size(90, 20), L"フルスクリーン", BU_FULLSCREEN);
      new sgui::ToolTip(tb, L"指定解像度、32bitカラー、\nリフレッシュレート60Hzのフルスクリーンモードでの起動を試みます。\nゲーム再起動後反映されます。");
      if(conf.fullscreen) { tb->setButtonState(sgui::Button::DOWN); }



      base = sgui::Point(110, 30);
      tb = new sgui::ToggleButton(this, base, sgui::Size(120, 20), L"VSync", BU_VSYNC);
      new sgui::ToolTip(tb, L"画面の更新間隔をモニタのリフレッシュレートに合わせ、\n滑らかに画面を更新します。\nリフレッシュレートが60Hzの場合や\nフルスクリーンモードの場合に効果的です。\nゲーム再起動後反映されます。");
      if(conf.vsync) { tb->setButtonState(sgui::Button::DOWN); }

      base+=sgui::Point(0, 25);
      tb = new sgui::ToggleButton(this, base, sgui::Size(120, 20), L"フレームスキップ", BU_FRAMESKIP);
      new sgui::ToolTip(tb, L"処理落ちしたとき、描画をスキップして\nゲームの進行速度をある程度維持します。");
      if(conf.frameskip) { tb->setButtonState(sgui::Button::DOWN); }

      base+=sgui::Point(0, 25);
      tb = new sgui::ToggleButton(this, base, sgui::Size(120, 20), L"頂点バッファ", BU_VERTEX_BUFFER);
      new sgui::ToolTip(tb, L"対応していれば頂点バッファを使用します。\n描画の若干の高速化が期待できます。\nゲーム再起動後反映されます。");
      if(conf.vertex_buffer) { tb->setButtonState(sgui::Button::DOWN); }

      base+=sgui::Point(0, 25);
      tb = new sgui::ToggleButton(this, base, sgui::Size(120, 20), L"追加光源", BU_EXLIGHT);
      new sgui::ToolTip(tb, L"敵破壊時等に光源を設置し、周囲を明るくします。\nオフにすると大分見た目が寂しくなってしまいますが、\n若干処理負荷が軽減します。");
      if(conf.exlight) { tb->setButtonState(sgui::Button::DOWN); }

      base+=sgui::Point(0, 25);
      tb = new sgui::ToggleButton(this, base, sgui::Size(120, 20), L"ブラー無効化", BU_NOBLUR);
      new sgui::ToolTip(tb, L"でかい敵を破壊したときなどに発生する、\n画面が歪む感じのエフェクト類を無効にします。\n若干処理負荷が軽減されますが、見た目が寂しくなります。");
      if(conf.noblur) { tb->setButtonState(sgui::Button::DOWN); }

      base+=sgui::Point(0, 25);
      tb = new sgui::ToggleButton(this, base, sgui::Size(120, 20), L"30FPS", BU_FPS_30);
      new sgui::ToolTip(tb, L"描画の頻度を半分にします。\nどうしても動作が重い場合有効にしてみてください。\nまた、これを有効にしている場合、\nVSyncの設定を無視してタイマーで待つようになります。");
      if(conf.fps_30) { tb->setButtonState(sgui::Button::DOWN); }


      base+=sgui::Point(0, 30);
      tb = new sgui::ToggleButton(this, base, sgui::Size(120, 20), L"シェーダ", BU_SHADER);
      new sgui::ToolTip(tb, L"対応していれば各種シェーダを使用します。\n発光エフェクトや動的テクスチャを使ったエフェクト等\nが有効になり、見た目が派手になりますが、\nその分処理負荷も大きくなります。");
      if(conf.shader) { tb->setButtonState(sgui::Button::DOWN); }

      base+=sgui::Point(0, 25);
      tb = new sgui::ToggleButton(this, base, sgui::Size(120, 20), L"背景簡略化", BU_SIMPLEBG);
      new sgui::ToolTip(tb, L"背景へのシェーダの使用を無効化します。\nシェーダ有効時の背景は処理負荷が高いので、\n動作が遅い場合まずはこれを変えてみるといいでしょう。\nシェーダを使わない設定の場合変化はありません。");
      if(conf.simplebg) { tb->setButtonState(sgui::Button::DOWN); }

      base+=sgui::Point(0, 25);
      new sgui::Label(this, base, sgui::Size(60, 20), L"ブルーム");
      swprintf(buf, L"%.1f", conf.bloom);
      b = new sgui::Button(this, base+sgui::Point(55, 0), sgui::Size(20, 20), L"<", BU_BLOOM_DEC);
      new sgui::ToolTip(b, L"シェーダ有効時の、明るい部分をぼかして\n眩しい感じにするエフェクトの強弱を設定します。\nシェーダを使わない設定の場合変化はありません。");
      m_bloom = new sgui::Label(this, base+sgui::Point(80, 0), sgui::Size(20, 20), buf);
      b = new sgui::Button(this, base+sgui::Point(100, 0), sgui::Size(20, 20), L">", BU_BLOOM_INC);
      new sgui::ToolTip(b, L"シェーダ有効時の、明るい部分をぼかして\n眩しい感じにするエフェクトの強弱を設定します。\nシェーダを使わない設定の場合変化はありません。");

      base+=sgui::Point(0, 30);
      new sgui::Label(this, base, sgui::Size(60, 20), L"並列処理");
      swprintf(buf, 32, L"%d", conf.threads);
      b = new sgui::Button(this, base+sgui::Point(55, 0), sgui::Size(20, 20), L"<", BU_THREAD_DEC);
      new sgui::ToolTip(b, L"内部処理を何スレッドに分けるかを指定します。\nデフォルト最適になるようにしていますが、\n動作が異常に重いとき等は調整してみると変化があるかもしれません。\n大抵はCPUのコア数と同数の時最も効果があります。");
      m_thread = new sgui::Label(this, base+sgui::Point(80, 0), sgui::Size(20, 20), buf);
      b = new sgui::Button(this, base+sgui::Point(100, 0), sgui::Size(20, 20), L">", BU_THREAD_INC);
      new sgui::ToolTip(b, L"内部処理を何スレッドに分けるかを指定します。\nデフォルト最適になるようにしていますが、\n動作が異常に重いとき等は調整してみると変化があるかもしれません。\n大抵はCPUのコア数と同数の時最も効果があります。");



      base = sgui::Point(250, 30);
      new sgui::Label(this, base, sgui::Size(100, 16), L"スコアネーム:");
      base+=sgui::Point(0, 16);
      sgui::Edit *ed = new sgui::Edit(this, base, sgui::Size(100, 16), _L(conf.scorename), ED_SCORENAME);

      base+=sgui::Point(0, 25);
      b = new sgui::Button(this, base, sgui::Size(70, 20), L"自機色", BU_COLOR);
      new sgui::ToolTip(b, L"自機の色をカスタマイズします。");
      m_color_sample = new sgui::Panel(this, base+sgui::Point(75, 0), sgui::Size(20,20));
      m_color_sample->setBackgroundColor(sgui::Color(conf.color.v));

      base+=sgui::Point(0, 25);
      tb = new sgui::ToggleButton(this, base, sgui::Size(100, 20), L"自動アップデート", BU_AUTOUPDATE);
      new sgui::ToolTip(tb, L"ゲーム起動時にWeb経由で最新パッチの有無を調べます。");
      if(conf.update) { tb->setButtonState(sgui::Button::DOWN); }

      base+=sgui::Point(0, 25);
      tb = new sgui::ToggleButton(this, base, sgui::Size(100, 20), L"FPS表示", BU_SHOW_FPS);
      new sgui::ToolTip(tb, L"フレームレートを表示します。");
      if(conf.show_fps) { tb->setButtonState(sgui::Button::DOWN); }

      base+=sgui::Point(0, 25);
      tb = new sgui::ToggleButton(this, base, sgui::Size(100, 20), L"OBJ数表示", BU_SHOW_OBJ);
      new sgui::ToolTip(tb, L"ゲーム内のオブジェクトの数を表示します。");
      if(conf.show_obj) { tb->setButtonState(sgui::Button::DOWN); }

      base+=sgui::Point(0, 25);
      tb = new sgui::ToggleButton(this, base, sgui::Size(30, 20), L"BGM", BU_BGM);
      if(!conf.bgm_mute) { tb->setButtonState(sgui::Button::DOWN); }
      b = new sgui::Button(this, base+sgui::Point(35, 0), sgui::Size(20, 20), L"<", BU_BGM_VOLUME_DOWN);
      b = new sgui::Button(this, base+sgui::Point(80, 0), sgui::Size(20, 20), L">", BU_BGM_VOLUME_UP);
      swprintf(buf, 32, L"%d", conf.bgm_volume);
      m_bgm_vol = new sgui::Label(this, base+sgui::Point(60, 0), sgui::Size(30, 20), buf);

      base+=sgui::Point(0, 25);
      tb = new sgui::ToggleButton(this, base, sgui::Size(30, 20), L"SE", BU_SE);
      if(!conf.se_mute) { tb->setButtonState(sgui::Button::DOWN); }
      b = new sgui::Button(this, base+sgui::Point(35, 0), sgui::Size(20, 20), L"<", BU_SE_VOLUME_DOWN);
      b = new sgui::Button(this, base+sgui::Point(80, 0), sgui::Size(20, 20), L">", BU_SE_VOLUME_UP);
      swprintf(buf, 32, L"%d", conf.se_volume);
      m_se_vol = new sgui::Label(this, base+sgui::Point(60, 0), sgui::Size(30, 20), buf);

      toTopLevel();
    }

    ~ConfigDialog()
    {
      g_config_dialog = 0;
    }

    bool handleEvent(const sgui::Event& evt)
    {
      size_t id = evt.getSrc() ? evt.getSrc()->getID() : 0;
      Config& conf = *dynamic_cast<Config*>(GetConfig());

      char buf[32];
      if(evt.getType()==sgui::EVT_BUTTON_UP) {
        if(id==BU_FULLSCREEN)        { conf.fullscreen=false; return true; }
        else if(id==BU_VSYNC)        { conf.vsync=false; return true; }
        else if(id==BU_FRAMESKIP)    { conf.frameskip=false; return true; }
        else if(id==BU_EXLIGHT)      { conf.exlight=false; return true; }
        else if(id==BU_SHADER)       { conf.shader=false; return true; }
        else if(id==BU_VERTEX_BUFFER){ conf.vertex_buffer=false; return true; }
        else if(id==BU_SIMPLEBG)     { conf.simplebg=false; return true; }
        else if(id==BU_FPS_30)       { conf.fps_30=false; return true; }
        else if(id==BU_NOBLUR)       { conf.noblur = false; return true; }
        else if(id==BU_AUTOUPDATE)   { conf.update=false; return true; }
        else if(id==BU_SHOW_FPS)     { conf.show_fps=false; return true; }
        else if(id==BU_SHOW_OBJ)     { conf.show_obj=false; return true; }
        else if(id==BU_SE) {
          conf.se_mute = true;
          Mix_Volume(-1, 0);
          return true;
        }
        else if(id==BU_BGM) {
          conf.bgm_mute = true;
          Mix_VolumeMusic(0);
          return true;
        }
        else if(id==BU_BGM_VOLUME_UP) {
          conf.bgm_volume = std::max<int>(0, std::min<int>(128, conf.bgm_volume+16));
          sprintf(buf, "%d", conf.bgm_volume);
          m_bgm_vol->setText(_L(buf));
          if(conf.sound && !conf.bgm_mute) {
            Mix_VolumeMusic(conf.bgm_volume);
          }
        }
        else if(id==BU_BGM_VOLUME_DOWN) {
          conf.bgm_volume = std::max<int>(0, std::min<int>(128, conf.bgm_volume-16));
          sprintf(buf, "%d", conf.bgm_volume);
          m_bgm_vol->setText(_L(buf));
          if(conf.sound && !conf.bgm_mute) {
            Mix_VolumeMusic(conf.bgm_volume);
          }
        }
        else if(id==BU_SE_VOLUME_UP) {
          conf.se_volume = std::max<int>(0, std::min<int>(128, conf.se_volume+16));
          sprintf(buf, "%d", conf.se_volume);
          m_se_vol->setText(_L(buf));
          if(conf.sound && !conf.se_mute) {
            Mix_Volume(-1, conf.se_volume);
          }
        }
        else if(id==BU_SE_VOLUME_DOWN) {
          conf.se_volume = std::max<int>(0, std::min<int>(128, conf.se_volume-16));
          sprintf(buf, "%d", conf.se_volume);
          m_se_vol->setText(_L(buf));
          if(conf.sound && !conf.se_mute) {
            Mix_Volume(-1, conf.se_volume);
          }
        }
        else if(id==BU_THREAD_DEC) {
          if(conf.threads > 1) {
            conf.threads--;
            sprintf(buf, "%d", conf.threads);
            m_thread->setText(_L(buf));
            dynamic_cast<App&>(*App::instance()).updateScheduler();
          }
          return true;
        }
        else if(id==BU_THREAD_INC) {
          if(conf.threads < 8) {
            conf.threads++;
            char buf[16];
            sprintf(buf, "%d", conf.threads);
            m_thread->setText(_L(buf));
            dynamic_cast<App&>(*App::instance()).updateScheduler();
          }
          return true;
        }
        else if(id==BU_BLOOM_DEC) {
          conf.bloom = std::max<float>(0.0f, conf.bloom-0.1f);
          sprintf(buf, "%.1f", conf.bloom);
          m_bloom->setText(_L(buf));
          return true;
        }
        else if(id==BU_BLOOM_INC) {
          conf.bloom = std::min<float>(1.0f, conf.bloom+0.1f);
          sprintf(buf, "%.1f", conf.bloom);
          m_bloom->setText(_L(buf));
          return true;
        }
        else if(id==BU_COLOR) {
          sgui::ColorDialog *dlg = new sgui::ColorDialog(
            g_widget_window, sgui::Point(getPosition()+sgui::Point(250,90)), DL_COLOR);
          dlg->setColor(sgui::Color(GetConfig()->color.v));
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_BUTTON_DOWN) {
        if(id==BU_FULLSCREEN)     { conf.fullscreen=true; return true; }
        else if(id==BU_VSYNC)     { conf.vsync=true; return true; }
        else if(id==BU_FRAMESKIP) { conf.frameskip=true; return true; }
        else if(id==BU_FPS_30)    { conf.fps_30=true; return true; }
        else if(id==BU_SIMPLEBG)  { conf.simplebg=true; return true; }
        else if(id==BU_EXLIGHT)   { conf.exlight=true; return true; }
        else if(id==BU_NOBLUR)    { conf.noblur=true; return true; }
        else if(id==BU_AUTOUPDATE){ conf.update=true; return true; }
        else if(id==BU_SHOW_FPS)  { conf.show_fps=true; return true; }
        else if(id==BU_SHOW_OBJ)  { conf.show_obj=true; return true; }
        else if(id==BU_VERTEX_BUFFER) {
          sgui::ToggleButton *tb = static_cast<sgui::ToggleButton*>(evt.getSrc());
          if(!IsVBOAvailable()) {
            new sgui::MessageDialog(
              L"error",
              L"未対応の環境のようです。\nこの機能を使用するにはOpenGL1.5に対応している必要があります。",
              g_widget_window, sgui::Point(50,50), sgui::Size(300, 150));
            tb->setButtonState(sgui::Button::UP);
          }
          else {
            conf.vertex_buffer = true;
          }
          return true;
        }
        else if(id==BU_SHADER) {
          sgui::ToggleButton *tb = static_cast<sgui::ToggleButton*>(evt.getSrc());
          if(!IsGLSLAvailable()) {
            new sgui::MessageDialog(
              L"error",
              L"未対応の環境のようです。\nこの機能を使用するにはOpenGL2.0およびGL_EXT_framebuffer_object拡張に対応している必要があります。",
              g_widget_window, sgui::Point(50,50), sgui::Size(300, 150));
            tb->setButtonState(sgui::Button::UP);
          }
          else if(SDL_GetVideoInfo()->vfmt->BitsPerPixel!=32) {
            new sgui::MessageDialog(
              L"warning",
              L"画面の色が32bitではありません。\n32bitカラーでない状態でシェーダを使う場合、極端に動作が遅くなる可能性があります。\n32bitカラーにするか、フルスクリーンモードにすることで対処可能です。",
              g_widget_window, sgui::Point(50,50), sgui::Size(300, 150));
            conf.shader = true;
          }
          else {
            conf.shader = true;
          }
          return true;
        }
        else if(id==BU_SE) {
          conf.se_mute = false;
          Mix_Volume(-1, conf.se_volume);
          return true;
        }
        else if(id==BU_BGM) {
          conf.bgm_mute = false;
          Mix_VolumeMusic(conf.bgm_volume);
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_LIST_SELECT ||
              evt.getType()==sgui::EVT_LIST_MOVE_SELECTION) {
        const sgui::ListEvent& e = dynamic_cast<const sgui::ListEvent&>(evt);
        if(id==LI_RESOLUTION) {
          int w,h;
          string text = _S(e.getItem()->getText());
          if(sscanf(text.c_str(), "%dx%d", &w, &h)==2) {
            conf.width = w;
            conf.height = h;
            return true;
          }
        }
      }
      else if(evt.getType()==sgui::EVT_EDIT) {
        const sgui::EditEvent& e = dynamic_cast<const sgui::EditEvent&>(evt);
        if(id==ED_SCORENAME && !g_game_panel) {
          conf.scorename = _S(e.getString());
          conf.checkScorename();
          e.getSrc()->setText(_L(conf.scorename));
        }
      }
      else if(evt.getType()==sgui::EVT_COLORDIALOG) {
        const sgui::DialogEvent& e = dynamic_cast<const sgui::DialogEvent&>(evt);
        if(id==DL_COLOR && e.isOK()) {
          sgui::Color c = e.getColor();
          conf.color = vector4(c.getR(), c.getG(), c.getB());
          m_color_sample->setBackgroundColor(c);
          return true;
        }
      }
      return Super::handleEvent(evt);
    }
  };


  class ServerConfigDialog : public sgui::Dialog
  {
  typedef sgui::Dialog Super;
  private:
    sgui::ToggleButton *m_enable_pause;
    sgui::ToggleButton *m_level[5];
    sgui::Combo *m_max_connection;

  public:
    ServerConfigDialog() :
      Super(g_backwidget_window, sgui::Point(425,65), sgui::Size(205,160), L"server config", DL_STATE)
    {
      if(g_server_config_dialog) {
        throw Error("ServerConfigDialog::ServerConfigDialog()");
      }
      g_server_config_dialog = this;

      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_BUTTON_DOWN);
      listenEvent(sgui::EVT_EDIT_ENTER);
      listenEvent(sgui::EVT_COMBO);

      IConfig& conf = *GetConfig();
      sgui::Window *w;

      m_enable_pause = new sgui::ToggleButton(w, sgui::Point(0,25), sgui::Size(95,20), L"enable pause");
      new sgui::ToolTip(m_enable_pause, L"ポーズの許可。");
      m_enable_pause->setButtonState(conf.server_allow_pause ? sgui::Button::DOWN : sgui::Button::UP);

      new sgui::Label(w, sgui::Point(0,50), sgui::Size(70,16), L"max players");
      m_max_connection = new sgui::Combo(w, sgui::Point(65,50), sgui::Size(30,16));
      new sgui::ToolTip(m_max_connection, L"最大同時プレイ人数。");
      for(int i=2; i<=16; ++i) {
        wchar_t buf[8];
        swprintf(buf, 8, L"%d", i);
        m_max_connection->addItem(new sgui::ListItem(buf));
        if(i==conf.server_max_connection) {
          m_max_connection->setSelection(m_max_connection->getList()->getItemCount()-1);
        }
      }

      toTopLevel();
    }

    ~ServerConfigDialog()
    {
      g_server_config_dialog = 0;
    }

    bool handleEvent(const sgui::Event& evt)
    {
      size_t id = evt.getSrc() ? evt.getSrc()->getID() : 0;
      IConfig& conf = *GetConfig();
      if(evt.getType()==sgui::EVT_BUTTON_DOWN) {
        if(evt.getSrc()==m_enable_pause) {
          conf.server_allow_pause = true;
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_BUTTON_UP) {
        if(evt.getSrc()==m_enable_pause) {
          conf.server_allow_pause = false;
          return true;
        }
        else if(id>=BU_SERVER_LIGHT && id<=BU_SERVER_FUTURE) {
          dynamic_cast<sgui::ToggleButton*>(evt.getSrc())->setButtonState(sgui::Button::DOWN);
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_COMBO) {
        const sgui::ListEvent& e = dynamic_cast<const sgui::ListEvent&>(evt);
        if(evt.getSrc()==m_max_connection) {
          int max_connection = 0;
          if(swscanf(e.getItem()->getText().c_str(), L"%d", &max_connection)==1) {
            conf.server_max_connection = max_connection;
          }
          return true;
        }
      }

      return Super::handleEvent(evt);
    }
  };



  class FadeoutWindow : public sgui::Window
  {
  typedef sgui::Window Super;
  private:
    int m_fadetime;

  public:
    FadeoutWindow(sgui::Window *parent) :
      Super(parent, sgui::Point(), parent->getSize()), m_fadetime(60)
    {
      if(g_fadeout_window) {
        throw Error("FadeWindow::FadeWindow()");
      }
      g_fadeout_window = this;

      setBackgroundColor(sgui::Color(0,0,0,0));
      setDrawPriority(1.5f);

      IMusic::Resume();
    }

    ~FadeoutWindow()
    {
      g_fadeout_window = 0;
    }

    void setFadeTime(int frame) { m_fadetime=frame; }

    virtual void action()=0;

    void update()
    {
      Super::update();
      sgui::Color c = getBackgroundColor();
      c.a+=(1.0f/m_fadetime);
      setBackgroundColor(c);

      if(c.a>1.0f) {
        action();
      }
    }

    void draw()
    {
      glColor4fv(getBackgroundColor().v);
      sgui::DrawRect(sgui::Rect(sgui::Point(-1,-1), getSize()+sgui::Size(2,2)));
      glColor4f(1,1,1,1);
    }
  };


  class FadeToGameWindow : public FadeoutWindow
  {
  typedef FadeoutWindow Super;
  private:
    GameOption m_opt;
    string m_replay;

  public:
    FadeToGameWindow(sgui::Window *parent, const GameOption& opt) : Super(parent), m_opt(opt)
    {
      setFadeTime(60);
      IMusic::FadeOut(900);
    }

    FadeToGameWindow(sgui::Window *parent, const string r) : Super(parent), m_replay(r)
    {
      setFadeTime(60);
      IMusic::FadeOut(900);
    }

    void read(const string& path)
    {
      ist::gzbstream s(path, "rb");
      if(!s) {
        throw Error("リプレイデータを開けませんでした。");
      }

      int version = 0;
      size_t fps;
      size_t playtime;

      s >> version;
      if(version!=EXCEPTION_REPLAY_VERSION) {
        throw Error("不正な形式のリプレイデータです。");
      }

      s >> m_opt >> fps >> playtime;
      g_iserver.reset();
      g_iclient.reset();
      g_iclient.reset(new InputClientReplay(s, version));
    }

    void action()
    {
      try {
        IMusic::WaitFadeOut();

        if(!m_replay.empty()) {
          read(m_replay);
          if(m_replay=="record/ranking.tmp") {
            remove(m_replay.c_str());
          }
        }
        else if(!g_iclient) {
          g_iclient.reset(new InputClientLocal());
        }
        else if(InputClientIP* icip = dynamic_cast<InputClientIP*>(g_iclient.get())) {
          if(!icip->isRunning()) {
            throw Error("disconnected");
          }
        }
        IGame *game = CreateGame(m_opt);
        CreateGamePanel(game);

        getParent()->destroy();
      }
      catch(const Error& e) {
        destroy();
        show_error(e.what());
      }
    }
  };

  class FadeToTitleWindow : public FadeoutWindow
  {
  typedef FadeoutWindow Super;
  public:
    FadeToTitleWindow(sgui::Window *parent) : Super(parent)
    {
      setFadeTime(60);
      IMusic::FadeOut(900);
    }

    void action()
    {
      IMusic::WaitFadeOut();
      getParent()->destroy();
      CreateTitlePanel();
    }
  };

  class FadeToExitPanel : public FadeoutWindow
  {
  typedef FadeoutWindow Super;
  public:
    FadeToExitPanel(sgui::Window *parent) : Super(parent)
    {
      setFadeTime(60);
      IMusic::FadeOut(900);
    }

    void action()
    {
      IMusic::WaitFadeOut();
      sgui::App::instance()->exit();
    }
  };






  class RecordItem : public sgui::ListItem
  {
  typedef sgui::ListItem Super;
  public:
    string file;
    string name;
    int level;
    int stage;
    int fps;
    int players;
    int playtime;

    RecordItem() :
      Super(L""), level(0), stage(0), fps(0), players(0), playtime(0)
    {}

    void setToolTip()
    {
      char buf[256];
      sprintf(buf,
        "name: %s\n"
        "mode: %s\n"
        "stage: %s\n"
        "time: %dm"
        "%02ds\n"
        "avg fps:%d\n"
        "filename:%s",
        name.c_str(), GetLevelString(level).c_str(), GetMapString(stage).c_str(),
        int(playtime/3600), int(playtime%3600/60), fps, file.c_str());
      new sgui::ToolTip(this, _L(buf));
    }

    void genText()
    {
      char buf[256];
      sprintf(buf, "%s", name.c_str());
      setText(_L(buf));
    }

    void genDetailedText()
    {
      char buf[256];
      sprintf(buf, "%s %s %s", name.c_str(), GetLevelString(level).c_str(), GetMapString(stage).c_str());
      setText(_L(buf));
    }
  };

  class RecordDialog : public sgui::Dialog
  {
  typedef sgui::Dialog Super;
  public:

    class Loader : public Thread
    {
    private:
      RecordDialog *m_rd;
      bool m_stopped;

    public:
      Loader(RecordDialog *rd) : m_rd(rd), m_stopped(false)
      {}

      ~Loader()
      {
        stop();
        join();
      }

      void stop()
      {
        m_stopped = true;
      }

      void exec()
      {
        ist::Dir dir("record");
        for(size_t i=0; i<dir.size(); ++i) {
          m_rd->loadItem(dir[i]);
          if(m_stopped) {
            break;
          }
        }
      }
    };

  private:
    boost::mutex m_mutex;
    thread_ptr m_loader;
    std::vector<RecordItem*> m_items;
    sgui::List *m_list;

  public:
    RecordDialog() :
      Super(g_widget_window, sgui::Point(15,15), sgui::Size(210,325), L"record", DL_RECORD)
    {
      if(g_record_dialog) {
        throw Error("RecordDialog::RecordDialog()");
      }
      g_record_dialog = this;

      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_LIST_DOUBLECLICK);
      listenEvent(sgui::EVT_CONFIRMDIALOG);

      sgui::Window *w;
      w = new sgui::Window(this, sgui::Point(5, 30), sgui::Size(200,290));

      new sgui::Label(w, sgui::Point(0,  0), sgui::Size(100,60), L"local data");
      new sgui::Button(w, sgui::Point(0, 20), sgui::Size(60,20), L"play", BU_RECORD_OPEN);
      new sgui::Button(w, sgui::Point(130, 20), sgui::Size(60,20), L"delete", BU_RECORD_DELETE);
      m_list = new sgui::List(w, sgui::Point(0, 45), sgui::Size(200,245), LI_RECORD);

      m_loader.reset(new Loader(this));
      m_loader->run();

      toTopLevel();
    }

    ~RecordDialog()
    {
      g_record_dialog = 0;

      m_loader.reset();
      for(int i=0; i<m_items.size(); ++i) {
        m_items[i]->release();
      }
    }

    void loadItem(const string& filename)
    {
      boost::mutex::scoped_lock lock(m_mutex);

      ist::gzbstream f(string("record/")+filename, "rb");
      if(!f) {
        return;
      }
      int version = 0;
      GameOption opt;
      int fps = 0;
      size_t players = 0;
      string name;
      size_t playtime = 0;

      f >> version;
      if(version!=EXCEPTION_REPLAY_VERSION) { // バージョンが合ってるもののみリスト 
        return;
      }
      f >> opt >> fps >> playtime >> players >> name;

      {
        RecordItem * ri = new RecordItem();
        ri->file = filename;
        ri->name = name;
        ri->level = opt.level;
        ri->stage = opt.stage;
        ri->fps = fps;
        ri->players = players;
        ri->playtime = playtime;
        ri->genDetailedText();
        ri->setToolTip();
        m_items.push_back(ri);
      }
    }

    void update()
    {
      Super::update();

      boost::mutex::scoped_lock lock(m_mutex);
      for(int i=0; i<m_items.size(); ++i) {
        m_list->addItem(m_items[i]);
      }
      m_items.clear();
    }

    void start(const string& record)
    {
      FadeToGame(string("record/")+record);
    }

    bool handleEvent(const sgui::Event& evt)
    {
      size_t id = evt.getSrc() ? evt.getSrc()->getID() : 0;

      if(evt.getType()==sgui::EVT_BUTTON_UP) {
        if(id==BU_RECORD_OPEN) {
          if(RecordItem *ri = dynamic_cast<RecordItem*>(m_list->getSelectedItem())) {
            start(ri->file);
          }
          return true;
        }
        else if(id==BU_RECORD_DELETE) {
          if(m_list->getSelectionCount()>0) {
            new sgui::ConfirmDialog(L"delete", L"選択されたデータを消去します。\nよろしいですか？",
              BU_RECORD_DELETE_CONFIRM, g_widget_window, sgui::Point(20, 50));
          }
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_CONFIRMDIALOG) {
        const sgui::DialogEvent& e = dynamic_cast<const sgui::DialogEvent&>(evt);
        if(id==BU_RECORD_DELETE_CONFIRM && e.isOK()) {
          if(RecordItem *ri = dynamic_cast<RecordItem*>(m_list->getSelectedItem())) {
            remove((string("record/")+ri->file).c_str());
            m_list->removeItem(ri);
          }
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_LIST_DOUBLECLICK) {
        const sgui::ListEvent& e = dynamic_cast<const sgui::ListEvent&>(evt);
        if(id==LI_RECORD) {
          if(RecordItem *ri = dynamic_cast<RecordItem*>(e.getItem())) {
            start(ri->file);
          }
          return true;
        }
      }

      return Super::handleEvent(evt);
    }
  };




  class ServerListItem : public sgui::ListItem
  {
  typedef sgui::ListItem Super;
  public:
    string ip;
    int port;
    string name;

    ServerListItem(const string& _ip, int _port, const string& _name, const string& comment) :
      Super(L""), ip(_ip), port(_port), name(_name)
    {
      if(!comment.empty()) {
        name+=": ";
        name+=comment;
      }
      setText(_L(name));
    }

    string getHTTPAddress()
    {
      return string("http://"+ip+":"+lexical_cast<string>(port)+"/");
    }
  };



  class StartDialog : public sgui::Dialog
  {
  typedef sgui::Dialog Super;
  private:

#ifdef EXCEPTION_ENABLE_NETPLAY
    class ConnectThread : public Thread
    {
    private:
      typedef shared_ptr<InputClientIP> client_ptr;
      client_ptr m_client;
      string m_server;
      ushort m_port;
      string m_error;
      bool m_connected;

    public:
      ConnectThread(const client_ptr& c, const string& server, ushort port) :
        m_client(c), m_server(server), m_port(port), m_connected(false)
      {}

      ~ConnectThread()
      {
        join();
      }

      bool isConnected() { return m_connected; }
      const string& getError() { return m_error; }

      void exec()
      {
        try {
          m_client->connect(m_server, m_port);
          m_connected = true;
        }
        catch(std::exception& e) {
          m_error = e.what();
        }
      }

      void runClient()
      {
        m_client->run();
      }
    };
    typedef shared_ptr<ConnectThread> connect_thread_ptr;

    connect_thread_ptr m_connect_thread;
    httpasync_ptr m_http_serverlist;
    httpasync_ptr m_http_serverlist_checker;
#endif

    static const int s_num_maputton = 3;

    sgui::Window *m_network_panel;
    sgui::Window *m_server_panel;
    sgui::Window *m_client_panel;
    sgui::Window *m_game_panel;
    sgui::Window *m_game_extra_panel;

    sgui::Window *m_horde_panel;
    sgui::Window *m_deathmatch_panel;
    sgui::Window *m_teamfortress_panel;

    sgui::ToggleButton *m_netb[3];
    sgui::ToggleButton *m_level[5];
    sgui::ToggleButton *m_map[s_num_maputton];

    sgui::ToggleButton *m_extra;
    sgui::Edit *m_cboost;
    sgui::Edit *m_fboost;
    sgui::Edit *m_eboost;

    sgui::Button *m_startbutton;
    sgui::Button *m_browsebutton;;

    sgui::Edit *m_serveraddress;
    sgui::Edit *m_serverport;
    sgui::List *m_serverlist;
    sgui::Label *m_connecting;

    sgui::Combo *m_horde_wave;
    sgui::Combo *m_deathmatch_time;
    sgui::Combo *m_teamfortress_life;

    GameOption m_opt;

  public:
    StartDialog() :
        Super(g_widget_window, sgui::Point(140,100), sgui::Size(360,240), L"start", DL_START)
    {
      if(g_start_dialog) {
        throw Error("StartDialog::StartDialog()");
      }
      g_start_dialog = this;

      listenEvent(sgui::EVT_CONSTRUCT);
      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_COMBO);
      listenEvent(sgui::EVT_LIST_DOUBLECLICK);
      listenEvent(sgui::EVT_EDIT_ENTER);

      sgui::Point pos(5, 0);

      // ネットワーク選択 
      m_network_panel = new sgui::Window(this, sgui::Point(0, 30), sgui::Size(60, 200));
#ifdef EXCEPTION_ENABLE_NETPLAY
      new sgui::Label(m_network_panel, pos, sgui::Size(100, 20), L"network");
      pos+=sgui::Point(0, 20);
      m_netb[0] = new sgui::ToggleButton(m_network_panel, pos, sgui::Size(50,20), L"server", BU_START_SERVER);
      pos+=sgui::Point(0, 25);
      m_netb[1] = new sgui::ToggleButton(m_network_panel, pos, sgui::Size(50,20), L"client", BU_START_CLIENT);
      pos+=sgui::Point(0, 25);
      m_netb[2] = new sgui::ToggleButton(m_network_panel, pos, sgui::Size(50,20), L"local", BU_START_LOCAL);
      pos+=sgui::Point(0, 40);
      m_browsebutton = new sgui::Button(m_network_panel, pos, sgui::Size(50,16), L"browse", BU_START_BLOWSE);
#endif

      m_server_panel = new sgui::Window(this, sgui::Point(0, 30), sgui::Size(360, 200));
      m_server_panel->setVisible(true);
#ifdef EXCEPTION_ENABLE_NETPLAY
#endif

      // サーバーリスト/アドレス入力欄 
      m_client_panel = new sgui::Window(this, sgui::Point(60, 30), sgui::Size(300, 200));
      m_client_panel->setVisible(false);
#ifdef EXCEPTION_ENABLE_NETPLAY
      new sgui::Label(m_client_panel, sgui::Point(  5, 0), sgui::Size(190, 20), L"server list");
      m_serverlist = new sgui::List(m_client_panel, sgui::Point(  5,20), sgui::Size(290, 55), LI_START_SERVERLIST);
      m_serverlist->setBackgroundColor(sgui::Color(0,0,0,0));
      new sgui::Button(m_client_panel,sgui::Point(170,80), sgui::Size(60, 16), L"update", BU_START_UPDATELIST);
      new sgui::Button(m_client_panel,sgui::Point(235,80), sgui::Size(60, 16), L"connect", BU_START_CONNECTLIST);

      new sgui::Label(m_client_panel, sgui::Point(  5,110), sgui::Size(100, 20), L"address");
      new sgui::Label(m_client_panel, sgui::Point(120,110), sgui::Size( 40, 20), L"port");
      m_serveraddress = new sgui::Edit(m_client_panel,  sgui::Point(5,130), sgui::Size(100, 16), _L(GetConfig()->last_server), ED_START_ADDRESS);
      m_serverport = new sgui::Edit(m_client_panel,  sgui::Point(110,130), sgui::Size( 40, 16), lexical_cast<wstring>(GetConfig()->last_port), ED_START_PORT);
      new sgui::Button(m_client_panel,sgui::Point(155,130), sgui::Size(60, 16), L"connect", BU_START_CONNECT);
      m_connecting = new sgui::Label(m_client_panel, sgui::Point(5,160), sgui::Size(200, 20), L"");
#endif


      // 難度/ステージ選択 
      m_game_panel = new sgui::Window(this, sgui::Point(60, 30), sgui::Size(300, 200));
      pos = sgui::Point(5, 0);
      new sgui::Label(m_game_panel, pos, sgui::Size(100, 20), L"mode");

      for(int i=0; i<5; ++i) {
        m_level[i] = 0;
      }

      pos+=sgui::Point(0, 20);

      m_level[0] = new sgui::ToggleButton(m_game_panel, pos, sgui::Size(90,16), L"light", BU_START_LIGHT);
      new sgui::ToolTip(m_level[0], L"初期破片倍率1.0\n少々画面が寂しいモードです。");
      pos+=sgui::Point(0, 20);

      m_level[1] = new sgui::ToggleButton(m_game_panel, pos, sgui::Size(90,16), L"normal", BU_START_NORMAL);
      new sgui::ToolTip(m_level[1], L"初期破片倍率1.8\nそこそこ画面が賑やかなモードです。");
      pos+=sgui::Point(0, 20);

      m_level[2] = new sgui::ToggleButton(m_game_panel, pos, sgui::Size(90,16), L"heavy", BU_START_HEAVY);
      m_level[2]->setButtonState(sgui::Button::DOWN);
      new sgui::ToolTip(m_level[2], L"初期破片倍率3.2\n適度に画面が賑やかなモードです。");
      pos+=sgui::Point(0, 20);

      m_level[3] = new sgui::ToggleButton(m_game_panel, pos, sgui::Size(90,16), L"excess", BU_START_EXCESS);
      new sgui::ToolTip(m_level[3], L"初期破片倍率4.8\n必要以上に画面が賑やかなモードです。");
      pos+=sgui::Point(0, 20);

      m_level[4] = new sgui::ToggleButton(m_game_panel, pos, sgui::Size(90,16), L"future", BU_START_FUTURE);
      new sgui::ToolTip(m_level[4], L"初期破片倍率9.0\n2年後くらいの最新マシンで快適に遊べると思います。");


      pos+=sgui::Point(0, 40);
      m_extra = new sgui::ToggleButton(m_game_panel, pos+sgui::Point(0,20), sgui::Size(35,16), L"extra");
      m_game_extra_panel = new sgui::Window(m_game_panel, pos+sgui::Point(40,0), sgui::Size(200,100));
      pos = sgui::Point(0, 0);
      new sgui::Label(m_game_extra_panel, pos, sgui::Size(90, 16), L"catapult boost");
      m_cboost = new sgui::Edit(m_game_extra_panel, pos+sgui::Point(90,0), sgui::Size(30,18), L"1.0");
      new sgui::ToolTip(m_cboost, L"カタパルトのゲージ回復速度の倍率。\n有効範囲 0.0～3.0");
      pos+=sgui::Point(0, 20);
      new sgui::Label(m_game_extra_panel, pos, sgui::Size(90, 16), L"fraction boost");
      m_fboost = new sgui::Edit(m_game_extra_panel, pos+sgui::Point(90,0), sgui::Size(30,18), L"1.0");
      new sgui::ToolTip(m_fboost, L"破片倍率。\nモード別倍率*boost値が破片出現量となります。\n有効範囲 0.0～2.0");
      pos+=sgui::Point(0, 20);
      new sgui::Label(m_game_extra_panel, pos, sgui::Size(90, 16), L"   enemy boost");
      m_eboost = new sgui::Edit(m_game_extra_panel, pos+sgui::Point(90,0), sgui::Size(30,18), L"1.0");
      new sgui::ToolTip(m_eboost, L"敵の耐久力の倍率。\n有効範囲 0.1～3.0");
      m_game_extra_panel->setVisible(false);


      pos = sgui::Point(100, 0);
      new sgui::Label(m_game_panel, pos, sgui::Size(100, 20), L"map");
      pos+=sgui::Point(0, 20);

      m_map[0] = new sgui::ToggleButton(m_game_panel, pos, sgui::Size(110,20), L"horde", BU_START_MAP_HORDE);
      m_map[0]->setButtonState(sgui::Button::DOWN);
      new sgui::ToolTip(m_map[0], L"全プレイヤーで協力し、\n押し寄せる敵からひたすら耐えるマップです。");
      pos+=sgui::Point(0, 25);

      m_map[1] = new sgui::ToggleButton(m_game_panel, pos, sgui::Size(110,20), L"deathmatch", BU_START_MAP_DEATHMATCH);
      new sgui::ToolTip(m_map[1], L"個人戦。\nプレイヤー毎のkill数を競い合うマップです。");
      pos+=sgui::Point(0, 25);

      m_map[2] = new sgui::ToggleButton(m_game_panel, pos, sgui::Size(110,20), L"team fortress", BU_START_MAP_TEAMFORTRESS);
      new sgui::ToolTip(m_map[2], L"チームバトル。\nチーム毎に要塞が用意され、\n相手チームの要塞のコアを破壊することで\n勝利となります。");
      pos+=sgui::Point(0, 25);

      m_startbutton = new sgui::Button(m_game_panel, sgui::Point(220,180), sgui::Size(70,20), L"start", BU_START_START);


      // horde用設定 
      m_horde_panel = new sgui::Window(m_game_panel, sgui::Point(210, 0), sgui::Size(180, 200));
      pos = sgui::Point(10, 0);
      new sgui::Label(m_horde_panel, pos, sgui::Size(100, 20), L"wave");
      pos+=sgui::Point(0, 20);
      m_horde_wave = new sgui::Combo(m_horde_panel, pos, sgui::Size(50,16), CB_START_HORDE_WAVE);
      for(int i=1; i<=30; ++i) {
        wchar_t buf[8];
        swprintf(buf, 8, L"%d", i);
        m_horde_wave->addItem(new sgui::ListItem(buf));
      }
      m_horde_wave->getList()->setSelection(size_t(0));


      // deathmatch用設定 
      m_deathmatch_panel = new sgui::Window(m_game_panel, sgui::Point(210, 0), sgui::Size(180, 200));
      pos = sgui::Point(10, 0);
      new sgui::Label(m_deathmatch_panel, pos, sgui::Size(100, 20), L"time");
      pos+=sgui::Point(0, 20);
      m_deathmatch_time = new sgui::Combo(m_deathmatch_panel, pos, sgui::Size(50,16), CB_START_DEATHMATCH_TIME);
      m_deathmatch_time->addItem(new sgui::ListItem(L"3m"));
      m_deathmatch_time->addItem(new sgui::ListItem(L"5m"));
      m_deathmatch_time->addItem(new sgui::ListItem(L"8m"));
      m_deathmatch_time->getList()->setSelection(size_t(0));
      m_deathmatch_panel->setVisible(false);


      // team_fortress用設定 
      m_teamfortress_panel = new sgui::Window(m_game_panel, sgui::Point(210, 0), sgui::Size(180, 200));
      pos = sgui::Point(10, 0);
      new sgui::Label(m_teamfortress_panel, pos, sgui::Size(100, 20), L"core life");
      pos+=sgui::Point(0, 20);
      m_teamfortress_life = new sgui::Combo(m_teamfortress_panel, pos, sgui::Size(50,16), CB_START_TEAMFORTRESS_LIFE);
      m_teamfortress_life->addItem(new sgui::ListItem(L"x0.5"));
      m_teamfortress_life->addItem(new sgui::ListItem(L"x1.0"));
      m_teamfortress_life->addItem(new sgui::ListItem(L"x2.0"));
      m_teamfortress_life->addItem(new sgui::ListItem(L"x3.0"));
      m_teamfortress_life->addItem(new sgui::ListItem(L"x5.0"));
      m_teamfortress_life->setSelection(size_t(1));
      m_teamfortress_panel->setVisible(false);

      toTopLevel();

#ifdef EXCEPTION_ENABLE_NETPLAY
      int nmode = GetConfig()->last_network_mode;
      if(nmode==NETWORK_SERVER) {
        m_netb[0]->setButtonState(sgui::Button::DOWN);
        resetClientAndServer();
        startServer();
      }
      else if(nmode==NETWORK_CLIENT) {
        m_netb[1]->setButtonState(sgui::Button::DOWN);
        resetClientAndServer();
        loadServerList();
      }
      else {
        m_netb[2]->setButtonState(sgui::Button::DOWN);
        resetClientAndServer();
        g_iclient.reset(new InputClientLocal());
      }
      m_game_panel->setVisible(nmode!=NETWORK_CLIENT);
      m_server_panel->setVisible(nmode==NETWORK_SERVER);
      m_client_panel->setVisible(nmode==NETWORK_CLIENT);
#endif // EXCEPTION_ENABLE_NETPLAY 
    }

    ~StartDialog()
    {
      g_start_dialog = 0;
    }

    void assignConfig(const NMessage& m)
    {
      if(m.gconf.level>=0 && m.gconf.level<=LEVEL_FUTURE) {
        for(int i=0; i<=LEVEL_FUTURE; ++i) {
          m_level[i]->setButtonState(sgui::Button::UP);
        }
        m_opt.level = m.gconf.level;
        m_level[m_opt.level]->setButtonState(sgui::Button::DOWN);
      }
      if(m.gconf.stage>=0 && m.gconf.stage<=MAP_TEAMFORTRESS) {
        for(int i=0; i<=MAP_TEAMFORTRESS; ++i) {
          m_map[i]->setButtonState(sgui::Button::UP);
        }
        m_opt.stage = m.gconf.stage;
        m_map[m_opt.stage]->setButtonState(sgui::Button::DOWN);
        m_horde_panel->setVisible(m_opt.stage==MAP_HORDE);
        m_deathmatch_panel->setVisible(m_opt.stage==MAP_DEATHMATCH);
        m_teamfortress_panel->setVisible(m_opt.stage==MAP_TEAMFORTRESS);
      }

      wstring v;
      sgui::List *ls;

      v = sgui::Format(L"%d", m.gconf.horde_wave);
      ls = m_horde_wave->getList();
      for(int i=0; i<ls->getItemCount(); ++i) {
        if(ls->getItem(i)->getText()==v) {
          m_horde_wave->setSelection(i);
          m_opt.horde_wave = m.gconf.horde_wave;
          break;
        }
      }

      v = sgui::Format(L"%dm", m.gconf.deathmatch_time);
      ls = m_deathmatch_time->getList();
      for(int i=0; i<ls->getItemCount(); ++i) {
        if(ls->getItem(i)->getText()==v) {
          m_deathmatch_time->setSelection(i);
          m_opt.deathmatch_time = m.gconf.deathmatch_time;
          break;
        }
      }

      v = sgui::Format(L"x%.1f", m.gconf.teamfortress_life);
      ls = m_teamfortress_life->getList();
      for(int i=0; i<ls->getItemCount(); ++i) {
        if(ls->getItem(i)->getText()==v) {
          m_teamfortress_life->setSelection(i);
          m_opt.teamfortress_life = m.gconf.teamfortress_life;
          break;
        }
      }

      if(m.gconf.cboost>=0.0f && m.gconf.cboost<=3.0f) {
        m_cboost->setText(sgui::Format(L"%.1f", m.gconf.cboost));
      }
      if(m.gconf.fboost>=0.0f && m.gconf.fboost<=2.0f) {
        m_fboost->setText(sgui::Format(L"%.1f", m.gconf.fboost));
      }
      if(m.gconf.eboost>=0.1f && m.gconf.eboost<=3.0f) {
        m_eboost->setText(sgui::Format(L"%.1f", m.gconf.eboost));
      }
      validateBoostValue();
    }

    void validateBoostValue()
    {
      try { m_opt.cboost = trim(0.0f, lexical_cast<float>(m_cboost->getText()), 3.0f); } catch(...) {}
      m_cboost->setText(sgui::Format(L"%.1f", m_opt.cboost));

      try { m_opt.fboost = trim(0.0f, lexical_cast<float>(m_fboost->getText()), 2.0f); } catch(...) {}
      m_fboost->setText(sgui::Format(L"%.1f", m_opt.fboost));

      try { m_opt.eboost = trim(0.1f, lexical_cast<float>(m_eboost->getText()), 3.0f); } catch(...) {}
      m_eboost->setText(sgui::Format(L"%.1f", m_opt.eboost));

      if(m_opt.cboost!=1.0f || m_opt.fboost!=1.0f || m_opt.eboost!=1.0f) {
        m_extra->setButtonState(sgui::Button::DOWN);
        m_game_extra_panel->setVisible(true);
      }
    }


    void onCloseButton()
    {
      Super::onCloseButton();
      g_iserver.reset();
      g_iclient.reset();
    }

#ifdef EXCEPTION_ENABLE_NETPLAY
    void update()
    {
      Super::update();
      updateClientConnection();
      updateServerList();
      m_browsebutton->setVisible(g_iclient && g_iclient->isRunning());
    }
#endif // EXCEPTION_ENABLE_NETPLAY


#ifdef EXCEPTION_ENABLE_NETPLAY
    void startServer()
    {
      g_iserver.reset(new InputServer());
      g_iserver->run();
      while(!g_iserver->isAccepting()) {
        if(!g_iserver->getError().empty()) {
          sgui::MessageDialog *d = new sgui::MessageDialog(
            L"network error", _L(g_iserver->getError()),
            g_widget_window, sgui::Point(120, 90), sgui::Size(300, 200));
          d->toTopLevel();
          g_iserver.reset();
          return;
        }
        else {
          sgui::Sleep(1);
        }
      }

      connect("127.0.0.1", GetConfig()->server_port);
      CreateChatWindow();
      toTopLevel();
    }

    void updateClientConnection()
    {
      if(m_connect_thread) {
        if(m_connect_thread->isConnected()) {
          m_connect_thread->runClient();
          m_connect_thread.reset();
        }
        else if(!m_connect_thread->getError().empty()) {
          new sgui::MessageDialog(
            L"network error", _L(m_connect_thread->getError()),
            g_widget_window, sgui::Point(120, 90), sgui::Size(300, 200));
          m_connect_thread.reset();
          if(m_serverlist->getSelectedItem()) {
            requestValidateServerList();
          }
        }

        if(!m_connect_thread) {
          m_connecting->setText(L"");
        }
      }
    }

    void requestValidateServerList()
    {
      if(!m_http_serverlist_checker || m_http_serverlist_checker->isComplete()) {
        m_http_serverlist_checker.reset(new ist::HTTPRequestAsync(g_io_service));
        m_http_serverlist_checker->get(EXCEPTION_HOST, EXCEPTION_HOST_PATH "server/validate");
      }
    }

    void connect(const string& server, int port)
    {
      m_connecting->setText(L"connecting. please wait...");
      g_iclient.reset();

      shared_ptr<InputClientIP> client(new InputClientIP());
      m_connect_thread.reset(new ConnectThread(client, server, port));
      m_connect_thread->run();
      g_iclient = client;
    }

    std::string DecodeParcentCode(const std::string& str)
    {
      std::string r;
      char c;
      size_t i=0;
      while(i<str.size() && sscanf(str.c_str()+i, "%%%x", &c)==1) {
        r+=c;
        i+=3;
      }
      return r;
    }

    void updateServerList()
    {
      if(!m_http_serverlist || !m_http_serverlist->isComplete()) {
        return;
      }
      m_serverlist->clearItem();

      if(m_http_serverlist->getStatus()==200) {
        std::istream in(&m_http_serverlist->getBuf());
        string l;
        while(std::getline(in, l)) {
          char ip[32];
          int port;
          char name[64];
          char comment[64] = "";
          if(sscanf(l.c_str(), "'%31[^']', %d, '%63[^']', '%63[^']'", ip, &port, name, comment)>=3) {
            m_serverlist->addItem(new ServerListItem(ip, port, DecodeParcentCode(name), comment));
          }
        }
      }
      else {
        new sgui::MessageDialog(
          _L("network error"),
          _L(sgui::Format("return code: %d", m_http_serverlist->getStatus())),
          g_widget_window, sgui::Point(120, 90), sgui::Size(300, 200));
      }
      m_http_serverlist.reset();
    }

    void loadServerList()
    {
      if(m_http_serverlist) {
        return;
      }
      m_serverlist->clearItem();
      m_serverlist->addItem(new sgui::ListItem(L"connecting..."));

      m_http_serverlist.reset(new ist::HTTPRequestAsync(g_io_service));
      m_http_serverlist->get(EXCEPTION_HOST, EXCEPTION_HOST_PATH "server/");
    }

    void resetClientAndServer()
    {
      g_iserver.reset();
      g_iclient.reset();
      m_connect_thread.reset();
    }

#endif // EXCEPTION_ENABLE_NETPLAY 

    void start()
    {
      validateBoostValue();
      if(IsServerMode()) {
#ifdef EXCEPTION_ENABLE_NETPLAY
        m_opt.delay = GetInputClient()->getDelay();
        SendNMessage(NMessage::Start(m_opt));
#endif // EXCEPTION_ENABLE_NETPLAY
      }
      else {
        FadeToGame(m_opt);
      }
    }

    bool handleEvent(const sgui::Event& evt)
    {
      size_t id = evt.getSrc() ? evt.getSrc()->getID() : 0;

      if(evt.getType()==sgui::EVT_BUTTON_DOWN) {
        if(id>=BU_START_LOCAL && id<=BU_START_CLIENT) {
          int& nmode = GetConfig()->last_network_mode;
#ifdef EXCEPTION_ENABLE_NETPLAY
          if(id==BU_START_SERVER) {
            resetClientAndServer();
            startServer();
            nmode = NETWORK_SERVER;
          }
          else if(id==BU_START_CLIENT) {
            resetClientAndServer();
            loadServerList();
            nmode = NETWORK_CLIENT;
          }
          else if(id==BU_START_LOCAL) {
            resetClientAndServer();
            g_iclient.reset(new InputClientLocal());
            nmode = NETWORK_LOCAL;
          }
          m_game_panel->setVisible(id==BU_START_LOCAL || id==BU_START_SERVER);
          m_server_panel->setVisible(id==BU_START_SERVER);
          m_client_panel->setVisible(id==BU_START_CLIENT);

          for(int i=0; i<3; ++i) {
            if(m_netb[i] && m_netb[i]!=evt.getSrc()) {
              m_netb[i]->setButtonState(sgui::Button::UP);
            }
          }
          return true;
#endif // EXCEPTION_ENABLE_NETPLAY
        }
        else if(id>=BU_START_LIGHT && id<=BU_START_FUTURE) {
          m_opt.level = id - BU_START_LIGHT;
          for(int i=0; i<5; ++i) {
            if(m_level[i] && m_level[i]!=evt.getSrc()) {
              m_level[i]->setButtonState(sgui::Button::UP);
            }
          }
          return true;
        }
        else if(id>=BU_START_MAP_HORDE && id<=BU_START_MAP_TEAMFORTRESS) {
          m_opt.stage = id - BU_START_MAP_HORDE;
          for(int i=0; i<s_num_maputton; ++i) {
            if(m_map[i] && m_map[i]!=evt.getSrc()) {
              m_map[i]->setButtonState(sgui::Button::UP);
            }
          }
          m_horde_panel->setVisible(id==BU_START_MAP_HORDE);
          m_deathmatch_panel->setVisible(id==BU_START_MAP_DEATHMATCH);
          m_teamfortress_panel->setVisible(id==BU_START_MAP_TEAMFORTRESS);
          return true;
        }
        else if(evt.getSrc()==m_extra) {
          m_game_extra_panel->setVisible(true);
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_BUTTON_UP) {
        if((id>=BU_START_LOCAL && id<=BU_START_CLIENT) ||
           (id>=BU_START_LIGHT && id<=BU_START_FUTURE) ||
           (id>=BU_START_MAP_HORDE && id<=BU_START_MAP_TEAMFORTRESS) ) {
          if(sgui::ToggleButton *tb = dynamic_cast<sgui::ToggleButton*>(evt.getSrc())) {
            tb->setButtonState(sgui::Button::DOWN);
          }
          return true;
        }
        else if(evt.getSrc()==m_extra) {
          m_game_extra_panel->setVisible(false);
          return true;
        }
        else if(id==BU_START_START) {
          start();
          return true;
        }
        else if(id==BU_START_CONNECT) {
#ifdef EXCEPTION_ENABLE_NETPLAY
          try {
            string server = _S(m_serveraddress->getText());
            int port = lexical_cast<int>(m_serverport->getText());
            GetConfig()->last_server = server;
            GetConfig()->last_port = port;
            connect(server, port);
          }
          catch(const boost::bad_lexical_cast&) {
            new sgui::MessageDialog(_L("error"), L"portには数値を入力してください", g_widget_window, sgui::Point(120, 90), sgui::Size(300, 200));
          }
          return true;
#endif // EXCEPTION_ENABLE_NETPLAY
        }
        else if(id==BU_START_CONNECTLIST) {
#ifdef EXCEPTION_ENABLE_NETPLAY
          if(ServerListItem *li=dynamic_cast<ServerListItem*>(m_serverlist->getSelectedItem())) {
            connect(li->ip, li->port);
          }
          return true;
#endif // EXCEPTION_ENABLE_NETPLAY
        }
        else if(id==BU_START_UPDATELIST) {
#ifdef EXCEPTION_ENABLE_NETPLAY
          loadServerList();
          return true;
#endif // EXCEPTION_ENABLE_NETPLAY
        }
        else if(id==BU_START_BLOWSE) {
          if(g_iclient) {
            g_iclient->browse();
          }
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_COMBO) {
        const sgui::ListEvent& e = dynamic_cast<const sgui::ListEvent&>(evt);
        if(id==CB_START_HORDE_WAVE) {
          int wave = 0;
          if(swscanf(e.getItem()->getText().c_str(), L"%d", &wave)==1) {
            m_opt.horde_wave = wave;
          }
          return true;
        }
        else if(id==CB_START_DEATHMATCH_TIME) {
          int time = 0;
          if(swscanf(e.getItem()->getText().c_str(), L"%dm", &time)==1) {
            m_opt.deathmatch_time = time;
          }
          return true;
        }
        else if(id==CB_START_TEAMFORTRESS_LIFE) {
          float life = 0;
          if(swscanf(e.getItem()->getText().c_str(), L"x%f", &life)==1) {
            m_opt.teamfortress_life = life;
          }
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_LIST_DOUBLECLICK) {
        const sgui::ListEvent& e = dynamic_cast<const sgui::ListEvent&>(evt);
        if(evt.getSrc()==m_serverlist) {
#ifdef EXCEPTION_ENABLE_NETPLAY
          if(ServerListItem *li=dynamic_cast<ServerListItem*>(e.getItem())) {
            connect(li->ip, li->port);
          }
          return true;
#endif // EXCEPTION_ENABLE_NETPLAY
        }
      }
      else if(evt.getType()==sgui::EVT_CONSTRUCT) {
        if(evt.getSrc()==m_startbutton && g_selecter) {
          if(m_game_panel->isVisible()) {
            g_selecter->setFocus(m_startbutton);
          }
        }
      }
      else if(evt.getType()==sgui::EVT_EDIT_ENTER) {
        if(evt.getSrc()==m_cboost || evt.getSrc()==m_fboost || evt.getSrc()==m_eboost) {
          validateBoostValue();
          return true;
        }
      }

      return sgui::Dialog::handleEvent(evt);
    }
  };





  class WaitForNewPlayerPanel : public sgui::Panel
  {
  typedef sgui::Panel Super;
  private:
    sgui::StopWatch m_sw;
    std::set<int> m_waiting;

  public:
    WaitForNewPlayerPanel(sgui::Window *parent) :
      Super(parent, sgui::Point(), parent->getSize(), L"pause")
    {
      if(g_wait_for_new_player_panel) {
        throw Error("WaitForNewPlayerPanel::WaitForNewPlayerPanel()");
      }
      g_wait_for_new_player_panel = this;

      IMusic::Pause();

      setBackgroundColor(sgui::Color(0.0f, 0.0f, 0.0f, 0.0f));
      setBorderColor(getBackgroundColor());
      m_sw.run(50);
    }

    ~WaitForNewPlayerPanel()
    {
      g_wait_for_new_player_panel = 0;
    }

    void destroy()
    {
      Super::destroy();
      if(IGame *g = GetGame()) {
        if(!g->isPaused()) {
          IMusic::Resume();
        }
      }
    }

    void draw()
    {
      glColor4fv(getBackgroundColor().v);
      sgui::DrawRect(sgui::Rect(sgui::Point(0,-1), getSize()+sgui::Size(1,2)));
      glColor4f(1,1,1,1);
      drawText(L"incoming new player. please wait...", sgui::Rect(getSize()), sgui::Font::HCENTER|sgui::Font::VCENTER);
    }

    void update()
    {
      Super::update();

      float opa = 0.4f;
      if(m_sw.isRunning()) {
        opa = std::min<float>(opa, float(m_sw.getPast())/50.0f*0.4f);
      }
      setBackgroundColor(sgui::Color(0.0f, 0.0f, 0.0f, opa));
      setBorderColor(getBackgroundColor());
    }

    void incrementWaiting(int sid)
    {
      m_waiting.insert(sid);
    }

    void decrementWaiting(int sid)
    {
      m_waiting.erase(sid);
      if(m_waiting.empty()) {
        destroy();
      }
    }
  };





  class PausePanel : public sgui::Panel
  {
  typedef sgui::Panel Super;
  private:
    sgui::StopWatch m_sw;
    sgui::StopWatch m_sw_resume;
    sgui::Window *m_button_window;

  public:
    PausePanel(sgui::Window *parent) :
      Super(parent, sgui::Point(-1,-1), sgui::Size(642,482), L"pause")
    {
      if(g_pause_panel) {
        throw Error("PauseDialog::PauseDialog()");
      }
      g_pause_panel = this;

      listenEvent(sgui::EVT_JOY_BUTTONUP);
      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_KEYDOWN);
      listenEvent(sgui::EVT_CONFIRMDIALOG);
      setBackgroundColor(sgui::Color(0.0f, 0.0f, 0.0f, 0.0f));
      setBorderColor(getBackgroundColor());
      m_sw.run(50);

      m_button_window = new sgui::Window(g_widget_window, sgui::Point(), g_widget_window->getSize());
      new sgui::Button(m_button_window, sgui::Point(270, 320), sgui::Size(100,20), L"resume", BU_PAUSE_RESUME);
      new sgui::Button(m_button_window, sgui::Point(270, 370), sgui::Size(100,20), L"config", BU_PAUSE_CONFIG);
      new sgui::Button(m_button_window, sgui::Point(270, 420), sgui::Size(100,20), L"exit", BU_PAUSE_EXIT);

      GetGame()->setPause(true);
      IMusic::Pause();
    }

    ~PausePanel()
    {
      g_pause_panel = 0;
    }

    void destroy()
    {
      Super::destroy();
      m_button_window->destroy();
      if(IGame *g=GetGame()) {
        g->setPause(false);
        if(!g->isPaused()) {
          IMusic::Resume();
        }
      }
    }

    void draw()
    {
      glColor4fv(getBackgroundColor().v);
      sgui::DrawRect(sgui::Rect(sgui::Point(0,-1), getSize()+sgui::Size(1,2)));
      glColor4f(1,1,1,1);
    }

    void update()
    {
      Super::update();

      float opa = 0.4f;
      if(m_sw.isRunning()) {
        opa = std::min<float>(opa, float(m_sw.getPast())/50.0f*0.4f);
      }
      if(m_sw_resume.isActive()) {
        opa = std::min<float>(opa, float(m_sw_resume.getLeft())/50.0f*0.4f);
      }
      setBackgroundColor(sgui::Color(0.0f, 0.0f, 0.0f, opa));
      setBorderColor(getBackgroundColor());

      if(m_sw_resume.isFinished()) {
        destroy();
      }
    }

    void resume()
    {
      m_sw_resume.run(50);
    }

    bool handleEvent(const sgui::Event& evt)
    {
      size_t id = evt.getSrc() ? evt.getSrc()->getID() : 0;

      if(evt.getType()==sgui::EVT_BUTTON_UP && !evt.isHandled()) {
        if(id==BU_PAUSE_RESUME) {
          SendNMessage(NMessage::Resume());
          return true;
        }
        else if(id==BU_PAUSE_CONFIG) {
          CreateConfigDialog();
          return true;
        }
        else if(id==BU_PAUSE_EXIT) {
          new sgui::ConfirmDialog(L"exit", L"タイトル画面に戻ります。\nよろしいですか？", BU_PAUSE_EXIT_CONFIRM, g_widget_window);
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_CONFIRMDIALOG) {
        const sgui::DialogEvent& e = dynamic_cast<const sgui::DialogEvent&>(evt);
        if(id==BU_PAUSE_EXIT_CONFIRM && e.isOK()) {
          SendNMessage(NMessage::End());
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_KEYDOWN) {
        const sgui::KeyboardEvent& e = dynamic_cast<const sgui::KeyboardEvent&>(evt);
        if(e.getKey()==sgui::KEY_ESCAPE) {
          SendNMessage(NMessage::Resume());
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_JOY_BUTTONUP) {
        const sgui::JoyEvent& e = dynamic_cast<const sgui::JoyEvent&>(evt);
        if(e.getButton()==9) {
          SendNMessage(NMessage::Resume());
          return true;
        }
      }

      return Super::handleEvent(evt);
    }
  };


  class GamePanel : public sgui::Panel
  {
  typedef sgui::Panel Super;
  private:
    IGame *m_game;
    bool m_draw_skip;

  public:
    GamePanel(IGame *game) :
      Super(g_background_window, sgui::Point(), g_background_window->getSize(), _L(""), 0, 0),
      m_draw_skip(false)
    {
      if(g_game_panel) {
        throw Error("GamePanel::GamePanel()");
      }
      g_game_panel = this;

      listenEvent(sgui::EVT_APP_LOSINGKEYFOCUS);
      listenEvent(sgui::EVT_APP_ICONIZE);
      listenEvent(sgui::EVT_MOUSE_BUTTONDOWN);
      listenEvent(sgui::EVT_MOUSE_BUTTONUP);
      listenEvent(sgui::EVT_MOUSE_MOVE);
      listenEvent(sgui::EVT_KEYDOWN);
      listenEvent(sgui::EVT_JOY_BUTTONUP);
      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_BUTTON_DOWN);

      if(GetConfig()->enable_server_mode) {
        m_draw_skip = true;
        sgui::ToggleButton *tb = new sgui::ToggleButton(this, sgui::Point(575, 455), sgui::Size(60, 20), L"draw skip", BU_SERVER_DRAWSKIP);
        tb->setButtonState(sgui::Button::DOWN);
      }

      m_game = game;
    }

    // 右上の×で閉じられるとdestroyを経由せず破棄されるので、必要な手順 
    ~GamePanel()
    {
      if(!g_title_panel) {
        releaseGame();
        g_game_panel = 0;
      }
    }

    void destroy()
    {
      releaseGame();
      g_game_panel = 0;
      g_widget_window->destroy();
      g_widget_window = new sgui::Window(sgui::View::instance(), sgui::Point(), sgui::View::instance()->getSize());
      if(g_chat_window) {
        g_chat_window->destroy();
      }

      Super::destroy();
    }

    bool saveState(const string& path)
    {
      if(m_game) {
        std::fstream io(path.c_str(), std::ios::out | std::ios::binary);
        if(!io) {
          return false;
        }
        ist::biostream bio(io);

        Serializer s;
        ist::bbuffer cs;
        m_game->serialize(s);
        ist::compress(s, cs);
        bio << s.size() << cs;
        return true;
      }
      return false;
    }

    bool loadState(const string& path)
    {
      std::fstream io(path.c_str(), std::ios::in | std::ios::binary);
      if(!io) {
        return false;
      }
      ist::biostream bio(io);

      size_t size;
      ist::bbuffer cs;
      Deserializer s;
      bio >> size >> cs;
      s.resize(size);
      ist::uncompress(cs, s);

      if(m_game) {
        delete m_game;
      }
      m_game = CreateGame(s);

      return true;
    }

    void loadState(Deserializer& s)
    {
      if(m_game) {
        delete m_game;
      }
      m_game = CreateGame(s);
    }

    void releaseGame()
    {
      if(m_game) {
        m_game->exit();
        m_game->write();
        delete m_game;
        m_game = 0;
      }
      g_iserver.reset();
      g_iclient.reset();
    }

    void update()
    {
      Super::update();
      if(m_game) {
        ViewportSaver vs;
        fitViewport();
        try {
          m_game->update();
        }
        catch(shared_ptr<Deserializer> s) {
          int frame = m_game->getPast();
          loadState(*s);
          SendNMessage(NMessage::Response("state restored"));
          int gap = m_game->getPast()-frame;
          for(int i=0; i<gap; ++i) {
            SendNMessage(NMessage::Input(0, 0, frame+i));
          }
        }
      }
    }

    void draw()
    {
      if(m_game && !m_draw_skip) {
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        m_game->draw();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
      }
    }

    bool handleEvent(const sgui::Event& evt)
    {
      if(evt.getType()==sgui::EVT_APP_ICONIZE) {
        SendNMessage(NMessage::Pause());
        return true;
      }
      else if(evt.getType()==sgui::EVT_KEYDOWN && !evt.isHandled()) {
        const sgui::KeyboardEvent& e = dynamic_cast<const sgui::KeyboardEvent&>(evt);
        if(e.getKey()==sgui::KEY_ESCAPE) {
          SendNMessage(NMessage::Pause());
          return true;
        }
#ifdef EXCEPTION_ENABLE_STATE_SAVE
        else if(e.getKey()==sgui::KEY_F6) {
          char filename[128];
          sprintf(filename, "%d%d.dump", ::time(0), sgui::GetTicks());
          saveState(filename);
          StateDialogUpdateList();
          return true;
        }
        else if(e.getKey()==sgui::KEY_F7) {
          ToggleStateDialog();
          return true;
        }
#endif // EXCEPTION_ENABLE_STATE_SAVE 
#ifdef EXCEPTION_ENABLE_PROFILE
        else if(e.getKey()==sgui::KEY_F8) {
          ToggleDebugDialog();
          return true;
        }
        else if(e.getKey()==sgui::KEY_F9) {
          ToggleObjectBlowserDialog();
          return true;
        }
        else if(e.getKey()==sgui::KEY_F10) {
          ToggleConfigDialog();
          return true;
        }
        else if(e.getKey()==sgui::KEY_F11) {
          ToggleProfileDialog();
          return true;
        }
#endif // EXCEPTION_ENABLE_PROFILE 
      }
      else if(evt.getType()==sgui::EVT_JOY_BUTTONUP) {
        const sgui::JoyEvent& e = dynamic_cast<const sgui::JoyEvent&>(evt);
        if(e.getButton()==GetConfig()->pad[6]) {
          if(!g_pause_panel) {
            new PausePanel(this);
          }
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_BUTTON_DOWN) {
        if(evt.getSrc() && evt.getSrc()->getID()==BU_SERVER_DRAWSKIP) {
          m_draw_skip = true;
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_BUTTON_UP) {
        if(evt.getSrc() && evt.getSrc()->getID()==BU_SERVER_DRAWSKIP) {
          m_draw_skip = false;
          return true;
        }
      }
      return Super::handleEvent(evt);
    }
  };




  class TitlePanel : public sgui::Panel
  {
  typedef sgui::Window Super;
  private:
    obj_ptr m_background;

  public:
    TitlePanel() :
      sgui::Panel(g_background_window, sgui::Point(), g_background_window->getSize()), m_background(0)
    {
      if(g_title_panel) {
        throw Error("TitlePanel::TitlePanel()");
      }
      g_title_panel = this;

      m_background = CreateTitleBackground();
      GetMusic("title.ogg")->play();

      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_CLOSEDIALOG);
      listenEvent(sgui::EVT_LIST_DOUBLECLICK);
      listenEvent(sgui::EVT_KEYDOWN);
      listenEvent(sgui::EVT_CONFIRMDIALOG);
      setBorderColor(sgui::Color(0,0,0,0));
      
      sgui::Point base = sgui::Point(260, 300);
      new sgui::Button(g_widget_window, base, sgui::Size(120, 20), L"start", BU_START);
      base+=sgui::Point(0, 40);
      new sgui::Button(g_widget_window, base, sgui::Size(120, 20), L"record", BU_RECORD);
      base+=sgui::Point(0, 40);
      new sgui::Button(g_widget_window, base, sgui::Size(120, 20), L"config", BU_CONFIG);
      base+=sgui::Point(0, 40);
      new sgui::Button(g_widget_window, base, sgui::Size(120, 20), L"exit", BU_EXIT);

      char buf[128];
      sprintf(buf, "version %.2f (c)2007-2009 primitive", float(EXCEPTION_VERSION)/100.0f);
      new sgui::Label(this, sgui::Point(425, 465), sgui::Size(260, 20), _L(buf));

      update();

      if(GetConfig()->enable_server_mode) {
        CreateStartDialog();
      }
    }

    ~TitlePanel()
    {
      m_background->release();
      g_title_panel = 0;
    }

    void destroy()
    {
      g_title_panel = 0;
      g_widget_window->destroy();
      g_widget_window = new sgui::Window(sgui::View::instance(), sgui::Point(), sgui::View::instance()->getSize());

      Super::destroy();
    }

    void update()
    {
      Super::update();
      m_background->update();

#ifdef EXCEPTION_ENABLE_NETUPDATE
      if(updater::g_patch_version > EXCEPTION_VERSION) {
        char buf[256];
        sprintf(buf, "version %.2f へのパッチが見つかりました。\nアップデートを行いますか？", float(updater::g_patch_version)/100.0f+0.001);
        new sgui::ConfirmDialog(L"update", _L(buf), DL_UPDATE, g_widget_window);
        updater::g_patch_version = EXCEPTION_VERSION;
      }
#endif // EXCEPTION_ENABLE_NETUPDATE 
    }

    void draw()
    {
#ifdef EXCEPTION_ENABLE_PROFILE
      Uint32 t = sgui::GetTicks();
#endif // EXCEPTION_ENABLE_PROFILE 
      {
        ViewportSaver vs;
        fitViewport();

        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        m_background->draw();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
      }
      glColor4fv(vector4(0,0,0, 0.3f).v);
      sgui::DrawRect(sgui::Rect(sgui::Point(-1,-1), getSize()+sgui::Size(2,2)));
      glColor4f(1,1,1,1);

      glDepthMask(GL_FALSE);
      glEnable(GL_TEXTURE_2D);
      texture_ptr tex = GetTexture("title.png");
      tex->assign();
      sgui::DrawRect(sgui::Rect(sgui::Point(40, 80), sgui::Size(560, 160)));
      tex->disassign();
      glDisable(GL_TEXTURE_2D);
      glDepthMask(GL_TRUE);

#ifdef EXCEPTION_ENABLE_PROFILE
      AddDrawTime(float(sgui::GetTicks()-t));
#endif // EXCEPTION_ENABLE_PROFILE 
    }

    bool handleEvent(const sgui::Event& evt)
    {
      size_t id = evt.getSrc() ? evt.getSrc()->getID() : 0;

      if(evt.getType()==sgui::EVT_BUTTON_UP) {
        if(id==BU_START) {
          CreateStartDialog();
          return true;
        }
        else if(id==BU_CONFIG) {
          CreateConfigDialog();
          return true;
        }
        else if(id==BU_RECORD) {
          if(!g_record_dialog) {
            new RecordDialog();
          }
          return true;
        }
        else if(id==BU_EXIT) {
          FadeToExit();
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_CONFIRMDIALOG) {
        const sgui::DialogEvent& e = dynamic_cast<const sgui::DialogEvent&>(evt);
#ifdef EXCEPTION_ENABLE_NETUPDATE
        if(id==DL_UPDATE && e.isOK()) {
          ExecUpdater();
          return true;
        }
#endif // EXCEPTION_ENABLE_NETUPDATE 
      }
      else if(evt.getType()==sgui::EVT_KEYDOWN && !evt.isHandled()) {
        const sgui::KeyboardEvent& e = dynamic_cast<const sgui::KeyboardEvent&>(evt);
        if(e.getKey()==sgui::KEY_ESCAPE) {
          FadeToExit();
          return true;
        }
        else if(e.getKey()==sgui::KEY_F10) {
          ToggleConfigDialog();
          return true;
        }
        else if(e.getKey()==sgui::KEY_F7) {
#ifdef EXCEPTION_ENABLE_STATE_SAVE
          ToggleStateDialog();
          return true;
#endif // EXCEPTION_ENABLE_STATE_SAVE 
        }
        else if(e.getKey()==sgui::KEY_F11) {
#ifdef EXCEPTION_ENABLE_PROFILE
          ToggleProfileDialog();
          return true;
#endif // EXCEPTION_ENABLE_PROFILE 
        }
      }

      return sgui::Panel::handleEvent(evt);
    }
  };





  void CreateStartDialog()
  {
    if(!g_start_dialog) {
      new StartDialog();
    }
  }

  void AssignConfig(const NMessage& m)
  {
    if(g_start_dialog) {
      g_start_dialog->assignConfig(m);
    }
  }

  void ForceGameStart()
  {
    if(g_start_dialog && IsServerMode()) {
      g_start_dialog->start();
    }
  }


  void CreateConfigDialog()
  {
    if(!g_config_dialog) {
      new ConfigDialog();
    }
  }

  void ToggleConfigDialog()
  {
    if(!g_config_dialog) {
      new ConfigDialog();
    }
    else {
      g_config_dialog->destroy();
    }
  }


  void CreateTitlePanel()
  {
    if(!g_title_panel) {
      new TitlePanel();
    }
  }

  void CreateGamePanel(IGame *game)
  {
    if(!g_game_panel) {
      new GamePanel(game);
    }
  }

  void SaveState(const string& path)
  {
    if(g_game_panel) {
      g_game_panel->saveState(path);
    }
  }

  void LoadState(const string& path)
  {
    if(g_title_panel) {
      g_title_panel->destroy();
    }
    if(!g_game_panel) {
      new GamePanel(0);
    }
    g_game_panel->loadState(path);
  }

  void LoadState(Deserializer& s)
  {
    if(g_title_panel) {
      g_title_panel->destroy();
    }
    if(!g_game_panel) {
      new GamePanel(0);
    }
    g_game_panel->loadState(s);
  }

  void Pause()
  {
    if(!g_pause_panel) {
      new PausePanel(g_game_panel);
    }
  }

  void Resume()
  {
    if(g_pause_panel) {
      g_pause_panel->resume();
    }
  }

  void WaitForNewPlayer(int sid)
  {
    if(!g_game_panel) {
      return;
    }
    if(!g_wait_for_new_player_panel) {
      new WaitForNewPlayerPanel(g_game_panel);
    }
    g_wait_for_new_player_panel->incrementWaiting(sid);
  }

  void ReleaseWaitingForNewPlayer(int sid)
  {
    if(!g_game_panel || !g_wait_for_new_player_panel) {
      return;
    }
    g_wait_for_new_player_panel->decrementWaiting(sid);
  }


  void FadeToGame(const GameOption& opt)
  {
    if(!g_fadeout_window && !g_game_panel) {
      new FadeToGameWindow(GetTitleWindow(), opt);
    }
  }

  void FadeToGame(const string& path)
  {
    if(!g_fadeout_window && !g_game_panel) {
      new FadeToGameWindow(GetTitleWindow(), path);
    }
  }

  void FadeToTitle()
  {
    if(!g_fadeout_window && !g_title_panel) {
      new FadeToTitleWindow(GetGameWindow());
    }
  }

  void FadeToExit()
  {
    if(!g_fadeout_window) {
      new FadeToExitPanel(GetTitleWindow());
    }
  }


  sgui::Window* GetGameWindow() { return g_game_panel; }
  sgui::Window* GetTitleWindow() { return g_title_panel; }




  void OpenURL(const std::string& url)
  {
#ifdef WIN32
    ShellExecuteA(sgui::GetHWND(), "open", url.c_str(), "", sgui::GetCWD().c_str(), SW_SHOWDEFAULT);
#endif // WIN32
  }

  bool FindProcess(const char *exe)
  {
#ifdef WIN32
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
#else // WIN32 
    return false;
#endif // WIN32 
  }

} // exception 


