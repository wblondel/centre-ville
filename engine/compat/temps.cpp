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
    system_clock::time_point Maintenant=system_clock::now();
    std::time_t t=system_clock::to_time_t(Maintenant);
    Millisecondes=(int)(duration_cast<milliseconds>(Maintenant.time_since_epoch()).count()%1000);
    if (Millisecondes<0) Millisecondes+=1000;
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
    return ((Local.tm_hour*60.0+Local.tm_min)*60.0+Local.tm_sec+Millisecondes/1000.0)/86400.0;
   }
 }
//---------------------------------------------------------------------------
TDateTime Time()
 {
  int ms;
  std::tm Local=HeureLocale(ms);
  return TDateTime(FractionDeJour(Local, ms));
 }
//---------------------------------------------------------------------------
TDateTime Now()
 {
  int ms;
  std::tm Local=HeureLocale(ms);
  // Nombre de jours civils depuis le 30/12/1899 (origine de TDateTime)
  int a=Local.tm_year+1900, m=Local.tm_mon+1, j=Local.tm_mday;
  // Algorithme « days from civil » (H. Hinnant) : jours depuis le 01/01/1970
  a-=(m<=2);
  int Ere=(a>=0?a:a-399)/400;
  unsigned AnDeLEre=(unsigned)(a-Ere*400);
  unsigned JourDeLAn=(153u*(unsigned)(m+(m>2?-3:9))+2u)/5u+(unsigned)j-1u;
  unsigned JourDeLEre=AnDeLEre*365u+AnDeLEre/4u-AnDeLEre/100u+JourDeLAn;
  long long Jours1970=(long long)Ere*146097LL+(long long)JourDeLEre-719468LL;
  return TDateTime((double)(Jours1970+25569LL)+FractionDeJour(Local, ms));
 }
//---------------------------------------------------------------------------
