#ifndef enemy_weakiterator_h
#define enemy_weakiterator_h

namespace exception {

  class WeakIterator : public Inherit3(HaveSpinAttrib, HaveDirection, Enemy)
  {
  typedef Inherit3(HaveSpinAttrib, HaveDirection, Enemy) Super;
  public:
    class Guard : public ChildEnemy
    {
    typedef ChildEnemy Super;
    private:
      WeakIterator *m_core;

    public:
      Guard(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_core);
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_core);
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_core);
      }

    public:
      Guard(gobj_ptr parent, WeakIterator *core) : m_core(core)
      {
        setParent(parent);
        setGroup(m_core->getGroup());
        setLife(9999);
      }

      WeakIterator* getCore() { return m_core; }

      void drawLifeGauge() {}

      float getDeltaDamage()
      {
        if(m_core) {
          return m_core->getDeltaDamage();
        }
        return 0.0f;
      }

      void draw()
      {
        if(!m_core) {
          return;
        }
        if(isHyper()) {
          glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(1.0f, 0.8f, 0.8f, 0.8f).v);
          glMaterialfv(GL_FRONT, GL_EMISSION, vector4(1.0f, 0.2f, 0.2f).v);
        }
        else {
          glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.8f, 0.8f, 1.0f, 0.8f).v);
          glMaterialfv(GL_FRONT, GL_EMISSION, vector4(0.4f, 0.4f, 1.0f).v);
        }
        Super::drawNoHyper();
        glMaterialfv(GL_FRONT, GL_EMISSION, vector4(0.0f).v);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.8f, 0.8f, 0.8f, 1.0f).v);
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        SweepDeadObject(m_core);
      }

      void onDamage(DamageMessage& m)
      {
        if(!dynamic_cast<GLaser*>(m.getSource()) &&
           !dynamic_cast<Ray*>(m.getSource()) &&
           !dynamic_cast<Laser*>(m.getSource())) {
          SendDamageMessage(m.getFrom(), m_core, m.getDamage()*0.9f, this);
        }
      }

      void onCollide(CollideMessage& m)
      {
        Super::onCollide(m);

        gobj_ptr from = m.getFrom();
        if(dynamic_cast<MediumBlock*>(from) || dynamic_cast<LargeBlock*>(from)) {
          SendDamageMessage(this, m.getFrom(), 0.4f);
        }
      }
    };

  private:
    RotLayer *m_gg1; // コアを親にしたら余計な回転してしまうので分離 
    ChildRotLayer *m_gg2;
    Guard *m_guard[4];

  public:
    WeakIterator(Deserializer& s) : Super(s), m_gg1(0)
    {
      DeserializeLinkage(s, m_gg1);
      DeserializeLinkage(s, m_gg2);
      DeserializeLinkage(s, m_guard);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_gg1);
      SerializeLinkage(s, m_gg2);
      SerializeLinkage(s, m_guard);
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_gg1);
      ReconstructLinkage(m_gg2);
      ReconstructLinkage(m_guard);
    }

  public:
    WeakIterator(controler_ptr c)
    {
      setControler(c);
      setBox(box(vector4(25)));
      setLife(50);
      setEnergy(200.0f);
      setAxis(vector4(1,1,1).normal());
      setRotateSpeed(0.3f);

      {
        m_gg1 = new RotLayer();
        m_gg1->chain();
        m_gg1->setAxis(vector4(0,0,1));
        m_gg1->setRotateSpeed(0.5f);
      }
      {
        m_gg2 = new ChildRotLayer();
        m_gg2->setParent(m_gg1);
        m_gg2->setAxis(vector4(0,1,1));
        m_gg2->setRotateSpeed(0.7f);
      }

      const box b[4] = {
        box(vector4(25,170,10), vector4(-25,40,-10)),
        box(vector4(25,-170,10), vector4(-25,-40,-10)),
        box(vector4(130,25,10), vector4(55,-25,-10)),
        box(vector4(-130,25,10), vector4(-55,-25,-10)),
      };
      for(int i=0; i<2; ++i) {
        m_guard[i] = new Guard(m_gg1, this);
        m_guard[i]->setBox(b[i]);
      }
      for(int i=2; i<4; ++i) {
        m_guard[i] = new Guard(m_gg2, this);
        m_guard[i]->setBox(b[i]);
      }
    }

    void hyper()
    {
      Super::hyper();
      setLife(100);
      for(int i=0; i<4; ++i) {
        m_guard[i]->hyper();
      }
    }

    void setGroup(gid v)
    {
      Super::setGroup(v);
      SetGroup(m_guard, v);
    }

    void invertRotation()
    {
      m_gg1->setRotateSpeed(-m_gg1->getRotateSpeed());
      m_gg2->setRotateSpeed(-m_gg2->getRotateSpeed());
    }

    void draw()
    {
      if(!isDamaged()) {
        if(isHyper()) {
          glMaterialfv(GL_FRONT, GL_DIFFUSE,  vector4(1.0f,0.3f,0.3f).v);
          glMaterialfv(GL_FRONT, GL_EMISSION, vector4(1.0f, 0.2f, 0.2f).v);
        }
        else {
          glMaterialfv(GL_FRONT, GL_DIFFUSE,  vector4(0.3f,0.3f,1.0f).v);
          glMaterialfv(GL_FRONT, GL_EMISSION, vector4(0.4f, 0.4f, 1.0f).v);
        }
        Super::drawNoHyper();
        glMaterialfv(GL_FRONT, GL_EMISSION, vector4(0.0f).v);
        glMaterialfv(GL_FRONT, GL_DIFFUSE,  vector4(0.8f,0.8f,0.8f).v);
      }
      else {
        Super::draw();
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      m_gg1->setPosition(getRelativePosition());
    }

    void onDamage(DamageMessage& m)
    {
      if(!dynamic_cast<GLaser*>(m.getSource()) && // 自機の武装無効化 
         !dynamic_cast<Ray*>(m.getSource())) {
        Super::onDamage(m);
      }
    }

    void onCollide(CollideMessage& m)
    {
      Super::onCollide(m);
      if(IsFraction(m.getFrom())) {
        SendDestroyMessage(0, m.getFrom());
      }
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);
      SendDestroyMessage(this, m_gg1);
      if(m.getStat()==0) {
        PutMediumImpact(getCenter());
      }
    }

    void onKill(KillMessage& m)
    {
      SendKillMessage(this, m_gg1);
      Super::onKill(m);
    }

    Guard* getGuard(int i) { return i<4 ? m_guard[i] : 0; }
  };


  class WeakIterator_Controler : public TControler<WeakIterator>
  {
  typedef TControler<WeakIterator> Super;
  public:
    typedef WeakIterator::Guard Guard;

    WeakIterator_Controler(Deserializer& s) : Super(s) {}
    WeakIterator_Controler() {}

    Getter(getParent, gobj_ptr);
    Getter(getMatrix, const matrix44&);
    Getter(getParentMatrix, const matrix44&);
    Getter(getParentIMatrix, const matrix44&);
    Getter(getRelativePosition, const vector4&);
    Getter(getGroup, gid);
    Getter(getPosition, const vector4&);
    Getter(getDirection, const vector4&);
    Getter(isHyper, bool);

    Setter(setGroup, gid);
    Setter(setPosition, const vector4&);
    Setter(setDirection, const vector4&);

    Getter2(getGuard, Guard*, int);

  };

  class WeakIterator_Defense : public WeakIterator_Controler
  {
  typedef WeakIterator_Controler Super;
  private:
    int m_frame;
    vector4 m_move;
    int m_action;

  public:
    WeakIterator_Defense(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_move >> m_action;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_move << m_action;
    }

  public:
    WeakIterator_Defense() : m_frame(0), m_action(0)
    {}

    void onConstruct(ConstructMessage& m)
    {
      m_move = getDirection()*1000.0f;
    }

    void appear()
    {
      int f = ++m_frame;
      float pq = Sin90((1.0f/360)*(f-1));
      float q = Sin90((1.0f/360)*f);
      setPosition(getRelativePosition()+m_move*(q-pq));

      if(f==360) {
        ++m_action;
        m_frame = 0;
      }
    }

    void wait()
    {
      int f = ++m_frame;
      if(f==1) {
        new PararelLaser(get(), GetNearestPlayerPosition(getPosition()), 65, isHyper() ? 8 : 5);
      }
      if(f==90) {
        ++m_action;
        m_frame = 0;
        m_move = getDirection()*1000.0f;
      }
    }

    void away()
    {
      int f = ++m_frame;
      float pq = Cos90I((1.0f/360)*(f-1));
      float q = Cos90I((1.0f/360)*f);
      setPosition(getRelativePosition()+m_move*(q-pq));

      if(f==360) {
        SendKillMessage(0, get());
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      switch(m_action) {
      case 0: appear(); break;
      case 1: wait();   break;
      case 2: away();   break;
      }
    }
  };


} //  exception
#endif
