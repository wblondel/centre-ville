//---------------------------------------------------------------------------
// test_corrections.cpp - Bogues d'origine du moteur corrigés en 6.0.1
//
// Chaque cas échoue (ou, pour le blocage des vehlibs, ne se termine pas : le
// délai de CTest le fait échouer) sur le moteur 6.0.0, identique à l'original.
//---------------------------------------------------------------------------
#include <catch2/catch_test_macros.hpp>
#include <string>
#include "cartes.h"
#include "harnais.h"
#include "vue.h"
#include "compat/alea.h"
//---------------------------------------------------------------------------
namespace
 {
  // Une place que le vehlib n°NumVehlib pourrait réserver (libre, autre que sa place de départ)
  bool ExistePlaceReservable(int NumVehlib)
   {
    for(int p=0; p<cv->NbPlacesVehlib; p++)
     if ((p+1!=cv->Vehlib[NumVehlib-1].NumPlaceDepart)&&cv->PlaceVehlib[p].EstLibre())
      return true;
    return false;
   }
  //-------------------------------------------------------------------------
  // Nombre de piétons qui attendent le vehlib n°NumVehlib à sa place
  int NbPietonsQuiAttendent(int NumVehlib)
   {
    int n=0;
    for(int i=0; i<cv->NbPietons; i++)
     n+=cv->Pieton[i].AttendVehlib&&(cv->Pieton[i].NumPlaceVehlib==cv->Vehlib[NumVehlib-1].NumPlaceDepart);
    return n;
   }
 }
//---------------------------------------------------------------------------
TEST_CASE("Vehlibs : sans place libre, le vehlib attend au lieu de figer la simulation", "[corrections][vehlibs]")
 { // Géant : 29 vehlibs pour 32 places. Avec la graine 123, l'original se
   // figeait avant le tour 50 dans vehlib::ReservePlaceVehlibCible
  VueNulle Vue;
  vue=&Vue;
  alea::FixeGraine(123);
  REQUIRE(harnais::Ouvre(CheminCarte(Cartes[4])));
  AnsiString asMsgErr;
  REQUIRE(harnais::Demarre(asMsgErr));
  REQUIRE(cv->NbVehlibs==29);
  REQUIRE(cv->NbPlacesVehlib==32);
  int NbAttentesSansPlace=0;
  for(int t=1; t<=300; t++)
   {
    REQUIRE(harnais::Tick());
    for(int i=0; i<cv->NbVehlibs; i++)
     {
      const vehlib &vl=cv->Vehlib[i];
      INFO("Tour " << t << ", vehlib n°" << i+1);
      // Un vehlib ne part que vers une place réservée pour lui
      if (vl.Situation==svDepart) REQUIRE(vl.NumPlaceArrivee>0);
      if (vl.NumPlaceArrivee) REQUIRE(cv->PlaceVehlib[vl.NumPlaceArrivee-1].NumVehlibReserve==i+1);
      // Situation qui figeait l'original : des piétons attendent ce vehlib,
      // qui n'a pas de destination, et aucune place n'est libre. Il attend :
      // les piétons restent dehors, il réessaiera au tour suivant
      if ((vl.Situation==svAttente)&&(!vl.NumPlaceArrivee)&&NbPietonsQuiAttendent(i+1)&&!ExistePlaceReservable(i+1))
       NbAttentesSansPlace++;
     }
    for(int p=0; p<cv->NbPlacesVehlib; p++)
     if (int n=cv->PlaceVehlib[p].NumVehlibReserve)
      {
       INFO("Tour " << t << ", place de vehlib n°" << p+1);
       REQUIRE(cv->Vehlib[n-1].NumPlaceArrivee==p+1);
      }
   }
  CHECK(cv->TourCrt==300);
  CHECK(NbAttentesSansPlace>0); // le cas est bien exercé
  CHECK(Vue.Messages.empty());
  harnais::Ferme();
  alea::LibereGraine();
  vue=nullptr;
 }
//---------------------------------------------------------------------------
