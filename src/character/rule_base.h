#ifndef rule_h
#define rule_h

namespace exception {
namespace rule {

  bool g_round2 = false;
  random_ptr g_rand;

  float brand() { return g_rand->genReal()*2.0f-1.0f; }
  bool round2() { return g_round2; }



  struct order_by_pkill {
    bool operator()(team_ptr l, team_ptr r) {
      if(l->getPlayerKill()==r->getPlayerKill()) {
        return l->getDeath() < r->getDeath();
      }
      return l->getPlayerKill() > r->getPlayerKill();
    }
  };

  struct order_by_fkill {
    bool operator()(team_ptr l, team_ptr r) {
      return l->getFractionKill() > r->getFractionKill();
    }
  };

  struct order_by_respawn_time
  {
    bool operator()(pinfo_ptr l, pinfo_ptr r) {
      return l->getRespawnTime() < r->getRespawnTime();
    }
  };



  class RuleBase : public Inherit2(HavePosition, IRule)
  {
  typedef Inherit2(HavePosition, IRule) Super;
  private:
    int m_frame;

  public:
    RuleBase(Deserializer& s) : Super(s)
    {
      s >> m_frame;
      g_rand.reset(new ist::Random());
      s >> g_round2 >> (*g_rand);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame;
      s << g_round2 << (*g_rand);
    }

  public:
    RuleBase() : m_frame(0)
    {
      Register(this);
      g_round2 = false;
      g_rand.reset(new ist::Random());
    }

    ~RuleBase()
    {
      g_round2 = false;
      g_rand.reset();
    }

    float getDrawPriority() { return 10.0f; }
    int getFrame() { return m_frame; }
    void setFrame(int v) { m_frame=v; }

    virtual void progress(int f)=0;

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      progress(m_frame);
      ++m_frame;
    }

    void drawRespawnTime()
    {
      std::vector<pinfo_ptr> pinfo;
      for(size_t i=0; i<GetPlayerCount(); ++i) {
        pinfo_ptr pi = GetPlayerInfo(i);
        if(!pi->getPlayer()) {
          pinfo.push_back(pi);
        }
      }
      std::sort(pinfo.begin(), pinfo.end(), order_by_respawn_time());

      ScreenMatrix sm;
      glDisable(GL_LIGHTING);
      glDisable(GL_DEPTH_TEST);

      char buf[128];
      for(size_t i=0; i<pinfo.size(); ++i) {
        pinfo_ptr pi = pinfo[i];
        sprintf(buf, "%s: respawn %ds", pi->getName().c_str(), pi->getRespawnTime()/60);

        vector4 color(1,1,1,1);
        team_ptr lt = GetLocalTeam();
        if(pi->getSessionID()==GetGame()->getSessionID()) {
        }
        else if(lt && lt->isInclude(pi->getSessionID())) {
          color = vector4(0.4f, 0.4f, 1.0f);
        }
        else {
          color = vector4(1.0f, 0.4f, 0.4f);
        }
        glColor4fv(color.v);
        DrawText(_L(buf), sgui::Point(5, 25+15*i));
        glColor4fv(vector4(1,1,1,1).v);
      }

      glEnable(GL_LIGHTING);
      glEnable(GL_DEPTH_TEST);
    }

    virtual string p()
    {
      string r = Super::p();
      char buf[32];
      sprintf(buf, "  frame: %d\n", m_frame);
      r+=buf;
      return r;
    }
  };

  class IWave : public Optional
  {
  typedef Optional Super;
  private:
    int m_frame;

  public:
    IWave(Deserializer& s) : Super(s)
    {
      s >> m_frame;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame;
    }

    int getFrame() { return m_frame; }
    void setFrame(int v) { m_frame=v; }

  public:
    IWave() : m_frame(0) {}
    virtual void progress(int frame)=0;
    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      progress(m_frame);
      ++m_frame;
    }
  };






  class MediumBlockGround : public Inherit2(SpinningBlockAttrib, Ground)
  {
  typedef Inherit2(SpinningBlockAttrib, Ground) Super;
  public:
    MediumBlockGround(Deserializer& s) : Super(s) {}
    MediumBlockGround()
    {
      setBox(box(vector4(25)));
      setAxis(vector4(brand()-0.5f, brand()-0.5f, brand()-0.5f).normalize());
      setRotateSpeed(1.0f);
      setMaxSpeed(1.4f);
      setVelAttenuation(0.98f);
    }

    void onCollide(CollideMessage& m)
    {
      solid_ptr s = ToSolid(m.getFrom());
      if(!s) {
        return;
      }

      vector4 vel = getVel();
      if(!IsFraction(s) && vel.norm()>getMaxSpeed()+2.0f) { // 高速で衝突したらダメージ食らわせて自壊 
        SendDamageMessage(m.getFrom(), s, 50.0f, this);
        SendDestroyMessage(m.getFrom(), this);
      }
      else {
        float max_speed = std::max<float>(getMaxSpeed(), vel.norm());
        vector4 n = getParentIMatrix()*m.getNormal();
        vector4 pos = getRelativePosition();
        if(IsGround(s)) {
          vel = matrix44().rotateA(n, 180)*-vel;
          vel+=n*0.2f;
          vel.z = 0;
          pos+=n*1.0f;
        }
        else if(IsFraction(s)) {
          vel+=n*0.003f;
          vel.z = 0;
        }
        else if(IsEnemy(s)) {
          pos+=n*(vel.norm()+0.5f);
          vel+=n*0.8f;
          vel.z = 0;
        }
        if(vel.norm()>max_speed) {
          vel = vel.normalize()*max_speed;
        }
        setVel(vel);
        setPosition(pos);
      }
    }

    virtual void explode()
    {
      PutSmallImpact(getPosition());
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);

      if(m.getStat()==0) {
        explode();
      }
    }
  };

  class LargeBlockGround : public MediumBlockGround
  {
  typedef MediumBlockGround Super;
  public:
    LargeBlockGround(Deserializer& s) : Super(s) {}

  public:
    LargeBlockGround()
    {
      setBox(box(vector4(40.0f)));
      setAxis(vector4(brand()-0.5f, brand()-0.5f, brand()-0.5f).normalize());
      setRotateSpeed(0.3f);
      setMaxSpeed(1.0f);
      setVelAttenuation(0.96f);
      setAccelResist(0.5f);
    }

    void onCollide(CollideMessage& m)
    {
      solid_ptr s = ToSolid(m.getFrom());
      enemy_ptr e = ToEnemy(m.getFrom());
      if(!s) {
        return;
      }

      vector4 n = getParentIMatrix()*m.getNormal();
      vector4 vel = getVel();
      vector4 pos = getRelativePosition();
      if(IsGround(m.getFrom()) || s->getVolume()>=getVolume() || (e && e->getLife()>100.0f)) {
        vel = matrix44().rotateA(n, 180.0f)*-vel;
        vel.z = 0;
        pos+=n*2.0f;
      }
      else if(IsFraction(s)) {
        vel+=n*0.001f;
        vel.z = 0;
      }
      setVel(vel);
      setPosition(pos);
    }

    void explode()
    {
      PutMediumImpact(getPosition());
    }
  };


  class MediumSpinningGround : public Inherit2(SpinningBlockAttrib, Ground)
  {
  typedef Inherit2(SpinningBlockAttrib, Ground) Super;
  public:
    MediumSpinningGround(Deserializer& s) : Super(s) {}

  public:
    MediumSpinningGround()
    {
      setBox(box(vector4(50, 50, 30)));
      setAxis(vector4(0, 0, 1));
      setRotateSpeed(0.5f);
      setMaxSpeed(2.5f);
      setVelAttenuation(0.98f);
      setAccelResist(0.025f);
    }

    virtual int getFractionCount()
    {
      return Super::getFractionCount()/4;
    }

    void onCollide(CollideMessage& m)
    {
      solid_ptr s = ToSolid(m.getFrom());
      if(!s) {
        return;
      }

      vector4 vel = getVel();
      float max_speed = std::max<float>(getMaxSpeed(), vel.norm());
      vector4 n = getParentIMatrix()*m.getNormal();
      vector4 pos = getRelativePosition();
      if(IsGround(s)) {
        vel = matrix44().rotateA(n, 180)*-vel;
        vel.z = 0;
        vel*=0.8f;
        pos+=n*1.0f;
      }
      if(vel.norm()>max_speed) {
        vel = vel.normalize()*max_speed;
      }
      setVel(vel);
      setPosition(pos);
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);

      if(m.getStat()==0) {
        PutMediumImpact(getPosition());
      }
    }
  };

  class LargeSpinningGround : public Inherit2(SpinningBlockAttrib, Ground)
  {
  typedef Inherit2(SpinningBlockAttrib, Ground) Super;
  public:
    LargeSpinningGround(Deserializer& s) : Super(s) {}

  public:
    LargeSpinningGround()
    {
      setBox(box(vector4(80, 80, 45)));
      setAxis(vector4(0, 0, 1));
      setRotateSpeed(0.5f);
      setMaxSpeed(2.0f);
      setVelAttenuation(0.98f);
      setAccelResist(0.01f);
    }

    virtual int getFractionCount()
    {
      return Super::getFractionCount()/5;
    }

    void onCollide(CollideMessage& m)
    {
      solid_ptr s = ToSolid(m.getFrom());
      if(!s) {
        return;
      }

      vector4 vel = getVel();
      float max_speed = std::max<float>(getMaxSpeed(), vel.norm());
      vector4 n = getParentIMatrix()*m.getNormal();
      vector4 pos = getRelativePosition();
      if(IsGround(s) && s->getVolume()>getVolume()*0.5f) {
        vel = matrix44().rotateA(n, 180)*-vel;
        vel.z = 0;
        vel*=0.8f;
        pos+=n*1.0f;
      }
      if(vel.norm()>max_speed) {
        vel = vel.normalize()*max_speed;
      }
      setVel(vel);
      setPosition(pos);
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);

      if(m.getStat()==0) {
        PutMediumImpact(getPosition());
      }
    }
  };






  MediumBlock* putMediumBlock(const vector4& pos, const vector4& vel=GetGlobalAccel()*2.5f)
  {
    MediumBlock *e = new MediumBlock();
    e->setBound(box(vector4(700)));
    e->setPosition(pos);
    e->setVel(vel);
    e->setAccel(vel);
    e->setLife(40);
    e->setMaxSpeed(2.0f);
    if(round2()) {
      e->setEnergy(e->getEnergy()*2.0f);
    }
    return e;
  }

  LargeBlock* putLargeBlock(const vector4& pos, const vector4& vel=GetGlobalAccel()*2.5f)
  {
    LargeBlock *e = new LargeBlock();
    e->setBound(box(vector4(700)));
    e->setPosition(pos);
    e->setVel(vel);
    e->setAccel(vel);
    e->setLife(80);
    e->setMaxSpeed(1.5f);
    if(round2()) {
      e->setEnergy(e->getEnergy()*2.0f);
    }
    return e;
  }

  MediumBlockGround* putMediumBlockGround(const vector4& pos, const vector4& vel=GetGlobalAccel()*2.5f)
  {
    MediumBlockGround *e = new MediumBlockGround();
    e->setBound(box(vector4(1000,1000,1000), vector4(-1000,-1000,-1000)));
    e->setPosition(pos);
    e->setVel(vel);
    e->setAccel(vel*0.4f);
    e->setMaxSpeed(2.0f);
    return e;
  }

  LargeBlockGround* putLargeBlockGround(const vector4& pos, const vector4& vel=GetGlobalAccel()*2.5f)
  {
    LargeBlockGround *e = new LargeBlockGround();
    e->setBound(box(vector4(1000,1000,1000), vector4(-1000,-1000,-1000)));
    e->setPosition(pos);
    e->setVel(vel);
    e->setAccel(vel*0.4f);
    e->setMaxSpeed(1.5f);
    return e;
  }

  Fighter* putFighter(const vector4& pos, const vector4& dir, float speed)
  {
    Fighter *e = new Fighter(new Fighter_Straight(speed));
    e->setPosition(pos);
    e->setDirection(dir);
    if(round2()) {
      e->hyper();
    }
    return e;
  }

  Shell* putBurstShell(const vector4& pos, const vector4& dir)
  {
    Shell_BurstMissile *c = new Shell_BurstMissile(true);
    c->setLength(300.0f);

    Shell *e = new Shell(c);
    e->setPosition(pos);
    e->setDirection(dir);
    if(round2()) {
      e->hyper();
    }
    return e;
  }

  Shell* putGravityShell(const vector4& pos, const vector4& dir)
  {
    Shell_GravityMissile *c = new Shell_GravityMissile(true);
    c->setLength(300.0f);

    Shell *e = new Shell(c);
    e->setPosition(pos);
    e->setDirection(dir);
    if(round2()) {
      e->hyper();
    }
    return e;
  }

  Egg* putLaserEgg(const vector4& pos, const vector4& dir)
  {
    Egg_Laser *c = new Egg_Laser(true);
    c->setLength(300.0f);

    Egg *e = new Egg(c);
    e->setPosition(pos);
    e->setDirection(dir);
    if(round2()) {
      e->hyper();
    }
    return e;
  }

  Zab* putZab(const vector4& pos)
  {
    Zab *e = new Zab(new Zab_Rush(true));
    e->setPosition(pos);
    if(round2()) {
      e->hyper();
    }
    return e;
  }

  Zab* putStraightZab(const vector4& pos, const vector4& dir)
  {
    Zab *e = new Zab(new Zab_Straight(dir, true));
    e->setPosition(pos);
    if(round2()) {
      e->hyper();
    }
    return e;
  }



  class Ground_Enemy4_Linkage : public Actor
  {
  typedef Actor Super;
  private:
    ground_ptr m_ground;
    enemy_ptr m_hatch[4];

  public:
    Ground_Enemy4_Linkage(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_ground);
      DeserializeLinkage(s, m_hatch);
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_ground);
      ReconstructLinkage(m_hatch);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_ground);
      SerializeLinkage(s, m_hatch);
    }

  public:
    Ground_Enemy4_Linkage(ground_ptr g, enemy_ptr h[4])
    {
      m_ground = g;
      std::copy(h, h+4, m_hatch);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      SweepDeadObject(m_ground);
      SweepDeadObject(m_hatch);
      if(!m_ground) {
        SendKillMessage(0, this);
      }
      else if(!AliveAny(m_hatch)) {
        SendDestroyMessage(this, m_ground);
        SendKillMessage(0, this);
      }
    }
  };

  MediumSpinningGround* putSpinningGroundWithMineHatch(const vector4& pos, const vector4& vel, float rotate=0.5f)
  {
    MediumSpinningGround *g = new MediumSpinningGround();
    g->setPosition(pos);
    g->setVel(vel);
    g->setRotateSpeed(rotate);
    g->setBound(box(vector4(700)));

    enemy_ptr hatch[4];
    vector4 hpos[4] = { vector4(50, 0, 0), vector4(0, 50, 0), vector4(-50, 0, 0), vector4(0,-50, 0), };
    vector4 hdir[4] = { vector4( 1, 0, 0), vector4(0,  1, 0), vector4( -1, 0, 0), vector4(0, -1, 0), };
    for(int i=0; i<4; ++i) {
      Hatch_MiniMine *c = new Hatch_MiniMine();
      c->setWait(60+40*i);
      c->setInterval(160);

      SmallHatch *h = new SmallHatch(c);
      h->setParent(g);
      h->setPosition(hpos[i]);
      h->setDirection(hdir[i]);
      if(round2()) {
        h->hyper();
      }
      hatch[i] = h;
    }
    new Ground_Enemy4_Linkage(g, hatch);
    return g;
  }

  MediumSpinningGround* putSpinningGroundWithLaserHatch(const vector4& pos, const vector4& vel, float rotate=0.5f)
  {
    MediumSpinningGround *g = new MediumSpinningGround();
    g->setPosition(pos);
    g->setVel(vel);
    g->setRotateSpeed(rotate);
    g->setBound(box(vector4(700)));

    enemy_ptr hatch[4];
    vector4 hpos[4] = { vector4(50, 0, 0), vector4(0, 50, 0), vector4(-50, 0, 0), vector4(0,-50, 0), };
    vector4 hdir[4] = { vector4( 1, 0, 0), vector4(0,  1, 0), vector4( -1, 0, 0), vector4(0, -1, 0), };
    for(int i=0; i<4; ++i) {
      SmallHatch *h = new SmallHatch(new Hatch_Laser(260+10*i));
      h->setParent(g);
      h->setPosition(hpos[i]);
      h->setDirection(hdir[i]);
      if(round2()) {
        h->hyper();
      }
      hatch[i] = h;
    }
    new Ground_Enemy4_Linkage(g, hatch);
    return g;
  }

  LargeSpinningGround* putSpinningGroundWithFighterHatch(const vector4& pos, const vector4& vel, float rotate=0.5f)
  {
    LargeSpinningGround *g = new LargeSpinningGround();
    g->setPosition(pos);
    g->setVel(vel);
    g->setRotateSpeed(rotate);
    g->setBound(box(vector4(750)));

    enemy_ptr hatch[4];
    vector4 hpos[4] = { vector4(70, 0, 0), vector4(0, 70, 0), vector4(-70, 0, 0), vector4(0,-70, 0), };
    vector4 hdir[4] = { vector4( 1, 0, 0), vector4(0,  1, 0), vector4( -1, 0, 0), vector4(0, -1, 0), };
    for(int i=0; i<4; ++i) {
      Hatch_GenRushFighter *c = new Hatch_GenRushFighter();
      c->setWait(40+40*i);
      c->setInterval(160);

      LargeHatch *h = new LargeHatch(c);
      h->setParent(g);
      h->setPosition(hpos[i]);
      h->setDirection(hdir[i]);
      if(round2()) {
        h->hyper();
      }
      hatch[i] = h;
    }
    new Ground_Enemy4_Linkage(g, hatch);
    return g;
  }

  LargeCarrier* putLargeCarrier(const vector4& pos, const vector4& dir)
  {
    LargeCarrier *e = new LargeCarrier(new LargeCarrier_GenFighter());
    e->setBound(box(vector4(1200)));
    e->setDirection(dir);
    e->setPosition(pos);
    if(round2()) {
      e->hyper();
    }
    return e;
  }

  HeavyFighter* putHeavyFighter(const vector4& pos, const vector4& dir)
  {
    HeavyFighter *e = new HeavyFighter(new HeavyFighter_Straight());
    e->setBound(box(vector4(1200)));
    e->setPosition(pos);
    e->setDirection(dir);
    if(round2()) {
      e->hyper();
    }
    return e;
  }

  WeakIterator* putWeakIterator(const vector4& pos, const vector4& dir)
  {
    WeakIterator *e = new WeakIterator(new WeakIterator_Defense());
    e->setBound(box(vector4(1500)));
    e->setPosition(pos);
    e->setDirection(dir);
    if(round2()) {
      e->hyper();
    }
    return e;
  }






} // namespace rule 
} // namespace exception 


#endif
