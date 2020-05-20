#include "debughud.h"
#include "iomod.h"
#include "rendercontext.h"
#include "gameconfig.h"

DebugHUD::DebugHUD() :
  messages(), visible(false),
  tr(), tg(), tb(), ta(),
  br(), bg(), bb(), ba(),
  lr(), lg(), lb(), la()
{
  const XMLTag &debug = GameConfig::getInstance()["debug"];
  tr = debug["textColor"]["r"].toInt();
  tg = debug["textColor"]["g"].toInt();
  tb = debug["textColor"]["b"].toInt();
  ta = debug["textColor"]["a"].toInt();
  br = debug["backColor"]["r"].toInt();
  bg = debug["backColor"]["g"].toInt();
  bb = debug["backColor"]["b"].toInt();
  ba = debug["backColor"]["a"].toInt();
  lr = debug["outlineColor"]["r"].toInt();
  lg = debug["outlineColor"]["g"].toInt();
  lb = debug["outlineColor"]["b"].toInt();
  la = debug["outlineColor"]["a"].toInt();
}

void DebugHUD::draw() const
{
  if (!visible) return;

  int count = messages.rbegin()->first + 1;
  
  SDL_Renderer *renderer = RenderContext::getInstance().getRenderer();

  SDL_Rect area = { 15, 35, 600, static_cast<int>(30 + count * 30) };
  
  SDL_SetRenderDrawBlendMode( renderer, SDL_BLENDMODE_BLEND );
  SDL_SetRenderDrawColor( renderer, br, bg, bb, ba );
  SDL_RenderFillRect( renderer, &area );
  SDL_SetRenderDrawColor( renderer, lr, lg, lb, la );
  SDL_RenderDrawRect( renderer, &area );
  
  for (auto &p : messages)
    IoMod::getInstance().writeText(p.second, 30, 50 + p.first * 30, {(Uint8)tr, (Uint8)tg, (Uint8)tb, (Uint8)ta} );
}
