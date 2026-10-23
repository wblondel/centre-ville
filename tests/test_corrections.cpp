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
#include "format_reference.h"
#include "harnais.h"
#include "sortie.h"
#include "vue.h"
#include "compat/fichier.h"
#include "compat/alea.h"
//---------------------------------------------------------------------------
namespace
 {
  const champ_cvw &Champ(const conversion_cvw &c, const std::string &Nom)
   {
    for(const champ_cvw &ch : c.Champs)
     if (ch.Nom==Nom) return ch;
    FAIL("champ introuvable : " << Nom);
    return c.Champs.front();
   }
  //-------------------------------------------------------------------------
  int LitEntier(const std::string &o, const champ_cvw &ch)
   {
    unsigned u=0;
    for(int i=3; i>=0; i--) u=(u<<8)|(unsigned char)o[ch.Debut+(std::size_t)i];
    return (int)u;
   }
  //-------------------------------------------------------------------------
  // Champ de coordonnée d'un élément (feu1.x, feup2.y[1], depot.x...), hors cases
  bool EstCoordonnee(const std::string &Nom)
   {
    if (Nom.rfind("case(", 0)==0) return false;
    std::size_t p=Nom.rfind('.');
    if (p==std::string::npos) return false;
    std::string Fin=Nom.substr(p+1);
    return (Fin=="x")||(Fin=="y")||(Fin.rfind("x[", 0)==0)||(Fin.rfind("y[", 0)==0);
   }
  //-------------------------------------------------------------------------
  void EcritEntier(std::string &o, const champ_cvw &ch, int v)
   {
    for(int i=0; i<4; i++) o[ch.Debut+(std::size_t)i]=(char)(((unsigned)v>>(8*i))&0xFF);
   }
  //-------------------------------------------------------------------------
  void EcritFichier(const std::string &asChemin, const std::string &asContenu)
   {
    int h=FileCreate(asChemin.c_str());
    REQUIRE(h>0);
    REQUIRE(FileWrite(h, asContenu.data(), (unsigned)asContenu.size())==(int)asContenu.size());
    REQUIRE(FileFlush(h));
    FileClose(h);
   }
  //-------------------------------------------------------------------------
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
TEST_CASE("Copier vers... : un élément non défini reste non défini (-1) dans la copie", "[corrections][copie]")
 { // L'original enregistrait x-Xc et y-Yc même pour un élément non défini
   // (x=y=-1 : dépôt de bus absent, feu supprimé...), soit -2 avec une
   // sélection qui commence en (1,1), que la vérification d'intégrité signale
  VueNulle Vue;
  vue=&Vue;
  for(const carte &c : Cartes)
   {
    DYNAMIC_SECTION(c.Nom)
     {
      REQUIRE(harnais::Ouvre(CheminCarte(c)));
      const std::string asOrigine=FichierSortie(std::string("copie_indefinis_")+c.Nom+"_origine.cvw"),
                        asCopie=FichierSortie(std::string("copie_indefinis_")+c.Nom+".cvw");
      REQUIRE(harnais::SauveSansChaos(asOrigine));
      cv->PrepareCopie(DEBUT, 1, 1, cv->NbX-2, cv->NbY-2);
      REQUIRE(cv->CopieValide());
      REQUIRE(harnais::SauveSansChaos(asCopie));
      cv->PrepareCopie(FIN, 0, 0, 0, 0);
      harnais::Ferme();
      const std::string asOctetsOrigine=harnais::LitFichier(asOrigine), asOctetsCopie=harnais::LitFichier(asCopie);
      const conversion_cvw Origine=ConvertitEn530(asOctetsOrigine), Copie=ConvertitEn530(asOctetsCopie);
      REQUIRE(Origine.Ok);
      REQUIRE(Copie.Ok);
      int NbIndefinis=0;
      for(const champ_cvw &ch : Origine.Champs)
       if (EstCoordonnee(ch.Nom)&&(LitEntier(asOctetsOrigine, ch)<0))
        {
         NbIndefinis++;
         const champ_cvw *chc=nullptr;
         for(const champ_cvw &k : Copie.Champs) if (k.Nom==ch.Nom) { chc=&k; break; }
         REQUIRE(chc);
         INFO(ch.Nom);
         CHECK(LitEntier(asOctetsCopie, *chc)==-1);
        }
      CHECK(NbIndefinis>0); // chaque carte a des éléments non définis
     }
   }
  CHECK(Vue.Messages.empty());
  vue=nullptr;
 }
//---------------------------------------------------------------------------
TEST_CASE("Vérification d'intégrité : messages des contrôles de position, direction, priorité, état...", "[corrections][verifie]")
 { // VerifiePosition, VerifieDirection, VerifiePriorite, VerifieDirPoss,
   // VerifieEtat, VerifieDmdVert et VerifiePsgRouge recevaient asMsgErr par
   // valeur : leurs messages étaient perdus
  VueNulle Vue;
  vue=&Vue;
  std::string asCarte=harnais::LitFichier(CheminCarte(Cartes[4])); // Géant.cvw, format 0x530, 109 feux
  conversion_cvw Ref=ConvertitEn530(asCarte);
  REQUIRE(Ref.Ok);
  REQUIRE(Ref.Version==VERSION);
  const int NbY=LitEntier(asCarte, Champ(Ref, "NbY"));
  int f=1; // premier feu défini (les places libres, x=-1, ne sont pas vérifiées)
  while(LitEntier(asCarte, Champ(Ref, "feu"+std::to_string(f)+".x"))<0) f++;
  const std::string asFeu="feu"+std::to_string(f)+".";
  asCarte[Champ(Ref, asFeu+"Etat").Debut]=7;
  asCarte[Champ(Ref, asFeu+"TypeDmdVert").Debut]=9;
  asCarte[Champ(Ref, asFeu+"TypePsgRouge").Debut]=9;
  EcritEntier(asCarte, Champ(Ref, asFeu+"Dir"), 9);
  EcritEntier(asCarte, Champ(Ref, asFeu+"y"), NbY);
  const char DirPoss00=asCarte[Champ(Ref, "case(0,0).DirPoss").Debut];
  asCarte[Champ(Ref, "case(0,0).DirPoss").Debut]=20;
  asCarte[Champ(Ref, "case(0,0).Priorite[0]").Debut]=9;
  const std::string asFichier=FichierSortie("verifie_champs.cvw");
  EcritFichier(asFichier, asCarte);
  REQUIRE(harnais::Ouvre(asFichier));
  AnsiString asMsgErr;
  cv->Verifie(asMsgErr);
  const std::string asMessages=asMsgErr.str();
  INFO(asMessages);
  const std::string asSource="Feu n°"+std::to_string(f);
  for(const std::string &Attendu : {asSource+": état incorrect (7)\n",
                                    asSource+": type de demande du vert incorrect (9)\n",
                                    asSource+": type de passage au rouge incorrect (9)\n",
                                    asSource+": direction incorrecte (9)\n",
                                    asSource+": ordonnée incorrecte ("+std::to_string(NbY)+")\n",
                                    std::string("Case (0,0): directions possibles incorrectes (20)\n"),
                                    std::string("Case (0,0) (Nord): priorité incorrecte (9)\n")})
   CHECK(asMessages.find(Attendu)!=std::string::npos);
  cv->v[0][0].DirPoss=(dirposs)DirPoss00; // ~voie() lit NbDir[DirPoss] : hors du tableau pour 20
  harnais::Ferme();
  CHECK(Vue.Messages.empty());
  vue=nullptr;
 }
//---------------------------------------------------------------------------
TEST_CASE("Vérification d'intégrité : le message du dépôt de bus donne ses coordonnées", "[corrections][verifie]")
 { // L'original appelait Format("Dépot bus : ", ...) sans spécificateur :
   // « Dépot bus : : abscisse incorrecte (...) », sans les coordonnées
  VueNulle Vue;
  vue=&Vue;
  std::string asCarte=harnais::LitFichier(CheminCarte(Cartes[4])); // Géant.cvw : dépôt de bus défini
  conversion_cvw Ref=ConvertitEn530(asCarte);
  REQUIRE(Ref.Ok);
  const int NbX=LitEntier(asCarte, Champ(Ref, "NbX")), y=LitEntier(asCarte, Champ(Ref, "depot.y"));
  REQUIRE(LitEntier(asCarte, Champ(Ref, "depot.x"))>=0);
  EcritEntier(asCarte, Champ(Ref, "depot.x"), NbX);
  const std::string asFichier=FichierSortie("verifie_depot.cvw");
  EcritFichier(asFichier, asCarte);
  REQUIRE(harnais::Ouvre(asFichier));
  AnsiString asMsgErr;
  cv->Verifie(asMsgErr);
  harnais::Ferme();
  CHECK(asMsgErr.str()==Format("Dépot bus (%d,%d): abscisse incorrecte (%d)\n", ARRAYOFCONST((NbX, y, NbX))).str());
  CHECK(Vue.Messages.empty());
  vue=nullptr;
 }
//---------------------------------------------------------------------------
