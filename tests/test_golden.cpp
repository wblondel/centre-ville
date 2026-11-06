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
#include <filesystem>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>
#include "cartes.h"
#include "catch2/catch_message.hpp"
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
    std::istringstream ia(a);
    std::istringstream ib(b);
    std::string la;
    std::string lb;
    for (int n = 1;; n++)
    {
        const bool fa = (bool)std::getline(ia, la);
        const bool fb = (bool)std::getline(ib, lb);
        if (!fa && !fb)
        {
            return "aucune";
        }
        if ((fa != fb) || (la != lb))
        {
            return "ligne " + std::to_string(n) + "\n  attendu : " + (fa ? la : "<fin>") +
                   "\n  obtenu  : " + (fb ? lb : "<fin>");
        }
    }
}
} // namespace
//---------------------------------------------------------------------------
TEST_CASE("Golden master : simulation à graine fixe", "[golden][simulation]")
{
    for (const auto Config : ConfigsGolden)
    {
        for (const carte &c : Cartes)
        {
            DYNAMIC_SECTION(Config.Nom << "/" << c.Nom)
            {
                const std::string asRelatif = FichierGolden(c.Nom, Config);
                const std::string asAttendu = LitTexte(std::string(CV_DOSSIER_GOLDEN) + "/" + asRelatif);
                REQUIRE_FALSE(asAttendu.empty());
                const std::string asObtenu = ProduitGolden(CheminCarte(c), c.Nom, Config);
                if (asObtenu != asAttendu)
                {
                    const std::string asSortie = FichierSortie("golden/" + asRelatif);
                    std::ofstream(std::filesystem::path(std::u8string(asSortie.begin(), asSortie.end())),
                                  std::ios::binary)
                        << asObtenu;
                    INFO("Écart avec tests/golden/" << asRelatif << " (obtenu : " << asSortie << ")\n"
                                                    << PremiereLigneDifferente(asAttendu, asObtenu));
                    const bool Identique = false; // les textes diffèrent (voir ci-dessus)
                    CHECK(Identique);
                }
                else
                {
                    SUCCEED();
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
