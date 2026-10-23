//---------------------------------------------------------------------------
// test_corrections.cpp - Bogues d'origine du moteur corrigés en 6.0.1
//
// Chaque cas échoue (ou, pour le blocage des vehlibs, ne se termine pas : le
// délai de CTest le fait échouer) sur le moteur 6.0.0, identique à l'original.
//---------------------------------------------------------------------------
#include <catch2/catch_test_macros.hpp>
#include <string>
#ifndef _WIN32
 #include <csignal>
 #include <sys/resource.h>
#endif
#include "cartes.h"
#include "harnais.h"
#include "sortie.h"
#include "vue.h"
#include "compat/fichier.h"
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
TEST_CASE("Enregistrement : une erreur d'écriture fait échouer Sauve", "[corrections][sauvegarde]")
 { // L'original ignorait les erreurs d'écriture : FileWrite renvoie -1, vrai
   // dans les chaînes de &&, et voie::Sauve contenait un « ; » au lieu d'un
   // « && » ; compat/fichier ne voyait l'erreur qu'en vidant son tampon (tous
   // les 64 Ko ou à la fermeture)
  VueNulle Vue;
  vue=&Vue;
  for(int c : {0, 4}) // 8x8 (7 Ko, moins qu'un tampon) et Géant (470 Ko)
   {
    DYNAMIC_SECTION(Cartes[c].Nom)
     {
      REQUIRE(harnais::Ouvre(CheminCarte(Cartes[c])));
      // Fichier ouvert en lecture seule : aucune écriture n'aboutit
      const std::string asLectureSeule=FichierSortie(std::string("lecture_seule_")+Cartes[c].Nom+".cvw");
      int h=FileCreate(asLectureSeule.c_str());
      REQUIRE(h>0);
      FileClose(h);
      REQUIRE((h=FileOpen(asLectureSeule.c_str(), fmOpenRead))>0);
      CHECK_FALSE(cv->Sauve(h));
      FileClose(h);
      // Poignée invalide (FileCreate a échoué)
      CHECK_FALSE(cv->Sauve(-1));
#ifndef _WIN32
      // Taille de fichier limitée à 1000 octets, comme un disque plein :
      // l'écriture échoue en cours de route
      const std::string asLimite=FichierSortie(std::string("limite_")+Cartes[c].Nom+".cvw");
      struct rlimit Ancienne, Limite;
      REQUIRE(getrlimit(RLIMIT_FSIZE, &Ancienne)==0);
      Limite=Ancienne;
      Limite.rlim_cur=1000;
      void (*AncienSignal)(int)=std::signal(SIGXFSZ, SIG_IGN);
      REQUIRE(setrlimit(RLIMIT_FSIZE, &Limite)==0);
      const bool Enregistre=harnais::SauveSansChaos(asLimite);
      setrlimit(RLIMIT_FSIZE, &Ancienne);
      std::signal(SIGXFSZ, AncienSignal);
      CHECK_FALSE(Enregistre);
#endif
      // Sans erreur, l'enregistrement aboutit toujours
      CHECK(harnais::SauveSansChaos(FichierSortie(std::string("sans_erreur_")+Cartes[c].Nom+".cvw")));
      harnais::Ferme();
     }
   }
  CHECK(Vue.Messages.empty());
  vue=nullptr;
 }
//---------------------------------------------------------------------------
TEST_CASE("Copier vers... : PrepareCopie(FIN) remet toute la sélection à zéro", "[corrections][copie]")
 { // L'original remettait à zéro le paramètre NbY au lieu de NbYc
  VueNulle Vue;
  vue=&Vue;
  REQUIRE(harnais::Ouvre(CheminCarte(Cartes[0])));
  cv->PrepareCopie(DEBUT, 1, 2, 3, 4);
  CHECK(cv->CopieEnCours);
  CHECK(cv->Xc==1);
  CHECK(cv->Yc==2);
  CHECK(cv->NbXc==3);
  CHECK(cv->NbYc==4);
  cv->PrepareCopie(FIN, 0, 0, 0, 0);
  CHECK_FALSE(cv->CopieEnCours);
  CHECK(cv->Xc==0);
  CHECK(cv->Yc==0);
  CHECK(cv->NbXc==0);
  CHECK(cv->NbYc==0);
  harnais::Ferme();
  vue=nullptr;
 }
//---------------------------------------------------------------------------
