#ifndef player_h
#define player_h

namespace exception {




  class LockOnMarker : public Particle
  {
  typedef Particle Super;
  public:
    typedef TCache<LockOnMarker> Factory;
    friend class TCache<LockOnMarker>;
    void release()
    {
      Factory::insertNotUsed(this);
    }

    class Drawer : public ParticleSet<LockOnMarker>
    {
    typedef ParticleSet<LockOnMarker> Super;
    public:
      Drawer() {}
      Drawer(Deserializer& s) : Super(s) {}
      float getDrawPriority() { return 11.0f; }
      void drawSprite()
      {
        texture_ptr tex = GetTexture("lockon.png");
        glDisable(GL_LIGHTING);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        tex->assign();
        Super::drawSprite();
        tex->disassign();
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_LIGHTING);
      }
    };

  private:
    solid_ptr m_target;
    gobj_ptr m_owner;
    float m_scale;

    LockOnMarker() : m_target(0), m_owner(0), m_scale(120.0f)
    {
      Drawer::instance()->insert(this);

      setScale(vector4(m_scale));
      GetSound("lock.wav")->play(5);
    }

    LockOnMarker(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_target);
      DeserializeLinkage(s, m_owner);
      s >> m_scale;
    }

  public:
    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_target);
      ReconstructLinkage(m_owner);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_target);
      SerializeLinkage(s, m_owner);
      s << m_scale;
    }

    void setOwner(gobj_ptr p) { m_owner=p; }
    void setTarget(solid_ptr p) { m_target=p; }

    float getDrawPriority() { return -1.0f; }

    void updateSprite(Sprite& sp)
    {
      Super::updateSprite(sp);
      sp.setColor(vector4(1.0f, 1.0f, 1.0f, std::min<float>(1.0f, 1.0f-m_scale/120.0f)));
    }

    void update()
    {
      if(!m_target || m_target->isDead() || !m_owner || m_owner->isDead()) {
        kill();
      }
      else if(m_target) {
        setPosition(m_target->getCenter());
        if(m_scale > 20.0f) {
          m_scale = std::max<float>(20.0f, m_scale-8.0f);
          setScale(vector4(m_scale, m_scale, m_scale));
        }
      }
    }
  };




  class Team : public Inherit2(HavePosition, ITeam)
  {
  typedef Inherit2(HavePosition, ITeam) Super;
  public:

    class PlayerRecord : public IPlayerRecord
    {
    private:
      int m_session_id;
      string m_name;
      int m_fraction_kill;
      int m_player_kill;
      int m_death;

    public:
      PlayerRecord(Deserializer& s)
      {
        s >> m_session_id >> m_name >> m_fraction_kill >> m_player_kill >> m_death;
      }

      void serialize(Serializer& s) const
      {
        s << m_session_id << m_name << m_fraction_kill << m_player_kill << m_death;
      }

    public:
      PlayerRecord(int sid, const string& name) :
        m_session_id(sid), m_name(name), m_fraction_kill(0), m_player_kill(0), m_death(0)
      {}
      int getSessionID()      { return m_session_id; }
      const string& getName() { return m_name; }
      int getFractionKill() { return m_fraction_kill; }
      int getPlayerKill()   { return m_player_kill; }
      int getDeath()        { return m_death; }
      void incrementFractionKill() { ++m_fraction_kill; }
      void incrementPlayerKill()   { ++m_player_kill; }
      void incrementDeath()        { ++m_death; }
    };
    typedef intrusive_ptr<PlayerRecord> record_ptr;
    typedef std::vector<record_ptr> record_cont;

    struct pr_order_by_fkill
    {
      bool operator()(record_ptr l, record_ptr r)
      { return l->getFractionKill() > r->getFractionKill(); }
    };

    struct pr_order_by_pkill
    {
      bool operator()(record_ptr l, record_ptr r)
      { return l->getPlayerKill() > r->getPlayerKill(); }
    };


  private:
    record_cont m_record;

    typedef std::vector<team_ptr> team_cont;
    static team_cont& getTeamCont()
    {
      static team_cont s_team;
      return s_team;
    }

    static void insertTeam(team_ptr team)
    {
      getTeamCont().push_back(team);
    }

    static void eraseTeam(team_ptr team)
    {
      team_cont& tc = getTeamCont();
      team_cont::iterator i = std::find(tc.begin(), tc.end(), team);
      if(i!=tc.end()) {
        tc.erase(i);
      }
    }

  public:
    Team(Deserializer& s) : Super(s)
    {
      insertTeam(this);
      DeserializePtrContainer(s, m_record);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializePtrContainer(s, m_record);
    }

  public:
    static size_t getTeamCount() { return getTeamCont().size(); }
    static team_ptr getTeam(int i) { return getTeamCont()[i]; }
    static team_ptr getTeamBySID(int sid)
    {
      team_cont& tc = getTeamCont();
      for(size_t i=0; i<tc.size(); ++i) {
        if(tc[i]->isInclude(sid)) {
          return tc[i];
        }
      }
      return 0;
    }


    Team()
    {
      Register(this);
      insertTeam(this);
    }

    ~Team()
    {
      eraseTeam(this);
    }

    void join(int sid)
    {
      if(pinfo_ptr pi=GetPlayerInfoBySID(sid)) {
        m_record.push_back(new PlayerRecord(sid, pi->getName()));
      }
    }

    void leave(int sid)
    {
      for(record_cont::iterator i=m_record.begin(); i!=m_record.end(); ++i) {
        if((*i)->getSessionID()==sid) {
          m_record.erase(i);
          break;
        }
      }
    }


    void sortByPlayerKill()
    {
      std::sort(m_record.begin(), m_record.end(), pr_order_by_pkill());
    }

    void sortByFractionKill()
    {
      std::sort(m_record.begin(), m_record.end(), pr_order_by_fkill());
    }

    size_t getRecordCount()
    {
      return m_record.size();
    }

    PlayerRecord& getRecord(size_t i)
    {
      return *m_record[i];
    }

    PlayerRecord& getRecordBySID(int sid)
    {
      for(record_cont::iterator i=m_record.begin(); i!=m_record.end(); ++i) {
        if((*i)->getSessionID()==sid) {
          return *(*i);
        }
      }
      throw std::runtime_error("Team::getRecordBySID()");
    }

    bool isInclude(int sid)
    {
      for(record_cont::iterator i=m_record.begin(); i!=m_record.end(); ++i) {
        if((*i)->getSessionID()==sid) {
          return true;
        }
      }
      return false;
    }


    size_t getPlayerCount()
    {
      return m_record.size();
    }

    player_ptr getPlayer(size_t i)
    {
      return GetPlayerInfoBySID(getRecord(i).getSessionID())->getPlayer();
    }


    int getFractionKill()
    {
      int v = 0;
      for(record_cont::iterator i=m_record.begin(); i!=m_record.end(); ++i) {
        v+=(*i)->getFractionKill();
      }
      return v;
    }

    int getPlayerKill()
    {
      int v = 0;
      for(record_cont::iterator i=m_record.begin(); i!=m_record.end(); ++i) {
        v+=(*i)->getPlayerKill();
      }
      return v;
    }

    int getDeath()
    {
      int v = 0;
      for(record_cont::iterator i=m_record.begin(); i!=m_record.end(); ++i) {
        v+=(*i)->getDeath();
      }
      return v;
    }

    void incrementFractionKill(int sid)
    {
      if(!isInclude(sid)) { return; }
      getRecordBySID(sid).incrementFractionKill();
    }

    void incrementPlayerKill(int sid)
    {
      if(!isInclude(sid)) { return; }
      getRecordBySID(sid).incrementPlayerKill();
    }

    void incrementDeath(int sid)
    {
      if(!isInclude(sid)) { return; }
      getRecordBySID(sid).incrementDeath();
    }

    string p()
    {
      string r = Super::p();
      char buf[256];
      r+="  players: ";
      for(size_t i=0; i<m_record.size(); ++i) {
        sprintf(buf, "%d ", m_record[i]->getSessionID());
        r+=buf;
      }
      r+="\n";
      sprintf(buf,
        "  player_kill: %d\n"
        "  fraction_kill: %d\n"
        "  death: %d\n"
        , getPlayerKill(), getFractionKill(), getDeath());
      return r+buf;
    }
  };

  size_t ITeam::getTeamCount() { return Team::getTeamCount(); }
  team_ptr ITeam::getTeam(int i) { return Team::getTeam(i); }
  team_ptr ITeam::getTeamBySID(int sid) { return Team::getTeamBySID(sid); }


  typedef Inherit8(HaveTeam, HavePointCollision, SpinningCube, Box, HaveControler, HavePosition, Breakable, IPlayer) _Player;
  class Player : public _Player
  {
  typedef _Player Super;
  public:

    class Drawer : public BatchDrawer<Player>
    {
    typedef BatchDrawer<Player> Super;
    public:
      Drawer(Deserializer& s) : Super(s) {}
      Drawer() {}
      float getDrawPriority() { return 30.0f; }

      void draw()
      {
        for(size_t i=0; i<m_objs.size(); ++i) {
          object_ptr o = m_objs[i];
          if(!o->isDead()) { // 先にupdateされる関係上、死んでるオブジェクトを含むことがある 
            o->drawStatus();
          }
        }
      }
    };

  private:
    typedef LockOnMarker* marker_ptr;
    typedef GLaser* laser_ptr;
    typedef DamageFlash* dflash_ptr;
    typedef std::map<gid, std::pair<solid_ptr, marker_ptr> > lock_cont;

    int m_session_id;
    string m_name;
    vector4 m_color;

    lock_cont m_locked;
    dflash_ptr m_dflash;
    laser_ptr m_laser;
    player_ptr m_damage_from;
    int m_damage_time;

    int m_past;
    int m_invincible;
    int m_cooldown;
    float m_speed;
    float m_total_damage;
    float m_energy;
    int m_catapult_level;
    vector4 m_direction;
    float4 m_emission;
    float4 m_lifegage;
    float4 m_energygage;
    ist::Light m_laser_light;

  public:
    Player(Deserializer& s) : Super(s)
    {
      size_t size;
      s >> size;
      for(size_t i=0; i<size; ++i) {
        gid key;
        s >> key;
        m_locked[key] = std::pair<solid_ptr, marker_ptr>(0, 0);
        PushLinkage(s);
        PushLinkage(s);
      }
      DeserializeLinkage(s, m_dflash);
      DeserializeLinkage(s, m_laser);
      DeserializeLinkage(s, m_damage_from);
      s >> m_session_id >> m_name >> m_color >> m_damage_time >> m_past >> m_invincible
        >> m_cooldown >> m_speed >> m_total_damage >> m_energy >> m_catapult_level
        >> m_direction >> m_emission >> m_lifegage >> m_energygage >> m_laser_light;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_locked.size();
      for(lock_cont::const_iterator i=m_locked.begin(); i!=m_locked.end(); ++i) {
        s << i->first << i->second.first->getID() << i->second.second->getID();
      }
      SerializeLinkage(s, m_dflash);
      SerializeLinkage(s, m_laser);
      SerializeLinkage(s, m_damage_from);
      s << m_session_id << m_name << m_color << m_damage_time << m_past << m_invincible
        << m_cooldown << m_speed << m_total_damage << m_energy << m_catapult_level
        << m_direction << m_emission << m_lifegage << m_energygage << m_laser_light;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      for(lock_cont::iterator i=m_locked.begin(); i!=m_locked.end(); ++i) {
        solid_ptr s = CheckedPopLinkage<Solid>();
        marker_ptr m = LockOnMarker::Drawer::instance()->getObject(PopLinkageID());
        if(!m) {
          throw Error("Player::reconstructLinkage()");
        }
        i->second = std::make_pair(s, m);
      }
      ReconstructLinkage(m_dflash);
      ReconstructLinkage(m_laser);
      ReconstructLinkage(m_damage_from);
    }

  public:
    Player(ISession& s, controler_ptr cont) :
      m_session_id(s.getID()), m_name(s.getName()), m_color(s.getColor()),
      m_dflash(0), m_laser(0), m_damage_from(0), m_damage_time(0),
      m_past(0), m_invincible(300), m_cooldown(0), m_speed(3.0f),
      m_total_damage(0.0f), m_energy(0.0f), m_catapult_level(0)
    {
      Register(this);
      Drawer::instance()->insert(this);

      m_direction.x = 1.0f;
      m_laser_light.setSpecular(vector4(0.0f, 0.0f, 0.0f));
      m_laser_light.setDiffuse(vector4(0.0f, 0.0f, 0.0f));
      m_laser_light.setConstantAttenuation(1.0f);
      m_laser_light.setLinearAttenuation(0.02f);
      setBox(box(vector4(7.0f)));
      setControler(cont);
      setLife(getMaxLife());
      setRadius(5.0f);

      m_emission = float4(1.0f, 1.0f, 1.0f, 1.0f);
      m_lifegage = float4(1.0f, 1.0f, 1.0f, 0.0f);
      m_energygage = float4(1.0f, 1.0f, 1.0f, 0.0f);

      m_dflash = new DamageFlash(this);
      setTeam(Team::getTeamBySID(m_session_id));
    }

    float getDrawPriority() { return 9.0f; }

    int getSessionID() { return m_session_id; }
    const string& getName() { return m_name; }

    int getCatapultLevel() { return m_catapult_level; }
    float getSpeed() const { return m_speed; }
    float getEnergy() const { return m_energy; }
    const vector4& getDirection() { return m_direction; }
    size_t getLockCount() { return m_locked.size(); }

    size_t getMaxLockCount()
    {
      return 50;
    }

    void setInvincible(int v) { m_invincible=v; }

    float getMaxLife() { return 300.0f; }
  //  float getMaxLife() { return 3000.0f; }

    void setLife(float v)
    {
      Super::setLife(std::min<float>(getMaxLife(), v));
    }

    void setEnergy(float v)
    {
      if(m_catapult_level==3) {
        return;
      }

      int prev = m_catapult_level;
      m_energy = v;
      float max_energy = GetMaxEnergy();
      while(m_energy > max_energy) {
        if(m_catapult_level < 3) {
          m_energy-=max_energy;
          ++m_catapult_level;
          if(m_session_id==GetGame()->getSessionID()) {
            if(m_catapult_level==1) {
              GetSound("charge1.wav")->play(6);
            }
            else if(m_catapult_level==2) {
              GetSound("charge2.wav")->play(6);
            }
            else if(m_catapult_level==3) {
              GetSound("charge3.wav")->play(6);
            }
          }
        }
        else {
          m_energy = 0;
        }
      }
      if(prev!=m_catapult_level) {
         m_emission = float4(2.0f, 0.5f, 2.0f, 1.0f);
      }
    }


    float4 getCLevelColor(int level)
    {
      float4 color[4] = {
        float4(1.0f, 1.0f, 1.0f, 0.2f),
        float4(0.2f, 0.2f, 1.0f, 1.0f),
        float4(0.4f, 0.1f, 0.6f, 1.0f),
        float4(1.0f, 0.2f, 0.2f, 1.0f),
      };
      return color[level];
    }

    float4 getCLevelColor() { return getCLevelColor(m_catapult_level); }


    virtual void draw()
    {
      player_ptr lp = GetLocalPlayer();
      team_ptr lt = GetLocalTeam();

      // 本体の箱を描画 
      glMaterialfv(GL_FRONT, GL_DIFFUSE, m_color.v);
      glMaterialfv(GL_FRONT, GL_AMBIENT, m_color.v);
      if(m_emission.w > 0.02f) {
        glMaterialfv(GL_FRONT, GL_EMISSION, m_emission.v);
        Super::draw();
        glMaterialfv(GL_FRONT, GL_EMISSION, float4().v);
      }
      else {
        Super::draw();
      }
      glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.8f, 0.8f, 0.8f).v);
      glMaterialfv(GL_FRONT, GL_AMBIENT, vector4(0.2f, 0.2f, 0.2f).v);

      // リングと光を描画 
      vector4 teamcolor(1,1,1,1);
      {
        if(lt && lp!=this) {
          if(getTeam()==lt) {
            teamcolor = vector4(0.3f, 0.3f, 1.0f);
          }
          else {
            teamcolor = vector4(1.0f, 0.3f, 0.3f);
          }
        }
      }

      // リングを描画 
      {
        glDepthMask(GL_FALSE);
        ist::MatrixSaver ms(GL_MODELVIEW_MATRIX);
        matrix44 mat;
        mat.translate(getPosition().v);
        mat.rotateA(getDirection(), m_past*4.0f);
        glMultMatrixf(mat.v);
        glDisable(GL_LIGHTING);
        glBegin(GL_LINE_LOOP);
        float r = 20.0f;
        for(size_t i=0; i<getMaxLockCount(); ++i) {
          float rad = (360.0f/getMaxLockCount())*i*ist::radian;
          float s = ::sinf(rad);
          float c = ::cosf(rad);
          if(i<getLockCount()) {
            glColor4f(teamcolor.x, teamcolor.y, teamcolor.z, 0.2f);
          }
          else {
            glColor4f(teamcolor.x, teamcolor.y, teamcolor.z, 1.0f);
          }
          glVertex3f(c*r, s*r, 0.0f);
        }
        glEnd();
        glColor4f(1.0f, 1.0f, 1.0, 1.0f);
      }
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      {
        glBegin(GL_LINES);
        glColor4fv(teamcolor.v);
        glVertex3fv(getPosition().v);
        glColor4f(teamcolor.x, teamcolor.y, teamcolor.z, 0.0f);
        glVertex3fv((getPosition()+getDirection()*100.0f).v);
        glEnd();
        glColor4f(1.0f, 1.0f, 1.0, 1.0f);

        glEnable(GL_LIGHTING);
        glDepthMask(GL_TRUE);
      }
      {
        DrawSprite("flare.png",
          getPosition()+getDirection()*20.0f,
          vector4(20.0f),
          float(m_past)*0.33f);
      }

      // カタパルトの有効範囲を描画 
      if(m_catapult_level>0 && (!lt || getTeam()==lt)) {
        matrix44 mat;
        mat.translate(getPosition().v);
        mat.aimVector2(getDirection());
        box range[4] = {
          box(),
          box(vector4(200, 60, 0), vector4(0, -60, -0)),
          box(vector4(280, 84, 0), vector4(0, -84, -0)),
          box(vector4(360, 108, 0), vector4(0, -108, -0)),
        };
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        float4 col = getCLevelColor()+float4(0.2f, 0.2f, 0.2f);
        col.w = (sinf(ist::radian*GetPast()*2.5f)+1.0f)*0.3f+0.2f;
        if(this!=lp) {
          col.w*=0.25f;
        }
        glColor4fv(col.v);
        DrawBox(range[m_catapult_level], mat);

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
      }
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void drawStatus()
    {
      player_ptr lp = GetLocalPlayer();
      team_ptr lt = GetLocalTeam();

      vector2 ppos = GetProjectedPosition(getPosition());
      {
        ppos.x = std::max<float>(ppos.x,  30.0f);
        ppos.x = std::min<float>(ppos.x, 610.0f);
        ppos.y = std::max<float>(ppos.y,  -5.0f);
        ppos.y = std::min<float>(ppos.y, 440.0f);
      }
      vector2 bl = ppos-vector2(32,-12);

      vector4 teamcolor(1,1,1,1);
      if(lt && lp!=this) {
        if(getTeam()==lt) {
          teamcolor = vector4(0.4f, 0.4f, 1.0f);
        }
        else {
          teamcolor = vector4(1.0f, 0.4f, 0.4f);
        }
      }

      // リングを描画 
      {
        ist::MatrixSaver ms(GL_MODELVIEW_MATRIX);
        matrix44 mat;
        mat.translate(getPosition().v);
        mat.rotateA(getDirection(), m_past*4.0f);
        glMultMatrixf(mat.v);
        glDisable(GL_LIGHTING);
        glBegin(GL_LINE_LOOP);
        float r = 20.0f;
        for(size_t i=0; i<getMaxLockCount(); ++i) {
          float rad = (360.0f/getMaxLockCount())*i*ist::radian;
          float s = ::sinf(rad);
          float c = ::cosf(rad);
          if(i<getLockCount()) {
            glColor4f(teamcolor.x, teamcolor.y, teamcolor.z, 0.2f);
          }
          else {
            glColor4f(teamcolor.x, teamcolor.y, teamcolor.z, 1.0f);
          }
          glVertex3f(c*r, s*r, 0.0f);
        }
        glEnd();
        glColor4f(1.0f, 1.0f, 1.0, 1.0f);
      }
      {
        glBegin(GL_LINES);
        glColor4fv(teamcolor.v);
        glVertex3fv(getPosition().v);
        glColor4f(teamcolor.x, teamcolor.y, teamcolor.z, 0.0f);
        glVertex3fv((getPosition()+getDirection()*100.0f).v);
        glEnd();
        glColor4f(1.0f, 1.0f, 1.0, 1.0f);
      }

      // 名前を描画 
      {
        ScreenMatrix sm;
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glColor4fv(teamcolor.v);
        DrawText(_L(m_name), ppos-vector2(30,-10));
        glColor4f(1, 1, 1, 1);
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
      }

      bl = ppos-vector2(25,-32);
      glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
      DrawRect(bl, bl+vector2(50, 5)); // ライフゲージ 
      glColor4fv(m_lifegage.v);
      DrawRect(bl+vector2(1,1), bl+vector2(getLife()/getMaxLife()*50.0f, 5)-vector2(1,1));

      if(!lt || getTeam()==lt) {
        bl-=vector2(0.0f, 5.0f);
        glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
        DrawRect(bl, bl+vector2(50, 4)); // エナジーゲージ 
        float max_enegy = GetMaxEnergy();
        if(m_catapult_level==0) {
          glColor4fv(getCLevelColor().v);
          DrawRect(bl+vector2(1,1), bl+vector2(getEnergy()/max_enegy*50.0f, 4)-vector2(1,1));
        }
        else {
          glColor4fv(getCLevelColor().v);
          DrawRect(bl+vector2(1,1), bl+vector2(50.0f, 4)-vector2(1,1));

          if(m_catapult_level < 3) {
            glColor4fv(getCLevelColor(m_catapult_level+1).v);
            DrawRect(bl+vector2(1,1), bl+vector2(getEnergy()/max_enegy*50.0f, 4)-vector2(1,1));
          }
        }
      }

      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }


    void DrawBox(const box& box, const matrix44& mat)
    {
      ist::ModelviewMatrixSaver ms;
      glMultMatrixf(mat.v);

      const vector4& ur = box.getUpperRight();
      const vector4& bl = box.getBottomLeft();
      glBegin(GL_LINE_LOOP);
      glVertex3fv(ur.v);
      glVertex3fv(vector3(ur.x, ur.y, bl.z).v);
      glVertex3fv(vector3(bl.x, ur.y, bl.z).v);
      glVertex3fv(vector3(bl.x, ur.y, ur.z).v);
      glEnd();
      glBegin(GL_LINE_LOOP);
      glVertex3fv(bl.v);
      glVertex3fv(vector3(bl.x, bl.y, ur.z).v);
      glVertex3fv(vector3(ur.x, bl.y, ur.z).v);
      glVertex3fv(vector3(ur.x, bl.y, bl.z).v);
      glEnd();
      glBegin(GL_LINES);
      glVertex3fv(vector3(ur.x, ur.y, ur.z).v);
      glVertex3fv(vector3(ur.x, bl.y, ur.z).v);
      glVertex3fv(vector3(bl.x, ur.y, ur.z).v);
      glVertex3fv(vector3(bl.x, bl.y, ur.z).v);
      glVertex3fv(vector3(bl.x, ur.y, bl.z).v);
      glVertex3fv(vector3(bl.x, bl.y, bl.z).v);
      glVertex3fv(vector3(ur.x, ur.y, bl.z).v);
      glVertex3fv(vector3(ur.x, bl.y, bl.z).v);
      glEnd();
    }


    void update()
    {
      Super::update();
      updateCollisionMatrix();
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      SweepDeadObject(m_damage_from);
      if(--m_damage_time==0) {
        m_damage_from = 0;
      }

      { // 画面外に行ってたら押し戻す 
        vector4 pos = getPosition();
        const box& bound = GetGlobalPlayerBound();
        if(pos.x < bound.getBottomLeft().x) {
          pos.x+=m_speed;
        }
        else if(pos.x > bound.getUpperRight().x) {
          pos.x-=m_speed;
        }
        if(pos.y < bound.getBottomLeft().y) {
          pos.y+=m_speed;
        }
        else if(pos.y > bound.getUpperRight().y) {
          pos.y-=m_speed;
        }
        pos.z*=0.97f;
        setPosition(pos);
      }

      ++m_past;
      m_total_damage = std::max<float>(0.0f, m_total_damage-(5.0f/60));
      m_cooldown = std::max<int>(0, m_cooldown-1);

      // 無敵状態の時のカウンタと色処理 
      m_invincible = std::max<int>(0, m_invincible-1);
      if(m_invincible < 20) {
        m_emission*=0.95f;
      }
      else {
        m_emission = float4(1.0f, 1.0f, 1.0f, 1.0f);
      }

      setLife(getLife()+(getMaxLife()/3600)); // 自然回復 
      setEnergy(getEnergy()+(5.0f/60.0f)*GetGame()->getCatapultBoost()); // 自然チャージ 

      // レーザー撃ってる状態なら青ライト点灯 
      vector4 color = m_laser_light.getDiffuse();
      if(m_laser) {
        m_cooldown = 1;
        m_laser->setPosition(getPosition());
        m_laser->setDirection(getDirection());

        color.z = std::min<float>(color.z+0.02f, 1.0f);
        m_laser_light.setDiffuse(color);
        m_laser_light.setPosition(getPosition()+getDirection()*20);
      }
      else {
        color.z = std::max<float>(color.z-0.01f, 0.0f);
        m_laser_light.setDiffuse(color);
      }

      if(GetConfig()->exlight && color.z>0.0f && GetLocalPlayer()==this) {
        m_laser_light.enable();
      }
      else {
        m_laser_light.disable();
      }


      // 各種ゲージ色増減 
      m_lifegage+=(float4(1.0f, 1.0f, 1.0f, 0.9f)-m_lifegage)*0.05f;
      if(m_invincible>0) {
        m_lifegage = float4(0.1f,0.1f,1.0f,0.9f);
      }
      m_energygage+=(getCLevelColor()-m_energygage)*0.1f;

      // ロックオンターゲットの生死チェック 
      for(lock_cont::iterator p=m_locked.begin(); p!=m_locked.end(); /* */) {
        gobj_ptr target = p->second.first;
        marker_ptr mark = p->second.second;
        if(!target || target->isDead() || !mark || mark->isDead()) {
          m_locked.erase(p++);
        }
        else {
          ++p;
        }
      }

      BlueBlur::instance().setCenter(getPosition());
      BlueBlur::instance().append(this);
    }

    void onCollide(CollideMessage& m)
    {
      if(m_invincible<=0) {
        vector4 pos = getPosition() + (m.getNormal()*m.getDistance());
        if(GetGlobalPlayerBound().isInner(pos)) {
          setPosition(pos);
        }
      }

      float damage = 1.0f;
      if(fraction_ptr p = ToFraction(m.getFrom())) {
        damage = 10.0f;
        SendDestroyMessage(this, p);
      }
      SendDamageMessage(m.getFrom(), this, damage);
    }


    void shootLaser()
    {
      if(!m_laser) {
        m_laser = new GLaser(this);
      }
    }

    void stopLaser()
    {
      if(m_laser) {
        m_laser->stop();
        m_laser = 0;
      }
    }

    void catapult()
    {
      if(m_catapult_level==0) {
        return;
      }

      new DirectionalImpact(getPosition(), getDirection());

      box range[4] = {
        box(),
        box(vector4(200, 60, 60), vector4(0, -60, -60)),
        box(vector4(280, 84, 84), vector4(0, -84, -84)),
        box(vector4(360, 108, 108), vector4(0, -108, -108)),
      };
      cdetector cd;
      box_collision bcm;
      matrix44 mat;
      mat.translate(getPosition().v);
      mat.aimVector2(getDirection());
      bcm.setBox(range[m_catapult_level]);
      bcm.setMatrix(mat);

      vector4 pos = getPosition()-(getDirection()*250.0f);
      gobj_iter& it = GetObjects(bcm.getBoundingBox());
      while(it.has_next()) {
        solid_ptr s = ToSolid(it.iterate());
        if(s && getDirection().dot(((s->getPosition()-getPosition()).normal()))>0.0f
          && cd.detect(bcm, s->getCollision()))
        {
          vector4 aim = (s->getPosition()-pos);
          aim.z = 0.0f;
          float dist = aim.norm();
          vector4 dir = aim.normal();
          float str = 8.5f+dist/100.0f;
          SendAccelMessage(this, s, vector4(dir*str));
        }
      }

      m_catapult_level = 0;
      m_cooldown = 5;
    }

    bool readyRayShoot()
    {
      return !m_locked.empty();
    }

    struct greater_distance
    {
      vector4 m_pos;

      greater_distance(const vector4& pos) : m_pos(pos) {}
      bool operator()(gobj_ptr l, gobj_ptr r) {
        float ld = (l->getPosition()-m_pos).square();
        float rd = (r->getPosition()-m_pos).square();
        return ld < rd;
      }
    };

    void lockRay()
    {
      if(m_cooldown>0 || m_locked.size()>=getMaxLockCount()) {
        return;
      }

      float min_dist = 0.0f;
      enemy_ptr nearest = 0;
      for(int i=0; i<5; ++i) {
        gobj_iter& it = GetObjects(sphere(getPosition(), 150.0f*(i+1)));
        while(it.has_next()) {
          enemy_ptr e = ToEnemy(it.iterate());
          if(e && e->getCollision().getType()!=ist::CM_FALSE &&
             m_locked.find(e->getID())==m_locked.end()) {
            float d = (e->getPosition()-getPosition()).norm();
            if(!nearest || d < min_dist) {
              nearest = e;
              min_dist = d;
            }
          }
        }

        if(nearest) {
          break;
        }
      }

      if(nearest) {
        marker_ptr mark = LockOnMarker::Factory::create();
        mark->setOwner(this);
        mark->setTarget(nearest);
        m_locked[nearest->getID()] = std::make_pair(nearest, mark);
        m_cooldown = 1;
      }
    }

    void shootRay()
    {
      vector4 vel = getDirection()*-15.0f;
      int i = 0;
      matrix44 mat = matrix44().rotateZ(1.5f);
      matrix44 mirror = matrix44().rotateA(getDirection(), 180.0f);
      vector4 pos = getPosition()+getDirection()*20.0f;
      for(lock_cont::iterator p=m_locked.begin(); p!=m_locked.end(); ++p) {
        gobj_ptr b = new Ray(this, pos, vel, p->second.first);
        p->second.second->setOwner(b);

        ++i;
        if(i%2==0) {
          vel = mat*vel;
        }
        vel = mirror*vel;
      }
      m_cooldown = 5;
      m_locked.clear();
      m_laser_light.setDiffuse(vector4(0.0f, 0.0f, 1.0f));
    }

    void onDamage(DamageMessage& m)
    {
      if(m_invincible>0) {
        return;
      }
      if(dynamic_cast<GLaser*>(m.getSource())) {
        SendDamageMessage(m.getFrom(), this, m.getDamage()*0.25f);
        return;
      }

      Super::onDamage(m);
      m_total_damage+=m.getDamage();
      if(m_total_damage>=180.0f) {
        m_total_damage = 0.0f;
        m_invincible = 120;
      }

      m_lifegage = float4(1.0f, 0.0f, 0.0f, 1.0f);
      m_emission = float4(2.0f, 0.3f, 0.0f, 1.0f);
      if(m_session_id==GetGame()->getSessionID()) {
        float o = 1.2f;
        if(m.getDamage()<=1.0f) {
          o = 0.5f;
        }
        else if(m.getDamage()<=3.0f) {
          o = 0.75f;
        }
        m_dflash->setOpacity(std::max<float>(m_dflash->getOpacity(), o));
      }

      if(player_ptr from = ToPlayer(m.getFrom())) {
        if(getTeam()!=from->getTeam()) {
          m_damage_from = from;
          m_damage_time = 1200;
        }
      }
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);
      PutSmallExplode(getPosition(), 20);
      PutBigImpact(getPosition());

      if(team_ptr t = getTeam()) {
        t->incrementDeath(m_session_id);
        if(m_damage_from) {
          if(team_ptr team = m_damage_from->getTeam()) {
            team->incrementPlayerKill(m_damage_from->getSessionID());
          }
          PushChatText(string("# ")+m_damage_from->getName()+" killed "+getName());
        }
      }
    }

    void aim(const vector4& _v)
    {
      vector4 v = _v.normal();
      if(v.square()==0 || (v-m_direction).norm()<0.01f) {
        return;
      }

      const float rot_speed = 6.0f;
      float dot = v.dot(m_direction);
      float dot2 = (matrix44().rotateZ(90.0f)*v).dot(m_direction);
      if( (dot>=0.0f && dot2<=0.0f)
        ||(dot<=0.0f && dot2<=0.0f)) {
        m_direction = matrix44().rotateZ(rot_speed)*m_direction;
      }
      else {
        m_direction = matrix44().rotateZ(-rot_speed)*m_direction;
      }
    }

    void move(const vector4& v)
    {
      setPosition(getPosition()+v);
    }

    string p()
    {
      string r = Super::p();
      char buf[256];
      sprintf(buf,
        "  session_id: %d\n"
        "  name: %s\n"
        , m_session_id, m_name.c_str());
      return r+buf;
    }
  };



  class Player_Controler : public TControler<Player>
  {
  typedef TControler<Player> Super;
  private:
    IInput *m_input;

  public:
    Player_Controler(IInput *input) : m_input(input) {}

    Player_Controler(Deserializer& s) : Super(s)
    {}

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      int session = get()->getSessionID();
      m_input = GetGame()->getInput(session);
    }

    IInput& getInput() { return *m_input; }

    void onUpdate(UpdateMessage& m)
    {
      Player *player = get();
      vector4 move;
      float speed = player->getSpeed();
      if(getInput().right()){ move.x+=speed; }
      if(getInput().left()) { move.x-=speed; }
      if(getInput().up())   { move.y+=speed; }
      if(getInput().down()) { move.y-=speed; }
      // 移動 | ディレクションキーが押されてたら方向転換 
      if(getInput().button(2)) {
        player->aim(move);
      }
      else {
        player->move(move);
      }

      // 回転キーが押されてたら方向転換(ディレクションとの併用不可) 
      if(!getInput().button(2)) {
        const vector4& dir = player->getDirection();
        // 両回転キーが押されてたら近い軸x|yに合わせる 
        if(getInput().button(4) && getInput().button(5)) {
          vector4 a;
          if     (dir.x> 0.71f) {
            a.x = 1.0f;
          }
          else if(dir.x<-0.71f) {
            a.x =-1.0f;
          }
          else if(dir.y> 0.71f) {
            a.y = 1.0f;
          }
          else if(dir.y<-0.71f) {
            a.y =-1.0f;
          }
          player->aim(a);
        }
        else if(getInput().button(4)) {
          player->aim(matrix44().rotateZ( 90)*dir);
        }
        else if(getInput().button(5)) {
          player->aim(matrix44().rotateZ(-90)*dir);
        }
      }

      // レーザー&レイ 
      // 同時に押した場合ロックオンを優先 
      if(getInput().button(1)) {
        player->stopLaser();
        player->lockRay();
      }
      else if(getInput().button(0)) {
        player->shootLaser();
      }

      if(getInput().buttonReleased(1)) {
        player->shootRay();
      }
      if(getInput().buttonReleased(0)) {
        player->stopLaser();
      }

      if(getInput().buttonPressed(3)) {
        player->catapult();
      }
    }
  };



}
#endif
