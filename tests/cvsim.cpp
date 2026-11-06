//---------------------------------------------------------------------------
// cvsim.cpp - Centre-Ville en ligne de commande (moteur seul, sans Qt)
//
//   cvsim CARTE.cvw [--tours N] [--graine G] [--sanglante] [--epidemie]
//                   [--resurrection T] [--sauve SORTIE.cvw] [--etat]
//       Charge la carte, démarre la simulation comme la fiche principale,
//       effectue N tours (100 par défaut), affiche un résumé ; --sauve
//       enregistre le résultat, --etat écrit l'état complet sur la sortie ;
//       --resurrection T demande la résurrection des piétons avant le tour T.
//       Avec --graine G, la simulation est celle du golden master.
//   cvsim --golden DOSSIER_MAPS DOSSIER_GOLDEN
//       Régénère les fichiers de référence du golden master.
//---------------------------------------------------------------------------
#include <span>
#include <exception>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include "cartes.h"
#include "compat/chaine.h"
#include "etat.h"
#include "golden.h"
#include "harnais.h"
#include "simul.h"
#include "vue.h"
#include "compat/alea.h"
//---------------------------------------------------------------------------
namespace
{
int Usage()
{
    std::cerr << "Usage : cvsim CARTE.cvw [--tours N] [--graine G] [--sanglante] [--epidemie] [--resurrection T]\n"
                 "                    [--sauve SORTIE.cvw] [--etat]\n"
                 "        cvsim --golden DOSSIER_MAPS DOSSIER_GOLDEN\n";
    return 2;
}
//-------------------------------------------------------------------------
std::filesystem::path Chemin(const std::string &s)
{
    return {std::u8string(s.begin(), s.end())};
}
//-------------------------------------------------------------------------
int RegenereGolden(const std::string &asMaps, const std::string &asGolden)
{
    for (const carte &c : Cartes)
    {
        for (const auto k : ConfigsGolden)
        {
            const std::string Texte = ProduitGolden(asMaps + "/" + c.Fichier, c.Nom, k);
            const std::filesystem::path p = Chemin(asGolden) / Chemin(FichierGolden(c.Nom, k));
            std::filesystem::create_directories(p.parent_path());
            std::ofstream f(p, std::ios::binary);
            f << Texte;
            std::cout << p.string() << " (" << Texte.size() << " octets)\n";
        }
    }
    const std::filesystem::path Temp = std::filesystem::temp_directory_path() / "cvsim";
    std::filesystem::create_directories(Temp);
    const std::string asTemp = Temp.string();
    for (const auto &k : ConfigsGeneration)
    {
        const std::string Texte = ProduitGoldenGeneration(k, asTemp + "/generation.cvw");
        const std::filesystem::path p = Chemin(asGolden) / "generation" / (std::string(k.Nom) + ".txt");
        std::filesystem::create_directories(p.parent_path());
        std::ofstream(p, std::ios::binary) << Texte;
        std::cout << p.string() << " (" << Texte.size() << " octets)\n";
    }
    for (const carte &c : Cartes)
    {
        const std::string Texte = ProduitGoldenCopie(asMaps + "/" + c.Fichier, c.Nom, asTemp);
        const std::filesystem::path p = Chemin(asGolden) / "copie" / (std::string(c.Nom) + ".txt");
        std::filesystem::create_directories(p.parent_path());
        std::ofstream(p, std::ios::binary) << Texte;
        std::cout << p.string() << " (" << Texte.size() << " octets)\n";
    }
    return 0;
}
} // namespace
//---------------------------------------------------------------------------
int main(int argc, char *const *argv)
try
{
    const std::span<char *const> Args(argv, static_cast<std::size_t>(argc));
    if (argc < 2)
    {
        return Usage();
    }
    if (std::strcmp(Args[1], "--golden") == 0)
    {
        if (argc != 4)
        {
            return Usage();
        }
        return RegenereGolden(Args[2], Args[3]);
    }
    const std::string asCarte = Args[1];
    std::string asSortie;
    int NbTours = 100;
    int TourResurrection = 0;
    bool Etat = false;
    bool GraineFixee = false;
    unsigned Graine = 0;
    VueNulle Vue;
    for (int i = 2; i < argc; i++)
    {
        if ((std::strcmp(Args[i], "--tours") == 0) && (i + 1 < argc))
        {
            NbTours = std::stoi(Args[++i]);
        }
        else if ((std::strcmp(Args[i], "--graine") == 0) && (i + 1 < argc))
        {
            Graine = static_cast<unsigned>(std::strtoul(Args[++i], nullptr, 0));
            GraineFixee = true;
        }
        else if (std::strcmp(Args[i], "--sanglante") == 0)
        {
            Vue.Sanglante = true;
        }
        else if (std::strcmp(Args[i], "--epidemie") == 0)
        {
            Vue.Epidemie = true;
        }
        else if ((std::strcmp(Args[i], "--resurrection") == 0) && (i + 1 < argc))
        {
            TourResurrection = std::stoi(Args[++i]);
        }
        else if ((std::strcmp(Args[i], "--sauve") == 0) && (i + 1 < argc))
        {
            asSortie = Args[++i];
        }
        else if (std::strcmp(Args[i], "--etat") == 0)
        {
            Etat = true;
        }
        else
        {
            return Usage();
        }
    }
    vue = &Vue;
    if (GraineFixee)
    {
        alea::FixeGraine(Graine);
    }
    if (!harnais::Ouvre(asCarte))
    {
        std::cerr << "Chargement impossible : " << asCarte << "\n";
        return 1;
    }
    std::cout << "Carte " << asCarte << " : " << cv->NbX << "x" << cv->NbY << ", version 0x" << std::hex
              << VersionFichier << std::dec << ", " << cv->NbVehicules << " véhicules, " << cv->NbPietons
              << " piétons, " << cv->NbParkings << " parkings, " << cv->NbFeux << " feux\n";
    AnsiString asMsgErr;
    if (!harnais::Demarre(asMsgErr))
    {
        std::cerr << "Démarrage impossible : " << asMsgErr << "\n";
        return 1;
    }
    int t = 0;
    for (t = 0; t < NbTours; t++)
    {
        if ((TourResurrection != 0) && (t + 1 == TourResurrection))
        {
            Vue.Resurrection = true; // comme le golden master
        }
        if (!harnais::Tick())
        {
            std::cout << "Simulation terminée (" << cv->NbVehicules << " véhicules)\n";
            break;
        }
        if ((((t + 1) % 10) == 0) || (t + 1 == NbTours))
        {
            std::cout << "Tour " << cv->TourCrt << " : " << cv->NbVehiculesSortis << " sortis, "
                      << cv->NbVehiculesArrives << " arrivés, " << cv->NbVehiculesDeplaces << " déplacés\n";
        }
    }
    for (const VueNulle::message &m : Vue.Messages)
    {
        std::cout << "Message : " << m.asTitre << " | " << m.asMessage << "\n";
    }
    if (Etat)
    {
        std::cout << ExportEtat::Texte(cv, true);
    }
    int Retour = 0;
    if (!asSortie.empty())
    {
        if (harnais::Sauve(asSortie))
        {
            std::cout << "Enregistré : " << asSortie << "\n";
        }
        else
        {
            std::cerr << "Enregistrement impossible : " << asSortie << "\n";
            Retour = 1;
        }
    }
    harnais::Ferme();
    vue = nullptr;
    return Retour;
}
catch (const std::exception &e)
{
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
}
//---------------------------------------------------------------------------
