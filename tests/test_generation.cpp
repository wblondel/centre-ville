//---------------------------------------------------------------------------
// test_generation.cpp - Génération aléatoire de réseau et « Copier vers... »
//---------------------------------------------------------------------------
#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <string>
#include "cartes.h"
#include "golden.h"
#include "harnais.h"
#include "sortie.h"
//---------------------------------------------------------------------------
namespace
 {
  void CompareAuGolden(const std::string &asRelatif, const std::string &asObtenu)
   {
    const std::string asAttendu=harnais::LitFichier(std::string(CV_DOSSIER_GOLDEN)+"/"+asRelatif);
    REQUIRE_FALSE(asAttendu.empty());
    if (asObtenu!=asAttendu)
     {
      const std::string asSortie=FichierSortie("golden/"+asRelatif);
      std::ofstream(std::filesystem::path(std::u8string(asSortie.begin(), asSortie.end())), std::ios::binary)<<asObtenu;
      INFO("Écart avec tests/golden/" << asRelatif << " (obtenu : " << asSortie << ")");
      const bool Identique=false; // les textes diffèrent (voir ci-dessus)
      CHECK(Identique);
     }
   }
 }
//---------------------------------------------------------------------------
TEST_CASE("Génération aléatoire de réseau à graine fixe", "[golden][generation]")
 {
  for(int k=0; k<NB_CONFIGS_GENERATION; k++)
   {
    const config_generation &Config=ConfigsGeneration[k];
    DYNAMIC_SECTION(Config.Nom)
     {
      std::string asTexte=ProduitGoldenGeneration(Config, FichierSortie(std::string("generation_")+Config.Nom+".cvw"));
      CompareAuGolden(std::string("generation/")+Config.Nom+".txt", asTexte);
     }
   }
 }
//---------------------------------------------------------------------------
TEST_CASE("Copier vers... (PrepareCopie)", "[golden][copie]")
 {
  for(const carte &c : Cartes)
   {
    DYNAMIC_SECTION(c.Nom)
     {
      std::string asDossier=FichierSortie("copie/.");
      asDossier=asDossier.substr(0, asDossier.size()-2);
      std::string asTexte=ProduitGoldenCopie(CheminCarte(c), c.Nom, asDossier);
      CompareAuGolden(std::string("copie/")+c.Nom+".txt", asTexte);
     }
   }
 }
//---------------------------------------------------------------------------
