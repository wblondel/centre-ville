//---------------------------------------------------------------------------
// test_corrections.cpp - Bogues d'origine du moteur corrigés en 6.0.1
//
// Chaque cas échoue (ou, pour le blocage des vehlibs, ne se termine pas : le
// délai de CTest le fait échouer) sur le moteur 6.0.0, identique à l'original.
//---------------------------------------------------------------------------
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
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
      REQUIRE(vl.NumPlaceDepart>0);
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
TEST_CASE("Vehlibs : les passagers descendent à destination et redeviennent des piétons", "[corrections][vehlibs]")
 { // L'original cherchait le trottoir depuis la case du vehlib (toujours
   // occupée par lui) et voie::FaitDescendrePieton ne gérait pas les vehlibs :
   // aucun passager ne descendait, le vehlib repartait dès son arrivée et ses
   // passagers n'étaient plus jamais simulés
  VueNulle Vue;
  vue=&Vue;
  alea::FixeGraine(12345);
  REQUIRE(harnais::Ouvre(CheminCarte(Cartes[4]))); // Géant
  AnsiString asMsgErr;
  REQUIRE(harnais::Demarre(asMsgErr));
  REQUIRE(cv->NbVehlibs==29);
  auto Ecart=[](int a, int b, int n) { int e=std::abs(a-b); return std::min(e, n-e); }; // la carte est un tore
  std::vector<int> VehlibAvant(cv->NbPietons), xAvant(cv->NbPietons), yAvant(cv->NbPietons), ArriveeAvant(cv->NbVehlibs);
  std::vector<long> DemiCasesALaDescente(cv->NbPietons, -1); // -1 : pas descendu, ou déjà reparti à pied
  int NbArrivees=0, NbDescentes=0, NbRestesABord=0, NbRepartis=0;
  for(int t=1; t<=600; t++)
   {
    for(int p=0; p<cv->NbPietons; p++)
     {
      VehlibAvant[p]=cv->Pieton[p].NumVehlib;
      xAvant[p]=cv->Pieton[p].x;
      yAvant[p]=cv->Pieton[p].y;
     }
    for(int i=0; i<cv->NbVehlibs; i++) ArriveeAvant[i]=cv->Vehlib[i].NumPlaceArrivee;
    REQUIRE(harnais::Tick());
    for(int i=0; i<cv->NbVehlibs; i++)
     {
      const vehlib &vl=cv->Vehlib[i];
      INFO("Tour " << t << ", vehlib n°" << i+1);
      REQUIRE(vl.NumPlaceDepart>0);
      // Chaque passager était déjà à bord, ou à pied à côté de la place de départ
      // (il y arrive, puis monte, pendant le même tour)
      const place_vehlib &pd=cv->PlaceVehlib[vl.NumPlaceDepart-1];
      int NbPassagers=0;
      for(int p=0; p<cv->NbPietons; p++)
       if (cv->Pieton[p].NumVehlib==i+1)
        {
         NbPassagers++;
         INFO("piéton n°" << p+1);
         if (VehlibAvant[p]!=i+1)
          {
           REQUIRE(VehlibAvant[p]==0);
           REQUIRE(Ecart(xAvant[p], pd.x, cv->NbX)+Ecart(yAvant[p], pd.y, cv->NbY)<=2);
          }
        }
      REQUIRE(vl.NbPersonnes==NbPassagers);
      REQUIRE(vl.NbPersonnes<=cv->NbMaxPersonnesVehlib);
      if (vl.Situation==svDepart)
       { // Il roule, avec au moins un passager, vers la place réservée pour lui
        REQUIRE(vl.NbPersonnes>0);
        REQUIRE(vl.NumPlaceArrivee>0);
        REQUIRE(cv->PlaceVehlib[vl.NumPlaceArrivee-1].NumVehlibReserve==i+1);
       }
      else
       { // Il attend à sa place de départ
        REQUIRE(vl.NumPlaceArrivee==0);
        REQUIRE(pd.x==vl.x);
        REQUIRE(pd.y==vl.y);
       }
      if (ArriveeAvant[i]&&(vl.NumPlaceDepart==ArriveeAvant[i]))
       { // Arrivé pendant ce tour : sa place d'arrivée est devenue sa place de départ
        NbArrivees++;
        const place_vehlib &pv=cv->PlaceVehlib[ArriveeAvant[i]-1];
        REQUIRE(pv.NumVehlibReserve==0);
        for(int p=0; p<cv->NbPietons; p++)
         if (VehlibAvant[p]==i+1)
          {
           pieton &pt=cv->Pieton[p];
           INFO("piéton n°" << p+1);
           if (pt.NumVehlib)
            { // Aucun trottoir voisin ne pouvait l'accueillir : il reste à bord et
              // descendra à la place suivante (le vehlib repart, ou attend une place libre)
             NbRestesABord++;
             REQUIRE(pt.NumVehlib==i+1);
             REQUIRE(((vl.Situation==svDepart)||!ExistePlaceReservable(i+1)));
            }
           else
            { // Descendu sur un trottoir voisin de la place (ou à côté de celui-ci)
             NbDescentes++;
             REQUIRE(pt.Defini());
             REQUIRE(Ecart(pt.x, pv.x, cv->NbX)+Ecart(pt.y, pv.y, cv->NbY)<=2);
             const voie &v=cv->v[pt.x][pt.y];
             REQUIRE(((v.NumPieton[0]==p+1)||(v.NumPieton[1]==p+1)));
             REQUIRE_FALSE(pt.AttendVehlib);
             DemiCasesALaDescente[p]=pt.DemiCasesParcourues;
            }
          }
       }
     }
    for(int p=0; p<cv->NbPietons; p++)
     {
      pieton &pt=cv->Pieton[p];
      if (pt.NumVehlib)
       {
        INFO("Tour " << t << ", piéton n°" << p+1);
        REQUIRE_FALSE(pt.Defini());
       }
      else
       if ((DemiCasesALaDescente[p]>=0)&&(pt.DemiCasesParcourues>DemiCasesALaDescente[p]))
        { // Redevenu un piéton simulé : il marche
         NbRepartis++;
         DemiCasesALaDescente[p]=-1;
        }
     }
    for(int p=0; p<cv->NbPlacesVehlib; p++)
     if (int n=cv->PlaceVehlib[p].NumVehlibReserve)
      {
       INFO("Tour " << t << ", place de vehlib n°" << p+1);
       REQUIRE(cv->Vehlib[n-1].NumPlaceArrivee==p+1);
      }
   }
  INFO(NbArrivees << " arrivées, " << NbDescentes << " descentes, " << NbRestesABord << " restés à bord, " << NbRepartis << " repartis à pied");
  CHECK(NbDescentes>NbArrivees/2); // l'original : aucune
  CHECK(NbRepartis>NbDescentes/2);
  CHECK(NbRestesABord>0); // le cas est bien exercé
  CHECK(Vue.Messages.empty());
  harnais::Ferme();
  alea::LibereGraine();
  vue=nullptr;
 }
//---------------------------------------------------------------------------
TEST_CASE("Ouverture : une erreur de lecture fait échouer Charge", "[corrections][chargement]")
 { // v6.0.2 : l'original ignorait les erreurs de lecture : FileRead renvoie -1, vrai dans
   // les chaînes de &&, et un champ lu en partie comptait pour lu
  VueNulle Vue;
  vue=&Vue;
  auto Charge=[](int h)
   {
    cv=new centre_ville(false, 0, 0); // AUCUN_ELEMENT_PAR_DEFAUT
    bool Ok=cv->Charge(h);
    delete cv;
    cv=nullptr;
    return Ok;
   };
  const std::string asCarte=CheminCarte(Cartes[0]);
  // Référence : la carte entière se charge
  int h=FileOpen(asCarte.c_str(), fmOpenRead);
  REQUIRE(h>0);
  CHECK(Charge(h));
  FileClose(h);
  // Poignée invalide (FileOpen a échoué)
  CHECK_FALSE(Charge(-1));
  // Fichier ouvert en écriture seule : aucune lecture n'aboutit
  const std::string asCopie=FichierSortie("lecture_impossible.cvw");
  const std::string asOctets=harnais::LitFichier(asCarte);
  {
   std::ofstream f(asCopie, std::ios::binary);
   f.write(asOctets.data(), (std::streamsize)asOctets.size());
  }
  REQUIRE((h=FileOpen(asCopie.c_str(), fmOpenWrite))>0);
  CHECK_FALSE(Charge(h));
  FileClose(h);
  // Fichier coupé au milieu du dernier champ, et plus tôt
  for(std::size_t Coupe : {(std::size_t)1, (std::size_t)2, asOctets.size()/2, asOctets.size()-4})
   {
    {
     std::ofstream f(asCopie, std::ios::binary|std::ios::trunc);
     f.write(asOctets.data(), (std::streamsize)(asOctets.size()-Coupe));
    }
    REQUIRE((h=FileOpen(asCopie.c_str(), fmOpenRead))>0);
    INFO("Coupé de " << Coupe << " octets");
    CHECK_FALSE(Charge(h));
    FileClose(h);
   }
  vue=nullptr;
 }
//---------------------------------------------------------------------------
TEST_CASE("Ouverture : un nombre d'éléments impossible fait échouer Charge", "[corrections][chargement]")
 { // v6.0.2 : un nombre négatif ou plus grand que le fichier allait jusqu'à new[]
  VueNulle Vue;
  vue=&Vue;
  const std::string asSource=FichierSortie("nombres_source.cvw");
  REQUIRE(harnais::Ouvre(CheminCarte(Cartes[0])));
  REQUIRE(harnais::SauveSansChaos(asSource)); // Format 0x530 : version, puis les nombres dans cet ordre
  harnais::Ferme();
  const std::string asOctets=harnais::LitFichier(asSource);
  const std::string asCopie=FichierSortie("nombres_impossibles.cvw");
  auto Charge=[&asCopie](const std::string &asDonnees)
   {
    {
     std::ofstream f(asCopie, std::ios::binary|std::ios::trunc);
     f.write(asDonnees.data(), (std::streamsize)asDonnees.size());
    }
    int h=FileOpen(asCopie.c_str(), fmOpenRead);
    REQUIRE(h>0);
    cv=new centre_ville(false, 0, 0); // AUCUN_ELEMENT_PAR_DEFAUT
    bool Ok=cv->Charge(h);
    delete cv;
    cv=nullptr;
    FileClose(h);
    return Ok;
   };
  CHECK(Charge(asOctets));
  struct { const char *Nom; std::size_t Position; } Champs[]=
   {{"NbFeux", 4}, {"NbX", 8}, {"NbY", 12}, {"NbFeuxPieton", 16}, {"NbParkings", 20}, {"NbArretsBus", 32},
    {"NbArretsTram", 40}, {"NbPlacesTaxi", 48}, {"NbPlacesPark", 56}, {"NbPlacesVehlib", 60}};
  for(const auto &c : Champs)
   for(int Valeur : {-5, 0x7FFFFFF0})
    {
     INFO(c.Nom << " = " << Valeur);
     std::string asDonnees=asOctets;
     std::memcpy(asDonnees.data()+c.Position, &Valeur, sizeof(Valeur));
     CHECK_FALSE(Charge(asDonnees));
    }
  // Zone sans case : v resterait nul
  for(std::size_t Position : {(std::size_t)8, (std::size_t)12})
   {
    std::string asDonnees=asOctets;
    int Zero=0;
    std::memcpy(asDonnees.data()+Position, &Zero, sizeof(Zero));
    CHECK_FALSE(Charge(asDonnees));
   }
  vue=nullptr;
 }
//---------------------------------------------------------------------------
long NombreAllocationsVives(); // support/allocation.cpp
//---------------------------------------------------------------------------
TEST_CASE("~centre_ville libère toute la mémoire du réseau", "[corrections][memoire]")
 { // v6.0.2 : l'original ne libérait ni FeuP, ni Bus, ni Tram
  VueNulle Vue;
  vue=&Vue;
  SECTION("feux de piétons d'un nouveau réseau")
   {
    const long Avant=NombreAllocationsVives();
    cv=new centre_ville(true, 20, 12);
    REQUIRE(cv->NbFeuxPieton>0);
    delete cv;
    cv=nullptr;
    CHECK(NombreAllocationsVives()==Avant);
   }
  SECTION("réseau en simulation : véhicules, bus, trams, taxis, vehlibs et piétons")
   {
    const std::string asCarte=CheminCarte(Cartes[4]); // Géant
    alea::FixeGraine(1);
    long Avant=0;
    for(int Essai=0; Essai<2; Essai++) // Le premier passage remplit les caches (chaînes, tampons de fichiers...)
     {
      Avant=NombreAllocationsVives();
      REQUIRE(harnais::Ouvre(asCarte));
      AnsiString asMsgErr;
      REQUIRE(harnais::Demarre(asMsgErr));
      REQUIRE(cv->NbBus>0);
      REQUIRE(cv->NbTrams>0);
      for(int i=0; i<20; i++) harnais::Tick();
      harnais::Ferme();
     }
    CHECK(NombreAllocationsVives()==Avant);
    alea::LibereGraine();
   }
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
TEST_CASE("Vérification d'intégrité : moins de deux parkings, sans division par zéro (v6.0.3)", "[corrections][verification]")
 { // L'original divisait par NbParkings*(NbParkings-1) avant de tester le nombre
   // de parkings : arrêt de l'application sur x86, valeur quelconque sur ARM
  VueNulle Vue;
  vue=&Vue;
  for(int NbParkings : {0, 1})
   {
    cv=new centre_ville(true, NBXMIN, NBYMIN);
    cv->DefinitNbParkings(NbParkings);
    AnsiString asMsgErr;
    cv->Verifie(asMsgErr);
    const std::string asMessages=asMsgErr.str();
    delete cv;
    cv=nullptr;
    INFO(asMessages);
    CHECK(asMessages.find("Le nombre de parkings est incorrect ("+std::to_string(NbParkings)+")\n")!=std::string::npos);
    CHECK(asMessages.find("Le nombre de véhicules ne permet pas une simulation équilibrée")!=std::string::npos);
   }
  vue=nullptr;
 }
//---------------------------------------------------------------------------
TEST_CASE("Listes de feux : le feu retiré est bien celui demandé", "[corrections][feux]")
 { // Une liste d'un seul feu ne le montre pas : il faut en retirer au début, au milieu et à la fin
  VueNulle Vue;
  vue=&Vue;
  cv=new centre_ville(true, NBXMIN, NBYMIN);
  auto Liste=[](int *Num, int Nb) { return std::vector<int>(Num, Num+Nb); };
  feu &f=cv->Feu[0];
  f.NbFeuxDmd=f.NbFeuxAtt=3;
  f.NumFeuxDmd=new int[3]{4, 5, 6};
  f.NumFeuxAtt=new int[3]{4, 5, 6};
  f.RetireFeuDmd(5);
  f.RetireFeuAtt(5);
  CHECK(Liste(f.NumFeuxDmd, f.NbFeuxDmd)==std::vector<int>{4, 6});
  CHECK(Liste(f.NumFeuxAtt, f.NbFeuxAtt)==std::vector<int>{4, 6});
  f.RetireFeuDmd(4);
  f.RetireFeuAtt(4);
  CHECK(Liste(f.NumFeuxDmd, f.NbFeuxDmd)==std::vector<int>{6});
  CHECK(Liste(f.NumFeuxAtt, f.NbFeuxAtt)==std::vector<int>{6});
  f.RetireFeuDmd(6);
  f.RetireFeuAtt(6);
  CHECK(f.NbFeuxDmd==0);
  CHECK(f.NumFeuxDmd==nullptr);
  CHECK(f.NbFeuxAtt==0);
  CHECK(f.NumFeuxAtt==nullptr);
  feu_pieton &p=cv->FeuP[0];
  for(int n : {4, 5, 6}) p.AjouteFeu(n);
  p.RetireFeu(5);
  CHECK(Liste(p.NumFeux, p.NbFeux)==std::vector<int>{4, 6});
  p.RetireFeu(6);
  CHECK(Liste(p.NumFeux, p.NbFeux)==std::vector<int>{4});
  delete cv;
  cv=nullptr;
  vue=nullptr;
 }
//---------------------------------------------------------------------------
TEST_CASE("Nouveau feu : durées par défaut du réseau", "[corrections][feux]")
 {
  VueNulle Vue;
  vue=&Vue;
  cv=new centre_ville(true, NBXMIN, NBYMIN);
  cv->DureeVert=7;
  cv->DureeRouge=9;
  const int n=cv->AjouteFeu(1, 1, est);
  REQUIRE(n>0);
  CHECK(cv->Feu[n-1].DureeVert==7);
  CHECK(cv->Feu[n-1].DureeRouge==9);
  delete cv;
  cv=nullptr;
  vue=nullptr;
 }
//---------------------------------------------------------------------------
TEST_CASE("Direction d'une case : modifiable seulement avant le premier tour", "[corrections][edition]")
 {
  VueNulle Vue;
  vue=&Vue;
  cv=new centre_ville(true, NBXMIN, NBYMIN);
  CHECK(cv->ChangeDirectionSiPossible(1, 1, E));
  CHECK(cv->v[1][1].DirPoss==E);
  cv->TourCrt=1;
  CHECK_FALSE(cv->ChangeDirectionSiPossible(1, 1, E));
  CHECK(cv->v[1][1].DirPoss==E);
  delete cv;
  cv=nullptr;
  vue=nullptr;
 }
//---------------------------------------------------------------------------
