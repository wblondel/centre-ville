//---------------------------------------------------------------------------
// test_chemins.cpp - Chemins du moteur que les cartes livrées ne parcourent pas
//
// Repérés par le rapport de couverture (job « Couverture des tests ») :
// modifications du réseau faites depuis l'interface (places et arrêts
// supprimés), option « Réinfections » de l'épidémie, puis distances avec des
// cases « Tout droit » sur deux cases.
//---------------------------------------------------------------------------
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include "allocation.h"
#include "cartes.h"
#include "catch2/catch_message.hpp"
#include "compat/chaine.h"
#include "harnais.h"
#include "simul.h"
#include "sortie.h"
#include "vue.h"
#include "compat/alea.h"
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
namespace
{
using coord = std::pair<int, int>;
//-------------------------------------------------------------------------
// Cases de l'arrêt de bus (Bus) ou de tram n°Numero
std::vector<coord> CasesArret(bool Bus, int Numero)
{
    std::vector<coord> Cases;
    for (int x = 0; x < cv->NbX; x++)
    {
        for (int y = 0; y < cv->NbY; y++)
        {
            if ((Bus ? cv->v[x][y].NumArretBus : cv->v[x][y].NumArretTram) == Numero)
            {
                Cases.emplace_back(x, y);
            }
        }
    }
    return Cases;
}
//-------------------------------------------------------------------------
// Simulation de NbTours tours, sans message
void Simule(int NbTours)
{
    AnsiString asMsgErr;
    REQUIRE(harnais::Demarre(asMsgErr));
    for (int t = 0; t < NbTours; t++)
    {
        REQUIRE(harnais::Tick());
    }
}
//-------------------------------------------------------------------------
// Empreinte FNV-1a (64 bits) d'un texte
std::uint64_t Empreinte(const std::string &Texte)
{
    std::uint64_t h = 0xcbf29ce484222325ULL;
    for (const char c : Texte)
    {
        h ^= static_cast<unsigned char>(c);
        h *= 0x100000001b3ULL;
    }
    return h;
}
//-------------------------------------------------------------------------
// Tables de distances de chaque case (voie::CalculeDistance*), en texte
std::string TablesDistances()
{
    std::string Texte;
    for (int x = 0; x < cv->NbX; x++)
    {
        for (int y = 0; y < cv->NbY; y++)
        {
            const voie &V = cv->v[x][y];
            Texte += std::format("{} {} :", x, y);
            for (int k = 0; k < V.NbDirDist; k++)
            {
                for (int e = 0; e < V.DistParking().size(); e++)
                {
                    Texte += std::format(" P{},{}", V.DistParking()[e][k][0], V.DistParking()[e][k][1]);
                }
                for (int e = 0; e < V.DistFilePark().size(); e++)
                {
                    Texte += std::format(" F{},{}", V.DistFilePark()[e][k][0], V.DistFilePark()[e][k][1]);
                }
                for (int e = 0; e < V.DistArretBus().size(); e++)
                {
                    Texte += std::format(" B{}", V.DistArretBus()[e][k]);
                }
                for (int e = 0; e < V.DistFileTaxi().size(); e++)
                {
                    Texte += std::format(" T{}", V.DistFileTaxi()[e][k]);
                }
                for (int e = 0; e < V.DistPlaceVehlib().size(); e++)
                {
                    Texte += std::format(" V{}", V.DistPlaceVehlib()[e][k]);
                }
            }
            if (V.SensVoieTram > 0)
            {
                const int nd = V.NbDirPossSensVoieTram(indefinie);
                for (int e = 0; e < V.DistArretTram().size(); e++)
                {
                    for (int k = 0; k < nd; k++)
                    {
                        Texte += std::format(" R{}", V.DistArretTram()[e][k]);
                    }
                }
            }
            Texte += '\n';
        }
    }
    return Texte;
}
} // namespace
//---------------------------------------------------------------------------
TEST_CASE("Places de parking et de vehlib supprimées : retirées et renumérotées à la réouverture", "[chemins][places]")
{ // ActionPlaceParkExecute et ActionPlaceVehlibExecute sur une place existante,
  // puis Enregistrer et Ouvrir, comme le demande VerifieReseau (« Pour
  // renuméroter : sauvez puis rouvrez le réseau ») : Charge retire les places
  // indéfinies et renumérote les suivantes (v5.1 et v5.3)
    VueNulle Vue;
    vue = &Vue;
    alea::FixeGraine(1);
    REQUIRE(harnais::Ouvre(CheminCarte(Cartes[4]))); // Géant : 28 places de parking, 32 places de vehlib
    const int NbPark = cv->NbPlacesPark;
    const int NbVehlib = cv->NbPlacesVehlib;
    REQUIRE(NbPark == 28);
    REQUIRE(NbVehlib == 32);
    std::vector<coord> Park;
    std::vector<coord> Vehlib;
    Park.reserve(NbPark);
    for (int p = 0; p < NbPark; p++)
    {
        Park.emplace_back(cv->PlacePark[p].x, cv->PlacePark[p].y);
    }
    Vehlib.reserve(NbVehlib);
    for (int p = 0; p < NbVehlib; p++)
    {
        Vehlib.emplace_back(cv->PlaceVehlib[p].x, cv->PlaceVehlib[p].y);
    }
    // Places n°2 et dernière
    cv->SupprimePlacePark(1);
    cv->SupprimePlacePark(NbPark - 1);
    cv->SupprimePlaceVehlib(1);
    cv->SupprimePlaceVehlib(NbVehlib - 1);
    CHECK_FALSE(cv->PlacePark[1].Definie());
    CHECK_FALSE(cv->PlaceVehlib[1].Definie());
    CHECK(cv->v[Park[1].first][Park[1].second].NumPlacePark == 0);
    CHECK(cv->v[Vehlib[1].first][Vehlib[1].second].NumPlaceVehlib == 0);
    const std::string asFichier = FichierSortie("places_supprimees.cvw");
    REQUIRE(harnais::SauveSansChaos(asFichier));
    harnais::Ferme();

    REQUIRE(harnais::Ouvre(asFichier));
    REQUIRE(cv->NbPlacesPark == NbPark - 2);
    REQUIRE(cv->NbPlacesVehlib == NbVehlib - 2);
    for (int p = 0; p < cv->NbPlacesPark; p++) // nouveau n° p+1 : ancien n° p+1 (p=0) ou p+2
    {
        const coord c = Park[(p != 0) ? p + 1 : p];
        INFO("Place de parking n°" << p + 1);
        CHECK(cv->PlacePark[p].x == c.first);
        CHECK(cv->PlacePark[p].y == c.second);
        CHECK(cv->v[c.first][c.second].NumPlacePark == p + 1);
    }
    for (int p = 0; p < cv->NbPlacesVehlib; p++)
    {
        const coord c = Vehlib[(p != 0) ? p + 1 : p];
        INFO("Place de vehlib n°" << p + 1);
        CHECK(cv->PlaceVehlib[p].x == c.first);
        CHECK(cv->PlaceVehlib[p].y == c.second);
        CHECK(cv->v[c.first][c.second].NumPlaceVehlib == p + 1);
    }
    for (const coord &c : {Park[1], Park[NbPark - 1]})
    {
        CHECK(cv->v[c.first][c.second].NumPlacePark == 0);
    }
    for (const coord &c : {Vehlib[1], Vehlib[NbVehlib - 1]})
    {
        CHECK(cv->v[c.first][c.second].NumPlaceVehlib == 0);
    }
    AnsiString asMsgErr;
    CHECK(harnais::VerifieReseau(asMsgErr));
    CHECK(asMsgErr.str().empty());
    Simule(200);
    CHECK(Vue.Messages.empty());
    harnais::Ferme();
    alea::LibereGraine();
    vue = nullptr;
}
//---------------------------------------------------------------------------
TEST_CASE("Arrêts de bus et de tram : case de tête retirée, rajoutée, puis arrêt supprimé", "[chemins][arrets]")
{ // ActionArretBusTramExecute sur une case d'un arrêt de plusieurs cases :
  // sa voisine devient la tête (ExisteCaseArretBus et ExisteCaseArretTram,
  // v3.0.3). La case rajoutée rejoint l'arrêt contigu (ArretBusExistant et
  // ArretTramExistant)
    VueNulle Vue;
    vue = &Vue;
    alea::FixeGraine(1);
    REQUIRE(harnais::Ouvre(CheminCarte(Cartes[4]))); // Géant : arrêts de bus et de tram de 2 ou 3 cases

    SECTION("tête retirée puis rajoutée : la simulation tourne")
    {
        for (const bool Bus : {true, false})
        {
            INFO((Bus ? "Arrêt de bus n°1" : "Arrêt de tram n°1"));
            const coord Tete =
                Bus ? coord(cv->ArretBus[0].x, cv->ArretBus[0].y) : coord(cv->ArretTram[0].x, cv->ArretTram[0].y);
            REQUIRE(CasesArret(Bus, 1).size() == 3);
            if (Bus)
            {
                cv->SupprimeArretBus(Tete.first, Tete.second);
            }
            else
            {
                cv->SupprimeArretTram(Tete.first, Tete.second);
            }
            const std::vector<coord> Reste = CasesArret(Bus, 1);
            REQUIRE(Reste.size() == 2);
            const coord NvTete =
                Bus ? coord(cv->ArretBus[0].x, cv->ArretBus[0].y) : coord(cv->ArretTram[0].x, cv->ArretTram[0].y);
            CHECK(NvTete != Tete);
            CHECK(((NvTete == Reste[0]) || (NvTete == Reste[1]))); // voisine de l'ancienne tête
            CHECK(std::abs(NvTete.first - Tete.first) + std::abs(NvTete.second - Tete.second) == 1);
            CHECK((Bus ? cv->AjouteArretBus(Tete.first, Tete.second) : cv->AjouteArretTram(Tete.first, Tete.second)) ==
                  1);
            CHECK(CasesArret(Bus, 1).size() == 3);
        }
        CHECK(cv->ArretTram[0].x == 35); // tram : la tête ne revient pas (double sens possible)
        CHECK(cv->ArretTram[0].y == 51);
        AnsiString asMsgErr;
        REQUIRE(harnais::VerifieReseau(asMsgErr));
        Simule(300);
        CHECK(Vue.Messages.empty());
    }

    SECTION("arrêt de tram de deux cases retiré case par case, puis arrêt supprimé en entier")
    {
        std::vector<coord> Cases = CasesArret(false, 3);
        REQUIRE(Cases.size() == 2);
        cv->SupprimeArretTram(Cases[0].first, Cases[0].second);
        CHECK(cv->ArretTram[2].Defini());
        AnsiString asMsgErr;
        CHECK_FALSE(harnais::VerifieReseau(asMsgErr));
        CHECK(asMsgErr.str() == "L'arrêt de tram n°3 n'est défini que sur la case (" + std::to_string(Cases[1].first) +
                                    ", " + std::to_string(Cases[1].second) + ").");
        cv->SupprimeArretTram(Cases[1].first, Cases[1].second);
        CHECK_FALSE(cv->ArretTram[2].Defini()); // plus aucune case
        CHECK(CasesArret(false, 3).empty());
        CHECK_FALSE(harnais::VerifieReseau(asMsgErr));
        CHECK(asMsgErr.str().find("utilise l'arrêt n°3 qui n'est pas défini.") != std::string::npos);
        // SupprimeArretsUtilisesDansSelection : toutes les cases d'un coup
        REQUIRE(CasesArret(false, 2).size() == 3);
        cv->SupprimeArretTram(1);
        CHECK_FALSE(cv->ArretTram[1].Defini());
        CHECK(CasesArret(false, 2).empty());
    }
    harnais::Ferme();
    alea::LibereGraine();
    vue = nullptr;
}
//---------------------------------------------------------------------------
TEST_CASE("Arrêts et feux de piétons ajoutés quand tous les numéros sont pris : tableaux agrandis", "[chemins][arrets]")
{ // IncrementeNombre (v5.1) recopie les éléments existants par leur Copie dans
  // un tableau d'un élément de plus. Géant a 25 arrêts de bus, 16 arrêts de
  // tram et 6 feux de piétons, tous définis
    VueNulle Vue;
    vue = &Vue;
    alea::FixeGraine(1);
    REQUIRE(harnais::Ouvre(CheminCarte(Cartes[4])));
    struct arret
    {
        int x, y, NbLignes;
        std::vector<int> NumLigne;
        bool operator==(const arret &) const = default;
    };
    const auto Arrets = [](const auto &a, int n) {
        std::vector<arret> l;
        l.reserve(static_cast<std::size_t>(n));
        for (int i = 0; i < n; i++)
        {
            l.push_back(
                arret{a[i].x, a[i].y, a[i].NbLignes,
                      std::vector<int>(a[i].NumLigne.begin(), std::next(a[i].NumLigne.begin(), a[i].NbLignes))});
        }
        return l;
    };
    // Case de route (Bus) ou de voie de tram seule, sans arrêt ni arrêt voisin
    const auto CaseLibre = [](bool Bus) {
        for (int x = 1; x < cv->NbX - 1; x++)
        {
            for (int y = 1; y < cv->NbY - 1; y++)
            {
                const voie &v = cv->v[x][y];
                bool Libre = Bus ? (v.DirPoss && !v.SensVoieTram && !v.NumParking && !v.NumPlaceTaxi &&
                                    !v.NumPlacePark && !v.NumPlaceVehlib)
                                 : (v.SensVoieTram && !v.DirPoss);
                for (int d = nord; Libre && (d <= ouest); d++)
                {
                    Libre = !(Bus ? cv->Voisine(x, y, d).NumArretBus : cv->Voisine(x, y, d).NumArretTram);
                }
                if (Libre && !(Bus ? v.NumArretBus : v.NumArretTram))
                {
                    return coord(x, y);
                }
            }
        }
        return coord(-1, -1);
    };

    SECTION("arrêts de bus et de tram, puis simulation")
    {
        const std::vector<arret> Bus = Arrets(cv->ArretBus, cv->NbArretsBus);
        const std::vector<arret> Tram = Arrets(cv->ArretTram, cv->NbArretsTram);
        REQUIRE(Bus.size() == 25);
        REQUIRE(Tram.size() == 16);
        const coord b = CaseLibre(true);
        const coord t = CaseLibre(false);
        REQUIRE(b.first >= 0);
        REQUIRE(t.first >= 0);
        CHECK(cv->AjouteArretBus(b.first, b.second) == 26);
        CHECK(cv->AjouteArretTram(t.first, t.second) == 17);
        CHECK(Arrets(cv->ArretBus, 25) == Bus);
        CHECK(Arrets(cv->ArretTram, 16) == Tram);
        CHECK(cv->ArretBus[25].x == b.first);
        CHECK(cv->ArretTram[16].y == t.second);
        // Un arrêt d'une seule case est refusé par VerifieReseau : retirés, ils restent indéfinis
        cv->SupprimeArretBus(b.first, b.second);
        cv->SupprimeArretTram(t.first, t.second);
        CHECK_FALSE(cv->ArretBus[25].Defini());
        CHECK_FALSE(cv->ArretTram[16].Defini());
        AnsiString asMsgErr;
        REQUIRE(harnais::VerifieReseau(asMsgErr));
        Simule(100);
        CHECK(Vue.Messages.empty());
    }

    SECTION("feux de piétons : listes de feux gardées")
    {
        struct feu_p
        {
            int x, y, Dir;
            std::vector<int> NumFeux;
            bool operator==(const feu_p &) const = default;
        };
        const auto FeuxP = [](int n) {
            std::vector<feu_p> l;
            l.reserve(static_cast<std::size_t>(n));
            for (int i = 0; i < n; i++)
            {
                l.push_back(feu_p{
                    .x = cv->FeuP[i].x[0],
                    .y = cv->FeuP[i].y[0],
                    .Dir = cv->FeuP[i].Dir[0],
                    .NumFeux =
                        std::vector<int>(cv->FeuP[i].NumFeux.begin(), cv->FeuP[i].NumFeux.begin() + cv->FeuP[i].NbFeux),
                });
            }
            return l;
        };
        const std::vector<feu_p> Avant = FeuxP(cv->NbFeuxPieton);
        REQUIRE(Avant.size() == 6);
        REQUIRE(std::ranges::any_of(Avant, [](const feu_p &f) { return !f.NumFeux.empty(); }));
        // Comme la fiche Priorités piétons : passage piétons sans feu dans cette direction
        int x = -1;
        int y = -1;
        int d = 0;
        for (int i = 0; (x < 0) && (i < cv->NbX); i++)
        {
            for (int j = 0; (x < 0) && (j < cv->NbY); j++)
            {
                for (int k = nord; (x < 0) && (k <= ouest); k++)
                {
                    if (cv->v[i][j].PassagePietons && (cv->v[i][j].NumFeuP[k - 1] == 0) &&
                        cv->FeuPietonsPossible(i, j, static_cast<direction>(k)))
                    {
                        x = i;
                        y = j;
                        d = k;
                    }
                }
            }
        }
        REQUIRE(x >= 0);
        CHECK(cv->AjouteFeuPietons(x, y, (direction)d) == 7);
        CHECK(FeuxP(6) == Avant);
        CHECK(cv->FeuP[6].x[0] == x);
        CHECK(cv->FeuP[6].y[0] == y);
    }
    harnais::Ferme();
    alea::LibereGraine();
    vue = nullptr;
}
//---------------------------------------------------------------------------
TEST_CASE("Épidémie avec réinfections : un piéton guéri retombe malade", "[chemins][epidemie]")
{ // Option « Réinfections » de la fiche Épidémie (v5.4.2), que le golden
  // master n'active pas : la nouvelle infection est chaînée à la précédente
  // (infection::Svt), libérée avec le piéton
    VueNulle Vue;
    Vue.Epidemie = true;
    vue = &Vue;
    alea::FixeGraine(1);
    long Avant = 0;
    int NbReinfectes = 0;
    for (int Essai = 0; Essai < 2; Essai++) // Le premier passage remplit les caches
    {
        Avant = NombreAllocationsVives();
        REQUIRE(harnais::Ouvre(CheminCarte(Cartes[4]))); // Géant : 327 piétons
        cv->EpidemieReinfections = 1;
        Simule(500);
        NbReinfectes = 0;
        for (int i = 0; i < cv->NbPietons; i++)
        {
            pieton &p = cv->Pieton[i];
            INFO("Piéton n°" << i + 1);
            if (p.NbInfections() > 1)
            {
                NbReinfectes++;
            }
            if (p.EstContagieux()) // l'infection en cours, même chaînée
            {
                CHECK(p.DonneChargeVirale() > 0);
                CHECK(p.DonneRang() >= 0);
            }
            else if (p.EstVivant())
            {
                CHECK(p.DonneChargeVirale() == 0); // un mort garde sa charge
            }
        }
        harnais::Ferme();
    }
    CHECK(NbReinfectes > 0);
    CHECK(NombreAllocationsVives() == Avant);
    alea::LibereGraine();
    vue = nullptr;
}
//---------------------------------------------------------------------------
TEST_CASE("Distances avec des cases « Tout droit » et des passages de bus ajoutés : valeurs de la 6.0.16",
          "[chemins][distances]")
{ // Aucune carte livrée ne parcourt le « Tout droit » sur deux cases de voie::CalculeDistance (v3.9.1). Cases tirées
  // au hasard (std::mt19937, identique partout) parmi les voies dont toutes les voisines atteintes sont des voies ;
  // empreintes des tables calculées par le moteur 6.0.16, avant la réécriture des parcours (6.0.17). Géant est la
  // seule carte où le saut de deux cases change aussi les distances simples (arrêts de bus, taxis, vehlibs)
    struct cas
    {
        int Carte;
        unsigned Graine;
        int PctToutDroit, PctPassageBus;
        std::uint64_t Empreinte;
    };
    const auto Cas = std::to_array<cas>({
        {.Carte = 3, .Graine = 1, .PctToutDroit = 20, .PctPassageBus = 5, .Empreinte = 0x1da54af89b52d89cULL},
        {.Carte = 3, .Graine = 1, .PctToutDroit = 100, .PctPassageBus = 50, .Empreinte = 0x2db2793e1e3d9c3eULL},
        {.Carte = 1, .Graine = 1, .PctToutDroit = 20, .PctPassageBus = 5, .Empreinte = 0x7294ff367e4cdb6aULL},
        {.Carte = 5, .Graine = 1, .PctToutDroit = 20, .PctPassageBus = 5, .Empreinte = 0x6e86b30c33250619ULL},
        {.Carte = 4, .Graine = 1, .PctToutDroit = 20, .PctPassageBus = 5, .Empreinte = 0x358fd7283164eb42ULL},
    });
    VueNulle Vue;
    vue = &Vue;
    for (const cas &c : Cas)
    {
        INFO(Cartes[c.Carte].Nom << ", " << c.PctToutDroit << " % tout droit, " << c.PctPassageBus
                                 << " % passages de bus");
        REQUIRE(harnais::Ouvre(CheminCarte(Cartes[c.Carte])));
        std::mt19937 Alea(c.Graine);
        int NbToutDroit = 0;
        int NbPassagesBus = 0;
        for (int x = 0; x < cv->NbX; x++)
        {
            for (int y = 0; y < cv->NbY; y++)
            {
                voie &V = cv->v[x][y];
                if (V.DirPoss == 0)
                {
                    continue;
                }
                bool VoisinesVoies = true;
                for (int d = nord; d <= ouest; d++)
                {
                    if (((V.DirPoss & PossDir[d]) == PossDir[d]) && (cv->Voisine(x, y, d).DirPoss == 0))
                    {
                        VoisinesVoies = false;
                    }
                }
                if (VoisinesVoies && std::cmp_less(Alea() % 100, c.PctToutDroit))
                {
                    V.ToutDroit = true;
                    NbToutDroit++;
                }
                if (std::cmp_less(Alea() % 100, c.PctPassageBus))
                {
                    V.PassageBus = !V.PassageBus;
                    NbPassagesBus++;
                }
            }
        }
        cv->DefinitFilesTaxi();
        cv->DefinitFilesPark();
        REQUIRE(cv->PrepareCalculDistances());
        cv->CalculeDistances();
        const std::string Tables =
            std::format("tout droit {}, passages de bus {}\n", NbToutDroit, NbPassagesBus) + TablesDistances();
        CHECK(Empreinte(Tables) == c.Empreinte);
        harnais::Ferme();
    }
    vue = nullptr;
}
//---------------------------------------------------------------------------
