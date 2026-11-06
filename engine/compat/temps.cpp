//---------------------------------------------------------------------------
// compat/temps.cpp - Portage de Centre-Ville hors de C++Builder
//---------------------------------------------------------------------------
#include "temps.h"
#include <chrono>
#include <ctime>
//---------------------------------------------------------------------------
namespace
 {
  // Décompose l'instant courant en heure locale + millisecondes
  std::tm HeureLocale(int &Millisecondes)
   {
    using namespace std::chrono;
    const system_clock::time_point Maintenant=system_clock::now();
    std::time_t t=system_clock::to_time_t(Maintenant);
    Millisecondes=static_cast<int>(duration_cast<milliseconds>(Maintenant.time_since_epoch()).count()%1000);
    if (Millisecondes<0) { Millisecondes+=1000;
}
    std::tm Local{};
#ifdef _WIN32
    localtime_s(&Local, &t);
#else
    localtime_r(&t, &Local);
#endif
    return Local;
   }
  //-------------------------------------------------------------------------
  double FractionDeJour(const std::tm &Local, int Millisecondes)
   {
    return ((((Local.tm_hour*60.0)+Local.tm_min)*60.0)+Local.tm_sec+(Millisecondes/1000.0))/86400.0;
   }
 }
//---------------------------------------------------------------------------
TDateTime Time()
 {
  int ms = 0;
  std::tm Local=HeureLocale(ms);
  return {FractionDeJour(Local, ms)};
 }
//---------------------------------------------------------------------------
