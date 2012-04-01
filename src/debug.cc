#include "stdafx.h"

#include "game.h"
#include "network.h"
#include "app.h"

namespace exception {

  class DebugDialog;
  class ObjectBrowserDialog;
  class ProfileDialog;
  class StateDialog;

  namespace {
    DebugDialog *g_debug_dialog = 0;
    ObjectBrowserDialog *g_objbrowser_dialog = 0;
    ProfileDialog *g_profile_dialog = 0;
    StateDialog *g_state_dialog = 0;
  }


#ifdef EXCEPTION_ENABLE_PROFILE

  class DebugDialog : public sgui::Dialog
  {
  typedef sgui::Dialog Super;
  private:
    sgui::Button *m_bu_detail;
    sgui::Button *m_bu_pause;
    sgui::Button *m_bu_step;
    sgui::Label *m_message;

  public:
    DebugDialog() :
      Super(g_widget_window, sgui::Point(30,30), sgui::Size(360,250), L"debug")
    {
      if(g_debug_dialog) {
        throw Error("DebugDialog::DebugDialog()");
      }
      g_debug_dialog = this;

      m_bu_detail = new sgui::Button(this, sgui::Point(5,30), sgui::Size(45, 20), L"detail");
      m_bu_pause = new sgui::Button(this, sgui::Point(105,30), sgui::Size(45, 20), L"pause");
      m_bu_step = new sgui::Button(this, sgui::Point(155,30), sgui::Size(45, 20), L"step");
      m_message = new sgui::Label(this, sgui::Point(5,50), sgui::Size(350, 195), L"");
    }

    ~DebugDialog()
    {
      g_debug_dialog = 0;
    }

    void update()
    {
      Super::update();
      if(IGame *game=GetGame()) {
        m_message->setText(_L(game->p()));
      }
    }

    bool handleEvent(const sgui::Event& evt)
    {
      if(evt.getType()==sgui::EVT_BUTTON_UP) {
        if(evt.getSrc()==m_bu_detail) {
          printf(GetGame()->pDetail().c_str());
          fflush(stdout);
          return true;
        }
        else if(evt.getSrc()==m_bu_pause) {
          GetGame()->setPause(!IsPaused());
          return true;
        }
        else if(evt.getSrc()==m_bu_step) {
          GetGame()->step();
          return true;
        }
      }
      return sgui::Dialog::handleEvent(evt);
    }
  };


  class GObjItem : public sgui::ListItem
  {
  public:
    gid id;

    GObjItem(sgui::wstring l, gid _id) : sgui::ListItem(l), id(_id) {}
  };

  class ObjectBrowserDialog : public sgui::Dialog
  {
  typedef sgui::Dialog Super;
  private:
    sgui::List *m_list;
    sgui::Button *m_bu_update;
    int m_order_by;

  public:
    ObjectBrowserDialog() :
      Super(g_widget_window, sgui::Point(30,30), sgui::Size(360,400), L"object inspector"), m_order_by(0)
    {
      if(g_objbrowser_dialog) {
        throw Error("ObjectBrowserDialog::ObjectBrowserDialog()");
      }
      g_objbrowser_dialog = this;

      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_LIST_DOUBLECLICK);

      m_bu_update = new sgui::Button(this, sgui::Point(5,30), sgui::Size(50, 20), L"update");
      new sgui::Button(this, sgui::Point(55,30), sgui::Size(50, 20), L"p", BU_DEBUG_P);
      new sgui::Button(this, sgui::Point(110,30), sgui::Size(50, 20), L"destroy", BU_DEBUG_DESTROY);
      new sgui::Button(this, sgui::Point(170,30), sgui::Size(40, 20), L"id", BU_DEBUG_ORDER_BY_ID);
      new sgui::Button(this, sgui::Point(220,30), sgui::Size(40, 20), L"draw", BU_DEBUG_ORDER_BY_DRAW_PRIORITY);
      m_list = new sgui::List(this, sgui::Point(5,55), sgui::Size(350, 300));
    }

    ~ObjectBrowserDialog()
    {
      g_objbrowser_dialog = 0;
    }

    void updateList()
    {
      gobj_vector gv;
      gobj_iter& iter = GetMainTSM().getAllObjects();
      while(iter.has_next()) {
        gobj_ptr p = iter.iterate();
        if(!IsFraction(p)) {
          gv.push_back(p);
        }
      }
      if(m_order_by==1) {
        std::sort(gv.begin(), gv.end(), greater_draw_priority());
      }

      m_list->clearItem();
      for(size_t i=0; i<gv.size(); ++i) {
        m_list->addItem(new GObjItem(_L(typeid(*gv[i]).name()), gv[i]->getID()));
      }
    }

    void showInfo(gid id)
    {
      if(gobj_ptr p = GetObjectByID(id)) {
        new sgui::MessageDialog(L"object", _L(p->p()), getParent(), sgui::Point(50, 50));
      }
    }

    bool handleEvent(const sgui::Event& evt)
    {
      if(evt.getType()==sgui::EVT_BUTTON_UP) {
        if(evt.getSrc()==m_bu_update) {
          updateList();
          return true;
        }
        else if(evt.getSrc()->getID()==BU_DEBUG_P) {
          if(GObjItem *gi = dynamic_cast<GObjItem*>(m_list->getSelectedItem())) {
            showInfo(gi->id);
          }
          return true;
        }
        else if(evt.getSrc()->getID()==BU_DEBUG_DESTROY) {
          if(GObjItem *gi = dynamic_cast<GObjItem*>(m_list->getSelectedItem())) {
            if(gobj_ptr obj = GetObjectByID(gi->id)) {
              GetMainTSM().sendDestroyMessage(0, obj);
            }
          }
          return true;
        }
        else if(evt.getSrc()->getID()==BU_DEBUG_ORDER_BY_ID) {
          m_order_by = 0;
          return true;
        }
        else if(evt.getSrc()->getID()==BU_DEBUG_ORDER_BY_DRAW_PRIORITY) {
          m_order_by = 1;
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_LIST_DOUBLECLICK) {
        const sgui::ListEvent& e = dynamic_cast<const sgui::ListEvent&>(evt);
        if(GObjItem *gi = dynamic_cast<GObjItem*>(e.getItem())) {
          showInfo(gi->id);
          return true;
        }
      }
      return sgui::Dialog::handleEvent(evt);
    }
  };


  class Graph : public sgui::Window
  {
  private:
    boost::mutex m_mutex;
    size_t m_max_data;
    sgui::Color m_line_color;
    sgui::Range m_range;
    std::deque<float> m_dat;

  public:
    const sgui::Color& getLineColor() const { return m_line_color; }
    const sgui::Range& getRange() const { return m_range; }
    size_t getMaxData() const { return m_max_data; }
    void setLineColor(const sgui::Color& v) { m_line_color=v; }
    void setRange(const sgui::Range& v) { m_range=v; }
    void setMaxData(size_t v) { m_max_data=v; }

    void clearData() { m_dat.clear(); }
    void addData(float v)
    {
      boost::mutex::scoped_lock l(m_mutex);
      m_dat.push_front(v);
      if(m_dat.size()>m_max_data) {
        m_dat.pop_back();
      }
    }

    Graph(sgui::Window *w, const sgui::Point& pos, const sgui::Size& size) :
      sgui::Window(w, pos, size),
      m_max_data(100)
    {}

    void draw()
    {
      {
        ist::ModelviewMatrixSaver m_ms;
        ist::ProjectionMatrixSaver m_ps;

        ist::OrthographicCamera cam;
        cam.setPosition(vector4(0.0f, 0.0f, 500.0f));
        cam.setScreen(getSize().getWidth(),0, m_range.getMin(),m_range.getMax());
        cam.setZNear(-1.0f);
        cam.setZFar(1000.0f);
        cam.look();

        float half = (m_range.getMax()-m_range.getMin())/2.0f;
        glColor4fv(sgui::Color(1,1,1,0.2f).v);
        glBegin(GL_LINE_STRIP);
        glVertex2f(getSize().getWidth(), half);
        glVertex2f(0, half);
        glEnd();

        glColor4fv(getLineColor().v);
        glBegin(GL_LINE_STRIP);
        {
          boost::mutex::scoped_lock l(m_mutex);
          for(size_t i=0; i<m_dat.size(); ++i) {
            glVertex2f(float(i), m_dat[i]);
          }
        }
        glEnd();
      }
      glColor4fv(getBorderColor().v);
      sgui::DrawRectEdge(sgui::Rect(getSize()));
      glColor4f(1,1,1,1);
    }
  };

  class ProfileDialog : public sgui::Dialog
  {
  typedef sgui::Dialog Super;
  private:
    typedef std::map<boost::thread::id, Graph*> graph_cont;
    Graph *m_update;
    Graph *m_draw;
    graph_cont m_async;

  public:
    ProfileDialog() : 
      sgui::Dialog(g_widget_window, sgui::Point(5,5), sgui::Size(500,400), L"profile"),
      m_update(0), m_draw(0)
    {
      if(g_profile_dialog) {
        throw Error("ProfileDialog::ProfileDialog()");
      }
      g_profile_dialog = this;

      setBackgroundColor(sgui::Color(0,0,0,0.6f));

      new sgui::Label(this, sgui::Point(5, 30), sgui::Size(55, 30), L"同期\n更新");
      m_update = new Graph(this, sgui::Point(45, 30), sgui::Size(400, 40));
      m_update->setRange(sgui::Range(0.0f, 20.0f));
      m_update->setLineColor(sgui::Color(0.4f, 0.4f, 1.0f));
      m_update->setMaxData(400);

      new sgui::Label(this, sgui::Point(5, 75), sgui::Size(55, 30), L"描画");
      m_draw = new Graph(this, sgui::Point(45, 75), sgui::Size(400, 40));
      m_draw->setRange(sgui::Range(0.0f, 20.0f));
      m_draw->setLineColor(sgui::Color(0.0f, 1.0f, 0.0f));
      m_draw->setMaxData(400);

      new sgui::Label(this, sgui::Point(5, 120), sgui::Size(55, 30), L"非同期\n更新");
      onThreadCountChange();

      toTopLevel();
    }

    ~ProfileDialog()
    {
      g_profile_dialog = 0;
    }

    void onThreadCountChange()
    {
      for(graph_cont::iterator i=m_async.begin(); i!=m_async.end(); ++i) {
        i->second->destroy();
      }
      m_async.clear();

      sgui::Size s(450, 120);
      ist::Scheduler *schedulr = ist::Scheduler::instance();
      for(size_t i=0; i<schedulr->getThreadCount()+1; ++i) {
        Graph *t = new Graph(this, sgui::Point(45, 75+45*(i+1)), sgui::Size(400, 40));
        t->setRange(sgui::Range(0.0f, 20.0f));
        t->setLineColor(sgui::Color(1.0f, 0.0f, 0.0f));
        t->setMaxData(400);
        if(i<schedulr->getThreadCount()) {
          m_async[schedulr->getThreadID(i)] = t;
        }
        else {
          m_async[boost::this_thread::get_id()] = t;
        }
        s+=sgui::Size(0, 45);
      }
      setSize(s);
    }

    void addUpdateTime(float v) { m_update->addData(v); }
    void addDrawTime(float v) { m_draw->addData(v); }
    void addThreadTime(boost::thread::id tid, float v) { m_async[tid]->addData(v); }
 };

  void AddUpdateTime(float v)
  {
    if(g_profile_dialog) {
      g_profile_dialog->addUpdateTime(float(v)*0.5f);
    }
  }

  void AddDrawTime(float v)
  {
    if(g_profile_dialog) {
      g_profile_dialog->addDrawTime(float(v)*0.5f);
    }
  }

  void AddThreadTime(boost::thread::id tid, float v)
  {
    if(g_profile_dialog) {
      g_profile_dialog->addThreadTime(tid, float(v)*0.5f);
    }
  }
  void ProfileDialogNotifyThreadCountChange()
  {
    if(g_profile_dialog) {
      g_profile_dialog->onThreadCountChange();
    }
  }


  void ToggleDebugDialog()
  {
    if(!g_debug_dialog) {
      new DebugDialog();
    }
    else {
      g_debug_dialog->destroy();
    }
  }

  void ToggleObjectBlowserDialog()
  {
    if(!g_objbrowser_dialog) {
      new ObjectBrowserDialog();
    }
    else {
      g_objbrowser_dialog->destroy();
    }
  }

  void ToggleProfileDialog()
  {
    if(!g_profile_dialog) {
      new ProfileDialog();
    }
    else {
      g_profile_dialog->destroy();
    }
  }

#endif // EXCEPTION_ENABLE_PROFILE 



#ifdef EXCEPTION_ENABLE_STATE_SAVE

  class StateDialog : public sgui::Dialog
  {
  typedef sgui::Dialog Super;
  private:
    sgui::List *m_list;

  public:
    StateDialog() :
      Super(g_widget_window, sgui::Point(15,15), sgui::Size(160,285), L"state", DL_STATE)
      {
      if(g_state_dialog) {
        throw Error("StateDialog::StateDialog()");
      }
      g_state_dialog = this;

      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_LIST_DOUBLECLICK);
      listenEvent(sgui::EVT_CONFIRMDIALOG);

      sgui::Window *w;
      w = new sgui::Window(this, sgui::Point(5, 30), sgui::Size(150,290));

      new sgui::Button(w, sgui::Point(0, 0), sgui::Size(45,20), L"save", BU_STATE_SAVE);
      new sgui::Button(w, sgui::Point(50, 0), sgui::Size(45,20), L"load", BU_STATE_LOAD);
      new sgui::Button(w, sgui::Point(100, 0), sgui::Size(45,20), L"delete", BU_STATE_DELETE);
      m_list = new sgui::List(w, sgui::Point(0, 25), sgui::Size(150,225), LI_STATE);
      listLocal();

      toTopLevel();
    }

    ~StateDialog()
    {
      g_state_dialog = 0;
    }


    void listLocal()
    {
      m_list->clearItem();

      ist::Dir dir(".");
      for(size_t i=0; i<dir.size(); ++i) {
        const string& path = dir[i];
        boost::smatch m;
        if(regex_search(path, m, regex("\\.dump$"))) {
          m_list->addItem(new sgui::ListItem(_L(path)));
        }
      }
    }

    bool handleEvent(const sgui::Event& evt)
    {
      size_t id = evt.getSrc() ? evt.getSrc()->getID() : 0;

      if(evt.getType()==sgui::EVT_BUTTON_DOWN) {
      }
      else if(evt.getType()==sgui::EVT_BUTTON_UP) {
        if(id==BU_STATE_SAVE) {
          if(IGame *game = GetGame()) {
            char filename[128];
            sprintf(filename, "%d%d.dump", ::time(0), sgui::GetTicks());
            SaveState(filename);
            listLocal();
          }
          return true;
        }
        else if(id==BU_STATE_LOAD) {
          LoadState(_S(m_list->getSelectedItem()->getText()));
          return true;
        }
        else if(id==BU_STATE_DELETE) {
          if(m_list->getSelectionCount()>0) {
            new sgui::ConfirmDialog(L"delete", L"選択されたデータを消去します。\nよろしいですか？",
              BU_STATE_DELETE_CONFIRM, g_widget_window, sgui::Point(20, 50));
          }
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_CONFIRMDIALOG) {
        const sgui::DialogEvent& e = dynamic_cast<const sgui::DialogEvent&>(evt);
        if(id==BU_STATE_DELETE_CONFIRM && e.isOK()) {
          sgui::ListItem *li = m_list->getSelectedItem();
          remove(_S(li->getText()).c_str());
          m_list->removeItem(li);
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_LIST_DOUBLECLICK) {
        const sgui::ListEvent& e = dynamic_cast<const sgui::ListEvent&>(evt);
        if(id==LI_STATE) {
          LoadState(_S(e.getItem()->getText()));
          return true;
        }
      }

      return Super::handleEvent(evt);
    }
  };


  void ToggleStateDialog()
  {
    if(!g_state_dialog) {
      new StateDialog();
    }
    else {
      g_state_dialog->destroy();
    }
  }

  void StateDialogUpdateList()
  {
    if(g_state_dialog) {
      g_state_dialog->listLocal();
    }
  }


#endif // EXCEPTION_ENABLE_STATE_SAVE 


  void DumpReplay(const string& path)
  {
    ist::gzbstream s(path, "rb");
    if(!s) {
      return;
    }

    int version = 0;
    size_t fps;
    GameOption opt;

    s >> version;
    printf("version %d\n", version);
    if(version!=EXCEPTION_REPLAY_VERSION) {
      return;
    }

    s >> opt >> fps;
    InputClientReplay *replay = new InputClientReplay(s, version);
    opt.print();
    replay->print();
  }



} // namespace exception 
