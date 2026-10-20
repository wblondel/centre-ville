//---------------------------------------------------------------------------
// test_golden.cpp - Golden master de la simulation (500 tours x 6 cartes)
//
// Compare la simulation à graine fixe aux fichiers de référence de
// tests/golden/<config>/<carte>.txt. En cas d'écart, le texte obtenu est écrit
// dans <build>/tests/sortie/golden/... et la première ligne différente est
// affichée. Régénération (après validation d'un changement VOULU) :
//   cvsim --golden maps tests/golden
//---------------------------------------------------------------------------
#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include "cartes.h"
#include "golden.h"
#include "harnais.h"
#include "sortie.h"
//---------------------------------------------------------------------------
namespace
 {
  std::string LitTexte(const std::string &asChemin)
   {
    return harnais::LitFichier(asChemin);
   }
  //-------------------------------------------------------------------------
  // Numéro (à partir de 1) et contenu de la première ligne différente
  std::string PremiereLigneDifferente(const std::string &a, const std::string &b)
   {
    std::istringstream ia(a), ib(b);
    std::string la, lb;
    for(int n=1; ; n++)
     {
      bool fa=(bool)std::getline(ia, la), fb=(bool)std::getline(ib, lb);
      if (!fa&&!fb) return "aucune";
      if ((fa!=fb)||(la!=lb))
       return "ligne "+std::to_string(n)+"\n  attendu : "+(fa?la:"<fin>")+"\n  obtenu  : "+(fb?lb:"<fin>");
     }
   }
 }
//---------------------------------------------------------------------------
TEST_CASE("Golden master : simulation à graine fixe", "[golden][simulation]")
 {
  for(int k=0; k<NB_CONFIGS_GOLDEN; k++)
   for(const carte &c : Cartes)
    {
     const config_golden &Config=ConfigsGolden[k];
     DYNAMIC_SECTION(Config.Nom << "/" << c.Nom)
      {
       const std::string asRelatif=FichierGolden(c.Nom, Config);
       const std::string asAttendu=LitTexte(std::string(CV_DOSSIER_GOLDEN)+"/"+asRelatif);
       REQUIRE_FALSE(asAttendu.empty());
       const std::string asObtenu=ProduitGolden(CheminCarte(c), c.Nom, Config);
       if (asObtenu!=asAttendu)
        {
         const std::string asSortie=FichierSortie("golden/"+asRelatif);
         std::ofstream(std::filesystem::path(std::u8string(asSortie.begin(), asSortie.end())), std::ios::binary)<<asObtenu;
         INFO("Écart avec tests/golden/" << asRelatif << " (obtenu : " << asSortie << ")\n"
              << PremiereLigneDifferente(asAttendu, asObtenu));
         const bool Identique=false; // les textes diffèrent (voir ci-dessus)
         CHECK(Identique);
        }
       else
        SUCCEED();
      }
    }
 }
//---------------------------------------------------------------------------
