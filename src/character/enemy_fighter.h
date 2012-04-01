#ifndef enemy_fighter_h
#define enemy_fighter_h

namespace exception {

  class Fighter : public Inherit3(HaveVelocity, HaveDirection, Enemy)
  {
  typedef Inherit3(HaveVelocity, HaveDirection, Enemy) Super;
  public:
    class Parts : public ChildEnemy
    {
    typedef ChildEnemy Super;
    public:
      Parts(Deserializer& s) : Super(s) {}
      Parts() {}
      void onCollide(CollideMessage& m)
      {
        Super::onCollide(m);
        gobj_ptr from = m.getFrom();
        if(IsSolid(from) && !IsFraction(from)) {
          SendDamageMessage(this, from, 2.0f);
          SendDestroyMessage(from, this);
        }
      }
    };

  private:
    static const int s_num_parts = 2;

    Parts *m_parts[s_num_parts];
    int m_frame;
    float m_spin;

  public:
    Fighter(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_parts);
      s >> m_frame >> m_spin;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_parts);
      s << m_frame << m_spin;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_parts);
    }

  public:
    Fighter(controler_ptr c) : m_frame(0), m_spin(0.0f)
    {
      setControler(c);
      setLife(5.0f);
      setEnergy(15.0f);
      setBox(box(vector4(20.0f, 12.5f, 12.5f), vector4(-5.0f, -12.5f, -12.5f)));

      const box pbox[s_num_parts] = {
        box(vector4(5.0f,  25.0f, 5.0f), vector4(-30.0f,  2.0f, -12.5f)),
        box(vector4(5.0f, -25.0f, 5.0f), vector4(-30.0f, -2.0f, -12.5f)),
      };
      for(int i=0; i<s_num_parts; ++i) {
        Parts *p = new Parts();
        p->setParent(this);
        p->setLife(2.0f);
        p->setBox(pbox[i]);
        m_parts[i] = p;
      }
    }

    void hyper()
    {
      Super::hyper();
      setAccelResist(0.5f);
      for(int i=0; i<s_num_parts; ++i) {
        m_parts[i]->hyper();
      }
    }

    float getDrawPriority() { return 1.1f; }

    void setGroup(gid v)
    {
      Super::setGroup(v);
      SetGroup(m_parts, v);
    }


    void draw()
    {
      Super::draw();

      const vector4 bpos[2] = {
        vector4(-35.0f,  13.5, 0),
        vector4(-35.0f, -13.5, 0)
      };
      for(int i=0; i<2; ++i) {
        if(m_parts[i]) {
          DrawSprite("burner.png",
            getMatrix()*bpos[i],
            vector4(27.0f+::sinf(53.0f*m_frame*ist::radian)*2.5f),
            1.0f);
        }
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      int f = ++m_frame;
      SweepDeadObject(m_parts);
      if(getControler() && DeadAny(m_parts)) {
        setControler(0);
      }

      if(getControler()==0) {
        setVel(getVel()+getParentIMatrix()*GetGlobalAccel());
        if(!m_parts[0] && m_parts[1]) {
          m_spin = std::min<float>(3.0f, m_spin+0.1f);
        }
        else if(!m_parts[1] && m_parts[0]) {
          m_spin = std::max<float>(-3.0f, m_spin-0.1f);
        }
        setDirection(matrix44().rotateZ(m_spin)*getDirection());
      }

      setPosition(getRelativePosition()+getVel());
    }

    void onCollide(CollideMessage& m)
    {
      Super::onCollide(m);
      gobj_ptr from = m.getFrom();
      if(IsSolid(from) && !IsFraction(from)) {
        SendDamageMessage(this, from, 2.0f);
        SendDestroyMessage(from, this);
      }
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);
      GetSound("explosion2.wav")->play(1);
    }
  };



  class Fighter_Controler : public TControler<Fighter>
  {
  typedef TControler<Fighter> Super;
  public:
    Fighter_Controler(Deserializer& s) : Super(s) {}
    Fighter_Controler() {}

    Getter(getParent, gobj_ptr);
    Getter(getMatrix, const matrix44&);
    Getter(getParentMatrix, const matrix44&);
    Getter(getParentIMatrix, const matrix44&);
    Getter(getRelativePosition, const vector4&);
    Getter(getGroup, gid);
    Getter(getPosition, const vector4&);
    Getter(getVel, const vector4&);
    Getter(getDirection, const vector4&);
    Getter(isHyper, bool);

    Setter(setGroup, gid);
    Setter(setPosition, const vector4&);
    Setter(setVel, const vector4&);
    Setter(setDirection, const vector4&);
  };


  class Fighter_Straight : public Fighter_Controler
  {
  typedef Fighter_Controler Super;
  private:
    float m_speed;
    bool m_scroll;

  public:
    Fighter_Straight(Deserializer& s) : Super(s)
    {
      s >> m_speed >> m_scroll;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_speed << m_scroll;
    }

  public:
    Fighter_Straight(float speed=2.0f, bool scroll=false) :
      m_speed(speed), m_scroll(scroll)
    {}

    void onConstruct(ConstructMessage& m)
    {
      setVel(getDirection()*m_speed);
    }

    void onUpdate(UpdateMessage& m)
    {
      if(m_scroll) {
        setPosition(getRelativePosition()+getParentIMatrix()*GetGlobalScroll());
      }
    }
  };

  class Fighter_Rush : public Fighter_Controler
  {
  typedef Fighter_Controler Super;
  private:
    int m_frame;
    float m_length;
    bool m_scroll;
    int m_action;

  public:
    Fighter_Rush(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_length >> m_scroll >> m_action;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_length << m_scroll << m_action;
    }

  public:
    Fighter_Rush(bool scroll) :
      m_frame(0), m_length(90.0f), m_scroll(scroll), m_action(0)
    {}

    void setLength(float v) { m_length=v; }

    void move()
    {
      int f = ++m_frame;

      int end = int(m_length/3.0f);
      vector4 move = getDirection()*m_length;
      float pq = Sin90(1.0f/end*(f-1));
      float q = Sin90(1.0f/end*f);
      setPosition(getRelativePosition()+move*(q-pq));

      if(f>=end) {
        ++m_action;
        m_frame = 0;
        setGroup(Solid::createGroupID());
      }
    }

    void aim()
    {
      int f = ++m_frame;
      vector4 dir = getParentIMatrix()*(GetNearestPlayerPosition(getPosition())-getPosition()).normal();
      setDirection(getDirection()+dir*0.1f);
      if(f>=40) {
        ++m_action;
        m_frame = 0;
      }
    }

    void rush()
    {
      float max_speed = isHyper() ? 8.0f : 5.0f;
      if(getVel().norm() < max_speed) {
        setVel(getVel()+getDirection()*0.1f);
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      switch(m_action) {
      case 0: move(); break;
      case 1: aim();  break;
      case 2: rush(); break;
      }

      if(m_scroll) {
        setPosition(getRelativePosition()+getParentIMatrix()*GetGlobalScroll());
      }
    }
  };


  namespace stage2 {
    class Fighter_Turn : public Fighter_Controler
    {
    typedef Fighter_Controler Super;
    private:
      float m_speed;
      bool m_invv;
      int m_state;
      int m_frame;

    public:
      Fighter_Turn(Deserializer& s) : Super(s)
      {
        s >> m_speed >> m_invv >> m_state >> m_frame;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_speed << m_invv << m_state << m_frame;
      }

    public:
      Fighter_Turn(bool invv=false) : m_invv(invv), m_state(0), m_frame(0)
      {}

      void onUpdate(UpdateMessage& m)
      {
        ++m_frame;
        if(m_state!=1) {
          vector4 prog = getDirection()*3.5f;
          setPosition(getRelativePosition()+prog);
        }
        if(m_state==0) {
          if(m_frame==180) {
            m_frame = 0;
            ++m_state;
          }
        }
        else if(m_state==1) {
          vector4 dir = getDirection();
          setDirection(matrix44().rotateZ(m_invv ? 3 : -3)*dir);
          if(m_frame==30) {
            m_frame = 0;
            ++m_state;
          }
        }
      }
    };
  } // namespace stage2 
}
#endif
