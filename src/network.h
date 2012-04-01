#ifndef network_h
#define network_h

#include <sstream>
#include <boost/lexical_cast.hpp>
#include "input.h"
#include "game.h"
#include "system.h"

namespace exception {

#ifdef EXCEPTION_ENABLE_NETPLAY 
  using boost::asio::ip::tcp;
  typedef shared_ptr<tcp::iostream> socket_ptr;
#endif // EXCEPTION_ENABLE_NETPLAY 

  size_t GetSessionID();
  bool ParseChatCommand(const wstring& s);


  class NetworkError : public std::runtime_error
  {
  typedef std::runtime_error Super;
  public:
    NetworkError(const string& message) : Super(message) {}
  };

  class ProtocolError : public NetworkError
  {
  typedef NetworkError Super;
  public:
    ProtocolError(const string& message) : Super(message) {}
  };


  struct NMessage
  {
    enum {
      UNKNOWN = 0,
      QUERY,
      RESPONSE,
      ENTRY,
      ACCEPT,
      REJECT,
      CLOSE,
      START,
      END,
      PAUSE,
      RESUME,
      STATE,
      JOIN,
      LEAVE,
      CSTAT,
      GSTAT,
      SSTAT,
      INPUT,
      TEXT,
      DELAY,
      GCONF,
      SYNC,
      RESYNC,
    };

    struct CStatField {
      size_t ping;
    };

    struct GStatField {
      size_t elapsed;
      size_t elapsed_par_frame;
      size_t fps;
      size_t delay;
      size_t obj;
      size_t players;
      size_t alives;
    };

    struct SStatField {
      char text[64];
    };

    struct StartField {
      int delay;
      int level;
      int stage;
      int seed;
      int horde_wave;
      int deathmatch_time;
      float teamfortress_life;
      float cboost;
      float fboost;
      float eboost;

      void fromGameOption(const GameOption& opt)
      {
        delay = opt.delay;
        level = opt.level;
        stage = opt.stage;
        seed = opt.seed;
        horde_wave = opt.horde_wave;
        deathmatch_time = opt.deathmatch_time;
        teamfortress_life = opt.teamfortress_life;
        cboost = opt.cboost;
        fboost = opt.fboost;
        eboost = opt.eboost;
      }

      void toGameOption(GameOption& opt) const
      {
        opt.delay = delay;
        opt.level = level;
        opt.stage = stage;
        opt.seed = seed;
        opt.horde_wave = horde_wave;
        opt.deathmatch_time = deathmatch_time;
        opt.teamfortress_life = teamfortress_life;
        opt.cboost = cboost;
        opt.fboost = fboost;
        opt.eboost = eboost;
      }
    };

    struct GConfField {
      int frame;
      int level;
      int stage;
      int delay;
      int horde_wave;
      int deathmatch_time;
      float teamfortress_life;
      float cboost;
      float fboost;
      float eboost;

      void initialize()
      {
        frame = 0;
        level = -1;
        stage = -1;
        horde_wave = -1;
        deathmatch_time = -1;
        teamfortress_life = -1.0f;
        cboost = -1.0f;
        fboost = -1.0f;
        eboost = -1.0f;
      }

      void fromGameOption(const GameOption& opt)
      {
        level = opt.level;
        stage = opt.stage;
        horde_wave = opt.horde_wave;
        deathmatch_time = opt.deathmatch_time;
        teamfortress_life = opt.teamfortress_life;
        cboost = opt.cboost;
        fboost = opt.fboost;
        eboost = opt.eboost;
      }

      void toGameOption(GameOption& opt) const
      {
        toGameOptionWithoutStaticParameter(opt);
        if(stage>=0)             { opt.stage=stage; }
        if(horde_wave>=0)        { opt.horde_wave=horde_wave; }
        if(deathmatch_time>=0)   { opt.deathmatch_time=deathmatch_time; }
        if(teamfortress_life>=0) { opt.teamfortress_life=teamfortress_life; }
      }

      void toGameOptionWithoutStaticParameter(GameOption& opt) const
      {
        if(level>=0)     { opt.level =level; }
        if(cboost>=0.0f) { opt.cboost=cboost; }
        if(fboost>=0.0f) { opt.fboost=fboost; }
        if(eboost>=0.1f) { opt.eboost=eboost; }
      }
    };

    struct InputField {
      int input;
      size_t elapsed;
      size_t frame;
    };

    struct EntryField {
      char name[16];
      float color[4];
      size_t frame;
    };

    struct TextField {
      char text[48];
      float color[4];
    };

    struct JoinField {
      size_t frame;
    };

    struct LeaveField {
      size_t frame;
    };

    struct PauseField {
      size_t frame;
    };

    struct ResumeField {
      size_t frame;
    };

    struct StateField {
      size_t size;
      size_t frame;
    };

    struct DelayField {
      size_t delay;
    };

    struct SyncField {
      short life[16];
      size_t frame;

      bool compare(SyncField& r) const
      {
        for(int i=0; i<16; ++i) {
          if(life[i]!=r.life[i]) {
            return false;
          }
        }
        return true;
      }
    };


    int type;
    size_t sid;
    union {
      CStatField cstat;
      GStatField gstat;
      SStatField sstat;
      StartField start;
      GConfField gconf;
      InputField input;
      EntryField entry;
      TextField reject;
      TextField response;
      TextField text;
      TextField query;
      JoinField join;
      LeaveField leave;
      PauseField pause;
      ResumeField resume;
      StateField state;
      DelayField delay;
      SyncField sync;
    };
    boost::shared_ptr<ist::bbuffer> statep;

    NMessage() {}
    NMessage(ist::bstream& b) { deserialize(b); }

    void serialize(ist::bstream& b) const
    {
      b << type;
      switch(type) {
      case INPUT:    b << sid << input.input << input.elapsed << input.frame; break;
      case CSTAT:    b << sid << cstat.ping; break;
      case GSTAT:    b << sid << gstat.elapsed << gstat.elapsed_par_frame << gstat.fps << gstat.delay
                       << gstat.obj << gstat.players << gstat.alives; break;
      case SSTAT:    b << sid << sstat.text; break;
      case TEXT:     b << sid << text.text << text.color; break;
      case PAUSE:    b << sid << pause.frame; break;
      case RESUME:   b << sid << resume.frame; break;
      case START:    b << sid << start.delay << start.level << start.stage << start.seed
                       << start.horde_wave << start.deathmatch_time << start.teamfortress_life
                       << start.cboost << start.fboost << start.eboost; break;
      case GCONF:   b << sid << gconf.frame << gconf.level << gconf.stage
                       << gconf.horde_wave << gconf.deathmatch_time << gconf.teamfortress_life
                       << gconf.cboost << gconf.fboost << gconf.eboost; break;
      case END:      b << sid; break;
      case CLOSE:    b << sid; break;
      case ACCEPT:   b << sid; break;
      case ENTRY:    b << sid << entry.name << entry.color << entry.frame; break;
      case REJECT:   b << sid << reject.text; break;
      case QUERY:    b << sid << query.text; break;
      case RESPONSE: b << sid << response.text; break;
      case JOIN:     b << sid << join.frame; break;
      case LEAVE:    b << sid << leave.frame; break;
      case DELAY:    b << sid << delay.delay; break;
      case STATE:    b << *statep << state.size << state.frame; break;
      case SYNC:     b << sid << sync.life << sync.frame; break;
      case RESYNC:   break;
      default: throw NetworkError("protocol error"); break;
      }
    }

    void deserialize(ist::bstream& b)
    {
      b >> type;
      switch(type) {
      case INPUT:    b >> sid >> input.input >> input.elapsed >> input.frame; break;
      case CSTAT:    b >> sid >> cstat.ping; break;
      case GSTAT:    b >> sid >> gstat.elapsed >> gstat.elapsed_par_frame >> gstat.fps >> gstat.delay
                       >> gstat.obj >> gstat.players >> gstat.alives; break;
      case SSTAT:    b >> sid >> sstat.text; break;
      case TEXT:     b >> sid >> text.text >> text.color; break;
      case PAUSE:    b >> sid >> pause.frame; break;
      case RESUME:   b >> sid >> resume.frame; break;
      case START:    b >> sid >> start.delay >> start.level >> start.stage >> start.seed
                       >> start.horde_wave >> start.deathmatch_time >> start.teamfortress_life
                       >> start.cboost >> start.fboost >> start.eboost; break;
      case GCONF:   b >> sid >> gconf.frame >> gconf.level >> gconf.stage
                       >> gconf.horde_wave >> gconf.deathmatch_time >> gconf.teamfortress_life
                       >> gconf.cboost >> gconf.fboost >> gconf.eboost; break;
      case END:      b >> sid; break;
      case CLOSE:    b >> sid; break;
      case ACCEPT:   b >> sid; break;
      case ENTRY:    b >> sid >> entry.name >> entry.color >> entry.frame; break;
      case REJECT:   b >> sid >> reject.text; break;
      case QUERY:    b >> sid >> query.text; break;
      case RESPONSE: b >> sid >> response.text; break;
      case JOIN:     b >> sid >> join.frame; break;
      case LEAVE:    b >> sid >> leave.frame; break;
      case DELAY:    b >> sid >> delay.delay; break;
      case STATE:    statep.reset(new ist::bbuffer());
                     b >> *statep >> state.size >> state.frame; break;
      case SYNC:     b >> sid >> sync.life >> sync.frame; break;
      case RESYNC:   break;
      default: throw NetworkError("protocol error"); break;
      }
    }


    static NMessage Input(int stat, size_t elapsed, size_t frame)
    {
      NMessage t;
      t.type = INPUT;
      t.sid = 0;
      t.input.input = stat;
      t.input.elapsed = elapsed;
      t.input.frame = frame;
      return t;
    }

    static NMessage Start(const GameOption& opt)
    {
      NMessage t;
      t.type = START;
      t.sid = 0;
      t.start.fromGameOption(opt);
      return t;
    }

    static NMessage End()
    {
      NMessage t;
      t.type = END;
      t.sid = 0;
      return t;
    }

    static NMessage Pause()
    {
      NMessage t;
      t.type = PAUSE;
      t.sid = 0;
      t.pause.frame = 0;
      return t;
    }

    static NMessage Resume()
    {
      NMessage t;
      t.type = RESUME;
      t.sid = 0;
      t.resume.frame = 0;
      return t;
    }

    static NMessage Close(size_t sid)
    {
      NMessage t;
      t.type = CLOSE;
      t.sid = sid;
      return t;
    }

    static NMessage ClientStatus(size_t sid, size_t ping)
    {
      NMessage t;
      t.type = CSTAT;
      t.sid = sid;
      t.cstat.ping = ping;
      return t;
    }

    static NMessage ServerStatus(string text_)
    {
      string text = trim(text_, 63);
      for(size_t i=0; i<text.size(); ++i) {
        if(text[i]=='\'') { text[i]='"'; }
      }

      NMessage t;
      t.type = SSTAT;
      t.sid = 0;
      strcpy(t.sstat.text, text.c_str());
      return t;
    }

    static NMessage Text(const string& text_, const vector4& color=vector4(1,1,1,0.9f))
    {
      string text = trim(text_, 47);
      NMessage t;
      t.type = TEXT;
      t.sid = 0;
      strcpy(t.text.text, text.c_str());
      t.text.color[0] = color.x;
      t.text.color[1] = color.y;
      t.text.color[2] = color.z;
      t.text.color[3] = color.w;
      return t;
    }

    static NMessage Accept(size_t sid)
    {
      NMessage t;
      t.type = ACCEPT;
      t.sid = sid;
      return t;
    }

    static NMessage Reject(const string& text)
    {
      NMessage t;
      t.type = REJECT;
      strcpy(t.reject.text, text.c_str());
      return t;
    }

    static NMessage Entry(size_t sid, const string& name, const vector4& color, size_t frame=0)
    {
      NMessage t;
      t.type = ENTRY;
      t.sid = sid;
      strcpy(t.entry.name, name.c_str());
      t.entry.color[0] = color.x;
      t.entry.color[1] = color.y;
      t.entry.color[2] = color.z;
      t.entry.color[3] = color.w;
      t.entry.frame = frame;
      return t;
    }

    static NMessage Query(const string& text)
    {
      NMessage t;
      t.type = QUERY;
      t.sid = 0;
      strcpy(t.query.text, text.c_str());
      return t;
    }

    static NMessage Response(const string& text)
    {
      NMessage t;
      t.type = RESPONSE;
      t.sid = 0;
      strcpy(t.response.text, text.c_str());
      return t;
    }

    static NMessage Join(size_t sid, size_t frame=0)
    {
      NMessage t;
      t.type = JOIN;
      t.sid = sid;
      t.join.frame = frame;
      return t;
    }

    static NMessage Leave(size_t sid, size_t frame=0)
    {
      NMessage t;
      t.type = LEAVE;
      t.sid = sid;
      t.leave.frame = frame;
      return t;
    }

    static NMessage State(ist::bbuffer *b, size_t size, size_t frame)
    {
      NMessage t;
      t.type = STATE;
      t.sid = 0;
      t.state.frame = frame;
      t.state.size = size;
      t.statep.reset(b);
      return t;
    }

    static NMessage Delay(size_t delay)
    {
      NMessage t;
      t.type = DELAY;
      t.sid = 0;
      t.delay.delay = delay;
      return t;
    }

    static NMessage Resync()
    {
      NMessage t;
      t.type = RESYNC;
      return t;
    }
  };

  typedef std::vector<NMessage> nmessage_cont;



#ifdef EXCEPTION_ENABLE_NETPLAY
  inline void SendNetworkMessage(tcp::iostream& socket, const nmessage_cont& mes)
  {
    std::stringstream ss;
    ss << "exception_conflict " << EXCEPTION_VERSION << "\n";
    ist::biostream bio(ss);
    bio << mes.size();
    for(size_t i=0; i<mes.size(); ++i) {
      mes[i].serialize(bio);
    }

    const string& s = ss.str();
    socket.write(s.c_str(), s.size());
  }

  inline void RecvNetworkMessage(tcp::iostream& ss, nmessage_cont& mes)
  {
    string header;
    int version;

    std::getline(ss, header);
    if(sscanf(header.c_str(), "exception_conflict %d", &version)!=1) {
      throw ProtocolError(header);
    }
    else if(version!=EXCEPTION_VERSION) {
      throw NetworkError(string("version not matched")
        +" ("+sgui::Format("%.2f", float(version/100.0f))+")");
    }

    ist::biostream bio(ss);
    size_t size;
    bio >> size;
    for(size_t i=0; i<size; ++i) {
      mes.push_back(NMessage(bio));
    }
  }

  inline string GetIP(socket_ptr s)
  {
    return s->rdbuf()->remote_endpoint().address().to_string();
  }
#endif // EXCEPTION_ENABLE_NETPLAY 

  class Thread
  {
  private:
    typedef boost::shared_ptr<boost::thread> thread_t;
    thread_t m_thread;
    bool m_is_running;
    bool m_is_complete;

  public:
    Thread() : m_is_running(false), m_is_complete(false)
    {}

    virtual ~Thread()
    {}

    virtual void run()
    {
      m_thread.reset(new boost::thread(boost::ref(*this)));
    }

    void join()
    {
      if(m_thread) {
        m_thread->join();
      }
    }

    bool timed_join(int ms)
    {
      if(m_thread) {
        return m_thread->timed_join(boost::posix_time::milliseconds(ms));
      }
      return true;
    }

    void detach()
    {
      if(m_thread) {
        m_thread->detach();
      }
    }

    bool isRunning() const
    {
      return m_is_running;
    }

    bool isComplete() const
    {
      return m_is_complete;
    }

    void operator()()
    {
      m_is_running = true;
      exec();
      m_is_running = false;
      m_is_complete = true;
    }

    virtual void exec()=0;
  };

  typedef shared_ptr<Thread> thread_ptr;


  class IInputServer : public Thread
  {
  public:
    virtual bool isAccepting() const=0;
    virtual string getError() const=0;
  };

#ifdef EXCEPTION_ENABLE_NETPLAY

  class InputServer : public IInputServer
  {
  typedef IInputServer Super;
  public:

    class Client : public Thread
    {
    private:
      boost::mutex m_recv_mutex;
      boost::mutex m_send_mutex;
      nmessage_cont m_trecv_data;
      nmessage_cont m_recv_data;
      nmessage_cont m_send_data;
      socket_ptr m_socket;
      const string m_ip;
      const size_t m_session_id;
      const string m_name;
      const vector4 m_color;
      bool m_stopped;
      bool m_closed;
      size_t m_ping;
      size_t m_elapsed;
      size_t m_begin_frame;
      size_t m_frame;

      boost::mutex m_sync_mutex;
      std::deque<NMessage> m_sync_info;

    public:
      Client(socket_ptr s, size_t sid, const string& name, const vector4& color) :
        m_socket(s), m_ip(GetIP(s)), m_session_id(sid), m_name(name), m_color(color),
        m_stopped(false), m_closed(false),
        m_ping(0), m_elapsed(0), m_begin_frame(0), m_frame(0)
      {}

      ~Client()
      {
        stop();
        join();
      }

      socket_ptr& getSocket() { return m_socket; }
      const string& getIP() { return m_ip; }
      bool isClosed() { return m_closed; }
      size_t getID() { return m_session_id; }
      const string& getName() { return m_name; }
      const vector4& getColor() { return m_color; }
      size_t getPing() { return m_ping; }
      size_t getElapsed() { return m_ping; }
      size_t getFrame() { return m_frame; }
      size_t getBeginFrame() { return m_begin_frame; }

      void setPing(size_t v) { m_ping=v; }
      void setBeginFrame(size_t v) { m_begin_frame=v; }

      boost::mutex& getSyncMutex() { return m_sync_mutex; }
      std::deque<NMessage>& getSyncInfo() { return m_sync_info; }

      void push(const NMessage& m) // sync 
      {
        boost::mutex::scoped_lock lock(m_send_mutex);
        m_send_data.push_back(m);
      }

      void push(const nmessage_cont& mc) // sync 
      {
        boost::mutex::scoped_lock lock(m_send_mutex);
        m_send_data.insert(m_send_data.end(), mc.begin(), mc.end());
      }

      void pop(nmessage_cont& mc) // sync 
      {
        boost::mutex::scoped_lock lock(m_recv_mutex);
        mc.insert(mc.end(), m_recv_data.begin(), m_recv_data.end());
        m_recv_data.clear();
      }

      NMessage waitMessage(int type) // sync 
      {
        for(;;) {
          {
            boost::mutex::scoped_lock lock(m_recv_mutex);
            for(nmessage_cont::iterator p=m_recv_data.begin(); p!=m_recv_data.end(); ++p) {
              if(p->type==type) {
                NMessage tmp = *p;
                m_recv_data.erase(p);
                return tmp;
              }
            }
          }
          if(!isRunning()) {
            throw NetworkError("connection closed");
          }
          sgui::Sleep(5);
        }
      }

      void stop()
      {
        m_stopped = true;
      }

      void exec()
      {
        try {
          while(!m_stopped) {
            int t = sgui::GetTicks();

            recv();
            send();

            m_ping = sgui::GetTicks()-t;
            // ローカルだと凄い勢いで空回りするため、早すぎる場合少し待つ 
            if(m_ping < 1) {
              sgui::Sleep(1);
            }
          }
        }
        catch(...) {
        }
        m_closed = true;
        m_socket->close();
      }


    private:
      void send() // sync 
      {
        boost::mutex::scoped_lock lock(m_send_mutex);
        SendNetworkMessage(*m_socket, m_send_data);
        m_send_data.clear();
      }

      bool handleMessage(const NMessage& m)
      {
        if(m.type==NMessage::INPUT) {
          m_elapsed = m.input.elapsed;
          m_frame = m.input.frame;
        }
        else if(m.type==NMessage::CLOSE || m.type==NMessage::END) {
          stop();
          return true;
        }
        else if(m.type==NMessage::PAUSE) {
          if(!GetConfig()->server_allow_pause) {
            nmessage_cont nm;
            nm.push_back(NMessage::Text("pause command is disabled."));
            nm.push_back(NMessage::Text("type '/leave' to leave this game."));
            nm.push_back(NMessage::Text("type '/config' to call config dialog."));
            push(nm);
            return true;
          }
        }
        else if(m.type==NMessage::SYNC) {
          boost::mutex::scoped_lock lock(m_sync_mutex);
          m_sync_info.push_back(m);
          return true;
        }
        return false;
      }

      struct handler
      {
        Client *m_client;
        handler(Client *c) : m_client(c) {}
        bool operator()(const NMessage& m)
        {
          return m_client->handleMessage(m);
        }
      };
      friend struct handler;

      void recv() // sync 
      {
        RecvNetworkMessage(*m_socket, m_trecv_data);
        for(size_t i=0; i<m_trecv_data.size(); ++i) {
          m_trecv_data[i].sid = m_session_id;
        }
        m_trecv_data.erase(
          std::remove_if(m_trecv_data.begin(), m_trecv_data.end(), handler(this)),
          m_trecv_data.end());

        {
          boost::mutex::scoped_lock lock(m_recv_mutex);
          m_recv_data.insert(m_recv_data.end(), m_trecv_data.begin(), m_trecv_data.end());
        }
        m_trecv_data.clear();
      }
    };


    class Appender : public Thread
    {
    private:
      string m_comment;

    public:
      Appender(const string& comment="") : m_comment(comment)
      {}

      ~Appender()
      {
        join();
      }

      void exec()
      {
        boost::asio::io_service ios;
        ist::HTTPRequest req(ios);

        char url[256];

        IConfig& c = *GetConfig();
        sprintf(url,
          EXCEPTION_HOST_PATH "server/add&port=%d&name=%s&comment=%s",
          c.server_port, c.scorename.c_str(), m_comment.c_str());
        req.get(EXCEPTION_HOST, url);
      }
    };

    class Remover : public Thread
    {
    public:
      ~Remover()
      {
        join();
      }

      void exec()
      {
        boost::asio::io_service ios;
        ist::HTTPRequest req(ios);
        req.get(EXCEPTION_HOST, EXCEPTION_HOST_PATH "server/del");
      }
    };

    class Flusher : public Thread
    {
    private:
      InputServer *m_server;
      volatile bool m_stoped;

    public:
      Flusher(InputServer& server) :
        m_server(&server), m_stoped(false)
      {}

      ~Flusher()
      {
        stop();
        join();
      }

      void stop()
      {
        m_stoped = true;
      }

      void exec()
      {
        while(!m_stoped) {
          int t = sgui::GetTicks();

          m_server->flush();

          // 空回り緩和 
          int past = sgui::GetTicks()-t;
          if(past<1) {
            sgui::Sleep(1);
          }
        }
      }
    };

    class SyncValidator : public Thread
    {
    private:
      InputServer *m_server;
      volatile bool m_stoped;

    public:
      SyncValidator(InputServer& server) :
        m_server(&server), m_stoped(false)
      {}

      ~SyncValidator()
      {
        m_stoped = true;
        join();
      }

      void exec()
      {
        while(!m_stoped) {
          int t = sgui::GetTicks();
          m_server->validateSync();
          while(!m_stoped && sgui::GetTicks()-t<10000) {
            sgui::Sleep(100);
          }
        }
      }
    };

    class Acceptor : public Thread
    {
    private:
      InputServer *m_server;
      socket_ptr m_socket;

    public:
      Acceptor(InputServer& server, socket_ptr s) :
          m_server(&server), m_socket(s)
      {}

      ~Acceptor()
      {
        if(!isComplete() && !timed_join(50)) {
          m_socket->close();
          join();
        }
      }

      void exec()
      {
        m_server->accept(m_socket);
      }
    };

    typedef shared_ptr<Client> client_ptr;
    typedef std::map<int, client_ptr> client_cont;

    struct is_complete {
      bool operator()(const thread_ptr& p) {
        return p->isComplete();
      }
    };


  private:
    typedef shared_ptr<tcp::acceptor> acceptor_ptr;
    boost::asio::io_service m_io_service;
    mutable boost::recursive_mutex m_mutex;
    mutable boost::mutex m_stop_mutex;
    mutable boost::mutex m_error_mutex;
    acceptor_ptr m_acceptor;
    client_cont m_clients;
    thread_ptr m_flusher;
    thread_ptr m_sync_validator;
    std::list<thread_ptr> m_accept;
    nmessage_cont m_send_data;
    string m_error;
    string m_status;
    bool m_stopped;
    bool m_accepting;
    bool m_started;
    int m_idgen;
    size_t m_frame;
    size_t m_delay;
    GameOption m_initial_opt;
    GameOption m_opt;
    NMessage::GStatField m_gstat;

    std::vector<regex> m_http_admin;
    std::vector<regex> m_http_allow;
    std::vector<regex> m_http_deny;
    bool m_http_allow_players;
    bool m_http_admin_players;

  public:
    InputServer() :
      m_stopped(false), m_accepting(false), m_started(false), m_idgen(0), m_frame(0), m_delay(5),
      m_http_allow_players(false), m_http_admin_players(false)
    {
      loadHTTPConfig();
    }

    ~InputServer()
    {
      if(!isComplete()) {
        stop();
        if(!timed_join(100)) {
          m_acceptor->close();
          join();
        }
      }
      m_accept.clear();
      closeClients();
      m_flusher.reset();
      m_sync_validator.reset();
    }

    void setGameOption(const GameOption& opt)
    {
      m_initial_opt = m_opt = opt;
    }

    void pushMessage(const NMessage& m)
    {
      boost::recursive_mutex::scoped_lock lock(m_mutex);
      m_send_data.push_back(m);
    }

    void startGame()
    {
      pushMessage(NMessage::Start(m_initial_opt));
    }

    bool isGameStarted() const
    {
      return m_started;
    }

    void stop()
    {
      boost::mutex::scoped_lock lock(m_stop_mutex);
      if(m_stopped) {
        return;
      }
      m_stopped = true;

      tcp::iostream socket("127.0.0.1", lexical_cast<string>(GetConfig()->server_port));
      nmessage_cont sm;
      SendNetworkMessage(socket, sm);

      removeServerInfomation();
    }

    string getError() const
    {
      boost::mutex::scoped_lock lock(m_error_mutex); // sync 
      return m_error;
    }

    bool isAccepting() const
    {
      return m_accepting;
    }

    size_t getClientCount() const
    {
      boost::recursive_mutex::scoped_lock lock(m_mutex);
      return m_clients.size();
    }

    string getClientCountString() const
    {
      return sgui::Format("%d/%dplayers", getClientCount(), GetConfig()->server_max_connection);
    }



    void exec()
    {
      try {
        m_flusher.reset(new Flusher(*this));
        m_flusher->run();

        m_sync_validator.reset(new SyncValidator(*this));
        m_sync_validator->run();

        boost::system::error_code ec;
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), GetConfig()->server_port);
        m_acceptor.reset(new tcp::acceptor(m_io_service));
        if(m_acceptor->open(endpoint.protocol(), ec)) {
          throw NetworkError(ec.message());
        }
        if(m_acceptor->bind(endpoint, ec)) {
          throw NetworkError(ec.message());
        }
        if(m_acceptor->listen(tcp::acceptor::max_connections, ec)) {
          throw NetworkError(ec.message());
        }
        m_accepting = true;

        for(;;) {
          socket_ptr s(new tcp::iostream());
          m_acceptor->accept(*s->rdbuf());
          if(m_stopped && GetIP(s)=="127.0.0.1") {
            break;
          }
          else {
            thread_ptr a(new Acceptor(*this, s));
            a->run();
            m_accept.push_back(a);
            m_accept.erase(std::remove_if(m_accept.begin(), m_accept.end(), is_complete()), m_accept.end());
          }
        }
      }
      catch(const std::exception& e) {
        boost::mutex::scoped_lock lock(m_error_mutex); // sync 
        m_error = e.what();
      }
    }

    void accept(socket_ptr socket)
    {
      nmessage_cont rm;
      nmessage_cont sm;
      try {
        RecvNetworkMessage(*socket, rm);
      }
      catch(const ProtocolError& e) {
        boost::smatch m;
        string header = e.what();
        if(regex_search(header, m, regex("^GET ([^ ]+)"))) {
          processHTTPRequest(socket, m.str(1));
        }
        else {
          sm.push_back(NMessage::Reject("protocol error"));
          SendNetworkMessage(*socket, sm);
        }
      }
      catch(const NetworkError& e) {
        sm.push_back(NMessage::Reject(e.what()));
        SendNetworkMessage(*socket, sm);
      }

      if(rm.empty() || rm[0].type!=NMessage::ENTRY || !processEntry(socket, rm[0])) {
        socket->close();
      }
    }


    void processHTTPRequest(socket_ptr socket, const string& request)
    {
      string l;
      for(;;) {
        std::getline(*socket, l);
        if(l.empty() || l[0]=='\r' || l[0]=='\n') {
          break;
        }
      }

      boost::recursive_mutex::scoped_lock lock(m_mutex); // sync 
      int parmission = getParmission(socket);
      string r;
      if(parseHTTPCommand(socket, request, parmission)) {
        sgui::Sleep(10);
        *socket <<
"HTTP/1.1 302 Found\r\n"
"Location: ./\r\n"
"Content-Length: 186\r\n"
"Connection: close\r\n"
"Content-Type: text/html; charset=iso-8859-1\r\n"
"\r\n"
"<!DOCTYPE HTML PUBLIC '-//IETF//DTD HTML 2.0//EN'>"
"<html><head>"
"<title>302 Found</title>"
"</head><body>"
"<h1>Found</h1>"
"<p>The document has moved <a href='./'>here</a>.</p>"
"</body></html>";
      }
      else {
        string form = HTMLForm(parmission);
        *socket <<
"HTTP/1.1 200 OK\r\n"
"Content-Length: "+lexical_cast<string>(form.size())+"\r\n"
"Connection: close\r\n"
"Content-Type: text/html; charset=Shift_JIS\r\n"
"\r\n";
        *socket << form;
      }
    }

    bool parseHTTPCommand(socket_ptr socket, const string& header, int parmission)
    {
      bool ret = false;
      const string& address = GetIP(socket);
      boost::smatch m;
      if(parmission>0 && regex_search(header, m, regex("cmd=gconf"))) {
        parseHTTPGameConfig(header);
        ret = true;
      }
      else if(parmission>1 && regex_search(header, m, regex("cmd=sconf"))) {
        parseHTTPServerConfig(header);
        ret = true;
      }
      else if(parmission>1 && regex_search(header, m, regex("cmd=reset"))) {
        NMessage t;
        t.type = NMessage::END;
        t.sid = GetSessionID();
        m_send_data.push_back(t);
        ret = true;
      }
      if(ret) {
        Print(sgui::Format("http: %s %s\n", address.c_str(), header.c_str()));
      }

      return ret;
    }

    void parseHTTPServerConfig(const string& header)
    {
      IConfig& conf = *GetConfig();
      boost::smatch m;
      conf.server_allow_pause = regex_search(header, m, regex("allow_pause=1"));
      if(regex_search(header, m, regex("max_players=([0-9]+)"))) {
        conf.server_max_connection = trim<int>(2, lexical_cast<int>(m.str(1)), 16);
      }
    }

    void parseHTTPGameConfig(const string& header)
    {
      NMessage t;
      t.type = NMessage::GCONF;
      t.gconf.initialize();
      t.gconf.frame = m_frame+m_delay*2;

      boost::smatch m;
      if(regex_search(header, m, regex("level=([0-9]+)"))) {
        t.gconf.level = std::min<float>(lexical_cast<int>(m.str(1)), LEVEL_FUTURE);
      }
      if(regex_search(header, m, regex("cboost=([0-9.]+)"))) {
        try { t.gconf.cboost = std::min<float>(lexical_cast<float>(m.str(1)), 3.0f); } catch(...) {}
      }
      if(regex_search(header, m, regex("fboost=([0-9.]+)"))) {
        try { t.gconf.fboost = std::min<float>(lexical_cast<float>(m.str(1)), 2.0f); } catch(...) {}
      }
      if(regex_search(header, m, regex("eboost=([0-9.]+)"))) {
        try { t.gconf.eboost = std::min<float>(lexical_cast<float>(m.str(1)), 3.0f); } catch(...) {}
      }
      if(!m_started) {
        if(regex_search(header, m, regex("map=([0-9]+)"))) {
          t.gconf.stage = std::min<int>(lexical_cast<int>(m.str(1)), MAP_TEAMFORTRESS);
        }
        if(regex_search(header, m, regex("h_wave=([0-9]+)"))) {
          int i = std::min<int>(lexical_cast<int>(m.str(1)), 29);
          if(i>=0) {
            t.gconf.horde_wave = i+1;
          }
        }
        if(regex_search(header, m, regex("dm_time=([0-9]+)"))) {
          int i = std::min<int>(lexical_cast<int>(m.str(1)), 2);
          if(i>=0) {
            int ls[] = {3,5,8};
            t.gconf.deathmatch_time = ls[i];
          }
        }
        if(regex_search(header, m, regex("tf_life=([0-9]+)"))) {
          int i = std::min<int>(lexical_cast<int>(m.str(1)), 4);
          if(i>=0) {
            float ls[] = {0.5f, 1.0f, 2.0f, 3.0f, 5.0f};
            t.gconf.teamfortress_life = ls[i];
          }
        }
      }
      m_send_data.push_back(t);

      if(!m_started && regex_search(header, m, regex("start=start"))) {
        GameOption opt = m_opt;
        t.gconf.toGameOption(opt);
        m_send_data.push_back(NMessage::Start(opt));
      }
    }

    string HTMLForm(int parmission)
    {
      IConfig& conf = *GetConfig();
      string b;
      b+=
"<?xml version='1.0' encoding='Shift_JIS'?>"
"<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Strict//EN' 'http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd'>"
"<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='ja' lang='ja'>"
"<head>"
"<meta http-equiv='content-type' content='text/html; charset=Shift_JIS' />"
"<title>exception conflict server</title>"
"</head>"
"<body>"
"<div>";

      b+=string("exception conflict version ")
        +sgui::Format("%.2f",float(EXCEPTION_VERSION)/100.0f)+"<br /><br />";
      b+=m_status+" "+getClientCountString()+"<br />";

      if(m_frame>0) {
        b+=sgui::Format("elapsed: %d:%02d<br />", m_gstat.elapsed/1000/60, m_gstat.elapsed/1000%60);
        b+=sgui::Format("elapsed par frame: %.2fms<br />", float(m_gstat.elapsed_par_frame)/60.0f);
        b+=sgui::Format("average fps: %d<br />", m_gstat.fps);
        b+=sgui::Format("delay: %d<br />", m_gstat.delay);
        b+=sgui::Format("objects: %d<br />", m_gstat.obj);
        b+=sgui::Format("alive players: %d/%d<br />", m_gstat.alives, m_gstat.players);
      }

      b+="<br />";
      b+="player name:<br />";
      for(client_cont::iterator p=m_clients.begin(); p!=m_clients.end(); ++p) {
        b+=p->second->getName()+ "<br />";
      }
      b+="<br /><br />";

      if(parmission>0) {
        b+=
string("you are ")+(parmission==1?"permitted user":"administrator")+".<br /><br />"
"game config:<br />"
"<form action='./' method='get'>"
"<div>"
"<input type='hidden' name='cmd' value='gconf' />"
"level:";
        string level[] = {"light","normal","heavy","excess","future"};
        for(int i=0; i<5; ++i) {
          b+=" <input type='radio' name='level' value='"+sgui::Format("%d", i)+"' "+
            (i==m_opt.level ? "checked='checked'" : "")+" />"+level[i];
        }
        b+=
"<br />"
"catapult boost: "
"<input type='text' name='cboost' size='5' value='"+sgui::Format("%.1f", m_opt.cboost)+"' /> (0.0-3.0)<br />"
"fraction boost: "
"<input type='text' name='fboost' size='5' value='"+sgui::Format("%.1f", m_opt.fboost)+"' /> (0.0-2.0)<br />"
"enemy boost: "
"<input type='text' name='eboost' size='5' value='"+sgui::Format("%.1f", m_opt.eboost)+"' /> (0.1-3.0)<br />";
        if(!m_started) {
          b+=
"<br />"
"map: ";
          string map[] = {"horde","deathmatch","team fortress"};
          for(int i=0; i<3; ++i) {
            b+=" <input type='radio' name='map' value='"+sgui::Format("%d", i)+"' "+
              (i==m_opt.stage ? "checked='checked'" : "")+" />"+map[i];
          }
          b+="<br />";

          {
            b+="horde wave: <select name='h_wave'>";
            for(int i=0; i<30; ++i) {
              b+=" <option value='"+sgui::Format("%d", i)+"' "+
                (i+1==m_opt.horde_wave ? "selected='selected'" : "")+">"+sgui::Format("%d", i+1)+"</option>";
            }
            b+="</select><br />";
          }
          {
            b+="deathmatch time: <select name='dm_time'>";
            int dm_time[3] = {3,5,8};
            for(int i=0; i<3; ++i) {
              b+=" <option value='"+sgui::Format("%d", i)+"' "+
                (dm_time[i]==m_opt.deathmatch_time ? "selected='selected'" : "")+">"+sgui::Format("%dm", dm_time[i])+"</option>";
            }
            b+="</select><br />";
          }
          {
            b+="teamfortress life: <select name='tf_life'>";
            float tf_life[5] = {0.5f, 1.0f, 2.0f, 3.0f, 5.0f};
            for(int i=0; i<5; ++i) {
              b+="<option value='"+sgui::Format("%d", i)+"' "+
                (tf_life[i]==m_opt.teamfortress_life ? "selected='selected'" : "")+">"+sgui::Format("x%.1f", tf_life[i])+"</option>";
            }
            b+="</select><br /><br />";
          }
        }
        b+=
"<input type='submit' value='update' />";
        if(!m_started) {
          b+=
" <input type='submit' name='start' value='start' />";
        }

        b+=
"</div>"
"</form>"
"<br /><br />";
        if(parmission>1) {
          b+=string(
"server config:<br />"
"<form action='./' method='get'>"
"<div>"
"<input type='hidden' name='cmd' value='sconf' />"
"allow pause: <input type='checkbox' name='allow_pause' value='1' ")+(conf.server_allow_pause?"checked='checked'":"")+" /><br />"
"max players: <select name='max_players'>";
          for(int i=2; i<=16; ++i) {
            b+=string(" <option value='")+lexical_cast<string>(i)+"' "+
              (i==conf.server_max_connection ? "selected='selected'" : "")+
              ">"+lexical_cast<string>(i)+"</option>";
          }
          b+=
"</select><br />"
"<input type='submit' value='update' />"
"</div>"
"</form>";
          if(m_started) {
            b+=
"<br /><br /><br />"
"<form action='./' method='get'>"
"<div>"
"<input type='hidden' name='cmd' value='reset' />"
"<input type='submit' value='reset game' />"
"</div>"
"</form>";
          }
        }
      }
      else {
        b+=
"you are not permitted user.<br />";
      }
      b+=
"</div>"
"</body>"
"</html>";
      return b;
    }

    bool isJoined(socket_ptr socket)
    {
      for(client_cont::iterator p=m_clients.begin(); p!=m_clients.end(); ++p) {
        socket_ptr& c = p->second->getSocket();
        if(c->rdbuf()->remote_endpoint().address()==socket->rdbuf()->remote_endpoint().address()) {
          return true;
        }
      }
      return false;
    }

    int getParmission(socket_ptr socket)
    {
      const string& address = GetIP(socket);
      boost::smatch m;
      for(int i=0; i<m_http_deny.size(); ++i) {
        if(regex_search(address, m, m_http_deny[i])) {
          return 0;
        }
      }
      int parmission = 0;
      if(isJoined(socket)) {
        if(m_http_allow_players) {
          parmission = 1;
        }
        if(m_http_admin_players) {
          parmission = 2;
        }
      }
      for(int i=0; i<m_http_allow.size(); ++i) {
        if(regex_search(address, m, m_http_allow[i])) {
          parmission = std::max<int>(1, parmission);
        }
      }
      for(int i=0; i<m_http_admin.size(); ++i) {
        if(regex_search(address, m, m_http_admin[i])) {
          parmission = std::max<int>(2, parmission);
        }
      }
      return parmission;
    }


    void loadHTTPConfig()
    {
      std::ifstream in("http_config");
      if(!in) {
        m_http_admin.push_back(regex("127\\.0\\.0\\.1"));
        return;
      }

      string l;
      while(std::getline(in, l)) {
        if(l.empty() || l[0]=='#') {
          continue;
        }
        boost::smatch m;
        if(regex_search(l, m, regex("allow (.*)"))) {
          string r = m.str(1);
          if(r=="players") {
            m_http_allow_players = true;
          }
          else {
            try {
              m_http_allow.push_back(regex(r));
            }
            catch(std::runtime_error& e) {
              fprintf(stderr, "%s : %s\n", l.c_str(), e.what());
            }
          }
        }
        else if(regex_search(l, m, regex("admin (.*)"))) {
          string r = m.str(1);
          if(r=="players") {
            m_http_admin_players = true;
          }
          else {
            try {
              m_http_admin.push_back(regex(r));
            }
            catch(std::runtime_error& e) {
              fprintf(stderr, "%s : %s\n", l.c_str(), e.what());
            }
          }
        }
        else if(regex_search(l, m, regex("deny (.*)"))) {
          string r = m.str(1);
          try {
            m_http_deny.push_back(regex(r));
          }
          catch(std::runtime_error& e) {
            fprintf(stderr, "%s : %s\n", l.c_str(), e.what());
          }
        }
      }
    }

    void updateServerStatusString(const string& ss)
    {
      m_status = regex_replace(ss, regex("\\+"), " ");
    }

    bool processEntry(socket_ptr socket, NMessage& m)
    {
      socket->rdbuf()->set_option(tcp::no_delay(true));
      boost::recursive_mutex::scoped_lock lock(m_mutex); // sync 
      nmessage_cont sm;

      // 最大人数超えてたらreject 
      int max_connection = GetConfig()->server_max_connection;
      if(m_clients.size()>=max_connection) {
        char buf[128];
        sprintf(buf, "too many players (max: %dplayers)", max_connection);
        sm.push_back(NMessage::Reject(buf));
        SendNetworkMessage(*socket, sm);
        return false;
      }
      // 100ms以内に応答が返ってこなかったらreject 
      size_t ping = getResponseTime(socket);
      if(ping>=100) {
        sm.push_back(NMessage::Reject("network too lazy"));
        SendNetworkMessage(*socket, sm);
        return false;
      }
      // ゲーム終了後エントリーしてしまったらreject、ここに引っかかるのは奇跡の域だけど一応 
      if(m_frame>0 && m_clients.empty()) {
        sm.push_back(NMessage::Reject("server is now shutting down.\nretry please."));
        SendNetworkMessage(*socket, sm);
        return false;
      }

      // ID割り当て 
      int sid = ++m_idgen;
      client_ptr c(new Client(socket, sid, m.entry.name, vector4(m.entry.color)));
      c->setPing(ping);
      sm.push_back(NMessage::Accept(sid));

      // 途中参加の場合、ステートと途中参加メッセージを送信 
      if(m_started && m_frame>0) {
        {
          NMessage m = NMessage::Response("new player");
          m.sid = sid;
          c->push(m);
          for(client_cont::iterator p=m_clients.begin(); p!=m_clients.end(); ++p) {
            p->second->push(m); // incoming new player表示 
          }
        }

        try {
          client_ptr primary = m_clients.begin()->second;
          primary->push(NMessage::Query("state"));
          NMessage m = primary->waitMessage(NMessage::STATE);
          sm.push_back(m);
          c->setBeginFrame(m.state.frame);
          m_send_data.push_back(NMessage::Entry(c->getID(), c->getName(), c->getColor(), m.state.frame+m_delay*2));
          m_send_data.push_back(NMessage::Join(c->getID(), m.state.frame+m_delay*2));
          m_clients[sid] = c;
        }
        catch(...) {
          c->push(NMessage::Reject("primary player disconnected.\nretry please."));
          for(client_cont::iterator p=m_clients.begin(); p!=m_clients.end(); ++p) {
            client_ptr& cli = p->second;
            NMessage m = NMessage::Response("state restored");
            m.sid = sid;
            cli->push(m);
          }
          return false;
        }
      }
      else {
        // 他のプレイヤーの情報を送信 
        m_clients[sid] = c;
        for(client_cont::iterator p=m_clients.begin(); p!=m_clients.end(); ++p) {
          client_ptr& cli = p->second;
          sm.push_back(NMessage::Entry(cli->getID(), cli->getName(), cli->getColor()));
        }
        m_send_data.push_back(NMessage::Entry(c->getID(), c->getName(), c->getColor()));
        if(m_started) {
          sm.push_back(NMessage::Start(m_opt));
        }
      }

      // ゲーム開始前ならウェルカムメッセージ送信 
      if(!m_started) {
        string autoreply = createAutoReplyMessage();
        if(!autoreply.empty()) {
          vector4 color(0.5f, 0.5f, 1.0f, 0.9f);
          sm.push_back(NMessage::Text("server autoreply:\n", color));
          std::istringstream ss(autoreply);
          string l;
          while(std::getline(ss, l)) {
            sm.push_back(NMessage::Text(l, color));
          }
        }
      }

      SendNetworkMessage(*socket, sm);
      c->run();

      Print(string("join: ")+c->getIP()+" "+c->getName()+"\n");
      return true;
    }

    string createAutoReplyMessage()
    {
      IConfig& c = *GetConfig();
      boost::mutex::scoped_lock lock(c.mutex);
      string r = c.server_autoreply;
      r = regex_replace(r, regex("\\{name\\}"), c.scorename);
      r = regex_replace(r, regex("\\{max_connection\\}"), sgui::Format("%d", c.server_max_connection));
      r = regex_replace(r, regex("\\{autostart\\}"), sgui::Format("%d", c.server_autostart));
      r = regex_replace(r, regex("\\{level\\}"), GetLevelString(m_opt.level));
      r = regex_replace(r, regex("\\{map\\}"), GetMapString(m_opt.stage));
      r = regex_replace(r, regex("\\{horde_wave\\}"), sgui::Format("%d", m_opt.horde_wave));
      r = regex_replace(r, regex("\\{deathmatch_time\\}"), sgui::Format("%d", m_opt.deathmatch_time));
      r = regex_replace(r, regex("\\{teamfortress_life\\}"), sgui::Format("%.2f", m_opt.teamfortress_life));
      r = regex_replace(r, regex("\\{catapult_boost\\}"), sgui::Format("%.2f", m_opt.cboost));
      r = regex_replace(r, regex("\\{fraction_boost\\}"), sgui::Format("%.2f", m_opt.fboost));
      r = regex_replace(r, regex("\\{enemy_boost\\}"), sgui::Format("%.2f", m_opt.eboost));
      return r;
    }

    size_t getResponseTime(socket_ptr socket)
    {
      nmessage_cont mc;
      Uint32 start = sgui::GetTicks();
      SendNetworkMessage(*socket, mc);
      RecvNetworkMessage(*socket, mc);
      Uint32 t = sgui::GetTicks()-start;
      return t;
    }


    void sendServerInfomation(const string& info)
    {
      static shared_ptr<Appender> s_updater;
      if(!s_updater || !s_updater->isRunning()) {
        s_updater.reset(new Appender(info+getClientCountString()));
        s_updater->run();
      }
    }

    void removeServerInfomation()
    {
      static shared_ptr<Remover> s_remover;
      if(!s_remover || !s_remover->isRunning()) {
        s_remover.reset(new Remover());
        s_remover->run();
      }
    }


    struct handler
    {
      InputServer *server;
      handler(InputServer& s) : server(&s) {}
      bool operator()(NMessage& m) {
        return server->fetch(m);
      }
    };
    friend struct handler;

    void flush() // sync 
    {
      boost::recursive_mutex::scoped_lock lock(m_mutex);

      // クライアントに溜まったメッセージを一まとめに 
      for(client_cont::iterator p=m_clients.begin(); p!=m_clients.end(); ++p) {
        p->second->pop(m_send_data);
      }
      updateFrame();
      m_send_data.erase(std::remove_if(m_send_data.begin(), m_send_data.end(), handler(*this)), m_send_data.end());

      // クライアントの状態をメッセージ化 
      for(client_cont::iterator p=m_clients.begin(); p!=m_clients.end(); /**/) {
        client_ptr c = p->second;
        int sid = p->first;
        if(c->isClosed()) { // コネクションが切れてたら消去 
          m_clients.erase(p++);
          m_send_data.push_back(NMessage::Close(sid));
          if(m_frame>0) {
            if(sid==GetSessionID()) { // サーバープレイヤーが接続を切ったらゲーム終了 
              m_send_data.push_back(NMessage::End());
            }
            else {
              m_send_data.push_back(NMessage::Leave(sid, m_frame+m_delay));
            }
          }

          Print(string("leave: ")+c->getIP()+" "+c->getName()+"\n");
          if(m_clients.empty() && m_started) {
            stop();
          }
        }
        else {
          m_send_data.push_back(NMessage::ClientStatus(sid, c->getPing()));
          ++p;
        }
      }

      // 全メッセージ送信 
      for(client_cont::iterator p=m_clients.begin(); p!=m_clients.end(); ++p) {
        p->second->push(m_send_data);
      }
      m_send_data.clear();
    }

    void updateFrame()
    {
      for(int i=0; i<m_send_data.size(); ++i) {
        NMessage& m = m_send_data[i];
        if(m.type==NMessage::INPUT) {
          m_frame = std::max<size_t>(m_frame, m.input.frame);
        }
      }
    }

    bool fetch(NMessage& m)
    {
      bool primary = m.sid==0 || (!m_clients.empty() && m.sid==m_clients.begin()->second->getID());
      if(m.type==NMessage::PAUSE) {
        m.pause.frame = m_frame+m_delay*2;
      }
      else if(m.type==NMessage::START) {
        if(m_started) {
          return true;
        }
        m_started = true;
        m.start.toGameOption(m_initial_opt);
        m_opt = m_initial_opt;
        m_delay = m_opt.delay;
        Print("game start\n");
      }
      else if(m.type==NMessage::GCONF) {
        m.gconf.frame = m_frame+m_delay*2;
        if(!m_started) {
          m.gconf.toGameOption(m_initial_opt);
          m_opt = m_initial_opt;
        }
        else {
          m.gconf.toGameOptionWithoutStaticParameter(m_opt);
        }
      }
      else if(m.type==NMessage::DELAY) {
        if(!primary) {
          return true;
        }
        m_delay = m_opt.delay;
      }
      else if(m.type==NMessage::GSTAT) {
        if(primary) {
          m_gstat = m.gstat;
        }
        return true;
      }
      else if(m.type==NMessage::SSTAT) {
        if(primary) {
          sendServerInfomation(m.sstat.text);
          updateServerStatusString(m.sstat.text);
        }
        return true;
      }
      else if(m.type==NMessage::QUERY) {
        string q = m.query.text;
        nmessage_cont mc;
        if(q=="state") {
          client_cont::iterator p = m_clients.find(m.sid);
          if(p==m_clients.end()) {
            return true;
          }
          client_ptr cli = p->second;
          client_ptr primary = m_clients.begin()->second;
          if(cli==primary) {
            cli->push(NMessage::Text("# you are primary client."));
          }
          try {
            primary->push(NMessage::Query("state"));
            cli->push(NMessage::Text("# recieving state data..."));
            cli->push(primary->waitMessage(NMessage::STATE));
            cli->waitMessage(NMessage::RESPONSE);
            cli->push(NMessage::Text("# complete."));
          }
          catch(...) {
            cli->push(NMessage::Text("# getting state data failed."));
          }
          return true;
        }
      }
      else if(m.type==NMessage::RESYNC) {
        return true;
      }
      return false;
    }



    void validateSync()
    {
      boost::recursive_mutex::scoped_lock lock(m_mutex);
      if(m_clients.empty()) {
        return;
      }

      NMessage base;
      {
        client_ptr cli = m_clients.begin()->second;
        boost::mutex::scoped_lock lock(cli->getSyncMutex());
        if(cli->getSyncInfo().empty()) {
          return;
        }
        base = cli->getSyncInfo().front();
      }

      for(client_cont::iterator p=m_clients.begin(); p!=m_clients.end(); ++p) {
        client_ptr cli = p->second;
        boost::mutex::scoped_lock lock(cli->getSyncMutex());
        while(!cli->getSyncInfo().empty()) {
          NMessage t = cli->getSyncInfo().front();
          if(base.sync.frame>t.sync.frame) {
            cli->getSyncInfo().pop_front();
          }
          else if(base.sync.frame==t.sync.frame) {
            if(!base.sync.compare(t.sync)) {
              cli->push(NMessage::Text("# broken synchronization detected.", vector4(1.0f, 0.2f, 0.2f, 0.9f)));
              cli->push(NMessage::Resync());
            }
            cli->getSyncInfo().pop_front();
            break;
          }
          else {
            break;
          }
        }
      }
    }


    void closeClients() // sync 
    {
      boost::recursive_mutex::scoped_lock lock(m_mutex);
      for(client_cont::iterator p=m_clients.begin(); p!=m_clients.end(); ++p) {
        p->second->stop();
      }
      m_clients.clear();
    }
  };
#endif // EXCEPTION_ENABLE_NETPLAY 



  class Session : public ISession
  {
  private:
    input_ptr m_input;
    size_t m_id;
    string m_name;
    vector4 m_color;
    size_t m_ping;
    size_t m_delay;
    size_t m_elapsed;
    size_t m_begin_frame;
    size_t m_join_frame;
    size_t m_leave_frame;

  public:
    Session(Deserializer& s)
    {
      m_input = new InputStream(s);
      s >> m_id >> m_name >> m_color >> m_color >> m_ping >> m_delay>> m_elapsed
        >> m_begin_frame >> m_join_frame >> m_leave_frame;
    }

    void serialize(Serializer& s) const
    {
      m_input->serialize(s);
      s << m_id << m_name << m_color << m_color << m_ping << m_delay<< m_elapsed
        << m_begin_frame << m_join_frame << m_leave_frame;
    }

  public:
    Session(size_t id, const string& name, const vector4& color, input_ptr input) :
      m_input(input), m_id(id), m_name(name), m_color(color), m_ping(0), m_delay(5), m_elapsed(0),
      m_begin_frame(0), m_join_frame(0), m_leave_frame(-1)
    {}

    Session(ist::gzbstream& s, int version) :
      m_id(0), m_ping(0), m_elapsed(0), m_begin_frame(0), m_join_frame(0), m_leave_frame(-1)
    {
      s >> m_name >> m_color >> m_id >> m_begin_frame >> m_join_frame >> m_leave_frame;
      m_input = new InputStream(s);
    }

    void write(ist::gzbstream& s) const
    {
      s << m_name << m_color << m_id << m_begin_frame << m_join_frame << m_leave_frame;
      m_input->write(s);
    }

    size_t getID() const { return m_id; }
    const string& getName() const { return m_name; }
    const vector4& getColor() const { return m_color; }
    InputStream* getInput() { return m_input.get(); }

    size_t getPing() const       { return m_ping; }
    size_t getDelay() const      { return m_delay; }
    size_t getElapsed() const    { return m_elapsed; }
    size_t getBeginFrame() const { return m_begin_frame; }
    size_t getJoinFrame() const  { return m_join_frame; }
    size_t getLeaveFrame() const { return m_leave_frame; }
    void setPing(size_t v)       { m_ping=v; }
    void setDelay(size_t v)      { m_delay=v; }
    void setElapsed(size_t v)    { m_elapsed=v; }
    void setBeginFrame(size_t v) { m_begin_frame=v; }
    void setJoinFrame(size_t v)  { m_join_frame=v; }
    void setLeaveFrame(size_t v) { m_leave_frame=v; }

    void setID(size_t v) { m_id=v; }
  };
  typedef intrusive_ptr<Session> session_ptr;





  void AssignConfig(const NMessage& m);
  void ForceGameStart();



  class IInputClient : public Thread
  {
  public:
    virtual void serialize(Serializer& s)=0;
    virtual void deserialize(Deserializer& s)=0;
    virtual void write(ist::gzbstream& s)=0;
    virtual size_t getSessionCount()=0;
    virtual session_ptr getSession(size_t i)=0;
    virtual session_ptr getSessionByID(size_t id)=0;
    virtual size_t getSessionID()=0;
    virtual int getDelay()=0;
    virtual void push(const NMessage& m)=0;
    virtual void flush()=0;
    virtual void sync()=0;
    virtual void update()=0;
    virtual void browse()=0;
  };

  class BaseInputClient : public IInputClient
  {
  protected:
    typedef std::map<size_t, session_ptr> session_cont;
    session_cont m_session;
    nmessage_cont m_replay_mes;

  public:
    virtual void serialize(Serializer& s)
    {
      s << m_session.size();
      for(session_cont::const_iterator p=m_session.begin(); p!=m_session.end(); ++p) {
        p->second->serialize(s);
      }
      s << m_replay_mes.size();
      for(size_t i=0; i<m_replay_mes.size(); ++i) {
        m_replay_mes[i].serialize(s);
      }
    }

    virtual void deserialize(Deserializer& s)
    {
      m_session.clear();
      m_replay_mes.clear();

      size_t size;
      s >> size;
      for(size_t i=0; i<size; ++i) {
        session_ptr session(new Session(s));
        m_session[session->getID()] = session;
      }
      s >> size;
      for(size_t i=0; i<size; ++i) {
        m_replay_mes.push_back(NMessage(s));
      }
    }

    virtual void write(ist::gzbstream& s)
    {
      s << m_session.size();
      for(session_cont::iterator p=m_session.begin(); p!=m_session.end(); ++p) {
        p->second->write(s);
      }
      s << m_replay_mes.size();
      for(size_t i=0; i<m_replay_mes.size(); ++i) {
        m_replay_mes[i].serialize(s);
      }
    }

    virtual size_t getSessionCount()
    {
      return m_session.size();
    }

    virtual session_ptr getSession(size_t i)
    {
      if(i>=m_session.size()) {
        return session_ptr();
      }
      session_cont::iterator p = m_session.begin();
      std::advance(p, i);
      return p->second;
    }

    virtual session_ptr getSessionByID(size_t id)
    {
      session_cont::iterator p = m_session.find(id);
      if(p==m_session.end()) {
        return session_ptr();
      }
      return p->second;
    }

    virtual session_ptr findSession(size_t id)
    {
      session_cont::iterator p = m_session.find(id);
      return (p==m_session.end()) ? session_ptr() : p->second;
    }

    virtual size_t getSessionID()=0;
    virtual int getDelay() { return 0; }
    virtual void push(const NMessage& m)=0;

    virtual void update()
    {
      for(session_cont::iterator p=m_session.begin(); p!=m_session.end(); ++p) {
        p->second->getInput()->update();
      }
    }

    virtual void exec() {}


    struct handler
    {
      BaseInputClient *client;
      handler(BaseInputClient *c) : client(c) {}
      bool operator()(const NMessage& m) {
        return client->dispatch(m);
      }
    };
    friend struct handler;

    virtual void flush() {}
    virtual void sync() {}

    virtual bool dispatch(const NMessage& m) { return true; }

    bool assignConfigMessage(const NMessage& m)
    {
      IGame *game = GetGame();
      if(!game) {
        AssignConfig(m);
      }
      else if(m.gconf.frame > game->getPast()) {
        return false;
      }
      else if(m.gconf.frame < game->getPast()) {
        throw Error("NMessage::GCONF");
      }
      else if(m.gconf.frame==game->getPast()) {
        game->setLevel(m.gconf.level);
        game->setCatapultBoost(m.gconf.cboost);
        game->setFractionBoost(m.gconf.fboost);
        game->setEnemyBoost(m.gconf.eboost);
        m_replay_mes.push_back(m);
      }
      return true;
    }

    virtual void browse() {}
  };



  class InputClientLocal : public BaseInputClient
  {
  typedef BaseInputClient Super;
  private:
    nmessage_cont m_store;

  public:
    InputClientLocal()
    {
      session_ptr s(
        new Session(0, GetConfig()->scorename, GetConfig()->color, new InputStream()));
      m_session[s->getID()] = s;
    }

    void serialize(Serializer& s)
    {
      Super::serialize(s);
      s << m_store.size();
      for(size_t i=0; i<m_store.size(); ++i) {
        m_store[i].serialize(s);
      }
    }

    void deserialize(Deserializer& s)
    {
      Super::deserialize(s);
      size_t size;
      s >> size;
      for(size_t i=0; i<size; ++i) {
        m_store.push_back(NMessage(s));
      }
    }

    size_t getSessionID()
    {
      return 0;
    }

    void push(const NMessage& m)
    {
      int t = m.type;
      IGame *game = GetGame();
      if(t==NMessage::INPUT) {
        getSessionByID(m.sid)->getInput()->push(m.input.input);
      }
      else if(t==NMessage::PAUSE) {
        Pause();
      }
      else if(t==NMessage::RESUME){
        Resume();
      }
      else if(t==NMessage::END)   {
        FadeToTitle();
      }
      else if(t==NMessage::GCONF) {
        NMessage t = m;
        t.gconf.frame+=1;
        m_store.push_back(t);
      }
    }

    void sync()
    {
      m_store.erase(std::remove_if(m_store.begin(), m_store.end(), handler(this)), m_store.end());
    }

    bool dispatch(const NMessage& m)
    {
      int t = m.type;
      if(t==NMessage::GCONF) {
        return assignConfigMessage(m);
      }
      return true;
    }
  };


  class InputClientReplay : public BaseInputClient
  {
  typedef BaseInputClient Super;
  private:
    nmessage_cont m_nmes;
    session_cont m_join;
    size_t m_length;

  public:
    InputClientReplay(ist::gzbstream& s, int version) : m_length(0)
    {
      size_t size;
      s >> size;
      for(int i=0; i<size; ++i) {
        session_ptr session(new Session(s, version));
        m_length = std::max<size_t>(m_length, session->getBeginFrame()+session->getInput()->getLength());
        if(session->getBeginFrame()==0) {
          m_session[session->getID()] = session;
        }
        else {
          m_join[session->getID()] = session;
        }
      }

      s >> size;
      for(int i=0; i<size; ++i) {
        m_nmes.push_back(NMessage(s));
      }
    }

    InputClientReplay() : m_length(0)
    {}

    virtual void serialize(Serializer& s)
    {
      Super::serialize(s);
      s << m_join.size();
      for(session_cont::const_iterator p=m_join.begin(); p!=m_join.end(); ++p) {
        p->second->serialize(s);
      }
      s << m_length;

      s << m_nmes.size();
      for(int i=0; i<m_nmes.size(); ++i) {
        m_nmes[i].serialize(s);
      }
    }

    virtual void deserialize(Deserializer& s)
    {
      Super::deserialize(s);
      size_t size;
      s >> size;
      for(size_t i=0; i<size; ++i) {
        session_ptr session(new Session(s));
        m_join[session->getID()] = session;
      }
      s >> m_length;

      s >> size;
      for(int i=0; i<size; ++i) {
        m_nmes.push_back(NMessage(s));
      }
    }

    size_t getLength() { return m_length; }
    size_t getSessionID() { return 0; }

    void push(const NMessage& m)
    {
      int t = m.type;
      if(t==NMessage::PAUSE) {
        Pause();
      }
      else if(t==NMessage::RESUME){
        Resume();
      }
      else if(t==NMessage::END)   {
        FadeToTitle();
      }
    }

    void sync()
    {
      IGame *game = GetGame();
      if(!game) {
        return;
      }

      m_nmes.erase(std::remove_if(m_nmes.begin(), m_nmes.end(), handler(this)), m_nmes.end());
      for(session_cont::iterator p=m_join.begin(); p!=m_join.end(); /**/) {
        session_ptr s = p->second;
        if(s->getBeginFrame()==game->getPast()) {
          m_session[s->getID()] = s;
          m_join.erase(p++);
        }
        else {
          ++p;
        }
      }
      for(session_cont::iterator p=m_session.begin(); p!=m_session.end(); /**/) {
        session_ptr s = p->second;
        if(s->getJoinFrame()==game->getPast()) {
          game->join(s->getID());
        }
        if(s->getLeaveFrame()==game->getPast()) {
          game->leave(s->getID());
          m_session.erase(p++);
        }
        else {
          ++p;
        }
      }
    }

    bool dispatch(const NMessage& m)
    {
      int t = m.type;
      if(t==NMessage::GCONF) {
        return assignConfigMessage(m);
      }
      return true;
    }

    void print()
    {
      session_cont ses;
      for(session_cont::iterator p=m_session.begin(); p!=m_session.end(); ++p) { ses[p->first]=p->second; }
      for(session_cont::iterator p=m_join.begin(); p!=m_join.end(); ++p) { ses[p->first]=p->second; }
      for(session_cont::iterator p=ses.begin(); p!=ses.end(); ++p) {
        session_ptr s = p->second;
        printf("%s begin:%d end:%d\n", s->getName().c_str(), s->getBeginFrame(), s->getLeaveFrame());
      }

      size_t frame = 0;
      for(;; ++frame) {
        string l = sgui::Format("%05d ", frame);
        int in = 0;
        for(session_cont::iterator p=ses.begin(); p!=ses.end(); ++p) {
          session_ptr s = p->second;
          if(frame>=s->getBeginFrame() && frame<s->getBeginFrame()+s->getInput()->getLength()) {
            l+=sgui::Format(" %d", s->getInput()->getState());
            s->getInput()->update();
            ++in;
          }
        }
        if(in==0) {
          break;
        }
        else {
          printf("%s\n", l.c_str());
        }
      }

      printf("nmessage: %d\n", m_nmes.size());
      for(int i=0; i<m_nmes.size(); ++i) {
        NMessage& m = m_nmes[i];
        if(m.type==NMessage::GCONF) {
          printf("gconf: %d, %.2f, %.2f, %.2f\n", m.gconf.frame, m.gconf.cboost, m.gconf.fboost, m.gconf.eboost);
        }
      }
    }
  };


#ifdef EXCEPTION_ENABLE_NETPLAY 
  IInputServer* GetInputServer();

  class InputClientIP : public BaseInputClient
  {
  typedef BaseInputClient Super;
  private:
    socket_ptr m_socket;
    boost::recursive_mutex m_recv_mutex;
    boost::mutex m_send_mutex;
    boost::mutex m_tmp_mutex;
    nmessage_cont m_trecv_data1;
    nmessage_cont m_trecv_data2;
    nmessage_cont m_recv_data;
    nmessage_cont m_send_data;
    nmessage_cont m_start_data;
    session_cont m_closed;
    bool m_stopped;
    size_t m_session_id;
    size_t m_delay;
    bool m_request_state;
    int m_wait_count;
    int m_nowait_count;
    string m_server_http_address;

  public:
    InputClientIP() :
        m_stopped(false), m_session_id(0), m_delay(5),
        m_request_state(false), m_wait_count(0), m_nowait_count(0)
    {
    }

    ~InputClientIP()
    {
      stop();
      join();
    }

    void serialize(Serializer& s)
    {
      Super::serialize(s);
      s << m_closed.size();
      for(session_cont::const_iterator p=m_closed.begin(); p!=m_closed.end(); ++p) {
        p->second->serialize(s);
      }
      s << m_delay;

      {
        boost::recursive_mutex::scoped_lock lock(m_recv_mutex);
        s << m_recv_data.size();
        for(size_t i=0; i<m_recv_data.size(); ++i) {
          m_recv_data[i].serialize(s);
        }
      }
    }

    void deserialize(Deserializer& s)
    {
      Super::deserialize(s);

      {
        boost::mutex::scoped_lock lock(m_tmp_mutex);
        m_closed.clear();
        m_recv_data.clear();
        m_trecv_data1.clear();
        m_trecv_data2.clear();
      }

      size_t size;
      s >> size;
      for(size_t i=0; i<size; ++i) {
        session_ptr s(new Session(s));
        m_closed[s->getID()] = s;
      }
      s >> m_delay;

      {
        boost::recursive_mutex::scoped_lock lock(m_recv_mutex);
        s >> size;
        for(size_t i=0; i<size; ++i) {
          m_recv_data.push_back(NMessage(s));
        }
      }
    }

    void write(ist::gzbstream& s)
    {
      session_cont tmp;
      tmp.insert(m_session.begin(), m_session.end());
      tmp.insert(m_closed.begin(), m_closed.end());

      s << tmp.size();
      for(session_cont::iterator p=tmp.begin(); p!=tmp.end(); ++p) {
        p->second->write(s);
      }
      s << m_replay_mes.size();
      for(size_t i=0; i<m_replay_mes.size(); ++i) {
        m_replay_mes[i].serialize(s);
      }
    }


    int getDelay() { return m_delay; }
    size_t getSessionID() { return m_session_id; }

    virtual session_ptr getSessionByID(size_t id)
    {
      if(session_ptr s = Super::getSessionByID(id)) {
        return s;
      }
      session_cont::iterator p = m_closed.find(id);
      if(p==m_closed.end()) {
        return session_ptr();
      }
      return p->second;
    }

    void stop() // sync 
    {
      m_stopped = true;
    }

    void run()
    {
      for(size_t i=0; i<m_start_data.size(); ++i) {
        dispatch(m_start_data[i]);
      }
      m_start_data.clear();
      Super::run();
    }

    void connect(const string& host, ushort port)
    {
      m_socket.reset(new tcp::iostream(host, lexical_cast<string>(port)));
      if(!*m_socket) {
        throw NetworkError("connection failed");
      }
      m_socket->rdbuf()->set_option(tcp::no_delay(true));

      // 参加要求送信 
      {
        IConfig& conf = *GetConfig();
        nmessage_cont sm;
        sm.push_back(NMessage::Entry(0, conf.scorename, conf.color));
        SendNetworkMessage(*m_socket, sm);
      }
      // ping確認フェーズ 
      {
        nmessage_cont mc;
        RecvNetworkMessage(*m_socket, m_start_data);
        for(size_t i=0; i<m_start_data.size(); ++i) {
          NMessage& m = m_start_data[i];
          if(m.type==NMessage::REJECT) {
            throw NetworkError(string("rejected: ")+m.reject.text);
          }
        }
        SendNetworkMessage(*m_socket, mc);
      }
      {
        RecvNetworkMessage(*m_socket, m_start_data);
        for(size_t i=0; i<m_start_data.size(); ++i) {
          NMessage& m = m_start_data[i];
          if(m.type==NMessage::ACCEPT) {
            m_session_id = m.sid;
          }
          else if(m.type==NMessage::REJECT) {
            throw NetworkError(string("rejected: ")+m.reject.text);
          }
        }
      }
      if(m_session_id==0) {
        throw NetworkError(string("protocol error"));
      }
      m_server_http_address = string("http://")+host+":"+lexical_cast<string>(port)+"/";
    }


    void push(const NMessage& t) // sync 
    {
      if(t.type==NMessage::INPUT && !findSession(getSessionID())) {
        return;
      }
      boost::mutex::scoped_lock lock(m_send_mutex);
      m_send_data.push_back(t);
    }


    struct closer
    {
      std::vector<int> closed;
      void selectClosed(nmessage_cont& nc)
      {
        for(size_t i=0; i<nc.size(); ++i) {
          if(nc[i].type==NMessage::CLOSE) {
            closed.push_back(nc[i].sid);
          }
        }
      }
      bool operator()(NMessage& m) {
        return m.type==NMessage::ENTRY && std::find(closed.begin(), closed.end(), m.sid)!=closed.end();
      }
    };

    void flush() // sync 
    {
      {
        boost::recursive_mutex::scoped_lock lock(m_recv_mutex);
        m_recv_data.insert(m_recv_data.end(), m_trecv_data2.begin(), m_trecv_data2.end());
        m_trecv_data2.clear();
      }

      closer closed;
      closed.selectClosed(m_recv_data);
      m_recv_data.erase(std::remove_if(m_recv_data.begin(), m_recv_data.end(), handler(this)), m_recv_data.end());
      m_recv_data.erase(std::remove_if(m_recv_data.begin(), m_recv_data.end(), closed), m_recv_data.end());

      if(m_request_state) {
        m_request_state = false;
        Serializer s;
        ist::bbuffer *cs = new ist::bbuffer();
        if(IGame *game = GetGame()) {
          game->serialize(s);
          ist::compress(s, *cs);
          push(NMessage::State(cs, s.size(), game->getPast()));
        }
        else {
          push(NMessage::State(cs, s.size(), 0));
        }
      }
    }


    bool needSync()
    {
      if(!GetGame() || GetGame()->isPaused() || !isRunning()) {
        return false;
      }
      for(session_cont::iterator p=m_session.begin(); p!=m_session.end(); ++p) {
        InputStream& input = *p->second->getInput();
        if(input.getIndex()>=input.getLength()) {
          return true;
        }
      }
      return false;
    }

    void sync()
    {
      flush();

      // 自動ディレイ調整有効の場合、同期待ちの頻度を見てdelayを調節する 
      IGame *game = GetGame();
      if(GetConfig()->server_autodelay && (game && !game->isPaused())) {
        bool delayed = false;
        bool overwork = false;
        size_t ping = 0;
        for(session_cont::iterator p=m_session.begin(); p!=m_session.end(); ++p) {
          session_ptr s = p->second;
          ping = std::max<size_t>(ping, s->getPing());
          if(s->getElapsed()<16 || s->getElapsed()>17) {
            overwork = true;
          }
          InputStream& input = *s->getInput();
          if(input.getIndex()>=input.getLength()) { // 同期待ちが必要 
            delayed = true;
          }
        }

        if(delayed) {
          if(ping>=m_delay*5) {
            m_wait_count+=15;
            m_nowait_count = std::max<int>(0, m_nowait_count-5);
          }
          else if(overwork) {
            m_wait_count = std::max<int>(0, m_wait_count-1);
          }
          else {
            m_wait_count+=5;
          }
          if(m_wait_count>=100) {
            m_wait_count = 0;
            m_nowait_count = 0;
            push(NMessage::Delay(m_delay+1));
          }
        }
        else {
          m_nowait_count+=1;
          m_wait_count = std::max<int>(0, m_wait_count-1);
          if(m_nowait_count>200 && m_delay>1) {
            m_nowait_count = 0;
            m_wait_count = 90;
            push(NMessage::Delay(m_delay-1));
          }
        }
      }

      // 遅れてるpeerがいたら待つ 
      int time = sgui::GetTicks();
      while(needSync()) {
        sgui::Sleep(1);
        flush();
        if(sgui::GetTicks()-time>=10) {
          GetGame()->setSkip(true);
          break;
        }
      }
    }

  private:
    bool dispatch(const NMessage& m)
    {
      int t = m.type;
      if(t==NMessage::INPUT) {
        if(session_ptr s = findSession(m.sid)) {
          s->getInput()->push(m.input.input);
          s->setElapsed(m.input.elapsed);
        }
        else {
        //  throw Error("NMessage::INPUT");
        }
      }
      else if(t==NMessage::CSTAT) {
        if(session_ptr s = findSession(m.sid)) {
          s->setPing(m.cstat.ping);
        }
      }
      else if(t==NMessage::STATE) {
        if(m.state.size>0) {
          shared_ptr<Deserializer> s(new Deserializer());
          s->resize(m.state.size);
          ist::uncompress(*m.statep, *s);
          if(!GetGame()) {
            LoadState(*s);
            push(NMessage::Response("state restored"));
          }
          else {
            throw s;
          }
        }
      }
      else if(t==NMessage::ENTRY) {
        if(IGame *game = GetGame()) {
          if(m.entry.frame > game->getPast()) {
            return false;
          }
          else if(m.entry.frame < game->getPast()) {
            throw Error("NMessage::ENTRY");
          }
        }

        if(!findSession(m.sid)) {
          session_ptr s(new Session(m.sid, m.entry.name, vector4(m.entry.color), new InputStream()));
          m_session[m.sid] = s;
          PushChatText(string("# ")+s->getName()+string(" join"));
          if(IGame *game = GetGame()) {
            s->setBeginFrame(m.entry.frame);
            s->setJoinFrame(-1);
            for(int j=0; j<m_delay; ++j) {
              s->getInput()->push(0);
            }
          }
          else {
            GetSound("charge1.wav")->play(6);
            UpdateServerInfomation("");
          }
        }
      }
      else if(t==NMessage::JOIN) {
        if(IGame *game = GetGame()) {
          if(m.join.frame > game->getPast()) {
            return false;
          }
          else if(m.join.frame < game->getPast()) {
            throw Error("NMessage::JOIN");
          }
          else if(m.join.frame==game->getPast()) {
            if(session_ptr s=findSession(m.sid)) {
              s->setJoinFrame(m.join.frame);
              game->join(m.sid);
            }
          }
        }
      }
      else if(t==NMessage::LEAVE) {
        if(IGame *game = GetGame()) {
          if(m.leave.frame > game->getPast()) {
            return false;
          }
          else if(m.leave.frame < game->getPast()) {
            throw Error("NMessage::LEAVE");
          }
          else if(m.leave.frame==game->getPast()) {
            session_ptr s;
            session_cont::iterator i = m_closed.find(m.sid);
            if(i!=m_closed.end()) {
              s = i->second;
            }
            else {
              s = findSession(m.sid);
            }
            if(s) {
              game->leave(m.sid);
              s->setLeaveFrame(m.leave.frame);
            }
          }
        }
      }
      else if(t==NMessage::TEXT)  {
        string text = m.text.text;
        sgui::Color color(m.text.color);
        if(text.empty()) {
          return true;
        }
        if(session_ptr s = findSession(m.sid)) {
          PushChatText(string(s->getName())+": "+text, color);
        }
        else {
          PushChatText(text, color);
        }
      }
      else if(t==NMessage::PAUSE) {
        if(IGame *game = GetGame()) {
          if(m.pause.frame > game->getPast()) {
            return false;
          }
          else if(m.pause.frame<=game->getPast()) {
            Pause();
          }
        }
      }
      else if(t==NMessage::RESUME){
        if(IGame *game = GetGame()) {
          if(m.resume.frame > game->getPast()) {
            return false;
          }
          else if(m.pause.frame<=game->getPast()) {
            Resume();
          }
        }
      }
      else if(t==NMessage::START) {
        GameOption opt;
        m.start.toGameOption(opt);
        m_delay = opt.delay;
        for(session_cont::iterator i=m_session.begin(); i!=m_session.end(); ++i) {
          i->second->getInput()->resize(m_delay);
        }
        FadeToGame(opt);
      }
      else if(t==NMessage::END) {
        if(IGame *game = GetGame()) {
          game->setPause(true);
          FadeToTitle();
        }
        else {
          m_closed.clear();
          m_session.clear();
        }
      }
      else if(t==NMessage::CLOSE) {
        ReleaseWaitingForNewPlayer(m.sid);
        if(session_ptr s = findSession(m.sid)) {
          PushChatText(string("# ")+s->getName()+" disconnected");
          m_session.erase(s->getID());
          if(GetGame()) {
            m_closed[s->getID()] = s;
          }
          else {
            UpdateServerInfomation("");
          }
        }
      }
      else if(t==NMessage::DELAY) {
        if(m.delay.delay>20) { // 20以上は無視 
          return true;
        }
        int gap = m.delay.delay-m_delay;
        m_delay = m.delay.delay;
        if(GetGame()) {
          for(session_cont::iterator i=m_session.begin(); i!=m_session.end(); ++i) {
            i->second->getInput()->modify(gap);
          }
        }
      }
      else if(t==NMessage::GCONF) {
        return assignConfigMessage(m);
      }
      else if(t==NMessage::QUERY)   {
        string query(m.query.text);
        if(query=="state") {
          m_request_state = true;
        }
      }
      else if(t==NMessage::RESPONSE) {
        string mes = m.response.text;
        if(mes=="new player") {
          WaitForNewPlayer(m.sid);
        }
        else if(mes=="state restored") {
          ReleaseWaitingForNewPlayer(m.sid);
        }
      }
      else if(t==NMessage::RESYNC) {
        push(NMessage::Query("state"));
      }
      return true;
    }


    void exec()
    {
      try {
        while(!m_stopped) {
          send();
          recv();
        }
      }
      catch(...) {
        boost::recursive_mutex::scoped_lock lock(m_recv_mutex);
        m_recv_data.push_back(NMessage::Text("# connection closed"));
        m_recv_data.push_back(NMessage::End());
      }
      m_socket->close();
      m_server_http_address.clear();
    }

    void send() // sync 
    {
      boost::mutex::scoped_lock lock(m_send_mutex);
      SendNetworkMessage(*m_socket, m_send_data);
    //  printf("send() %d\n", m_send_data.size());
      m_send_data.clear();
    }

    void recv() // sync 
    {
      // 遅滞テスト用 
      /*
      int s_interval = 10;
      if(::time(0)%30<15) {
        s_interval = 40;
      }
      else {
        s_interval = 10;
      }
      sgui::Sleep(s_interval);
      */

      boost::mutex::scoped_lock lock(m_tmp_mutex);
      RecvNetworkMessage(*m_socket, m_trecv_data1);
      for(size_t i=0; i<m_trecv_data1.size(); ++i) {
        NMessage& m = m_trecv_data1[i];
        if(m.type==NMessage::END) {
          m_stopped = true;
        }
      }
      {
        boost::recursive_mutex::scoped_lock lock(m_recv_mutex);
        m_trecv_data2.insert(m_trecv_data2.end(), m_trecv_data1.begin(), m_trecv_data1.end());
      }
      m_trecv_data1.clear();
    }

    void browse()
    {
      OpenURL(m_server_http_address);
    }
  };
#endif // EXCEPTION_ENABLE_NETPLAY 

  typedef shared_ptr<IInputServer> iserver_ptr;
  typedef shared_ptr<IInputClient> iclient_ptr;

  extern iserver_ptr g_iserver;
  extern iclient_ptr g_iclient;

  IInputClient* GetInputClient();
  void SendNMessage(const NMessage& t);
  size_t GetSessionCount();
  session_ptr GetSession(size_t i);
  session_ptr GetSessionByID(size_t i);

}
#endif
