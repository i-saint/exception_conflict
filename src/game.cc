#include "stdafx.h"

#include <time.h>
#include <ist/grid.h>
#include <ist/wchar.h>
#include "input.h"
#include "network.h"
#include "thread_specific.h"
#include "game.h"
#include "character/creater.h"


namespace exception {


  class FrameBuffer : public IFrameBuffer
  {
  private:
    int m_source;
    GLuint m_tex;
    GLsizei m_width;
    GLsizei m_height;
    GLsizei m_screen_width;
    GLsizei m_screen_height;

  public:
    // GL_FRONT/GL_BACK 
    FrameBuffer(int src) :
      m_source(src), m_tex(0), m_width(0), m_height(0)
    {
      int viewport[4];
      glGetIntegerv(GL_VIEWPORT, viewport);
      m_width = m_screen_width = viewport[2];
      m_height = m_screen_height = viewport[3];
      if(!GetConfig()->npttexture) {
        GLsizei w = 16;
        GLsizei h = 16;
        while(w<m_width) { w*=2; }
        while(h<m_height) { h*=2; }
        m_width = w;
        m_height = h;
      }

      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glGenTextures(1, &m_tex);
      glBindTexture(GL_TEXTURE_2D, m_tex);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
      glBindTexture(GL_TEXTURE_2D, 0);
    }

    ~FrameBuffer()
    {
      clear();
    }

    void copy()
    {
      glBindTexture(GL_TEXTURE_2D, m_tex);
      glReadBuffer(m_source);
      glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, m_screen_width, m_screen_height);
      glReadBuffer(GL_FRONT);
      glBindTexture(GL_TEXTURE_2D, 0);
    }

    void assign()
    {
      glBindTexture(GL_TEXTURE_2D, m_tex);
    }

    void disassign()
    {
      glBindTexture(GL_TEXTURE_2D, 0);
    }

    virtual GLsizei getWidth() const { return m_width; }
    virtual GLsizei getHeight() const { return m_height; }
    virtual GLsizei getScreenWidth() const { return m_screen_width; }
    virtual GLsizei getScreenHeight() const { return m_screen_height; }

    void clear()
    {
      if(m_tex) {
        glDeleteTextures(1, &m_tex);
        m_tex = 0;
      }
    }
  };



  ushort GetMouseInput()
  {
    ushort s = 0;
    if(Uint8 b = SDL_GetMouseState(0, 0)) {
      if     (b&1) { s|=(1<<4); }
      else if(b&4) { s|=(1<<5); }
      else if(b&2) { s|=(1<<7); }
    }
    return s;
  }

  ushort GetKeyboardInput()
  {
    ushort s = 0;
    const IConfig& conf = *GetConfig();
    Uint8 *key = SDL_GetKeyState(NULL);

    if     (key[conf.key[6]] || key[sgui::KEY_UP])   { s|=(1<<0); }
    else if(key[conf.key[7]] || key[sgui::KEY_DOWN]) { s|=(1<<1); }
    if     (key[conf.key[8]] || key[sgui::KEY_RIGHT]){ s|=(1<<3); }
    else if(key[conf.key[9]] || key[sgui::KEY_LEFT]) { s|=(1<<2); }

    for(int i=0; i<6; ++i) {
      if(key[conf.key[i]]) {
        s|=(1<<(i+4));
      }
    }
    return s;
  }

  ushort GetJoystickInput()
  {
    ushort s = 0;
    const IConfig& conf = *GetConfig();
    PadState ps = GetPadState();

    int t = conf.threshold1;
    if     (ps.move_y <-t) { s|=(1<<0); }
    else if(ps.move_y > t) { s|=(1<<1); }
    if     (ps.move_x > t) { s|=(1<<3); }
    else if(ps.move_x <-t) { s|=(1<<2); }

    for(int i=0; i<6; ++i) {
      if(ps.button[conf.pad[i]]) {
        s|=(1<<(i+4));
      }
    }

    return s;
  }

  class InputStateSender : public RefCounter
  {
  private:
    std::deque<ushort> m_sendqueue;
    vector2 m_pmpos;
    ushort m_state;
    ushort m_pre_state;

  public:
    InputStateSender() : m_state(0), m_pre_state(0)
    {
      m_pmpos = GetMousePosition();
    }

    void update()
    {
      m_pre_state = m_state;
      m_state = getSystemState();
    }

    ushort getState() { return m_state; }
    ushort getPreState() { return m_pre_state; }
    ushort getSystemState()
    {
      ushort s = 0;
      const IConfig& conf = *GetConfig();
      PadState ps = GetPadState();

      vector2 mpos = GetMousePosition();
      vector2 jdir(ps.dir_x, -ps.dir_y);
      player_ptr pl = 0;
      if(IGame *game = GetGame()) {
        if(pinfo_ptr pi = game->getPlayerInfoBySID(GetSessionID())) {
          pl = pi->getPlayer();
        }
      }
      if(pl && m_sendqueue.empty()) {
        vector2 ppos = GetProjectedPosition(pl->getPosition());
        vector4 dir4 = pl->getDirection();
        vector2 dir = vector2(dir4.x, dir4.y).normal();

        vector2 v;
        bool stick = false;
        bool mouse = false;
        if(jdir.norm()>conf.threshold2) {
          v = jdir;
          v.normalize();
          stick = true;
        }
        if((mpos-m_pmpos).norm()>4.0f) {
          v = (mpos-ppos).normal();
          v.y = -v.y;
          mouse = true;
          m_pmpos = mpos;
        }

        int r = int(fabsf(dir.cos(v))/ist::radian/6.0f);
        if(r>0 && (mouse || stick)) {
          float dot = dir.dot(v);
          float dot2 = (matrix22().rotate(90.0f)*dir).dot(v);
          int rs = ((dot>=0.0f && dot2<=0.0f) || (dot<=0.0f && dot2<=0.0f)) ? 1<<9 : 1<<8;

          int delay = GetInputClient()->getDelay();
          if(delay==0 && stick) {
            m_sendqueue.push_back(rs);
          }
          else {
            for(int i=0; i<r; ++i) {
              m_sendqueue.push_back(rs);
            }
            for(int i=0; i<delay; ++i) { // delayの分ズレるのを強引に抑止 
              m_sendqueue.push_back(0);
            }
          }
        }
      }
      if(!m_sendqueue.empty()) {
        s|=m_sendqueue.front();
        m_sendqueue.pop_front();
      }

      s|= GetMouseInput() | GetKeyboardInput() | GetJoystickInput();

      return s;
    }
  };





  class Game : public IGame
  {
  public:
    typedef intrusive_ptr<InputStateSender> isender_ptr;
    typedef intrusive_ptr<InputStream> input_ptr;
    typedef intrusive_ptr<FrameBuffer> fb_ptr;
    typedef ist::grid_structure3<gobj_ptr> gobj_grid;
    typedef std::vector<ist::task_ptr> task_cont;
    typedef boost::intrusive_ptr<ThreadSpecificMethodSync> tsms_ptr;


    class PlayerInfo : public IPlayerInfo
    {
    private:
      size_t m_session_id;
      string m_name;
      input_ptr m_input;
      player_ptr m_player;
      vector4 m_pos;
      int m_respawn_time;

      PlayerInfo(const PlayerInfo&);
      PlayerInfo& operator=(const PlayerInfo&);
    public:
      PlayerInfo(Deserializer& s)
      {
        DeserializeLinkage(s, m_player);
        s >> m_session_id >> m_name >> m_pos >> m_respawn_time;
        m_input = GetSessionByID(m_session_id)->getInput();
      }

      void serialize(Serializer& s) const
      {
        SerializeLinkage(s, m_player);
        s << m_session_id << m_name << m_pos << m_respawn_time;
      }

      void reconstructLinkage()
      {
        ReconstructLinkage(m_player);
      }

    public:
      PlayerInfo(Session& s) :
        m_session_id(s.getID()), m_name(s.getName()), m_input(s.getInput()), m_player(0), m_respawn_time(0)
      {}

      int getSessionID() { return m_session_id; }
      const string& getName() { return m_name; }
      player_ptr getPlayer() { return m_player; }
      IInput* getInput() { return m_input.get(); }

      const vector4& getPosition() { return m_pos; }
      int getRespawnTime() { return m_respawn_time; }
      void setPosition(const vector4& v) { m_pos=v; }
      void setRespawnTime(int f) { m_respawn_time=f; }

      player_ptr newPlayer()
      {
        if(!m_player) {
          m_player = CreatePlayer(*GetSessionByID(m_session_id));
          m_player->setPosition(m_pos);
        }
        m_respawn_time = 0;
        return m_player;
      }

      void updatePlayerStatus()
      {
        if(m_player) {
          m_pos = m_player->getPosition();
          if(m_player->isDead()) {
            m_player = 0;
          }
        }
        else if(m_respawn_time>0) {
          if(--m_respawn_time==0) {
            newPlayer();
            if(GetGame()->getSessionID()==getSessionID()) {
              GetSound("charge1.wav")->play(6);
            }
          }
        }
      }
    };
    typedef std::vector<pinfo_ptr> player_cont;


    // 破片群をx方向y方向にソートして高速に衝突判定するためのコンテナとタスク類 
    class fraction_set
    {
    private:
      class holder
      {
      friend class fraction_set;
      private:
        solid_ptr obj;
        gid id;
        const vector4& pos;
        size_t xindex;
        size_t yindex;

      public:
        holder(solid_ptr o) : obj(o), id(o->getID()), pos(o->getPosition()), xindex(0), yindex(0)
        {}

        holder(const vector4& p) : obj(0), id(0), pos(p), xindex(0), yindex(0)
        {}
      };
      typedef holder* holder_ptr;
      typedef std::vector<holder_ptr> holder_vector;

      holder_vector m_idorder;
      holder_vector m_xorder;
      holder_vector m_yorder;
      holder_vector m_dead;

    public:
      fraction_set()
      {}

      ~fraction_set()
      {
        clear();
      }

      void serialize(Serializer& s) const
      {
        s << m_idorder.size();
        for(size_t i=0; i<m_idorder.size(); ++i) {
          holder_ptr h = m_idorder[i];
          s << h->id << h->xindex << h->yindex;
        }
      }

      struct less_x_index {
        bool operator()(holder_ptr l, holder_ptr r) const {
          return l->xindex < r->xindex;
        }
      };
      struct less_y_index {
        bool operator()(holder_ptr l, holder_ptr r) const {
          return l->yindex < r->yindex;
        }
      };

      void deserialize(Deserializer& s)
      {
        size_t size;
        s >> size;
        for(size_t i=0; i<size; ++i) {
          gid id;
          s >> id;
          holder_ptr h = new holder(ToSolid(GetObjectByID(id)));
          s >> h->xindex >> h->yindex;
          m_idorder.push_back(h);
        }
        m_xorder = m_idorder;
        m_yorder = m_idorder;
        std::sort(m_xorder.begin(), m_xorder.end(), less_x_index());
        std::sort(m_yorder.begin(), m_yorder.end(), less_y_index());
      }

      void clear()
      {
        for(size_t i=0; i<m_idorder.size(); ++i) {
          delete m_idorder[i];
        }
        m_idorder.clear();
        m_xorder.clear();
        m_yorder.clear();
        m_dead.clear();
      }

      size_t size()
      {
        return m_idorder.size();
      }

      solid_ptr operator[](size_t i)
      {
        return m_idorder[i]->obj;
      }

      void insert(solid_ptr o)
      {
#ifdef EXCEPTION_ENABLE_RUNTIME_CHECK
        if(!IsFraction(o)) {
          throw Error("fraction_set::insert() : 非破片オブジェクト");
        }
        if(!m_idorder.empty() && m_idorder.back()->id>o->getID()) {
          throw Error("fraction_set::insert() : idが異常");
        }
#endif // EXCEPTION_ENABLE_RUNTIME_CHECK 
        m_idorder.push_back(new holder(o));
      }


      struct dead
      {
        bool operator()(holder_ptr p) const
        {
          return p->obj->isDead();
        }
      };

      void eraseDeadObject()
      {
        for(size_t i=0; i<m_idorder.size(); ++i) {
          holder_ptr h = m_idorder[i];
          if(h->obj->isDead()) {
            m_dead.push_back(h);
          }
        }
        m_idorder.erase(std::remove_if(m_idorder.begin(), m_idorder.end(), dead()), m_idorder.end());
        for(size_t i=0; i<m_dead.size(); ++i) {
          delete m_dead[i];
        }
        m_dead.clear();
      }


      struct less_x {
        bool operator()(holder_ptr l, holder_ptr r) const {
          return l->pos.x < r->pos.x;
        }
      };
      struct less_y {
        bool operator()(holder_ptr l, holder_ptr r) const {
          return l->pos.y < r->pos.y;
        }
      };

      void sortByX()
      {
        m_xorder = m_idorder;
        std::sort(m_xorder.begin(), m_xorder.end(), less_x());
        for(size_t i=0; i<m_xorder.size(); ++i) {
          m_xorder[i]->xindex = i;
        }
      }

      void sortByY()
      {
        m_yorder = m_idorder;
        std::sort(m_yorder.begin(), m_yorder.end(), less_y());
        for(size_t i=0; i<m_yorder.size(); ++i) {
          m_yorder[i]->yindex = i;
        }
      }


      void getFractions(gobj_vector& store, const vector2& ur2, const vector2& bl2)
      {
        if(m_idorder.empty()) {
          return;
        }

        size_t xbegin=0, ybegin=0, xend=0, yend=0;
        holder_vector::iterator it;

        vector4 ur(ur2.x, ur2.y, 0.0f);
        vector4 bl(bl2.x, bl2.y, 0.0f);
        holder urh(ur);
        holder blh(bl);

        it = std::lower_bound(m_xorder.begin(), m_xorder.end(), &blh, less_x());
        xbegin = it==m_xorder.end() ? m_xorder.size() : (*it)->xindex;
        it = std::lower_bound(it, m_xorder.end(), &urh, less_x());
        xend =   it==m_xorder.end() ? m_xorder.size() : (*it)->xindex;
        it = std::lower_bound(m_yorder.begin(), m_yorder.end(), &blh, less_y());
        ybegin = it==m_yorder.end() ? m_yorder.size() : (*it)->yindex;
        it = std::lower_bound(it, m_yorder.end(), &urh, less_y());
        yend =   it==m_yorder.end() ? m_yorder.size() : (*it)->yindex;

        for(size_t i=xbegin; i<xend; ++i) {
          holder_ptr h = m_xorder[i];
          if((h->yindex>=ybegin && h->yindex<=yend)) {
            store.push_back(h->obj);
         }
        }
      }

      void getFractions(gobj_vector& store, const box& bound)
      {
        const vector4& ur = bound.getUpperRight();
        const vector4& bl = bound.getBottomLeft();
        const float rad = 6.0f; // 破片の通常時の衝突判定が半径6.0f 
        getFractions(store, vector2(ur.x+rad, ur.y+rad), vector2(bl.x-rad, bl.y-rad));
      }

      void getFractions(gobj_vector& store, const sphere& bound)
      {
        const vector4& pos = bound.getPosition();
        const float rad = bound.getRadius()+6.0f;
        getFractions(store, vector2(pos.x+rad, pos.y+rad), vector2(pos.x-rad, pos.y-rad));
      }


      size_t doCollide(size_t index, ThreadSpecificMethodAsync& tsm)
      {
        size_t xbegin=0, ybegin=0, xend=0, yend=0;
        holder_vector::iterator it;

        const float rad = 6.0f*2.0f;
        holder_ptr lh = m_idorder[index];
        vector4 ur(lh->pos.x+rad, lh->pos.y+rad, 0);
        vector4 bl(lh->pos.x-rad, lh->pos.y-rad, 0);
        holder urh(ur);
        holder blh(bl);

        it = std::lower_bound(m_xorder.begin(), m_xorder.begin()+lh->xindex, &blh, less_x());
        xbegin = it==m_xorder.end() ? m_xorder.size() : (*it)->xindex;
        it = std::lower_bound(m_xorder.begin()+lh->xindex, m_xorder.end(), &urh, less_x());
        xend =   it==m_xorder.end() ? m_xorder.size() : (*it)->xindex;

        it = std::lower_bound(m_yorder.begin(), m_yorder.begin()+lh->yindex, &blh, less_y());
        ybegin = it==m_yorder.end() ? m_yorder.size() : (*it)->yindex;
        it = std::lower_bound(m_yorder.begin()+lh->yindex, m_yorder.end(), &urh, less_y());
        yend =   it==m_yorder.end() ? m_yorder.size() : (*it)->yindex;

        size_t count = 0;
        for(size_t i=xbegin; i<xend; ++i) {
          holder_ptr rh = m_xorder[i];
          if((rh->yindex>=ybegin && rh->yindex<=yend) && rh->id>lh->id) {
            solid_ptr l = lh->obj;
            solid_ptr r = rh->obj;
            cdetector cd;
            if(cd.detect(l->getCollision(), r->getCollision())) {
              tsm.sendCollideMessage(l, r, cd.getPosition(), cd.getNormal(), cd.getDistance());
              tsm.sendCollideMessage(r, l, cd.getPosition(),-cd.getNormal(), cd.getDistance());
              ++count;
            }
          }
        }
        return count;
      }
    };

    class FractionSortByXTask : public ist::Task
    {
    private:
      fraction_set& m_objs;
    public:
      FractionSortByXTask(fraction_set& objs) : m_objs(objs) {}
      void operator()()
      {
        m_objs.sortByX();
      }
    };

    class FractionSortByYTask : public ist::Task
    {
    private:
      fraction_set& m_objs;
    public:
      FractionSortByYTask(fraction_set& objs) : m_objs(objs) {}
      void operator()()
      {
        m_objs.sortByY();
      }
    };






    class AsyncUpdateTask : public ist::Task
    {
    private:
      Game *m_game;

    public:
      AsyncUpdateTask(Game *g) : m_game(g) {}
      void operator()()
      {
        m_game->asyncupdate();
      }
    };


    class SolidTask : public ist::Task
    {
    protected:
      ThreadSpecificMethodAsync m_tsm;
      size_t m_begin;
      size_t m_end;

    public:
      SolidTask() : m_begin(0), m_end(0) {}

      virtual void setRange(size_t begin, size_t end)
      {
        m_begin = begin;
        m_end = end;
      }

      void flushMessage()
      {
        m_tsm.flushMessage();
      }

      virtual string p()
      {
        return "";
      }
    };

    class CollideTask : public SolidTask
    {
    private:
      solid_vector& m_objs;
      IGame *m_game;

    public:
      CollideTask(solid_vector& objs) : m_objs(objs)
      {
        m_game = GetGame();
      }

      void setRange(size_t begin, size_t end)
      {
        SolidTask::setRange(begin, std::min<size_t>(end, m_objs.size()));
      }

      void operator()()
      {
        for(size_t i=m_begin; i<m_end; ++i) {
          solid_ptr l = m_objs[i];
          gid lid = l->getID();
          gobj_iter& it = m_tsm.getObjects(l->getCollision().getBoundingBox());
          while(it.has_next()) {
            if(solid_ptr r=ToSolid(it.iterate())) {
              if(IsFraction(r) || lid>r->getID()) {
                doCollisionDetection(l, r);
              }
            }
          }
        }
      }

      void doCollisionDetection(solid_ptr l, solid_ptr r)
      {
        gid group = l->getGroup();
        if(!group || group!=r->getGroup()) {
          cdetector cd;
          const collision& lc = l->getCollision();
          const collision& rc = r->getCollision();
          if(l->getVolume() < r->getVolume() && cd.detect(lc, rc)) {
            m_tsm.sendCollideMessage(l, r, cd.getPosition(), cd.getNormal(), cd.getDistance());
            m_tsm.sendCollideMessage(r, l, cd.getPosition(),-cd.getNormal(), cd.getDistance());
          }
          else if(cd.detect(rc, lc)) {
            m_tsm.sendCollideMessage(l, r, cd.getPosition(),-cd.getNormal(), cd.getDistance());
            m_tsm.sendCollideMessage(r, l, cd.getPosition(), cd.getNormal(), cd.getDistance());
          }
        }
      }
    };

    class FractionCollideTask : public SolidTask
    {
    private:
      fraction_set& m_objs;

    public:
      FractionCollideTask(fraction_set& objs) : m_objs(objs)
      {}

      void setRange(size_t begin, size_t end)
      {
        SolidTask::setRange(begin, std::min<size_t>(end, m_objs.size()));
      }

      void operator()()
      {
        for(size_t i=m_begin; i<m_end; ++i) {
          m_objs.doCollide(i, m_tsm);
        }
      }
    };

    class FractionUpdateTask : public SolidTask
    {
    private:
      gobj_vector& m_objs;

    public:
      FractionUpdateTask(gobj_vector& objs) : m_objs(objs)
      {}

      void setRange(size_t begin, size_t end)
      {
        SolidTask::setRange(begin, std::min<size_t>(end, m_objs.size()));
      }

      void operator()()
      {
        for(size_t i=m_begin; i<m_end; ++i) {
          doAsyncupdate(m_objs[i]);
        }
      }

      void doAsyncupdate(gobj_ptr obj)
      {
        m_tsm.setCurrent(obj);
        obj->setTSM(m_tsm);
        obj->asyncupdate();
      }
    };


#ifdef EXCEPTION_ENABLE_PROFILE

    class ProfileBeginTask : public ist::Task
    {
    private:
      Uint32 m_tick;

    public:
      ProfileBeginTask(const boost::thread::id& tid) : m_tick(0)
      {
        setAffinity(tid);
      }

      Uint32 getTicks()
      {
        return m_tick;
      }

      void operator()()
      {
        m_tick = sgui::GetTicks();
      }
    };

    class ProfileEndTask : public ist::Task
    {
    private:
      ProfileBeginTask& m_begin;

    public:
      ProfileEndTask(const boost::thread::id& tid, ProfileBeginTask& begin) : m_begin(begin)
      {
        setAffinity(tid);
      }

      void operator()()
      {
        AddThreadTime(boost::this_thread::get_id(), sgui::GetTicks()-m_begin.getTicks());
      }
    };

#endif // EXCEPTION_ENABLE_PROFILE 


    static Game* instance() { return s_instance; }

  private:
    static Game *s_instance;

    isender_ptr m_sender;

    bool m_synchronized;
    ist::Scheduler *m_scheduler;
    ist::task_ptr m_asyncupdate_task;
    task_cont m_sort_fraction_tasks;
    task_cont m_collide_tasks;
    task_cont m_fcollide_tasks;
    task_cont m_fraction_tasks;
#ifdef EXCEPTION_ENABLE_PROFILE
    task_cont m_profile_begin_tasks;
    task_cont m_profile_end_tasks;
#endif // EXCEPTION_ENABLE_PROFILE 
    tsms_ptr m_main_tsm;

    gobj_grid m_grid;
    gobj_vector m_objs;       // 全オブジェクト 
    gobj_vector m_garbage;    // 死んだオブジェクト。「死んでる」という状態を外から確認可能にするため、消去は1フレーム待つ 
    gobj_vector m_pre_garbage;// これから消去するオブジェクト 
    gobj_vector m_draw;       // 描画用リスト 
    gobj_vector m_new_objs;   // そのフレーム内で追加されたオブジェクト 
    solid_vector m_new_fractions;
    solid_vector m_solids;    // 衝突判定を持つ破片以外のオブジェクト 
    fraction_set m_fractions; // 破片オブジェクト 
    player_cont m_players;
    rule_ptr m_rule;

    std::pair<size_t, fb_ptr> m_ffb;
    std::pair<size_t, fb_ptr> m_bfb;
    matrix44 m_aimmatrix;
    ist::PerspectiveCamera m_cam;
    ist::Light m_light;
    vector2 m_cam_ur;
    vector2 m_cam_bl;
    int m_cam_follow;

    size_t m_past;
    int m_total_fps;
    int m_total_sec;

    bool m_pause;
    bool m_skip;
    bool m_step;

    ist::Random m_rand;
    GameOption m_initial_opt;
    GameOption m_opt;

  public:
    Game(const GameOption& opt) :
      m_synchronized(true),
      m_rule(0),
      m_cam_follow(0),
      m_past(0), m_total_fps(0), m_total_sec(0),
      m_pause(false), m_skip(false), m_step(false),
      m_rand(opt.seed), m_initial_opt(opt), m_opt(opt)
    {
      initialize();

      if     (m_opt.stage==MAP_DEATHMATCH) {
        m_rule = CreateDeathMatch(opt.deathmatch_time);
      }
      else if(m_opt.stage==MAP_TEAMFORTRESS) {
        m_rule = CreateTeamFortress(opt.teamfortress_life);
      }
      else if(m_opt.stage==MAP_HORDE) {
        m_rule = CreateHorde(opt.horde_wave-1);
      }

      for(int i=0; i<GetSessionCount(); ++i) {
        session_ptr s = GetSession(i);
        if(s->getJoinFrame()==0) {
          join(s->getID());
        }
        if(IsReplayMode() && i==0) {
          m_cam_follow = s->getID();
        }
      }

      PutBloom();
    }

    Game(Deserializer& s) : m_synchronized(true)
    {
      try {
        deserialize(s);
      }
      catch(Error& e) {
        puts(e.what());
        fflush(stdout);
        clearAllObjects();
        s_instance = 0;
        throw e;
      }
    }

    void deserialize(Deserializer& s)
    {
      size_t size;
      string name;
      s >> name;
      if(name==typeid(InputClientReplay).name()) {
        g_iclient.reset(new InputClientReplay());
      }
      else if(name==typeid(InputClientLocal).name()) {
        g_iclient.reset(new InputClientLocal());
      }
      g_iclient->deserialize(s);

      initialize();

      s >> m_aimmatrix >> m_cam >> m_light >> m_cam_ur >> m_cam_bl >> m_cam_follow
        >> m_past >> m_total_fps >> m_total_sec
        >> m_pause >> m_skip >> m_step >> m_rand >> m_initial_opt >> m_opt;

      DeserializeLinkage(s, m_rule);
      s >> size;
      for(size_t i=0; i<size; ++i) { m_players.push_back(new PlayerInfo(s)); }
      s >> size;
      for(size_t i=0; i<size; ++i) { m_objs.push_back(DeserializeObject(s)); }
      s >> size;
      for(size_t i=0; i<size; ++i) { m_pre_garbage.push_back(DeserializeObject(s)); }

      s >> size;
      for(size_t i=0; i<size; ++i) { m_draw.push_back(getObject(s)); }
      s >> size;
      for(size_t i=0; i<size; ++i) { m_solids.push_back(ToSolid(getObject(s))); }
      for(size_t i=0; i<m_grid.size(); ++i) {
        gobj_grid::block_type& b = m_grid.getBlock(i);
        s >> size;
        for(size_t j=0; j<size; ++j) { b.push_back(getObject(s)); }
      }
      m_fractions.deserialize(s);

      ReconstructLinkage(m_rule);
      for(size_t i=0; i<m_players.size(); ++i)     { m_players[i]->reconstructLinkage(); }
      for(size_t i=0; i<m_objs.size(); ++i)        { m_objs[i]->reconstructLinkage(); }
      for(size_t i=0; i<m_pre_garbage.size(); ++i) { m_pre_garbage[i]->reconstructLinkage(); }

      IMusic::Deserialize(s);

      if(m_pause) {
        Pause();
      }
      if(IsClientMode()) {
        m_cam_follow = GetSessionID();
      }
    }


    void initialize()
    {
      if(s_instance) { // インスタンスが2つある！ 
        throw Error("Game::Game()");
      }
      s_instance = this;

      m_sender = new InputStateSender();
      m_main_tsm.reset(new ThreadSpecificMethodSync());

      onThreadCountChange();
      m_asyncupdate_task.reset(new AsyncUpdateTask(this));
      m_sort_fraction_tasks.push_back(ist::task_ptr(new FractionSortByXTask(m_fractions)));
      m_sort_fraction_tasks.push_back(ist::task_ptr(new FractionSortByYTask(m_fractions)));

      m_grid.resize(vector4(1500.0f), vector4(-1500.0f), 100, 100, 1);
      m_cam.setPosition(vector4(0.0f, 0.0f, 500.0f));
      m_cam.setTarget(vector4(0.0f, 0.0f, 0.0f));
      m_cam.setFovy(60.0f);
      m_cam.setZFar(10000.0f);
      m_cam_follow = GetSessionID();

      m_ffb.second = new FrameBuffer(GL_FRONT);
      m_bfb.second = new FrameBuffer(GL_BACK);

      m_light.setPosition(vector4(1000.0f, 2000.0f, 1000.0f));
    }


    ~Game()
    {
      clearAllObjects();
      s_instance = 0;
    }

    void serialize(Serializer& s) const
    {
      m_scheduler->waitFor(m_asyncupdate_task);

      s << string(typeid(*g_iclient).name());
      g_iclient->serialize(s);

      s << m_aimmatrix << m_cam << m_light << m_cam_ur << m_cam_bl << m_cam_follow
        << m_past << m_total_fps << m_total_sec
        << m_pause << m_skip << m_step << m_rand << m_initial_opt << m_opt;

      SerializeLinkage(s, m_rule);
      s << m_players.size();
      for(size_t i=0; i<m_players.size(); ++i) { m_players[i]->serialize(s); }
      s << m_objs.size();
      for(size_t i=0; i<m_objs.size(); ++i) { SerializeObject(s, m_objs[i]); }
      s << m_pre_garbage.size();
      for(size_t i=0; i<m_pre_garbage.size(); ++i) { SerializeObject(s, m_pre_garbage[i]); }

      s << m_draw.size();
      for(size_t i=0; i<m_draw.size(); ++i) { s << ToID(m_draw[i]); }
      s << m_solids.size();
      for(size_t i=0; i<m_solids.size(); ++i) { s << ToID(m_solids[i]); }
      for(size_t i=0; i<m_grid.size(); ++i) {
        const gobj_grid::block_type& b = m_grid.getBlock(i);
        s << b.size();
        for(size_t j=0; j<b.size(); ++j) { s << ToID(b[j]); }
      }
      m_fractions.serialize(s);

      IMusic::Serialize(s);
    }

    void join(size_t sid)
    {
      for(size_t i=0; i<m_players.size(); ++i) { // 参加済みなら無視 
        if(m_players[i]->getSessionID()==sid) {
          return;
        }
      }

      session_ptr s = GetSessionByID(sid);
      pinfo_ptr ps = new PlayerInfo(*s);
      m_players.push_back(ps);
      if(m_rule) {
        m_rule->join(ps);
      }
      if(getPast()==0 || m_players.size()==1) {
        ps->newPlayer();
      }
    }

    void leave(size_t sid)
    {
      pinfo_ptr pi;
      for(player_cont::iterator p=m_players.begin(); p!=m_players.end(); ++p) {
        if((*p)->getSessionID()==sid) {
          pi = *p;
          m_main_tsm->sendKillMessage(0, (*p)->getPlayer());
          m_players.erase(p);
          break;
        }
      }
      if(m_rule && pi) {
        m_rule->leave(pi);
      }
    }


    void clearAllObjects()
    {
      m_scheduler->waitForAll();
      m_synchronized = true;

      for(size_t i=0; i<m_objs.size(); ++i) {
        gobj_ptr obj = m_objs[i];
        m_main_tsm->sendKillMessage(0, obj);
        obj->processMessageQueue();
      }
      for(gobj_vector::iterator p=m_pre_garbage.begin(); p!=m_pre_garbage.end(); ++p) {
        (*p)->release();
      }
      for(gobj_vector::iterator p=m_garbage.begin(); p!=m_garbage.end(); ++p) {
        (*p)->release();
      }
      for(gobj_vector::iterator p=m_objs.begin(); p!=m_objs.end(); ++p) {
        (*p)->release();
      }
      m_pre_garbage.clear();
      m_garbage.clear();
      m_objs.clear();

      m_light.disable();
    }


    IThreadSpecificMethod& getTSM() { return *m_main_tsm; }
    void onThreadCountChange()
    {
      m_scheduler = ist::Scheduler::instance();

#ifdef EXCEPTION_ENABLE_PROFILE
      m_profile_begin_tasks.clear();
      m_profile_end_tasks.clear();

      std::vector<boost::thread::id> tids;
      tids.push_back(boost::this_thread::get_id());
      for(size_t i=0; i<m_scheduler->getThreadCount(); ++i) {
        tids.push_back(m_scheduler->getThreadID(i));
      }
      for(size_t i=0; i<tids.size(); ++i) {
        ProfileBeginTask *pbt = new ProfileBeginTask(tids[i]);
        ProfileEndTask *pet = new ProfileEndTask(tids[i], *pbt);
        m_profile_begin_tasks.push_back(ist::task_ptr(pbt));
        m_profile_end_tasks.push_back(ist::task_ptr(pet));
      }
#endif // EXCEPTION_ENABLE_PROFILE 
    }



    void returnToTitle()
    {
      ResetGlobals();
      SendNMessage(NMessage::End());
    }

    void write()
    {
      if(IsReplayMode()) {
        return;
      }

      char buf[64];
      sprintf(buf, "record/%d.dat", int(::time(0)));
      ist::gzbstream s(buf, "wb");
      if(!s) {
        return;
      }

      int version = EXCEPTION_REPLAY_VERSION;
      int fps = int(float(m_total_fps)/float(m_total_sec))+1;
      int playtime = getPast();
      s << version << m_initial_opt << fps << playtime;

      g_iclient->write(s);
    }

    // 可動範囲に合わせてカメラにプレイヤーを追わせる 
    void cameraFollowPlayer()
    {
      vector4 ppos;
      for(size_t i=0; i<m_players.size(); ++i) {
        if(m_players[i]->getSessionID()==m_cam_follow) {
          ppos = m_players[i]->getPosition();
        }
      }

      vector4 cam_pos = m_cam.getPosition();
      vector4 cam_pre_pos = cam_pos;
      vector2 cam_xy;
      cam_xy.x = std::min<float>(m_cam_ur.x, std::max<float>(m_cam_bl.x, ppos.x));
      cam_xy.y = std::min<float>(m_cam_ur.y, std::max<float>(m_cam_bl.y, ppos.y));
      cam_pos.x+=(cam_xy.x-cam_pos.x)*0.025f;
      cam_pos.y+=(cam_xy.y-cam_pos.y)*0.025f;
      m_cam.setPosition(cam_pos);

      vector4 cam_tar = m_cam.getTarget();
      cam_tar+=cam_pos-cam_pre_pos;
      m_cam.setTarget(cam_tar);
    }


    // アップデート処理 
    // リプレイ時の倍速を楽に実現するため、内部処理の実体は progress() に丸投げ 
    void update()
    {
      int x = 1;
      if(IsReplayMode()) {
        ushort pstate = m_sender->getPreState();
        ushort state = m_sender->getState();
        if(state&(1<<(4))) { x=2; }
        if(state&(1<<(5))) { x=4; }
        if(state&(1<<(7))) { x=8; }

        // 上下キーでカメラのフォロー先を変更 
        if(!(state & InputStream::UP) && (pstate & InputStream::UP)) {
          for(size_t i=0; i<m_players.size(); ++i) {
            if(m_players[i]->getSessionID()==m_cam_follow) {
              m_cam_follow = m_players[(i+1)%m_players.size()]->getSessionID();
              break;
            }
            else if(i==m_players.size()-1) {
              m_cam_follow = m_players.front()->getSessionID();
            }
          }
        }
        if(!(state & InputStream::DOWN) && (pstate & InputStream::DOWN)) {
          for(size_t i=0; i<m_players.size(); ++i) {
            if(m_players[i]->getSessionID()==m_cam_follow) {
              m_cam_follow = m_players[i==0 ? m_players.size()-1 : i-1]->getSessionID();
              break;
            }
            else if(i==m_players.size()-1) {
              m_cam_follow = m_players.back()->getSessionID();
            }
          }
        }
      }
      for(int i=0; i<x; ++i) {
        progress();
      }
    }

    // アップデート処理の実体 
    void progress()
    {
      static size_t s_update_begin;
      static size_t s_elapsed;

      m_scheduler->waitFor(m_asyncupdate_task);
#ifdef EXCEPTION_ENABLE_PROFILE
      m_scheduler->waitFor(m_profile_end_tasks.begin(), m_profile_end_tasks.end());
#endif // EXCEPTION_ENABLE_PROFILE 
      m_main_tsm->resetMessageCache();
      m_synchronized = true;
      m_skip = false;

      s_elapsed = sgui::GetTicks()-s_update_begin;

      GetInputClient()->sync();

      s_update_begin = sgui::GetTicks();

      if(isPaused()) {
        if(m_step) {
          m_step = false;
        }
        else {
          return;
        }
      }
      if(m_skip) {
        return;
      }

      // 平均FPS計測処理 
      {
        static int s_pre_sec = 0;
        static int s_fps = 0;
        ++s_fps;
        size_t sec = size_t(::time(0));
        if(s_pre_sec!=sec) {
          m_total_fps+=s_fps;
          m_total_sec++;
          s_pre_sec = sec;
          s_fps = 0;
        }
      }

#ifdef EXCEPTION_ENABLE_PROFILE
      Uint32 t = sgui::GetTicks();
#endif

      // リプレイ再生終了時はタイトルに戻る 
      if(IsReplayMode()) {
        InputClientReplay& ic = dynamic_cast<InputClientReplay&>(*GetInputClient());
        if(getPast()==ic.getLength()) {
          setPause(true);
          returnToTitle();
        }
      }
      // 1秒毎にゲーム状態を送信 
      if(getPast()%60==0) {
        static Uint32 s_start_time;
        static Uint32 s_pre_time;
        static Uint32 s_time;
        if(getPast()==0) {
          s_start_time = sgui::GetTicks();
        }
        if(getPast()%60==0) {
          s_pre_time = s_time;
          s_time = sgui::GetTicks();
        }

        NMessage m;
        m.type = NMessage::GSTAT;
        m.gstat.elapsed = sgui::GetTicks()-s_start_time;
        m.gstat.elapsed_par_frame = s_time-s_pre_time;
        m.gstat.obj = Object::getCount();
        m.gstat.fps = m_total_fps/m_total_sec;
        m.gstat.delay = g_iclient->getDelay();
        m.gstat.players = m_players.size();
        m.gstat.alives = 0;
        for(int i=0; i<m_players.size(); ++i) {
          if(m_players[i]->getPlayer()) {
            ++m.gstat.alives;
          }
        }
        SendNMessage(m);
      }
      // 10秒毎に同期チェック情報を送信 
      if(getPast()%600==0) {
        NMessage m;
        m.type = NMessage::SYNC;
        m.sync.frame = getPast();
        for(int i=0; i<16; ++i) {
          if(i<m_players.size()) {
            player_ptr pl = m_players[i]->getPlayer();
            m.sync.life[i] = pl ? (short)pl->getLife() : 0;
          }
          else {
            m.sync.life[i] = 0;
          }
        }
        SendNMessage(m);
      }

      m_cam.look();
      m_sender->update();
      SendNMessage(NMessage::Input(m_sender->getState(), s_elapsed, getPast()));
      GetInputClient()->update();
      m_past+=1;


      // プレイヤーの生死チェック 
      for(size_t i=0; i<m_players.size(); ++i) {
        static_cast<PlayerInfo&>(*m_players[i]).updatePlayerStatus();
      }
      SweepDeadObject(m_rule);

      // カメラ追随 
      cameraFollowPlayer();
      setAimCameraMatrix(matrix44().aimVector(m_cam.getPosition()-m_cam.getTarget()));
      m_cam.look();

      // 全オブジェクトupdate 
      for(size_t i=0; i<m_objs.size(); ++i) {
        gobj_ptr obj = m_objs[i];

        solid_ptr sp = ToSolid(obj);
        bool update_grid = sp && !IsFraction(obj);
        if(update_grid) {
          m_grid.erase(sp->getCollision().getBoundingBox(), obj);
        }

        m_main_tsm->sendUpdateMessage(0, obj);
        obj->setTSM(*m_main_tsm);
        obj->update();

        if(obj->isDead()) {
          m_garbage.push_back(obj);
        }
        else {
          if(update_grid) {
            m_grid.insert(sp->getCollision().getBoundingBox(), obj);
          }
        }
      }

      m_objs.erase(std::remove_if(m_objs.begin(), m_objs.end(), is_dead()), m_objs.end());
      m_draw.erase(std::remove_if(m_draw.begin(), m_draw.end(), is_dead()), m_draw.end());
      m_solids.erase(std::remove_if(m_solids.begin(), m_solids.end(), is_dead()), m_solids.end());

      // 前のフレームで死んだオブジェクトをrelease 
      for(size_t i=0; i<m_pre_garbage.size(); ++i) {
        m_pre_garbage[i]->release();
      }
      // このフレームで死んだオブジェクトを退避 
      m_pre_garbage = m_garbage;
      m_garbage.clear();


      // このフレームで追加されたオブジェクトを各種リストに追加 
      for(size_t i=0; i<m_new_objs.size(); ++i) {
        gobj_ptr p = m_new_objs[i];
        if(p->getDrawPriority()>=0.0f) {
          m_draw.push_back(p);
        }
        if(solid_ptr s=ToSolid(p)) {
          if(IsFraction(s)) {
            m_new_fractions.push_back(s);
          }
          else {
            m_solids.push_back(s);
          }
        }
      }
      m_new_objs.clear();

#ifdef EXCEPTION_ENABLE_PROFILE
      AddUpdateTime(float(sgui::GetTicks()-t));
#endif

      m_synchronized = false;
      m_scheduler->schedule(m_asyncupdate_task);
    }


    void asyncupdate()
    {
#ifdef EXCEPTION_ENABLE_PROFILE
      m_scheduler->schedule(m_profile_begin_tasks.begin(), m_profile_begin_tasks.end());
#endif // EXCEPTION_ENABLE_PROFILE 

      static task_cont s_tasks;
      // 衝突判定 
      {
        size_t div = 300;
        size_t num_tasks = m_solids.size()/div + (m_solids.size()%div==0 ? 0 : 1);
        while(m_collide_tasks.size()<num_tasks) {
          m_collide_tasks.push_back(ist::task_ptr(new CollideTask(m_solids)));
        }
        for(size_t i=0; i<num_tasks; ++i) {
          static_cast<SolidTask&>(*m_collide_tasks[i]).setRange(div*i, div*(i+1));
          s_tasks.push_back(m_collide_tasks[i]);
        }
        m_scheduler->schedule(m_collide_tasks.begin(), m_collide_tasks.begin()+num_tasks);
      }
      {
        size_t div = 300;
        size_t num_tasks = m_fractions.size()/div + (m_fractions.size()%div==0 ? 0 : 1);
        while(m_fcollide_tasks.size()<num_tasks) {
          m_fcollide_tasks.push_back(ist::task_ptr(new FractionCollideTask(m_fractions)));
        }
        for(size_t i=0; i<num_tasks; ++i) {
          static_cast<SolidTask&>(*m_fcollide_tasks[i]).setRange(div*i, div*(i+1));
          s_tasks.push_back(m_fcollide_tasks[i]);
        }
        m_scheduler->schedule(m_fcollide_tasks.begin(), m_fcollide_tasks.begin()+num_tasks);
      }
      m_scheduler->waitFor(s_tasks.begin(), s_tasks.end());
      for(size_t i=0; i<s_tasks.size(); ++i) {
        static_cast<SolidTask&>(*s_tasks[i]).flushMessage();
      }
      s_tasks.clear();


      // オブジェクト単位の非同期更新 
      {
        size_t div = 300;
        size_t num_tasks = m_objs.size()/div + (m_objs.size()%div==0 ? 0 : 1);
        while(m_fraction_tasks.size()<num_tasks) {
          m_fraction_tasks.push_back(ist::task_ptr(new FractionUpdateTask(m_objs)));
        }
        for(size_t i=0; i<num_tasks; ++i) {
          static_cast<SolidTask&>(*m_fraction_tasks[i]).setRange(div*i, div*(i+1));
        }
        m_scheduler->schedule(m_fraction_tasks.begin(), m_fraction_tasks.begin()+num_tasks);
        m_scheduler->waitFor(m_fraction_tasks.begin(), m_fraction_tasks.begin()+num_tasks);
        for(size_t i=0; i<num_tasks; ++i) {
          static_cast<SolidTask&>(*m_fraction_tasks[i]).flushMessage();
        }
      }

      for(size_t i=0; i<m_new_fractions.size(); ++i) {
        m_fractions.insert(m_new_fractions[i]);
      }
      m_new_fractions.clear();
      m_fractions.eraseDeadObject();
      m_scheduler->schedule(m_sort_fraction_tasks.begin(), m_sort_fraction_tasks.end());
      m_scheduler->waitFor(m_sort_fraction_tasks.begin(), m_sort_fraction_tasks.end());

#ifdef EXCEPTION_ENABLE_PROFILE
      m_scheduler->schedule(m_profile_end_tasks.begin(), m_profile_end_tasks.end());
#endif // EXCEPTION_ENABLE_PROFILE 
    }

    void exit()
    {
      clearAllObjects();
    }


    void draw()
    {
#ifdef EXCEPTION_ENABLE_PROFILE
      Uint32 t = sgui::GetTicks();
#endif
      m_cam.look();
      m_light.enable();
      std::sort(m_draw.begin(), m_draw.end(), greater_draw_priority());
      for(gobj_vector::iterator p=m_draw.begin(); p!=m_draw.end(); ++p) {
        (*p)->draw();
      }
      m_light.disable();
      drawStat();
#ifdef EXCEPTION_ENABLE_PROFILE
      AddDrawTime(float(sgui::GetTicks()-t));
#endif
    }

    void drawStat()
    {
      ScreenMatrix sm;
      glDisable(GL_LIGHTING);
      glDisable(GL_DEPTH_TEST);

      char buf[128];

      if(IsReplayMode()) {
        size_t len = dynamic_cast<InputClientReplay&>(*g_iclient).getLength();
        size_t pos = getPast();
        sprintf(buf, "%d:%02d / %d:%02d", pos/3600, pos/60%60, len/3600, len/60%60);
        DrawText(_L(buf), sgui::Point(280,5));
        if(getPlayerCount()>1) {
          glColor4f(1,1,1,0.5f);
          DrawText(L"up|down: change player", sgui::Point(260,460));
          glColor4f(1,1,1,1);
        }
      }
    }

    void insertObject(gobj_ptr p)
    {
#ifdef EXCEPTION_ENABLE_RUNTIME_CHECK
      if(std::lower_bound(m_objs.begin(), m_objs.end(), p, less_id())!=m_objs.end()) {
        throw Error(std::string("insertObject():")+typeid(*p).name()+" 多重Register");
      }
      if(!m_objs.empty() && m_objs.back()->getID()>=p->getID()) {
        throw Error(std::string("insertObject():")+typeid(*p).name()+"登録順が異常");
      }
#endif
      m_objs.push_back(p);
      m_new_objs.push_back(p);
    }


    struct less_id2
    {
      gid m_id;
      less_id2(gid id) : m_id(id) {}
      bool operator()(gobj_ptr l, gobj_ptr r) const
      {
#ifdef _DEBUG // VisualC++のデバッグ版STL対策 
        if(l && r) {
          return l->getID() < r->getID();
        }
        else if(!r) {
          return l->getID() < m_id;
        }
        else {
          return m_id < r->getID();
        }
      }
#else
      return l->getID() < m_id;
#endif
      }
    };

    gobj_ptr getObject(Deserializer& s)
    {
      gid id;
      s >> id;
      return getObject(id);
    }

    gobj_ptr getObject(gid id)
    {
      if(id==0) {
        return gobj_ptr();
      }

      gobj_vector::iterator p;
      p = std::lower_bound(m_objs.begin(), m_objs.end(), gobj_ptr(0), less_id2(id));
      if(p!=m_objs.end() && (*p)->getID()==id) {
        return *p;
      }
      p = std::lower_bound(m_pre_garbage.begin(), m_pre_garbage.end(), gobj_ptr(0), less_id2(id));
      if(p!=m_pre_garbage.end() && (*p)->getID()==id) {
        return *p;
      }
      return gobj_ptr();
    }



    struct gobj_gather
    {
      mutable gobj_vector& store;

      gobj_gather(gobj_vector& s) : store(s) {}
      void operator()(gobj_grid::block_type& b) const
      {
        store.insert(store.end(), b.begin(), b.end());
      }
    };

    void getObjects(gobj_vector& store, const box& box)
    {
      store.clear();
      m_grid.eachBlocks(box, gobj_gather(store));
      std::sort(store.begin(), store.end(), less_id());
      store.erase(std::unique(store.begin(), store.end(), equal_id()), store.end());

      m_fractions.getFractions(store, box);
    }

    void getObjects(gobj_vector& store, const sphere& sphere)
    {
      store.clear();
      m_grid.eachBlocks(sphere, gobj_gather(store));
      std::sort(store.begin(), store.end(), less_id());
      store.erase(std::unique(store.begin(), store.end(), equal_id()), store.end());

      m_fractions.getFractions(store, sphere);
    }

    gobj_vector& getAllObjects()
    {
      return m_objs;
    }


    rule_ptr getRule()
    {
      return m_rule;
    }

    size_t getPlayerCount()
    {
      return m_players.size();
    }

    pinfo_ptr getPlayerInfo(size_t index)
    {
      return m_players[index];
    }

    pinfo_ptr getPlayerInfoBySID(size_t session_id)
    {
      for(size_t i=0; i<m_players.size(); ++i) {
        if(m_players[i]->getSessionID()==session_id) {
          return m_players[i];
        }
      }
      return pinfo_ptr();
    }


    IFrameBuffer& getFrontFrameBuffer()
    {
      if(m_ffb.first!=m_past) {
        m_ffb.first = m_past;
        m_ffb.second->copy();
      }
      return *m_ffb.second;
    }

    IFrameBuffer& getBackFrameBuffer()
    {
      if(m_bfb.first!=m_past) {
        m_bfb.first = m_past;
        m_bfb.second->copy();
      }
      return *m_bfb.second;
    }
    ist::Light& getLight() { return m_light; }
    ist::PerspectiveCamera& getCamera() { return m_cam; }
    const ist::PerspectiveCamera& getCamera() const { return m_cam; }
    const matrix44& getAimCameraMatrix() { return m_aimmatrix; }
    void setAimCameraMatrix(const matrix44& v) { m_aimmatrix=v; }

    void setCameraMovableArea(const vector2& ur, const vector2& bl)
    {
      m_cam_ur = ur;
      m_cam_bl = bl;
    }

    size_t getPast() { return m_past; }
    float getRand() { return float(m_rand.genReal()); }


    int getLevel() { return m_opt.level; }
    float getCatapultBoost() { return m_opt.cboost; }
    float getFractionBoost() { return m_opt.fboost; }
    float getEnemyBoost()    { return m_opt.eboost; }

    void setLevel(int v)
    {
      if(getLevel()!=v && v>=LEVEL_LIGHT && v<=LEVEL_FUTURE) {
        m_opt.level = v;
        PushChatText(string("# level changed: ")+GetLevelString(), sgui::Color(0.5f, 0.5f, 0.5f, 0.9f));
      }
    }
    void setCatapultBoost(float v)
    {
      if(getCatapultBoost()!=v && v>=0.0f) {
        v = trim<float>(0.0f, v, 3.0f);
        m_opt.cboost = v;
        PushChatText(string("# catapult boost changed: ")+sgui::Format("%.2f", v), sgui::Color(0.5f, 0.5f, 0.5f, 0.9f));
      }
    }

    void setFractionBoost(float v)
    {
      if(getFractionBoost()!=v && v>=0.0f) {
        v = trim<float>(0.0f, v, 3.0f);
        m_opt.fboost = v;
        PushChatText(string("# fraction boost changed: ")+sgui::Format("%.2f", v), sgui::Color(0.5f, 0.5f, 0.5f, 0.9f));
      }
    }

    void setEnemyBoost(float v)
    {
      if(getEnemyBoost()!=v && v>=0.0f) {
        v = trim<float>(0.0f, v, 3.0f);
        m_opt.eboost = v;
        PushChatText(string("# enemy boost changed: ")+sgui::Format("%.2f", v), sgui::Color(0.5f, 0.5f, 0.5f, 0.9f));
      }
    }


    size_t getSessionID() { return m_cam_follow; }
    IInput* getInput(size_t session_id) { return GetSessionByID(session_id)->getInput(); }


    void setPause(bool f) { m_pause=f; }
    void setSkip(bool f) { m_skip=f; }
    bool isPaused() { return m_pause; }


    void step() { m_step=true; }
    bool isSynchronized() const { return m_synchronized; }

    string p()
    {
      string r;
      char buf[256];
      sprintf(buf, "frame: %d\n", m_past);
      r+=buf;
      sprintf(buf, "avg. fps: %d\n", int(float(m_total_fps)/float(m_total_sec))+1);
      r+=buf;
      sprintf(buf, "object: %d\n", m_objs.size());
      r+=buf;
      sprintf(buf, "solid: %d\n", m_solids.size());
      r+=buf;
      sprintf(buf, "fraction: %d\n", m_fractions.size());
      r+=buf;

      return r;
    }

    string pDetail()
    {
      string r = p();
      r+="cache:\n";
      for(size_t i=0; i<CacheInfo::getCacheInfoCount(); ++i) {
        r+="  ";
        r+=CacheInfo::getCacheInfo(i).p();
      }
      r+="\n";
      for(size_t i=0; i<m_objs.size(); ++i) {
        gobj_ptr o = m_objs[i];
        if(!IsFraction(o)) {
          r+=o->p();
          r+="\n";
        }
      }
      return r;
    }
  };

  Game* Game::s_instance = 0;
  IGame* GetGame() { return Game::instance(); }

  IGame* CreateGame(const GameOption& opt)
  {
    sgui::View::instance()->fitViewport();
    return new Game(opt);
  }

  IGame* CreateGame(Deserializer& s)
  {
    sgui::View::instance()->fitViewport();
    return new Game(s);
  }

} // exception 
