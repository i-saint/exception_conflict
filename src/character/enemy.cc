#include "stdafx.h"
#include "creater.h"
#include "enemy_util.h"
#include "draw.h"
#include "enemy.h"
#include "effect.h"
#include "bullet.h"
#include "ground.h"
#include "player.h"

#include "enemy_fraction.h"
#include "enemy_missile.h"
#include "enemy_bolt.h"
#include "enemy_zab.h"
#include "enemy_block.h"
#include "enemy_fighter.h"
#include "enemy_egg.h"
#include "enemy_shell.h"
#include "enemy_turret.h"
#include "enemy_hatch.h"
#include "enemy_carrier.h"
#include "enemy_gear.h"
#include "enemy_heavyfighter.h"
#include "enemy_armorship.h"
#include "enemy_weakiterator.h"

#include "stage_common.h"
#include "stage_title.h"
#include "rule_base.h"
#include "rule_deathmatch.h"
#include "rule_team_fortress.h"
#include "rule_horde.h"

namespace exception {


  class Globals : public LayerBase
  {
  typedef LayerBase Super;
  private:
    static Globals *s_inst;
    vector4 m_accel;
    vector4 m_scroll;
    matrix44 m_mat;
    matrix44 m_imat;
    box m_bound_box;
    rect m_bound_rect;
    float m_fraction_boost;
    box m_player_bound;

  public:
    static Globals* instance()
    {
      if(!s_inst) {
        s_inst = new Globals();
      }
      return s_inst;
    }

    Globals() : m_fraction_boost(1.0f)
    {
      chain(); // ŸŽè‚ÉŠJ•ú‚³‚ê‚È‚¢‚æ‚¤‚É 
      resetGlobals();
    }

    Globals(Deserializer& s) : Super(s)
    {
      s_inst = this;
      s >> m_accel >> m_scroll >> m_mat >> m_imat >> m_bound_box >> m_bound_rect >> m_fraction_boost >> m_player_bound;
    }

    ~Globals()
    {
      s_inst = 0;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_accel << m_scroll << m_mat << m_imat << m_bound_box << m_bound_rect << m_fraction_boost << m_player_bound;
    }

    const vector4& getPosition()
    {
      static vector4 pos;
      pos = getGlobalMatrix()*vector4();
      return pos;
    }

    void resetGlobals()
    {
      m_accel = vector4(-0.01f, 0, 0, 0);
      m_scroll = vector4(0.0f, 0, 0, 0);
      m_mat = matrix44();
      m_imat = matrix44();
      m_bound_box = box(vector4(500)); // 384, 288 
      m_bound_rect = rect(vector2(300));
      m_fraction_boost = 1.0f;
      m_player_bound = box(vector4(384, 288, 1));
    }

    void setGlobalScroll(const vector4& v) { m_scroll=v; m_scroll.w=0; }
    void setGlobalAccel(const vector4& v)  { m_accel=v; m_accel.w=0; }
    void setGlobalMatrix(const matrix44& v){ m_mat=v; m_imat=matrix44(v).invert(); }
    void setGlobalBoundBox(const box& v)   { m_bound_box=v; }
    void setGlobalBoundRect(const rect& v) { m_bound_rect=v; }
    void setGlobalFractionBoost(float v)   { m_fraction_boost=v; }
    void setGlobalPlayerBound(const box& v){ m_player_bound=v; }

    const vector4& getGlobalScroll()  { return m_scroll; }
    const vector4& getGlobalAccel()   { return m_accel; }
    const matrix44& getGlobalMatrix() { return m_mat; }
    const matrix44& getGlobalIMatrix(){ return m_imat; }
    const box& getGlobalBoundBox()    { return m_bound_box; }
    const rect& getGlobalBoundRect()  { return m_bound_rect; }
    float getGlobalFractionBoost()    { return m_fraction_boost; }
    const box& getGlobalPlayerBound() { return m_player_bound; }

    void onDestroy(DestroyMessage& m)
    {
      // DestroyMessageˆ¬‚è‚Â‚Ô‚µ 
    }
  };
  Globals* Globals::s_inst = 0;


  gobj_ptr GetGlobals() { return Globals::instance(); }

  void ResetGlobals() { Globals::instance()->resetGlobals(); }
  void SetGlobalScroll(const vector4& v) { Globals::instance()->setGlobalScroll(v); }
  void SetGlobalAccel(const vector4& v)  { Globals::instance()->setGlobalAccel(v); }
  void SetGlobalMatrix(const matrix44& v){ Globals::instance()->setGlobalMatrix(v); }
  void SetGlobalBoundBox(const box& v)   { Globals::instance()->setGlobalBoundBox(v); }
  void SetGlobalBoundRect(const rect& v) { Globals::instance()->setGlobalBoundRect(v); }
  void SetGlobalFractionBoost(float v)   { Globals::instance()->setGlobalFractionBoost(v); }
  void SetGlobalPlayerBound(const box& v){ Globals::instance()->setGlobalPlayerBound(v); }

  const vector4& GetGlobalScroll()  { return Globals::instance()->getGlobalScroll(); }
  const vector4& GetGlobalAccel()   { return Globals::instance()->getGlobalAccel(); }
  const matrix44& GetGlobalMatrix() { return Globals::instance()->getGlobalMatrix(); }
  const matrix44& GetGlobalIMatrix(){ return Globals::instance()->getGlobalIMatrix(); }
  const box& GetGlobalBoundBox()    { return Globals::instance()->getGlobalBoundBox(); }
  const rect& GetGlobalBoundRect()  { return Globals::instance()->getGlobalBoundRect(); }
  float GetGlobalFractionBoost()    { return Globals::instance()->getGlobalFractionBoost(); }
  const box& GetGlobalPlayerBound() { return Globals::instance()->getGlobalPlayerBound(); }



  BlueBlur* BlueBlur::s_inst;


  namespace impact {
    std::map<gid, float> g_opacity;
  }



  team_ptr CreateTeam()
  {
    return new Team();
  }

  player_ptr CreatePlayer(ISession& ses)
  {
    BlueBlur::instance();
    return new Player(ses, new Player_Controler(ses.getInput()));
  }


  fraction_ptr CreateFraction()
  {
    return Fraction::Factory::create();
  }
}



namespace exception {

  obj_ptr CreateTitleBackground()   { return new title::Background(); }

  rule_ptr CreateDeathMatch(int time)     { return new rule::deathmatch::Rule(time); }
  rule_ptr CreateTeamFortress(float life) { return new rule::team_fortress::Rule(life); }
  rule_ptr CreateHorde(int wave)          { return new rule::horde::Rule(wave); }


namespace impl {
  template<class T>
  inline bool deserialize_object(const std::string& name, Deserializer& s, gobj_ptr& r)
  {
    if(name==typeid(T).name()) {
      r = new T(s);
      return true;
    }
    return false;
  }

  template<class T>
  inline bool deserialize_object_cached(const std::string& name, Deserializer& s, gobj_ptr& r)
  {
    if(name==typeid(T).name()) {
      r = T::Factory::create(s);
      return true;
    }
    return false;
  }

  template<class T>
  inline bool deserialize_controler(const std::string& name, Deserializer& s, controler_ptr& r)
  {
    if(name==typeid(T).name()) {
      r = new T(s);
      return true;
    }
    return false;
  }

  inline bool throw_exception(const string& message) {
    throw Error(message);
    return false;
  }
}

  void SerializeObject(Serializer& s, const gobj_ptr p)
  {
    s << string(typeid(*p).name());
    p->serialize(s);
  }

  gobj_ptr DeserializeObject(Deserializer& s)
  {
    gobj_ptr r = 0;
    string name;
    s >> name;

#ifdef EXCEPTION_ENABLE_RUNTIME_CHECK
    printf("%s ", name.c_str());
    fflush(stdout);
#endif // EXCEPTION_ENABLE_RUNTIME_CHECK 

    impl::deserialize_object_cached<Fraction>(name, s, r) ||

    impl::deserialize_object<Player>(name, s, r) ||

    impl::deserialize_object<Globals>(name, s, r)       ||
    impl::deserialize_object<LayerBase>(name, s, r)     ||
    impl::deserialize_object<ChildLayer>(name, s, r)    ||
    impl::deserialize_object<Layer>(name, s, r)         ||
    impl::deserialize_object<RotLayer>(name, s, r)      ||
    impl::deserialize_object<ChildRotLayer>(name, s, r) ||
    impl::deserialize_object<ScrolledLayer>(name, s, r) ||

    impl::deserialize_object<BoxModel>(name, s, r) ||

    impl::deserialize_object<MediumBlock>(name, s, r)    ||
    impl::deserialize_object<LargeBlock>(name, s, r)     ||
    impl::deserialize_object<CrashGround>(name, s, r)    ||
    impl::deserialize_object<CrashGround>(name, s, r)    ||
    impl::deserialize_object<BigCrashGround>(name, s, r) ||
    impl::deserialize_object<DynamicBlock>(name, s, r)   ||
    impl::deserialize_object<PillerBlock>(name, s, r)    ||
    impl::deserialize_object<StaticGround>(name, s, r)   ||
    impl::deserialize_object<DynamicGround>(name, s, r)  ||
    impl::deserialize_object<CoverGround>(name, s, r)    ||

    impl::deserialize_object<Fighter>(name, s, r)        ||
    impl::deserialize_object<Fighter::Parts>(name, s, r) ||

    impl::deserialize_object<Shell>(name, s, r)             ||
    impl::deserialize_object<Shell::Parts>(name, s, r)      ||
    impl::deserialize_object<Shell::DummyParts>(name, s, r) ||

    impl::deserialize_object<LargeCarrier>(name, s, r) ||

    impl::deserialize_object<LargeHatch>(name, s, r) ||
    impl::deserialize_object<SmallHatch>(name, s, r) ||

    impl::deserialize_object<HeavyFighter>(name, s, r)        ||
    impl::deserialize_object<HeavyFighter::Parts>(name, s, r) ||
    impl::deserialize_object<HeavyFighter::Arm>(name, s, r)   ||

    impl::deserialize_object<ArmorShip::Core>(name, s, r)  ||
    impl::deserialize_object<ArmorShip::Parts>(name, s, r) ||
    impl::deserialize_object<ArmorShip::Fort>(name, s, r)  ||
    impl::deserialize_object<Turtle>(name, s, r)           ||

    impl::deserialize_object<Bolt>(name, s, r) ||

    impl::deserialize_object<Egg>(name, s, r)        ||
    impl::deserialize_object<Egg::Parts>(name, s, r) ||

    impl::deserialize_object<Zab>(name, s, r) ||
    impl::deserialize_object<Zab::Parts>(name, s, r) ||

    impl::deserialize_object<LaserTurret>(name, s, r)        ||
    impl::deserialize_object<LaserTurret::Parts>(name, s, r) ||

    impl::deserialize_object<FloatingTurret>(name, s, r)            ||
    impl::deserialize_object<FloatingTurret::HeadParts>(name, s, r) ||

    impl::deserialize_object<WeakIterator>(name, s, r)        ||
    impl::deserialize_object<WeakIterator::Guard>(name, s, r) ||

    impl::deserialize_object<GravityMine>(name, s, r)    ||
    impl::deserialize_object<BurstMine>(name, s, r)      ||
    impl::deserialize_object<MiniBurstMine>(name, s, r)  ||
    impl::deserialize_object<GravityMissile>(name, s, r) ||
    impl::deserialize_object<BurstMissile>(name, s, r)   ||

    impl::deserialize_object<Gravity>(name, s, r)   ||
    impl::deserialize_object<MineBurst>(name, s, r) ||

    impl::deserialize_object<ChildGround>(name, s, r) ||
    impl::deserialize_object<Ground>(name, s, r)      ||
    impl::deserialize_object<MoveGround>(name, s, r)  ||

    impl::deserialize_object<GearParts>(name, s, r) ||
    impl::deserialize_object<SmallGear>(name, s, r) ||
    impl::deserialize_object<LargeGear>(name, s, r) ||
    impl::deserialize_object<LaserAmp>(name, s, r)  ||
    impl::deserialize_object<LaserGear>(name, s, r) ||

    impl::deserialize_object<BlueBlur>(name, s, r)    ||
    impl::deserialize_object<Ray>(name, s, r)         ||
    impl::deserialize_object<GLaser>(name, s, r)      ||
    impl::deserialize_object<Laser>(name, s, r)       ||
    impl::deserialize_object<LaserBit>(name, s, r)    ||
    impl::deserialize_object<PararelLaser>(name, s, r)||
    impl::deserialize_object<RedRay>(name, s, r)      ||
    impl::deserialize_object<RedRayBit>(name, s, r)   ||
    impl::deserialize_object<Blaster>(name, s, r)     ||

    impl::deserialize_object<Player::Drawer>(name, s, r)       ||
    impl::deserialize_object<Fraction::Drawer>(name, s, r)     ||
    impl::deserialize_object<LockOnMarker::Drawer>(name, s, r) ||
    impl::deserialize_object<Flash::Drawer>(name, s, r)        ||
    impl::deserialize_object<RedRing::Drawer>(name, s, r)      ||
    impl::deserialize_object<BlueRing::Drawer>(name, s, r)     ||
    impl::deserialize_object<Explode::Drawer>(name, s, r)      ||
    impl::deserialize_object<CubeExplode::Drawer>(name, s, r)  ||
    impl::deserialize_object<BlueParticle::Drawer>(name, s, r) ||

    impl::deserialize_object<DirectionalImpact>(name, s, r) ||
    impl::deserialize_object<SmallImpact>(name, s, r)       ||
    impl::deserialize_object<MediumImpact>(name, s, r)      ||
    impl::deserialize_object<BigImpact>(name, s, r)         ||
    impl::deserialize_object<BossDestroyImpact>(name, s, r) ||
    impl::deserialize_object<Shine>(name, s, r)       ||
    impl::deserialize_object<DamageFlash>(name, s, r) ||
    impl::deserialize_object<Bloom>(name, s, r)       ||


    impl::deserialize_object<Team>(name, s, r)    ||

    impl::deserialize_object<rule::MediumBlockGround>(name, s, r)     ||
    impl::deserialize_object<rule::LargeBlockGround>(name, s, r)      ||
    impl::deserialize_object<rule::MediumSpinningGround>(name, s, r)  ||
    impl::deserialize_object<rule::LargeSpinningGround>(name, s, r)   ||
    impl::deserialize_object<rule::Ground_Enemy4_Linkage>(name, s, r) ||

    impl::deserialize_object<rule::horde::Background>(name, s, r)   ||
    impl::deserialize_object<rule::horde::Rule>(name, s, r)         ||
    impl::deserialize_object<rule::horde::Wave1>(name, s, r)        ||
    impl::deserialize_object<rule::horde::Wave2>(name, s, r)        ||
    impl::deserialize_object<rule::horde::Wave3>(name, s, r)        ||
    impl::deserialize_object<rule::horde::Wave4>(name, s, r)        ||
    impl::deserialize_object<rule::horde::Wave5>(name, s, r)        ||
    impl::deserialize_object<rule::horde::Wave6>(name, s, r)        ||
    impl::deserialize_object<rule::horde::Wave7>(name, s, r)        ||
    impl::deserialize_object<rule::horde::Wave8>(name, s, r)        ||
    impl::deserialize_object<rule::horde::Wave9>(name, s, r)        ||
    impl::deserialize_object<rule::horde::Wave10>(name, s, r)       ||
    impl::deserialize_object<rule::horde::Wave11>(name, s, r)       ||
    impl::deserialize_object<rule::horde::Wave12>(name, s, r)       ||
    impl::deserialize_object<rule::horde::Wave13>(name, s, r)       ||
    impl::deserialize_object<rule::horde::Wave14>(name, s, r)       ||
    impl::deserialize_object<rule::horde::Wave15>(name, s, r)       ||
    impl::deserialize_object<rule::horde::WaveAnnounce>(name, s, r) ||
    impl::deserialize_object<rule::horde::FadeOut>(name, s, r)      ||
    impl::deserialize_object<rule::horde::Result>(name, s, r)       ||

    impl::deserialize_object<rule::deathmatch::Background>(name, s, r) ||
    impl::deserialize_object<rule::deathmatch::Rule>(name, s, r)       ||
    impl::deserialize_object<rule::deathmatch::Wave1>(name, s, r)      ||
    impl::deserialize_object<rule::deathmatch::Black>(name, s, r)      ||
    impl::deserialize_object<rule::deathmatch::Result>(name, s, r)     ||

    impl::deserialize_object<rule::team_fortress::Background>(name, s, r)        ||
    impl::deserialize_object<rule::team_fortress::Rule>(name, s, r)              ||
    impl::deserialize_object<rule::team_fortress::Wave1>(name, s, r)             ||
    impl::deserialize_object<rule::team_fortress::Fortress::Parts>(name, s, r) ||
    impl::deserialize_object<rule::team_fortress::Fortress::Core>(name, s, r)  ||
    impl::deserialize_object<rule::team_fortress::Fortress>(name, s, r)        ||
    impl::deserialize_object<rule::team_fortress::Black>(name, s, r)             ||
    impl::deserialize_object<rule::team_fortress::Result>(name, s, r)            ||


    impl::throw_exception("DeserializeObject() : "+name);

#ifdef EXCEPTION_ENABLE_RUNTIME_CHECK
    printf("(%d)\n", r->getID());
    fflush(stdout);
#endif // EXCEPTION_ENABLE_RUNTIME_CHECK 

    return r;
  }

  void SerializeControler(Serializer& s, const controler_ptr p)
  {
    if(p) {
      s << string(typeid(*p).name());
      p->serialize(s);
    }
    else {
      s << string("null");
    }
  }

  controler_ptr DeserializeControler(Deserializer& s)
  {
    controler_ptr r = 0;
    string name;
    s >> name;

#ifdef EXCEPTION_ENABLE_RUNTIME_CHECK
    printf("& %s ", name.c_str());
    fflush(stdout);
#endif // EXCEPTION_ENABLE_RUNTIME_CHECK 

    if(name=="null") { return 0; }

    impl::deserialize_controler<Player_Controler>(name, s, r) ||

    impl::deserialize_controler<Fighter_Rush>(name, s, r)           ||
    impl::deserialize_controler<Fighter_Straight>(name, s, r)       ||

    impl::deserialize_controler<Shell_Blaster>(name, s, r)        ||
    impl::deserialize_controler<Shell_BurstMissile>(name, s, r)   ||
    impl::deserialize_controler<Shell_GravityMissile>(name, s, r) ||

    impl::deserialize_controler<LargeCarrier_GenFighter>(name, s, r)      ||
    impl::deserialize_controler<LargeCarrier_GenMissileShell>(name, s, r) ||

    impl::deserialize_controler<Hatch_GenRushFighter>(name, s, r)       ||
    impl::deserialize_controler<Hatch_GenMissileShell>(name, s, r)      ||
    impl::deserialize_controler<Hatch_Bolt>(name, s, r)                 ||
    impl::deserialize_controler<Hatch_Bolt2>(name, s, r)                ||
    impl::deserialize_controler<Hatch_MiniMine>(name, s, r)             ||
    impl::deserialize_controler<Hatch_Laser>(name, s, r)                ||

    impl::deserialize_controler<HeavyFighter_Straight>(name, s, r)         ||
    impl::deserialize_controler<HeavyFighter_Missiles>(name, s, r)         ||
    impl::deserialize_controler<HeavyFighter_Missiles2>(name, s, r)        ||
    impl::deserialize_controler<HeavyFighter_PutMines>(name, s, r)         ||

    impl::deserialize_controler<Turtle_Wait>(name, s, r)    ||
    impl::deserialize_controler<Turtle_Sliding>(name, s, r) ||

    impl::deserialize_controler<Bolt_Straight>(name, s, r) ||
    impl::deserialize_controler<Bolt_Rush>(name, s, r)     ||

    impl::deserialize_controler<Egg_Mine>(name, s, r)    ||
    impl::deserialize_controler<Egg_Missile>(name, s, r) ||
    impl::deserialize_controler<Egg_Laser>(name, s, r)   ||

    impl::deserialize_controler<Zab_Rush>(name, s, r)     ||
    impl::deserialize_controler<Zab_Straight>(name, s, r) ||

    impl::deserialize_controler<LaserTurret_Normal>(name, s, r) ||

    impl::deserialize_controler<FloatingTurret_Wait>(name, s, r) ||

    impl::deserialize_controler<WeakIterator_Defense>(name, s, r)          ||

    impl::throw_exception(string("DeserializeControler() : ")+name);

    return r;
  }
}
