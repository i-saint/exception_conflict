#ifndef rule_team_fortress_h
#define rule_team_fortress_h

namespace exception {
namespace rule {
namespace team_fortress {


  class Background : public BackgroundBase
  {
  typedef BackgroundBase Super;
  private:

    class LineTexture : public RefCounter
    {
    private:
      fbo_ptr m_fbo;

    public:
      LineTexture()
      {
        m_fbo = new ist::FrameBufferObject(128, 64);
        m_fbo->enable();
        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT);
        draw();
        m_fbo->disable();
      }

      void draw()
      {
        ScreenMatrix sm(m_fbo->getWidth(), m_fbo->getHeight());

        glDisable(GL_LIGHTING);
        glColor4f(0,0,0,1);
        glLineWidth(3.0f);

        for(int j=0; j<5; ++j) {
          const float speed = 1.0f;
          vector2 pos;
          vector2 dir(speed, 0);

          glBegin(GL_LINE_STRIP);
          glVertex2fv(pos.v);
          for(int i=0; i<250; ++i) {
            pos+=dir;
            glVertex2fv(pos.v);
            if(GenRand() < 0.05f) {
              float f = float(GenRand());
              if(f<0.35f) {
                dir = vector2(0.0f, speed);
              }
              else {
                dir = vector2(speed, 0.0f);
              }
            }
          }
          glEnd();
        }

        glLineWidth(1.0f);
        glColor4f(1,1,1,1);
        glEnable(GL_LIGHTING);
      }

      void assign() { m_fbo->assign(); }
      void disassign() { m_fbo->disassign(); }
    };
    typedef intrusive_ptr<LineTexture> ltex_ptr;

    class Block : public Inherit5(HaveDirection, Box, HavePosition, Dummy, RefCounter)
    {
    typedef Inherit5(HaveDirection, Box, HavePosition, Dummy, RefCounter) Super;
    private:
      ltex_ptr m_lt;
      vector4 m_vel;

    public:
      Block(Deserializer& s) : Super(s)
      {
        s >> m_vel;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_vel;
      }

    public:
      Block()
      {
        setBox(box(vector4(30)));
      }

      void setLineTexture(ltex_ptr v) { m_lt=v; }
      void assign() { m_lt->assign(); }
      void disassign() { m_lt->disassign(); }

      struct invert_x_coord
      { vector2 operator()(const vector2& v) { return vector2(1.0f-v.x, v.y); } };

      struct invert_y_coord
      { vector2 operator()(const vector2& v) { return vector2(v.x, 1.0f-v.y); } };

      void initTexcoord(vector2 *tex, const vector4& ur, const vector4& bl)
      {
        tex[0] = vector2(1.0f, 1.0f);
        tex[1] = vector2(0.0f, 1.0f);
        tex[2] = vector2(0.0f, 0.0f);
        tex[3] = vector2(1.0f, 0.0f);
        if(GenRand()<0.5f) { std::transform(tex+0, tex+4, tex+0, invert_x_coord()); }
        if(GenRand()<0.5f) { std::transform(tex+0, tex+4, tex+0, invert_y_coord()); }

        tex[4] = vector2(1.0f, 1.0f);
        tex[5] = vector2(0.0f, 1.0f);
        tex[6] = vector2(0.0f, 0.0f);
        tex[7] = vector2(1.0f, 0.0f);
        if(GenRand()<0.5f) { std::transform(tex+4, tex+8, tex+4, invert_x_coord()); }
        if(GenRand()<0.5f) { std::transform(tex+4, tex+8, tex+4, invert_y_coord()); }

        tex[8] = vector2(1.0f, 1.0f);
        tex[9] = vector2(0.0f, 1.0f);
        tex[10] = vector2(0.0f, 0.0f);
        tex[11] = vector2(1.0f, 0.0f);
        if(GenRand()<0.5f) { std::transform(tex+8, tex+12, tex+8, invert_x_coord()); }
        if(GenRand()<0.5f) { std::transform(tex+8, tex+12, tex+8, invert_y_coord()); }

        tex[12] = vector2(1.0f, 1.0f);
        tex[13] = vector2(1.0f, 0.0f);
        tex[14] = vector2(0.0f, 0.0f);
        tex[15] = vector2(0.0f, 1.0f);
        if(GenRand()<0.5f) { std::transform(tex+12, tex+16, tex+12, invert_x_coord()); }
        if(GenRand()<0.5f) { std::transform(tex+12, tex+16, tex+12, invert_y_coord()); }

        tex[16] = vector2(1.0f, 1.0f);
        tex[17] = vector2(0.0f, 1.0f);
        tex[18] = vector2(0.0f, 0.0f);
        tex[19] = vector2(1.0f, 0.0f);
        if(GenRand()<0.5f) { std::transform(tex+16, tex+20, tex+16, invert_x_coord()); }
        if(GenRand()<0.5f) { std::transform(tex+16, tex+20, tex+16, invert_y_coord()); }

        tex[20] = vector2(1.0f, 1.0f);
        tex[21] = vector2(0.0f, 1.0f);
        tex[22] = vector2(0.0f, 0.0f);
        tex[23] = vector2(1.0f, 0.0f);
        if(GenRand()<0.5f) { std::transform(tex+20, tex+24, tex+20, invert_x_coord()); }
        if(GenRand()<0.5f) { std::transform(tex+20, tex+24, tex+20, invert_y_coord()); }
      }

      void setVel(const vector4& v) { m_vel=v; }
      const vector4& getVel() const { return m_vel; }

      void update()
      {
        vector4 pos = getPosition();
        pos+=m_vel;
        if(pos.z > 400.0f) {
          pos.z-=1800.0f;
        }
        if(pos.z < -1400.0f) {
          pos.z+=1800.0f;
        }
        setPosition(pos);
      }
    };

    typedef intrusive_ptr<Block> block_ptr;
    typedef std::vector<block_ptr> block_cont;

    ltex_ptr m_ltex[4];
    po_ptr m_hline;
    po_ptr m_glow;
    fbo_ptr m_fbo_tmp;
    fbo_ptr m_fbo_lines;
    fbo_ptr m_fbo_glow;
    fbo_ptr m_fbo_glow_b;

    block_cont m_rblocks;
    block_cont m_lblocks;
    ist::PerspectiveCamera m_cam;
    ist::Fog m_fog;
    ist::Material m_bgmat;
    int m_frame;
    gid m_local_team;
    vector4 m_basecolor[2];
    int m_updown[2];

  public:
    Background(Deserializer& s) : Super(s)
    {
      size_t size;
      s >> size;
      for(size_t i=0; i<size; ++i) {
        m_rblocks.push_back(new Block(s));
      }
      s >> size;
      for(size_t i=0; i<size; ++i) {
        m_lblocks.push_back(new Block(s));
      }
      s >> m_cam >> m_fog >> m_bgmat >> m_frame >> m_local_team >> m_basecolor >> m_updown;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_rblocks.size();
      for(size_t i=0; i<m_rblocks.size(); ++i) {
        m_rblocks[i]->serialize(s);
      }
      s << m_lblocks.size();
      for(size_t i=0; i<m_lblocks.size(); ++i) {
        m_lblocks[i]->serialize(s);
      }
      s << m_cam << m_fog << m_bgmat<< m_frame << m_local_team << m_basecolor << m_updown;
    }

  public:
    Background() : m_frame(0), m_local_team(0)
    {
      m_cam.setPosition(vector4(0, 50, 400));
      m_cam.setTarget(vector4(0, 0, 0));
      m_cam.setFovy(60.0f);
      m_cam.setZFar(10000.0f);

      m_fog.setColor(vector4(0.0f, 0.0f, 0.0f));
      m_fog.setNear(0.0f);
      m_fog.setFar(1500.0f);

      m_bgmat.setDiffuse(vector4(0.2f, 0.2f, 0.35f));
      m_bgmat.setSpecular(vector4(0.9f, 0.9f, 1.0f));
      m_bgmat.setShininess(30.0f);

      for(size_t i=0; i<250; ++i) {
        Block *b = new Block();
        b->setPosition(vector4( 150+GetRand()*150.0f, GetRand2()*800.0f, GetRand2()*1400.0f-1000));
        m_rblocks.push_back(b);
      }
      for(size_t i=0; i<250; ++i) {
        Block *b = new Block();
        b->setPosition(vector4(-150-GetRand()*150.0f, GetRand2()*800.0f, GetRand2()*1400.0f-1000));
        m_lblocks.push_back(b);
      }
    }

    void checkLocalTeam()
    {
      team_ptr lt = GetLocalTeam();
      team_ptr teams[2];
      for(int i=0; i<2; ++i) { teams[i]=ITeam::getTeam(i); }

      if(!lt) {
        m_local_team = 0;
        for(size_t i=0; i<m_rblocks.size(); ++i) { m_rblocks[i]->setVel(vector4(0,0,-5)); }
        for(size_t i=0; i<m_lblocks.size(); ++i) { m_lblocks[i]->setVel(vector4(0,0,-5)); }
        m_basecolor[0]=m_basecolor[1]=vector4(1,1,2);
        m_updown[0]=0; m_updown[1]=0;
      }
      else if(lt->getID()!=m_local_team) {
        m_local_team = lt->getID();
        bool first = lt->getID()==teams[0]->getID();
        vector4 vel(0,0, first ? 5 : -5);
        for(size_t i=0; i<m_rblocks.size(); ++i) { m_rblocks[i]->setVel( vel); }
        for(size_t i=0; i<m_lblocks.size(); ++i) { m_lblocks[i]->setVel(-vel); }
        if(first) {
          m_basecolor[0]=vector4(2,1,1); m_basecolor[1]=vector4(1,1,2);
          m_updown[0]=1; m_updown[1]=0;
        }
        else {
          m_basecolor[0]=vector4(1,1,2); m_basecolor[1]=vector4(2,1,1);
          m_updown[0]=0; m_updown[1]=1;
        }
      }
    }


    void drawRect(const vector2& ur, const vector2& bl)
    {
      glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 1.0f);
      glVertex2f(bl.x, bl.y);
      glTexCoord2f(0.0f, 0.0f);
      glVertex2f(bl.x, ur.y);
      glTexCoord2f(1.0f, 0.0f);
      glVertex2f(ur.x, ur.y);
      glTexCoord2f(1.0f, 1.0f);
      glVertex2f(ur.x, bl.y);
      glEnd();
    }

    void draw()
    {
      {
        ist::ProjectionMatrixSaver pm;
        ist::ModelviewMatrixSaver mm;
        m_cam.look();

        m_fog.enable();
        m_bgmat.assign();
        for(size_t i=0; i<m_rblocks.size(); ++i) {
          m_rblocks[i]->draw();
        }
        for(size_t i=0; i<m_lblocks.size(); ++i) {
          m_lblocks[i]->draw();
        }
        ist::Material().assign();
        m_fog.disable();
      }

      if(GetConfig()->shader && !GetConfig()->simplebg) {
        draw_gl20();
      }

      drawFadeEffect();

      glClear(GL_DEPTH_BUFFER_BIT);
    }

    void draw_gl20()
    {
      if(!m_hline) {
        for(int i=0; i<4; ++i) {
          m_ltex[i] = new LineTexture();
        }
        for(size_t i=0; i<m_rblocks.size(); ++i) {
          m_rblocks[i]->setLineTexture(m_ltex[i%4]);
        }
        for(size_t i=0; i<m_lblocks.size(); ++i) {
          m_lblocks[i]->setLineTexture(m_ltex[i%4]);
        }

        m_hline = new ist::ProgramObject();
        m_hline->attach(GetVertexShader("team.vsh"));
        m_hline->attach(GetFragmentShader("team.fsh"));
        m_hline->link();

        m_glow = new ist::ProgramObject();
        m_glow->attach(GetFragmentShader("glow.fsh"));
        m_glow->link();

        m_fbo_lines = new ist::FrameBufferObject(640, 480, GL_COLOR_BUFFER_BIT);
        m_fbo_tmp = new ist::FrameBufferObject(640, 480, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_fbo_glow = new ist::FrameBufferObject(256, 256);
        m_fbo_glow_b = new ist::FrameBufferObject(256, 256);
      }

      {
        ist::ProjectionMatrixSaver pm;
        ist::ModelviewMatrixSaver mm;

        m_cam.look();
        matrix44 icam;
        glGetFloatv(GL_MODELVIEW_MATRIX, icam.v);

        glDisable(GL_LIGHTING);

        // 溝を描画 
        m_fbo_tmp->enable();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_TEXTURE_2D);
        m_hline->enable();
        m_hline->setMatrix4fv("icam", 1, false, icam.invert().v);
        m_hline->setUniform1f("sz", float(m_frame));

        m_hline->setUniform4f("basecolor", m_basecolor[0].x, m_basecolor[0].y, m_basecolor[0].z, m_basecolor[0].w);
        m_hline->setUniform1i("updown", m_updown[0]);
        for(size_t i=0; i<m_rblocks.size(); ++i) {
          m_rblocks[i]->assign();
          m_rblocks[i]->draw();
          m_rblocks[i]->disassign();
        }

        m_hline->setUniform4f("basecolor", m_basecolor[1].x, m_basecolor[1].y, m_basecolor[1].z, m_basecolor[1].w);
        m_hline->setUniform1i("updown", m_updown[1]);
        for(size_t i=0; i<m_lblocks.size(); ++i) {
          m_lblocks[i]->assign();
          m_lblocks[i]->draw();
          m_lblocks[i]->disassign();
        }
        m_hline->disable();
        glDisable(GL_TEXTURE_2D);
        m_fbo_tmp->disable();

        // フィードバックブラー 
        m_fbo_lines->enable();
        if(m_frame==1) {
          glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
          glClear(GL_COLOR_BUFFER_BIT);
        }
        {
          ScreenMatrix sm;

          glDisable(GL_DEPTH_TEST);
          glDepthMask(GL_FALSE);

          m_fbo_lines->assign();
          glEnable(GL_TEXTURE_2D);
          vector2 str(4.0f, 3.0f);
          glColor4f(1.0f, 1.0f, 1.0f, 0.4f);
          drawRect(vector2(640.0f, 480.0f)+str*3.0f, vector2(0.0f, 0.0f)-str*3.0f);
          drawRect(vector2(640.0f, 480.0f)+str*2.0f, vector2(0.0f, 0.0f)-str*2.0f);
          drawRect(vector2(640.0f, 480.0f)+str*1.0f, vector2(0.0f, 0.0f)-str*1.0f);
          glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
          glDisable(GL_TEXTURE_2D);
          m_fbo_lines->disassign();

          glColor4f(0.0f, 0.0f, 0.0f, 0.4f);
          drawRect(vector2(640,480), vector2(0,0));
          glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

          m_fbo_tmp->assign();
          glEnable(GL_TEXTURE_2D);
          glBlendFunc(GL_SRC_ALPHA, GL_ONE);
          glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
          drawRect(vector2(640,480), vector2(0,0));
          glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
          glDisable(GL_TEXTURE_2D);
          m_fbo_tmp->disassign();

          glDepthMask(GL_TRUE);
          glEnable(GL_DEPTH_TEST);
        }
        m_fbo_lines->disable();

        glEnable(GL_LIGHTING);
      }

      {
        ScreenMatrix sm;
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);

        // テクスチャ張った状態を合成 
        m_fbo_lines->assign();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        drawRect(vector2(640,480), vector2(0,0));
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        m_fbo_lines->disassign();

        // テクスチャ張った状態をぼかす(グローエフェクト化) 
        m_fbo_lines->assign();
        m_glow->enable();
        m_glow->setUniform1f("width", float(m_fbo_glow->getWidth()));
        m_glow->setUniform1f("height", float(m_fbo_glow->getHeight()));
        for(int i=0; i<2; ++i) {
          if(i!=0) {
            swap(m_fbo_glow, m_fbo_glow_b);
            m_fbo_glow_b->assign();
          }
          m_fbo_glow->enable();
          m_glow->setUniform1i("pass", i+1);
          DrawRect(vector2(640,480), vector2(0,0));
          m_fbo_glow->disable();
          m_fbo_glow->disassign();
        }
        m_glow->disable();


        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        // グローエフェクトを描画 
        m_fbo_glow->assign();
        glColor4f(0.3f, 0.3f, 1.0f, 1.0f);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        drawRect(vector2(640,480), vector2(0,0));
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1,1,1,1);
        m_fbo_glow->disassign();
        
      // 確認用 
      //  m_fbo_lines->assign();
      //  DrawRect(vector2(float(m_fbo_lines->getWidth()), float(m_fbo_lines->getHeight())), vector2(0.0f, 0.0f));
      //  m_fbo_lines->disassign();

        glDisable(GL_TEXTURE_2D);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      m_cam.setPosition(vector4(0, -50, 400) + GetCamera().getTarget()*0.5f);
      m_cam.setTarget(vector4(0, 0, 0) + GetCamera().getTarget()*0.5f);

      checkLocalTeam();
      for(size_t i=0; i<m_rblocks.size(); ++i) { m_rblocks[i]->update(); }
      for(size_t i=0; i<m_lblocks.size(); ++i) { m_lblocks[i]->update(); }
      std::sort(m_rblocks.begin(), m_rblocks.end(), greater_z<block_ptr>());
      std::sort(m_lblocks.begin(), m_lblocks.end(), greater_z<block_ptr>());

      ++m_frame;
    }
  };




  class Fortress : public Inherit2(HaveTeam, ChildRotLayer)
  {
  typedef Inherit2(HaveTeam, ChildRotLayer) Super;
  public:

    class Parts : public ChildGround
    {
    typedef ChildGround Super;
    private:
      vector4 m_emission;

    public:
      Parts(Deserializer& s) : Super(s)
      {
        s >> m_emission;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_emission;
      }

    public:
      Parts() {}
      void setEmission(const vector4& v) { m_emission=v; }

      void draw()
      {
        if(m_emission.x > 0.0f) {
          glMaterialfv(GL_FRONT, GL_EMISSION, m_emission.v);
          Super::draw();
          glMaterialfv(GL_FRONT, GL_EMISSION, vector4().v);
        }
        else {
          Super::draw();
        }
      }

      void onDestroy(DestroyMessage& m)
      {
        Super::onDestroy(m);
        PutMediumImpact(getCenter());
      }
    };

    class Core : public Inherit3(HaveTeam, HaveInvincibleMode, ChildEnemy)
    {
    typedef Inherit3(HaveTeam, HaveInvincibleMode, ChildEnemy) Super;
    private:
      float m_life_boost;
      float4 m_lifegage;

    public:
      Core(Deserializer& s) : Super(s)
      {
        s >> m_life_boost >> m_lifegage;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_life_boost << m_lifegage;
      }


    public:
      Core(float life) : m_life_boost(life)
      {
        setBox(box(vector4(50)));
        setLife(getMaxLife());
      }

      float getMaxLife()
      {
        float l[] = {200, 300, 400, 500, 600};
        return l[GetLevel()]*m_life_boost;
      }

      void drawLifeGauge()
      {}

      void drawStatus()
      {
        vector2 ppos = GetProjectedPosition(getPosition());
        {
          ppos.x = std::max<float>(ppos.x,  30.0f);
          ppos.x = std::min<float>(ppos.x, 610.0f);
          ppos.y = std::max<float>(ppos.y, -40.0f);
          ppos.y = std::min<float>(ppos.y, 420.0f);
        }
        vector2 bl = ppos-vector2(30,-50);
        glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
        DrawRect(bl, bl+vector2(60, 6)); // ライフゲージ 
        glColor4fv(m_lifegage.v);
        DrawRect(bl+vector2(1,1), bl+vector2(getLife()/getMaxLife()*60.0f, 6)-vector2(1,1));
        glColor4f(1,1,1,1);
      }

      void draw()
      {
        if(!isDamaged()) {
          vector4 color = (getTeam() && getTeam()==GetLocalTeam()) ? vector4(0.3f,0.3f,1.0f) : vector4(1.0f,0.3f,0.3f);
          glMaterialfv(GL_FRONT, GL_DIFFUSE,  color.v);
          Super::draw();
          glMaterialfv(GL_FRONT, GL_DIFFUSE,  vector4(0.8f,0.8f,0.8f).v);
        }
        else {
          Super::draw();
        }
      }

      void updateEmission()
      {
        vector4 emission = getEmission();
        if(isInvincible()) {
          emission+=(vector4(1.0f, 1.0f, 1.0f)-emission)*0.03f;
        }
        else {
          vector4 color = (getTeam() && getTeam()==GetLocalTeam()) ? vector4(0.4f,0.4f,1.0f) : vector4(1.0f,0.4f,0.4f);
          emission+=(color-emission)*0.03f;
        }
        setEmission(emission);
      }

      void updateEmission(bool v)
      {}

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);
        m_lifegage+=(float4(0.8f, 0.8f, 1.0f, 0.9f)-m_lifegage)*0.05f;
        if(isInvincible()) {
          m_lifegage = float4(0.1f,0.1f,1.0f,0.9f);
        }
      }

      void onCollide(CollideMessage& m)
      {
        gobj_ptr p = m.getFrom();
        if(p && (IsFraction(p) || typeid(*p)==typeid(MediumBlock&) || typeid(*p)==typeid(LargeBlock&))) {
          SendDestroyMessage(0, p);
        }
      }

      void onDamage(DamageMessage& m)
      {
        if(!dynamic_cast<GLaser*>(m.getSource()) &&
           !dynamic_cast<Ray*>(m.getSource())) { // 自機の武装無効 
          Super::onDamage(m);
          m_lifegage = float4(1.0f, 0.0f, 0.0f, 1.0f);
        }
      }

      int getFractionCount() { return 0; }

      void onDestroy(DestroyMessage& m)
      {
        Super::onDestroy(m);
        PutBossDestroyImpact(getCenter());
      }

      void onLifeZero(gobj_ptr from)
      {
        if(player_ptr pl=ToPlayer(from)) {
          PushChatText("# "+pl->getName()+" destroyed the core!");
        }
        PutSmallExplode(getBox(), getMatrix(), getExplodeCount()/2);
        PutBigImpact(getCenter());
      }
    };

  private:
    Core *m_core;
    SmallGear *m_gear[2];
    Parts *m_presser[2];
    Parts *m_parts[7];
    ArmorShip::Fort *m_turret[4];
    bool m_ih;
    int m_frame;
    int m_state;
    box m_move_area;

  public:
    Fortress(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_core);
      DeserializeLinkage(s, m_gear);
      DeserializeLinkage(s, m_presser);
      DeserializeLinkage(s, m_parts);
      DeserializeLinkage(s, m_turret);
      s >> m_ih >> m_frame >> m_state >> m_move_area;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_core);
      SerializeLinkage(s, m_gear);
      SerializeLinkage(s, m_presser);
      SerializeLinkage(s, m_parts);
      SerializeLinkage(s, m_turret);
      s << m_ih << m_frame << m_state << m_move_area;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_core);
      ReconstructLinkage(m_gear);
      ReconstructLinkage(m_presser);
      ReconstructLinkage(m_parts);
      ReconstructLinkage(m_turret);
    }

  public:
    Fortress(float life, bool invert) : m_ih(invert), m_frame(0), m_state(0)
    {
      ZeroClear(m_turret);
      setPosition(vector4(540*(invert?-1:1), 0, 0));
      m_move_area = box(getPosition()+vector4(0, 200, 1), getPosition()-vector4(0, 300, -1));

      gid g = Solid::createGroupID();
      m_core = new Core(life);
      m_core->setParent(this);
      m_core->setGroup(g);
      m_core->setInvincible(false);

      {
        box b[7] = {
          box(vector4(  55, 130, 30), vector4( 150,-130, -30)), // 右 

          box(vector4( 100,  55, 50), vector4(-100, 210, -50)), // 上 
          box(vector4( -70,  65, 60), vector4(-180, 170, -60)), // 上 
          box(vector4(-150,  55, 40), vector4(-250, 130, -40)), // 上 

          box(vector4( 100, -55, 50), vector4(-100,-210, -50)), // 下 
          box(vector4( -70, -65, 60), vector4(-180,-170, -60)), // 下 
          box(vector4(-150, -55, 40), vector4(-250,-130, -40)), // 上 
        };
        for(int i=0; i<7; ++i) {
          Parts *p = new Parts();
          p->setParent(this);
          p->setBox(b[i]);
          p->setGroup(g);
          m_parts[i] = p;
        }
      }
      {
        vector4 pos[4] = {
          vector4(-250,  90, 0),
          vector4(-100, 200, 0),
          vector4(-250, -90, 0),
          vector4(-100,-200, 0),
        };
        for(int i=0; i<4; ++i) {
          m_turret[i] = new ArmorShip::Fort();
          m_turret[i]->setPosition(pos[i]);
          m_turret[i]->setParent(this);
          m_turret[i]->setGroup(g);
          m_turret[i]->setCooldown(100);
          m_turret[i]->setSearchRange(8);
          m_turret[i]->setInitialDirection(vector4(-1,0,0));
        }
      }

      {
        box b[2] = {
          box(vector4(-85, 0, 25), vector4(-165, 70, -25)),
          box(vector4(-85, 0, 25), vector4(-165,-70, -25)),
        };
        for(int i=0; i<2; ++i) {
          m_presser[i] = new Parts();
          m_presser[i]->setParent(this);
          m_presser[i]->setBox(b[i]);
          m_presser[i]->setGroup(g);
        }
      }
      for(int i=0; i<2; ++i) {
        RotLayer *rl = new RotLayer();
        rl->setParent(this);
        rl->setAxis(vector4(1,0,0));
        rl->setRotate(i==0 ? 180 : 0);
        rl->setPosition(vector4(-210, 170*(i==0 ? 1 : -1), 0));

        m_gear[i] = new SmallGear();
        m_gear[i]->setParent(rl);
        m_gear[i]->setMinRotate(0);
        m_gear[i]->setMaxRotate(360.0f*3.0f);
        m_gear[i]->setIncrementOnly(true);
        m_gear[i]->setReverseSpeed(0.03f);
        m_gear[i]->setRotate(360.0f*3.0f);
        m_gear[i]->setGroup(g);
      }
    }

    void setInvert(bool v) { modMatrix(); m_ih=v; }

    void setTeam(team_ptr v)
    {
      Super::setTeam(v);
      if(m_core) {
        m_core->setTeam(v);
      }
      for(int i=0; i<4; ++i) {
        if(m_turret[i]) {
          m_turret[i]->setTeam(v);
        }
      }
    }

    void updateMatrix(matrix44& mat)
    {
      Super::updateMatrix(mat);
      if(m_ih) {
        mat.rotateY(180.0f);
      }
    }

    Core *getCore() { return m_core; }

    void destroy()
    {
      m_core->setEmission(vector4(1.2f/400*m_frame, 0.5f/400.0f*m_frame, 0));
      for(int i=0; i<7; ++i) {
        if(m_parts[i]) {
          m_parts[i]->setEmission(vector4(1.2f/400*m_frame, 0.5f/400.0f*m_frame, 0));
        }
      }
      if(m_frame%30==10) {
        Shine *s = new Shine(m_core);
      }
      if(m_frame==1) {
        Destroy(getTSM(), m_gear, 1);
        Destroy(getTSM(), m_presser, 1);
        Destroy(getTSM(), m_turret, 1);
      }
      if(m_frame==250) { SendDestroyMessage(0, m_parts[3], 1); }
      if(m_frame==300) { SendDestroyMessage(0, m_parts[6], 1); }
      if(m_frame==340) { SendDestroyMessage(0, m_parts[5], 1); }
      if(m_frame==370) { SendDestroyMessage(0, m_parts[2], 1); }
      if(m_frame==390) { SendDestroyMessage(0, m_parts[4], 1); }
      if(m_frame==400) { SendDestroyMessage(0, m_parts[0], 1); }
      if(m_frame==410) { SendDestroyMessage(0, m_parts[1], 1); }
      if(m_frame==420) {
        SendDestroyMessage(0, m_core);
        SendKillMessage(0, this);
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      SweepDeadObject(m_core);
      SweepDeadObject(m_gear);
      SweepDeadObject(m_presser);
      SweepDeadObject(m_parts);
      SweepDeadObject(m_turret);

      int f = ++m_frame;
      if(m_state!=1 && m_core && m_core->getLife()<=0.0f) {
        m_frame = 0;
        m_state = 1;
      }
      if(m_state==1) {
        destroy();
      }
      else {
        if(player_ptr pl = GetNearestPlayer(getPosition(), getTeam())) {
          vector4 tpos = pl->getPosition();
          tpos = m_move_area.inner(tpos);
          vector4 pos = getPosition();
          pos-=(pos-tpos)*0.002f;
          setPosition(pos);
        }
      }

      for(int i=0; i<2; ++i) {
        if(m_gear[i]) {
          if(f<30) {
            m_gear[i]->setRotateSpeed(0.0f);
          }
          if(f==30) {
            m_gear[i]->setRotateSpeed(-15.0f);
          }
        }
      }

      {
        float r = 0;
        for(int i=0; i<2; ++i) {
          if(m_gear[i]) {
            r = std::max<float>(r, m_gear[i]->getRotate());
          }
        }
        for(int i=0; i<2; ++i) {
          if(m_presser[i]) {
            float y = 70.0f/(360.0f*3.0f)*r * (i==0 ? 1 : -1);
            m_presser[i]->setPosition(vector4(0, y, 0));
          };
        }
      }
    }

    void onDestroy(DestroyMessage& m)
    {
      // デフォルトの挙動握り潰し 
      if(m_core) {
        m_core->setLife(0);
      }
    }
  };


  class Wave1 : public IWave
  {
  typedef IWave Super;
  public:
    Wave1(Deserializer& s) : Super(s) {}

  public:
    Wave1()
    {}

    void progress(int frame)
    {
      if(frame%25==0) {
        MediumBlock *b = putMediumBlock(vector4(0, 600, 0)+vector4(brand()*320.0f, brand()*50.0f, 0));
        b->setEnergy(b->getEnergy()*2.0f);
      }
      if(frame%110==0) {
        LargeBlock *b = putLargeBlock(vector4(0, 600, 0)+vector4(brand()*180.0f, brand()*50.0f, 0));
        b->setEnergy(b->getEnergy()*2.0f);
     }

      if(frame>2000) {
      //  SendKillMessage(0, this);
      }
    }
  };



  class Black : public Optional
  {
  typedef Optional Super;
  private:
    float m_opa;

  public:
    Black(Deserializer& s) : Super(s)
    {
      s >> m_opa;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_opa;
    }

  public:
    Black() : m_opa(0.0f)
    {}

    float getDrawPriority() { return 30.0f; }

    void draw()
    {
      glColor4f(0,0,0,m_opa);
      DrawRect(vector2(-1,-1), vector2(641, 481));
      glColor4f(1,1,1,1);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      m_opa = std::min<float>(0.6f, m_opa+0.02f);
    }
  };


  class Result : public Optional
  {
  typedef Optional Super;
  private:
    wstring m_message;
    wstring m_display;

  public:
    Result(Deserializer& s) : Super(s)
    {
      s >> m_message >> m_display;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_message << m_display;
    }

  public:
    Result(const wstring& m) : m_message(m)
    {}

    float getDrawPriority() { return 30.0f; }

    void draw()
    {
      ScreenMatrix sm;
      glDisable(GL_LIGHTING);
      glDisable(GL_DEPTH_TEST);

      DrawText(m_display, sgui::Point(220, 120));

      glEnable(GL_LIGHTING);
      glEnable(GL_DEPTH_TEST);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      if(m_display.size()!=m_message.size()) {
        m_display.push_back(m_message[m_display.size()]);
      }
    }
  };



  class Rule : public RuleBase
  {
  typedef RuleBase Super;
  private:
    Background *m_bg;
    team_ptr m_team[2];
    Fortress::Core *m_core[2];
    IWave *m_wave_obj;
    int m_state;
    enum {
      NORMAL,
      GAMESET,
    };

  public:
    Rule(Deserializer& s) : Super(s)
    {
      s >> m_state;
      DeserializeLinkage(s, m_bg);
      DeserializeLinkage(s, m_team);
      DeserializeLinkage(s, m_core);
      DeserializeLinkage(s, m_wave_obj);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_state;
      SerializeLinkage(s, m_bg);
      SerializeLinkage(s, m_team);
      SerializeLinkage(s, m_core);
      SerializeLinkage(s, m_wave_obj);
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_bg);
      ReconstructLinkage(m_team);
      ReconstructLinkage(m_core);
      ReconstructLinkage(m_wave_obj);
    }

  public:
    Rule(float life) : m_state(NORMAL)
    {
      SetCameraMovableArea(vector2(200,170), vector2(-200,-170));
      SetGlobalPlayerBound(box(vector4(583, 456, 100)));
      SetGlobalAccel(vector4(0, -0.012f, 0));
      SetGlobalBoundBox(box(vector4(1000)));

      for(int i=0; i<2; ++i) {
        m_team[i] = CreateTeam();
      }
      for(int i=0; i<2; ++i) {
        Fortress *s = new Fortress(life, i==0);
        s->setTeam(m_team[i]);
        m_core[i] = s->getCore();
      }
      m_wave_obj = new Wave1();

      GetMusic("vs.ogg")->play();
      m_bg = new Background();
    }


    void updateServerInfomation()
    {
      if(GetGame()->getPlayerCount()>0) {
        char buf[128];
        sprintf(buf, "team_fortress+%s+", GetLevelString().c_str());
        UpdateServerInfomation(buf);
      }
    }

    void join(pinfo_ptr pi)
    {
      updateServerInfomation();
      if(m_team[0]->getRecordCount()<=m_team[1]->getRecordCount()) {
        m_team[0]->join(pi->getSessionID());
        pi->setPosition(vector4(-200,0,0));
      }
      else {
        m_team[1]->join(pi->getSessionID());
        pi->setPosition(vector4( 200,0,0));
      }
      if(m_state!=GAMESET) {
        pi->setRespawnTime(60*10);
      }
    }

    void leave(pinfo_ptr pi)
    {
      updateServerInfomation();
      if(team_ptr t = ITeam::getTeamBySID(pi->getSessionID())) {
        t->leave(pi->getSessionID());
      }
    }


    void progressNormal(int f)
    {
      for(size_t i=0; i<GetPlayerCount(); ++i) {
        pinfo_ptr pi = GetPlayerInfo(i);
        if(!pi->getPlayer()) {
          if(pi->getRespawnTime()==0) {
            int n = m_team[0]->isInclude(pi->getSessionID()) ?
              m_team[0]->getPlayerCount() : m_team[1]->getPlayerCount();
            pi->setRespawnTime(60*(5+5*n));
          }
          else if(pi->getRespawnTime()==10) {
            vector4 pos = m_team[0]->isInclude(pi->getSessionID()) ? vector4(-200,0,0) : vector4(200,0,0);
            pi->setPosition(pos);
          }
        }
      }

      if(m_core[0]->getLife()<=0.0f || m_core[1]->getLife()<=0.0f) {
        m_state = GAMESET;
        setFrame(0);
        if(m_core[0]->getLife()<=0.0f && m_core[1]->getLife()>0.0f) {
          m_core[1]->setInvincible(true);
        }
        if(m_core[1]->getLife()<=0.0f && m_core[0]->getLife()>0.0f) {
          m_core[0]->setInvincible(true);
        }
      }
    }

    void clearEnemy()
    {
      gobj_iter& i = GetAllObjects();
      while(i.has_next()) {
        gobj_ptr p = i.iterate();
        if(enemy_ptr e = ToEnemy(p)) {
          if(e!=m_core[0] && e!=m_core[1]) {
            SendDestroyMessage(0, e, 1);
          }
        }
      }
    }

    void progressGameSet(int f)
    {
      for(size_t i=0; i<GetPlayerCount(); ++i) {
        pinfo_ptr pi = GetPlayerInfo(i);
        if(!pi->getPlayer()) {
          pi->setRespawnTime(9999);
        }
      }

      if(f==1) {
        InvincibleAllPlayers(9999);
        clearEnemy();
        SendKillMessage(0, m_wave_obj);
      }
      if(f>=500 && f%40==0) {
        for(int i=0; i<2; ++i) {
          if(!m_core[i]) {
            team_ptr t = m_team[i];
            for(int j=0; j<t->getPlayerCount(); ++j) {
              if(player_ptr pl=t->getPlayer(j)) {
                SendDestroyMessage(0, pl);
                break;
              }
            }
          }
        }
      }
      if(f==700) {
        string m;
        char buf[128];
        team_ptr lt = GetLocalTeam();
        if(!m_core[0] && !m_core[1]) {
          m+="                draw\n\n";
        }
        else if( (m_core[0] && !m_core[1] && m_team[0]==lt)
          ||(m_core[1] && !m_core[0] && m_team[1]==lt)) {
          m+="                you win!\n\n";
        }
        else {
          m+="                you lose...\n\n";
        }

        for(int i=0; i<2; ++i) {
          sprintf(buf, "team%d\n\n", i+1);
          m+=buf;
          for(int j=0; j<m_team[i]->getRecordCount(); ++j) {
            ITeam::IPlayerRecord& r = m_team[i]->getRecord(j);
            sprintf(buf, "%10s : %d kill, %d death\n", r.getName().c_str(), r.getPlayerKill(), r.getDeath());
            m+=buf;
          }
          m+="\n";
        }
        new Black();
        new Result(_L(m));
      }

      if(f==1200) {
        ReturnToTitle();
      }
    }

    void progress(int f)
    {
      SweepDeadObject(m_wave_obj);
      SweepDeadObject(m_core);
      if(m_state==NORMAL) {
        progressNormal(f);
      }
      else if(m_state==GAMESET) {
        progressGameSet(f);
      }
    }


    void draw()
    {
      if(m_state==NORMAL) {
        drawRespawnTime();
      }
      for(int i=0; i<2; ++i) {
        if(m_core[i] && m_core[i]->getLife()>0.0f) {
          m_core[i]->drawStatus();
        }
      }
    }
  };

} // namespace team_fortress 
} // namespace rule 
} // namespace exception 
#endif
