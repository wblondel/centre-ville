//---------------------------------------------------------------------------
// cartes.h - Tests du moteur : les 6 cartes livrées dans maps/
//---------------------------------------------------------------------------
#ifndef cartesH
#define cartesH
//---------------------------------------------------------------------------
#include <array>
#include <string>
//---------------------------------------------------------------------------
struct carte
{
    const char *Fichier; // nom du fichier dans maps/ (UTF-8, forme NFC comme dans git)
    const char *Nom;     // nom ASCII (fichiers de référence)
};
//---------------------------------------------------------------------------
const auto Cartes = std::to_array<carte>({
    {.Fichier = "8x8.cvw", .Nom = "8x8"},
    {.Fichier = "Carfours.cvw", .Nom = "Carfours"},
    {.Fichier = "Colimasson.cvw", .Nom = "Colimasson"},
    {.Fichier = "D\xC3\xA9mo.cvw", .Nom = "Demo"},
    {.Fichier = "G\xC3\xA9"
                "ant.cvw",
     .Nom = "Geant"},
    {.Fichier = "Spaghettis.cvw", .Nom = "Spaghettis"},
});
const int NB_CARTES = sizeof(Cartes) / sizeof(Cartes[0]);
//---------------------------------------------------------------------------
// Cartes du tutoriel (docs/tutorial, identiques à celles de l'installeur) :
// vérifiées par le round-trip seulement
const auto CartesTutoriel = std::to_array<carte>({
    {.Fichier = "tutocv.cvw", .Nom = "tutocv"},
    {.Fichier = "tutocvdeco.cvw", .Nom = "tutocvdeco"},
    {.Fichier = "tutocvptn.cvw", .Nom = "tutocvptn"},
});
//---------------------------------------------------------------------------
#ifdef CV_DOSSIER_CARTES
inline std::string CheminCarte(const carte &c)
{
    return std::string(CV_DOSSIER_CARTES) + "/" + c.Fichier;
}
#endif
#ifdef CV_DOSSIER_TUTORIEL
inline std::string CheminCarteTutoriel(const carte &c)
{
    return std::string(CV_DOSSIER_TUTORIEL) + "/" + c.Fichier;
}
#endif
//---------------------------------------------------------------------------
#endif
