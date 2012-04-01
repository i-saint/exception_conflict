#ifndef rule_horde_h
#define rule_horde_h

namespace exception {
namespace rule {
namespace horde {

  int getInitialWave();

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

        tex[4] = vector2(0.0f, 0.0f);
        tex[5] = vector2(0.0f, 0.0f);
        tex[6] = vector2(0.0f, 0.0f);
        tex[7] = vector2(0.0f, 0.0f);
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
          pos.z-=1800.0f;
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
      m_cam.setPosition(vector4(100, -50, 400));
      m_cam.setTarget(vector4(0, 0, 0));
      m_cam.setFovy(60.0f);
      m_cam.setZFar(10000.0f);

      m_fog.setColor(round2() ? vector4(0.1f, 0.0f, 0.0f) : vector4(0.0f, 0.0f, 0.0f));
      m_fog.setNear(0.0f);
      m_fog.setFar(1500.0f);

      m_bgmat.setShininess(30.0f);

      if(round2()) {
        m_basecolor = vector4(2.0f, 1.0f, 1.0f);
        m_fog.setColor(vector4(0.1f, 0.0f, 0.0f));
        m_bgmat.setDiffuse(vector4(0.4f, 0.2f, 0.2f));
      }
      else {
        m_basecolor = vector4(1.0f, 1.0f, 2.0f);
        m_fog.setColor(vector4(0.0f, 0.0f, 0.0f));
        m_bgmat.setDiffuse(vector4(0.2f, 0.2f, 0.6f));
      }

      for(size_t i=0; i<400; ++i) {
        float d = 200.0f+100.0f*GenRand();
        vector4 dir = vector4(GenRand2(), GenRand2(), 0).normal();

        Block *b = new Block();
        b->setPosition(dir*d + vector4(0, 0, -1800.0f*GenRand()+400.0f));
        b->setDirection(-dir);
        b->setVel(vector4(0, 0, 4.0f));
        m_blocks.push_back(b);
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
        m_hline->attach(GetVertexShader("horde.vsh"));
        m_hline->attach(GetFragmentShader("horde.fsh"));
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

      if(m_blocks.back()->getPosition().z>-1000.0f) {
        m_basecolor-=(m_basecolor-vector4(2,2,2))*0.01f;
        m_fog.setColor(m_fog.getColor()-(m_fog.getColor()-vector4(1.0f, 1.0f, 1.0f))*0.005f);
        m_bgmat.setDiffuse(m_bgmat.getDiffuse()-(m_bgmat.getDiffuse()-vector4(1.0f, 1.0f, 1.0f))*0.005f);
      }
      else if(round2()) {
        m_basecolor-=(m_basecolor-vector4(2,1,1))*0.01f;
        m_fog.setColor(m_fog.getColor()-(m_fog.getColor()-vector4(0.1f, 0.0f, 0.0f))*0.01f);
        m_bgmat.setDiffuse(m_bgmat.getDiffuse()-(m_bgmat.getDiffuse()-vector4(0.4f, 0.2f, 0.2f))*0.01f);
      }

      const vector4& color = m_fog.getColor();
      sgui::View::instance()->setBackgroundColor(sgui::Color(color.x, color.y, color.z));

      m_cam.setPosition(vector4(100, -50, 400) + GetCamera().getTarget()*0.33f);
      m_cam.setTarget(vector4(0, 0, 0) + GetCamera().getTarget()*0.33f);

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
      SetCameraMovableArea(vector2(50,100), vector2(-50,-100));
      SetGlobalPlayerBound(box(vector4(432, 387, 100)));
    }

    void progress(int frame)
    {
      if(round2() && getInitialWave()!=15) {
        if(frame==0) {
          IMusic::FadeOut(3000);
        }
        if(frame==200) {
          GetMusic("horde2.ogg")->play();
        }
      }


      if(frame>0 && frame<400 && frame%70==0) {
        vector4 pos[3] = {
          vector4(700, 100, 0),
          vector4(700,   0, 0),
          vector4(700,-100, 0),
        };
        float speed[3] = {
          1.5f,
          1.6f,
          1.5f
        };
        for(int i=0; i<3; ++i) {
          putFighter(pos[i], vector4(-1,0,0), speed[i]);
        }
      }

      if(frame>400 && frame<800 && frame%65==0) {
        vector4 pos[4] = {
          vector4( 150, 700, 0),
          vector4(  50, 700, 0),
          vector4( -50, 700, 0),
          vector4(-150, 700, 0),

        };
        float speed[4] = {
          1.7f,
          1.8f,
          1.8f,
          1.7f,
        };
        for(int i=0; i<4; ++i) {
          putFighter(pos[i], vector4(0,-1,0), speed[i]);
        }
      }

      if(frame>800 && frame<1200 && frame%60==0) {
        vector4 pos[5] = {
          vector4(-700, 200, 0),
          vector4(-700, 100, 0),
          vector4(-700,   0, 0),
          vector4(-700,-100, 0),
          vector4(-700,-200, 0),
        };
        float speed[5] = {
          1.9f,
          2.0f,
          2.1f,
          2.0f,
          1.9f,
        };
        for(int i=0; i<5; ++i) {
          putFighter(pos[i], vector4(1,0,0), speed[i]);
        }
      }

      if(frame>1200 && frame<1600 && frame%55==0) {
        vector4 pos[6] = {
          vector4( 250,-700, 0),
          vector4( 150,-700, 0),
          vector4(  50,-700, 0),
          vector4( -50,-700, 0),
          vector4(-150,-700, 0),
          vector4(-250,-700, 0),
        };
        float speed[6] = {
          2.1f,
          2.2f,
          2.3f,
          2.3f,
          2.2f,
          2.1f,
        };
        for(int i=0; i<6; ++i) {
          putFighter(pos[i], vector4(0,1,0), speed[i]);
        }
      }

      if(frame>=1700 && frame<=2500 && frame%50==0) {
        int n = (frame-1700)/50;
        vector4 d = matrix44().rotateZ(10*n)*vector4(-1,0,0);
        vector4 h = matrix44().rotateZ(90)*d;
        vector4 pos[7] = {
          d*-700 + h* 150,
          d*-700,
          d*-700 + h*-150,
          d* 700 + h* 225,
          d* 700 + h*  75,
          d* 700 + h* -75,
          d* 700 + h*-225,
        };
        vector4 dir[7] = {d, d, d, -d,-d,-d,-d,};
        for(int i=0; i<7; ++i) {
          putFighter(pos[i], dir[i], 2.5f);
        }
      }

      if(frame>2500) {
        SendKillMessage(0, this);
      }
    }
  };


  class Wave2 : public IWave
  {
  typedef IWave Super;
  public:
    Wave2(Deserializer& s) : Super(s) {}

  public:
    Wave2()
    {
      SetCameraMovableArea(vector2(50,100), vector2(-50,-100));
      SetGlobalPlayerBound(box(vector4(432, 387, 100)));
    }

    void progress(int frame)
    {
      if(frame%35==0) {
        putMediumBlock(vector4(0, 500, 0)+vector4(brand()*280.0f, brand()*50.0f, 0));
      }
      if(frame%100==0) {
        putLargeBlock(vector4(0, 500, 0)+vector4(brand()*180.0f, brand()*50.0f, 0));
      }

      if(frame>=300 && frame<650 && frame%35==0) {
        int n = (frame-300)/35;
        vector4 pos[2] = {vector4(600, -400+80*n, 0), vector4(-650, -400+80*n, 0)};
        vector4 dir[2] = {vector4(-1,0,0), vector4(1,0,0)};
        for(int i=0; i<2; ++i) {
          putBurstShell(pos[i], dir[i]);
        }
      }

      if(frame>=700 && frame<1000 && frame%30==0) {
        int n = (frame-700)/30;
        vector4 pos[2] = {vector4(610, -450+80*n, 0), vector4(-610, -400+80*n, 0)};
        vector4 dir[2] = {vector4(-1,0,0), vector4(1,0,0)};
        for(int i=0; i<2; ++i) {
          if(n==5) {
            putGravityShell(pos[i], dir[i]);
          }
          else {
            putBurstShell(pos[i], dir[i]);
          }
        }
      }

      if(frame>=1050 && frame<1300 && frame%25==0) {
        int n = (frame-1050)/25;
        vector4 pos[2] = {vector4(570, -400+80*n, 0), vector4(-570, -400+80*n, 0)};
        vector4 dir[2] = {vector4(-1,0,0), vector4(1,0,0)};
        for(int i=0; i<2; ++i) {
          if(n==3 || n==6) {
            putGravityShell(pos[i], dir[i]);
          }
          else {
            putBurstShell(pos[i], dir[i]);
          }
        }
      }

      if(frame>=1500 && frame<1700 && frame%20==0) {
        int n = (frame-1500)/20;
        vector4 pos[2] = {vector4(650, 400-80*n, 0), vector4(-650, 400-80*n, 0)};
        vector4 dir[2] = {vector4(-1,0,0), vector4(1,0,0)};
        for(int i=0; i<2; ++i) {
          if(n==3 || n==6) {
            putGravityShell(pos[i], dir[i]);
          }
          else {
            putBurstShell(pos[i], dir[i]);
          }
        }
      }
      if(frame>=1700 && frame<=1820 && frame%20==0) {
        int n = (frame-1700)/20;
        vector4 pos[2] = {vector4(-360+80*n, -650, 0), vector4(360-80*n, -650, 0)};
        vector4 dir[2] = {vector4(0,1,0), vector4(0,1,0)};
        for(int i=0; i<2; ++i) {
          if(n==2) {
            putGravityShell(pos[i], dir[i]);
          }
          else {
            putBurstShell(pos[i], dir[i]);
          }
        }
      }

      if(frame>2000) {
        SendKillMessage(0, this);
      }
    }
  };


  class Wave3 : public IWave
  {
  typedef IWave Super;
  public:
    Wave3(Deserializer& s) : Super(s) {}

  public:
    Wave3()
    {
      SetCameraMovableArea(vector2(50,100), vector2(-50,-100));
      SetGlobalPlayerBound(box(vector4(432, 387, 100)));
    }

    void progress(int frame)
    {
      if(frame%30==0) {
        putMediumBlock(vector4(brand()*280.0f, 500+brand()*50.0f, 0));
      }
      if(frame%110==0) {
        if(frame/90%4==0) {
          putLargeBlockGround(vector4(brand()*200.0f, 500+brand()*50.0f, 0));
        }
        else {
          putLargeBlock(vector4(brand()*200.0f, 500+brand()*50.0f, 0));
        }
      }

      if(frame>=300 && frame<500 && frame%40==0) {
        int n = (frame-300)/40;
        vector4 dir[2] = {vector4(1,1,0).normal(), vector4(-1,1,0).normal()};
        vector4 pos[2] = {
          vector4(-100, -250, 0) + vector4(-1, 0.7f, 0).normal()*100*n - dir[0]*300.0f,
          vector4( 100, -250, 0) + vector4( 1, 0.7f, 0).normal()*100*n - dir[1]*300.0f,
        };
        for(int i=0; i<2; ++i) {
          putLaserEgg(pos[i], dir[i]);
        }
      }

      if(frame>=700 && frame<1000 && frame%40==0) {
        int n = (frame-700)/40;
        vector4 dir[2] = {vector4(1,0,0), vector4(-1,0,0)};
        vector4 pos[2] = {
          vector4(-330, 300, 0) + vector4(0,-100,0)*n - dir[0]*300.0f,
          vector4( 330, 300, 0) + vector4(0,-100,0)*n - dir[1]*300.0f,
        };
        for(int i=0; i<2; ++i) {
          putLaserEgg(pos[i], dir[i]);
        }
      }

      if(frame>=1200 && frame<1400 && frame%40==0) {
        int n = (frame-1200)/40;
        vector4 dir[2] = {vector4(1,-1,0).normal(), vector4(-1,-1,0).normal()};
        vector4 pos[2] = {
          vector4(-100, 250, 0) + vector4(-1,-0.7f, 0).normal()*100*n - dir[0]*300.0f,
          vector4( 100, 250, 0) + vector4( 1,-0.7f, 0).normal()*100*n - dir[1]*300.0f,
        };
        for(int i=0; i<2; ++i) {
          putLaserEgg(pos[i], dir[i]);
        }
      }

      if(frame>=1600 && frame<=1820 && frame%20==0) {
        int n = (frame-1600)/20;
        vector4 dir = matrix44().rotateZ(30*(n+1))*vector4(0, 1, 0);
        putLaserEgg(dir*650, -dir);
      }

      if(frame>2200) {
        SendKillMessage(0, this);
      }
    }
  };


  class Wave4 : public IWave
  {
  typedef IWave Super;
  public:
    Wave4(Deserializer& s) : Super(s) {}

  public:
    Wave4()
    {
      SetCameraMovableArea(vector2(50,100), vector2(-50,-100));
      SetGlobalPlayerBound(box(vector4(432, 387, 100)));
    }

    void progress(int frame)
    {
      if(frame%50==0) {
        putMediumBlock(vector4(brand()*280.0f, 500+brand()*50.0f, 0));
      }
      if(frame%100==0) {
        if(frame/100%2==0) {
          putLargeBlockGround(vector4(brand()*200.0f, 500+brand()*50.0f, 0));
        }
        else {
          putLargeBlock(vector4(brand()*200.0f, 500+brand()*50.0f, 0));
        }
      }

      if(frame== 200 || frame== 600 ||
         frame== 920 || frame==1240 ||
         frame==1520 || frame==1760) {
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
      if(frame== 400 || frame== 800 ||
         frame==1080 || frame==1400 ||
         frame==1640 || frame==1880) {
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

      if(frame>2200) {
        SendKillMessage(0, this);
      }
    }
  };


  class Wave5 : public IWave
  {
  typedef IWave Super;
  public:
    Wave5(Deserializer& s) : Super(s) {}

  public:
    Wave5()
    {
      SetCameraMovableArea(vector2(50,100), vector2(-50,-100));
      SetGlobalPlayerBound(box(vector4(432, 387, 100)));
    }

    void progress(int frame)
    {
      if(frame%110==0) {
        putMediumBlock(vector4(brand()*220.0f, 500+brand()*50.0f, 0));
      }
      if(frame%150==0) {
        if(frame/150%2==0) {
          putLargeBlockGround(vector4(brand()*150.0f, 500+brand()*50.0f, 0));
        }
        else {
          putLargeBlock(vector4(brand()*150.0f, 500+brand()*50.0f, 0));
        }
      }

      if(frame>=180 && frame<2800 && frame%70==0) {
        vector4 pos[11] = {
          vector4(600, 350, 0),
          vector4(600, 280, 0),
          vector4(600, 210, 0),
          vector4(600, 140, 0),
          vector4(600,  70, 0),
          vector4(600,   0, 0),
          vector4(600, -70, 0),
          vector4(600,-140, 0),
          vector4(600,-210, 0),
          vector4(600,-280, 0),
          vector4(600,-350, 0),
        };
        float speed[11] = {
          5.5f,
          4.5f,
          3.8f,
          3.2f,
          2.9f,
          2.7f,
          2.9f,
          3.2f,
          3.8f,
          4.5f,
          5.5f,
        };
        for(int i=0; i<11; ++i) {
          putFighter(pos[i], vector4(-1, 0, 0), speed[i]);
        }
      }

      if(frame==500) {
        for(int i=0; i<5; ++i) {
          putBurstShell(vector4(-600, 300-150*i, 0), vector4(1,0,0));
        }
      }
      if(frame==740) {
        for(int i=0; i<7; ++i) {
          putBurstShell(vector4(-660, 360-120*i, 0), vector4(1,0,0));
        }
      }

      if(frame>=900 && frame<=1020 && frame%20==0) {
        int n = (frame-900)/20;
        vector4 dir = matrix44().rotateZ(30*n)*vector4(0, 1, 0);
        putLaserEgg(dir*600-vector4(150, 0, 0), -dir);
      }
      if(frame>=1400 && frame<=1520 && frame%20==0) {
        int n = (frame-1400)/20;
        vector4 dir = matrix44().rotateZ(30*n)*vector4(0, 1, 0);
        putLaserEgg(dir*630-vector4(150, 0, 0), -dir);
      }

      if(frame>=2000 && frame<2300 && frame%120==0) {
        int n = (frame-2000)/120;
        for(int i=0; i<=12; ++i) {
          putZab(vector4(-200-60*n, 360-60*i, 0));
        }
      }

      if(frame>2600) {
        SendKillMessage(0, this);
      }
    }
  };


  class Wave6 : public IWave
  {
  typedef IWave Super;
  public:
    Wave6(Deserializer& s) : Super(s) {}

  public:
    Wave6()
    {
      SetCameraMovableArea(vector2(50,170), vector2(-50,-170));
      SetGlobalPlayerBound(box(vector4(432, 456, 100)));
    }

    void progress(int frame)
    {
      if(frame%40==0) {
        putMediumBlock(vector4(brand()*350.0f, 600+brand()*50.0f, 0));
      }

      if(frame>=180 && frame<=2800 && frame%180==0) {
        putSpinningGroundWithMineHatch(
          vector4(650, brand()*280.0f-50.0f, 0),
          vector4(-1.0f, 0.4f*brand(), 0)*1.3f,
          brand()*0.9f);
      }
      if(frame>=180 && frame<=2800 && frame%180==90) {
        putSpinningGroundWithMineHatch(
          vector4(-650, brand()*280.0f-50.0f, 0),
          vector4( 1.0f, 0.4f*brand(), 0)*1.3f,
          brand()*0.9f);
      }

      if(frame>3000) {
        SendKillMessage(0, this);
      }
    }
  };


  class Wave7 : public IWave
  {
  typedef IWave Super;
  public:
    Wave7(Deserializer& s) : Super(s) {}

  public:
    Wave7()
    {
      SetCameraMovableArea(vector2(50,170), vector2(-50,-170));
      SetGlobalPlayerBound(box(vector4(432, 456, 100)));
    }

    void progress(int frame)
    {
      if(frame%40==0) {
        putMediumBlock(vector4(brand()*350.0f, 600+brand()*50.0f, 0));
      }

      if(frame>0 && frame<=2800 && frame%350==0) {
        putSpinningGroundWithFighterHatch(
          vector4(650, brand()*250.0f-50.0f, 0),
          vector4(-1.0f, 0.4f*brand(), 0)*1.2f,
          brand()*0.6f);
      }
      if(frame>0 && frame<=2800 && frame%350==175) {
        putSpinningGroundWithFighterHatch(
          vector4(-650, brand()*250.0f-50.0f, 0),
          vector4( 1.0f, 0.4f*brand(), 0)*1.2f,
          brand()*0.6f);
      }

      if(frame>3400) {
        SendKillMessage(0, this);
      }
    }
  };


  class Wave8 : public IWave
  {
  typedef IWave Super;
  public:
    Wave8(Deserializer& s) : Super(s) {}

  public:
    Wave8()
    {
      SetCameraMovableArea(vector2(100,170), vector2(-100,-170));
      SetGlobalPlayerBound(box(vector4(483, 456, 100)));
    }

    void progress(int frame)
    {
      if(frame%40==0) {
        putMediumBlock(vector4(brand()*280.0f, 600+brand()*50.0f, 0));
      }

      if(frame==200) {
        vector4 pos[2] = {vector4(390, -700, 0), vector4(-390, -700, 0)};
        for(int i=0; i<2; ++i) {
          putLargeCarrier(pos[i], vector4(0,1,0));
        }
      }

      if(frame==450) {
        vector4 pos[2] = {vector4(170, -900, 0), vector4(-170, -900, 0)};
        for(int i=0; i<2; ++i) {
          putLargeCarrier(pos[i], vector4(0,1,0));
        }
      }

      if(frame==1200) {
        vector4 pos[2] = {vector4(350, -800, 0), vector4(-350, -800, 0)};
        for(int i=0; i<2; ++i) {
          putLargeCarrier(pos[i], vector4(0,1,0));
        }
      }

      if(frame==1800) {
        vector4 pos[3] = {vector4(390, -1000, 0), vector4(0, -900, 0), vector4(-390, -1000, 0)};
        for(int i=0; i<3; ++i) {
          putLargeCarrier(pos[i], vector4(0,1,0));
        }
      }

      if(frame>3000) {
        SendKillMessage(0, this);
      }
    }
  };


  class Wave9 : public IWave
  {
  typedef IWave Super;
  public:
    Wave9(Deserializer& s) : Super(s) {}

  public:
    Wave9()
    {
      SetCameraMovableArea(vector2(100,170), vector2(-100,-170));
      SetGlobalPlayerBound(box(vector4(483, 456, 100)));
    }

    void progress(int frame)
    {
      if(frame%40==0) {
        putMediumBlock(vector4(brand()*280.0f, 600+brand()*50.0f, 0));
      }

      int kf[17] = {
         100,  100,
         250,  250,
         400,
         550, 550,
         700, 700,
         850, 850,
         1000,
         1250, 1250, 1250, 1250, 1250,
      };
      for(int i=0; i<17; ++i) {
        if(frame==kf[i]) {
          vector4 pos[17] = {
            vector4(360, -700, 0), vector4(-360, -700, 0),
            vector4(180, -700, 0), vector4(-180, -700, 0),
            vector4(0, -700, 0),
            vector4(180, -750, 0), vector4(-180, -750, 0),
            vector4(360, -750, 0), vector4(-360, -750, 0),
            vector4(180, -750, 0), vector4(-180, -750, 0),
            vector4(0, -750, 0),

            vector4(360, -700, 0), vector4(-360, -700, 0),
            vector4(180, -750, 0), vector4(-180, -750, 0),
            vector4(0, -800, 0),
          };
          putHeavyFighter(pos[i], vector4(0,1,0));
        }
      }

      if(frame>2100) {
        SendKillMessage(0, this);
      }
    }
  };


  class Wave10 : public IWave
  {
  typedef IWave Super;
  public:
    Wave10(Deserializer& s) : Super(s) {}

  public:
    Wave10()
    {
      SetCameraMovableArea(vector2(100,170), vector2(-100,-170));
      SetGlobalPlayerBound(box(vector4(483, 456, 100)));
    }

    void progress(int frame)
    {
      if(frame%30==0) {
        putMediumBlock(vector4(brand()*340.0f, 600+brand()*50.0f, 0));
      }
      if(frame%100==0) {
        if(frame/100%3==1 && frame<2100) {
          putLargeBlockGround(vector4(brand()*250.0f, 600+brand()*50.0f, 0));
        }
        else {
          putLargeBlock(vector4(brand()*280.0f, 600+brand()*50.0f, 0));
        }
      }

      int kf[12] = {
         100,  300,  500,
         900, 1050, 1200,
        1400, 1520, 1640,
        2000, 2000,
        2050,
      };
      for(int i=0; i<12; ++i) {
        if(frame==kf[i]) {
          vector4 pos[12] = {
            vector4(280, -800, 0), vector4(-800, -280, 0), vector4(-280, 800, 0),
            vector4(280, -800, 0), vector4(-800, -280, 0), vector4(-280, 800, 0),
            vector4(280, -800, 0), vector4(-800, -280, 0), vector4(-280, 800, 0),
            vector4(300,-1100, 0), vector4(-300,-1100, 0),
            vector4(  0,-1250, 0),
          };
          vector4 dir[12] = {
            vector4(0, 1, 0), vector4(1, 0, 0), vector4(0, -1, 0),
            vector4(0, 1, 0), vector4(1, 0, 0), vector4(0, -1, 0),
            vector4(0, 1, 0), vector4(1, 0, 0), vector4(0, -1, 0),
            vector4(0, 1, 0), vector4(0, 1, 0),
            vector4(0, 1, 0),
          };
          putWeakIterator(pos[i], dir[i]);
        }
      }

      if(frame>2600) {
        SendKillMessage(0, this);
      }
    }
  };


  class Wave11 : public IWave
  {
  typedef IWave Super;
  public:
    Wave11(Deserializer& s) : Super(s) {}

  public:
    Wave11()
    {
      SetCameraMovableArea(vector2(100,170), vector2(-100,-170));
      SetGlobalPlayerBound(box(vector4(483, 456, 100)));
    }

    void progress(int frame)
    {
      if(frame%40==0) {
        putMediumBlock(vector4(brand()*350.0f, 600+brand()*50.0f, 0));
      }

      if(frame<2500 && frame%175==0) {
        int n = frame/175;
        vector4 pos[2] = {vector4(140,0,0), vector4(-140,0,0)};
        putSpinningGroundWithMineHatch(
          pos[n%2]+vector4(brand()*80.0f, 650.0f+brand()*30.0f, 0),
          vector4(0.05f*brand(), -1.0f, 0)*1.3f,
          brand()*0.9f);
      }

      if(frame>=0 && frame<=800 && frame%80==0) {
        vector4 pos[2] = { vector4( 100,-700, 0), vector4(-100,-700, 0), };
        for(int i=0; i<2; ++i) {
          putFighter(pos[i], vector4(0,1,0), 1.25f);
        }
      }
      if(frame>=600 && frame<=1400 && frame%80==0) {
        vector4 pos[2] = { vector4( 200,-700, 0), vector4(-200,-700, 0), };
        for(int i=0; i<2; ++i) {
          putFighter(pos[i], vector4(0,1,0), 1.25f);
        }
      }

      if(frame>=600 && frame<=740 && frame%20==0) {
        int n = (frame-600)/20;
        vector4 dir[2] = {vector4(-1,0,0), vector4(1,0,0)};
        vector4 pos[2] = {
          vector4( 380, -390+130*n, 0) - dir[0]*300.0f,
          vector4(-380, -390+130*n, 0) - dir[1]*300.0f,
        };
        for(int i=0; i<2; ++i) {
          putBurstShell(pos[i], dir[i]);
        }
      }
      if(frame>=860 && frame<=1000 && frame%20==0) {
        int n = (frame-860)/20;
        vector4 dir[2] = {vector4(-1,0,0), vector4(1,0,0)};
        vector4 pos[2] = {
          vector4( 380, -390+130*n, 0) - dir[0]*300.0f,
          vector4(-380, -390+130*n, 0) - dir[1]*300.0f,
        };
        for(int i=0; i<2; ++i) {
          putBurstShell(pos[i], dir[i]);
        }
      }

      if(frame>=1200 && frame<=1410 && frame%30==0) {
        int n = (frame-1200)/30;
        vector4 dir[2] = {vector4(-1,0,0), vector4(1,0,0)};
        vector4 pos[2] = {
          vector4( 380, -390+130*n, 0) - dir[0]*300.0f,
          vector4(-380, -390+130*n, 0) - dir[1]*300.0f,
        };
        for(int i=0; i<2; ++i) {
          putLaserEgg(pos[i], dir[i]);
        }
      }
      if(frame>=1500 && frame<=1710 && frame%30==0) {
        int n = (frame-1500)/30;
        vector4 dir[2] = {vector4(-1,0,0), vector4(1,0,0)};
        vector4 pos[2] = {
          vector4( 380, -390+130*n, 0) - dir[0]*300.0f,
          vector4(-380, -390+130*n, 0) - dir[1]*300.0f,
        };
        for(int i=0; i<2; ++i) {
          putLaserEgg(pos[i], dir[i]);
        }
      }

      if(frame>2200) {
        SendKillMessage(0, this);
      }
    }
  };


  class Wave12 : public IWave
  {
  typedef IWave Super;
  public:
    Wave12(Deserializer& s) : Super(s) {}

  public:
    Wave12()
    {
      SetCameraMovableArea(vector2(100,170), vector2(-100,-170));
      SetGlobalPlayerBound(box(vector4(483, 456, 100)));
    }

    void progress(int frame)
    {
      if(frame%40==0) {
        putMediumBlock(vector4(brand()*350.0f, 600+brand()*50.0f, 0));
      }

      if(frame<2500 && frame%190==0) {
        int n = frame/190;
        vector4 pos[2] = {vector4(150,0,0), vector4(-150,0,0)};
        if(n%3==1) {
          putSpinningGroundWithFighterHatch(
            pos[n%2]*0.5f+vector4(brand()*50.0f, 690.0f+brand()*10.0f, 0),
            vector4(0.03f*brand(), -1.0f, 0)*1.3f,
            brand()*0.6f);
        }
        else {
          putSpinningGroundWithMineHatch(
            pos[n%2]+vector4(brand()*120.0f, 690.0f+brand()*10.0f, 0),
            vector4(0.05f*brand(), -1.0f, 0)*1.3f,
            brand()*0.9f);
        }
      }

      {
        int kf[4] = {100, 400, 700, 1000};
        for(int i=0; i<4; ++i) {
          if(frame==kf[i]) {
            vector4 pos[4][2] = {
              {vector4(380,-700, 0), vector4(-380,-700, 0)},
              {vector4(380,-750, 0), vector4(-380,-750, 0)},
              {vector4(380,-800, 0), vector4(-380,-800, 0)},
              {vector4(380,-850, 0), vector4(-380,-850, 0)},
            };
            for(int j=0; j<2; ++j) {
              putHeavyFighter(pos[i][j], vector4(0,1,0));
            }
          }
        }
      }
      {
        int kf[3] = {1500, 1800};
        for(int i=0; i<2; ++i) {
          if(frame==kf[i]) {
            vector4 pos[2][2] = {
              {vector4(380, -950, 0), vector4(-380, -950, 0)},
              {vector4(380,-1250, 0), vector4(-380,-1250, 0)},
            };
            for(int j=0; j<2; ++j) {
              putWeakIterator(pos[i][j], vector4(0,1,0));
            }
          }
        }
      }

      if(frame>2700) {
        SendKillMessage(0, this);
      }
    }
  };


  class Wave13 : public IWave
  {
  typedef IWave Super;
  public:
    Wave13(Deserializer& s) : Super(s) {}

  public:
    Wave13()
    {
      SetCameraMovableArea(vector2(100,170), vector2(-100,-170));
      SetGlobalPlayerBound(box(vector4(483, 456, 100)));
    }

    void progress(int frame)
    {
      if(frame%40==0) {
        putMediumBlock(vector4(brand()*350.0f, 600+brand()*50.0f, 0));
      }

      if(frame<2400 && frame%190==0) {
        int n = frame/190;
        vector4 pos[2] = {vector4(150,0,0), vector4(-150,0,0)};
        if(n%3==1) {
          putSpinningGroundWithFighterHatch(
            pos[n%2]*0.5f+vector4(brand()*50.0f, 690.0f+brand()*10.0f, 0),
            vector4(0.05f*brand(), -1.0f, 0)*1.3f,
            brand()*0.6f);
        }
        else {
          putSpinningGroundWithLaserHatch(
            pos[n%2]+vector4(brand()*120.0f, 690.0f+brand()*10.0f, 0),
            vector4(0.1f*brand(), -1.0f, 0)*1.3f,
            brand()*0.5f);
        }
      }

      if(frame>=300 && frame<=2300 && frame%300==0) {
        for(int i=0; i<6; ++i) {
          vector4 pos[2] = {vector4(390, -360, 0), vector4(-390, -360, 0)};
          for(int j=0; j<2; ++j) {
            putZab(pos[j]+vector4(0, 120*i, 0));
          }
        }
      }

      if(frame>2800) {
        SendKillMessage(0, this);
      }
    }
  };


  class Wave14 : public IWave
  {
  typedef IWave Super;
  public:
    Wave14(Deserializer& s) : Super(s) {}

  public:
    Wave14()
    {
      SetCameraMovableArea(vector2(100,170), vector2(-100,-170));
      SetGlobalPlayerBound(box(vector4(483, 456, 100)));
    }

    void progress(int frame)
    {
      if(frame%60==0) {
        putMediumBlock(vector4(brand()*300.0f, 600+brand()*50.0f, 0));
      }

      if(frame<2900 && frame%80==0) {
        putFighter(vector4(0, -700, 0), vector4(0,1,0), 1.25f);
      }
      if(frame<2900 && frame%350==100) {
        vector4 pos[2] = {vector4(120, 700, 0), vector4(-120, 700, 0)};
        for(int i=0; i<2; ++i) {
          putHeavyFighter(pos[i], vector4(0,-1,0));
        }
      }
      if(frame<2500 && frame%700==300) {
        vector4 pos[2] = {vector4(390,-850, 0), vector4(-390,-850, 0)};
        for(int i=0; i<2; ++i) {
          putLargeCarrier(pos[i], vector4(0,1,0));
        }
      }

      if(frame>3600) {
        SendKillMessage(0, this);
      }
    }
  };


  class Wave15 : public IWave
  {
  typedef IWave Super;
  public:
    Wave15(Deserializer& s) : Super(s) {}

  public:
    Wave15()
    {
      SetCameraMovableArea(vector2(100,170), vector2(-100,-170));
      SetGlobalPlayerBound(box(vector4(483, 456, 100)));
    }

    void progress(int frame)
    {
      if(frame%30==0) {
        putMediumBlock(vector4(brand()*360.0f, 600+brand()*50.0f, 0));
      }
      if(frame%80==0) {
        if(frame/80%4==3) {
          putLargeBlockGround(vector4(brand()*300.0f, 600+brand()*50.0f, 0));
        }
        else {
          putLargeBlock(vector4(brand()*300.0f, 600+brand()*50.0f, 0));
        }
      }

      if(frame>=600 && frame<=2000 && frame%300==0) {
        for(int i=0; i<6; ++i) {
          vector4 pos[2] = {vector4(380, -390, 0), vector4(-380, -390, 0)};
          for(int j=0; j<2; ++j) {
            putZab(pos[j]+vector4(0, 130*i, 0));
          }
        }
      }

      if(frame>=600 && frame<=2000 && frame%300==150) {
        for(int i=0; i<6; ++i) {
          putZab(vector4(390, -380, 0)+vector4(-130*i, 0, 0));
        }
      }

      {
        int kf[10] = {
           200, 200,
           400, 400,
           600,
          1000,1000,
          1150,1150,
          1300,
        };
        for(int i=0; i<10; ++i) {
          if(frame==kf[i]) {
            vector4 pos[5] = {
              vector4(420, -900, 0), vector4(-420, -900, 0),
              vector4(210,-1150, 0), vector4(-210,-1150, 0),
              vector4(  0,-1300, 0),
            };
            putWeakIterator(pos[i%5], vector4(0,1,0));
          }
        }
      }
      {
        int kf[6] = {
           1700, 1700,
           1790, 1790,
           1880, 1880,
        };
        for(int i=0; i<6; ++i) {
          if(frame==kf[i]) {
            vector4 pos[6] = {
              vector4(420,   0, 0), vector4(-420,   0, 0),
              vector4(300,-150, 0), vector4(-300,-150, 0),
              vector4(180,-300, 0), vector4(-180,-300, 0),
            };
            vector4 dir[6] = {
              vector4(0, -1, 0), vector4(0, -1, 0),
              vector4(-0.8f, -1, 0).normal(), vector4(0.8f, -1, 0).normal(),
              vector4(-1, -0.3f, 0).normal(), vector4(1, -0.3f, 0).normal(),
            };
            putWeakIterator(pos[i%6]-dir[i%6]*1000, dir[i%6]);
          }
        }
      }

      if(frame>2600) {
        SendKillMessage(0, this);
      }
    }
  };






  class FadeOut : public Optional
  {
  typedef Optional Super;
  private:
    vector4 m_color;
    float m_opa;

  public:
    FadeOut(Deserializer& s) : Super(s)
    {
      s >> m_color >> m_opa;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_color << m_opa;
    }

  public:
    FadeOut(const vector4& col) : m_color(col), m_opa(0.0f)
    {}

    float getDrawPriority() { return 30.0f; }

    void draw()
    {
      glColor4f(m_color.x, m_color.y, m_color.z, m_opa);
      DrawRect(vector2(-1,-1), vector2(641, 481));
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      m_opa = std::min<float>(m_color.w, m_opa+0.02f);
    }
  };

  class Result : public Optional
  {
  typedef Optional Super;
  private:
    wstring m_message;
    wstring m_display;
    vector4 m_color;

  public:
    Result(Deserializer& s) : Super(s)
    {
      s >> m_message >> m_display >> m_color;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_message << m_display << m_color;
    }

  public:
    Result(const wstring& m, const vector4& col) : m_message(m), m_color(col)
    {}

    float getDrawPriority() { return 30.0f; }

    void draw()
    {
      ScreenMatrix sm;
      glDisable(GL_LIGHTING);
      glDisable(GL_DEPTH_TEST);

      glColor4fv(m_color.v);
      DrawText(m_display, sgui::Point(220, 120));
      glColor4f(1,1,1,1);

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

  class WaveAnnounce : public Optional
  {
  typedef Optional Super;
  private:
    wstring m_message;
    wstring m_display;
    wstring m_next;
    vector2 m_pos2;
    float m_opa;

  public:
    WaveAnnounce(Deserializer& s) : Super(s)
    {
      s >> m_message >> m_display >> m_next >> m_pos2 >> m_opa;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_message << m_display << m_next << m_pos2 << m_opa;
    }

  public:
    WaveAnnounce() : m_opa(0.0f)
    {
      m_pos2 = vector2(10, 5);
    }

    float getDrawPriority() { return 30.0f; }

    void setNewWave(int wave)
    {
      char buf[128];
      sprintf(buf, "wave %d", wave);
      m_next = _L(buf);
    }

    void draw()
    {
      ScreenMatrix sm;
      glDisable(GL_LIGHTING);
      glDisable(GL_DEPTH_TEST);

      glColor4f(1,1,1,m_opa);
      DrawText(m_display, sgui::Point(m_pos2.x, m_pos2.y));
      glColor4f(1,1,1,1);

      glEnable(GL_LIGHTING);
      glEnable(GL_DEPTH_TEST);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      if(!m_next.empty()) {
        m_opa-=0.01f;
        if(m_opa<=0.0f) {
          m_display.clear();
          m_message = m_next;
          m_next.clear();
          m_opa = 1.0f;
        }
      }
      if(m_display.size()!=m_message.size()) {
        m_display.push_back(m_message[m_display.size()]);
      }
    }
  };

  class Rule : public RuleBase
  {
  typedef RuleBase Super;
  private:
    static Rule *s_inst;

    int m_state;
    int m_initial_wave;
    int m_wave;
    Background *m_bg;
    IWave *m_wave_obj;
    WaveAnnounce *m_wave_announce;
    team_ptr m_team;

    enum {
      NORMAL,
      GAMESET,
      GAMEOVER,
    };

  public:
    Rule(Deserializer& s) : Super(s)
    {
      s_inst = this;
      s >> m_state >> m_initial_wave >> m_wave;
      DeserializeLinkage(s, m_bg);
      DeserializeLinkage(s, m_wave_obj);
      DeserializeLinkage(s, m_wave_announce);
      DeserializeLinkage(s, m_team);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_state << m_initial_wave << m_wave;
      SerializeLinkage(s, m_bg);
      SerializeLinkage(s, m_wave_obj);
      SerializeLinkage(s, m_wave_announce);
      SerializeLinkage(s, m_team);
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_bg);
      ReconstructLinkage(m_wave_obj);
      ReconstructLinkage(m_wave_announce);
      ReconstructLinkage(m_team);
    }

  public:
    static Rule* instance() { return s_inst; }

    Rule(int wave=0) :
      m_state(NORMAL), m_initial_wave(wave), m_wave(wave),
      m_wave_obj(0), m_wave_announce(0), m_team(0)
    {
      s_inst = this;
      SetGlobalAccel(vector4(0, -0.012f, 0));
      m_team = CreateTeam();
      m_wave_announce = new WaveAnnounce();
      goNextWave();

      m_bg = new Background();
      if(round2()) {
        GetMusic("horde2.ogg")->play();
      }
      else {
        GetMusic("horde1.ogg")->play();
      }
    }

    ~Rule()
    {
      s_inst = 0;
    }

    float getDrawPriority() { return 30.0f; }
    int getInitialWave() { return m_initial_wave; }

    void updateServerInfomation()
    {
      if(GetGame()->getPlayerCount()>0) {
        char buf[128];
        sprintf(buf, "horde+%s+wave%d+", GetLevelString().c_str(), m_wave);
        UpdateServerInfomation(buf);
      }
    }

    void join(pinfo_ptr pi)
    {
      updateServerInfomation();
      m_team->join(pi->getSessionID());
      if(m_state!=GAMEOVER) {
        pi->setRespawnTime(60*(10+5*GetPlayerCount()));
      }
    }

    void leave(pinfo_ptr pi)
    {
      updateServerInfomation();
      m_team->leave(pi->getSessionID());
    }

    void progressNormal(int f)
    {
      int alive = 0;
      for(size_t i=0; i<GetPlayerCount(); ++i) {
        pinfo_ptr pi = GetPlayerInfo(i);
        if(pi->getPlayer()) {
          ++alive;
        }
        else if(pi->getRespawnTime()==0) {
          pi->setRespawnTime(60*(10+5*GetPlayerCount()));
        }
      }
      if(alive==0) {
        for(size_t i=0; i<GetPlayerCount(); ++i) {
          GetPlayerInfo(i)->setRespawnTime(60*60*60);
        }
        m_state = GAMEOVER;
        setFrame(0);
      }

      if(!m_wave_obj) {
        if(m_wave==30) {
          m_state = GAMESET;
          setFrame(0);
        }
        else {
          goNextWave();
        }
      }
    }


    string getResultString()
    {
      string m;
      char buf[128];
      m_team->sortByFractionKill();
      m+="                result\n\n";
      for(size_t i=0; i<m_team->getRecordCount(); ++i) {
        ITeam::IPlayerRecord& r = m_team->getRecord(i);
        sprintf(buf, "%10s : %d destroy, %d death\n\n", r.getName().c_str(), r.getFractionKill(), r.getDeath());
        m+=buf;
      }
      return m;
    }

    void progressGameSet(int f)
    {
      if(f==300) {
        DestroyAllEnemy(getTSM());
        InvincibleAllPlayers(6000);
        m_bg->stop();
      }
      if(f==700) {
        new FadeOut(vector4(1,1,1));
      }
      if(f==850) {
        new Result(_L(getResultString()), vector4(0,0,0));
      }

      if(f==1300) {
        ReturnToTitle();
      }
    }

    void progressGameover(int f)
    {
      if(f==100) {
        new FadeOut(vector4(0,0,0, 0.8f));
      }
      if(f==130) {
        new Result(_L(getResultString()), vector4(1,1,1));
      }
      if(f==600) {
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
      else {
        progressGameover(f);
      }
    }

    void goNextWave()
    {
      ++m_wave;
      m_wave_announce->setNewWave(m_wave);
      g_rand.reset(new ist::Random(m_wave));
      updateServerInfomation();

      if(m_wave>15 && !g_round2) {
        g_round2 = true;
        SetGlobalFractionBoost(GetGlobalFractionBoost()+0.5f);
      }

      switch(m_wave%15) {
      case  1: m_wave_obj=new Wave1(); break;
      case  2: m_wave_obj=new Wave2(); break;
      case  3: m_wave_obj=new Wave3(); break;
      case  4: m_wave_obj=new Wave4(); break;
      case  5: m_wave_obj=new Wave5(); break;
      case  6: m_wave_obj=new Wave6(); break;
      case  7: m_wave_obj=new Wave7(); break;
      case  8: m_wave_obj=new Wave8(); break;
      case  9: m_wave_obj=new Wave9(); break;
      case 10: m_wave_obj=new Wave10();break;
      case 11: m_wave_obj=new Wave11();break;
      case 12: m_wave_obj=new Wave12();break;
      case 13: m_wave_obj=new Wave13();break;
      case 14: m_wave_obj=new Wave14();break;
      case  0: m_wave_obj=new Wave15();break;
      }
    }


    void draw()
    {
      if(m_state==NORMAL || m_state==GAMESET) {
        drawRespawnTime();
      }
    }
  };
  Rule* Rule::s_inst = 0;

  int getInitialWave() { return Rule::instance()->getInitialWave(); }

} // namespace horde 
} // namespace rule 
} // namespace exception 
#endif
