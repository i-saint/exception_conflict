#include "../interface.h"

namespace exception {

  obj_ptr CreateTitleBackground();

  rule_ptr CreateDeathMatch(int time);
  rule_ptr CreateTeamFortress(float core_life);
  rule_ptr CreateHorde(int wave);

  gobj_ptr CreateGameOver();
  gobj_ptr CreateStageResult(int bosstime, int hit, float score);


  void PutSmallExplode(const vector4& pos, int num, float strength=1.5f);
  void PutSmallExplode(const box& box, const matrix44& mat, int num, float strength=1.5f);
  void PutFlash(const vector4& pos, float size=100.0f);
  void PutCubeExplode(const vector4& pos);
  void PutSmallImpact(const vector4& pos);
  void PutMediumImpact(const vector4& pos);
  void PutBloom();
  void PutDistortion(const vector4& pos, const vector4& dir);

  gobj_ptr GetGlobals();
  team_ptr CreateTeam();
  player_ptr CreatePlayer(ISession& s);
  fraction_ptr CreateFraction();


  void ResetGlobals();
  void SetGlobalScroll(const vector4& v);
  void SetGlobalAccel(const vector4& v);
  void SetGlobalMatrix(const matrix44& v);
  void SetGlobalBoundBox(const box& v);
  void SetGlobalBoundRect(const rect& v);
  void SetGlobalFractionBoost(float v);
  void SetGlobalPlayerBound(const box& v);

  const vector4& GetGlobalScroll();
  const vector4& GetGlobalAccel();
  const matrix44& GetGlobalMatrix();
  const matrix44& GetGlobalIMatrix();
  const box& GetGlobalBoundBox();
  const rect& GetGlobalBoundRect();
  float GetGlobalFractionBoost();
  const box& GetGlobalPlayerBound();

};
