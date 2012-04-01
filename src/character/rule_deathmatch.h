#ifndef rule_deathmatch_h
#define rule_deathmatch_h

namespace exception {
namespace rule {
namespace deathmatch {


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

    class Block : public Inherit4(Box, HavePosition, Dummy, RefCounter)
    {
    typedef Inherit4(Box, HavePosition, Dummy, RefCounter) Super;
    private:
      ltex_ptr m_lt;
      vector4 m_vel;
      bool m_stop;

    public:
      Block(Deserializer& s) : Super(s)
      {
        s >> m_vel >> m_stop;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_vel << m_stop;
      }

    public:
      Block() : m_stop(false)
      {
        setBox(box(vector4(30)));
      }

      void setLineTexture(ltex_ptr v) { m_lt=v; }
      void assign() { m_lt->assign(); }
      void disassign() { m_lt->disassign(); }
      void stop() { m_stop=true; }

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
        if(pos.z > 400.0f && !m_stop) {
          pos.z-=2000.0f;
        }
        else if(pos.z < -1600.0f && !m_stop) {
          pos.z+=2000.0f;
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

    ist::PerspectiveCamera m_cam;
    ist::Fog m_fog;
    ist::Material m_bgmat;
    block_cont m_blocks;
    int m_frame;
    vector4 m_basecolor;

  public:
    Background(Deserializer& s) : Super(s)
    {
      s >> m_cam >> m_fog >> m_bgmat;
      size_t size;
      s >> size;
      for(size_t i=0; i<size; ++i) {
        m_blocks.push_back(new Block(s));
      }
      s >> m_frame >> m_basecolor;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_cam << m_fog << m_bgmat;
      s << m_blocks.size();
      for(size_t i=0; i<m_blocks.size(); ++i) {
        m_blocks[i]->serialize(s);
      }
      s << m_frame << m_basecolor;
    }

  public:
    Background() : m_frame(0)
    {
      m_cam.setPosition(vector4(0, 0, 400));
      m_cam.setTarget(vector4(0, 0, 0));
      m_cam.setFovy(60.0f);
      m_cam.setZFar(10000.0f);
      m_cam.setDirection(vector3(1.0f, 1.0f, 0.0f).normal());

      m_fog.setColor(vector4(0.0f, 0.0f, 0.0f));
      m_fog.setNear(0.0f);
      m_fog.setFar(1800.0f);

      m_bgmat.setDiffuse(vector4(0.2f, 0.2f, 0.6f));
      m_bgmat.setShininess(30.0f);
      m_bgmat.setDiffuse(vector4(0.2f, 0.2f, 0.6f));

      m_basecolor = vector4(1.0f, 1.0f, 2.0f);
      m_bgmat.setDiffuse(vector4(0.2f, 0.2f, 0.6f));

      vector4 ring[20] = {
        vector4(-250, -250, 0),
        vector4(-150, -250, 0),
        vector4( -50, -250, 0),
        vector4(  50, -250, 0),
        vector4( 150, -250, 0),
        vector4( 250, -250, 0),

        vector4( 250, -150, 0),
        vector4( 250,  -50, 0),
        vector4( 250,   50, 0),
        vector4( 250,  150, 0),
        vector4( 250,  250, 0),

        vector4( 150,  250, 0),
        vector4(  50,  250, 0),
        vector4( -50,  250, 0),
        vector4(-150,  250, 0),
        vector4(-250,  250, 0),

        vector4(-250,  150, 0),
        vector4(-250,   50, 0),
        vector4(-250,  -50, 0),
        vector4(-250, -150, 0),
      };
      for(size_t i=0; i<20; ++i) {
        for(size_t j=0; j<20; ++j) {
          Block *b = new Block();
          b->setPosition(ring[j]+vector4(0,0,100)*i);
          b->setVel(vector4(0, 0,-7.0f));
          m_blocks.push_back(b);
        }
      }
    }

    void stop()
    {
      for(size_t i=0; i<m_blocks.size(); ++i) {
        m_blocks[i]->stop();
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
        for(size_t i=0; i<m_blocks.size(); ++i) {
          m_blocks[i]->draw();
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
        for(size_t i=0; i<m_blocks.size(); ++i) {
          m_blocks[i]->setLineTexture(m_ltex[i%4]);
        }

        m_hline = new ist::ProgramObject();
        m_hline->attach(GetVertexShader("deathmatch.vsh"));
        m_hline->attach(GetFragmentShader("deathmatch.fsh"));
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
        m_hline->setUniform4f("basecolor", m_basecolor.x, m_basecolor.y, m_basecolor.z, m_basecolor.w);
        for(size_t i=0; i<m_blocks.size(); ++i) {
          m_blocks[i]->assign();
          m_blocks[i]->draw();
          m_blocks[i]->disassign();
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

      if(round2()) {
        m_basecolor-=(m_basecolor-vector4(2,1,1))*0.01f;
        m_fog.setColor(m_fog.getColor()-(m_fog.getColor()-vector4(0.1f, 0.0f, 0.0f))*0.01f);
        m_bgmat.setDiffuse(m_bgmat.getDiffuse()-(m_bgmat.getDiffuse()-vector4(0.4f, 0.2f, 0.2f))*0.01f);
      }

      const vector4& color = m_fog.getColor();
      sgui::View::instance()->setBackgroundColor(sgui::Color(color.x, color.y, color.z));

      m_cam.setPosition(vector4(0, 0, 400) + GetCamera().getTarget());
      m_cam.setTarget(vector4(0, 0, 0) + GetCamera().getTarget());

      for(size_t i=0; i<m_blocks.size(); ++i) {
        m_blocks[i]->update();
      }
      std::sort(m_blocks.begin(), m_blocks.end(), greater_z<block_ptr>());

      ++m_frame;
    }

    void onKill(KillMessage& m)
    {
      sgui::View::instance()->setBackgroundColor(sgui::Color(0, 0, 0));
      Super::onKill(m);
    }
  };





  class Wave1 : public IWave
  {
  typedef IWave Super;
  public:
    Wave1(Deserializer& s) : Super(s) {}

  public:
    Wave1()
    {
    }

    void progress(int frame)
    {
      if(frame%25==0) {
        putMediumBlock(vector4(0, 600, 0)+vector4(brand()*300.0f, brand()*50.0f, 0));
      }
      if(frame%80==0) {
        putLargeBlock(vector4(0, 600, 0)+vector4(brand()*220.0f, brand()*50.0f, 0));
      }

      if(frame>1000 && frame<=2500 && frame%250==0) {
        putSpinningGroundWithMineHatch(
          vector4(650, brand()*280.0f-50.0f, 0),
          vector4(-1.0f, 0.4f*brand(), 0)*1.3f,
          brand()*0.9f);
      }
      if(frame>1000 && frame<=2500 && frame%250==125) {
        putSpinningGroundWithMineHatch(
          vector4(-650, brand()*280.0f-50.0f, 0),
          vector4( 1.0f, 0.4f*brand(), 0)*1.3f,
          brand()*0.9f);
      }

      if(frame>=3000 && frame<=4500 && frame%250==0) {
        if(frame/250%2==0) {
          vector4 pos[4] = {
            vector4( 120, 280, 0),vector4( 120,-280, 0),
            vector4(-120, 280, 0),vector4(-120, -280, 0),
          };
          for(int i=0; i<4; ++i) {
            for(int j=0; j<3; ++j) {
              vector4 x(i<2 ? 100 : -100, 0, 0);
              putZab(pos[i]+x*j);
            }
          }
        }
        else {
          vector4 pos[2] = {
            vector4(-370, 300, 0),
            vector4( 370, 300, 0),
          };
          for(int i=0; i<2; ++i) {
            for(int j=0; j<6; ++j) {
              vector4 x(0,-120, 0);
              putZab(pos[i]+x*j);
            }
          }
        }
      }

      if(frame>5000 && frame<=6500 && frame%450==0) {
        putSpinningGroundWithFighterHatch(
          vector4(650, brand()*250.0f-50.0f, 0),
          vector4(-1.0f, 0.4f*brand(), 0)*1.2f,
          brand()*0.6f);
      }
      if(frame>5000 && frame<=6500 && frame%450==225) {
        putSpinningGroundWithFighterHatch(
          vector4(-650, brand()*250.0f-50.0f, 0),
          vector4( 1.0f, 0.4f*brand(), 0)*1.2f,
          brand()*0.6f);
      }

      if(frame>=7000 && frame<=8500 && frame%600==0) {
        vector4 pos[4] = {
          vector4( 800, 350, 0), vector4( 800,-350, 0),
          vector4(-800, 110, 0), vector4(-800,-110, 0),
        };
        vector4 dir[4] = {
          vector4(-1,0,0), vector4(-1,0,0),
          vector4( 1,0,0), vector4( 1,0,0),
        };
        for(int i=0; i<4; ++i) {
          putHeavyFighter(pos[i], dir[i]);
        }
      }

      if(frame==9000 || frame==9500) {
        vector4 pos[2] = {
          vector4( 300,-1100, 0),
          vector4(-300,-1100, 0),
        };
        vector4 dir[2] = {
          vector4(0, 1, 0),
          vector4(0, 1, 0),
        };
        for(int i=0; i<2; ++i) {
          putWeakIterator(pos[i], dir[i]);
        }
      }

      if(frame==10000) {
        setFrame(0);
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
    typedef std::vector<team_ptr> team_cont;
    Background *m_bg;
    IWave *m_wave_obj;
    team_cont m_team;
    int m_time_limit;
    int m_state;
    enum {
      NORMAL,
      GAMESET,
    };

  public:
    Rule(Deserializer& s) : Super(s)
    {
      s >> m_time_limit >> m_state;
      DeserializeLinkage(s, m_bg);
      DeserializeLinkage(s, m_wave_obj);
      DeserializeLinkageContainer(s, m_team);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_time_limit << m_state;
      SerializeLinkage(s, m_bg);
      SerializeLinkage(s, m_wave_obj);
      SerializeLinkageContainer(s, m_team);
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_bg);
      ReconstructLinkage(m_wave_obj);
      ReconstructLinkageContainer(m_team);
    }

  public:
    Rule(int time) : m_time_limit(60*60*time), m_state(NORMAL)
    {
      SetCameraMovableArea(vector2(100,170), vector2(-100,-170));
      SetGlobalPlayerBound(box(vector4(483, 456, 100)));
      SetGlobalAccel(vector4(0, -0.012f, 0));

      m_wave_obj = new Wave1();

      GetMusic("vs.ogg")->play();
      m_bg = new Background();
    }


    void updateServerInfomation()
    {
      if(GetGame()->getPlayerCount()>0) {
        char buf[128];
        sprintf(buf, "deathmatch+%s+", GetLevelString().c_str());
        UpdateServerInfomation(buf);
      }
    }

    void join(pinfo_ptr pi)
    {
      updateServerInfomation();
      team_ptr t = CreateTeam();
      t->join(pi->getSessionID());
      m_team.push_back(t);
      if(m_state!=GAMESET) {
        pi->setRespawnTime(60*10);
      }
    }

    void leave(pinfo_ptr pi)
    {
      updateServerInfomation();
      if(team_ptr t = ITeam::getTeamBySID(pi->getSessionID())) {
        m_team.erase(std::find(m_team.begin(), m_team.end(), t));
      }
    }


    void progressNormal(int f)
    {
      if(f>0 && f%3600==0) { // 1分毎に破片倍率上昇 
        SetGlobalFractionBoost(GetGlobalFractionBoost()+0.1f);
      }
      for(size_t i=0; i<GetPlayerCount(); ++i) {
        pinfo_ptr pi = GetPlayerInfo(i);
        if(!pi->getPlayer() && pi->getRespawnTime()==0) {
          pi->setRespawnTime(60*10);
        }
      }
      if(getFrame()==60*60*3+1) {
        g_round2 = true;
      }
      if(getFrame()>=m_time_limit) {
        m_state = GAMESET;
        setFrame(0);
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
        DestroyAllEnemy(getTSM());
        SendKillMessage(0, m_wave_obj);
      }

      if(f==200) {
        std::sort(m_team.begin(), m_team.end(), order_by_pkill());
        string m;
        char buf[128];
        m+="                result\n\n";
        for(int i=0; i<m_team.size(); ++i) {
          ITeam::IPlayerRecord& r = m_team[i]->getRecord(0);
          sprintf(buf, "%10s : %d kill, %d death\n\n", r.getName().c_str(), r.getPlayerKill(), r.getDeath());
          m+=buf;
        }
        new Black();
        new Result(_L(m));
      }

      if(f==700) {
        ReturnToTitle();
      }
    }

    void progress(int f)
    {
      SweepDeadObject(m_wave_obj);
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

        ScreenMatrix sm;
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        char buf[128];
        int time_left = std::max<int>(m_time_limit-getFrame(), 0);
        sprintf(buf, "time %d:%02d", time_left/3600, time_left/60%60);
        DrawText(_L(buf), sgui::Point(290,20));
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
      }
    }
  };

} // namespace deathmatch 
} // namespace rule 
} // namespace exception 
#endif
