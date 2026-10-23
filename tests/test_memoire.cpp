//---------------------------------------------------------------------------
// test_memoire.cpp - Le moteur ne dépend pas de la remise à zéro de la mémoire
//
// Compilé dans cvtests_motif, avec test_cvw.cpp, test_golden.cpp et
// test_generation.cpp : operator new y remplit la mémoire de l'octet 0xA5
// (support/allocation_motif.cpp) au lieu de la remettre à zéro comme dans
// cvtests, cvsim et cvw. Le round-trip, la vérification, le golden master,
// la génération et la copie doivent donner les mêmes résultats.
//
// L'original laissait indéterminés centre_ville::NbVehlibs (fichiers
// antérieurs au format 0x530, enregistré tel quel), centre_ville::FeuP (sans
// feux de piétons), centre_ville::DureeVert et DureeRouge (nouveau réseau,
// enregistrées telles quelles) et pieton::TourAttBus/TourAttTram/TourAttTaxi ;
// la 6.0.1 les initialise.
//---------------------------------------------------------------------------
#include <catch2/catch_test_macros.hpp>
#include <string>
#include "cartes.h"
#include "format_reference.h"
#include "harnais.h"
#include "sortie.h"
#include "vue.h"
#include "compat/alea.h"
//---------------------------------------------------------------------------
TEST_CASE("Mémoire non remise à zéro : NbVehlibs, FeuP, DureeVert et DureeRouge initialisés", "[memoire]")
 {
  VueNulle Vue;
  vue=&Vue;
  centre_ville *c=new centre_ville(false, 0, 0);
  CHECK(c->NbVehlibs==0);
  CHECK(c->FeuP==nullptr);
  CHECK(c->DureeVert==0); // nouveau réseau (« Nouveau », génération) : enregistrées telles quelles
  CHECK(c->DureeRouge==0);
  delete c;
  // Démo : format 0x500, sans vehlibs ni feux de piétons (champs apparus en 0x530 et 0x520)
  const std::string asDemo=CheminCarte(Cartes[3]);
  REQUIRE(harnais::Ouvre(asDemo));
  REQUIRE(VersionFichier==0x500);
  CHECK(cv->NbVehlibs==0);
  CHECK(cv->FeuP==nullptr);
  // Réenregistrée en 0x530 : NbVehlibs vaut 0, comme dans la conversion de référence
  const std::string asCopie=FichierSortie("motif_Demo.cvw");
  REQUIRE(harnais::SauveSansChaos(asCopie));
  const conversion_cvw Reference=ConvertitEn530(harnais::LitFichier(asDemo));
  REQUIRE(Reference.Ok);
  const bool Identique=(harnais::LitFichier(asCopie)==Reference.Octets);
  CHECK(Identique);
  // Premier feu de piétons : IncrementeNombre libère l'ancien tableau FeuP
  // (l'original libérait un pointeur indéterminé)
  CHECK(cv->AjouteFeuPietons(0, 0, nord)==1);
  CHECK(cv->NbFeuxPieton==1);
  harnais::Ferme();
  CHECK(Vue.Messages.empty());
  vue=nullptr;
 }
//---------------------------------------------------------------------------
TEST_CASE("Mémoire non remise à zéro : piétons initialisés", "[memoire]")
 { // TourAttBus, TourAttTram et TourAttTaxi : 0 tant que le piéton n'a rien attendu
  VueNulle Vue;
  vue=&Vue;
  alea::FixeGraine(1);
  REQUIRE(harnais::Ouvre(CheminCarte(Cartes[4]))); // Géant : piétons, bus, trams, taxis
  AnsiString asMsgErr;
  REQUIRE(harnais::Demarre(asMsgErr));
  REQUIRE(cv->NbPietons>0);
  int NbIndetermines=0;
  for(int i=0; i<cv->NbPietons; i++)
   NbIndetermines+=(cv->Pieton[i].TourAttBus!=0)+(cv->Pieton[i].TourAttTram!=0)+(cv->Pieton[i].TourAttTaxi!=0);
  CHECK(NbIndetermines==0);
  harnais::Ferme();
  alea::LibereGraine();
  vue=nullptr;
 }
//---------------------------------------------------------------------------
