//---------------------------------------------------------------------------
// #include <vcl.h> // portage : C++Builder
// #pragma hdrstop // portage : C++Builder
//---------------------------------------------------------------------------
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <span>
#include <utility>
#include <vector>
//---------------------------------------------------------------------------
#include "compat/chaine.h"
#include "vue.h" // portage : remplace base.h (AfficheMessage) et les fiches f_*.h
#include "simul.h"
#include "format_cvw.h"     // IWYU pragma: keep (static_assert sur les types sérialisés)
#include "compat/fichier.h" // portage : remplace f_stats.h ; FileRead/FileWrite (SysUtils)
#include "compat/alea.h"    // portage : remplace f_statsv.h ; rand()/srand() de la RTL Borland
// #include "f_infveh.h" // portage : remplacé par vue.h
// #include "f_infptn.h" // portage : remplacé par vue.h
// #include "f_infbus.h" // portage : remplacé par vue.h
// #include "f_inftram.h" // portage : remplacé par vue.h
// #include "f_inftaxi.h" // portage : remplacé par vue.h
// #include "f_infvehlib.h" // portage : remplacé par vue.h
// #include "f_rech.h" // portage : remplacé par vue.h
//---------------------------------------------------------------------------
// #pragma package(smart_init) // portage : C++Builder
//---------------------------------------------------------------------------
constexpr bool PEUT_ETRE_INDEFINI = true;
constexpr bool DOIT_ETRE_DEFINI = false;
//---------------------------------------------------------------------------
namespace
{
// v6.0.1 : coordonnée enregistrée par « Copier vers... » : une coordonnée non définie (-1) le reste
int CoordonneeCopiee(int c, int c0)
{
    return (c > -1) ? c - c0 : c;
}
} // namespace
//---------------------------------------------------------------------------

// Tables des directions : simul.h (PossDir, Dirdp, InvDir, SensVoieTramDir...)
//---------------------------------------------------------------------------
constexpr std::array<sens_voie_tram_base, NBSENSVOIETRAM> SensVoieTramBase = {
    aucune_voie_base,
    ne,
    ns,
    aucune_voie_base,
    no,
    aucune_voie_base,
    aucune_voie_base,
    aucune_voie_base,
    se,
    aucune_voie_base,
    aucune_voie_base,
    aucune_voie_base,
    aucune_voie_base,
    aucune_voie_base,
    aucune_voie_base,
    aucune_voie_base,
    eo,
    aucune_voie_base,
    aucune_voie_base,
    aucune_voie_base,
    aucune_voie_base,
    aucune_voie_base,
    aucune_voie_base,
    aucune_voie_base,
    aucune_voie_base,
    aucune_voie_base,
    aucune_voie_base,
    aucune_voie_base,
    aucune_voie_base,
    aucune_voie_base,
    aucune_voie_base,
    aucune_voie_base,
    so,
};
//---------------------------------------------------------------------------
// v3.5                                                           DirDest:  indef, nord,  est,  sud,ouest  // DirOrig
constexpr std::array<std::array<std::array<bool, NBDIR + 1>, NBDIR + 1>, NBSENSVOIETRAMBASE> DirPossSensVoieTramBase = {
    {
        {{
            {false, false, false, false, false}, // indef.// aucune_voie_base
            {false, false, false, false, false}, // nord
            {false, false, false, false, false}, // est
            {false, false, false, false, false}, // sud
            {false, false, false, false, false},
        }}, // ouest
        {{
            {false, true, true, false, false},   // indef.// ne
            {false, false, true, false, false},  // nord
            {false, true, false, false, false},  // est
            {false, false, false, false, false}, // sud
            {false, false, false, false, false},
        }}, // ouest
        {{
            {false, true, false, true, false},   // indef.// ns
            {false, false, false, true, false},  // nord
            {false, false, false, false, false}, // est
            {false, true, false, false, false},  // sud
            {false, false, false, false, false},
        }}, // ouest
        {{
            {false, true, false, false, true},   // indef.// no
            {false, false, false, false, true},  // nord
            {false, false, false, false, false}, // est
            {false, false, false, false, false}, // sud
            {false, true, false, false, false},
        }}, // ouest
        {{
            {false, false, true, true, false},   // indef.// se
            {false, false, false, false, false}, // nord
            {false, false, false, true, false},  // est
            {false, false, true, false, false},  // sud
            {false, false, false, false, false},
        }}, // ouest
        {{
            {false, false, true, false, true},   // indef.// eo
            {false, false, false, false, false}, // nord
            {false, false, false, false, true},  // est
            {false, false, false, false, false}, // sud
            {false, false, true, false, false},
        }}, // ouest
        {{
            {false, false, false, true, true},   // indef.// so
            {false, false, false, false, false}, // nord
            {false, false, false, false, false}, // est
            {false, false, false, false, true},  // sud
            {false, false, false, true, false},
        }},
    }};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// v6.0.2 : FileRead renvoie le nombre d'octets lus, ou -1 : les deux étaient vrais dans
// les chaînes de && des Charge(). Seul un champ lu en entier compte pour lu
namespace
{
bool LitChamp(int hfile, void *Champ, unsigned NbOctets)
{
    return FileRead(hfile, Champ, NbOctets) == static_cast<int>(NbOctets);
}
//---------------------------------------------------------------------------
// v6.0.2 : un nombre d'éléments lu dans le fichier ne peut être ni négatif, ni plus grand
// que le fichier (un élément y occupe au moins un octet) : il irait sinon jusqu'à new[]
long long TailleFichierCharge = 0;
bool NombreCredible(long long n)
{
    return (n >= 0) && (n <= TailleFichierCharge);
}
} // namespace
//---------------------------------------------------------------------------
bool VerifierApresOuverture = false, VerifierAvantSimulation = false;
//---------------------------------------------------------------------------
std::array<int, NBDIR + 1> dx = {0, 0, 1, 0, NBXDEF - 1}; // v4.0 (const int->int, DEF)
std::array<int, NBDIR + 1> dy = {0, NBYDEF - 1, 0, 1, 0}; // v4.0 (const int->int, DEF)
int VersionFichier = 0;                                   // v3.0
int SvgNbFeux = 0;
int SvgNbParkings = 0;
int SvgNbVehicules = 0;
int SvgNbPietons = 0;    // v2.0
int SvgNbArretsBus = 0;  // v3.0
int SvgNbArretsTram = 0; // v3.5
int SvgNbPlacesTaxi = 0; // v3.6
//---------------------------------------------------------------------------
centre_ville *cv = nullptr;
//-----------------------------------------------------------------------------
template <class E> int IncrementeNombre(std::vector<E> &e, int &n) // v5.1
{
    std::vector<E> ee(static_cast<std::size_t>(n) + 1);
    for (int i = 0; i < n; i++)
    {
        ee[i].Copie(&e[i]); // Effectue la copie des membres (les listes passent au nouvel élément)
    }
    ee[n].AffecteNumero(n + 1); // v5.2 (oubli)
    ee[n].Initialise();         // v5.2 (oubli)
    e = std::move(ee);
    return ++n;
}
// portage : instanciation explicite pour f_carfours.cpp (IncrementeNombre(cv->Feu, ...)) ;
// sans elle, l'optimiseur peut ne garder aucune copie appelable de l'instance implicite
template int IncrementeNombre<feu>(std::vector<feu> &e, int &n);
//-----------------------------------------------------------------------------
/* Conditions d'utilisation
1. Exemple d'appel :  IncrementeNombre(Feu, NbFeux)
2. La classe E doit avoir les définitions suivantes :
   - un constructeur par défaut, AffecteNumero(int) et Initialise() ;
   - Copie(E *e), qui reprend les membres de e ; e est détruit juste après, ses listes
     (std::vector) peuvent lui être prises par std::move (6.0.17 : l'original partageait
     des pointeurs, protégés par un drapeau CopieEnCours).
*/
//---------------------------------------------------------------------------
void AppelleAuChaos()
{
    time_t t = 0;
    alea::Srand(static_cast<unsigned>(time(&t)));
}
//-----------------------------------------------------------------------------
// Routines privées
//-----------------------------------------------------------------------------
// v6.0.1 : asMsgErr par référence (par valeur dans l'original : les messages étaient perdus)
namespace
{
void VerifiePosition(AnsiString &asMsgErr, const AnsiString &asSource, int x, int y, bool PeutEtreIndefinie)
{
    if (((x < -1) && PeutEtreIndefinie) || ((x < 0) && (!PeutEtreIndefinie)) || (x >= cv->NbX))
    {
        asMsgErr += asSource + Format(": abscisse incorrecte (%d)\n", TableauDeConstantes(x));
    }
    if (((y < -1) && PeutEtreIndefinie) || ((y < 0) && (!PeutEtreIndefinie)) || (y >= cv->NbY))
    {
        asMsgErr += asSource + Format(": ordonnée incorrecte (%d)\n", TableauDeConstantes(y));
    }
}
//-----------------------------------------------------------------------------
void VerifieDirection(AnsiString &asMsgErr, const AnsiString &asSource, direction d, bool PeutEtreIndefinie)
{
    if (((d < indefinie) && PeutEtreIndefinie) || ((d <= indefinie) && (!PeutEtreIndefinie)) || (d > ouest))
    {
        asMsgErr +=
            asSource + Format(": direction incorrecte (%d)\n", TableauDeConstantes(static_cast<int>(d))); // v5.4 enum
                                                                                                          // as char
                                                                                                          // type (cast
                                                                                                          // int)
    }
}
//-----------------------------------------------------------------------------
void VerifiePriorite(AnsiString &asMsgErr, const AnsiString &asSource, priorite p)
{
    if ((p < a_droite) || (p > feux))
    {
        asMsgErr +=
            asSource + Format(": priorité incorrecte (%d)\n", TableauDeConstantes(static_cast<int>(p))); // v5.4 enum as
                                                                                                         // char type
                                                                                                         // (cast int)
    }
}
//-----------------------------------------------------------------------------
void VerifieDirPoss(AnsiString &asMsgErr, const AnsiString &asSource, dirposs dp)
{
    if ((dp < aucune) || (dp > NESO))
    {
        asMsgErr += asSource + Format(": directions possibles incorrectes (%d)\n",
                                      TableauDeConstantes(static_cast<int>(dp))); // v5.4
                                                                                  // enum as
                                                                                  // char
                                                                                  // type
                                                                                  // (cast
                                                                                  // int)
    }
}
//-----------------------------------------------------------------------------
void VerifieEtat(AnsiString &asMsgErr, const AnsiString &asSource, etat_feu e)
{
    if ((e < inactif) || (e > rouge))
    {
        asMsgErr += asSource + Format(": état incorrect (%d)\n", TableauDeConstantes(static_cast<int>(e))); // v5.4 enum
                                                                                                            // as char
                                                                                                            // type
                                                                                                            // (cast
                                                                                                            // int)
    }
}
//-----------------------------------------------------------------------------
void VerifieDmdVert(AnsiString &asMsgErr, const AnsiString &asSource, dmd_vert dv)
{
    if ((dv < arrivee_vehicule) || (dv > immediate))
    {
        asMsgErr += asSource + Format(": type de demande du vert incorrect (%d)\n",
                                      TableauDeConstantes(static_cast<int>(dv))); // v5.4
                                                                                  // enum as
                                                                                  // char
                                                                                  // type
                                                                                  // (cast
                                                                                  // int)
    }
}
//-----------------------------------------------------------------------------
void VerifiePsgRouge(AnsiString &asMsgErr, const AnsiString &asSource, psg_rouge pr)
{
    if ((pr < prm_dmd) || (pr > immediat))
    {
        asMsgErr += asSource + Format(": type de passage au rouge incorrect (%d)\n",
                                      TableauDeConstantes(static_cast<int>(pr))); // v5.4 enum as char type (cast int)
    }
}
} // namespace
//-----------------------------------------------------------------------------
// Méthodes des classes
//-----------------------------------------------------------------------------
infection::infection(const centre_ville *cv) // Patient zéro initialisé à la création (centre_ville *cv) ou au démarrage
                                             // de la simulation
    : TourDeb(0), TourFin(cv->EpidemieInfectiosite), // au tour TourFin, le piéton n'est plus contagieux
      TourFatal(0),                     // Permet de savoir le tour fatal (TourFatal + cv->EpidemieInfectiosite)
      NumPrmPieton(0), NumDrnPieton(0), // Pas de patient source car patient zéro donc... vaut zéro !
      PrmRang(0), DrnRang(0),           // Patient zéro ben... vaut zéro !
      ChargeVirale(1),                  // Incrémenté à chaque (ré)infection sur la période de contagion
      Svt(nullptr)
{}
//-----------------------------------------------------------------------------
infection::infection(int Tour, int NumPieton, int Rang) // v5.4.1. NumPieton>0 (pieton::Numero+1)
    : TourDeb(Tour + 1),                                // Le piéton est contagieux à partir du tour suivant
      TourFin(Tour + 1 + cv->EpidemieInfectiosite), TourFatal(0), NumPrmPieton(NumPieton), NumDrnPieton(NumPieton),
      PrmRang(Rang), DrnRang(Rang), ChargeVirale(1), Svt(nullptr)
{}
//-----------------------------------------------------------------------------
bool infection::Reinfecte(int Tour, int NumPieton, int Rang) // comme le constructeur mais pour une personne qui a déjà
                                                             // été infectée
{                                                            // retourne vrai si la réinfection a bien eu lieu
    if ((Tour + 1 >= TourDeb) && (Tour + 1 <= TourFin))
    {
        if (NumDrnPieton == NumPieton)
        {
            return false; // Un même piéton ne contamine pas deux fois de suite la même personne
        }

        if (TourFatal == 0)
        {
            TourFin = Tour + cv->EpidemieInfectiosite;
        }
        TourFin = Tour + cv->EpidemieInfectiosite;
        NumDrnPieton = NumPieton;
        DrnRang = Rang;
        ChargeVirale++;
        if (ChargeVirale == cv->EpidemieChargeFatale)
        {
            TourFatal = Tour; // Mort programmée
        }
        return true;
    }

    if (Svt)
    {
        return Svt->Reinfecte(Tour, NumPieton, Rang);
    }

    Svt = new infection(Tour, NumPieton, Rang);
    return true;
}
//-----------------------------------------------------------------------------
bool infection::DoitSuccomber(int Tour) const
{
    if (TourFatal != 0)
    {
        return (Tour >= TourFatal + cv->EpidemieInfectiosite);
    }
    if (Svt)
    {
        return Svt->DoitSuccomber(Tour);
    }

    return false;
}
//-----------------------------------------------------------------------------
void pieton::Verifie(AnsiString &asMsgErr) const
{
    const AnsiString asSource = Format("Piéton n°%d (%d,%d)", TableauDeConstantes(Numero, x, y));
    if (Numero < 0)
    {
        asMsgErr += asSource + Format(": numéro incorrect (%d)\n", TableauDeConstantes(Numero));
    }
    VerifiePosition(asMsgErr, asSource, x, y, DOIT_ETRE_DEFINI);
    VerifieDirection(asMsgErr, asSource, static_cast<direction>(Dir), DOIT_ETRE_DEFINI);
    if ((DemiCase < 0) || (DemiCase > 1))
    {
        asMsgErr += asSource + Format(": valeur ½ case incorrecte (%d)\n", TableauDeConstantes(DemiCase));
    }
}
//-----------------------------------------------------------------------------
void pieton::ChercheFileTaxi() // v3.6
{
    voie *v = nullptr;
    int NumPlaceTaxi = 0;
    direction DirPlace = indefinie;
    direction ProchDir = indefinie;
    if (NumFileTaxi == 0) // Cas improbable mais on ne sait jamais
    {                     // On abandonne la recherche de file de taxi
        FileTaxiTrv = false;
        return; // Numéro de file de taxi obligatoire pour que l'on puisse trouver la tête !
    }
    for (int d = nord; (NumPlaceTaxi == 0) && (d <= ouest); d++)
    {
        v = &cv->Voisine(x, y, d);
        if ((v->NumPlaceTaxi != 0) && (cv->PlaceTaxi[v->NumPlaceTaxi - 1].NumFile == NumFileTaxi - 1))
        {
            NumPlaceTaxi = v->NumPlaceTaxi;
            DirPlace = static_cast<direction>(d);
        }
    }
    if (NumPlaceTaxi == 0) // Pas trouvé de place de taxi aux allentours...
    {                      // On abandonne la recherche de file de taxi
        FileTaxiTrv = false;
        NumFileTaxi = 0;
        return;
    }

    if ((!cv->v[x][y].PassagePietons) && cv->PlaceTaxi[NumPlaceTaxi - 1].EstTeteFile())
    { // On a finalement trouvé la tête : il attend le taxi (bloque le piéton)
        FileTaxiTrv = false;
        AttendTaxi = true;
        TourAttTaxi = cv->TourCrt;
        Dir = DirPlace;
        return;
    }
    ProchDir = cv->PlaceTaxi[NumPlaceTaxi - 1].DirPlaceSvtMemeFile();
    if (ProchDir != DirPlace)
    {
        if (cv->Voisine(x, y, ProchDir).PietonPeutAller(ProchDir))
        {
            Dir = ProchDir; // Cas ou on longe simplement la file
            return;
        }
    }
    else // Même direction : il faut contourner la case !
    {
        ProchDir = ADroiteDe[DirPlace];
        if (cv->Voisine(x, y, ProchDir).PietonPeutAller(ProchDir) && v->v(ProchDir)->PietonPeutAller(ProchDir))
        {
            Dir = ProchDir;
            return;
        }
        ProchDir = AGaucheDe[DirPlace]; // v3.6.1. Erreur de direction
        if (cv->Voisine(x, y, ProchDir).PietonPeutAller(ProchDir) && v->v(ProchDir)->PietonPeutAller(ProchDir))
        {
            Dir = ProchDir;
            return;
        }
        // On a pas trouvé de direction, on abandonne la recherche
        FileTaxiTrv = false;
        NumFileTaxi = 0;
    }
}
//-----------------------------------------------------------------------------
bool pieton::DecideDeDescendre(int Probabilite) // v6.0.17 : tirage seulement s'il ne veut pas déjà descendre
{
    if (!VeutDescendre)
    {
        VeutDescendre = Probabilite > (alea::Rand() % 100);
    }
    return VeutDescendre;
}
//-----------------------------------------------------------------------------
bool pieton::EstContagieux()
{
    if (EstVivant() && Infection)
    {
        return Infection->EstContagieux(cv->TourCrt);
    }
    return false;
}
//-----------------------------------------------------------------------------
void pieton::EstContaminePar(int NumPieton, int Rang) // v5.4.1. NumPieton>0 (pieton::Numero+1)
{ // Utilisation : UnPieton->EstContaminePar(PietonContagieux->Numero, 1+PietonContagieux->DonneRang())
    if (Infection)
    {
        Infection->Reinfecte(cv->TourCrt, NumPieton, Rang);
    }
    else
    {
        Infection = new infection(cv->TourCrt, NumPieton, Rang);
    }
}
//-----------------------------------------------------------------------------
int pieton::DonneRang() const
{
    if (Infection)
    {
        return Infection->DonneRang(cv->TourCrt);
    }
    return -1;
}
//-----------------------------------------------------------------------------
int pieton::DonneChargeVirale() const
{
    if (Infection)
    {
        return Infection->DonneChargeVirale(cv->TourCrt);
    }
    return 0;
}
//-----------------------------------------------------------------------------
bool pieton::DoitSuccomber() const
{
    if (Infection)
    {
        return Infection->DoitSuccomber(cv->TourCrt);
    }
    return false;
}
//-----------------------------------------------------------------------------
void feu::AjouteFeuDmd(int NumNvFeu)
{
    int i = 0;
    std::vector<int> NvNumFeuxDmd;
    NbFeuxDmd++;
    NvNumFeuxDmd = std::vector<int>(NbFeuxDmd);
    for (i = 0; i < NbFeuxDmd - 1; i++)
    {
        NvNumFeuxDmd[i] = NumFeuxDmd[i];
    }
    NvNumFeuxDmd[NbFeuxDmd - 1] = NumNvFeu;
    NumFeuxDmd = std::move(NvNumFeuxDmd);
}
//-----------------------------------------------------------------------------
void feu::AjouteFeuAtt(int NumNvFeu)
{
    int i = 0;
    std::vector<int> NvNumFeuxAtt;
    NbFeuxAtt++;
    NvNumFeuxAtt = std::vector<int>(NbFeuxAtt);
    for (i = 0; i < NbFeuxAtt - 1; i++)
    {
        NvNumFeuxAtt[i] = NumFeuxAtt[i];
    }
    NvNumFeuxAtt[NbFeuxAtt - 1] = NumNvFeu;
    NumFeuxAtt = std::move(NvNumFeuxAtt);
}
//-----------------------------------------------------------------------------
void feu::AjouteFeuPtn(int NumNvFeu) // v5.2
{
    int i = 0;
    std::vector<int> NvNumFeuxPtn;
    NbFeuxPtn++;
    NvNumFeuxPtn = std::vector<int>(NbFeuxPtn);
    for (i = 0; i < NbFeuxPtn - 1; i++)
    {
        NvNumFeuxPtn[i] = NumFeuxPtn[i];
    }
    NvNumFeuxPtn[NbFeuxPtn - 1] = NumNvFeu;
    NumFeuxPtn = std::move(NvNumFeuxPtn);
}
//-----------------------------------------------------------------------------
void feu::RetireFeuDmd(int NumFeu) // NumFeu(>0) doit être dans NumFeuxDmd[]
{
    int i = 0;
    int Trv = 0;
    std::vector<int> NvNumFeuxDmd;
    if ((--NbFeuxDmd) != 0)
    {
        NvNumFeuxDmd = std::vector<int>(NbFeuxDmd);
        for (i = 0; i < NbFeuxDmd; i++)
        {
            NvNumFeuxDmd[i] = NumFeuxDmd[i + (Trv |= static_cast<int>(NumFeuxDmd[i] == NumFeu))];
        }
    }
    NumFeuxDmd = std::move(NvNumFeuxDmd);
}
//-----------------------------------------------------------------------------
void feu::RetireFeuAtt(int NumFeu) // NumFeu(>0) doit être dans NumFeuxAtt[]
{
    int i = 0;
    int Trv = 0;
    std::vector<int> NvNumFeuxAtt;
    if ((--NbFeuxAtt) != 0)
    {
        NvNumFeuxAtt = std::vector<int>(NbFeuxAtt);
        for (i = 0; i < NbFeuxAtt; i++)
        {
            NvNumFeuxAtt[i] = NumFeuxAtt[i + (Trv |= static_cast<int>(NumFeuxAtt[i] == NumFeu))];
        }
    }
    NumFeuxAtt = std::move(NvNumFeuxAtt);
}
//-----------------------------------------------------------------------------
void feu::ChangeEtat()
{
    const bool ChgEtat = (Etat != ProchainEtat);
    Etat = ProchainEtat;
    if (ChgEtat)
    {
        vue->RedessineCase(x, y, false); // portage
    }
}
//-----------------------------------------------------------------------------
void feu::DefinitProchainEtat()
{
    int i = 0;
    TourDrnChgEtat = cv->TourCrt;
    if (Etat != 0)
    {
        ProchainEtat = static_cast<etat_feu>(1 + (Etat % 3));
    }
    switch (ProchainEtat)
    {
    case vert:
        DemandeVert = false;
        FeuxTousRouges = false;
        ProchainVert = false;
        break;
    case rouge:
        RougeDemande = false;
        // On informe quels sont les prochains feux à se mettre au vert
        for (i = 0; i < NbFeuxDmd; i++)
        {
            if ((cv->Feu[NumFeuxDmd[i] - 1].Etat == rouge) && (!cv->Feu[NumFeuxDmd[i] - 1].ProchainVert) &&
                (cv->Feu[NumFeuxDmd[i] - 1].DemandeVert || (cv->Feu[NumFeuxDmd[i] - 1].TourDrnChgEtat < cv->TourCrt)))
            {
                cv->Feu[NumFeuxDmd[i] - 1].ProchainVert = true;
                cv->Feu[NumFeuxDmd[i] - 1].TourDrnChgEtat = cv->TourCrt;
            }
        }
        // v5.2 : ...Et les feux piétons liés...
        for (i = 0; i < NbFeuxPtn; i++)
        {
            if ((cv->FeuP[NumFeuxPtn[i] - 1].Etat == rouge) && // ETAT=Etat pour les feux adverses et non l'état pour
                                                               // les piétons (compte le temps de traversée
                (!cv->FeuP[NumFeuxPtn[i] - 1].ProchainVert) &&
                (cv->FeuP[NumFeuxPtn[i] - 1].DemandeVert || (cv->FeuP[NumFeuxPtn[i] - 1].TourDrnChgEtat < cv->TourCrt)))
            {
                cv->FeuP[NumFeuxPtn[i] - 1].ProchainVert = true;
                cv->FeuP[NumFeuxPtn[i] - 1].TourDrnChgEtat = cv->TourCrt;
            }
        }
        break;
    default:
        break;
    }
}
//-----------------------------------------------------------------------------
void feu::Evolue()
{
    int i = 0;
    // Un mobile (véhicule, vehlib, bus, tram, taxi) de numéro Num est sur la case dans la direction du feu
    const auto SurLaCase = [this](int Num, const auto &Mobiles) { return (Num != 0) && (Mobiles[Num - 1].Dir == Dir); };
    ProchainEtat = Etat; // Par défaut
    switch (Etat)
    {
    case inactif:
        break;
    case vert: // Si (rouge demandé ou passage immediat) et durée vert écoulée -> Orange
        if (((RougeDemande && (TourDrnChgEtat < cv->TourCrt)) || (TypePsgRouge == immediat)) &&
            (cv->TourCrt - TourDrnChgEtat > DureeVert))
        {
            DefinitProchainEtat();
        }
        break;
    case orange:
        if (cv->TourCrt - TourDrnChgEtat > cv->DureeOrange)
        {
            DefinitProchainEtat();
        }
        break;
    case rouge:                                                                     // Le vert n'est pas encore demandé
        if ((!DemandeVert) && ((cv->TourCrt - TourDrnChgEtat > DureeRouge) &&       // la durée du rouge est écoulée ET
                               ((TypeDmdVert == immediate) ||                       // La demande est immédiate Ou
                                SurLaCase(cv->v[x][y].NumVehicule, cv->Vehicule) || // un véhicule... Ou
                                SurLaCase(cv->v[x][y].NumVehlib, cv->Vehlib) ||     // (v5.3) un vehlib... Ou
                                SurLaCase(cv->v[x][y].NumBus, cv->Bus) ||           // un bus... Ou
                                SurLaCase(cv->v[x][y].NumTram, cv->Tram) ||         // (v3.5) un tram... Ou
                                SurLaCase(cv->v[x][y].NumTaxi, cv->Taxi))))         // (v3.6) un taxi
        { // Alors...On demande le vert DES QU'UN DES FEUX (devant être rouge) EST VERT
            for (i = 0; (i < NbFeuxAtt) && ((cv->Feu[NumFeuxAtt[i] - 1].Etat != vert) ||
                                            ((cv->Feu[NumFeuxAtt[i] - 1].TourDrnChgEtat == cv->TourCrt) &&
                                             (!cv->Feu[NumFeuxAtt[i] - 1].RougeDemande)));
                 i++)
            {
                ;
            }
            // v5.2 : on vérifie les feux piétons (comme ci-dessus)
            DemandeVert = (i < NbFeuxAtt);
            if (!DemandeVert) // Si pas de demande de vert dans les feux de circulation
            {
                for (i = 0; (i < NbFeuxPtn) && ((cv->FeuP[NumFeuxPtn[i] - 1].Etat != vert) || // ETAT=Etat pour les
                                                                                              // feux adverses et
                                                                                              // non l'état pour les
                                                                                              // piétons (compte le
                                                                                              // temps de traversée
                                                ((cv->FeuP[NumFeuxPtn[i] - 1].TourDrnChgEtat == cv->TourCrt) &&
                                                 (!cv->FeuP[NumFeuxPtn[i] - 1].RougeDemande)));
                     i++)
                {
                    ;
                }
                DemandeVert = (i < NbFeuxPtn);
            }
            if (DemandeVert)
            {
                TourDrnChgEtat = cv->TourCrt; // v5.2: DemandeVert est défini ci-dessus
            }
        }
        // Le vert est demandé...
        if (DemandeVert)
        {
            // On le fait toujours savoir (si jamais un feu devient vert entre-temps)
            for (i = 0; i < NbFeuxAtt; i++)
            {                                                    // On parcours les feux devant être rouge
                if ((cv->Feu[NumFeuxAtt[i] - 1].Etat == vert) && // feu opposé vert et
                    (cv->Feu[NumFeuxAtt[i] - 1].TourDrnChgEtat < cv->TourCrt) &&
                    (!cv->Feu[NumFeuxAtt[i] - 1].RougeDemande)) // le rouge n'a pas encore été demandé)
                {
                    cv->Feu[NumFeuxAtt[i] - 1].RougeDemande = true;          // Ben maintenant...
                    cv->Feu[NumFeuxAtt[i] - 1].TourDrnChgEtat = cv->TourCrt; //  c'est fait !
                }
            }
            // v5.2 : Idem pour les feux de piétons
            for (i = 0; i < NbFeuxPtn; i++)
            {                                                     // On parcours les feux piétons devant être rouge
                if ((cv->FeuP[NumFeuxPtn[i] - 1].Etat == vert) && // feu opposé vert et (ETAT=Etat pour les feux
                                                                  // adverses et non l'état pour les piétons (compte le
                                                                  // temps de traversée)
                    (cv->FeuP[NumFeuxPtn[i] - 1].TourDrnChgEtat < cv->TourCrt) &&
                    (!cv->FeuP[NumFeuxPtn[i] - 1].RougeDemande)) // le rouge n'a pas encore été demandé)
                {
                    cv->FeuP[NumFeuxPtn[i] - 1].RougeDemande = true;          // Ben maintenant...
                    cv->FeuP[NumFeuxPtn[i] - 1].TourDrnChgEtat = cv->TourCrt; //  c'est fait !
                }
            }
        }
        // On vérifie que les feux opposés ne sont pas déjà tous rouges (vert demandé ou non)
        if (!FeuxTousRouges)
        {
            for (i = 0; (i < NbFeuxAtt) && (cv->Feu[NumFeuxAtt[i] - 1].Etat == rouge) &&
                        (cv->Feu[NumFeuxAtt[i] - 1].DemandeVert || cv->Feu[NumFeuxAtt[i] - 1].ProchainVert ||
                         (cv->Feu[NumFeuxAtt[i] - 1].TourDrnChgEtat < cv->TourCrt));
                 i++)
            {
                ;
            }
            // v5.2 : Idem pour les feux piétons
            FeuxTousRouges = (i == NbFeuxAtt);
            if (FeuxTousRouges) // Si Feux de circulation tous Rouges
            {
                for (i = 0; (i < NbFeuxPtn) && (cv->FeuP[NumFeuxPtn[i] - 1].Etat == rouge) &&
                            (cv->FeuP[NumFeuxPtn[i] - 1].DemandeVert || cv->FeuP[NumFeuxPtn[i] - 1].ProchainVert ||
                             (cv->FeuP[NumFeuxPtn[i] - 1].TourDrnChgEtat < cv->TourCrt));
                     i++)
                {
                    ;
                }
                FeuxTousRouges = (i == NbFeuxPtn);
            }
            if (FeuxTousRouges)
            {
                TourDrnChgEtat = cv->TourCrt; // v5.2 : FeuxTousRouges défini ci-dessus
            }
        }
        // On passe au vert si on le demande ou on l'autorise (vert demandé ou non)
        if (FeuxTousRouges && ProchainVert && (TourDrnChgEtat < cv->TourCrt))
        {
            DefinitProchainEtat();
        }
        break;
    }
}
//-----------------------------------------------------------------------------
bool feu::Charge(int hfile)
{
    int i = 0;
    bool Ok = false;
    Ok = LitChamp(hfile, &Numero, sizeof(Numero)) && LitChamp(hfile, &x, sizeof(x)) && LitChamp(hfile, &y, sizeof(y)) &&
         LitChamp(hfile, &Dir, sizeof(Dir)) && LitChamp(hfile, &Etat, sizeof(Etat)) &&
         LitChamp(hfile, &NbFeuxDmd, sizeof(NbFeuxDmd)) && LitChamp(hfile, &NbFeuxAtt, sizeof(NbFeuxAtt)) &&
         // v5.2 : /!\ On ne charge pas les feux de piétons
         LitChamp(hfile, &TypeDmdVert, sizeof(TypeDmdVert)) && LitChamp(hfile, &TypePsgRouge, sizeof(TypePsgRouge)) &&
         LitChamp(hfile, &DureeRouge, sizeof(DureeRouge)) && LitChamp(hfile, &DureeVert, sizeof(DureeVert)) &&
         LitChamp(hfile, &DemandeVert, sizeof(DemandeVert)) && LitChamp(hfile, &RougeDemande, sizeof(RougeDemande));

    Ok = Ok && NombreCredible(NbFeuxDmd) && NombreCredible(NbFeuxAtt); // v6.0.2
    if (Ok)
    {
        if (NbFeuxDmd != 0)
        {
            NumFeuxDmd = std::vector<int>(NbFeuxDmd);
        }
        if (NbFeuxAtt != 0)
        {
            NumFeuxAtt = std::vector<int>(NbFeuxAtt);
        }
        // v5.2 : /!\ On ne charge pas les feux de piétons
    }
    for (i = 0; Ok && (i < NbFeuxDmd); i++)
    {
        Ok = LitChamp(hfile, &NumFeuxDmd[i], sizeof(NumFeuxDmd[i]));
    }
    for (i = 0; Ok && (i < NbFeuxAtt); i++)
    {
        Ok = LitChamp(hfile, &NumFeuxAtt[i], sizeof(NumFeuxAtt[i]));
    }
    // v5.2 : /!\ On ne charge pas les feux de piétons
    return Ok;
}
//-----------------------------------------------------------------------------
bool feu::Sauve(int hfile)
{
    int i = 0;
    int Ok = 0;
    int xc = CoordonneeCopiee(x, cv->Xc);
    int yc = CoordonneeCopiee(y, cv->Yc); // v4.0.1. v6.0.1 : COORDCOPIE
    Ok = static_cast<int>((static_cast<int>(FileWrite(hfile, &Numero, sizeof(Numero)) != 0) != 0) &&
                          (FileWrite(hfile, cv->CopieEnCours ? &xc : &x, sizeof(x)) != 0) && // v4.0.1
                          (FileWrite(hfile, cv->CopieEnCours ? &yc : &y, sizeof(y)) != 0) && // v4.0.1
                          (FileWrite(hfile, &Dir, sizeof(Dir)) != 0) && (FileWrite(hfile, &Etat, sizeof(Etat)) != 0) &&
                          (FileWrite(hfile, &NbFeuxDmd, sizeof(NbFeuxDmd)) != 0) &&
                          (FileWrite(hfile, &NbFeuxAtt, sizeof(NbFeuxAtt)) != 0) &&
                          // v5.2 : /!\ On ne sauve pas les feux de piétons
                          (FileWrite(hfile, &TypeDmdVert, sizeof(TypeDmdVert)) != 0) &&
                          (FileWrite(hfile, &TypePsgRouge, sizeof(TypePsgRouge)) != 0) &&
                          (FileWrite(hfile, &DureeRouge, sizeof(DureeRouge)) != 0) &&
                          (FileWrite(hfile, &DureeVert, sizeof(DureeVert)) != 0) &&
                          (FileWrite(hfile, &DemandeVert, sizeof(DemandeVert)) != 0) &&
                          (FileWrite(hfile, &RougeDemande, sizeof(RougeDemande)) != 0));

    for (i = 0; (Ok != 0) && (i < NbFeuxDmd); i++)
    {
        Ok = FileWrite(hfile, &NumFeuxDmd[i], sizeof(NumFeuxDmd[i]));
    }
    for (i = 0; (Ok != 0) && (i < NbFeuxAtt); i++)
    {
        Ok = FileWrite(hfile, &NumFeuxAtt[i], sizeof(NumFeuxAtt[i]));
    }
    // v5.2 : /!\ On ne sauve pas les feux de piétons
    return Ok != 0;
}
//-----------------------------------------------------------------------------
void feu::Verifie(AnsiString &asMsgErr, const AnsiString &asSource)
{
    int i = 0;
    if (Numero < 0)
    {
        asMsgErr += asSource + Format(": numéro incorrect (%d)\n", TableauDeConstantes(Numero));
    }
    VerifiePosition(asMsgErr, asSource, x, y, DOIT_ETRE_DEFINI);
    VerifieDirection(asMsgErr, asSource, static_cast<direction>(Dir), DOIT_ETRE_DEFINI);
    VerifieEtat(asMsgErr, asSource, Etat);
    VerifieEtat(asMsgErr, asSource, ProchainEtat);

    VerifEnCours = true; // Evite les boucles infinies dues aux pointages mutuels de références

    if ((NbFeuxDmd < 0) || (NbFeuxDmd > cv->NbFeux))
    {
        asMsgErr +=
            asSource + Format(": nombre des prochains feux verts incorrect (%d)\n", TableauDeConstantes(NbFeuxDmd));
    }
    else
    {
        for (i = 0; i < NbFeuxDmd; i++)
        {
            if ((NumFeuxDmd[i] < 1) || (NumFeuxDmd[i] > cv->NbFeux))
            { // v3.7.2 : > strict
                asMsgErr += asSource + Format(": numéro du %d° prochain feu vert incorrect (%d)\n",
                                              TableauDeConstantes(i + 1, NumFeuxDmd[i]));
            }
            else if (!cv->Feu[NumFeuxDmd[i] - 1].VerifEnCours)
            {
                cv->Feu[NumFeuxDmd[i] - 1].Verifie(
                    asMsgErr, Format("%d° prochain feu vert [n°%d, (%d,%d)]\n",
                                     TableauDeConstantes(i + 1, NumFeuxDmd[i], cv->Feu[NumFeuxDmd[i] - 1].x,
                                                         cv->Feu[NumFeuxDmd[i] - 1].y)));
            }
        }
    }

    if ((NbFeuxAtt < 0) || (NbFeuxAtt > cv->NbFeux))
    {
        asMsgErr +=
            asSource + Format(": nombre des feux devant être rouge incorrect (%d)\n", TableauDeConstantes(NbFeuxAtt));
    }
    else
    {
        for (i = 0; i < NbFeuxAtt; i++)
        {
            if ((NumFeuxAtt[i] < 1) || (NumFeuxAtt[i] > cv->NbFeux))
            { // v3.7.2 : > strict
                asMsgErr += asSource + Format(": numéro du %d° feu devant être rouge incorrect (%d)\n",
                                              TableauDeConstantes(i + 1, NumFeuxAtt[i]));
            }
            else if (!cv->Feu[NumFeuxAtt[i] - 1].VerifEnCours)
            {
                cv->Feu[NumFeuxAtt[i] - 1].Verifie(
                    asMsgErr, Format("%d° feu devant être rouge [n°%d, (%d,%d)]\n",
                                     TableauDeConstantes(i + 1, NumFeuxAtt[i], cv->Feu[NumFeuxAtt[i] - 1].x,
                                                         cv->Feu[NumFeuxAtt[i] - 1].y)));
            }
        }
    }

    VerifEnCours = false;

    VerifieDmdVert(asMsgErr, asSource, TypeDmdVert);
    VerifiePsgRouge(asMsgErr, asSource, TypePsgRouge);

    if (DemandeVert && (Etat != rouge))
    {
        asMsgErr += asSource + ": demande le vert alors qu'il n'est pas rouge\n";
    }
    if (RougeDemande && (Etat != vert))
    {
        asMsgErr += asSource + ": le rouge est demandé alors qu'il n'est pas vert\n";
    }
}
//-----------------------------------------------------------------------------
void feu_pieton::Initialise() // v5.2
{
    for (int i = 0; i < NBFEUXPIETONSPARNUMERO; i++)
    {
        x[i] = y[i] = -1;
        Dir[i] = indefinie;
    }
    for (int &i : iFeuDir)
    {
        i = 0;
    }
    ProchainEtat = Etat = inactif;
    TypeDmdVert = arrivee_vehicule;
    TypePsgRouge = prm_dmd;
    DureeTraversee = DureeVert = DureeRouge = NbFeux = 0;
    TourDrnChgEtat = 0L;
    ProchainVert = RougeDemande = FeuxTousRouges = DemandeVert = false;
    VerifEnCours = false;
}
//-----------------------------------------------------------------------------
void feu_pieton::Copie(feu_pieton *f) // v5.2
{
    Numero = f->Numero;
    for (int i = 0; i < NBFEUXPIETONSPARNUMERO; i++)
    {
        x[i] = f->x[i];
        y[i] = f->y[i];
        Dir[i] = f->Dir[i];
    }
    for (int i = 0; i < NBDIR; i++)
    {
        iFeuDir[i] = f->iFeuDir[i];
    }
    Etat = f->Etat;
    ProchainEtat = f->ProchainEtat;
    NbFeux = f->NbFeux;
    NumFeux = std::move(f->NumFeux);
    DureeRouge = f->DureeRouge;
    DureeVert = f->DureeVert;
    DureeTraversee = f->DureeTraversee;
    TourDrnChgEtat = f->TourDrnChgEtat;
    TypeDmdVert = f->TypeDmdVert;
    TypePsgRouge = f->TypePsgRouge;
    DemandeVert = f->DemandeVert;
    FeuxTousRouges = f->FeuxTousRouges;
    RougeDemande = f->RougeDemande;
    ProchainVert = f->ProchainVert;
    VerifEnCours = f->VerifEnCours;
}
//-----------------------------------------------------------------------------
void feu_pieton::DefinitFeuOppose()
{ // On suppose ici que :
  //  - x[0],y[0],Dir[0] sont définis
  //  - On peut poser le feu opposé (contrôle déjà effectué)
  //  - v[x[0]][y[0]].NumFeuP[Dir[0]-1]=Numero
    int k = 0;
    for (k = 1; cv->Voisine(x[0], y[0], k, Dir[0]).PassagePietons; k++)
    {
        ;
    }
    const int xx = (x[0] + (k * dx[Dir[0]])) % cv->NbX;
    const int yy = (y[0] + (k * dy[Dir[0]])) % cv->NbY;
    const direction dd = InvDir[Dir[0]];
    x[1] = xx;
    y[1] = yy;
    Dir[1] = dd;
    iFeuDir[Dir[1] - 1] = 1;
    cv->v[xx][yy].NumFeuP[dd - 1] = Numero;
    DureeTraversee = (2 * (k - 1)) + 1;
}
//-----------------------------------------------------------------------------
void feu_pieton::AjouteFeu(int NumNvFeu) // v5.2
{
    int i = 0;
    std::vector<int> NvNumFeux;
    NbFeux++;
    NvNumFeux = std::vector<int>(NbFeux);
    for (i = 0; i < NbFeux - 1; i++)
    {
        NvNumFeux[i] = NumFeux[i];
    }
    NvNumFeux[NbFeux - 1] = NumNvFeu;
    NumFeux = std::move(NvNumFeux);
}
//-----------------------------------------------------------------------------
void feu_pieton::RetireFeu(int NumFeu) // //v5.2
{
    int i = 0;
    int Trv = 0;
    std::vector<int> NvNumFeux;
    if ((--NbFeux) != 0)
    {
        NvNumFeux = std::vector<int>(NbFeux);
        for (i = 0; i < NbFeux; i++)
        {
            NvNumFeux[i] = NumFeux[i + (Trv |= static_cast<int>(NumFeux[i] == NumFeu))];
        }
    }
    NumFeux = std::move(NvNumFeux);
}
//-----------------------------------------------------------------------------
void feu_pieton::ChangeEtat()
{
    const bool ChgEtat = (Etat != ProchainEtat);
    Etat = ProchainEtat;
    if (ChgEtat)
    {
        for (int i = 0; i < NBFEUXPIETONSPARNUMERO; i++)
        {
            vue->RedessineCase(x[i], y[i], false); // portage
        }
    }
}
//-----------------------------------------------------------------------------
void feu_pieton::DefinitProchainEtat()
{
    TourDrnChgEtat = cv->TourCrt;
    if (Etat != 0)
    {
        ProchainEtat = static_cast<etat_feu>(4 - Etat);
    }
    switch (ProchainEtat)
    {
    case vert:
        DemandeVert = false;
        FeuxTousRouges = false;
        ProchainVert = false;
        break;
    case rouge:
        RougeDemande = false;
        break;
    default:
        break;
    }
}
//-----------------------------------------------------------------------------
bool feu_pieton::PietonAttend()
{
    int NumPieton = 0;
    bool Attend = false; // Pessimiste
    for (int i = 0; (!Attend) && (i < NBFEUXPIETONSPARNUMERO); i++)
    {
        for (int j = 0; (!Attend) && (j < NBMAXPIETONSPARCASE); j++)
        {
            NumPieton = cv->v[x[i]][y[i]].NumPieton[j];
            if (NumPieton != 0)
            {
                pieton *p = &cv->Pieton[NumPieton - 1];
                Attend = p->PsgPtTrv && (p->Dir == Dir[i]); // Le piéton attend dans le sens du feu piéton.
            }
        }
    }
    return Attend;
}
//-----------------------------------------------------------------------------
void feu_pieton::Evolue()
{
    int i = 0;
    ProchainEtat = Etat; // Par défaut
                         //  pieton *p;
    switch (Etat)
    {
    case inactif:
        break;
    case vert:
        if (cv->TourCrt - TourDrnChgEtat <= DureeTraversee)
        {
            break; // On attend que la traversée se passe avant d'agir
        }
        // Si (rouge demandé ou passage immediat) et durée vert écoulée -> Rouge
        if (((RougeDemande && (TourDrnChgEtat < cv->TourCrt)) || (TypePsgRouge == immediat)) &&
            (cv->TourCrt - TourDrnChgEtat > DureeVert))
        {
            DefinitProchainEtat();
        }
        break;
    case rouge:
        if (cv->TourCrt - TourDrnChgEtat == DureeTraversee)
        { // On attend que le temps minimum de traversée se termine
            // Le bloc for ci-dessous a été déplacé de DefinitProchainEtat jusqu'ici car il y a un décalage de
            // DureeTraversee On informe quels sont les prochains feux à se mettre au vert
            for (i = 0; i < NbFeux; i++)
            {
                if ((cv->Feu[NumFeux[i] - 1].Etat == rouge) && (!cv->Feu[NumFeux[i] - 1].ProchainVert) &&
                    (cv->Feu[NumFeux[i] - 1].DemandeVert || (cv->Feu[NumFeux[i] - 1].TourDrnChgEtat < cv->TourCrt)))
                {
                    cv->Feu[NumFeux[i] - 1].ProchainVert = true;
                    cv->Feu[NumFeux[i] - 1].TourDrnChgEtat = cv->TourCrt;
                }
            }
        }
        if ((!DemandeVert) &&                               // Le vert n'est pas encore demandé
            ((cv->TourCrt - TourDrnChgEtat > DureeRouge) && // la durée du rouge est écoulée ET
             ((TypeDmdVert == immediate) ||                 // La demande est immédiate Ou
              PietonAttend())))                             // un pieton attend au pied d'un des feux de piétons
        { // Alors...On demande le vert DES QU'UN DES FEUX (devant être rouge) EST VERT
            for (i = 0; (i < NbFeux) && ((cv->Feu[NumFeux[i] - 1].Etat != vert) ||
                                         ((cv->Feu[NumFeux[i] - 1].TourDrnChgEtat == cv->TourCrt) &&
                                          (!cv->Feu[NumFeux[i] - 1].RougeDemande)));
                 i++)
            {
                ;
            }
            DemandeVert = (i < NbFeux);
            if (DemandeVert)
            {
                TourDrnChgEtat = cv->TourCrt;
            }
        }
        // Le vert est demandé...
        if (DemandeVert)
        {
            // On le fait toujours savoir (si jamais un feu devient vert entre-temps)
            for (i = 0; i < NbFeux; i++)
            {                                                 // On parcours les feux devant être rouge
                if ((cv->Feu[NumFeux[i] - 1].Etat == vert) && // feu opposé vert et
                    (cv->Feu[NumFeux[i] - 1].TourDrnChgEtat < cv->TourCrt) &&
                    (!cv->Feu[NumFeux[i] - 1].RougeDemande)) // le rouge n'a pas encore été demandé)
                {
                    cv->Feu[NumFeux[i] - 1].RougeDemande = true;          // Ben maintenant...
                    cv->Feu[NumFeux[i] - 1].TourDrnChgEtat = cv->TourCrt; //  c'est fait !
                }
            }
        }
        // On vérifie que les feux opposés ne sont pas déjà tous rouges (vert demandé ou non)
        if (!FeuxTousRouges)
        {
            for (i = 0; (i < NbFeux) && (cv->Feu[NumFeux[i] - 1].Etat == rouge) &&
                        (cv->Feu[NumFeux[i] - 1].DemandeVert || cv->Feu[NumFeux[i] - 1].ProchainVert ||
                         (cv->Feu[NumFeux[i] - 1].TourDrnChgEtat < cv->TourCrt));
                 i++)
            {
                ;
            }
            FeuxTousRouges = (i == NbFeux);
            if (FeuxTousRouges)
            {
                TourDrnChgEtat = cv->TourCrt;
            }
        }
        // On passe au vert si on le demande ou on l'autorise (vert demandé ou non)
        if (FeuxTousRouges && ProchainVert && (TourDrnChgEtat < cv->TourCrt))
        {
            DefinitProchainEtat();
        }
        break;
    default:
        break; // v5.4 : évite l'avertissement
    }
}
//-----------------------------------------------------------------------------
bool feu_pieton::Charge(int hfile)
{
    int i = 0;
    bool Ok = false;
    Ok = LitChamp(hfile, &Numero, sizeof(Numero)) && LitChamp(hfile, x.data(), NBFEUXPIETONSPARNUMERO * sizeof(x[0])) &&
         LitChamp(hfile, y.data(), NBFEUXPIETONSPARNUMERO * sizeof(y[0])) &&
         LitChamp(hfile, Dir.data(), NBFEUXPIETONSPARNUMERO * sizeof(Dir[0])) &&
         LitChamp(hfile, iFeuDir.data(), NBDIR * sizeof(iFeuDir[0])) && LitChamp(hfile, &Etat, sizeof(Etat)) &&
         LitChamp(hfile, &NbFeux, sizeof(NbFeux)) && LitChamp(hfile, &TypeDmdVert, sizeof(TypeDmdVert)) &&
         LitChamp(hfile, &TypePsgRouge, sizeof(TypePsgRouge)) && LitChamp(hfile, &DureeRouge, sizeof(DureeRouge)) &&
         LitChamp(hfile, &DureeVert, sizeof(DureeVert)) && LitChamp(hfile, &DemandeVert, sizeof(DemandeVert)) &&
         LitChamp(hfile, &RougeDemande, sizeof(RougeDemande));
    Ok = Ok && NombreCredible(NbFeux); // v6.0.2
    if (Ok && (NbFeux != 0))
    {
        NumFeux = std::vector<int>(NbFeux);
    }

    for (i = 0; Ok && (i < NbFeux); i++)
    {
        Ok = LitChamp(hfile, &NumFeux[i], sizeof(NumFeux[i]));
    }
    if (Ok)
    {
        DureeTraversee = (2 * (abs(x[1] - x[0]) + abs(y[1] - y[0]) - 1)) + 1;
    }
    return Ok;
}
//-----------------------------------------------------------------------------
bool feu_pieton::Sauve(int hfile)
{
    int i = 0;
    int Ok = 0;
    const std::array<int, NBFEUXPIETONSPARNUMERO> xc = {CoordonneeCopiee(x[0], cv->Xc),
                                                        CoordonneeCopiee(x[1], cv->Xc)}; // xc,yc :
                                                                                         // gestion copie
                                                                                         // vers fichier
                                                                                         // (v4.0.1)
    const std::array<int, NBFEUXPIETONSPARNUMERO> yc = {CoordonneeCopiee(y[0], cv->Yc),
                                                        CoordonneeCopiee(y[1], cv->Yc)}; // v6.0.1 :
                                                                                         // COORDCOPIE
    Ok = static_cast<int>(
        (static_cast<int>(FileWrite(hfile, &Numero, sizeof(Numero)) != 0) != 0) &&
        (FileWrite(hfile, (cv->CopieEnCours ? xc : x).data(), NBFEUXPIETONSPARNUMERO * sizeof(x[0])) != 0) &&
        (FileWrite(hfile, (cv->CopieEnCours ? yc : y).data(), NBFEUXPIETONSPARNUMERO * sizeof(y[0])) != 0) &&
        (FileWrite(hfile, Dir.data(), NBFEUXPIETONSPARNUMERO * sizeof(Dir[0])) != 0) &&
        (FileWrite(hfile, iFeuDir.data(), NBDIR * sizeof(iFeuDir[0])) != 0) &&
        (FileWrite(hfile, &Etat, sizeof(Etat)) != 0) && (FileWrite(hfile, &NbFeux, sizeof(NbFeux)) != 0) &&
        (FileWrite(hfile, &TypeDmdVert, sizeof(TypeDmdVert)) != 0) &&
        (FileWrite(hfile, &TypePsgRouge, sizeof(TypePsgRouge)) != 0) &&
        (FileWrite(hfile, &DureeRouge, sizeof(DureeRouge)) != 0) &&
        (FileWrite(hfile, &DureeVert, sizeof(DureeVert)) != 0) &&
        (FileWrite(hfile, &DemandeVert, sizeof(DemandeVert)) != 0) &&
        (FileWrite(hfile, &RougeDemande, sizeof(RougeDemande)) != 0));
    for (i = 0; (Ok != 0) && (i < NbFeux); i++)
    {
        Ok = FileWrite(hfile, &NumFeux[i], sizeof(NumFeux[i]));
    }
    return Ok != 0;
}
//-----------------------------------------------------------------------------
void parking::PlaceProchainVehicule()
{
    int i = 0;
    int NumVehicule = 0;
    if (cv->v[x][y].NumVehicule == 0)
    {
        for (i = 0; (NumVehicule == 0) && (i < cv->NbVehicules); i++)
        {
            NumVehicule = static_cast<int>((cv->Vehicule[i].NumParkingDepart == Numero) && (cv->Vehicule[i].x == x) &&
                                           (cv->Vehicule[i].y == y)) *
                          (i + 1);
        }
        if (NumVehicule != 0) // Il y a encore un véhicule à sortir
        {
            cv->v[x][y].NumVehicule = NumVehicule;
            cv->PlaceVehicule(x, y, NumVehicule); // v2.2.7. Inclut les stats d'Attente/Trafic
            cv->Vehicule[NumVehicule - 1].TourDepart = cv->TourCrt;
            cv->NbVehiculesSortis++;
        }
    }
}
//-----------------------------------------------------------------------------
bool parking::Charge(int hfile)
{
    return LitChamp(hfile, &Numero, sizeof(Numero)) && LitChamp(hfile, &x, sizeof(x)) && LitChamp(hfile, &y, sizeof(y));
}
//-----------------------------------------------------------------------------
bool parking::Sauve(int hfile)
{
    int xc = CoordonneeCopiee(x, cv->Xc);
    int yc = CoordonneeCopiee(y, cv->Yc); // v4.0.1. v6.0.1 : COORDCOPIE
    return (FileWrite(hfile, &Numero, sizeof(Numero)) != 0) &&
           (FileWrite(hfile, cv->CopieEnCours ? &xc : &x, sizeof(x)) != 0) && // v4.0.1
           (FileWrite(hfile, cv->CopieEnCours ? &yc : &y, sizeof(y)) != 0);   // v4.0.1
}
//-----------------------------------------------------------------------------
void parking::Verifie(AnsiString &asMsgErr) const
{
    const AnsiString asSource = Format("Parking n°%d (%d,%d)", TableauDeConstantes(Numero, x, y));
    if (Numero < 0)
    {
        asMsgErr += asSource + Format(": numéro incorrect (%d)\n", TableauDeConstantes(Numero));
    }
    VerifiePosition(asMsgErr, asSource, x, y, DOIT_ETRE_DEFINI);
}
//-----------------------------------------------------------------------------
void vehicule::Definit(int x0, int y0, direction Dir0, int NumDepart, int NumArrivee, int NumFileDepart,
                       int NumFileArrivee,           // v5.0
                       niveau_priorite NivPriorite0) // v2.2
{
    x = x0;
    y = y0;
    Dir = Dir0;
    NumParkingDepart = NumDepart;
    NumParkingArrivee = NumArrivee;
    NumFileParkDepart = NumFileDepart;
    NumFileParkArrivee = NumFileArrivee;
    NivPriorite = NivPriorite0;
}
//-----------------------------------------------------------------------------
bool vehicule::EstDestination(const voie *v) const // v5.0 : inclut les parkings extérieurs
{
    if (NumParkingArrivee != 0)
    {
        return (NumParkingArrivee == v->NumParking);
    }
    if ((NumFileParkArrivee != 0) && (v->NumPlacePark != 0))
    {
        return (NumFileParkArrivee == cv->PlacePark[v->NumPlacePark - 1].NumFile + 1); // v5.2.3 : NumFile+1 et non
                                                                                       // NumFile
    }
    return false;
}
//-----------------------------------------------------------------------------
void vehicule::CalculeProchaineDirection()
{
    voie *v = &cv->v[x][y];
    voie *Voie = nullptr;
    const direction AncDir = Dir;                    // v2.2.4
    const bool Prioritaire = (NivPriorite > normal); // v3.5 (pour DistParking)
    std::array<int, NBDIR> OrdreDir = {0, 0, 0, 0};
    int i = 0;
    int j = 0;
    const int ndp = NbDir[v->DirPoss];
    Dir = indefinie;

    // On classe les dir.poss. dans l'ordre < des distances aux parkings.
    // En cas d'égalité, c'est le hasard : (N°Veh.+Tour) Modulo Directions Possibles
    if (NumFileParkArrivee != 0)
    { // v5.0 : recopie du calcul d'OrdreDir puis remplacement des occurrences "Parking" par "FilePark"
        for (i = 0; i < ndp; i++)
        {
            for (j = 0; j < ndp; j++)
            {
                if (i != j)
                { // On dégringole si distance supérieure.
                    OrdreDir[i] += static_cast<int>(
                        (static_cast<int>(
                             (cv->Voisine(x, y, Dirdp[j][v->DirPoss]).NumPlacePark != 0) && // 1. File recherchée
                             (cv->PlacePark[cv->Voisine(x, y, Dirdp[j][v->DirPoss]).NumPlacePark - 1].NumFile + 1 ==
                              NumFileParkArrivee)) > // v5.2.3 : NumFile+1 et non NumFile
                         static_cast<int>(
                             (cv->Voisine(x, y, Dirdp[i][v->DirPoss]).NumPlacePark != 0) &&
                             (cv->PlacePark[cv->Voisine(x, y, Dirdp[i][v->DirPoss]).NumPlacePark - 1].NumFile + 1 ==
                              NumFileParkArrivee))) || // 2. Distance à la file. v5.2.3 : NumFile+1 et non NumFile
                        ((v->DistFilePark()[NumFileParkArrivee - 1][i][Prioritaire ? 1 : 0] >
                          v->DistFilePark()[NumFileParkArrivee - 1][j][Prioritaire ? 1 : 0]) ||
                         ((v->DistFilePark()[NumFileParkArrivee - 1][i][Prioritaire ? 1 : 0] ==
                           v->DistFilePark()[NumFileParkArrivee - 1][j][Prioritaire ? 1 : 0]) &&
                          ((Dirdp[i][v->DirPoss] != AncDir) || // En cas d'égalité, préfère garder la direction courante
                                                               // : v2.2.4.  // v3.5 : || par && !
                           ((i + Numero + cv->TourCrt) % ndp > (j + Numero + cv->TourCrt) % ndp))))); // sinon tirage au
                                                                                                      // hasard (Ici
                                                                                                      // TourCrt+1)
                }
            }
        }
    }
    if (NumParkingArrivee != 0)
    {
        for (i = 0; i < ndp; i++)
        {
            for (j = 0; j < ndp; j++)
            {
                if (i != j)
                { // On dégringole si distance supérieure.
                    OrdreDir[i] += static_cast<int>(
                        (v->DistParking()[NumParkingArrivee - 1][i][Prioritaire ? 1 : 0] >
                         v->DistParking()[NumParkingArrivee - 1][j][Prioritaire ? 1 : 0]) ||
                        ((v->DistParking()[NumParkingArrivee - 1][i][Prioritaire ? 1 : 0] ==
                          v->DistParking()[NumParkingArrivee - 1][j][Prioritaire ? 1 : 0]) &&
                         ((Dirdp[i][v->DirPoss] != AncDir) || // En cas d'égalité, préfère garder la direction courante
                                                              // : v2.2.4.  // v3.5 : || par && !
                          ((i + Numero + cv->TourCrt) % ndp > (j + Numero + cv->TourCrt) % ndp)))); // sinon tirage au
                                                                                                    // hasard (Ici
                                                                                                    // TourCrt+1)
                }
            }
        }
    }

    for (i = 0; (Dir == 0) && (i < 5); i++)
    { // v3.5 : possibilité de blocage si test n'est pas large (i<5)
        for (j = 0; (Dir == 0) && (j < ndp); j++)
        {
            Voie = &cv->Voisine(x, y, Dirdp[j][v->DirPoss]);
            if ((OrdreDir[j] == i) &&
                ((!v->ToutDroit) || // Direction interdite si : - Obligation d'aller tout droit ET
                 Prioritaire ||     //                                   Véhicule NON prioritaire (v3.9.1) ET
                 (DirDrnDepl == Dirdp[j][v->DirPoss])) && //           pas même direction que lors du dernier
                                                          //           déplacement // v3.7.1 : Erreur !
                (Voie->NumPlaceTaxi == 0) &&              //                        - Place de taxi. v3.6
                (Voie->NumPlaceVehlib == 0) &&            //                      - Place de vehlib v5.3
                ((!Voie->PassageBus) || //                        - Passage bus dans cette direction sauf véhicules
                                        //                        prioritaires (police, urgence)
                 Prioritaire) &&
                (((Voie->NumVehicule == 0) && (Voie->NumVehlib == 0) &&     // v5.3
                  (Voie->NumBus == 0) &&                                    // v3.0
                  (Voie->NumTram == 0) &&                                   // v3.5
                  (Voie->NumTaxi == 0)) ||                                  // v3.6
                 (((Voie->NumParking != 0) || (Voie->NumPlacePark != 0)) && // v5.0 Voie->NumPlacePark
                  EstDestination(Voie)) ||                                  // v5.0 EstDestination
                 (cv->TourCrt <=
                  1 + TourDrnDepl + (static_cast<int>(NivPriorite == 0) * cv->AttenteMaxVeh * (1 + OrdreDir[j])))))
            { // v2.2
                Dir = Dirdp[j][v->DirPoss];
            }
        }
    }

    // Si la direction est toujours indéfinie : on prend la meilleure direction possible
    if (Dir == 0)
    {
        for (i = 0; (Dir == 0) && (i < 5); i++)
        { // v3.5 : possibilité de blocage si test n'est pas large (i<5)
            for (j = 0; (Dir == 0) && (j < ndp); j++)
            {
                Voie = &cv->Voisine(x, y, Dirdp[j][v->DirPoss]);
                if (((NumFileParkArrivee == 0) || // v5.0 : Empêcher le véhicule arrivé à la file de place de continuer
                     (Voie->NumPlacePark == 0) || // v5.0 (suite)
                     (cv->PlacePark[Voie->NumPlacePark - 1].NumFile + 1 == NumFileParkArrivee)) && // v5.0 (suite).
                                                                                                   // v5.2.3 : NumFile+1
                                                                                                   // et non NumFile
                    (OrdreDir[j] == i))
                {
                    Dir = Dirdp[j][v->DirPoss];
                }
            }
        }
    }
}
//-----------------------------------------------------------------------------
direction vehicule::ProchaineDirectionAuCarrefour() const // ATTENTION : DIR DOIT ETRE DEFINIE
{
    voie *v = &cv->Voisine(x, y, Dir);               // Ici v est la prochaine case
    const bool Prioritaire = (NivPriorite > normal); // v3.5 (pour DistParking)
    std::array<int, NBDIR> OrdreDir = {0, 0, 0, 0};
    int i = 0;
    int j = 0;
    const int ndp = NbDir[v->DirPoss];
    direction DirAuCarrefour = indefinie;

    // On classe les dir.poss. dans l'ordre < des distances aux parkings si la case est libre.
    // En cas d'égalité, c'est le hasard : (N°Veh.+Tour) Modulo Directions Possibles
    if (NumFileParkArrivee != 0)
    { // v5.0 : recopie du calcul d'OrdreDir puis remplacement des occurrences "Parking" par "FilePark"
        for (i = 0; i < ndp; i++)
        {
            for (j = 0; j < ndp; j++)
            {
                if (i != j)
                {
                    OrdreDir[i] += static_cast<int>(
                        (static_cast<int>(
                             (cv->Voisine(x, y, Dirdp[j][v->DirPoss]).NumPlacePark != 0) && // 1. File recherchée
                             (cv->PlacePark[cv->Voisine(x, y, Dirdp[j][v->DirPoss]).NumPlacePark - 1].NumFile + 1 ==
                              NumFileParkArrivee)) > // v5.2.3 : NumFile+1 et non NumFile
                         static_cast<int>(
                             (cv->Voisine(x, y, Dirdp[i][v->DirPoss]).NumPlacePark != 0) &&
                             (cv->PlacePark[cv->Voisine(x, y, Dirdp[i][v->DirPoss]).NumPlacePark - 1].NumFile + 1 ==
                              NumFileParkArrivee))) || // 2. Distance à la file. v5.2.3 : NumFile+1 et non NumFile
                        ((v->DistFilePark()[NumFileParkArrivee - 1][i][Prioritaire ? 1 : 0] >
                          v->DistFilePark()[NumFileParkArrivee - 1][j][Prioritaire ? 1 : 0]) ||
                         ((v->DistFilePark()[NumFileParkArrivee - 1][i][Prioritaire ? 1 : 0] ==
                           v->DistFilePark()[NumFileParkArrivee - 1][j][Prioritaire ? 1 : 0]) &&
                          ((Dirdp[i][v->DirPoss] != Dir) || // En cas d'égalité, préfère garder la direction courante :
                                                            // v2.2.4.  // v3.5 : || par && !
                           ((i + Numero + cv->TourCrt) % ndp > (j + Numero + cv->TourCrt) % ndp))))); // tirage au
                                                                                                      // hasard en cas
                                                                                                      // d'égalité (Ici
                                                                                                      // TourCrt+1)
                }
            }
        }
    }
    if (NumParkingArrivee != 0)
    { // v5.0 : if (NumParkingArrivee)
        for (i = 0; i < ndp; i++)
        {
            for (j = 0; j < ndp; j++)
            {
                if (i != j)
                {
                    OrdreDir[i] += static_cast<int>(
                        (v->DistParking()[NumParkingArrivee - 1][i][Prioritaire ? 1 : 0] >
                         v->DistParking()[NumParkingArrivee - 1][j][Prioritaire ? 1 : 0]) ||
                        ((v->DistParking()[NumParkingArrivee - 1][i][Prioritaire ? 1 : 0] ==
                          v->DistParking()[NumParkingArrivee - 1][j][Prioritaire ? 1 : 0]) &&
                         ((Dirdp[i][v->DirPoss] != Dir) || // En cas d'égalité, préfère garder la direction courante :
                                                           // v2.2.4.  // v3.5 : || par && !
                          ((i + Numero + cv->TourCrt) % ndp > (j + Numero + cv->TourCrt) % ndp)))); // tirage au hasard
                                                                                                    // en cas d'égalité
                                                                                                    // (Ici TourCrt+1)
                }
            }
        }
    }

    for (i = 0; (DirAuCarrefour == 0) && (i < 5); i++)
    { // v3.5 : possibilité de blocage si test n'est pas large (i<5)
        for (j = 0; (DirAuCarrefour == 0) && (j < ndp); j++)
        {
            if ((OrdreDir[j] == i) && (v->NumPlaceTaxi == 0) && // Direction interdite si :     - Place de Taxi. v3.6
                (v->NumPlaceVehlib == 0) &&                     //                            - Place de vehlib v5.3
                ((!v->ToutDroit) || //                              - Obligation d'aller tout droit ET
                 Prioritaire ||     //                                      Véhicule NON Prioritaire (v3.9.1) ET
                 (DirDrnDepl == Dirdp[j][v->DirPoss])))
            { // pas même direction que lors du dernier déplacement. v3.7.1 : Erreur ! v5.2.3 : Erreur sur Test même
              // direction
                DirAuCarrefour = Dirdp[j][v->DirPoss];
            }
        }
    }
    return DirAuCarrefour;
}
//-----------------------------------------------------------------------------
bool vehicule::PeutPasser() const
{
    voie *v = &cv->v[x][y];
    int NumFeuVeh = 0;
    return (Dir != 0) && ((NivPriorite != 0) ||
                          (((v->Priorite[Dir - 1] != stop) || (cv->TourCrt - TourDrnDepl >= 2)) &&
                           (((NumFeuVeh = v->NumFeu[Dir - 1]) == 0) || (cv->Feu[NumFeuVeh - 1].Etat < orange))));
}
//-----------------------------------------------------------------------------
void vehicule::Verifie(AnsiString &asMsgErr) const
{
    const AnsiString asSource = Format("Véhicule n°%d (%d,%d)", TableauDeConstantes(Numero, x, y));
    if (Numero < 0)
    {
        asMsgErr += asSource + Format(": numéro incorrect (%d)\n", TableauDeConstantes(Numero));
    }
    VerifiePosition(asMsgErr, asSource, x, y, PEUT_ETRE_INDEFINI);
    VerifieDirection(asMsgErr, asSource, Dir, PEUT_ETRE_INDEFINI);

    if (NumFileParkDepart != 0) // v5.0
    {
        if (NumFileParkDepart < 1)
        {
            asMsgErr += asSource + Format(": numéro de file de parking de départ incorrect (%d)\n",
                                          TableauDeConstantes(NumFileParkDepart));
        }
    }
    else if (NumParkingDepart < 1)
    {
        asMsgErr +=
            asSource + Format(": numéro de parking de départ incorrect (%d)\n", TableauDeConstantes(NumParkingDepart));
    }

    if (NumFileParkArrivee != 0) // v5.0
    {
        if (NumFileParkArrivee < 1)
        {
            asMsgErr += asSource + Format(": numéro de file de parking d'arrivée incorrect (%d)\n",
                                          TableauDeConstantes(NumFileParkArrivee));
        }
    }
    else if (NumParkingArrivee < 1)
    {
        asMsgErr +=
            asSource + Format(": numéro de parking d'arrivée incorrect (%d)\n", TableauDeConstantes(NumParkingArrivee));
    }
}
//-----------------------------------------------------------------------------
// CLASSES du réseau de VEHLIB (v5.3)
//-----------------------------------------------------------------------------
bool place_vehlib::Charge(int hfile)
{
    return LitChamp(hfile, &Numero, sizeof(Numero)) && LitChamp(hfile, &x, sizeof(x)) && LitChamp(hfile, &y, sizeof(y));
}
//-----------------------------------------------------------------------------
bool place_vehlib::Sauve(int hfile)
{
    int xc = CoordonneeCopiee(x, cv->Xc);
    int yc = CoordonneeCopiee(y, cv->Yc); // v4.0.1. v6.0.1 : COORDCOPIE
    return (FileWrite(hfile, &Numero, sizeof(Numero)) != 0) &&
           (FileWrite(hfile, cv->CopieEnCours ? &xc : &x, sizeof(x)) != 0) && // v4.0.1
           (FileWrite(hfile, cv->CopieEnCours ? &yc : &y, sizeof(y)) != 0);   // v4.0.1
}
//-----------------------------------------------------------------------------
void place_vehlib::Verifie(AnsiString &asMsgErr) const
{
    const AnsiString asSource = Format("Place Vehlib n°%d (%d,%d)", TableauDeConstantes(Numero, x, y));
    if (Numero < 0)
    {
        asMsgErr += asSource + Format(": numéro incorrect (%d)\n", TableauDeConstantes(Numero));
    }
    VerifiePosition(asMsgErr, asSource, x, y, PEUT_ETRE_INDEFINI);
}
//-----------------------------------------------------------------------------
bool place_vehlib::EstLibre() const
{
    return (NumVehlibReserve == 0) && (cv->v[x][y].NumVehlib == 0);
}
//-----------------------------------------------------------------------------
void vehlib::Place(int p) // ATTENTION: 0 <= p < NbPlacesVehlib
{
    x = cv->PlaceVehlib[p].x;
    y = cv->PlaceVehlib[p].y;
    cv->v[x][y].NumVehlib = Numero;
    voie *v = &cv->v[x][y];
    Situation = svAttente;
    for (int d = nord; (Dir == 0) && (d <= ouest); d++)
    {
        if ((v->DirPoss & PossDir[static_cast<direction>(d)]) != 0)
        {
            Dir = static_cast<direction>(d);
        }
    }
}
//-----------------------------------------------------------------------------
bool vehlib::EstDestination(const voie *v) const
{
    if (NumPlaceArrivee != 0)
    {
        return (NumPlaceArrivee == v->NumPlaceVehlib);
    }
    return false;
}
//-----------------------------------------------------------------------------
void vehlib::CalculeProchaineDirection()
{
    voie *v = &cv->v[x][y];
    voie *Voie = nullptr;
    const direction AncDir = Dir; // v2.2.4
    std::array<int, NBDIR> OrdreDir = {0, 0, 0, 0};
    int i = 0;
    int j = 0;
    const int ndp = NbDir[v->DirPoss];

    if ((Situation == svAttente) &&        // Si le vehlib attend,
        (NbPietonsQuiSontMontes() != 0) && // On regarde déjà s'il y a des piétons qui veulent monter...
        ((NumPlaceArrivee != 0) ||         // Des fois les piétons restent dans le vehlib...
         ReservePlaceVehlibCible()))       // v6.0.1 : sans place libre, le vehlib attend (nouvel essai au tour suivant)
    {                                      // pour pouvoir partir...
        Situation = svDepart;
        TourDepart = cv->TourCrt;
    }
    Dir = indefinie;
    // On classe les dir.poss. dans l'ordre < des distances aux files.
    // En cas d'égalité, c'est le hasard : (N°Veh.+Tour) Modulo Directions Possibles
    if ((NumPlaceArrivee != 0) && (Situation == svDepart))
    {
        for (i = 0; i < ndp; i++)
        {
            for (j = 0; j < ndp; j++)
            {
                if (i != j) // On dégringole si distance supérieure.
                {
                    OrdreDir[i] += static_cast<int>(
                        (static_cast<int>(cv->Voisine(x, y, Dirdp[j][v->DirPoss]).NumPlaceVehlib ==
                                          NumPlaceArrivee) > // 1. Place arrivée
                         static_cast<int>(cv->Voisine(x, y, Dirdp[i][v->DirPoss]).NumPlaceVehlib == NumPlaceArrivee)) ||
                        ((v->DistPlaceVehlib()[NumPlaceArrivee - 1][i] >
                          v->DistPlaceVehlib()[NumPlaceArrivee - 1][j]) || // 2. Distance à la place.
                         ((v->DistPlaceVehlib()[NumPlaceArrivee - 1][i] ==
                           v->DistPlaceVehlib()[NumPlaceArrivee - 1][j]) &&
                          ((Dirdp[i][v->DirPoss] != AncDir) || // En cas d'égalité, préfère garder la direction courante
                           ((i + Numero + cv->TourCrt) % ndp > (j + Numero + cv->TourCrt) % ndp))))); // sinon tirage au
                                                                                                      // hasard (Ici
                                                                                                      // TourCrt+1)
                }
            }
        }
    }
    for (i = 0; (Dir == 0) && (i < 5); i++)
    {
        for (j = 0; (Dir == 0) && (j < ndp); j++)
        {
            Voie = &cv->Voisine(x, y, Dirdp[j][v->DirPoss]);
            if ((OrdreDir[j] == i) &&
                ((!v->ToutDroit) ||                       // Direction interdite si : - Obligation d'aller tout droit ET
                 (DirDrnDepl == Dirdp[j][v->DirPoss])) && //           pas même direction que lors du dernier
                                                          //           déplacement
                (Voie->NumPlaceTaxi == 0) &&              //                        - Place de taxi.
                ((Voie->NumPlaceVehlib == 0) ||           //                     - Place de vehlib (v5.3)
                 (Voie->NumPlaceVehlib == NumPlaceArrivee)) && //      SAUF place d'arrivée (v5.3)
                (!Voie->PassageBus) &&                         //                        - Passage bus
                (((Voie->NumVehicule == 0) &&                  //                       - Véhicule de tout type
                  (Voie->NumVehlib == 0) && (Voie->NumBus == 0) && (Voie->NumTram == 0) && (Voie->NumTaxi == 0)) ||
                 (((Voie->NumParking != 0) || (Voie->NumPlacePark != 0) || (Voie->NumPlaceVehlib != 0)) &&
                  EstDestination(Voie)) ||
                 (cv->TourCrt <= 1 + TourDrnDepl + (cv->AttenteMaxVeh * (1 + OrdreDir[j])))))
            {
                Dir = Dirdp[j][v->DirPoss];
            }
        }
    }

    // Si la direction est toujours indéfinie : on prend la meilleure direction possible
    if (Dir == 0)
    {
        for (i = 0; (Dir == 0) && (i < 5); i++)
        { // possibilité de blocage si test n'est pas large (i<5)
            for (j = 0; (Dir == 0) && (j < ndp); j++)
            {
                Voie = &cv->Voisine(x, y, Dirdp[j][v->DirPoss]);
                if (((NumPlaceArrivee == 0) || // Empêcher le véhicule arrivé à la place de continuer
                     (Voie->NumPlaceVehlib == 0) || (Voie->NumPlaceVehlib == NumPlaceArrivee)) &&
                    (OrdreDir[j] == i))
                {
                    Dir = Dirdp[j][v->DirPoss];
                }
            }
        }
    }
}
//-----------------------------------------------------------------------------
direction vehlib::ProchaineDirectionAuCarrefour() const // ATTENTION : DIR DOIT ETRE DEFINIE
{
    voie *v = &cv->Voisine(x, y, Dir); // Ici v est la prochaine case
    std::array<int, NBDIR> OrdreDir = {0, 0, 0, 0};
    int i = 0;
    int j = 0;
    const int ndp = NbDir[v->DirPoss];
    direction DirAuCarrefour = indefinie;

    // On classe les dir.poss. dans l'ordre < des distances aux parkings si la case est libre.
    // En cas d'égalité, c'est le hasard : (N°Veh.+Tour) Modulo Directions Possibles
    if (NumPlaceArrivee != 0)
    { // v5.0 : recopie du calcul d'OrdreDir puis remplacement des occurrences "Parking" par "FilePark"
        for (i = 0; i < ndp; i++)
        {
            for (j = 0; j < ndp; j++)
            {
                if (i != j)
                {
                    OrdreDir[i] += static_cast<int>(
                        (static_cast<int>(cv->Voisine(x, y, Dirdp[j][v->DirPoss]).NumPlaceVehlib ==
                                          NumPlaceArrivee) > // 1. Place arrivée
                         static_cast<int>(cv->Voisine(x, y, Dirdp[i][v->DirPoss]).NumPlaceVehlib == NumPlaceArrivee)) ||
                        ((v->DistPlaceVehlib()[NumPlaceArrivee - 1][i] >
                          v->DistPlaceVehlib()[NumPlaceArrivee - 1][j]) || // 2. Distance à la place.
                         ((v->DistPlaceVehlib()[NumPlaceArrivee - 1][i] ==
                           v->DistPlaceVehlib()[NumPlaceArrivee - 1][j]) &&
                          ((Dirdp[i][v->DirPoss] != Dir) || // En cas d'égalité, préfère garder la direction courante
                           ((i + Numero + cv->TourCrt) % ndp > (j + Numero + cv->TourCrt) % ndp))))); // sinon tirage au
                                                                                                      // hasard (Ici
                                                                                                      // TourCrt+1)
                }
            }
        }
    }
    for (i = 0; (DirAuCarrefour == 0) && (i < 5); i++)
    { // possibilité de blocage si test n'est pas large (i<5)
        for (j = 0; (DirAuCarrefour == 0) && (j < ndp); j++)
        {
            if ((OrdreDir[j] == i) && (v->NumPlaceTaxi == 0) && // Direction interdite si :     - Place de Taxi. v3.6
                ((!v->ToutDroit) || //                              - Obligation d'aller tout droit ET
                 (DirDrnDepl == Dirdp[j][v->DirPoss])))
            { // pas même direction que lors du dernier déplacement
                DirAuCarrefour = Dirdp[j][v->DirPoss];
            }
        }
    }
    return DirAuCarrefour;
}
//-----------------------------------------------------------------------------
bool vehlib::PeutPasser() const
{
    voie *v = &cv->v[x][y];
    int NumFeuVeh = 0;
    return (Dir != 0) && (((v->Priorite[Dir - 1] != stop) || (cv->TourCrt - TourDrnDepl >= 2)) &&
                          (((NumFeuVeh = v->NumFeu[Dir - 1]) == 0) || (cv->Feu[NumFeuVeh - 1].Etat < orange)));
}
//-----------------------------------------------------------------------------
void vehlib::FaitDescendrePietons()
{
    int i = 0;
    int d = 0;
    // v6.0.1 : l'original testait la case du vehlib (cv->v[x][y]), qu'il occupe : aucun passager ne descendait
    for (d = nord; (d <= ouest) && (!cv->Voisine(x, y, d).PietonPeutAller(static_cast<direction>(d))); d++)
    {
        ; // direction où sortent les piétons et pas forcément à droite comme les taxis ! v6.0.1 : borne testée d'abord
    }
    if ((d <= ouest) && cv->Voisine(x, y, d).PietonPeutAller(static_cast<direction>(d))) // On s'assure que l'on trouve
                                                                                         // un trottoir quand même !
                                                                                         // v6.0.1 : l'original appelait
                                                                                         // PietonPeutAller(5)
    {
        voie *v = &cv->Voisine(x, y, d);
        pieton *p = nullptr;
        for (i = 0; i < cv->NbPietons; i++)
        {
            p = &cv->Pieton[i];
            if (p->NumVehlib == Numero)
            {
                v->FaitDescendrePieton(static_cast<direction>(d), i);
            }
        }
    }
}
//-----------------------------------------------------------------------------
bool vehlib::ReservePlaceVehlibCible() // v6.0.1 : renvoie false (sans tirage) s'il n'y a aucune place libre
{
    int NumPlace = 0; // 0 < NumPlace <= NbPlacesVehlib
    // v6.0.1 : l'original tirait sans fin quand aucune place n'était libre (simulation figée)
    for (NumPlace = 0; (NumPlace < cv->NbPlacesVehlib) &&
                       ((NumPlace + 1 == NumPlaceDepart) || (!cv->PlaceVehlib[NumPlace].EstLibre()));
         NumPlace++)
    {
        ;
    }
    if (NumPlace == cv->NbPlacesVehlib)
    {
        return false;
    }
    do // On choisit une place libre
    {
        NumPlace = alea::Rand() % cv->NbPlacesVehlib;
    } while ((NumPlace + 1 == NumPlaceDepart) || (!cv->PlaceVehlib[NumPlace].EstLibre()));
    place_vehlib *pv = &cv->PlaceVehlib[NumPlace];
    const int xp = pv->x;
    const int yp = pv->y;
    pv->NumVehlibReserve = Numero;
    NumPlaceArrivee = NumPlace + 1;
    vue->RedessineCase(xp, yp, false); // portage
    return true;
}
//-----------------------------------------------------------------------------
void vehlib::FaitMonterPieton(int NumPietonAFaireMonter)
{
    pieton *p = &cv->Pieton[NumPietonAFaireMonter];
    const int xp = p->x;
    const int yp = p->y;
    if ((NbPersonnes < cv->NbMaxPersonnesVehlib) &&
        ((NumPlaceArrivee != 0) || ReservePlaceVehlibCible())) // v6.0.1 : le piéton ne monte que si une place d'arrivée
                                                               // est réservée (sinon il attend)
    {
        NbPersonnes++;
        cv->v[xp][yp].RetirePieton(NumPietonAFaireMonter);
        p->MonteDansVehlib(Numero);
        vue->RedessineCase(xp, yp, false); // portage
    }
}
//-----------------------------------------------------------------------------
int vehlib::NbPietonsQuiSontMontes() // retourne le nombre de piétons qui attendaient et qui sont montés
{
    voie *v = nullptr;
    pieton *p = nullptr;
    if (cv->NbVehlibs < cv->NbPlacesVehlib)
    { // S'il y a des places libres dans la simulation
        for (int d = nord; (NbPersonnes <= cv->NbMaxPersonnesVehlib) && (d <= ouest); d++)
        {
            v = &cv->Voisine(x, y, d);
            for (int i = 0; (NbPersonnes <= cv->NbMaxPersonnesVehlib) && (i < NBMAXPIETONSPARCASE); i++)
            {
                if (v->NumPieton[i] != 0)
                {
                    p = &cv->Pieton[v->NumPieton[i] - 1];
                    if (p->AttendVehlib && (p->NumPlaceVehlib == NumPlaceDepart))
                    {
                        FaitMonterPieton(v->NumPieton[i] - 1);
                    }
                }
            }
        }
    }
    return NbPersonnes;
}
//-----------------------------------------------------------------------------
void vehlib::Verifie(AnsiString &asMsgErr) const
{
    const AnsiString asSource = Format("Vehlib n°%d (%d,%d)", TableauDeConstantes(Numero, x, y));
    if (Numero < 0)
    {
        asMsgErr += asSource + Format(": numéro incorrect (%d)\n", TableauDeConstantes(Numero));
    }
    VerifiePosition(asMsgErr, asSource, x, y, PEUT_ETRE_INDEFINI);
    VerifieDirection(asMsgErr, asSource, Dir, PEUT_ETRE_INDEFINI);

    if ((NumPlaceDepart != 0) && (NumPlaceDepart < 1))

    {
        asMsgErr +=
            asSource + Format(": numéro de place de départ incorrect (%d)\n", TableauDeConstantes(NumPlaceDepart));
    }

    if ((NumPlaceArrivee != 0) && (NumPlaceArrivee < 1))

    {
        asMsgErr +=
            asSource + Format(": numéro de place d'arrivée incorrect (%d)\n", TableauDeConstantes(NumPlaceArrivee));
    }
}
//-----------------------------------------------------------------------------
// FIN CLASSE VEHLIB (v5.3)
//-----------------------------------------------------------------------------
void depot_bus::PlaceProchainBus() // v3.0
{
    int NumProchBus = 0;
    if (Defini() && (cv->v[x][y].NumBus == 0) && // Pas de bus
        (cv->v[x][y].NumTram == 0) &&            // Pas de tram
        (cv->v[x][y].NumVehicule == 0) &&        // Pas de véhicule
        (cv->v[x][y].NumTaxi == 0) &&            // Pas de taxi (v3.6)... sur la case dépôt
        (cv->v[x][y].NumVehlib == 0) &&          // Pas de vehlib (v5.3)
        (NumDrnBusSorti < cv->NbBus))            // ET Encore au moins un bus à sortir
    {
        NumProchBus = ++NumDrnBusSorti;
        cv->PlaceBus(x, y, NumProchBus);
        cv->Bus[NumProchBus - 1].TourDepart = cv->TourCrt; // v3.5 : un oubli !
    }
}
//-----------------------------------------------------------------------------
bool depot_bus::Charge(int hfile) // v3.0
{
    return LitChamp(hfile, &x, sizeof(x)) && LitChamp(hfile, &y, sizeof(y));
}
//-----------------------------------------------------------------------------
bool depot_bus::Sauve(int hfile) // v3.0
{
    int xc = CoordonneeCopiee(x, cv->Xc);
    int yc = CoordonneeCopiee(y, cv->Yc);                                     // v4.0.1. v6.0.1 : COORDCOPIE
    return (FileWrite(hfile, cv->CopieEnCours ? &xc : &x, sizeof(x)) != 0) && // v4.0.1
           (FileWrite(hfile, cv->CopieEnCours ? &yc : &y, sizeof(y)) != 0);   // v4.0.1
}
//-----------------------------------------------------------------------------
void depot_bus::Verifie(AnsiString &asMsgErr) const // v3.03
{
    const AnsiString asSource = Format("Dépot bus (%d,%d)", TableauDeConstantes(x, y)); // v6.0.1 : "Dépot bus : " (sans
                                                                                        // spécificateur) dans
                                                                                        // l'original
    VerifiePosition(asMsgErr, asSource, x, y, PEUT_ETRE_INDEFINI);
}
//-----------------------------------------------------------------------------
void bus::FaitMonterPieton(int NumPietonAFaireMonter) // v3.0
{
    pieton *p = &cv->Pieton[NumPietonAFaireMonter];
    const int xp = p->x;
    const int yp = p->y;
    if (NbPersonnes < cv->NbMaxPersonnesBus)
    {
        NbPersonnes++;
        cv->v[xp][yp].RetirePieton(NumPietonAFaireMonter);
        p->MonteDansBus(Numero);           // v3.6
        vue->RedessineCase(xp, yp, false); // portage
    }
}
//-----------------------------------------------------------------------------
void bus::FaitMonterEtDescendre() // v3.0
{
    int i = 0;
    const direction d = ADroiteDe[Dir];
    voie *v = &cv->Voisine(x, y, d);
    pieton *p = nullptr;
    for (i = 0; i < cv->NbPietons; i++)
    {
        p = &cv->Pieton[i];
        if ((p->NumBus == Numero) && p->DecideDeDescendre(cv->ProbaArretBusDmd))
        {
            v->FaitDescendrePieton(d, i); // Si on peut pas faire descendre, il descendra au prochain arrêt
        }
        if ((p->NumArretBus == NumDrnArret) && p->AttendBus)
        {
            FaitMonterPieton(i); // Si on ne peut pas faire monter, il montera dans le prochain bus
        }
    }
}
//-----------------------------------------------------------------------------
void bus::CalculeProchaineDirection() // v3.0
{
    voie *v = &cv->v[x][y];
    voie *Voie = nullptr;
    const direction AncDir = Dir;
    std::array<int, NBDIR> OrdreDir = {0, 0, 0, 0};
    int i = 0;
    int j = 0;
    const int ndp = NbDir[v->DirPoss];
    Dir = indefinie;
    // On classe les dir.poss. dans l'ordre < des distances aux arrêts de bus
    // En cas d'égalité, c'est le hasard : (N°Bus.+Tour) Modulo Directions Possibles
    for (i = 0; i < ndp; i++)
    {
        for (j = 0; j < ndp; j++)
        {
            if (i != j)
            { // On dégringole si distance supérieure.
                OrdreDir[i] += static_cast<int>(
                    (v->DistArretBus()[NumArretSvt - 1][i] > v->DistArretBus()[NumArretSvt - 1][j]) ||
                    ((v->DistArretBus()[NumArretSvt - 1][i] == v->DistArretBus()[NumArretSvt - 1][j]) &&
                     ((Dirdp[i][v->DirPoss] != AncDir) || // En cas d'égalité, préfère garder la direction courante :
                                                          // v2.2.4
                      ((i + Numero + cv->TourCrt) % ndp > (j + Numero + cv->TourCrt) % ndp)))); // sinon tirage au
                                                                                                // hasard (Ici
                                                                                                // TourCrt+1)
            }
        }
    }
    for (i = 0; (Dir == 0) && (i < 5); i++)
    { // v3.5 : possibilité de blocage si test n'est pas large (i<5)
        for (j = 0; (Dir == 0) && (j < ndp); j++)
        {
            Voie = &cv->Voisine(x, y, Dirdp[j][v->DirPoss]);
            if ((OrdreDir[j] == i) && (v->NumPlaceTaxi == 0) && // Direction interdite si       - Place de taxi. v3.6
                (v->NumPlacePark == 0) &&                       //                              - Place de parking. v5.0
                (v->NumPlaceVehlib == 0) &&                     //                              - Place de vehlib. v5.3
                ((!v->ToutDroit) || //                              - Obligation d'aller tout droit ET
                 (DirDrnDepl == Dirdp[j][v->DirPoss])) && // pas même direction que lors du dernier déplacement. v3.7.1
                                                          // : Erreur ! v5.2.3 : Erreur sur Test même direction
                (((Voie->NumVehicule == 0) && (Voie->NumBus == 0) && (Voie->NumTram == 0) && (Voie->NumTaxi == 0) &&
                  (Voie->NumVehlib == 0)) || // v5.3 (vehlib)
                 ((Voie->NumArretBus != 0) && (Voie->NumArretBus == NumArretSvt)) ||
                 (cv->TourCrt <= 1 + TourDrnDepl + (5 * cv->AttenteMaxVeh * (1 + OrdreDir[j])))))
            { // AttentMaxBus = 5*AttenteMaxVeh
                Dir = Dirdp[j][v->DirPoss];
            }
        }
    }

    // Si la direction est toujours indéfinie : on prend la meilleure direction possible
    if (Dir == 0)
    {
        for (i = 0; (Dir == 0) && (i < 5); i++)
        { // v3.5 : possibilité de blocage si test n'est pas large (i<5)
            for (j = 0; (Dir == 0) && (j < ndp); j++)
            {
                if (OrdreDir[j] == i)
                {
                    Dir = Dirdp[j][v->DirPoss];
                }
            }
        }
    }
    if (DirDrnDepl == 0)
    {
        DirDrnDepl = Dir;
    }
    if (DirAvtDrnDepl == 0)
    {
        DirAvtDrnDepl = Dir;
    }
}
//-----------------------------------------------------------------------------
direction bus::ProchaineDirectionAuCarrefour() const // ATTENTION : DIR DOIT ETRE DEFINIE. v3.0
{
    voie *v = &cv->Voisine(x, y, Dir); // Ici v est la prochaine case
    std::array<int, NBDIR> OrdreDir = {0, 0, 0, 0};
    int i = 0;
    int j = 0;
    const int ndp = NbDir[v->DirPoss];
    direction DirAuCarrefour = indefinie;
    // On classe les dir.poss. dans l'ordre < des distances aux parkings si la case est libre.
    // En cas d'égalité, c'est le hasard : (N°Bus.+Tour) Modulo Directions Possibles
    for (i = 0; i < ndp; i++)
    {
        for (j = 0; j < ndp; j++)
        {
            if (i != j)
            {
                OrdreDir[i] += static_cast<int>(
                    (v->DistArretBus()[NumArretSvt - 1][i] > v->DistArretBus()[NumArretSvt - 1][j]) ||
                    ((v->DistArretBus()[NumArretSvt - 1][i] == v->DistArretBus()[NumArretSvt - 1][j]) &&
                     ((Dirdp[i][v->DirPoss] != Dir) || // En cas d'égalité, préfère garder la direction courante : //
                                                       // v3.5 : || par && !
                      ((i + Numero + cv->TourCrt) % ndp > (j + Numero + cv->TourCrt) % ndp)))); // tirage au hasard en
                                                                                                // cas d'égalité (Ici
                                                                                                // TourCrt+1)
            }
        }
    }
    for (i = 0; (DirAuCarrefour == 0) && (i < 5); i++)
    { // v3.5 : possibilité de blocage si test n'est pas large (i<5)
        for (j = 0; (DirAuCarrefour == 0) && (j < ndp); j++)
        {
            if ((OrdreDir[j] == i) && (v->NumPlaceTaxi == 0) && // Direction interdite si :    - Place de taxi
                (v->NumPlacePark == 0) &&                       //                             - Place de parking. v5.0
                (v->NumPlaceVehlib == 0) &&                     //                             - Place de vehlib. v5.3
                ((!v->ToutDroit) || //                             - Obligation d'aller tout droit ET
                 (DirDrnDepl == Dirdp[j][v->DirPoss])))
            { // pas même direction que lors du dernier déplacement. v3.7.1 : Erreur !
                DirAuCarrefour = Dirdp[j][v->DirPoss];
            }
        }
    }
    return DirAuCarrefour;
}
//-----------------------------------------------------------------------------
bool bus::PeutPasser() const // v3.0
{
    voie *v = &cv->v[x][y];
    int NumFeuBus = 0;
    return (Dir != 0) && ((v->Priorite[Dir - 1] != stop) || (cv->TourCrt - TourDrnDepl >= 2)) &&
           (((NumFeuBus = v->NumFeu[Dir - 1]) == 0) || (cv->Feu[NumFeuBus - 1].Etat < orange));
}
//-----------------------------------------------------------------------------
int bus::NumeroArretSuivant() const // v3.0
{
    int i = 0;
    for (i = 0; (i < cv->LigneBus[NumLigne - 1].NbArrets) && // v6.0.1 : borne testée d'abord (NumArret[NbArrets] lu par
                                                             // l'original)
                (NumArretSvt != cv->LigneBus[NumLigne - 1].NumArret[i]);
         i++)
    {
        ;
    }
    if (i < cv->LigneBus[NumLigne - 1].NbArrets)
    {
        return cv->LigneBus[NumLigne - 1].NumArret[(i + 1) % cv->LigneBus[NumLigne - 1].NbArrets];
    }
    return cv->LigneBus[NumLigne - 1].NumArret[0];
}
//-----------------------------------------------------------------------------
void bus::Verifie(AnsiString &asMsgErr) const // v3.03
{
    const AnsiString asSource = Format("Bus n°%d (%d,%d)", TableauDeConstantes(Numero, x, y));
    if (Numero < 0)
    {
        asMsgErr += asSource + Format(": numéro incorrect (%d)\n", TableauDeConstantes(Numero));
    }
    VerifiePosition(asMsgErr, asSource, x, y, PEUT_ETRE_INDEFINI);
    VerifieDirection(asMsgErr, asSource, Dir, PEUT_ETRE_INDEFINI);
    if ((NumLigne < 0) || (NumLigne >= NBMAXLIGNESBUSTRAM))
    {
        asMsgErr += asSource + Format(": numéro de ligne de bus incorrect (%d)\n", TableauDeConstantes(NumLigne));
    }
}
//-----------------------------------------------------------------------------
void arret_bus::Copie(arret_bus *a) // v5.1
{
    Numero = a->Numero;
    x = a->x;
    y = a->y;
    for (int i = 0; i < NBMAXLIGNESBUSTRAM; i++)
    {
        Ligne[i] = a->Ligne[i];
        NumLigne[i] = a->NumLigne[i];
    }
    NbLignes = a->NbLignes;
}
//-----------------------------------------------------------------------------
void arret_bus::Initialise() // v3.0
{
    int i = 0;
    x = -1;
    y = -1;
    NbLignes = 0;
    for (i = 0; i < NBMAXLIGNESBUSTRAM; i++)
    {
        Ligne[i] = false;
        NumLigne[i] = 0;
    }
}
//-----------------------------------------------------------------------------
void arret_bus::InitialiseInfosLignesBus() // v3.0
{
    int i = 0;
    NbLignes = 0;
    for (i = 0; i < NBMAXLIGNESBUSTRAM; i++)
    {
        Ligne[i] = false;
        NumLigne[i] = 0;
    }
}
//-----------------------------------------------------------------------------
void arret_bus::AjouteLigneBus(int n) // v3.0
{
    if (!Ligne[n - 1])
    { // v6.0.9 : ajoutée à chaque passage de la ligne par l'arrêt (« 4,4 ») : écriture hors de NumLigne dès le dixième
        NumLigne[NbLignes++] = n; // v3.0.5
    }
    Ligne[n - 1] = true;
}
//-----------------------------------------------------------------------------
AnsiString arret_bus::asListeLignesBus() // v3.0.5
{
    int i = 0;
    AnsiString asListe = "";
    for (i = 0; i < NbLignes; i++)
    {
        if (asListe.IsEmpty())
        {
            asListe = IntToStr(NumLigne[i]);
        }
        else
        {
            asListe = asListe + "," + IntToStr(NumLigne[i]);
        }
    }
    return asListe;
}
//-----------------------------------------------------------------------------
bool arret_bus::Charge(int hfile) // v3.0
{
    return LitChamp(hfile, &Numero, sizeof(Numero)) && LitChamp(hfile, &x, sizeof(x)) &&
           LitChamp(hfile, &y, sizeof(y)) && LitChamp(hfile, Ligne.data(), NBMAXLIGNESBUSTRAM * sizeof(Ligne[0])) &&
           LitChamp(hfile, &NbLignes, sizeof(NbLignes)) &&
           LitChamp(hfile, NumLigne.data(), NBMAXLIGNESBUSTRAM * sizeof(NumLigne[0]));
}
//-----------------------------------------------------------------------------
bool arret_bus::Sauve(int hfile) // v3.0
{
    int xc = CoordonneeCopiee(x, cv->Xc);
    int yc = CoordonneeCopiee(y, cv->Yc); // v4.0.1. v6.0.1 : COORDCOPIE
    return (FileWrite(hfile, &Numero, sizeof(Numero)) != 0) &&
           (FileWrite(hfile, cv->CopieEnCours ? &xc : &x, sizeof(x)) != 0) &&
           (FileWrite(hfile, cv->CopieEnCours ? &yc : &y, sizeof(y)) != 0) &&
           (FileWrite(hfile, Ligne.data(), NBMAXLIGNESBUSTRAM * sizeof(Ligne[0])) != 0) &&
           (FileWrite(hfile, &NbLignes, sizeof(NbLignes)) != 0) &&
           (FileWrite(hfile, NumLigne.data(), NBMAXLIGNESBUSTRAM * sizeof(NumLigne[0])) != 0);
}
//-----------------------------------------------------------------------------
void arret_bus::Verifie(AnsiString &asMsgErr) // v3.0
{
    int i = 0;
    const AnsiString asSource = Format("Arrêt de bus n°%d (%d,%d)", TableauDeConstantes(Numero, x, y));
    if (Numero < 0)
    {
        asMsgErr += asSource + Format(": numéro incorrect (%d)\n", TableauDeConstantes(Numero));
    }
    VerifiePosition(asMsgErr, asSource, x, y, PEUT_ETRE_INDEFINI);
    if ((NbLignes < 0) || (NbLignes > NBMAXLIGNESBUSTRAM))
    {
        asMsgErr += asSource + Format(": nombre de lignes de bus incorrect (%d)\n", TableauDeConstantes(NbLignes));
    }
    for (i = 0; i < NbLignes; i++)
    {
        if (Ligne[i] && ((NumLigne[i] < 0) || (NumLigne[i] > NBMAXLIGNESBUSTRAM)))
        {
            asMsgErr += asSource + Format(": numéro de la %d° ligne de bus incorrect (%d)\n",
                                          TableauDeConstantes(i + 1, NumLigne[i])); // v3.03
        }
    }
}
//-----------------------------------------------------------------------------
void ligne_bus::Initialise() // v3.0
{
    int i = 0;
    NbBus = NbArrets = 0;
    for (i = 0; i < NBMAXARRETSBUSTRAMPARLIGNE; i++)
    {
        NumArret[i] = 0;
    }
    for (i = 0; i < NBMAXBUSTRAMPARLIGNE; i++)
    {
        NumBus[i] = 0;
    }
}
//-----------------------------------------------------------------------------
void ligne_bus::AjouteBus(int NumeroBus) // v3.0
{
    NumBus[NbBus++] = NumeroBus;
}
//-----------------------------------------------------------------------------
bool ligne_bus::Charge(int hfile) // v3.0
{                                 // Attention : On ne stocke pas le nombre et les n° de bus (calculés avant simul)
    return LitChamp(hfile, &Numero, sizeof(Numero)) && LitChamp(hfile, &NbArrets, sizeof(NbArrets)) &&
           LitChamp(hfile, NumArret.data(), NBMAXARRETSBUSTRAMPARLIGNE * sizeof(NumArret[0]));
}
//-----------------------------------------------------------------------------
bool ligne_bus::Sauve(int hfile) // v3.0
{                                // Attention : On ne stocke pas le nombre et les n° de bus (calculés avant simul)
    return (FileWrite(hfile, &Numero, sizeof(Numero)) != 0) && (FileWrite(hfile, &NbArrets, sizeof(NbArrets)) != 0) &&
           (FileWrite(hfile, NumArret.data(), NBMAXARRETSBUSTRAMPARLIGNE * sizeof(NumArret[0])) != 0);
}
//-----------------------------------------------------------------------------
void ligne_bus::Verifie(AnsiString &asMsgErr) // v3.0
{
    int i = 0;
    const AnsiString asSource = Format("Ligne de bus n°%d", TableauDeConstantes(Numero));
    if (Numero < 0)
    {
        asMsgErr += asSource + Format(": numéro incorrect (%d)\n", TableauDeConstantes(Numero));
    }
    if ((NbArrets < 0) || (NbArrets > NBMAXARRETSBUSTRAMPARLIGNE))
    {
        asMsgErr += asSource + Format(": nombre d'arrêts de bus incorrect (%d)\n", TableauDeConstantes(NbArrets));
    }
    for (i = 0; i < NbArrets; i++)
    {
        if ((NumArret[i] < 0) || (NumArret[i] > cv->NbArretsBus))
        {
            asMsgErr += asSource + Format(": numéro du %d° arrêt de bus incorrect (%d)\n",
                                          TableauDeConstantes(i + 1, NumArret[i])); // v3.03
        }
    }
}
//-----------------------------------------------------------------------------
//////////// ICI COMMENCENT LES METHODES DES CLASSES LIEES AU RESEAU DE TRAM //
//-----------------------------------------------------------------------------
void tram::FaitMonterPieton(int NumPietonAFaireMonter) // v3.5
{
    pieton *p = &cv->Pieton[NumPietonAFaireMonter];
    const int xp = p->x;
    const int yp = p->y;
    if (NbPersonnes < cv->NbMaxPersonnesTram)
    {
        NbPersonnes++;
        cv->v[xp][yp].RetirePieton(NumPietonAFaireMonter);
        p->MonteDansTram(Numero);          // v3.6
        vue->RedessineCase(xp, yp, false); // portage
    }
}
//-----------------------------------------------------------------------------
void tram::FaitMonterEtDescendre() // v3.5
{
    int i = 0;
    const direction d = ADroiteDe[Dir];
    voie *v = &cv->Voisine(x, y, d);
    pieton *p = nullptr;
    for (i = 0; i < cv->NbPietons; i++)
    {
        p = &cv->Pieton[i];
        if ((p->NumTram == Numero) && p->DecideDeDescendre(cv->ProbaArretTramDmd))
        {
            v->FaitDescendrePieton(d, i); // Si on peut pas faire descendre, il descendra au prochain arrêt
        }
        if ((p->NumArretTram == NumDrnArret) && p->AttendTram)
        {
            FaitMonterPieton(i); // Si on ne peut pas faire monter, il montera dans le prochain tram
        }
    }
}
//-----------------------------------------------------------------------------
void tram::CalculeProchaineDirection() // v3.5
{
    voie *v = &cv->v[x][y];
    voie *Voie = nullptr;
    const direction AncDir = DirDrnDepl;
    std::array<int, NBDIR> OrdreDir = {0, 0, 0, 0};
    int d = 0;
    int i = 0;
    int j = 0;
    const int nd = v->NbDirPossSensVoieTram(indefinie);
    const int ndp = v->NbDirPossSensVoieTram(InvDir[AncDir]); // Nombre total de directions (pour DistArretTram) //
                                                              // Nombre de directions possibles
    Dir = indefinie;
    // On classe les dir.poss. dans l'ordre < des distances aux arrêts de tram
    // En cas d'égalité, c'est le hasard : (N°Tram.+Tour) Modulo Directions Possibles
    for (i = 0; i < nd; i++)
    {
        for (j = 0; j < nd; j++)
        {
            if (i != j)
            {
                d = v->DirSensVoieTram(indefinie, i);
                if ( // Direction possible
                    v->EstDirPossSensVoieTram(InvDir[AncDir], static_cast<direction>(d)))
                {
                    // On dégringole si distance supérieure.
                    OrdreDir[d - 1] += static_cast<int>(
                        (v->DistArretTram()[NumArretSvt - 1][i] > v->DistArretTram()[NumArretSvt - 1][j]) ||
                        ((v->DistArretTram()[NumArretSvt - 1][i] == v->DistArretTram()[NumArretSvt - 1][j]) &&
                         ((d != AncDir) || // En cas d'égalité, préfère garder la direction courante // v3.5 : || par &&
                                           // !
                          ((i + Numero + cv->TourCrt) % nd > (j + Numero + cv->TourCrt) % nd)))); // sinon tirage au
                                                                                                  // hasard (Ici
                                                                                                  // TourCrt+1)
                }
            }
        }
    }
    for (i = 0; (Dir == 0) && (i < 5); i++)
    { // v3.5 : possibilité de blocage si test n'est pas large (i<5)
        for (j = 0; (Dir == 0) && (j < ndp); j++)
        {
            d = v->DirSensVoieTram(InvDir[AncDir], j);
            Voie = &cv->Voisine(x, y, d);
            if ((OrdreDir[d - 1] == i) &&
                ((!v->ToutDroit) ||    // Direction interdite si : - Obligation d'aller tout droit ET
                 (DirDrnDepl == d)) && //                            pas même direction que lors du dernier déplacement
                (v->NumPlaceTaxi == 0) &&   //                          - Place taxi. v3.6
                (v->NumPlaceVehlib == 0) && //                         - Place de vehlib v5.3
                (((Voie->NumVehicule == 0) && (Voie->NumBus == 0) && (Voie->NumTram == 0) && (Voie->NumTaxi == 0) &&
                  (Voie->NumVehlib == 0)) || // v5.3 (vehlib)
                 ((Voie->NumArretTram != 0) && (Voie->NumArretTram == NumArretSvt)) ||
                 (cv->TourCrt <= 1 + TourDrnDepl + (5 * cv->AttenteMaxVeh * (1 + OrdreDir[d - 1])))))
            { // AttentMaxTram = 5*AttenteMaxVeh
                Dir = static_cast<direction>(d);
            }
        }
    }

    // Si la direction est toujours indéfinie : on prend la meilleure direction possible
    if (Dir == 0)
    {
        for (i = 0; (Dir == 0) && (i < 5); i++)
        { // v3.5 : possibilité de blocage si test n'est pas large (i<5)
            for (j = 0; (Dir == 0) && (j < ndp); j++)
            {
                d = v->DirSensVoieTram(InvDir[AncDir], j);
                if (OrdreDir[d - 1] == i)
                {
                    Dir = static_cast<direction>(d);
                }
            }
        }
    }
    if (DirDrnDepl == 0)
    {
        DirDrnDepl = Dir;
    }
    if (DirAvtDrnDepl == 0)
    {
        DirAvtDrnDepl = Dir;
    }
}
//-----------------------------------------------------------------------------
direction tram::ProchaineDirectionAuCarrefour() const // ATTENTION : DIR DOIT ETRE DEFINIE. v3.0
{
    voie *v = &cv->Voisine(x, y, Dir); // Ici v est la prochaine case
    std::array<int, NBDIR> OrdreDir = {0, 0, 0, 0};
    int d = 0;
    int i = 0;
    int j = 0;
    const int ndp = v->NbDirPossSensVoieTram(InvDir[Dir]); // Nombre de directions possibles
    direction DirAuCarrefour = indefinie;
    // On classe les dir.poss. dans l'ordre < des distances aux parkings si la case est libre.
    // En cas d'égalité, c'est le hasard : (N°Tram.+Tour) Modulo Directions Possibles
    for (i = 0; i < ndp; i++)
    {
        for (j = 0; j < ndp; j++)
        {
            if (i != j)
            {
                d = v->DirSensVoieTram(indefinie, i);
                if ( // Direction possible
                    v->EstDirPossSensVoieTram(InvDir[Dir], static_cast<direction>(d)))
                {
                    // On dégringole si distance supérieure.
                    OrdreDir[d - 1] += static_cast<int>(
                        (v->DistArretTram()[NumArretSvt - 1][i] > v->DistArretTram()[NumArretSvt - 1][j]) ||
                        ((v->DistArretTram()[NumArretSvt - 1][i] == v->DistArretTram()[NumArretSvt - 1][j]) &&
                         ((d != Dir) || // En cas d'égalité, préfère garder la direction courante. // v3.5 : || par && !
                          ((i + Numero + cv->TourCrt) % ndp > (j + Numero + cv->TourCrt) % ndp)))); // tirage au hasard
                                                                                                    // en cas d'égalité
                                                                                                    // (Ici TourCrt+1)
                }
            }
        }
    }
    for (i = 0; (DirAuCarrefour == 0) && (i < 5); i++)
    { // v3.5 : possibilité de blocage si test n'est pas large (i<5)
        for (j = 0; (DirAuCarrefour == 0) && (j < ndp); j++)
        {
            d = v->DirSensVoieTram(InvDir[Dir], j);
            if ((OrdreDir[d - 1] == i) && (v->NumPlaceTaxi == 0) && // Direction
                                                                    // interdite
                                                                    // si : -
                                                                    // Place
                                                                    // taxi. v3.6
                (v->NumPlaceVehlib == 0) &&                         //                         - Place de vehlib v5.3
                ((!v->ToutDroit) || //                          - Obligation d'aller tout droit ET
                 (DirDrnDepl == d)))
            { //                            pas même direction que lors du dernier déplacement
                DirAuCarrefour = static_cast<direction>(d);
            }
        }
    }
    return DirAuCarrefour;
}
//-----------------------------------------------------------------------------
bool tram::PeutPasser() const // v3.0
{
    voie *v = &cv->v[x][y];
    int NumFeuTram = 0;
    return (Dir != 0) && ((v->Priorite[Dir - 1] != stop) || (cv->TourCrt - TourDrnDepl >= 2)) &&
           (((NumFeuTram = v->NumFeu[Dir - 1]) == 0) || (cv->Feu[NumFeuTram - 1].Etat < orange));
}
//-----------------------------------------------------------------------------
int tram::NumeroArretSuivant() const // v3.0
{
    int i = 0;
    for (i = 0; (i < cv->LigneTram[NumLigne - 1].NbArrets) && // v6.0.1 : borne testée d'abord (NumArret[NbArrets] lu
                                                              // par l'original)
                (NumArretSvt != cv->LigneTram[NumLigne - 1].NumArret[i]);
         i++)
    {
        ;
    }
    if (i < cv->LigneTram[NumLigne - 1].NbArrets)
    {
        return cv->LigneTram[NumLigne - 1].NumArret[(i + 1) % cv->LigneTram[NumLigne - 1].NbArrets];
    }
    return cv->LigneTram[NumLigne - 1].NumArret[0];
}
//-----------------------------------------------------------------------------
void tram::Verifie(AnsiString &asMsgErr) const // v3.03
{
    const AnsiString asSource = Format("Tram n°%d (%d,%d)", TableauDeConstantes(Numero, x, y));
    if (Numero < 0)
    {
        asMsgErr += asSource + Format(": numéro incorrect (%d)\n", TableauDeConstantes(Numero));
    }
    VerifiePosition(asMsgErr, asSource, x, y, PEUT_ETRE_INDEFINI);
    VerifieDirection(asMsgErr, asSource, Dir, PEUT_ETRE_INDEFINI);
    if ((NumLigne < 0) || (NumLigne >= NBMAXLIGNESBUSTRAM))
    {
        asMsgErr += asSource + Format(": numéro de ligne de tram incorrect (%d)\n", TableauDeConstantes(NumLigne));
    }
}
//-----------------------------------------------------------------------------
void arret_tram::Copie(arret_tram *a) // v5.1
{
    Numero = a->Numero;
    x = a->x;
    y = a->y;
    for (int i = 0; i < NBMAXLIGNESBUSTRAM; i++)
    {
        Ligne[i] = a->Ligne[i];
        NumLigne[i] = a->NumLigne[i];
    }
    NbLignes = a->NbLignes;
}
//-----------------------------------------------------------------------------
void arret_tram::Initialise() // v3.0
{
    int i = 0;
    x = -1;
    y = -1;
    NbLignes = 0;
    for (i = 0; i < NBMAXLIGNESBUSTRAM; i++)
    {
        Ligne[i] = false;
        NumLigne[i] = 0;
    }
}
//-----------------------------------------------------------------------------
void arret_tram::InitialiseInfosLignesTram() // v3.5
{
    int i = 0;
    NbLignes = 0;
    for (i = 0; i < NBMAXLIGNESBUSTRAM; i++)
    {
        Ligne[i] = false;
        NumLigne[i] = 0;
    }
}
//-----------------------------------------------------------------------------
void arret_tram::AjouteLigneTram(int n) // v3.0
{
    if (!Ligne[n - 1])
    {                             // v6.0.9 : comme pour les arrêts de bus
        NumLigne[NbLignes++] = n; // v3.0.5
    }
    Ligne[n - 1] = true;
}
//-----------------------------------------------------------------------------
AnsiString arret_tram::asListeLignesTram() // v3.0.5
{
    int i = 0;
    AnsiString asListe = "";
    for (i = 0; i < NbLignes; i++)
    {
        if (asListe.IsEmpty())
        {
            asListe = AnsiString(static_cast<char>('A' + NumLigne[i] - 1));
        }
        else
        {
            asListe = asListe + "," + AnsiString(static_cast<char>('A' + NumLigne[i] - 1));
        }
    }
    return asListe;
}
//-----------------------------------------------------------------------------
bool arret_tram::Charge(int hfile) // v3.0
{
    return LitChamp(hfile, &Numero, sizeof(Numero)) && LitChamp(hfile, &x, sizeof(x)) &&
           LitChamp(hfile, &y, sizeof(y)) && LitChamp(hfile, Ligne.data(), NBMAXLIGNESBUSTRAM * sizeof(Ligne[0])) &&
           LitChamp(hfile, &NbLignes, sizeof(NbLignes)) &&
           LitChamp(hfile, NumLigne.data(), NBMAXLIGNESBUSTRAM * sizeof(NumLigne[0]));
}
//-----------------------------------------------------------------------------
bool arret_tram::Sauve(int hfile) // v3.0
{
    int xc = CoordonneeCopiee(x, cv->Xc);
    int yc = CoordonneeCopiee(y, cv->Yc); // v4.0.1. v6.0.1 : COORDCOPIE
    return (FileWrite(hfile, &Numero, sizeof(Numero)) != 0) &&
           (FileWrite(hfile, cv->CopieEnCours ? &xc : &x, sizeof(x)) != 0) && // v4.0.1
           (FileWrite(hfile, cv->CopieEnCours ? &yc : &y, sizeof(y)) != 0) && // v4.0.1
           (FileWrite(hfile, Ligne.data(), NBMAXLIGNESBUSTRAM * sizeof(Ligne[0])) != 0) &&
           (FileWrite(hfile, &NbLignes, sizeof(NbLignes)) != 0) &&
           (FileWrite(hfile, NumLigne.data(), NBMAXLIGNESBUSTRAM * sizeof(NumLigne[0])) != 0);
}
//-----------------------------------------------------------------------------
void arret_tram::Verifie(AnsiString &asMsgErr) // v3.0
{
    int i = 0;
    const AnsiString asSource = Format("Arrêt de tram n°%d (%d,%d)", TableauDeConstantes(Numero, x, y));
    if (Numero < 0)
    {
        asMsgErr += asSource + Format(": numéro incorrect (%d)\n", TableauDeConstantes(Numero));
    }
    VerifiePosition(asMsgErr, asSource, x, y, PEUT_ETRE_INDEFINI);
    if ((NbLignes < 0) || (NbLignes > NBMAXLIGNESBUSTRAM))
    {
        asMsgErr += asSource + Format(": nombre de lignes de tram incorrect (%d)\n", TableauDeConstantes(NbLignes));
    }
    for (i = 0; i < NbLignes; i++)
    {
        if (Ligne[i] && ((NumLigne[i] < 0) || (NumLigne[i] > NBMAXLIGNESBUSTRAM)))
        {
            asMsgErr += asSource + Format(": numéro de la %d° ligne de tram incorrect (%d)\n",
                                          TableauDeConstantes(i + 1, NumLigne[i])); // v3.03
        }
    }
}
//-----------------------------------------------------------------------------
void ligne_tram::Initialise() // v3.0
{
    int i = 0;
    NbTrams = NbArrets = 0;
    for (i = 0; i < NBMAXARRETSBUSTRAMPARLIGNE; i++)
    {
        NumArret[i] = 0;
    }
    for (i = 0; i < NBMAXBUSTRAMPARLIGNE; i++)
    {
        NumTram[i] = 0;
    }
}
//-----------------------------------------------------------------------------
void ligne_tram::AjouteTram(int NumeroTram) // v3.0
{
    NumTram[NbTrams++] = NumeroTram;
}
//-----------------------------------------------------------------------------
bool ligne_tram::Charge(int hfile) // v3.0
{                                  // Attention : On ne stocke pas le nombre et les n° de tram (calculés avant simul)
    return LitChamp(hfile, &Numero, sizeof(Numero)) && LitChamp(hfile, &NbArrets, sizeof(NbArrets)) &&
           LitChamp(hfile, NumArret.data(), NBMAXARRETSBUSTRAMPARLIGNE * sizeof(NumArret[0]));
}
//-----------------------------------------------------------------------------
bool ligne_tram::Sauve(int hfile) // v3.0
{                                 // Attention : On ne stocke pas le nombre et les n° de tram (calculés avant simul)
    return (FileWrite(hfile, &Numero, sizeof(Numero)) != 0) && (FileWrite(hfile, &NbArrets, sizeof(NbArrets)) != 0) &&
           (FileWrite(hfile, NumArret.data(), NBMAXARRETSBUSTRAMPARLIGNE * sizeof(NumArret[0])) != 0);
}
//-----------------------------------------------------------------------------
void ligne_tram::Verifie(AnsiString &asMsgErr) // v3.0
{
    int i = 0;
    const AnsiString asSource = Format("Ligne de tram n°%d", TableauDeConstantes(Numero));
    if (Numero < 0)
    {
        asMsgErr += asSource + Format(": numéro incorrect (%d)\n", TableauDeConstantes(Numero));
    }
    if ((NbArrets < 0) || (NbArrets > NBMAXARRETSBUSTRAMPARLIGNE))
    {
        asMsgErr += asSource + Format(": nombre d'arrêts de tram incorrect (%d)\n", TableauDeConstantes(NbArrets));
    }
    for (i = 0; i < NbArrets; i++)
    {
        if ((NumArret[i] < 0) || (NumArret[i] > cv->NbArretsTram))
        {
            asMsgErr += asSource + Format(": numéro du %d° arrêt de tram incorrect (%d)\n",
                                          TableauDeConstantes(i + 1, NumArret[i])); // v3.03
        }
    }
}
//-----------------------------------------------------------------------------
///////////// ICI FINISSENT LES METHODES DES CLASSES LIEES AU RESEAU DE TRAM //
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
///////////// ICI COMENCENT LES METHODES DES CLASSES LIEES AU RESEAU DE TAXI //
//-----------------------------------------------------------------------------

bool place_taxi::Charge(int hfile)
{
    return LitChamp(hfile, &Numero, sizeof(Numero)) && LitChamp(hfile, &x, sizeof(x)) && LitChamp(hfile, &y, sizeof(y));
}
//-----------------------------------------------------------------------------
bool place_taxi::Sauve(int hfile)
{
    int xc = CoordonneeCopiee(x, cv->Xc);
    int yc = CoordonneeCopiee(y, cv->Yc); // v4.0.1. v6.0.1 : COORDCOPIE
    return (FileWrite(hfile, &Numero, sizeof(Numero)) != 0) &&
           (FileWrite(hfile, cv->CopieEnCours ? &xc : &x, sizeof(x)) != 0) && // v4.0.1
           (FileWrite(hfile, cv->CopieEnCours ? &yc : &y, sizeof(y)) != 0);   // v4.0.1
}
//-----------------------------------------------------------------------------
void place_taxi::Verifie(AnsiString &asMsgErr)
{
    const AnsiString asSource = Format("Place Taxi n°%d (%d,%d)", TableauDeConstantes(Numero, x, y));
    if (Numero < 0)
    {
        asMsgErr += asSource + Format(": numéro incorrect (%d)\n", TableauDeConstantes(Numero));
    }
    if (cv->DistancesCalculees && (Definie() && (NumFile < 0))) // Vérification avant simulation seulement
    {
        asMsgErr += asSource + Format(": numéro de file indéfini ou incorrect (%d)\n", TableauDeConstantes(NumFile));
    }

    VerifiePosition(asMsgErr, asSource, x, y, PEUT_ETRE_INDEFINI);
}
//-----------------------------------------------------------------------------
bool place_taxi::EstQueueFile() const
{
    return cv->v[x][y].EstQueueFileTaxis();
}
//-----------------------------------------------------------------------------
bool place_taxi::EstTeteFile() const
{
    return cv->v[x][y].EstTeteFileTaxis();
}
//-----------------------------------------------------------------------------
int place_taxi::PlaceQueueFile() // Retourne 1+NumPlace si elle existe sinon 0
{
    return cv->v[x][y].PlaceQueueFileTaxi();
}
//-----------------------------------------------------------------------------
int place_taxi::PlaceTeteFile() // Retourne 1+NumPlace si elle existe sinon 0
{
    return cv->v[x][y].PlaceTeteFileTaxi();
}
//-----------------------------------------------------------------------------
direction place_taxi::DirPlaceSvtMemeFile() const
{
    int NumPlaceSvt = 0;
    direction DirSvt = indefinie;
    for (int d = nord; (DirSvt == 0) && (d <= ouest); d++)
    {
        NumPlaceSvt = cv->Voisine(x, y, d).NumPlaceTaxi;
        if ((NumPlaceSvt != 0) && (cv->PlaceTaxi[NumPlaceSvt - 1].NumFile == NumFile))
        {
            DirSvt = static_cast<direction>(d);
        }
    }
    return DirSvt; // Si indefinie, il s'agit de la tête de file !
}
//-----------------------------------------------------------------------------
void taxi::Place(int p) // 0 <= p < NbPlacesTaxi
{
    x = cv->PlaceTaxi[p].x;
    y = cv->PlaceTaxi[p].y;
    cv->v[x][y].NumTaxi = Numero;
    voie *v = &cv->v[x][y];
    if (cv->PlaceTaxi[p].EstTeteFile())
    {
        Situation = stAttente;
        for (int d = nord; (Dir == 0) && (d <= ouest); d++)
        {
            if ((v->DirPoss & PossDir[static_cast<direction>(d)]) != 0)
            {
                Dir = static_cast<direction>(d);
            }
        }
    }
    else
    {
        Situation = stRetour;
        for (int d = nord; (Dir == 0) && (d <= ouest); d++)
        {
            if (((v->DirPoss & PossDir[static_cast<direction>(d)]) != 0) && (cv->Voisine(x, y, d).NumPlaceTaxi != 0))
            {
                Dir = static_cast<direction>(d);
            }
        }
    }
}
//-----------------------------------------------------------------------------
void taxi::CalculeProchaineDirection()
{
    voie *v = &cv->v[x][y];
    voie *Voie = nullptr;
    const direction AncDir = Dir; // v2.2.4
    std::array<int, NBDIR> OrdreDir = {0, 0, 0, 0};
    int i = 0;
    int j = 0;
    const int ndp = NbDir[v->DirPoss];

    if ((Situation == stAttente) &&      // Si le taxi attend,
        (NbPietonsQuiSontMontes() != 0)) // On regarde déjà s'il y a des piétons qui veulent monter...
    {                                    // pour pouvoir partir...
        Situation = stDepart;
        TourDepartDrnCourse = cv->TourCrt;
    }
    Dir = indefinie;
    // On classe les dir.poss. dans l'ordre < des distances aux files.
    // En cas d'égalité, c'est le hasard : (N°Veh.+Tour) Modulo Directions Possibles
    for (i = 0; i < ndp; i++)
    {
        for (j = 0; j < ndp; j++)
        {
            if (i != j)
            { // On dégringole si distance supérieure.
                switch (Situation)
                {
                case stDepart:
                    OrdreDir[i] += (alea::Rand() % 2);
                    break; // Le hasard dans toute sa splendeur (en fait le taxi promène le client)
                case stRetour:
                    OrdreDir[i] += static_cast<int>(
                        (static_cast<int>(
                             (cv->Voisine(x, y, Dirdp[j][v->DirPoss]).NumPlaceTaxi != 0) && // 1. File recherchée
                             (cv->PlaceTaxi[cv->Voisine(x, y, Dirdp[j][v->DirPoss]).NumPlaceTaxi - 1].NumFile ==
                              NumFile)) >
                         static_cast<int>(
                             (cv->Voisine(x, y, Dirdp[i][v->DirPoss]).NumPlaceTaxi != 0) &&
                             (cv->PlaceTaxi[cv->Voisine(x, y, Dirdp[i][v->DirPoss]).NumPlaceTaxi - 1].NumFile ==
                              NumFile))) ||
                        ((v->DistFileTaxi()[NumFile][i] > v->DistFileTaxi()[NumFile][j]) || // 2. Distance à la file
                         ((v->DistFileTaxi()[NumFile][i] == v->DistFileTaxi()[NumFile][j]) &&
                          ((Dirdp[i][v->DirPoss] != AncDir) || // 3. En cas d'égalité, préfère garder la direction
                                                               // courante : v2.2.4. // v3.5 : || par && !
                           ((i + Numero + cv->TourCrt) % ndp > (j + Numero + cv->TourCrt) % ndp))))); // sinon tirage au
                                                                                                      // hasard (Ici
                                                                                                      // TourCrt+1)
                    break;
                default:
                    break; // v5.4 : évite l'avertissement
                }
            }
        }
    }
    for (i = 0; (Dir == 0) && (i < 5); i++)
    {
        for (j = 0; (Dir == 0) && (j < ndp); j++)
        {
            Voie = &cv->Voisine(x, y, Dirdp[j][v->DirPoss]);
            if ((OrdreDir[j] == i) &&
                ((!v->ToutDroit) || // Direction interdite si :     - obligation d'aller tout droit ET
                 (DirDrnDepl == Dirdp[j][v->DirPoss])) && // pas même direction que lors du dernier déplacement. v3.7.1
                                                          // : Erreur ! v5.2.3 : Erreur sur Test même direction
                (((Voie->NumVehicule == 0) && (Voie->NumVehlib == 0) && (Voie->NumBus == 0) && // Sinon... v5.3 (vehlib)
                  (Voie->NumTram == 0) && (Voie->NumTaxi == 0) && (Voie->NumPlaceVehlib == 0)) || //   - Il n'y a rien
                                                                                                  //   sur la route (y
                                                                                                  //   compris place
                                                                                                  //   vehlib v5.3) OU
                 ((Situation == stRetour) && //   - Il est de retour et c'est la file du retour OU
                  ((Voie->NumPlaceTaxi != 0) && (cv->PlaceTaxi[Voie->NumPlaceTaxi - 1].NumFile == NumFile))) ||
                 (cv->TourCrt <= 1 + TourDrnDepl + (cv->AttenteMaxVeh * (1 + OrdreDir[j])))))
            { // - Il est encore dans sa phase d'attente
                Dir = Dirdp[j][v->DirPoss];
            }
        }
    }

    // Si la direction est toujours indéfinie : on prend la meilleure direction possible
    if (Dir == 0)
    {
        for (i = 0; (Dir == 0) && (i < 5); i++)
        { // v3.5 : possibilité de blocage si test n'est pas large (i<5)
            for (j = 0; (Dir == 0) && (j < ndp); j++)
            {
                if (OrdreDir[j] == i)
                {
                    Dir = Dirdp[j][v->DirPoss];
                }
            }
        }
    }
}
//-----------------------------------------------------------------------------
direction taxi::ProchaineDirectionAuCarrefour() const // ATTENTION : DIR DOIT ETRE DEFINIE
{
    voie *v = &cv->Voisine(x, y, Dir); // Ici v est la prochaine case
    std::array<int, NBDIR> OrdreDir = {0, 0, 0, 0};
    int i = 0;
    int j = 0;
    const int ndp = NbDir[v->DirPoss];
    direction DirAuCarrefour = indefinie;
    // On classe les dir.poss. dans l'ordre < des distances aux parkings si la case est libre.
    // En cas d'égalité, c'est le hasard : (N°Veh.+Tour) Modulo Directions Possibles
    for (i = 0; i < ndp; i++)
    {
        for (j = 0; j < ndp; j++)
        {
            if (i != j)
            {
                switch (Situation)
                {
                case stDepart:
                    OrdreDir[i] += (alea::Rand() % 2);
                    break; // Le hasard dans toute sa splendeur (en fait le taxi promène le client)
                case stRetour:
                    OrdreDir[i] += static_cast<int>(
                        (static_cast<int>(
                             (cv->Voisine(x, y, Dirdp[j][v->DirPoss]).NumPlaceTaxi != 0) && // 1. File recherchée
                             (cv->PlaceTaxi[cv->Voisine(x, y, Dirdp[j][v->DirPoss]).NumPlaceTaxi - 1].NumFile ==
                              NumFile)) >
                         static_cast<int>(
                             (cv->Voisine(x, y, Dirdp[i][v->DirPoss]).NumPlaceTaxi != 0) &&
                             (cv->PlaceTaxi[cv->Voisine(x, y, Dirdp[i][v->DirPoss]).NumPlaceTaxi - 1].NumFile ==
                              NumFile))) ||
                        ((v->DistFileTaxi()[NumFile][i] > v->DistFileTaxi()[NumFile][j]) ||
                         ((v->DistFileTaxi()[NumFile][i] == v->DistFileTaxi()[NumFile][j]) &&
                          ((Dirdp[i][v->DirPoss] != Dir) || // En cas d'égalité, préfère garder la direction courante :
                                                            // v2.2.4.  // v3.5 : || par && !
                           ((i + Numero + cv->TourCrt) % ndp > (j + Numero + cv->TourCrt) % ndp))))); // sinon tirage au
                                                                                                      // hasard (Ici
                                                                                                      // TourCrt+1)
                    break;
                default:
                    break; // v5.4 : évite l'avertissement
                }
            }
        }
    }
    for (i = 0; (DirAuCarrefour == 0) && (i < 5); i++)
    { // v3.5 : possibilité de blocage si test n'est pas large (i<5)
        for (j = 0; (DirAuCarrefour == 0) && (j < ndp); j++)
        {
            if ((OrdreDir[j] == i) &&       // Direction interdite si :
                (v->NumPlaceVehlib == 0) && //   - pas de place vehlib (v5.3)
                ((!v->ToutDroit) ||         //   - obligation d'aller tout droit ET
                 (DirDrnDepl == Dirdp[j][v->DirPoss])))
            { // pas même direction que lors du dernier déplacement. v3.7.1 : Erreur ! v5.2.3 : Erreur sur Test même
              // direction
                DirAuCarrefour = Dirdp[j][v->DirPoss];
            }
        }
    }
    return DirAuCarrefour;
}
//-----------------------------------------------------------------------------
bool taxi::PeutPasser() const
{
    voie *v = &cv->v[x][y];
    int NumFeuTaxi = 0;
    return (Dir != 0) && ((v->Priorite[Dir - 1] != stop) || (cv->TourCrt - TourDrnDepl >= 2)) &&
           (((NumFeuTaxi = v->NumFeu[Dir - 1]) == 0) || (cv->Feu[NumFeuTaxi - 1].Etat < orange));
}
//-----------------------------------------------------------------------------
void taxi::FaitDescendrePietons()
{
    int i = 0;
    const direction d = ADroiteDe[Dir];
    voie *v = &cv->Voisine(x, y, d);
    pieton *p = nullptr;
    for (i = 0; i < cv->NbPietons; i++)
    {
        p = &cv->Pieton[i];
        if ((p->NumTaxi == Numero) && p->DecideDeDescendre(cv->ProbaArretTaxiDmd))
        {
            v->FaitDescendrePieton(d, i); // Si on peut pas faire descendre, il descendra une prochaine fois
        }
    }
}
//-----------------------------------------------------------------------------
void taxi::FaitMonterPieton(int NumPietonAFaireMonter)
{
    pieton *p = &cv->Pieton[NumPietonAFaireMonter];
    const int xp = p->x;
    const int yp = p->y;
    if (NbPersonnes < cv->NbMaxPersonnesTaxi)
    {
        NbPersonnes++;
        cv->v[xp][yp].RetirePieton(NumPietonAFaireMonter);
        p->MonteDansTaxi(Numero);          // v3.6
        vue->RedessineCase(xp, yp, false); // portage
    }
}
//-----------------------------------------------------------------------------
int taxi::NbPietonsQuiSontMontes() // retourne le nombre de piétons qui attendaient et qui sont montés
{
    voie *v = nullptr;
    pieton *p = nullptr;
    for (int d = nord; (NbPersonnes <= cv->NbMaxPersonnesTaxi) && (d <= ouest); d++)
    {
        v = &cv->Voisine(x, y, d);
        for (int i = 0; (NbPersonnes <= cv->NbMaxPersonnesTaxi) && (i < NBMAXPIETONSPARCASE); i++)
        {
            if (v->NumPieton[i] != 0)
            {
                p = &cv->Pieton[v->NumPieton[i] - 1];
                if (p->AttendTaxi && (p->NumFileTaxi - 1 == NumFile))
                {
                    FaitMonterPieton(v->NumPieton[i] - 1);
                }
            }
        }
    }
    return NbPersonnes;
}
//-----------------------------------------------------------------------------
void taxi::Verifie(AnsiString &asMsgErr) const
{
    const AnsiString asSource = Format("Taxi n°%d (%d,%d)", TableauDeConstantes(Numero, x, y));
    if (Numero < 0)
    {
        asMsgErr += asSource + Format(": numéro incorrect (%d)\n", TableauDeConstantes(Numero));
    }
    if (NumFile < 0)
    {
        asMsgErr += asSource + Format(": numéro de file indéfini ou incorrect (%d)\n", TableauDeConstantes(NumFile));
    }
    VerifiePosition(asMsgErr, asSource, x, y, DOIT_ETRE_DEFINI);
}
//-----------------------------------------------------------------------------
// Places de parking : v5.0
//-----------------------------------------------------------------------------

bool place_park::Charge(int hfile)
{
    return LitChamp(hfile, &Numero, sizeof(Numero)) && LitChamp(hfile, &x, sizeof(x)) && LitChamp(hfile, &y, sizeof(y));
}
//-----------------------------------------------------------------------------
bool place_park::Sauve(int hfile)
{
    int xc = CoordonneeCopiee(x, cv->Xc);
    int yc = CoordonneeCopiee(y, cv->Yc); // v6.0.1 : COORDCOPIE
    return (FileWrite(hfile, &Numero, sizeof(Numero)) != 0) &&
           (FileWrite(hfile, cv->CopieEnCours ? &xc : &x, sizeof(x)) != 0) &&
           (FileWrite(hfile, cv->CopieEnCours ? &yc : &y, sizeof(y)) != 0);
}
//-----------------------------------------------------------------------------
void place_park::Verifie(AnsiString &asMsgErr) const
{
    const AnsiString asSource = Format("Place prkg n°%d (%d,%d)", TableauDeConstantes(Numero, x, y));
    if (Numero < 0)
    {
        asMsgErr += asSource + Format(": numéro incorrect (%d)\n", TableauDeConstantes(Numero));
    }
    if (cv->DistancesCalculees && (NumFile < 0)) // Vérification avant simulation seulement
    {
        asMsgErr += asSource + Format(": numéro de file indéfini ou incorrect (%d)\n", TableauDeConstantes(NumFile));
    }

    VerifiePosition(asMsgErr, asSource, x, y, PEUT_ETRE_INDEFINI);
}
//-----------------------------------------------------------------------------
bool place_park::EstQueueFile() const
{
    return cv->v[x][y].EstQueueFilePark();
}
//-----------------------------------------------------------------------------
int place_park::PlaceQueueFile() // Retourne 1+NumPlace si elle existe sinon 0
{
    return cv->v[x][y].PlaceQueueFilePark();
}
//-----------------------------------------------------------------------------
voie::voie()
{
    Initialise();
}
//-----------------------------------------------------------------------------
void voie::LibereAttenteTrafic() // v6.0.1
{
    Attente.clear();
    Trafic.clear();
}
//-----------------------------------------------------------------------------
void voie::Initialise()
{
    int i = 0;
    ToutDroit = false;
    PassagePietons = false;
    PassageBus = false; // v3.0
    for (i = 0; i < NBDIR; i++)
    {
        Priorite[i] = a_droite;
        NumFeu[i] = 0;
        NumFeuP[i] = 0; // v5.2
    }
    for (i = 0; i < NBMAXPIETONSPARCASE; i++)
    {
        NumPieton[i] = 0; // v3.5 (sorti de la boucle précédente)
    }
    for (i = 0; i < NBMAXLIGNESBUSTRAM; i++)
    {
        LigneTram[i] = LigneBus[i] = false; // v3.5
    }
    DirPoss = aucune;
    NumSpotVehlib = NumPlaceVehlib = NumProchVL = NumVehlib = NbPietons = NumPlacePark = NumTaxi = NumPlaceTaxi =
        NumArretBus = NumArretTram = NumProchTaxi = NumProchTram = NumProchBus = NumProchVeh = NumParking = NumTram =
            NumBus = NumVehicule = 0; // v3.8 (Nature) v5.0 (Place) v5.3 (vehlib)
    Nature = voies;
    SensVoieTram = aucune_voie;
}
//-----------------------------------------------------------------------------
void voie::Initialise(type_voie TypeVoie) // v3.5
{
    if ((TypeVoie & tvRoute) != 0)
    {
        for (int i = nord; i <= ouest; i++)
        { // v3.7.2
            if (((DirPoss & PossDir[i]) != 0) && (!EstDirPossSensVoieTram(indefinie, static_cast<direction>(i - 1))))
            {
                Priorite[i - 1] = a_droite;
            }
        }
        DirPoss = aucune;
        ToutDroit = PassageBus = false;
        for (bool &LigneBu : LigneBus)
        {
            LigneBu = false;
        }
        NumPlaceVehlib = NumVehlib = NumPlacePark = NumPlaceTaxi = NumArretBus = NumProchTaxi = NumProchBus =
            NumProchVeh = NumTaxi = NumBus = NumVehicule = 0; // v3.7.2 : Pas le parking ! v5.0 (NumPlace) v5.3 (vehlib)
    }
    if ((TypeVoie & tvVoieTram) != 0)
    {
        for (int i = nord; i <= ouest; i++)
        { // v3.7.2
            if (EstDirPossSensVoieTram(indefinie, static_cast<direction>(i - 1)))
            {
                Priorite[i - 1] = a_droite;
            }
        }
        SensVoieTram = aucune_voie;
        for (bool &i : LigneTram)
        {
            i = false;
        }
        NumArretTram = NumProchTram = NumTram = 0;
    }
    SupprimePassagePietonsInvalide();
    for (int i = 0; i < NBDIR; i++)
    {
        if (ExisteVoieDansDirection(TypeVoie, direction(i + 1)))
        {
            Priorite[i] = a_droite;
            NumFeu[i] = 0;
        }
    }
    NbPietons = 0;
}
//-----------------------------------------------------------------------------
voie *voie::v(direction d) const
{ // v3.0
    return &cv->Voisine(x, y, d);
}
//-----------------------------------------------------------------------------
bool voie::PrepareCalculAttenteTrafic()
{
    const int ne = cv->NbParkings;
    LibereAttenteTrafic();  // v6.0.1
    NbParkingsAttente = ne; // v6.0.1
    if (ne > 0)
    {
        Attente.assign(ne + 1, std::vector<int>(ne + 1, 0));
        Trafic.assign(ne + 1, std::vector<int>(ne + 1, 0));
    }
    return true;
}
//-----------------------------------------------------------------------------
void voie::CalculeProchainVehiculeOuBusOuTramOuTaxiOuVehlib() // v3.6. Rq : Nécessite de connaître les 2 prochaines
                                                              // directions de chaque Véhicule/Bus/Tram/ (v5.3) Vehlib
{
    int d = 0;
    int MaxNivP = 0;
    int NumVV = 0;
    int NumLV = 0;                                    // NUMéro du Véhicule Voisin. v3.0
    int NumBV = 0;                                    // NUMéro du vehLib Voisin. v5.3
    int NumTV = 0;                                    // NUMéro du Bus Voisin. v3.0
    int NumXV = 0;                                    // NUMéro du Tram Voisin. v3.5
    std::array<int, 5> NivPriorite = {0, 0, 0, 0, 0}; // NUMéro du taXi Voisin. v3.6
    direction DirCrt{};
    direction DirAuCarrefour{};
    voie *v = nullptr;

    NumProchVL = NumProchTaxi = NumProchTram = NumProchBus = NumProchVeh = 0; // Aucun par défaut. v5.3 (vehlib)
    // 1. S'il y a un piéton, aucun ne peut arriver SAUF (v4.3) :
    if ((!vue->SimulationSanglante()) && // Mode Sanglant OU // portage
        (NbPietons != 0) &&              // NbPietons=NbPiétonsEcrases ALORS on peut rouler dessus (v4.3)
        (NbPietons !=
         static_cast<int>((NumPieton[0] != 0) && (!cv->Pieton[NumPieton[0] - 1].EstVivant())) + // v5.4.1 :
                                                                                                // !EstVivant()
                                                                                                // au lieu
                                                                                                // de
                                                                                                // Ecrase
             static_cast<int>((NumPieton[1] != 0) && (!cv->Pieton[NumPieton[1] - 1].EstVivant()))))
    { // v5.4.1 : !EstVivant() au lieu de Ecrase
        return;
    }
    // 1bis. S'il y a déjà un véhicule et que ce n'est pas un parking, aucun ne peut arriver
    if ((NumVehicule != 0) && (NumParking == 0))
    {
        return;
    }
    // 1ter. S'il y a un bus ou un tram ou un taxi, aucun ne peut arriver. v3.5
    if ((NumVehlib != 0) || (NumBus != 0) || (NumTram != 0) || (NumTaxi != 0))
    {
        return; // v5.3 (ajout vehlib)
    }
    for (d = nord; d <= ouest; d++)
    {
        v = &cv->Voisine(x, y, d);
        NumVV = v->NumVehicule;
        NumXV = v->NumTaxi;   // v3.6
        NumLV = v->NumVehlib; // v5.3
        if (v->EstTeteBus())
        {
            NumBV = v->NumBus;
        }
        else
        {
            NumBV = 0; // v3.0
        }
        if (v->EstTeteTram())
        {
            NumTV = v->NumTram;
        }
        else
        {
            NumTV = 0; // v3.5
        }
        if ((NumPlaceVehlib != 0) && (NumLV == 0))
        {
            continue; // v5.3 : on n'accepte que les vehlibs sur la places de vehlib !
        }
        if ((NumVV != 0) &&   // 1. SOIT 1 Vehicule voisin arrivant sur la case ? Priorité 1
            ((!PassageBus) || // v3.1. Passage Bus interdit sauf...
             (cv->Vehicule[NumVV - 1].NivPriorite > normal))) // pour les urgences
        {
            NivPriorite[d] = static_cast<int>((cv->Vehicule[NumVV - 1].Dir == InvDir[d]) && // 1 VEHICULE Qui
                                                                                            // arrive sur la
                                                                                            // case...
                                              cv->TourDeDeplacementVehicule(NumVV - 1) &&   // Et que c'est son tour
                                                                                            // de déplacement...
                                                                                            // (multivitesse :
                                                                                            // version 1.5). v3.0.4
                                                                                            // (chgtde nom)
                                              ((NumVehicule == 0) || // alors... Soit y a pas de véhicule sur la
                                                                     // case,
                                               (((NumParking != 0) || (NumPlacePark != 0)) && // soit y en a une
                                                                                              // mais c'est un
                                                                                              // parking (v5.0) ou
                                                                                              // place de
                                                                                              // parking...
                                                cv->Vehicule[NumVV - 1].EstDestination(this))));
            if (NivPriorite[d] != 0) // et c'est
                                     // l'arrivée
            {
                if (cv->Vehicule[NumVV - 1].NivPriorite == 0)
                {
                    // 2. Sont-elles obligées de s'arrêter (stop non marqué ou feu non vert) ? Priorité 2
                    NivPriorite[d] += static_cast<int>(
                        ((v->Priorite[InvDir[d] - 1] == stop) &&
                         (cv->Vehicule[NumVV - 1].TourDrnDepl > cv->TourCrt - 2L)) ||
                        ((v->Priorite[InvDir[d] - 1] == feux) && (cv->Feu[v->NumFeu[InvDir[d] - 1] - 1].Etat > vert)));
                    // 3. Doivent-elles céder le passage (clp ou stop marqué) ? Priorité 3
                    NivPriorite[d] += 2 * static_cast<int>((v->Priorite[InvDir[d] - 1] == cedez_le_passage) ||
                                                           ((v->Priorite[InvDir[d] - 1] == stop) &&
                                                            (cv->TourCrt - cv->Vehicule[NumVV - 1].TourDrnDepl >= 2L)));
                    // 4. Véhicules sur voie à priorité à droite (ou feu inactif/vert) ? Priorité 4
                    NivPriorite[d] += 3 * static_cast<int>((v->Priorite[InvDir[d] - 1] == a_droite) ||
                                                           ((v->Priorite[InvDir[d] - 1] == feux) &&
                                                            (cv->Feu[v->NumFeu[InvDir[d] - 1] - 1].Etat < orange)));
                }
                else
                { // 5. Véhicule prioritaire ? Urgence : Priorité 6 ; Police : Priorité 7
                    NivPriorite[d] += 5 + cv->Vehicule[NumVV - 1].NivPriorite; // v2.2
                }
            }
        }
        else // 1bis. SOIT 1 Bus voisin arrivant sur la case ? Priorité 1. v3.0
            if ((NumBV != 0) && (NumVehicule == 0))
            {
                NivPriorite[d] =
                    static_cast<int>((cv->Bus[NumBV - 1].Dir == InvDir[d]) && // 1 BUS Qui arrive sur la case...
                                     cv->TourDeDeplacementBus(NumBV - 1) &&   // ET que c'est son tour de
                                                                              // déplacement... (multivitesse)
                                     (cv->TourCrt >= cv->Bus[NumBV - 1].TourDepartArretSvt) && // ET que c'est son
                                                                                               // tour de départ
                                                                                               // vers le prochain
                                                                                               // arrêt
                                     (NumBus == 0));
                if (NivPriorite[d] != 0) // et qu'y a pas de bus sur la case
                {
                    // 2. Sont-ils obligés de s'arrêter (stop non marqué ou feu non vert) ? Priorité 2
                    NivPriorite[d] += static_cast<int>(
                        ((v->Priorite[InvDir[d] - 1] == stop) && (cv->Bus[NumBV - 1].TourDrnDepl > cv->TourCrt - 2L)) ||
                        ((v->Priorite[InvDir[d] - 1] == feux) && (cv->Feu[v->NumFeu[InvDir[d] - 1] - 1].Etat > vert)));
                    // 3. Doivent-ils céder le passage (clp ou stop marqué) ? Priorité 3
                    NivPriorite[d] += 2 * static_cast<int>((v->Priorite[InvDir[d] - 1] == cedez_le_passage) ||
                                                           ((v->Priorite[InvDir[d] - 1] == stop) &&
                                                            (cv->TourCrt - cv->Bus[NumBV - 1].TourDrnDepl >= 2L)));
                    // 4. Bus sur voie à priorité à droite (ou feu inactif/vert) ? Priorité 4
                    NivPriorite[d] += 3 * static_cast<int>((v->Priorite[InvDir[d] - 1] == a_droite) ||
                                                           ((v->Priorite[InvDir[d] - 1] == feux) &&
                                                            (cv->Feu[v->NumFeu[InvDir[d] - 1] - 1].Etat < orange)));
                }
            }
            else // 1ter. SOIT 1 Tram voisin arrivant sur la case ? Priorité 1. v3.5
                if ((NumTV != 0) && (NumVehicule == 0))
                {
                    NivPriorite[d] =
                        static_cast<int>((cv->Tram[NumTV - 1].Dir == InvDir[d]) && // 1 TRAM Qui arrive sur la case...
                                         cv->TourDeDeplacementTram(NumTV - 1) &&   // ET que c'est son tour de
                                                                                   // déplacement... (multivitesse)
                                         (cv->TourCrt >= cv->Tram[NumTV - 1].TourDepartArretSvt) && // ET que c'est son
                                                                                                    // tour de départ
                                                                                                    // vers le prochain
                                                                                                    // arrêt
                                         (NumTram == 0));
                    if (NivPriorite[d] != 0) // et qu'y a pas de tram sur la case
                    {
                        // 2. Sont-ils obligés de s'arrêter (stop non marqué ou feu non vert) ? Priorité 2
                        NivPriorite[d] += static_cast<int>(((v->Priorite[InvDir[d] - 1] == stop) &&
                                                            (cv->Tram[NumTV - 1].TourDrnDepl > cv->TourCrt - 2L)) ||
                                                           ((v->Priorite[InvDir[d] - 1] == feux) &&
                                                            (cv->Feu[v->NumFeu[InvDir[d] - 1] - 1].Etat > vert)));
                        // 3. Doivent-ils céder le passage (clp ou stop marqué) ? Priorité 3
                        NivPriorite[d] += 2 * static_cast<int>((v->Priorite[InvDir[d] - 1] == cedez_le_passage) ||
                                                               ((v->Priorite[InvDir[d] - 1] == stop) &&
                                                                (cv->TourCrt - cv->Tram[NumTV - 1].TourDrnDepl >= 2L)));
                        // 4. Tram sur voie à priorité à droite (ou feu inactif/vert) ? Priorité 4
                        NivPriorite[d] += 3 * static_cast<int>((v->Priorite[InvDir[d] - 1] == a_droite) ||
                                                               ((v->Priorite[InvDir[d] - 1] == feux) &&
                                                                (cv->Feu[v->NumFeu[InvDir[d] - 1] - 1].Etat < orange)));
                    }
                }
                else // 1ter. SOIT 1 Taxi voisin arrivant sur la case ? Priorité 1. v3.5
                    if ((NumXV != 0) && (NumVehicule == 0))
                    {
                        NivPriorite[d] = static_cast<int>((cv->Taxi[NumXV - 1].Dir == InvDir[d]) && // 1 TAXI Qui
                                                                                                    // arrive sur
                                                                                                    // la case...
                                                          cv->TourDeDeplacementTaxi(NumXV - 1) &&   // ET que c'est
                                                                                                    // son tour de
                                                          // déplacement...
                                                          // (multivitesse)
                                                          ((cv->Taxi[NumXV - 1].Situation % 2) != 0) && // ET qu'il
                                                                                                        // est sur
                                                                                                        // le
                                                                                                        // départ
                                                                                                        // ou le
                                                                                                        // retour
                                                          (NumTaxi == 0));
                        if (NivPriorite[d] != 0) // et qu'y a pas de taxi sur la
                                                 // case
                        {
                            // 2. Sont-ils obligés de s'arrêter (stop non marqué ou feu non vert) ? Priorité 2
                            NivPriorite[d] += static_cast<int>(((v->Priorite[InvDir[d] - 1] == stop) &&
                                                                (cv->Taxi[NumXV - 1].TourDrnDepl > cv->TourCrt - 2L)) ||
                                                               ((v->Priorite[InvDir[d] - 1] == feux) &&
                                                                (cv->Feu[v->NumFeu[InvDir[d] - 1] - 1].Etat > vert)));
                            // 3. Doivent-ils céder le passage (clp ou stop marqué) ? Priorité 3
                            NivPriorite[d] +=
                                2 * static_cast<int>((v->Priorite[InvDir[d] - 1] == cedez_le_passage) ||
                                                     ((v->Priorite[InvDir[d] - 1] == stop) &&
                                                      (cv->TourCrt - cv->Taxi[NumXV - 1].TourDrnDepl >= 2L)));
                            // 4. Taxi sur voie à priorité à droite (ou feu inactif/vert) ? Priorité 4
                            NivPriorite[d] +=
                                3 * static_cast<int>((v->Priorite[InvDir[d] - 1] == a_droite) ||
                                                     ((v->Priorite[InvDir[d] - 1] == feux) &&
                                                      (cv->Feu[v->NumFeu[InvDir[d] - 1] - 1].Etat < orange)));
                        }
                    }
                    else // 1ter. SOIT 1 vehLib voisin arrivant sur la case ? Priorité 1. v5.3
                        if (((NumLV != 0) && (NumVehicule == 0)))
                        {
                            NivPriorite[d] =
                                static_cast<int>((cv->Vehlib[NumLV - 1].Dir == InvDir[d]) && // 1 VEHLIB Qui arrive
                                                                                             // sur la case...
                                                 cv->TourDeDeplacementVehlib(NumLV - 1) &&   // ET que c'est son tour
                                                                                             // de déplacement...
                                                                                             // (multivitesse)
                                                 (cv->Vehlib[NumLV - 1].Situation == svDepart) && // ET qu'il est
                                                                                                  // sur le départ
                                                 (NumVehlib == 0));
                            if (NivPriorite[d] != 0) // et qu'y a pas de taxi sur la case
                            {
                                // 2. Sont-ils obligés de s'arrêter (stop non marqué ou feu non vert) ? Priorité 2
                                NivPriorite[d] +=
                                    static_cast<int>(((v->Priorite[InvDir[d] - 1] == stop) &&
                                                      (cv->Vehlib[NumLV - 1].TourDrnDepl > cv->TourCrt - 2L)) ||
                                                     ((v->Priorite[InvDir[d] - 1] == feux) &&
                                                      (cv->Feu[v->NumFeu[InvDir[d] - 1] - 1].Etat > vert)));
                                // 3. Doivent-ils céder le passage (clp ou stop marqué) ? Priorité 3
                                NivPriorite[d] +=
                                    2 * static_cast<int>((v->Priorite[InvDir[d] - 1] == cedez_le_passage) ||
                                                         ((v->Priorite[InvDir[d] - 1] == stop) &&
                                                          (cv->TourCrt - cv->Vehlib[NumLV - 1].TourDrnDepl >= 2L)));
                                // 4. Taxi sur voie à priorité à droite (ou feu inactif/vert) ? Priorité 4
                                NivPriorite[d] +=
                                    3 * static_cast<int>((v->Priorite[InvDir[d] - 1] == a_droite) ||
                                                         ((v->Priorite[InvDir[d] - 1] == feux) &&
                                                          (cv->Feu[v->NumFeu[InvDir[d] - 1] - 1].Etat < orange)));
                            }
                        }
    }
    // 5. Entre véhicules prioritaires (V.P.) :
    //    V.P. tournant à droite ? Priorité 5
    //    V.P. allant tout droit et pas de V.P. venant de droite ? Priorité 5
    //    V.P. tournant à gauche et pas de V.P. venant de la droite ou de devant ? Priorité 5
    for (d = nord; d <= ouest; d++)
    {
        if (NivPriorite[d] == 4)
        {
            v = &cv->Voisine(x, y, d);
            NumVV = v->NumVehicule;
            NumXV = v->NumTaxi;   // v3.6
            NumLV = v->NumVehlib; // v5.3
            if (v->EstTeteBus())
            {
                NumBV = v->NumBus;
            }
            else
            {
                NumBV = 0; // v3.0
            }
            if (v->EstTeteTram())
            {
                NumTV = v->NumTram;
            }
            else
            {
                NumTV = 0; // v3.5
            }
            if (NumVV != 0)
            {
                DirCrt = cv->Vehicule[NumVV - 1].Dir;
                DirAuCarrefour = cv->Vehicule[NumVV - 1].ProchaineDirectionAuCarrefour();
            }
            else if (NumBV != 0) // v3.0
            {
                DirCrt = cv->Bus[NumBV - 1].Dir;
                DirAuCarrefour = cv->Bus[NumBV - 1].ProchaineDirectionAuCarrefour();
            }
            else if (NumTV != 0) // v3.5
            {
                DirCrt = cv->Tram[NumTV - 1].Dir;
                DirAuCarrefour = cv->Tram[NumTV - 1].ProchaineDirectionAuCarrefour();
            }
            else if (NumXV != 0) // v3.6
            {
                DirCrt = cv->Taxi[NumXV - 1].Dir;
                DirAuCarrefour = cv->Taxi[NumXV - 1].ProchaineDirectionAuCarrefour();
            }
            else if (NumLV != 0) // v5.3
            {
                DirCrt = cv->Vehlib[NumLV - 1].Dir;
                DirAuCarrefour = cv->Vehlib[NumLV - 1].ProchaineDirectionAuCarrefour();
            }
            else
            {
                DirCrt = indefinie;
                DirAuCarrefour = indefinie;
            }
            NivPriorite[d] += static_cast<int>((DirAuCarrefour == ADroiteDe[DirCrt]) ||
                                               ((DirAuCarrefour == DirCrt) && (NivPriorite[ADroiteDe[DirCrt]] < 4)) ||
                                               ((DirAuCarrefour == AGaucheDe[DirCrt]) &&
                                                (NivPriorite[ADroiteDe[DirCrt]] < 4) && (NivPriorite[DirCrt] < 4)));
        }
    }
    for (d = nord; d <= ouest; d++)
    {
        if (NivPriorite[d] > MaxNivP)
        {
            MaxNivP = NivPriorite[d];
            NumProchVeh = cv->Voisine(x, y, d).NumVehicule;
            NumProchBus = cv->Voisine(x, y, d).NumBus;   // v3.0
            NumProchTram = cv->Voisine(x, y, d).NumTram; // v3.5
            NumProchTaxi = cv->Voisine(x, y, d).NumTaxi; // v3.6
            NumProchVL = cv->Voisine(x, y, d).NumVehlib; // v5.3
        }
    }
}
//-----------------------------------------------------------------------------
void voie::DeplaceVehiculeOuBusOuTramOuTaxi() // v3.6 (Taxis)
{
    if ((NumProchVeh != 0) && cv->Vehicule[NumProchVeh - 1].PeutPasser())
    { // Si le prochain véhicule peut arriver sur la case...
        cv->v[cv->Vehicule[NumProchVeh - 1].x][cv->Vehicule[NumProchVeh - 1].y].NumVehicule = 0; // Effacée de la case
                                                                                                 // d'origine
        vue->RedessineCase(cv->Vehicule[NumProchVeh - 1].x, cv->Vehicule[NumProchVeh - 1].y, false); // portage
        if (!cv->Vehicule[NumProchVeh - 1].EstDestination(this))                                     // v5.0
        {
            cv->PlaceVehicule(x, y, NumProchVeh); // v2.2.7. Inclut les stats d'Attente/Trafic
            NumVehicule = NumProchVeh;
        }
        else // Parking : NumVehicule est réservé pour les véhicules qui partent du parking uniquement
        {
            cv->NbToursParcourus += cv->TourCrt - cv->Vehicule[NumProchVeh - 1].TourDepart -
                                    cv->Vehicule[NumProchVeh - 1].NbToursRepos - // v2.2.4
                                    1;
            cv->NbCasesParcourues += cv->Vehicule[NumProchVeh - 1].NbCasesParcourues + 1;
            cv->PlaceVehicule(-1, -1, NumProchVeh); // v2.2.7. Inclut les stats d'Attente/Trafic
            cv->NbVehiculesArrives++;
        }
        cv->NbVehiculesDeplaces++;
        cv->Vehicule[NumProchVeh - 1].NbCasesParcourues++;
        cv->Vehicule[NumProchVeh - 1].TourDrnDepl = cv->TourCrt;
        vue->RedessineCase(x, y, false); // portage
        cv->Vehicule[NumProchVeh - 1].DirDrnDepl = cv->Vehicule[NumProchVeh - 1].Dir;
        NumProchVeh = 0;
    }
    if ((NumProchBus != 0) && cv->Bus[NumProchBus - 1].PeutPasser()) // v3.0
    {                                                                // Si le prochain bus peut arriver sur la case...
        cv->PlaceBus(x, y, NumProchBus);
        NumProchBus = 0;
    }
    if ((NumProchTram != 0) && cv->Tram[NumProchTram - 1].PeutPasser()) // v3.5
    { // Si le prochain tram peut arriver sur la case...
        cv->PlaceTram(x, y, NumProchTram);
        NumProchTram = 0;
    }
    if ((NumProchTaxi != 0) && cv->Taxi[NumProchTaxi - 1].PeutPasser()) // v3.6
    { // Si le prochain taxi peut arriver sur la case...
        cv->v[cv->Taxi[NumProchTaxi - 1].x][cv->Taxi[NumProchTaxi - 1].y].NumTaxi = 0; // Effacée de la case d'origine
        vue->RedessineCase(cv->Taxi[NumProchTaxi - 1].x, cv->Taxi[NumProchTaxi - 1].y, false); // portage
        cv->PlaceVehiculeTaxi(x, y, NumProchTaxi);
        NumProchTaxi = 0;
    }
    if ((NumProchVL != 0) && cv->Vehlib[NumProchVL - 1].PeutPasser()) // v5.3
    { // Si le prochain vehlib peut arriver sur la case...
        cv->v[cv->Vehlib[NumProchVL - 1].x][cv->Vehlib[NumProchVL - 1].y].NumVehlib = 0; // Effacée de la case d'origine
        vue->RedessineCase(cv->Vehlib[NumProchVL - 1].x, cv->Vehlib[NumProchVL - 1].y, false); // portage
        cv->PlaceVehiculeVehlib(x, y, NumProchVL);
        NumProchVL = 0;
    }
}
//-----------------------------------------------------------------------------
bool voie::PietonPeutAller(direction DirPieton) // v2.0
{
    return (NbPietons < NBMAXPIETONSPARCASE) && (NumVehicule == 0) && (NumVehlib == 0) && // v5.3
           (NumBus == 0) &&                                                               // v3.0
           (NumTram == 0) &&                                                              // v3.5
           (NumTaxi == 0) &&                                                              // v3.6
           ((PassagePietons && cv->PassagePietonFranchissable(x, y, DirPieton)) ||        // v5.2
            ((Nature == trottoirs) &&                                                     // v5.2
             (DirPoss == 0) && (SensVoieTram == 0))) &&                                   // v3.5
           ((NumPieton[0] == 0) ||
            (((cv->Pieton[NumPieton[0] - 1].DemiCase != 0) || (cv->Pieton[NumPieton[0] - 1].Dir != DirPieton)) &&
             ((cv->Pieton[NumPieton[0] - 1].DemiCase == 0) ||
              (InvDir[cv->Pieton[NumPieton[0] - 1].Dir] != DirPieton)))) &&
           ((NumPieton[1] == 0) ||
            (((cv->Pieton[NumPieton[1] - 1].DemiCase != 0) || (cv->Pieton[NumPieton[1] - 1].Dir != DirPieton)) &&
             ((cv->Pieton[NumPieton[1] - 1].DemiCase == 0) ||
              (InvDir[cv->Pieton[NumPieton[1] - 1].Dir] != DirPieton))));
}
//-----------------------------------------------------------------------------
bool voie::FaitDescendrePieton(direction DirDescente, int NumPietonAFaireDescendre) // v3.6 (taxis)
{
    int d = 0;
    voie *Voie = nullptr;
    pieton *p = nullptr;
    bool APuDescendre = false;
    if ((NbPietons < NBMAXPIETONSPARCASE) && PietonPeutAller(DirDescente))
    {
        AjoutePieton(NumPietonAFaireDescendre);
        p = &cv->Pieton[NumPietonAFaireDescendre];
        if (p->NumBus != 0) // v3.5
        {
            cv->Bus[p->NumBus - 1].NbPersonnes--;
            p->NumBus = 0;
        }
        if (p->NumTram != 0) // v3.5
        {
            cv->Tram[p->NumTram - 1].NbPersonnes--;
            p->NumTram = 0;
        }
        if (p->NumTaxi != 0) // v3.6
        {
            cv->Taxi[p->NumTaxi - 1].NbPersonnes--;
            p->NumTaxi = 0;
        }
        if (p->NumVehlib != 0) // v6.0.1 : l'original ne gérait pas les vehlibs (le piéton restait à bord)
        {
            cv->Vehlib[p->NumVehlib - 1].NbPersonnes--;
            p->NumVehlib = 0;
        }
        p->x = x;
        p->y = y;
        p->VeutDescendre = false;
        RedessineCase(false);
        return true;
    }

    for (APuDescendre = false, d = nord; (!APuDescendre) && (d <= ouest); d++)
    {
        Voie = v(static_cast<direction>(d));
        if (Voie->PietonPeutAller(static_cast<direction>(d)))
        { // v3.6
            APuDescendre = Voie->FaitDescendrePieton(static_cast<direction>(d), NumPietonAFaireDescendre);
        }
    }
    return APuDescendre;
}
//-----------------------------------------------------------------------------
bool voie::AjoutePieton(int NumPietonAAjouter) // v2.0
{
    int i = 0;
    if (NbPietons < NBMAXPIETONSPARCASE)
    {
        for (i = 0; (i < NBMAXPIETONSPARCASE) && (NumPieton[i] != 0); i++)
        {
            ; // v6.0.1 : borne testée d'abord
        }
        if (i == NBMAXPIETONSPARCASE)
        {
            return false; // v6.0.1 : NbPietons incohérent ; l'original écrivait hors du tableau
        }
        NumPieton[i] = NumPietonAAjouter + 1;
        NbPietons++;
        return true;
    }

    return false;
}
//-----------------------------------------------------------------------------
bool voie::RetirePieton(int NumPietonARetirer) // v2.0
{
    int i = 0;
    if (NbPietons != 0)
    {
        for (i = 0; (i < NBMAXPIETONSPARCASE) && (NumPieton[i] != NumPietonARetirer + 1); i++)
        {
            ; // v6.0.1 : borne testée d'abord
        }
        if (i < NBMAXPIETONSPARCASE)
        {
            NumPieton[i] = 0;
            NbPietons--;
            return true;
        }

        return false;
    }

    return false;
}
//-----------------------------------------------------------------------------
void voie::DeplacePietons() // v3.5 : Inclut l'attente aux arrêts de bus/tram, la montée et la descente du bus/tram
{
    bool DirTrv = false;
    int i = 0;
    int d = 0;
    int dd = 0;
    int n = 0;
    int NbPietonsDehors = 0;
    std::array<int, NBMAXPIETONSPARCASE> NumPietonDehors = {0, 0}; // v5.4.1 : pour savoir si l'on peut effectuer la
                                                                   // contamination
    pieton *p = nullptr;
    pieton *pv = nullptr;
    direction NvDir{};
    for (i = 0; i < NBMAXPIETONSPARCASE; i++)
    {
        if ((NumPieton[i] != 0) && (cv->Pieton[NumPieton[i] - 1].NumBus == 0) && // Ne doit pas être dans un bus, v3.0
            (cv->Pieton[NumPieton[i] - 1].NumTram == 0) &&                       //             ni dans un tram,   v3.5
            (cv->Pieton[NumPieton[i] - 1].NumTaxi == 0) &&                       //             ni dans un taxi,   v3.6
            (cv->Pieton[NumPieton[i] - 1].NumVehlib == 0) &&                     //             ni dans un vehlib  v5.3
            cv->Pieton[NumPieton[i] - 1].EstVivant() && //             ni mort            v4.3. v5.4.1 : !Ecrase
                                                        //             remplacé par EstVivant pour prendre en compte
                                                        //             Mort d'épidémie
            (cv->Pieton[NumPieton[i] - 1].TourDrnDepl < cv->TourCrt))
        {
            NbPietonsDehors++;
            NumPietonDehors[i] = NumPieton[i]; // v5.4.1 : pour savoir si l'on peut effectuer la contamination après
                                               // déplacement
            p = &cv->Pieton[NumPieton[i] - 1];
            NvDir = indefinie;
            if (p->ArretBusTrv && (!PassagePietons))
            {
                n = cv->Voisine(x, y, p->Dir).NumArretBus;
                if (n != 0) // Pas
                            // d'attente
                            // sur un
                            // passage
                            // piéton
                            // (v3.6)
                {           // Si Arrêt de bus trouvé et qu'il arrive dessus alors on attend le bus (bloque le piéton)
                    p->ArretBusTrv = false;
                    p->NumArretBus = n;
                    p->AttendBus = true;
                    p->TourAttBus = cv->TourCrt;
                }
            }
            if (p->ArretTramTrv && (!PassagePietons))
            {
                n = cv->Voisine(x, y, p->Dir).NumArretTram;
                if (n != 0) // Pas d'attente
                            // sur un passage
                            // piéton (v3.6)
                {           // Si Arrêt de tram trouvé et qu'il arrive dessus alors on attend le tram (bloque le piéton)
                    p->ArretTramTrv = false;
                    p->NumArretTram = n;
                    p->AttendTram = true;
                    p->TourAttTram = cv->TourCrt;
                }
            }
            if (p->FileTaxiTrv)
            {
                n = cv->Voisine(x, y, p->Dir).NumPlaceTaxi;
                if (n != 0)
                {                                                                // Si file de taxis trouvée
                    if ((!PassagePietons) && cv->PlaceTaxi[n - 1].EstTeteFile()) // v3.6. Pas sur passage piétons
                    { // Phase 2 : Si File de taxi trouvée ET qu'il arrive dessus et que c'est la tête de la file ALORS
                      // il attend le taxi (bloque le piéton)
                        p->NumFileTaxi = cv->PlaceTaxi[n - 1].NumFile + 1;
                        p->FileTaxiTrv = false;
                        p->AttendTaxi = true;
                        p->TourAttTaxi = cv->TourCrt;
                    }
                    else
                    { // Phase 1 : Recherche de la tête de la file de taxi. ATTENTION ! p->NumFileTaxi>0 n'implique pas
                      // que le piéton attend un taxi comme les bus ou les trams.
                        if (p->NumFileTaxi == 0)
                        {
                            p->NumFileTaxi = cv->PlaceTaxi[n - 1].NumFile + 1; // On fixe la file à chercher
                        }
                        p->ChercheFileTaxi(); // Change p->Dir pour se diriger vers la tête de file
                    }
                }
            }

            if (p->PlaceVehlibTrv)
            {
                n = cv->Voisine(x, y, p->Dir).NumPlaceVehlib;
                if ((n != 0) && (cv->Voisine(x, y, p->Dir).NumVehlib != 0)) // v5.3
                                                                            // Si la place est occupé par un vehlib
                { // Phase 2 : Si Place de vehlib trouvée avec vehlib ET qu'il arrive dessus ALORS il attend de prendre
                  // le vehlib sans tour d'attente (phase suivante dans le code)
                    p->NumPlaceVehlib = n;
                    p->PlaceVehlibTrv = false;
                    p->AttendVehlib = true; // En fait on attend pas car contrairement aux taxis, il y a forcément un
                                            // vehlib donc pas de p->TourAttVehlib
                }
            }

            if (p->AttendBus && (p->TourAttBus + cv->NbMaxToursAttenteBus < cv->TourCrt)) // v3.0.
            { // Si attente de bus trop longue (paramètre), le piéton abandonne
                p->ArretBusTrv = false;
                p->NumArretBus = 0;
                p->AttendBus = false;
                p->TourAttBus = 0;
            }
            if (p->AttendTram && (p->TourAttTram + cv->NbMaxToursAttenteTram < cv->TourCrt)) // v3.5.
            { // Si attente de tram trop longue (paramètre), le piéton abandonne
                p->ArretTramTrv = false;
                p->NumArretTram = 0;
                p->AttendTram = false;
                p->TourAttTram = 0;
            }
            if (p->AttendTaxi && (p->TourAttTaxi + cv->NbMaxToursAttenteTaxi < cv->TourCrt)) // v3.6.
            { // Si attente de taxi trop longue (paramètre), le piéton abandonne
                p->FileTaxiTrv = false;
                p->NumFileTaxi = 0;
                p->AttendTaxi = false;
                p->TourAttTaxi = 0;
            }
            if ((!p->AttendBus) &&    // Le pieton n'attend ni bus
                (!p->AttendTram) &&   //                   ni tram (v3.5)
                (!p->AttendTaxi) &&   //                   ni taxi (v3.6)
                (!p->AttendVehlib) && //                 ni vehlib (v5.3)
                (!p->PsgPtTrv) &&     // Le piéton ne se dirige pas déjà vers un passage piéton OU
                (!p->ArretBusTrv) &&  //                              vers un arrêt de bus OU
                (!p->ArretTramTrv) && //                             vers un arrêt de tram OU (v3.5)
                (!p->FileTaxiTrv) &&  //                              vers une tête de file de taxis OU (v3.6)
                (!p->PlaceVehlibTrv)) //                            vers une place de vehlibs (v5.3)
            { // 1. Recherche de passage piéton, puis 2. d'arrêt de bus (v3.0), 3. d'arrêt de tram (v3.5),
              // 4. de file de taxi (v3.6), 5. de place de vehlib (v5.3)
                const sens SensRecherche = ((p->Dir % 2) != 0) ? horizontal : vertical;
                // Si la recherche est demandée (Proba != 0), l'élément trouvé (NvDir) et le tirage favorable
                const auto Choisit = [&](int Proba, const auto &Cherche) {
                    if (Proba == 0)
                    {
                        return false;
                    }
                    NvDir = Cherche();
                    return (NvDir != 0) && ((alea::Rand() % 100) < Proba); // Probabilité d'y aller (paramètre)
                };
                if (Choisit(cv->ProbaDirPsgPt, [&] { return cv->CherchePassagePietons(x, y, SensRecherche); }))
                { // Il va vers le passage piéton trouvé
                    p->Dir = NvDir;
                    p->PsgPtTrv = true; // Indique qu'il est en train d'aller vers un passage piéton (inhibe les futures
                                        // recherches)
                }
                else if (Choisit(cv->ProbaDirArretBus, [&] { return cv->ChercheArretBus(x, y, SensRecherche); }))
                { // Il va vers l'arrêt de bus
                    p->Dir = NvDir;
                    p->ArretBusTrv = true; // Indique qu'il est en train d'aller vers un arrêt de bus (inhibe les
                                           // futures recherches)
                }
                else if (Choisit(cv->ProbaDirArretTram, [&] { return cv->ChercheArretTram(x, y, SensRecherche); }))
                { // Il va vers l'arrêt de tram
                    p->Dir = NvDir;
                    p->ArretTramTrv = true; // Indique qu'il est en train d'aller vers un arrêt de tram (inhibe les
                                            // futures recherches)
                }
                else if (Choisit(cv->ProbaDirFileTaxi, [&] { return cv->CherchePlaceTaxi(x, y, SensRecherche); }))
                { // Il va vers le tête de file de taxi
                    p->Dir = NvDir;
                    p->FileTaxiTrv = true; // Indique qu'il est en train d'aller vers une tête de file de taxi (inhibe
                                           // les futures recherches)
                }
                else if (Choisit(cv->ProbaDirFileVehlib, [&] { return cv->CherchePlaceVehlib(x, y, SensRecherche); }))
                { // Il va vers la place de vehlib
                    p->Dir = NvDir;
                    p->PlaceVehlibTrv = true; // Indique qu'il est en train d'aller vers une place de vehlib (inhibe
                                              // les futures recherches)
                }
            }
            if ((NvDir == 0) && (!p->AttendBus) && (!p->AttendTram) && (!p->AttendTaxi) &&
                (!p->AttendVehlib)) // La direction n'a pas changé et aucun bus/tram/taxi/vehlib attendu (il marche
                                    // tranquillement) v3.6 (taxi) v5.3 (vehlib)
            {
                if (p->DemiCase != 0) // Demi-case et même direction : il sort de la case
                {
                    if (cv->Voisine(x, y, p->Dir).PietonPeutAller(static_cast<direction>(p->Dir)))
                    {
                        cv->DeplacePieton(NumPieton[i] - 1, (x + dx[p->Dir]) % cv->NbX, (y + dy[p->Dir]) % cv->NbY,
                                          static_cast<direction>(p->Dir));
                    }
                    else // Peut pas sortir : on cherche une autre direction à gauche ou à droite (au sort)
                    {
                        if (cv->TourCrt - cv->Pieton[NumPieton[i] - 1].TourDrnDepl > 1 + (cv->AttenteMaxVeh / 2))
                        { // S'il a assez attendu (Paramètre AttenteVéhicule/2)
                            for (DirTrv = false, dd = 1 - (2 * (alea::Rand() % 2)), d = 2 - dd;
                                 (!DirTrv) && ((d != 0) && (d < NBDIR)); d += dd)
                            {
                                NvDir = static_cast<direction>(1 + ((p->Dir + d - 1) % NBDIR)); // à Gauche ou à droite
                                DirTrv = cv->Voisine(x, y, NvDir).PietonPeutAller(NvDir);
                                if (DirTrv)
                                {
                                    p->AnnuleRechercheEnCours(); // Toute recherche est annulée car la direction a
                                                                 // changé !
                                    cv->DeplacePieton(NumPieton[i] - 1, (x + dx[NvDir]) % cv->NbX,
                                                      (y + dy[NvDir]) % cv->NbY, NvDir);
                                }
                            }
                        }
                    }
                }
                else // Ben c'est la demi-case...
                {    // On regarde si on peut passer la demi-case !
                    if (NumPieton[1 - i] != 0)
                    {
                        pv = &cv->Pieton[NumPieton[1 - i] - 1]; // pv comme piéton voisin
                        if ((((pv->Dir == p->Dir) && (pv->DemiCase != 0)) ||
                             ((pv->Dir == InvDir[p->Dir]) && (pv->DemiCase == 0))) &&
                            (cv->TourCrt - cv->Pieton[NumPieton[i] - 1].TourDrnDepl > 1 + (cv->AttenteMaxVeh / 2)))
                        // On choisit une direction à gauche ou à droite (sûre) s'il a assez attendu
                        {
                            p->AnnuleRechercheEnCours(); // Toute recherche est annulée car la direction a changé !
                            p->Dir = static_cast<direction>(1 + ((p->Dir + 1 + (2 * (alea::Rand() % 2))) % NBDIR));
                        }
                    }
                    p->DemiCase++;
                    p->DemiCasesParcourues++; // v3.0
                    p->TourDrnDepl = cv->TourCrt;
                }
            }
        }
    }
    // v5.4.1 Contamination si mode épidémique
    if (vue->EpidemieActivee()) // portage
    {
        // 1. Infection par contact
        if (NbPietonsDehors == 2)
        {
            const int np1 = NumPietonDehors[0];
            const int np2 = NumPietonDehors[1];
            pieton *p1 = &cv->Pieton[np1 - 1];
            pieton *p2 = &cv->Pieton[np2 - 1];
            if (p1->EstContagieux() && ((cv->EpidemieReinfections != 0) || (!p2->EstGueri())))
            { // v5.4.2 : Si réinfections possibles (nouveau paramètre) alors un guéri peut être réinfecté
                p2->EstContaminePar(np1, p1->DonneRang() + 1);
            }
            if (p2->EstContagieux() && ((cv->EpidemieReinfections != 0) || (!p1->EstGueri())))
            { // v5.4.2 : Si réinfections possibles (nouveau paramètre) alors un guéri peut être réinfecté
                p1->EstContaminePar(np2, p2->DonneRang() + 1);
            }
        }

        // 2. Infection fatale
        for (i = 0; i < NBMAXPIETONSPARCASE; i++)
        {
            if (NumPietonDehors[i] != 0)
            {
                p = &cv->Pieton[NumPietonDehors[i] - 1];
                if ((!p->Mort) && p->DoitSuccomber())
                {
                    p->SuccombeInfection();
                    if (p->Defini())
                    {
                        cv->v[p->x][p->y].RedessineCase(false);
                    }
                }
            }
        }
    }
    vue->RedessineCase(x, y, false); // portage
}
//-----------------------------------------------------------------------------
void voie::RetireArretBus() // v3.0
{
    const int Numero = NumArretBus;
    int d = 0;
    NumArretBus = 0;
    if (vue)
    {
        vue->RedessineCase(x, y, true); // portage
    }
    for (d = nord; d <= ouest; d++)
    {
        if (cv->Voisine(x, y, d).NumArretBus == Numero)
        {
            cv->Voisine(x, y, d).RetireArretBus();
        }
    }
}
//-----------------------------------------------------------------------------
void voie::RetireArretTram() // v3.5
{
    const int Numero = NumArretTram;
    int d = 0;
    NumArretTram = 0;
    if (vue)
    {
        vue->RedessineCase(x, y, true); // portage
    }
    for (d = nord; d <= ouest; d++)
    {
        if (cv->Voisine(x, y, d).NumArretTram == Numero)
        {
            cv->Voisine(x, y, d).RetireArretTram();
        }
    }
}
//-----------------------------------------------------------------------------
void voie::RetirePlaceTaxi() // v3.6
{
    NumPlaceTaxi = 0;
    if (vue)
    {
        vue->RedessineCase(x, y, true); // portage
    }
}
//-----------------------------------------------------------------------------
void voie::RetirePlacePark() // v5.0
{
    NumPlacePark = 0;
    if (vue)
    {
        vue->RedessineCase(x, y, true); // portage
    }
}
//-----------------------------------------------------------------------------
void voie::RetirePlaceVehlib() // v5.3
{
    NumPlaceVehlib = 0;
    if (vue)
    {
        vue->RedessineCase(x, y, true); // portage
    }
}
//-----------------------------------------------------------------------------
bool voie::EstTeteArretBus() const // v3.0
{
    return (NumArretBus != 0) && (cv->ArretBus[NumArretBus - 1].x == x) && (cv->ArretBus[NumArretBus - 1].y == y);
}
//-----------------------------------------------------------------------------
bool voie::EstTeteArretTram() const // v3.5
{
    return (NumArretTram != 0) && (cv->ArretTram[NumArretTram - 1].x == x) && (cv->ArretTram[NumArretTram - 1].y == y);
}
//-----------------------------------------------------------------------------
bool voie::EstTeteArretBus(int NumeroArretBus) const // v3.0
{
    return (NumArretBus == NumeroArretBus) && (cv->ArretBus[NumeroArretBus - 1].x == x) &&
           (cv->ArretBus[NumeroArretBus - 1].y == y);
}
//-----------------------------------------------------------------------------
bool voie::EstTeteArretTram(int NumeroArretTram) const // v3.5
{
    return (NumArretTram == NumeroArretTram) && (cv->ArretTram[NumeroArretTram - 1].x == x) &&
           (cv->ArretTram[NumeroArretTram - 1].y == y);
}
//-----------------------------------------------------------------------------
bool voie::EstTeteBus() const // v3.0
{
    return (NumBus != 0) && (cv->Bus[NumBus - 1].x == x) && (cv->Bus[NumBus - 1].y == y);
}
//-----------------------------------------------------------------------------
bool voie::EstTeteTram() const // v3.5
{
    return (NumTram != 0) && (cv->Tram[NumTram - 1].x == x) && (cv->Tram[NumTram - 1].y == y);
}
//-----------------------------------------------------------------------------
bool voie::EstQueueFileTaxis() const // v3.6
{
    bool EstQueueFile = (NumPlaceTaxi > 0);
    voie *v = nullptr;
    for (int d = nord; EstQueueFile && (d <= ouest); d++)
    {
        v = &cv->Voisine(x, y, d);
        EstQueueFile = ((v->DirPoss & InvPossDir[d]) == aucune) || (v->NumPlaceTaxi == 0);
    }
    return EstQueueFile;
}
//-----------------------------------------------------------------------------
bool voie::EstTeteFileTaxis() const // v3.6
{
    bool EstTeteFile = (NumPlaceTaxi > 0);
    voie *v = nullptr;
    for (int d = nord; EstTeteFile && (d <= ouest); d++)
    {
        v = &cv->Voisine(x, y, d);
        EstTeteFile = (((DirPoss & PossDir[d]) == aucune) || (v->NumPlaceTaxi == 0));
    }
    return EstTeteFile;
}
//-----------------------------------------------------------------------------
int voie::PlaceQueueFileTaxi() // v3.6. Retourne 1+NumPlace si elle existe sinon 0
{
    int NumPlaceQueue = 0;
    voie *v = nullptr;
    if (NumPlaceTaxi == 0)
    {
        return 0; // N'est pas une place de taxi
    }
    for (int d = nord; (NumPlaceQueue == 0) && (d <= ouest); d++)
    {
        v = &cv->Voisine(x, y, d);
        if (((v->DirPoss & InvPossDir[d]) != 0) && (v->NumPlaceTaxi != 0))
        {
            NumPlaceQueue = v->PlaceQueueFileTaxi();
        }
    }
    if (NumPlaceQueue == 0)
    {
        return NumPlaceTaxi; // Pas d'autre place trouvée alors c'est la queue
    }
    return NumPlaceQueue; // Sinon c'est celle qui a été trouvée
}
//-----------------------------------------------------------------------------
int voie::PlaceTeteFileTaxi() // v3.6. Retourne 1+NumPlace si elle existe sinon 0
{
    int NumPlaceTete = 0;
    voie *v = nullptr;
    if (NumPlaceTaxi == 0)
    {
        return 0; // N'est pas une place de taxi
    }
    if (EstTeteFileTaxis())
    {
        return NumPlaceTaxi;
    }
    for (int d = nord; (NumPlaceTete == 0) && (d <= ouest); d++)
    {
        v = &cv->Voisine(x, y, d);
        if (((DirPoss & PossDir[d]) != 0) && (v->NumPlaceTaxi != 0))
        {
            NumPlaceTete = v->PlaceTeteFileTaxi();
        }
    }
    if (NumPlaceTete == 0)
    {
        return NumPlaceTaxi; // Pas d'autre place trouvée alors c'est la tête
    }
    return NumPlaceTete; // Sinon c'est celle qui a été trouvée
}
//-----------------------------------------------------------------------------
bool voie::ExistePlusieursQueuesFileTaxis() const // v3.6
{
    int NbQueues = 0;
    voie *v = nullptr;
    if (NumPlaceTaxi != 0)
    {
        for (int d = nord; d <= ouest; d++)
        {
            v = &cv->Voisine(x, y, d);
            NbQueues += static_cast<int>(((v->DirPoss & InvPossDir[d]) != 0) && (v->NumPlaceTaxi != 0));
        }
    }
    return NbQueues > 1;
}
//-----------------------------------------------------------------------------
bool voie::ExistePlusieursTetesFileTaxis() const // v3.6
{
    int NbTetes = 0;
    voie *v = nullptr;
    if (NumPlaceTaxi != 0)
    {
        for (int d = nord; d <= ouest; d++)
        {
            v = &cv->Voisine(x, y, d);
            NbTetes += static_cast<int>(((v->DirPoss & PossDir[d]) != 0) && (v->NumPlaceTaxi != 0));
        }
    }
    return NbTetes > 1;
}

//-----------------------------------------------------------------------------
bool voie::EstQueueFilePark() const // v5.0
{
    bool EstQueueFile = (NumPlacePark > 0);
    voie *v = nullptr;
    for (int d = nord; EstQueueFile && (d <= ouest); d++)
    {
        v = &cv->Voisine(x, y, d);
        EstQueueFile = ((v->DirPoss & InvPossDir[d]) == aucune) || (v->NumPlacePark == 0);
    }
    return EstQueueFile;
}
//-----------------------------------------------------------------------------
bool voie::EstTeteFilePark() const // v5.0
{
    bool EstTeteFile = (NumPlacePark > 0);
    voie *v = nullptr;
    for (int d = nord; EstTeteFile && (d <= ouest); d++)
    {
        v = &cv->Voisine(x, y, d);
        EstTeteFile = (((DirPoss & PossDir[d]) == aucune) || (v->NumPlacePark == 0));
    }
    return EstTeteFile;
}
//-----------------------------------------------------------------------------
int voie::PlaceQueueFilePark() // v5.0. Retourne 1+NumPlace si elle existe sinon 0
{
    int NumPlaceQueue = 0;
    voie *v = nullptr;
    if (NumPlacePark == 0)
    {
        return 0; // N'est pas une place de parking
    }
    for (int d = nord; (NumPlaceQueue == 0) && (d <= ouest); d++)
    {
        v = &cv->Voisine(x, y, d);
        if (((v->DirPoss & InvPossDir[d]) != 0) && (v->NumPlacePark != 0))
        {
            NumPlaceQueue = v->PlaceQueueFilePark();
        }
    }
    if (NumPlaceQueue == 0)
    {
        return NumPlacePark; // Pas d'autre place trouvée alors c'est la queue
    }
    return NumPlaceQueue; // Sinon c'est celle qui a été trouvée
}
//-----------------------------------------------------------------------------
bool voie::ExistePlusieursQueuesFilePark() const // v5.0
{
    int NbQueues = 0;
    voie *v = nullptr;
    if (NumPlacePark != 0)
    {
        for (int d = nord; d <= ouest; d++)
        {
            v = &cv->Voisine(x, y, d);
            NbQueues += static_cast<int>(((v->DirPoss & InvPossDir[d]) != 0) && (v->NumPlacePark != 0));
        }
    }
    return NbQueues > 1;
}
//-----------------------------------------------------------------------------
bool voie::ExistePlusieursTetesFilePark() const // v5.0
{
    int NbTetes = 0;
    voie *v = nullptr;
    if (NumPlacePark != 0)
    {
        for (int d = nord; d <= ouest; d++)
        {
            v = &cv->Voisine(x, y, d);
            NbTetes += static_cast<int>(((v->DirPoss & PossDir[d]) != 0) && (v->NumPlacePark != 0));
        }
    }
    return NbTetes > 1;
}
//-----------------------------------------------------------------------------
int voie::OrientationPlaceVehlib() const // v5.3 : Permet d'orienter l'image de la place selon l'orientation de la file
                                         // !
{
    int Orientation = 0;
    if (NumPlaceVehlib != 0)
    {
        for (int d = nord; (Orientation == 0) && (d <= ouest); d++)
        {
            if ((DirPoss & PossDir[static_cast<direction>(d)]) != 0)
            {
                Orientation = d - 1;
            }
        }
    }
    return Orientation;
}
//-----------------------------------------------------------------------------
void voie::DefinitArretBus(int n) // v3.0
{
    cv->DefinitArretBus(n, x, y);
}
//-----------------------------------------------------------------------------
void voie::DefinitArretTram(int n) // v3.5
{
    cv->DefinitArretTram(n, x, y);
}
//-----------------------------------------------------------------------------
void voie::RedessineCasesArretBus(direction d0) // v3.0
{
    if (vue)
    {
        vue->RedessineCase(x, y, false); // portage
    }
    if (d0 != 0)
    {
        if (cv->Voisine(x, y, d0).NumArretBus == NumArretBus)
        {
            cv->Voisine(x, y, d0).RedessineCasesArretBus(d0);
        }
    }
    else
    {
        for (int d = nord; d <= ouest; d++)
        {
            if (cv->Voisine(x, y, d).NumArretBus == NumArretBus)
            {
                cv->Voisine(x, y, d).RedessineCasesArretBus(static_cast<direction>(d));
            }
        }
    }
}
//-----------------------------------------------------------------------------
void voie::RedessineCasesArretTram(direction d0) // v3.5
{
    if (vue)
    {
        vue->RedessineCase(x, y, false); // portage
    }
    if (d0 != 0)
    {
        if (cv->Voisine(x, y, d0).NumArretTram == NumArretTram)
        {
            cv->Voisine(x, y, d0).RedessineCasesArretTram(d0);
        }
    }
    else
    {
        for (int d = nord; d <= ouest; d++)
        {
            if (cv->Voisine(x, y, d).NumArretTram == NumArretTram)
            {
                cv->Voisine(x, y, d).RedessineCasesArretTram(static_cast<direction>(d));
            }
        }
    }
}
//-----------------------------------------------------------------------------
bool voie::EstDepotBus() const // v3.0
{
    return (x == cv->DepotBus.x) && (y == cv->DepotBus.y);
}
//-----------------------------------------------------------------------------
// Distance simple (une par direction possible) à un élément : arrêt de bus (v3.0), file de taxis (v3.6), place de
// vehlib (v5.3). La case reçoit Distance en venant de DirOrig, puis la propage aux cases qui mènent à elle : sa
// voisine et, si celle-ci oblige à aller tout droit, la case suivante (v3.9.1). 6.0.17 : un parcours pour les trois
// distances simples, au lieu de trois copies. 6.0.19 : Dist contient les distances de toutes les cases à l'élément
// (table_distances::Element), à l'indice IndiceDist de chacune
void voie::CalculeDistance(std::span<int> Dist, int Distance, direction DirOrig)
{
    if (DirOrig != 0)
    {
        Dist[IndiceDist + NumDirdp[DirOrig][DirPoss]] = Distance;
    }
    for (int d = nord; d <= ouest; d++)
    {
        voie &VoieCrt = cv->Voisine(x, y, d);
        voie &Voie2Cases = cv->Voisine(x, y, 2, d);
        const direction Retour = InvDir[d];
        const dirposs PossRetour = InvPossDir[d];
        if (((VoieCrt.DirPoss & PossRetour) == PossRetour) && // Si VoieCrt mène ici ET sa distance s'en trouve réduite
            (Dist[VoieCrt.IndiceDist + NumDirdp[Retour][VoieCrt.DirPoss]] > Distance + 1))
        {
            VoieCrt.CalculeDistance(Dist, Distance + 1, Retour);
        }
        if (VoieCrt.ToutDroit && // v3.9.1 (On traite le ToutDroit : 2 cases)
            ((Voie2Cases.DirPoss & PossRetour) == PossRetour) &&
            (Dist[Voie2Cases.IndiceDist + NumDirdp[Retour][Voie2Cases.DirPoss]] > Distance + 2))
        {
            Voie2Cases.CalculeDistance(Dist, Distance + 2, Retour);
        }
    }
}

//-----------------------------------------------------------------------------
// Distances doubles à un élément, sans et avec les passages de bus (SANSPSGBUS, AVECPSGBUS) : parking (v3.5), file
// de parkings (v5.0). Même parcours que les distances simples, avec deux différences : une case qui oblige à aller
// tout droit part de la distance de la case d'en face (v3.9.1), et une voisine de passage de bus ne propage que la
// distance avec passages de bus. 6.0.17 : un parcours pour les deux, au lieu de deux copies
void voie::CalculeDistance(std::span<distance_double> Dist, int DistSansPsgBus, int DistAvecPsgBus, direction DirOrig)
{
    if (DirOrig != 0)
    { // v3.5
        distance_double &Ici = Dist[IndiceDist + NumDirdp[DirOrig][DirPoss]];
        Ici[SANSPSGBUS] = std::min(Ici[SANSPSGBUS], DistSansPsgBus);
        Ici[AVECPSGBUS] = std::min(Ici[AVECPSGBUS], DistAvecPsgBus);
    }
    for (int d = nord; d <= ouest; d++)
    { // ATTENTION, faire un saut à la case suivante si obligation d'aller tout droit car risque qu'elle ne soit pas
      // définie plus tard par isolation (on ne passe pas deux fois sur une même case)
        int DistCrtAvecPsgBus = DistAvecPsgBus;
        int DistCrtSansPsgBus = DistSansPsgBus; // Distances par défaut (v3.9.1)
        voie &VoieCrt = cv->Voisine(x, y, d);
        voie &Voie2Cases = cv->Voisine(x, y, 2, d);
        const direction Retour = InvDir[d];
        const dirposs PossRetour = InvPossDir[d];
        // SI ToutDroit ET d est une direction possible ALORS on ne prend que la distance de la case d'en face (v3.9.1),
        // incrémentée de 1 si elle est définie
        if (ToutDroit && ((DirPoss & PossRetour) == PossRetour))
        {
            const voie &VoieOpposee = cv->Voisine(x, y, Retour);
            const distance_double &DistOpposee = Dist[VoieOpposee.IndiceDist + NumDirdp[Retour][VoieOpposee.DirPoss]];
            DistCrtAvecPsgBus = DistOpposee[AVECPSGBUS];
            DistCrtSansPsgBus = DistOpposee[SANSPSGBUS];
            if (DistCrtAvecPsgBus != DIST_INDEFINIE)
            {
                DistCrtAvecPsgBus++;
            }
            if (DistCrtSansPsgBus != DIST_INDEFINIE)
            {
                DistCrtSansPsgBus++;
            }
        }
        // Propagation à la voisine si elle mène ici, puis à la case suivante si la voisine oblige à aller tout droit
        // (v3.9.1). Après une voisine de passage de bus, seule la distance avec passages de bus est propagée
        if (VoieCrt.PassageBus)
        {
            if (((VoieCrt.DirPoss & PossRetour) == PossRetour) &&
                (Dist[VoieCrt.IndiceDist + NumDirdp[Retour][VoieCrt.DirPoss]][AVECPSGBUS] > DistCrtAvecPsgBus + 1))
            {
                VoieCrt.CalculeDistance(Dist, DIST_INDEFINIE, DistCrtAvecPsgBus + 1, Retour);
            }
            if (VoieCrt.ToutDroit && ((Voie2Cases.DirPoss & PossRetour) == PossRetour) &&
                (Dist[Voie2Cases.IndiceDist + NumDirdp[Retour][Voie2Cases.DirPoss]][AVECPSGBUS] >
                 DistCrtAvecPsgBus + 2))
            {
                Voie2Cases.CalculeDistance(Dist, DIST_INDEFINIE, DistCrtAvecPsgBus + 2, Retour);
            }
        }
        else
        {
            if ((VoieCrt.DirPoss & PossRetour) == PossRetour)
            {
                const distance_double &DistCrt = Dist[VoieCrt.IndiceDist + NumDirdp[Retour][VoieCrt.DirPoss]];
                if ((DistCrt[AVECPSGBUS] > DistCrtAvecPsgBus + 1) || (DistCrt[SANSPSGBUS] > DistCrtSansPsgBus + 1))
                {
                    VoieCrt.CalculeDistance(
                        Dist, (DistCrtSansPsgBus == DIST_INDEFINIE) ? DIST_INDEFINIE : DistCrtSansPsgBus + 1,
                        DistCrtAvecPsgBus + 1, Retour);
                }
            }
            if (VoieCrt.ToutDroit && ((Voie2Cases.DirPoss & PossRetour) == PossRetour))
            {
                const distance_double &Dist2Cases = Dist[Voie2Cases.IndiceDist + NumDirdp[Retour][Voie2Cases.DirPoss]];
                if ((Dist2Cases[AVECPSGBUS] > DistCrtAvecPsgBus + 2) ||
                    (Dist2Cases[SANSPSGBUS] > DistCrtSansPsgBus + 2))
                {
                    Voie2Cases.CalculeDistance(
                        Dist, (DistCrtSansPsgBus == DIST_INDEFINIE) ? DIST_INDEFINIE : DistCrtSansPsgBus + 2,
                        DistCrtAvecPsgBus + 2, Retour);
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
void voie::CalculeDistanceArretTram(std::span<int> Dist, int Distance, direction DirOrig) // v3.5
{ // 6.0.19 : Dist, les distances de toutes les cases à l'arrêt, à l'indice IndiceDistTram de chacune
    if (DirOrig != 0)
    {
        Dist[IndiceDistTram + NumDirPossSensVoieTram(indefinie, DirOrig)] = Distance;
    }
    for (int d = nord; d <= ouest; d++)
    { // v5.4 : manquait parenthèses autour de b&&c (a||(b&&c))
        if (((DirOrig != 0) && EstDirPossSensVoieTram(static_cast<direction>(d), DirOrig)) || // Si une voie va vers
                                                                                              // DirOrig (définie)
                                                                                              // depuis d OU
            ((DirOrig == 0) && EstDirPossSensVoieTram(indefinie, static_cast<direction>(d)))) // on part d'un arrêt de
                                                                                              // tram
                                                                                              // (DirOrig==indefinie) et
                                                                                              // on regarde toutes les
                                                                                              // directions
        {
            voie *VoieCrt = v(static_cast<direction>(d));
            if (Dist[VoieCrt->IndiceDistTram + VoieCrt->NumDirPossSensVoieTram(indefinie, InvDir[d])] > Distance + 1)
            {                                                                     // Distance indéfinie
                VoieCrt->CalculeDistanceArretTram(Dist, Distance + 1, InvDir[d]); // On lance le calcul de distance
            }
        }
    }
}
//-----------------------------------------------------------------------------
void voie::InitialiseInfosLignesBus()
{
    for (bool &LigneBu : LigneBus)
    {
        LigneBu = false; // v3.0.3
    }
}
//-----------------------------------------------------------------------------
void voie::ModifieSensVoieTram(direction DirOrig, direction DirDest) // v3.5
{
    SensVoieTram = static_cast<sens_voie_tram>(
        (static_cast<int>(SensVoieTram)) ^ (static_cast<int>(SensVoieTramDir[DirOrig][DirDest]))); // v5.4 : conversion
                                                                                                   // BCB6>BCBX pas de
                                                                                                   // a^=b sur énumérés.
    RedessineCase(true);
}
//-----------------------------------------------------------------------------
direction voie::DirSensVoieTram(direction DirOrigine, int iDir) // v3.5
{                                                               // ATTENTION : 0<=iDir<=3 (0 = première, etc.)
    int n = 0;
    direction DirTrv = indefinie;
    for (int DirCrt = nord; (DirTrv == 0) && (DirCrt <= ouest); DirCrt++)
    {
        if (EstDirPossSensVoieTram(DirOrigine, static_cast<direction>(DirCrt)) && (iDir == n++))
        {
            DirTrv = static_cast<direction>(DirCrt);
        }
    }
    return DirTrv;
}
//-----------------------------------------------------------------------------
bool voie::EstDirPossSensVoieTram(direction DirOrigine, direction DirDestination) const // v3.5
{
    int i = 0;
    for (i = 32; (i != 0) && (!DirPossSensVoieTramBase[SensVoieTramBase[SensVoieTram & i]][DirOrigine][DirDestination]);
         i /= 2)
    {
        ;
    }
    return i > 0;
}
//-----------------------------------------------------------------------------
int voie::NumDirPossSensVoieTram(direction DirOrigine, direction DirDestination) // v3.5
{ // Attention : SUPPOSE DEJA que DirDestination est une direction possible de voie::SensVoieTram
    int i = 0;
    for (int DirCrt = nord; (DirDestination != DirCrt) && (DirCrt <= ouest); DirCrt++)
    {
        if (EstDirPossSensVoieTram(DirOrigine, static_cast<direction>(DirCrt)))
        {
            i++;
        }
    }
    return i;
}
//-----------------------------------------------------------------------------
int voie::NbDirPossSensVoieTram(direction DirOrigine) const // v3.5
{
    int n = 0;
    for (int DirDestination = nord; DirDestination <= ouest; DirDestination++)
    {
        if (EstDirPossSensVoieTram(DirOrigine, static_cast<direction>(DirDestination)))
        {
            n++;
        }
    }
    return n;
}
//-----------------------------------------------------------------------------
bool voie::SupprimePassagePietonsInvalide() // v3.5
{
    if (PassagePietons && (cv->SensPassagePietonsPossible(x, y) == 0))
    {
        PassagePietons = false;
        return true;
    }
    return false;
}
//-----------------------------------------------------------------------------
bool voie::ExisteVoieDansDirection(type_voie TypeVoie, direction d) // v5.4 : pas de inline pour éviter avertissement
                                                                    // "non trouvé"
{                                                                   // v3.5 // condition nécessaire d>indefinie
    return (((TypeVoie & tvRoute) != 0) && ((DirPoss & PossDir[d]) != 0)) ||
           (((TypeVoie & tvVoieTram) != 0) && EstDirPossSensVoieTram(indefinie, d));
}
//-----------------------------------------------------------------------------
void voie::DefinitNature(nature NvNature, bool Ecraser) // v3.8
{
    int d = 0;
    if ((!Ecraser) && (Nature > voies))
    {
        return;
    }
    // portage : borne testée d'abord. L'original lisait PossDir[5] (hors du tableau) avant
    // de constater d>ouest ; ce comportement indéfini permet à l'optimiseur de supposer
    // d<=ouest à la sortie de la boucle : en Release, la nature n'était jamais définie
    for (d = nord; (d <= ouest) &&
                   (((DirPoss & PossDir[d]) == 0) && ((v(static_cast<direction>(d))->DirPoss & InvPossDir[d]) == 0));
         d++)
    {
        ;
    }
    if ((d > ouest) && (SensVoieTram == 0))
    {
        if ((Nature != trottoirs) || (NvNature == trottoirs) || !FeuxPietons())
        { // v5.2 : on empêche la suppression des trottoirs libres (sans décors) si des feux de piétons existent
            Nature = NvNature;
        }
        RedessineCase(true);
        vue->DefinitSimulationModifiee(true); // portage
    }
}
//-----------------------------------------------------------------------------
void voie::TuePietons() // v4.3
{
    int NbPietonsEcrases = 0;
    for (const int i : NumPieton)
    {
        if (i != 0)
        {
            NbPietonsEcrases++;
            cv->Pieton[i - 1].SeFaitEcraser();
        }
    }
    if (NbPietonsEcrases != 0)
    {
        RedessineCase(false);
    }
}
//-----------------------------------------------------------------------------
bool voie::FeuPietonsPossible() const // v5.2
{
    return (Nature == trottoirs) &&               // Trottoirs sans décors  : les piétons peuvent passer
           (DirPoss == 0) && (SensVoieTram == 0); // ET Pas de voies de circulation
}
//-----------------------------------------------------------------------------
void voie::RedessineCase(bool EffaceFond) // v3.0
{
    vue->RedessineCase(x, y, EffaceFond); // portage
}
//-----------------------------------------------------------------------------
bool voie::Charge(int hfile)
{
    int i = 0;
    bool Ok = false;
    Ok = LitChamp(hfile, &x, sizeof(x)) && LitChamp(hfile, &y, sizeof(y)) &&
         LitChamp(hfile, &DirPoss, sizeof(DirPoss)) &&
         ((VersionFichier < 0x350) || // v3.5
          LitChamp(hfile, &SensVoieTram, sizeof(SensVoieTram))) &&
         ((VersionFichier < 0x380) || // v3.8
          LitChamp(hfile, &Nature, sizeof(Nature))) &&
         LitChamp(hfile, &NumParking, sizeof(NumParking)) && LitChamp(hfile, &ToutDroit, sizeof(ToutDroit)) &&
         LitChamp(hfile, &PassagePietons, sizeof(PassagePietons)) && // v2.0
         ((VersionFichier < 0x300) ||                                // v3.0
          (LitChamp(hfile, &PassageBus, sizeof(PassageBus)) && LitChamp(hfile, &NumArretBus, sizeof(NumArretBus)))) &&
         ((VersionFichier < 0x350) || // v3.5
          LitChamp(hfile, &NumArretTram, sizeof(NumArretTram))) &&
         ((VersionFichier < 0x360) || // v3.6
          LitChamp(hfile, &NumPlaceTaxi, sizeof(NumPlaceTaxi))) &&
         ((VersionFichier < 0x500) || // v5.0
          LitChamp(hfile, &NumPlacePark, sizeof(NumPlacePark))) &&
         ((VersionFichier < 0x530) || // v5.3
          (LitChamp(hfile, &NumPlaceVehlib, sizeof(NumPlaceVehlib)) &&
           LitChamp(hfile, &NumSpotVehlib, sizeof(NumSpotVehlib))));
    for (i = 0; Ok && (i < NBFEUXPARCASE); i++)
    { // v5.2: NBFEUXPARCASE
        Ok = LitChamp(hfile, &Priorite[i], sizeof(Priorite[i])) && LitChamp(hfile, &NumFeu[i], sizeof(NumFeu[i]));
    }
    if (Ok)
    { // v5.1.1 : élimination automatique des feux fantômes !
        for (i = 0; i < NBFEUXPARCASE; i++)
        {
            if ((NumFeu[i] != 0) && ((cv->Feu[NumFeu[i] - 1].x != x) || (cv->Feu[NumFeu[i] - 1].y != y)))
            {
                NumFeu[i] = 0; // Feu fantôme effacé !
            }
        }
    }
    if (VersionFichier >= 0x520)
    { // v5.2 : feux piétons
        for (i = 0; Ok && (i < NBFEUXPARCASE); i++)
        {
            Ok = LitChamp(hfile, &NumFeuP[i], sizeof(NumFeuP[i]));
        }
    }
    return Ok;
}
//-----------------------------------------------------------------------------
bool voie::Sauve(int hfile)
{
    int i = 0;
    int xc = x - cv->Xc;
    int yc = y - cv->Yc; // v4.0.1
    bool Ok = false;
    Ok = (FileWrite(hfile, cv->CopieEnCours ? &xc : &x, sizeof(x)) != 0) &&
         (FileWrite(hfile, cv->CopieEnCours ? &yc : &y, sizeof(y)) != 0) &&
         (FileWrite(hfile, &DirPoss, sizeof(DirPoss)) != 0) &&
         (FileWrite(hfile, &SensVoieTram, sizeof(SensVoieTram)) != 0) &&
         (FileWrite(hfile, &Nature, sizeof(Nature)) != 0) && (FileWrite(hfile, &NumParking, sizeof(NumParking)) != 0) &&
         (FileWrite(hfile, &ToutDroit, sizeof(ToutDroit)) != 0) &&
         (FileWrite(hfile, &PassagePietons, sizeof(PassagePietons)) != 0) && // v2.0. v6.0.1 : « ; » à la place de « &&
                                                                             // » dans l'original (fin de la chaîne
                                                                             // ignorée)
         (FileWrite(hfile, &PassageBus, sizeof(PassageBus)) != 0) &&         // v3.0
         (FileWrite(hfile, &NumArretBus, sizeof(NumArretBus)) != 0) &&       // v3.0
         (FileWrite(hfile, &NumArretTram, sizeof(NumArretTram)) != 0) &&     // v3.5
         (FileWrite(hfile, &NumPlaceTaxi, sizeof(NumPlaceTaxi)) != 0) &&     // v3.6
         (FileWrite(hfile, &NumPlacePark, sizeof(NumPlacePark)) != 0) &&     // v5.0
         (FileWrite(hfile, &NumPlaceVehlib, sizeof(NumPlaceVehlib)) != 0) && // v5.3
         (FileWrite(hfile, &NumSpotVehlib, sizeof(NumSpotVehlib)) != 0);     // v5.3
    for (i = 0; Ok && (i < NBFEUXPARCASE); i++)
    {
        Ok = (FileWrite(hfile, &Priorite[i], sizeof(Priorite[i])) != 0) &&
             (FileWrite(hfile, &NumFeu[i], sizeof(NumFeu[i])) != 0);
    }
    for (i = 0; Ok && (i < NBFEUXPARCASE); i++)
    {
        Ok = (FileWrite(hfile, &NumFeuP[i], sizeof(NumFeuP[i])) != 0);
    }
    return Ok;
}
//-----------------------------------------------------------------------------
void voie::Verifie(AnsiString &asMsgErr)
{
    int i = 0;
    int xx = 0;
    int yy = 0;
    const AnsiString asSource = Format("Case (%d,%d)", TableauDeConstantes(x, y));
    VerifiePosition(asMsgErr, asSource, x, y, DOIT_ETRE_DEFINI);
    VerifieDirPoss(asMsgErr, asSource, DirPoss);
    for (i = 0; i < NBDIR; i++)
    {
        VerifiePriorite(asMsgErr, asSource + " (" + asDir[i + 1] + ")", Priorite[i]);
        if ((NumFeu[i] != 0) && (Priorite[i] != feux))
        {
            asMsgErr += asSource + Format(": numéro de feu %s mais priorité différente de Feu (%d)\n",
                                          TableauDeConstantes(asDir[i + 1], static_cast<int>(Priorite[i]))); // v5.4
                                                                                                             // enum as
                                                                                                             // char
                                                                                                             // type
                                                                                                             // (cast
                                                                                                             // int)
        }
        if ((NumFeu[i] < 0) || (NumFeu[i] > cv->NbFeux))
        {
            asMsgErr +=
                asSource + Format(": numéro de feu %s incorrect (%d)\n", TableauDeConstantes(asDir[i + 1], NumFeu[i]));
        }
        else if (NumFeu[i] != 0)
        {
            cv->Feu[NumFeu[i] - 1].Verifie(asMsgErr, asSource + Format(" Feu %s", TableauDeConstantes(asDir[i + 1])));
            xx = cv->Feu[NumFeu[i] - 1].x;
            yy = cv->Feu[NumFeu[i] - 1].y;
            if ((x != xx) || (y != yy))
            {
                asMsgErr +=
                    asSource + Format(": coordonnées du feu %s (%d,%d) différentes des coordonnées de la case\n",
                                      TableauDeConstantes(asDir[i + 1], xx, yy));
            }
        }
    }
    // Complément des contrôles (v3.6)
    if ((NumParking < 0) || (NumParking > cv->NbParkings))
    {
        asMsgErr +=
            asSource + Format(": numéro de parking incorrect (%d)\n", TableauDeConstantes(NumParking)); // v5.0
                                                                                                        // (correction
                                                                                                        // message
                                                                                                        // erroné)
    }
    if ((NumArretBus < 0) || (NumArretBus > cv->NbArretsBus))
    {
        asMsgErr += asSource + Format(": numéro d'arrêt de bus incorrect (%d)\n", TableauDeConstantes(NumArretBus));
    }
    if ((NumArretTram < 0) || (NumArretTram > cv->NbArretsTram))
    {
        asMsgErr += asSource + Format(": numéro d'arrêt de tram incorrect (%d)\n", TableauDeConstantes(NumArretTram));
    }
    if ((NumPlaceTaxi < 0) || (NumPlaceTaxi > cv->NbPlacesTaxi))
    {
        asMsgErr += asSource + Format(": numéro de place de taxi incorrect (%d)\n", TableauDeConstantes(NumPlaceTaxi));
    }
    if ((NumPlacePark < 0) || (NumPlacePark > cv->NbPlacesPark))
    { // v5.0
        asMsgErr +=
            asSource + Format(": numéro de place de parking incorrect (%d)\n", TableauDeConstantes(NumPlacePark));
    }
}
//-----------------------------------------------------------------------------
void stats::Definit()
{
    NbVehiculesDeplaces = cv->NbVehiculesDeplaces;
    NbVehiculesArrives = cv->NbVehiculesArrives;
    NbVehiculesSortis = cv->NbVehiculesSortis;
    // v5.4.1 : stats épidémiques des piétons
    NbPietonsGueris = cv->NbPietonsGueris;
    NbPietonsInfectes = cv->NbPietonsInfectes;
    NbPietonsMorts = cv->NbPietonsMorts;
}
//-----------------------------------------------------------------------------
void centre_ville::CreeZone() // v4.0 (allocation dynamique)
{
    LibereDistances(); // 6.0.19 : celles de l'ancienne grille (jusqu'à la 6.0.18, détruites avec ses cases)
    if ((NbX > 0) && (NbY > 0))
    {
        v.Cree(NbX, NbY);
        for (int i = 0; i < NbX; i++)
        {
            for (int j = 0; j < NbY; j++)
            {
                v[i][j].Definit(i, j);
            }
        }
    }
    else
    {
        v.clear();
    }
    RetablitVoisinages();
}
//-----------------------------------------------------------------------------
void centre_ville::RetablitVoisinages() const // v6.0.2
{
    dx[static_cast<int>(ouest)] = NbX - 1;
    dy[static_cast<int>(nord)] = NbY - 1;
}
//-----------------------------------------------------------------------------
centre_ville::centre_ville(bool NbElementsParDefaut, int NbX, int NbY) // v4.0 (NbX, NbY)
    : NbX(NbX), NbY(NbY)
{ // Les autres membres ont leur valeur par défaut dans la déclaration (simul.h)
    CreeZone();
    for (int i = 0; i < NBMAXLIGNESBUSTRAM; i++)
    {
        LigneBus[i].AffecteNumero(i + 1);  // v3.0
        LigneTram[i].AffecteNumero(i + 1); // v3.5
    }
    // Tableaux avec nombre par défaut
    if (NbElementsParDefaut)
    {
        DefinitNbFeux(NBFEUXDEFAUT);
        DefinitNbFeuxPietons(NBFEUXPIETONDEFAUT); // v5.2
        DefinitNbParkings(NBETAPESDEFAUT);
        DefinitNbVehicules(NBVEHICULESDEFAUT);
        DefinitNbPietons(NBPIETONSDEFAUT); // v2.0
    }
}
//-----------------------------------------------------------------------------
centre_ville::~centre_ville()
{
    v.clear();
    if (!Feu.empty())
    {
        Feu.clear();
    }
    if (!FeuP.empty())
    {
        FeuP.clear();
    } // v6.0.2 : oubli
    if (!Parking.empty())
    {
        Parking.clear();
    }
    if (!Vehicule.empty())
    {
        Vehicule.clear();
    }
    if (!Vehlib.empty())
    {
        Vehlib.clear();
    } // v5.3
    if (!Bus.empty())
    {
        Bus.clear();
    } // v6.0.2 : oubli
    if (!Tram.empty())
    {
        Tram.clear();
    } // v6.0.2 : oubli
    if (!Pieton.empty())
    {
        Pieton.clear();
    }
    if (!ArretBus.empty())
    {
        ArretBus.clear();
    } // v3.6
    if (!ArretTram.empty())
    {
        ArretTram.clear();
    } // v3.6
    if (!Taxi.empty())
    {
        Taxi.clear();
    } // v3.6
    if (!PlaceTaxi.empty())
    {
        PlaceTaxi.clear();
    } // v3.6
    if (!FileTaxi.empty())
    {
        FileTaxi.clear();
    } // v3.6
    if (!PlacePark.empty())
    {
        PlacePark.clear();
    } // v5.0
    if (!FilePark.empty())
    {
        FilePark.clear();
    } // v5.0
    if (!PlaceVehlib.empty())
    {
        PlaceVehlib.clear();
    } // v5.3
    if (!Stats.empty())
    {
        Stats.clear();
    }
}
//-----------------------------------------------------------------------------
int centre_ville::DefinitNbFeux(int n)
{
    int i = 0;
    if (NbFeux != 0)
    {
        Feu.clear();
    }
    NbFeux = 0;
    if (n != 0)
    {
        Feu = std::vector<feu>(n);
        for (i = 0; i < n; i++)
        {
            Feu[i].AffecteNumero(i + 1);
        }
        NbFeux = n;
    }
    return NbFeux;
}
//-----------------------------------------------------------------------------
int centre_ville::DefinitNbFeuxPietons(int n) // v5.2
{
    int i = 0;
    if (NbFeuxPieton != 0)
    {
        FeuP.clear();
    }
    NbFeuxPieton = 0;
    if (n != 0)
    {
        FeuP = std::vector<feu_pieton>(n);
        for (i = 0; i < n; i++)
        {
            FeuP[i].AffecteNumero(i + 1);
        }
        NbFeuxPieton = n;
    }
    return NbFeuxPieton;
}
//-----------------------------------------------------------------------------
int centre_ville::DefinitNbParkings(int n)
{
    int i = 0;
    if (NbParkings != 0)
    {
        Parking.clear();
    }
    NbParkings = 0;
    Parking = std::vector<parking>(n);
    {
        for (i = 0; i < n; i++)
        {
            Parking[i].AffecteNumero(i + 1);
        }
        NbParkings = n;
    }
    return NbParkings;
}
//-----------------------------------------------------------------------------
int centre_ville::DefinitNbVehicules(int n)
{
    int i = 0;
    if (NbVehicules != 0)
    {
        Vehicule.clear();
    }
    NbVehicules = 0;
    Vehicule = std::vector<vehicule>(n);
    {
        for (i = 0; i < n; i++)
        {
            Vehicule[i].AffecteNumero(i + 1);
        }
        NbVehicules = n;
    }
    return NbVehicules;
}
//-----------------------------------------------------------------------------
int centre_ville::DefinitNbVehlibs(int n) // v5.3
{
    int i = 0;
    if (NbVehlibs != 0)
    {
        Vehlib.clear();
    }
    NbVehlibs = 0;
    Vehlib = std::vector<vehlib>(n);
    {
        for (i = 0; i < n; i++)
        {
            Vehlib[i].AffecteNumero(i + 1);
        }
        NbVehlibs = n;
    }
    return NbVehlibs;
}
//-----------------------------------------------------------------------------
int centre_ville::DefinitNbBus(int n) // v3.0
{
    int i = 0;
    if (NbBus != 0)
    {
        Bus.clear();
    }
    NbBus = 0;
    if (n != 0)
    {
        Bus = std::vector<bus>(n);
        for (i = 0; i < n; i++)
        {
            Bus[i].AffecteNumero(i + 1);
        }
        NbBus = n;
    }
    return NbBus;
}
//-----------------------------------------------------------------------------
int centre_ville::DefinitNbTrams(int n) // v3.5
{
    int i = 0;
    if (NbTrams != 0)
    {
        Tram.clear();
    }
    NbTrams = 0;
    if (n != 0)
    {
        Tram = std::vector<tram>(n);
        for (i = 0; i < n; i++)
        {
            Tram[i].AffecteNumero(i + 1);
        }
        NbTrams = n;
    }
    return NbTrams;
}
//-----------------------------------------------------------------------------
int centre_ville::DefinitNbPietons(int n)
{
    int i = 0;
    if (NbPietons != 0)
    {
        Pieton.clear();
    }
    NbPietons = 0;
    if (n != 0)
    {
        Pieton = std::vector<pieton>(n);
        for (i = 0; i < n; i++)
        {
            Pieton[i].AffecteNumero(i + 1);
            if (vue->EpidemieActivee() &&              // Si épidémie activée ET // portage
                (EpidemieIterationPatientZero != 0) && // Si patients zéro à définir ALORS
                (((i + 1) % EpidemieIterationPatientZero) == 0))
            { // On définit les patients zéro
                Pieton[i].Infection = new infection(this);
            }
        }
        NbPietons = n;
    }
    return NbPietons;
}
//-----------------------------------------------------------------------------
int centre_ville::DefinitNbFilesTaxi(int n) // v3.0
{
    int i = 0;
    if (NbFilesTaxi != 0)
    {
        FileTaxi.clear();
    }
    NbFilesTaxi = 0;
    if (n != 0)
    {
        FileTaxi = std::vector<file_taxi>(n);
        for (i = 0; i < n; i++)
        {
            FileTaxi[i].AffecteNumero(i + 1);
        }
        NbFilesTaxi = n;
    }
    return NbFilesTaxi;
}
//-----------------------------------------------------------------------------
int centre_ville::DefinitNbTaxis(int n) // v3.0
{
    int i = 0;
    if (NbTaxis != 0)
    {
        Taxi.clear();
    }
    NbTaxis = 0;
    if (n != 0)
    {
        Taxi = std::vector<taxi>(n);
        for (i = 0; i < n; i++)
        {
            Taxi[i].AffecteNumero(i + 1);
        }
        NbTaxis = n;
    }
    return NbTaxis;
}
//-----------------------------------------------------------------------------
int centre_ville::DefinitFilesTaxi() // v3.6
{
    int NumFile = 0;
    int NbFilesTrv = 0;
    if (!FileTaxi.empty())
    {
        FileTaxi.clear();
        NbFilesTaxi = 0;
    }
    if ((NbPlacesTaxi == 0) || PlaceTaxi.empty())
    {
        return 0;
    }
    for (int i = 0; i < NbPlacesTaxi; i++)
    {
        if (PlaceTaxi[i].Definie())
        {
            NbFilesTrv += static_cast<int>(PlaceTaxi[i].EstQueueFile());
        }
    }
    if (NbFilesTrv != 0)
    {
        if (DefinitNbFilesTaxi(NbFilesTrv) != NbFilesTrv)
        {
            return 0;
        }
        // D'abord on définit la file pour les queues de file
        for (int i = 0; i < NbPlacesTaxi; i++)
        {
            if (PlaceTaxi[i].Definie() && PlaceTaxi[i].EstQueueFile())
            {
                assert(NumFile < NbFilesTaxi); // autant de files que de queues comptées plus haut
                FileTaxi[NumFile].Definit(i);
                PlaceTaxi[i].DefinitFile(NumFile);
                NumFile++;
            }
        }
        // Ensuite pour toutes les autres places
        for (int i = 0; i < NbPlacesTaxi; i++)
        {
            if (PlaceTaxi[i].Definie())
            {
                PlaceTaxi[i].DefinitFile(PlaceTaxi[PlaceTaxi[i].PlaceQueueFile() - 1].NumFile);
            }
        }
    }
    return NbFilesTrv;
}
//-----------------------------------------------------------------------------
int centre_ville::DefinitNbFilesPark(int n) // v5.0
{
    int i = 0;
    if (NbFilesPark != 0)
    {
        FilePark.clear();
    }
    NbFilesPark = 0;
    if (n != 0)
    {
        FilePark = std::vector<file_park>(n);
        for (i = 0; i < n; i++)
        {
            FilePark[i].AffecteNumero(i + 1);
        }
        NbFilesPark = n;
    }
    return NbFilesPark;
}
//-----------------------------------------------------------------------------
int centre_ville::DefinitFilesPark() // v5.0
{
    int NumFilePark = 0;
    int NbFilesTrv = 0;
    if (!FilePark.empty())
    {
        FilePark.clear();
        NbFilesPark = 0;
    }
    if ((NbPlacesPark == 0) || PlacePark.empty())
    {
        return 0;
    }
    for (int i = 0; i < NbPlacesPark; i++)
    {
        if (PlacePark[i].Definie())
        {
            NbFilesTrv += static_cast<int>(PlacePark[i].EstQueueFile());
        }
    }
    if (NbFilesTrv != 0)
    {
        if (DefinitNbFilesPark(NbFilesTrv) != NbFilesTrv)
        {
            return 0;
        }
        // D'abord on définit la file pour les queues de file
        for (int i = 0; i < NbPlacesPark; i++)
        {
            if (PlacePark[i].Definie() && PlacePark[i].EstQueueFile())
            {
                assert(NumFilePark < NbFilesPark); // autant de files que de queues comptées plus haut
                FilePark[NumFilePark].Definit(i);
                PlacePark[i].DefinitFile(NumFilePark);
                NumFilePark++;
            }
        }
        // Ensuite pour toutes les autres places
        for (int i = 0; i < NbPlacesPark; i++)
        {
            if (PlacePark[i].Definie())
            {
                PlacePark[i].DefinitFile(PlacePark[PlacePark[i].PlaceQueueFile() - 1].NumFile);
            }
        }
    }
    return NbFilesTrv;
}
//-----------------------------------------------------------------------------
int centre_ville::NbFeuxLibres() const
{
    int i = 0;
    int n = 0;
    for (i = 0; i < NbFeux; i++)
    {
        n += static_cast<int>(!Feu[i].Defini());
    }
    return n;
}
//-----------------------------------------------------------------------------
int centre_ville::NumeroFeuLibre() const
{
    int i = 0;
    for (i = 0; (i < NbFeux) && Feu[i].Defini(); i++)
    {
        ;
    }
    return (i + 1) * static_cast<int>(i < NbFeux);
}
//-----------------------------------------------------------------------------
int centre_ville::NumeroFeuPietonsLibre() const // v5.2
{
    int i = 0;
    for (i = 0; (i < NbFeuxPieton) && FeuP[i].Defini(); i++)
    {
        ;
    }
    return (i + 1) * static_cast<int>(i < NbFeuxPieton);
}
//-----------------------------------------------------------------------------
int centre_ville::NumeroParkingLibre() const
{
    int i = 0;
    for (i = 0; (i < NbParkings) && Parking[i].Defini(); i++)
    {
        ;
    }
    return (i + 1) * static_cast<int>(i < NbParkings);
}
//-----------------------------------------------------------------------------
int centre_ville::ArretBusExistant(int x, int y) // v3.0
{
    int d = 0;
    int i = 0;
    voie *Voie = nullptr;
    for (d = nord; d <= ouest; d++)
    {
        Voie = &Voisine(x, y, d);
        if ((Voie->NumArretBus != 0) && (((v[x][y].DirPoss & PossDir[d]) != 0) || // on peut aller sur la case Voie OU
                                         ((Voie->DirPoss & InvPossDir[d]) != 0))) // on peut venir de la case Voie
        {
            if (i == 0)
            {
                i = Voie->NumArretBus;
            }
            if ((Voie->NumArretBus == i) &&              // Si case contiguë est arrêt de bus
                (Voie->v(AGaucheDe[d])->DirPoss == 0) && // ET qu'il y a un trottoir à sa gauche (en regardant vers
                                                         // l'arrière de l'arrêt)
                ((Voie->DirPoss & InvPossDir[d]) != 0))
            {                                  // Si d est dans la bonne direction (sens bus)
                ArretBus[i - 1].Definit(x, y); // (x,y) devient la tête d'arrêt de bus Voie->NumArretBus
            }
        }
    }
    return i;
}
//-----------------------------------------------------------------------------
int centre_ville::ArretTramExistant(int x, int y) const // v3.5
{ // Pour les trams on ne change pas la tête de l'arrêt car il peut y avoir un double sens sur la voie
    int d = 0;
    int i = 0;
    const voie *Voie = nullptr;
    for (d = nord; (i == 0) && (d <= ouest); d++)
    {
        Voie = &Voisine(x, y, d);
        if ((Voie->NumArretTram != 0) &&                       // Un arrêt contigu existe ET
            (v[x][y].SensVoieTram == SensVoieTramDir[d][d]) && // Même sens de la voie
            (Voie->SensVoieTram == SensVoieTramDir[d][d]))
        { // Dans le prolongement
            i = Voie->NumArretTram;
        }
    }
    return i;
}
//-----------------------------------------------------------------------------
int centre_ville::NumeroArretBusLibre(int x, int y) // v3.0
{
    int i = 0;
    i = ArretBusExistant(x, y);
    if (i != 0)
    {
        return i; // L'arrêt existe déjà (case contiguë qui communique : va ou vient)
    }
    // C'est un nouvel arrêt de bus
    for (i = 0; (i < NbArretsBus) && ArretBus[i].Defini(); i++)
    {
        ;
    }
    return (i + 1) * static_cast<int>(i < NbArretsBus);
}
//-----------------------------------------------------------------------------
int centre_ville::NumeroArretTramLibre(int x, int y) // v3.5
{
    int i = 0;
    i = ArretTramExistant(x, y);
    if (i != 0)
    {
        return i; // L'arrêt existe déjà (case contiguë qui communique : va ou vient)
    }
    // C'est un nouvel arrêt de tram
    for (i = 0; (i < NbArretsTram) && ArretTram[i].Defini(); i++)
    {
        ;
    }
    return (i + 1) * static_cast<int>(i < NbArretsTram);
}
//-----------------------------------------------------------------------------
int centre_ville::NumeroPlaceTaxiLibre() const // v3.6
{
    int i = 0;
    for (i = 0; (i < NbPlacesTaxi) && PlaceTaxi[i].Definie(); i++)
    {
        ;
    }
    return (i + 1) * static_cast<int>(i < NbPlacesTaxi);
}
//-----------------------------------------------------------------------------
int centre_ville::NumeroPlaceParkLibre() const // v5.0. v5.3 renommée de NumeroPlaceLibre pour ne pas confondre avec
                                               // NumeroPlaceVehlibLibre()
{
    int i = 0;
    for (i = 0; (i < NbPlacesPark) && PlacePark[i].Definie(); i++)
    {
        ;
    }
    return (i + 1) * static_cast<int>(i < NbPlacesPark);
}
//-----------------------------------------------------------------------------
int centre_ville::NumeroPlaceVehlibLibre() const // v5.3
{
    int i = 0;
    for (i = 0; (i < NbPlacesVehlib) && PlaceVehlib[i].Definie(); i++)
    {
        ;
    }
    return (i + 1) * static_cast<int>(i < NbPlacesVehlib);
}
//-----------------------------------------------------------------------------
int centre_ville::AjouteFeu(int x, int y, direction d)
{
    int i = NumeroFeuLibre();
    if (i == 0)
    { // v5.1
        i = IncrementeNombre(Feu, NbFeux);
    }
    if (i != 0)
    {
        Feu[i - 1].Definit(x, y, d);
        v[x][y].NumFeu[d - 1] = i;
        // Définition des durées par défaut
        Feu[i - 1].DureeVert = DureeVert;
        Feu[i - 1].DureeRouge = DureeRouge;
    }
    return i;
}
//-----------------------------------------------------------------------------
int centre_ville::AjouteFeuPietons(int x, int y, direction d) // v5.2
{                                                             // Inclut l'ajout du feu opposé de même numéro
    int i = NumeroFeuPietonsLibre();
    if (i == 0)
    { // v5.1
        i = IncrementeNombre(FeuP, NbFeuxPieton);
    }
    if (i != 0)
    {
        FeuP[i - 1].Definit(x, y, d);
        v[x][y].NumFeuP[d - 1] = i;
        // Définition des durées par défaut
        FeuP[i - 1].DureeVert = DureeVert;
        FeuP[i - 1].DureeRouge = DureeRouge;
        FeuP[i - 1].DefinitFeuOppose(); // Recherche et affecte les coordonnées et NumFeuP de la case opposée
    }
    return i;
}
//-----------------------------------------------------------------------------
void centre_ville::SupprimeFeu(int n)
{
    if ((n > -1) && (n < NbFeux))
    {
        v[Feu[n].x][Feu[n].y].NumFeu[Feu[n].Dir - 1] = 0;
        Feu[n].Initialise();
    }
}
//-----------------------------------------------------------------------------
void centre_ville::SupprimeFeuPietons(int n) // v5.2
{
    if ((n > -1) && (n < NbFeuxPieton))
    {
        for (int i = 0; i < NBFEUXPIETONSPARNUMERO; i++)
        {
            v[FeuP[n].x[i]][FeuP[n].y[i]].NumFeuP[FeuP[n].Dir[i] - 1] = 0;
        }
        FeuP[n].Initialise();
    }
}
//-----------------------------------------------------------------------------
void centre_ville::SupprimeFeuxVoie(int x, int y, type_voie TypeVoie) // v3.5 (TypeVoie)
{
    voie *Voie = &v[x][y];
    int i = 0;
    for (i = 0; i < NBDIR; i++)
    {
        if (Voie->ExisteVoieDansDirection(TypeVoie, (direction(i + 1))) && // v3.5
            (Voie->NumFeu[i] != 0))
        {
            SupprimeFeu(Voie->NumFeu[i] - 1);
        }
    }
}
//-----------------------------------------------------------------------------
void centre_ville::SupprimeParkingVoie(int x, int y) // v3.6
{
    voie *Voie = &v[x][y];
    if (Voie->NumParking != 0)
    {
        cv->SupprimeParking(Voie->NumParking - 1);
    }
}
//-----------------------------------------------------------------------------
void centre_ville::SupprimeArretBusVoie(int x, int y) // v3.6
{
    voie *Voie = &v[x][y];
    if (Voie->NumArretBus != 0)
    {
        Voie->RetireArretBus();
    }
}
//-----------------------------------------------------------------------------
void centre_ville::SupprimeDepotBusVoie(int x, int y) // v3.6
{
    voie *Voie = &v[x][y];
    if (Voie->EstDepotBus())
    {
        DepotBus.Initialise();
    }
}
//-----------------------------------------------------------------------------
void centre_ville::SupprimeArretTramVoie(int x, int y) // v3.6
{
    voie *Voie = &v[x][y];
    if (Voie->NumArretTram != 0)
    {
        Voie->RetireArretTram();
    }
}
//-----------------------------------------------------------------------------
void centre_ville::SupprimePlaceTaxiVoie(int x, int y) // v3.6
{
    voie *Voie = &v[x][y];
    if (Voie->NumPlaceTaxi != 0)
    {
        Voie->RetirePlaceTaxi();
    }
}
//-----------------------------------------------------------------------------
void centre_ville::SupprimePlaceParkVoie(int x, int y) // v5.0. v5.3 : Renommée de SupprimePlaceVoie pour ne pas
                                                       // confondre avec SupprimePlaceVehlibVoie(x, y)
{
    voie *Voie = &v[x][y];
    if (Voie->NumPlacePark != 0)
    {
        Voie->RetirePlacePark();
    }
}
//-----------------------------------------------------------------------------
void centre_ville::SupprimePlaceVehlibVoie(int x, int y) // v5.3
{
    voie *Voie = &v[x][y];
    if (Voie->NumPlaceVehlib != 0)
    {
        Voie->RetirePlaceVehlib();
    }
}
//-----------------------------------------------------------------------------

int centre_ville::AjouteParking(int x, int y)
{
    int i = NumeroParkingLibre();
    if (i == 0)
    { // v5.1
        i = IncrementeNombre(Parking, NbParkings);
    }
    if (i != 0)
    {
        Parking[i - 1].Definit(x, y);
        v[x][y].NumParking = i;
    }
    return i;
}
//-----------------------------------------------------------------------------
void centre_ville::SupprimeParking(int n)
{
    if ((n > -1) && (n < NbParkings))
    {
        v[Parking[n].x][Parking[n].y].NumParking = 0;
        Parking[n].Initialise();
    }
}
//-----------------------------------------------------------------------------
int centre_ville::AjouteArretBus(int x, int y) // v3.0
{
    int i = NumeroArretBusLibre(x, y);
    if (i == 0)
    { // v5.1
        i = IncrementeNombre(ArretBus, NbArretsBus);
    }
    if (i != 0)
    {
        if (!ArretBus[i - 1].Defini())
        {
            ArretBus[i - 1].Definit(x, y);
        }
        v[x][y].NumArretBus = i;
    }
    return i;
}
//-----------------------------------------------------------------------------
int centre_ville::AjouteArretTram(int x, int y) // v3.5
{
    int i = NumeroArretTramLibre(x, y);
    if (i == 0)
    { // v5.1
        i = IncrementeNombre(ArretTram, NbArretsTram);
    }
    if (i != 0)
    {
        if (!ArretTram[i - 1].Defini())
        {
            ArretTram[i - 1].Definit(x, y);
        }
        v[x][y].NumArretTram = i;
    }
    return i;
}
//-----------------------------------------------------------------------------
int centre_ville::AjoutePlaceTaxi(int x, int y) // v3.6
{
    int i = NumeroPlaceTaxiLibre();
    if (i == 0)
    { // v5.1
        i = IncrementeNombre(PlaceTaxi, NbPlacesTaxi);
    }
    if (i != 0)
    {
        if (!PlaceTaxi[i - 1].Definie())
        {
            PlaceTaxi[i - 1].Definit(x, y);
        }
        v[x][y].NumPlaceTaxi = i;
    }
    return i;
}
//-----------------------------------------------------------------------------
int centre_ville::AjoutePlacePark(int x, int y) // v5.0, v5.1 : ancien nom AjoutePlace
{
    int i = NumeroPlaceParkLibre(); // v5.3 renommée de NumeroPlaceLibre pour ne pas confondre avec
                                    // NumeroPlaceVehlibLibre()
    if (i == 0)
    { // v5.1
        i = IncrementeNombre(PlacePark, NbPlacesPark);
    }
    if (i != 0)
    {
        if (!PlacePark[i - 1].Definie())
        {
            PlacePark[i - 1].Definit(x, y);
        }
        v[x][y].NumPlacePark = i;
    }
    return i;
}
//-----------------------------------------------------------------------------
int centre_ville::AjoutePlaceVehlib(int x, int y) // v5.3
{
    int i = NumeroPlaceVehlibLibre();
    if (i == 0)
    { // v5.1
        i = IncrementeNombre(PlaceVehlib, NbPlacesVehlib);
    }
    if (i != 0)
    {
        if (!PlaceVehlib[i - 1].Definie())
        {
            PlaceVehlib[i - 1].Definit(x, y);
        }
        v[x][y].NumPlaceVehlib = i;
    }
    return i;
}
//-----------------------------------------------------------------------------
bool centre_ville::ExisteCaseArretBus(int x, int y) // v3.0
{
    bool Existe = false;
    int d = 0;
    const int i = v[x][y].NumArretBus;
    voie *Voie = nullptr;
    for (d = nord; d <= ouest; d++)
    {
        Voie = &Voisine(x, y, d);
        if (Voie->NumArretBus == i)
        { // On profite pour regarder si on change de tête d'arrêt de bus
            Existe = true;
            if (v[x][y].EstTeteArretBus())
            {                             // Si c'était une tête d'arrêt de bus,
                Voie->DefinitArretBus(i); // sa voisine prend la place. v3.0.3
            }
        }
    }
    return Existe;
}
//-----------------------------------------------------------------------------
bool centre_ville::ExisteCaseArretTram(int x, int y) // v3.5
{
    bool Existe = false;
    int d = 0;
    const int i = v[x][y].NumArretTram;
    voie *Voie = nullptr;
    for (d = nord; d <= ouest; d++)
    {
        Voie = &Voisine(x, y, d);
        if (Voie->NumArretTram == i)
        { // On profite pour regarder si on change de tête d'arrêt de tram
            Existe = true;
            if (v[x][y].EstTeteArretTram())
            {                              // Si c'était une tête d'arrêt de tram,
                Voie->DefinitArretTram(i); // sa voisine prend la place. v3.0.3
            }
        }
    }
    return Existe;
}
//-----------------------------------------------------------------------------
void centre_ville::SupprimeArretBus(int x, int y) // v3.0. Supprime sur une case seulement
{
    const int i = v[x][y].NumArretBus;
    if ((i > 0) && (i <= NbArretsBus))
    {
        if (!ExisteCaseArretBus(x, y))
        {
            ArretBus[i - 1].Initialise(); // Si l'arrêt n'a pas d'autre case
        }
        v[x][y].NumArretBus = 0;
    }
}
//-----------------------------------------------------------------------------
void centre_ville::SupprimeArretTram(int x, int y) // v3.5. Supprime sur une case seulement
{
    const int i = v[x][y].NumArretTram;
    if ((i > 0) && (i <= NbArretsTram))
    {
        if (!ExisteCaseArretTram(x, y))
        {
            ArretTram[i - 1].Initialise(); // Si l'arrêt n'a pas d'autre case
        }
        v[x][y].NumArretTram = 0;
    }
}
//-----------------------------------------------------------------------------
void centre_ville::SupprimeArretBus(int n) // v3.0. Supprime complètement l'arrêt (sur TOUTES les cases)
{
    v[ArretBus[n].x][ArretBus[n].y].RetireArretBus(); // Retire le n° d'arrêt de bus récursivement sur toutes les cases
    ArretBus[n].Initialise();
}
//-----------------------------------------------------------------------------
void centre_ville::SupprimeArretTram(int n) // v3.5. Supprime complètement l'arrêt (sur TOUTES les cases)
{
    v[ArretTram[n].x][ArretTram[n].y].RetireArretTram(); // Retire le n° d'arrêt de tram récursivement sur toutes les
                                                         // cases
    ArretTram[n].Initialise();
}
//-----------------------------------------------------------------------------
void centre_ville::SupprimePlaceTaxi(int n) // v3.6
{
    v[PlaceTaxi[n].x][PlaceTaxi[n].y].RetirePlaceTaxi();
    PlaceTaxi[n].Initialise();
}
//-----------------------------------------------------------------------------
void centre_ville::SupprimePlacePark(int n) // v5.0. v5.3 : renommée de SupprimePlace(n) pour distinguer avec
                                            // SupprimePlaceVehlib(n)
{
    v[PlacePark[n].x][PlacePark[n].y].RetirePlacePark();
    PlacePark[n].Initialise();
}
//-----------------------------------------------------------------------------
void centre_ville::SupprimePlaceVehlib(int n) // v5.3
{
    v[PlaceVehlib[n].x][PlaceVehlib[n].y].RetirePlaceVehlib();
    PlaceVehlib[n].Initialise();
}
//-----------------------------------------------------------------------------
sens centre_ville::SensPassagePietonsPossible(int x, int y) const
{ // v2.0
    // 1. Sens suivant DirPoss (Directions de l'intérieur vers l'extérieur)
    sens Sens = SensPsgPt[v[x][y].DirPoss];
    // 2. Test des cases voisines (Directions de l'extérieur vers l'intérieur)
    switch (Sens)
    {
    case vertical:
        if (((Voisine(x, y, nord).DirPoss & S) != 0) || ((Voisine(x, y, sud).DirPoss & N) != 0))
        {
            Sens = indefini;
        }
        break;
    case horizontal:
        if (((Voisine(x, y, est).DirPoss & O) != 0) || ((Voisine(x, y, ouest).DirPoss & E) != 0))
        {
            Sens = indefini;
        }
        break;
    default:
        Sens = indefini;
    }
    // v3.5 : S'il n'y a pas de route, on regarde les voies de tram...
    if (v[x][y].DirPoss == 0)
    {
        if (v[x][y].SensVoieTram == 0)
        {                    // Si pas de voie de tram alors
            Sens = indefini; // Pas de sens possible (case noire)
        }
        else
        { // Sinon deux cas de voies de tram permettent le passage piéton
            if (v[x][y].SensVoieTram == est_ouest)
            {
                Sens = vertical;
            }
            if (v[x][y].SensVoieTram == nord_sud)
            {
                Sens = horizontal;
            }
        }
    }
    else // Sinon, s'il y a une route,
        if (v[x][y].SensVoieTram != 0)
        { // il ne faut pas qu'il y ait une voie de tram
            Sens = indefini;
        }
    return Sens;
}
//-----------------------------------------------------------------------------
direction centre_ville::CherchePassagePietons(int x, int y, sens SensRecherche) // v2.0
{
    bool Trv = false;
    int i = 0;
    int d = 0;
    std::array<int, 2> Dist = {0, 0};
    const std::array<direction, 2> Dir = {static_cast<direction>(3 - SensRecherche),
                                          static_cast<direction>(5 - SensRecherche)};
    if ((NbMaxCasesDetectPsgPt == 0) || // NbMaxCasesDetectPsgPt et
        (SensRecherche == 0))
    {                     // SensRecherche
        return indefinie; //  doivent être définis (horizontal|vertical ; >0)
    }
    for (d = 0; d < 2; d++)
    {
        for (i = 1; (!(Trv = Voisine(x, y, i, 3 - SensRecherche + (2 * d)).PassagePietons) &&
                     (Voisine(x, y, i, 3 - SensRecherche + (2 * d)).DirPoss == 0) && (i <= NbMaxCasesDetectPsgPt));
             i++)
        {
            ;
        }
        if (Trv)
        {
            Dist[d] = i;
        }
    }
    switch (static_cast<int>(!(Dist[0] == 0)) + (2 * static_cast<int>(!(Dist[1] == 0))))
    {
    case 0:
        return indefinie;
    case 1:
        return Dir[0];
    case 2:
        return Dir[1];
    case 3:
        if (Dist[0] == Dist[1])
        {
            return Dir[alea::Rand() % 2];
        }
        return (Dist[0] < Dist[1] ? Dir[0] : Dir[1]);

    default:
        return indefinie;
    }
}
//-----------------------------------------------------------------------------
direction centre_ville::ChercheArretBus(int x, int y, sens SensRecherche) // v3.0
{
    bool Trv = false;
    int i = 0;
    int d = 0;
    std::array<int, 2> Dist = {0, 0};
    const std::array<direction, 2> Dir = {static_cast<direction>(3 - SensRecherche),
                                          static_cast<direction>(5 - SensRecherche)};
    if ((NbMaxCasesDetectArretBus == 0) || // NbMaxCasesDetectArretBus et
        (SensRecherche == 0))
    {                     // SensRecherche
        return indefinie; //  doivent être définis (horizontal|vertical ; >0)
    }
    for (d = 0; d < 2; d++)
    {
        for (i = 1; (!(Trv = (Voisine(x, y, i, 3 - SensRecherche + (2 * d)).NumArretBus != 0)) &&
                     (Voisine(x, y, i, 3 - SensRecherche + (2 * d)).DirPoss == 0) && (i <= NbMaxCasesDetectArretBus));
             i++)
        {
            ;
        }
        if (i <= NbMaxCasesDetectArretBus)
        { // Si toujours dans les distances on vérifie la dernière case (l'arrêt EST sur une route)
            Trv = Trv || (Voisine(x, y, i, 3 - SensRecherche + (2 * d)).NumArretBus != 0);
        }
        if (Trv)
        {
            Dist[d] = i;
        }
    }
    switch (static_cast<int>(!(Dist[0] == 0)) + (2 * static_cast<int>(!(Dist[1] == 0))))
    {
    case 0:
        return indefinie;
    case 1:
        return Dir[0];
    case 2:
        return Dir[1];
    case 3:
        if (Dist[0] == Dist[1])
        {
            return Dir[alea::Rand() % 2];
        }
        return (Dist[0] < Dist[1] ? Dir[0] : Dir[1]);

    default:
        return indefinie;
    }
}
//-----------------------------------------------------------------------------
direction centre_ville::ChercheArretTram(int x, int y, sens SensRecherche) // v3.0
{
    bool Trv = false;
    int i = 0;
    int d = 0;
    std::array<int, 2> Dist = {0, 0};
    const std::array<direction, 2> Dir = {static_cast<direction>(3 - SensRecherche),
                                          static_cast<direction>(5 - SensRecherche)};
    if ((NbMaxCasesDetectArretTram == 0) || // NbMaxCasesDetectArretTram et
        (SensRecherche == 0))
    {                     // SensRecherche
        return indefinie; //  doivent être définis (horizontal|vertical ; >0)
    }
    for (d = 0; d < 2; d++)
    {
        for (i = 1; (!(Trv = (Voisine(x, y, i, 3 - SensRecherche + (2 * d)).NumArretTram != 0)) &&
                     (Voisine(x, y, i, 3 - SensRecherche + (2 * d)).DirPoss == 0) && (i <= NbMaxCasesDetectArretTram));
             i++)
        {
            ;
        }
        if (i <= NbMaxCasesDetectArretTram)
        { // Si toujours dans les distances on vérifie la dernière case (l'arrêt EST sur une route)
            Trv = Trv || (Voisine(x, y, i, 3 - SensRecherche + (2 * d)).NumArretTram != 0);
        }
        if (Trv)
        {
            Dist[d] = i;
        }
    }
    switch (static_cast<int>(!(Dist[0] == 0)) + (2 * static_cast<int>(!(Dist[1] == 0))))
    {
    case 0:
        return indefinie;
    case 1:
        return Dir[0];
    case 2:
        return Dir[1];
    case 3:
        if (Dist[0] == Dist[1])
        {
            return Dir[alea::Rand() % 2];
        }
        return (Dist[0] < Dist[1] ? Dir[0] : Dir[1]);

    default:
        return indefinie;
    }
}
//-----------------------------------------------------------------------------
direction centre_ville::CherchePlaceTaxi(int x, int y, sens SensRecherche) // v3.6
{
    bool Trv = false;
    int i = 0;
    int d = 0;
    std::array<int, 2> Dist = {0, 0};
    const std::array<direction, 2> Dir = {static_cast<direction>(3 - SensRecherche),
                                          static_cast<direction>(5 - SensRecherche)};
    if ((NbMaxCasesDetectFileTaxi == 0) || // NbMaxCasesDetectFileTaxi et
        (SensRecherche == 0))
    {                     // SensRecherche
        return indefinie; //  doivent être définis (horizontal|vertical ; >0)
    }
    for (d = 0; d < 2; d++)
    {
        for (i = 1; (!(Trv = (Voisine(x, y, i, 3 - SensRecherche + (2 * d)).NumPlaceTaxi != 0)) &&
                     (Voisine(x, y, i, 3 - SensRecherche + (2 * d)).DirPoss == 0) && (i <= NbMaxCasesDetectFileTaxi));
             i++)
        {
            ;
        }
        if (i <= NbMaxCasesDetectFileTaxi)
        { // Si toujours dans les distances on vérifie la dernière case (le place de taxi EST sur une route)
            Trv = Trv || (Voisine(x, y, i, 3 - SensRecherche + (2 * d)).NumPlaceTaxi != 0);
        }
        if (Trv)
        {
            Dist[d] = i;
        }
    }
    switch (static_cast<int>(!(Dist[0] == 0)) + (2 * static_cast<int>(!(Dist[1] == 0))))
    {
    case 0:
        return indefinie;
    case 1:
        return Dir[0];
    case 2:
        return Dir[1];
    case 3:
        if (Dist[0] == Dist[1])
        {
            return Dir[alea::Rand() % 2];
        }
        return (Dist[0] < Dist[1] ? Dir[0] : Dir[1]);

    default:
        return indefinie;
    }
}
//-----------------------------------------------------------------------------
direction centre_ville::CherchePlaceVehlib(int x, int y, sens SensRecherche) // v5.3
{
    bool Trv = false;
    int i = 0;
    int d = 0;
    std::array<int, 2> Dist = {0, 0};
    const std::array<direction, 2> Dir = {static_cast<direction>(3 - SensRecherche),
                                          static_cast<direction>(5 - SensRecherche)};
    if ((NbMaxCasesDetectFileVehlib == 0) || // NbMaxCasesDetectFileVehlib et
        (SensRecherche == 0))
    {                     // SensRecherche
        return indefinie; //  doivent être définis (horizontal|vertical ; >0)
    }
    for (d = 0; d < 2; d++)
    {
        for (i = 1; (!(Trv = (Voisine(x, y, i, 3 - SensRecherche + (2 * d)).NumPlaceVehlib != 0)) &&
                     (Voisine(x, y, i, 3 - SensRecherche + (2 * d)).DirPoss == 0) && (i <= NbMaxCasesDetectFileVehlib));
             i++)
        {
            ;
        }
        if (i <= NbMaxCasesDetectFileVehlib)
        { // Si toujours dans les distances on vérifie la dernière case (le place de vehlib EST sur une route)
            Trv = Trv || (Voisine(x, y, i, 3 - SensRecherche + (2 * d)).NumPlaceVehlib != 0);
        }
        if (Trv)
        {
            Dist[d] = i;
        }
    }
    switch (static_cast<int>(!(Dist[0] == 0)) + (2 * static_cast<int>(!(Dist[1] == 0))))
    {
    case 0:
        return indefinie;
    case 1:
        return Dir[0];
    case 2:
        return Dir[1];
    case 3:
        if (Dist[0] == Dist[1])
        {
            return Dir[alea::Rand() % 2];
        }
        return (Dist[0] < Dist[1] ? Dir[0] : Dir[1]);

    default:
        return indefinie;
    }
}
//-----------------------------------------------------------------------------
void centre_ville::DefinitArretBus(int n, int x, int y) // v3.0
{
    if ((n > 0) && (n <= NbArretsBus))
    {
        ArretBus[n - 1].Definit(x, y);
    }
}
//-----------------------------------------------------------------------------
void centre_ville::DefinitArretTram(int n, int x, int y) // v3.5
{
    if ((n > 0) && (n <= NbArretsTram))
    {
        ArretTram[n - 1].Definit(x, y);
    }
}
//-----------------------------------------------------------------------------
direction centre_ville::DirectionArretBus(int x, int y) const // v3.0
{
    bool Ambiguite = false;
    int d = 0;
    direction Dir = indefinie;
    for (d = nord; d <= ouest; d++)
    {                                                         // On prend la dernière possibilité en cas de litige
        if (((v[x][y].DirPoss & PossDir[d]) == 0) &&          // Si pas de déplacement vers d ET
            (Voisine(x, y, d).DirPoss == 0) &&                // Mur vers d ET
            ((v[x][y].DirPoss & PossDir[AGaucheDe[d]]) != 0)) // Direction possible à gauche (sens de la circulation de
                                                              // la voie)
        {
            Ambiguite = !(Dir == 0);
            Dir = static_cast<direction>(d);
        }
    }
    return static_cast<direction>(static_cast<int>(!Ambiguite) * Dir);
}
//-----------------------------------------------------------------------------
position_arret_bus_tram centre_ville::PositionArretBus(int x, int y, direction d) const // v3.0
{
    switch (static_cast<int>(Voisine(x, y, AGaucheDe[d]).NumArretBus == v[x][y].NumArretBus) +
            (2 * static_cast<int>(Voisine(x, y, ADroiteDe[d]).NumArretBus == v[x][y].NumArretBus)))
    {
    case 1:
        return queue;
    case 2:
        return tete;
    case 3:
        return milieu;
    default:
        return tete;
    }
}
//-----------------------------------------------------------------------------
bool centre_ville::ArretsBusVoisinsDifferents(int x, int y) const // v3.0
{
    const voie *Voie = nullptr;
    bool DiffTrv = false;
    int d = 0;
    int n = 0;
    int i = 0;
    for (d = nord; (!DiffTrv) && (d <= ouest); d++)
    {
        Voie = &Voisine(x, y, d);
        n = Voie->NumArretBus;
        if ((n != 0) && (((v[x][y].DirPoss & PossDir[d]) != 0) || // on peut aller sur la case
                                                                  // Voie OU
                         ((Voie->DirPoss & InvPossDir[d]) != 0))) // on peut venir de la case Voie
        {
            if (i == 0)
            {
                i = n;
            }
            else
            {
                DiffTrv = (i != n);
            }
        }
    }
    return DiffTrv;
}
//-----------------------------------------------------------------------------
bool centre_ville::ArretsTramVoisinsDifferents(int x, int y) const // v3.5
{
    const voie *Voie = nullptr;
    bool DiffTrv = false;
    int d = 0;
    int n = 0;
    int i = 0;
    for (d = nord; (!DiffTrv) && (d <= ouest); d++)
    {
        Voie = &Voisine(x, y, d);
        n = Voie->NumArretTram;
        if ((n != 0) && (((v[x][y].DirPoss & PossDir[d]) != 0) || // on peut aller sur la case
                                                                  // Voie OU
                         ((Voie->DirPoss & InvPossDir[d]) != 0))) // on peut venir de la case
                                                                  // Voie
        {
            if (i == 0)
            {
                i = n;
            }
            else
            {
                DiffTrv = (i != n);
            }
        }
    }
    return DiffTrv;
}
//-----------------------------------------------------------------------------
direction centre_ville::DirArretBusVoisin(int x, int y) const // v3.0
{
    const voie *Voie = nullptr;
    int d = 0;
    int DirArretBusVoisin = indefinie;
    for (d = nord; (DirArretBusVoisin == 0) && (d <= ouest); d++)
    {
        Voie = &Voisine(x, y, d);
        if ((Voie->NumArretBus != 0) &&               // Arrêt de bus voisin ET
            (((v[x][y].DirPoss & PossDir[d]) != 0) || // on peut y aller ou
             ((Voie->DirPoss & InvPossDir[d]) != 0)))
        { // on peut en venir
            DirArretBusVoisin = d;
        }
    }
    return static_cast<direction>(DirArretBusVoisin);
}
//-----------------------------------------------------------------------------
direction centre_ville::DirArretTramVoisin(int x, int y) const // v3.5
{
    const voie *Voie = nullptr;
    int d = 0;
    int DirArretTramVoisin = indefinie;
    for (d = nord; (DirArretTramVoisin == 0) && (d <= ouest); d++)
    {
        Voie = &Voisine(x, y, d);
        if ((Voie->NumArretTram != 0) &&              // Arrêt de tram voisin ET
            (((v[x][y].DirPoss & PossDir[d]) != 0) || // on peut y aller ou
             ((Voie->DirPoss & InvPossDir[d]) != 0)))
        { // on peut en venir
            DirArretTramVoisin = d;
        }
    }
    return static_cast<direction>(DirArretTramVoisin);
}
//-----------------------------------------------------------------------------
bool centre_ville::AjoutArretBusPossible(int x, int y) // v3.0
{
    int d = 0;
    direction DirArretVoisin{};
    bool Possible = false;
    if ((v[x][y].DirPoss != 0) && (!ArretsBusVoisinsDifferents(x, y))) // C'est de la route ET il n'y a pas deux arrêts
                                                                       // différents voisins
    {
        DirArretVoisin = DirArretBusVoisin(x, y);
        for (d = nord; (!Possible) && (d <= ouest); d++)
        {
            if (((v[x][y].DirPoss & PossDir[d]) == 0) && // Si pas de direction possible vers d et dans cette direction
                                                         // il y a un mur ALORS
                (Voisine(x, y, d).DirPoss == 0))
            { // Soit 1 arrêt d'un côté ou de l'autre soit c'est le même n° (ou nul les 2)
                Possible = ((DirArretVoisin == 0) ||             // Si arrêt voisin il y a, ALORS
                            (DirArretVoisin == ADroiteDe[d]) ||  // il doit être dans le même
                            (DirArretVoisin == AGaucheDe[d])) && // sens que le futur arrêt
                           ((static_cast<int>(!(Voisine(x, y, AGaucheDe[d]).NumArretBus == 0)) +
                                 static_cast<int>(!(Voisine(x, y, ADroiteDe[d]).NumArretBus == 0)) ==
                             1) ||
                            (Voisine(x, y, AGaucheDe[d]).NumArretBus == Voisine(x, y, ADroiteDe[d]).NumArretBus));
            }
        }
        return Possible;
    }
    return false;
}
//-----------------------------------------------------------------------------
bool centre_ville::AjoutArretTramPossible(int x, int y) // v3.5
{
    int d = 0;
    direction DirArretVoisin{};
    bool Possible = false;
    if ((v[x][y].SensVoieTram != 0) && (!ArretsTramVoisinsDifferents(x, y))) // C'est de la route ET il n'y a pas deux
                                                                             // arrêts différents voisins
    {
        DirArretVoisin = DirArretTramVoisin(x, y);
        for (d = nord; (!Possible) && (d <= ouest); d++)
        {
            if (((v[x][y].DirPoss & PossDir[d]) == 0) && // Si pas de direction possible vers d et dans cette direction
                                                         // il y a un mur ALORS
                (Voisine(x, y, d).DirPoss == 0))
            { // Soit 1 arrêt d'un côté ou de l'autre soit c'est le même n° (ou nul les 2)
                Possible = ((DirArretVoisin == 0) ||             // Si arrêt voisin il y a, ALORS
                            (DirArretVoisin == ADroiteDe[d]) ||  // il doit être dans le même
                            (DirArretVoisin == AGaucheDe[d])) && // sens que le futur arrêt
                           ((static_cast<int>(!(Voisine(x, y, AGaucheDe[d]).NumArretTram == 0)) +
                                 static_cast<int>(!(Voisine(x, y, ADroiteDe[d]).NumArretTram == 0)) ==
                             1) ||
                            (Voisine(x, y, AGaucheDe[d]).NumArretTram == Voisine(x, y, ADroiteDe[d]).NumArretTram));
            }
        }
        return Possible;
    }
    return false;
}
//-----------------------------------------------------------------------------
bool centre_ville::SuppressionArretBusPossible(int x, int y) const // v3.0
{
    int d = 0;
    const int i = v[x][y].NumArretBus;
    int n = 0;
    for (d = nord; d <= ouest; d++)
    {
        n += static_cast<int>(Voisine(x, y, d).NumArretBus == i);
    }
    return (n < 2);
}
//-----------------------------------------------------------------------------
bool centre_ville::SuppressionArretTramPossible(int x, int y) const // v3.5
{
    int d = 0;
    const int i = v[x][y].NumArretTram;
    int n = 0;
    for (d = nord; d <= ouest; d++)
    {
        n += static_cast<int>(Voisine(x, y, d).NumArretTram == i);
    }
    return (n < 2);
}
//-----------------------------------------------------------------------------
void centre_ville::InitialiseInfosLignesBus() // v3.0
{
    int i = 0;
    NbLignesBus = 0;
    for (i = 0; i < NbArretsBus; i++)
    {
        ArretBus[i].InitialiseInfosLignesBus();
    }
}
//-----------------------------------------------------------------------------
void centre_ville::InitialiseInfosLignesTram() // v3.5
{
    int i = 0;
    NbLignesTram = 0;
    for (i = 0; i < NbArretsTram; i++)
    {
        ArretTram[i].InitialiseInfosLignesTram();
    }
}
//-----------------------------------------------------------------------------
void centre_ville::InitialiseInfosTrajetLignesBus()
{
    int i = 0;
    int j = 0;
    for (i = 0; i < NbX; i++)
    {
        for (j = 0; j < NbY; j++)
        {
            v[i][j].InitialiseInfosLignesBus();
        }
    }
}
//-----------------------------------------------------------------------------
void centre_ville::CalculeTrajetLignesBus() // v3.0.3
{ // InitialiseInfosTrajetLignesBus doit avoir été lancé avant pour mettre les voie::LigneBus[i] à false
    int i = 0;
    int j = 0;
    int dp = 0;
    int ndp = 0;
    int DistArretBus = 0;
    int DistCrt = 0;
    int NumArretSvt = 0;
    direction DirSvt{};
    voie *Voie = nullptr;
    for (i = 0; i < NBMAXLIGNESBUSTRAM; i++)
    {
        if (LigneBus[i].NbArrets != 0)
        {
            for (j = 0; j < LigneBus[i].NbArrets; j++)
            {
                Voie = &v[ArretBus[LigneBus[i].NumArret[j] - 1].x][ArretBus[LigneBus[i].NumArret[j] - 1].y];
                Voie->LigneBus[i] = true;
                NumArretSvt = LigneBus[i].NumArret[(j + 1) % LigneBus[i].NbArrets];
                do
                {
                    DistArretBus = 0x7FFFFFFF;
                    DirSvt = indefinie;
                    ndp = NbDir[Voie->DirPoss];
                    for (dp = 0; dp < ndp; dp++) // v3.5
                    {
                        DistCrt = Voie->DistArretBus()[NumArretSvt - 1][dp];
                        if (DistArretBus > DistCrt)
                        {
                            DistArretBus = DistCrt;
                            DirSvt = Dirdp[dp][Voie->DirPoss];
                        }
                    }
                    Voie = Voie->v(DirSvt);   // 1. On va dans la direction ayant la distance la plus petite vers le
                                              // prochain arrêt
                    Voie->LigneBus[i] = true; // 2. On marque la ligne sur cette case
                } while (!Voie->EstTeteArretBus(NumArretSvt));
            }
        }
    }
}
//-----------------------------------------------------------------------------
void centre_ville::CalculeTrajetLignesTram() // v3.5. Correction : v3.8
{ // Les voie::LigneTram[i] sont à false : les trajets ne sont calculés qu'au premier démarrage
    int i = 0;
    int j = 0;
    int dp = 0;
    int ndp = 0;
    int DistArretTram = 0;
    int DistCrt = 0;
    int NumArretSvt = 0;
    direction DirCrt{};
    direction DirPrc{};
    direction DirSvt{}; // v3.8
    voie *Voie = nullptr;
    for (i = 0; i < NBMAXLIGNESBUSTRAM; i++)
    {
        if (LigneTram[i].NbArrets != 0)
        {
            for (j = 0; j < LigneTram[i].NbArrets; j++)
            {
                DirPrc = indefinie; // v3.8
                Voie = &v[ArretTram[LigneTram[i].NumArret[j] - 1].x][ArretTram[LigneTram[i].NumArret[j] - 1].y];
                Voie->LigneTram[i] = true;
                NumArretSvt = LigneTram[i].NumArret[(j + 1) % LigneTram[i].NbArrets];
                do
                {
                    DistArretTram = 0x7FFFFFFF;
                    DirSvt = indefinie;
                    ndp = Voie->NbDirPossSensVoieTram(indefinie); // v3.8. DistArretTram travaille dans toutes les
                                                                  // directions !
                    for (dp = 0; dp < ndp; dp++)
                    { // donc il faut alors regarder si la direction courante est accessible depuis d'où l'on vient !
                        DirCrt = Voie->DirSensVoieTram(indefinie, dp);
                        if (Voie->EstDirPossSensVoieTram(InvDir[DirPrc], DirCrt))
                        {
                            DistCrt = Voie->DistArretTram()[NumArretSvt - 1][dp];
                            if (DistArretTram > DistCrt)
                            {
                                DistArretTram = DistCrt;
                                DirSvt = DirCrt; // v3.8
                            }
                        }
                    }
                    Voie = Voie->v(DirPrc = DirSvt); // 1. On va dans la direction ayant la distance la plus petite vers
                                                     // le prochain arrêt. v3.8 (DirPrc)
                    Voie->LigneTram[i] = true;       // 2. On marque la ligne sur cette case
                } while (!Voie->EstTeteArretTram(NumArretSvt));
            }
        }
    }
}
//-----------------------------------------------------------------------------
bool centre_ville::VerifieCalculDistances(type_distance &TypeDistance) // v3.5 : inclut également celui des arrêts de
                                                                       // bus et tram. v3.6... Et des files de taxi
{
    bool Ok = true;
    int i = 0;
    int j = 0;
    int k = 0;
    int d = 0;
    int ndp = 0;
    for (i = 0; Ok && (i < NbX); i++)
    {
        for (j = 0; Ok && (j < NbY); j++)
        {
            ndp = NbDir[v[i][j].DirPoss];
            for (d = 0; Ok && (d < ndp); d++)
            {
                for (k = 0; Ok && (k < NbParkings); k++)
                {
                    if (v[i][j].PassageBus ||                                // Si passage de bus OU
                        Voisine(i, j, Dirdp[d][v[i][j].DirPoss]).PassageBus) // pointe sur un passage de bus
                    { // On ne teste que la distance AVEC passage de bus
                        Ok = (v[i][j].DistParking()[k][d][AVECPSGBUS] < DIST_INDEFINIE);
                        if (!Ok)
                        {
                            xErr = i;
                            yErr = j;
                            TypeDistance = parkings;
                        } // v3.5
                    }
                    else
                    {
                        Ok = (v[i][j].DistParking()[k][d][AVECPSGBUS] < DIST_INDEFINIE) &&
                             (v[i][j].DistParking()[k][d][SANSPSGBUS] < DIST_INDEFINIE);
                        if (!Ok)
                        {
                            xErr = i;
                            yErr = j;
                            TypeDistance = parkings;
                        } // v3.5
                    }
                }
                for (k = 0; Ok && (k < NbArretsBus); k++)
                {                             // v3.0
                    if (ArretBus[k].Defini()) // v3.5
                    {
                        Ok = (v[i][j].DistArretBus()[k][d] < DIST_INDEFINIE);
                        if (!Ok)
                        {
                            xErr = i;
                            yErr = j;
                            TypeDistance = arrets_bus;
                        }
                    }
                }
                for (k = 0; Ok && (k < NbFilesTaxi); k++)
                { // v3.6
                    Ok = (v[i][j].DistFileTaxi()[k][d] < DIST_INDEFINIE);
                    if (!Ok)
                    {
                        xErr = i;
                        yErr = j;
                        TypeDistance = files_taxi;
                    } // v3.6
                }
                // v6.0.8 : files de parking (v5.0) et places de vehlib (v5.3) oubliées : la simulation
                // démarrait, et les véhicules destinés à une place inaccessible tournaient sans fin
                for (k = 0; Ok && (k < NbFilesPark); k++)
                {
                    Ok = (v[i][j].DistFilePark()[k][d][AVECPSGBUS] < DIST_INDEFINIE) &&
                         (v[i][j].PassageBus || // Sur un passage de bus ou pointant sur un, comme pour les
                                                // parkings
                          Voisine(i, j, Dirdp[d][v[i][j].DirPoss]).PassageBus ||
                          (v[i][j].DistFilePark()[k][d][SANSPSGBUS] < DIST_INDEFINIE));
                    if (!Ok)
                    {
                        xErr = i;
                        yErr = j;
                        TypeDistance = files_park;
                    }
                }
                for (k = 0; Ok && (k < NbPlacesVehlib); k++)
                {
                    if (PlaceVehlib[k].Definie())
                    {
                        Ok = (v[i][j].DistPlaceVehlib()[k][d] < DIST_INDEFINIE);
                        if (!Ok)
                        {
                            xErr = i;
                            yErr = j;
                            TypeDistance = files_vehlib;
                        }
                    }
                }
            }
            // v3.5
            ndp = v[i][j].NbDirPossSensVoieTram(indefinie);
            for (d = 0; Ok && (d < ndp); d++)
            {
                for (k = 0; Ok && (k < NbArretsTram); k++)
                {
                    if (ArretTram[k].Defini())
                    {
                        Ok = (v[i][j].DistArretTram()[k][d] < DIST_INDEFINIE);
                        if (!Ok)
                        {
                            xErr = i;
                            yErr = j;
                            TypeDistance = arrets_tram;
                        }
                    }
                }
            }
        }
    }
    return Ok;
}
//-----------------------------------------------------------------------------
bool centre_ville::PrepareCalculDistances() // v3.0 : inclut également celui des arrêts de bus/tram. v3.6 ...Et
                                            // des files de taxi
{ // 6.0.19 : une table par type d'élément (table_distances), au lieu de tableaux dans chaque case
    int NbDirTotal = 0;
    int NbDirTotalTram = 0; // v3.5 : directions possibles des voies de tram
    LibereDistances();      // v6.0.1
    for (int i = 0; i < NbX; i++)
    {
        for (int j = 0; j < NbY; j++)
        {
            voie &Case = v[i][j];
            Case.IndiceDist = NbDirTotal;
            Case.NbDirDist = NbDir[Case.DirPoss];
            NbDirTotal += Case.NbDirDist;
            Case.IndiceDistTram = NbDirTotalTram;
            Case.NbDirDistTram = Case.NbDirPossSensVoieTram(indefinie);
            NbDirTotalTram += Case.NbDirDistTram;
        }
    }
    const distance_double Indefinie2 = {DIST_INDEFINIE, DIST_INDEFINIE};
    DistancesParking.Prepare(NbParkings, NbDirTotal, Indefinie2);
    DistancesArretBus.Prepare(NbArretsBus, NbDirTotal, DIST_INDEFINIE);       // v3.0
    DistancesArretTram.Prepare(NbArretsTram, NbDirTotalTram, DIST_INDEFINIE); // v3.5
    DistancesFileTaxi.Prepare(NbFilesTaxi, NbDirTotal, DIST_INDEFINIE);       // v3.6
    DistancesFilePark.Prepare(NbFilesPark, NbDirTotal, Indefinie2);           // v5.0
    DistancesPlaceVehlib.Prepare(NbPlacesVehlib, NbDirTotal, DIST_INDEFINIE); // v5.3
    return true;
}
//-----------------------------------------------------------------------------
void centre_ville::LibereDistances() // v6.0.1
{
    DistancesParking.Libere();
    DistancesArretBus.Libere();
    DistancesArretTram.Libere();
    DistancesFileTaxi.Libere();
    DistancesFilePark.Libere();
    DistancesPlaceVehlib.Libere();
}
//-----------------------------------------------------------------------------
bool centre_ville::PrepareCalculAttenteTrafic()
{
    bool Ok = true;
    int i = 0;
    int j = 0;
    for (i = 0; Ok && (i < NbX); i++)
    {
        for (j = 0; Ok && (j < NbY); j++)
        {
            Ok = v[i][j].PrepareCalculAttenteTrafic();
        }
    }
    return Ok;
}
//-----------------------------------------------------------------------------
void centre_ville::IncrementeTrafic(int NumVehicule)
{ // A VOIR : créer infos trafic pour les parkings extérieurs.
    vehicule *veh = &Vehicule[NumVehicule - 1];
    if ((veh->NumParkingDepart != 0) && (veh->NumParkingArrivee != 0))
    {
        v[veh->x][veh->y].Trafic[veh->NumParkingDepart][veh->NumParkingArrivee]++;
    }
    if (veh->NumParkingArrivee != 0)
    {
        v[veh->x][veh->y].Trafic[0][veh->NumParkingArrivee]++;
    }
    if (veh->NumParkingDepart != 0)
    {
        v[veh->x][veh->y].Trafic[veh->NumParkingDepart][0]++;
    }
    v[veh->x][veh->y].Trafic[0][0]++;
}
//-----------------------------------------------------------------------------
void centre_ville::IncrementeAttente(int NumVehicule)
{ // A VOIR : créer infos trafic pour les parkings extérieurs.
    vehicule *veh = &Vehicule[NumVehicule - 1];
    const int n = TourCrt - veh->TourDrnDepl - 1;
    if (n > 0)
    {
        if ((veh->NumParkingDepart != 0) && (veh->NumParkingArrivee != 0))
        {
            v[veh->x][veh->y].Attente[veh->NumParkingDepart][veh->NumParkingArrivee] += n;
        }
        if (veh->NumParkingArrivee != 0)
        {
            v[veh->x][veh->y].Attente[0][veh->NumParkingArrivee] += n;
        }
        if (veh->NumParkingDepart != 0)
        {
            v[veh->x][veh->y].Attente[veh->NumParkingDepart][0] += n;
        }
        v[veh->x][veh->y].Attente[0][0] += n;
    }
}
//-----------------------------------------------------------------------------
void centre_ville::PlaceVehicule(int x, int y, int NumVehicule)
{
    IncrementeAttente(NumVehicule);
    if (vue->SimulationSanglante() && // portage
        (x > -1) && (y > -1) && (cv->v[x][y].NbPietons != 0))
    { // v4.3
        cv->v[x][y].TuePietons();
    }
    Vehicule[NumVehicule - 1].Place(x, y);
    if ((x > -1) && (y > -1))
    {
        IncrementeTrafic(NumVehicule);
    }
}
//-----------------------------------------------------------------------------
bool centre_ville::PrepareVehicules()
{
    int NumParkingDepart = 1;
    int NumParkingArrivee = 2;
    int NumFileParkDepart = 0;
    int NumFileParkArrivee = 0;                                           // v5.0
    int NumPlaceParkDepart = 0;                                           // v5.0
    int NumPlaceParkArrivee = 0;                                          // v5.0
    const int NumPrmVehiculePlacePark = NbVehicules - (2 * NbPlacesPark); // v5.0
    const int NbVehiculesAvantCreation = NbVehicules;                     // v5.0
    if (DefinitNbVehicules(NbVehiculesAvantCreation) != NbVehiculesAvantCreation)
    {
        return false;
    }
    for (int i = 0; i < NumPrmVehiculePlacePark; i++)
    {
        assert((NumParkingDepart >= 1) && (NumParkingDepart <= NbParkings));
        // NOLINTNEXTLINE(clang-analyzer-security.ArrayBound) : NumParkingDepart borné par l'assert ci-dessus
        Vehicule[i].Definit(Parking[NumParkingDepart - 1].x, Parking[NumParkingDepart - 1].y, indefinie,
                            NumParkingDepart, NumParkingArrivee, 0, 0, // v5.0 : On ne traite ici que les
                                                                       // départs/arrivées parkings souterrains
                            ((i % (NbVehicules / (2 * NbParkings))) != 0)
                                ? normal
                                : (((i % (NbVehicules / NbParkings)) != 0) ? police : urgence));
        do
        {
            NumParkingArrivee = 1 + (NumParkingArrivee % NbParkings);
            if (NumParkingArrivee == 1)
            {
                NumParkingDepart = 1 + (NumParkingDepart % NbParkings);
                if (NumParkingDepart == 1)
                {
                    NumParkingDepart = 1;
                    NumParkingArrivee = 2;
                }
            }
        } while (NumParkingDepart == NumParkingArrivee);
    }

    // v5.0 : On s'occupe des véhicules partant/arrivant de/sur une place de parking
    for (int i = NumPrmVehiculePlacePark; i < NbVehicules; i++)
    {
        NumParkingDepart = ((i - NumPrmVehiculePlacePark) / NbPlacesPark) *
                           (1 + ((i - NumPrmVehiculePlacePark) % NbParkings)); // Parking au départ pour les dernières
        NumParkingArrivee = (1 - ((i - NumPrmVehiculePlacePark) / NbPlacesPark)) *
                            (1 + ((i - NumPrmVehiculePlacePark) % NbParkings)); // Parking à l'arrivée pour les
                                                                                // premières
        NumPlaceParkDepart = (1 - ((i - NumPrmVehiculePlacePark) / NbPlacesPark)) *
                             (1 + ((i - NumPrmVehiculePlacePark) % NbPlacesPark)); // Place au départ pour les premières
        NumPlaceParkArrivee = ((i - NumPrmVehiculePlacePark) / NbPlacesPark) *
                              (1 + ((i - NumPrmVehiculePlacePark) % NbPlacesPark)); // Place à l'arrivée pour les
                                                                                    // dernières
        assert((NumPlaceParkDepart >= 0) && (NumPlaceParkDepart <= NbPlacesPark) && (NumPlaceParkArrivee >= 0) &&
               (NumPlaceParkArrivee <= NbPlacesPark));
        NumFileParkDepart = (NumPlaceParkDepart != 0) ? (1 + PlacePark[NumPlaceParkDepart - 1].NumFile) : 0;
        NumFileParkArrivee = (NumPlaceParkArrivee != 0) ? (1 + PlacePark[NumPlaceParkArrivee - 1].NumFile) : 0;
        // NOLINTNEXTLINE(clang-analyzer-security.ArrayBound) : places bornées par l'assert ci-dessus
        Vehicule[i].Definit(
            ((NumPlaceParkDepart != 0) ? PlacePark[NumPlaceParkDepart - 1].x : Parking[NumParkingDepart - 1].x),
            ((NumPlaceParkDepart != 0) ? PlacePark[NumPlaceParkDepart - 1].y : Parking[NumParkingDepart - 1].y),
            indefinie, NumParkingDepart, NumParkingArrivee, NumFileParkDepart, NumFileParkArrivee,
            ((i % (NbVehicules / (2 * NbParkings))) != 0)
                ? normal
                : (((i % (NbVehicules / NbParkings)) != 0) ? police : urgence));
    }
    return true;
}
//-----------------------------------------------------------------------------
void centre_ville::PlaceBus(int x, int y, int NumBus) // v3.0
{
    bus *b = &Bus[NumBus - 1];
    if (b->Defini() && (b->DirDrnDepl != 0))
    {
        Voisine(b->x, b->y, InvDir[b->DirDrnDepl]).NumBus = 0; // Effacé de la case d'origine (arrière du bus)
        Voisine(b->x, b->y, InvDir[b->DirDrnDepl]).RedessineCase(false);
    }
    b->DirAvtDrnDepl = b->DirDrnDepl;
    b->DirDrnDepl = b->Dir;
    if (vue->SimulationSanglante() && (cv->v[x][y].NbPietons != 0))
    { // v4.3 // portage
        cv->v[x][y].TuePietons();
    }
    b->Place(x, y);
    if ((v[x][y].NumArretBus != 0) && v[x][y].EstTeteArretBus() && (v[x][y].NumArretBus == b->NumArretSvt))
    { // C'est ici que l'on change d'ARRET
        b->NumDrnArret = b->NumArretSvt;
        b->NbCasesParcouruesDepuisDrnArret = 0;
        b->TourArriveDrnArret = TourCrt;
        b->NumArretSvt = b->NumeroArretSuivant();
        b->TourDepartArretSvt = TourCrt + NbToursStopArretBus;
        b->FaitMonterEtDescendre();
    }
    else
    {
        b->NbCasesParcouruesDepuisDrnArret++;
    }
    b->NbCasesParcourues++;
    b->TourDrnDepl = TourCrt;
    // On ne dessine pas l'arrière du bus car ils peuvent sortir du dépôt (contrairement aux trams).
    v[x][y].NumBus = NumBus;
    Voisine(x, y, InvDir[b->DirDrnDepl]).RedessineCase(false);
    v[x][y].RedessineCase(false);
}
//-----------------------------------------------------------------------------
void centre_ville::PlaceTram(int x, int y, int NumTram) // v3.5
{
    tram *t = &Tram[NumTram - 1];
    if (t->Defini() && (t->DirDrnDepl != 0))
    {
        Voisine(t->x, t->y, InvDir[t->DirDrnDepl]).NumTram = 0; // Effacé de la case d'origine (arrière du tram)
        Voisine(t->x, t->y, InvDir[t->DirDrnDepl]).RedessineCase(false);
    }
    t->DirAvtDrnDepl = t->DirDrnDepl;
    t->DirDrnDepl = t->Dir;
    if (vue->SimulationSanglante() && (cv->v[x][y].NbPietons != 0))
    { // v4.3 // portage
        cv->v[x][y].TuePietons();
    }
    t->Place(x, y);
    if ((v[x][y].NumArretTram != 0) && v[x][y].EstTeteArretTram() && (v[x][y].NumArretTram == t->NumArretSvt))
    { // C'est ici que l'on change d'ARRET
        t->NumDrnArret = t->NumArretSvt;
        t->NbCasesParcouruesDepuisDrnArret = 0;
        t->TourArriveDrnArret = TourCrt;
        t->NumArretSvt = t->NumeroArretSuivant();
        t->TourDepartArretSvt = TourCrt + NbToursStopArretTram;
        if (t->TourArriveDrnArret != 0)
        {
            t->FaitMonterEtDescendre();
        }
    }
    else
    {
        t->NbCasesParcouruesDepuisDrnArret++;
    }
    t->NbCasesParcourues++;
    t->TourDrnDepl = TourCrt;
    Voisine(x, y, InvDir[t->DirDrnDepl]).NumTram = NumTram;
    v[x][y].NumTram = NumTram;
    Voisine(x, y, InvDir[t->DirDrnDepl]).RedessineCase(false);
    v[x][y].RedessineCase(false);
}
//-----------------------------------------------------------------------------
void centre_ville::PlaceVehiculeTaxi(int x, int y, int NumTaxi) // v3.6
{
    taxi *t = &Taxi[NumTaxi - 1];
    int NumPlace = 0;
    t->DirDrnDepl = t->Dir;
    if (vue->SimulationSanglante() && (cv->v[x][y].NbPietons != 0))
    { // v4.3 // portage
        cv->v[x][y].TuePietons();
    }
    t->Place(x, y);
    t->NbCasesParcouruesDrnCourse++;
    t->NbCasesParcourues++;
    t->TourDrnDepl = TourCrt;
    t->DirDrnDepl = t->Dir;
    v[x][y].NumTaxi = NumTaxi;
    v[x][y].RedessineCase(false);
    switch (t->Situation)
    {
    case stDepart:
        t->FaitDescendrePietons();
        if (t->NbPersonnes == 0)
        {
            t->Situation = stRetour;
            t->TourDepotDrnCourse = TourCrt;
        }
        break;
    case stRetour:
        NumPlace = v[x][y].NumPlaceTaxi;
        if ((NumPlace != 0) &&                                 // Si c'est une place de taxi
            (PlaceTaxi[NumPlace - 1].NumFile == t->NumFile) && // de la file du taxi ET
            PlaceTaxi[NumPlace - 1].EstTeteFile())             // c'est la tête de file ALORS
        {
            t->Situation = stAttente;
            t->TourRetourDrnCourse = TourCrt;
        }
        break;
    default:
        break; // v5.4 : évite l'avertissement
    }
}
//-----------------------------------------------------------------------------
void centre_ville::PlaceVehiculeVehlib(int x, int y, int NumVehlib) // v5.3
{
    vehlib *vl = &Vehlib[NumVehlib - 1];
    int NumPlace = 0;
    vl->DirDrnDepl = vl->Dir;
    if (vue->SimulationSanglante() && (cv->v[x][y].NbPietons != 0))
    { // portage
        cv->v[x][y].TuePietons();
    }
    vl->Place(x, y);
    vl->NbCasesParcourues++;
    vl->TourDrnDepl = TourCrt;
    vl->DirDrnDepl = vl->Dir;
    v[x][y].NumVehlib = NumVehlib;
    v[x][y].RedessineCase(false);
    if (vl->Situation == svDepart)
    {
        NumPlace = v[x][y].NumPlaceVehlib;
        if (((NumPlace != 0) &&                                          // Si c'est une place de vehlib ET
             (NumPlace == vl->NumPlaceArrivee) &&                        // c'est la place d'arrivée ET
             (PlaceVehlib[NumPlace - 1].NumVehlibReserve == NumVehlib))) // elle a été
                                                                         // réservée pour ce
                                                                         // vehlib
        {
            vl->FaitDescendrePietons();
            vl->NumPlaceDepart = NumPlace;
            vl->NumPlaceArrivee = 0;
            vl->Situation = svAttente;
            PlaceVehlib[NumPlace - 1].NumVehlibReserve = 0;
        }
    }
}
//-----------------------------------------------------------------------------
void centre_ville::DefinitBus(int NumBus, int NumLigne) // v3.0
{
    Bus[NumBus - 1].Definit(NumLigne, LigneBus[NumLigne - 1].NumArret[0]);
    LigneBus[NumLigne - 1].AjouteBus(NumBus);
}
//-----------------------------------------------------------------------------
void centre_ville::DefinitTram(int NumTram, int NumLigne) // v3.5
{
    Tram[NumTram - 1].Definit(NumLigne, LigneTram[NumLigne - 1].NumArret[0]);
    LigneTram[NumLigne - 1].AjouteTram(NumTram);
}
//-----------------------------------------------------------------------------
bool centre_ville::PrepareBus() // v3.0
{
    int i = 0;
    int NbBusADefinir = 0;
    int NumLigneCrt = 0;
    // On définit 1 bus de 1 à 3 arrêts. 2 bus de 4 à 6 arrêts et 3 bus de 7 à 9 arrêts
    for (i = 0; i < NBMAXLIGNESBUSTRAM; i++)
    {
        if (LigneBus[i].NbArrets != 0)
        {
            NbBusADefinir += 1 + ((LigneBus[i].NbArrets - 1) / 3);
        }
    }
    if (DefinitNbBus(NbBusADefinir) != NbBusADefinir)
    {
        return false;
    }
    for (i = 0; i < NbBus; i++)
    {
        while ((LigneBus[NumLigneCrt].NbArrets == 0) ||
               (LigneBus[NumLigneCrt].NbBus == (1 + ((LigneBus[NumLigneCrt].NbArrets - 1) / 3))))
        {
            NumLigneCrt = (NumLigneCrt + 1) % NBMAXLIGNESBUSTRAM;
        }
        DefinitBus(i + 1, NumLigneCrt + 1);
        NumLigneCrt = (NumLigneCrt + 1) % NBMAXLIGNESBUSTRAM;
    }
    return true;
}
//-----------------------------------------------------------------------------
bool centre_ville::PrepareTrams() // v3.5
{
    int i = 0;
    int j = 0;
    int x = 0;
    int y = 0;
    int NbTramsADefinir = 0;
    int NumLigneCrt = 0;
    int NumArret = 0;
    int NumProchArret = 0;
    int dp = 0;
    int DistProchArret = 0;
    bool ArretTrv = false;
    voie *Voie = nullptr;
    // On définit 1 tram de 1 à 3 arrêts. 2 tram de 4 à 6 arrêts et 3 tram de 7 à 9 arrêts
    for (i = 0; i < NBMAXLIGNESBUSTRAM; i++)
    {
        if (LigneTram[i].NbArrets != 0)
        {
            NbTramsADefinir += 1 + ((LigneTram[i].NbArrets - 1) / 3);
        }
    }
    if (DefinitNbTrams(NbTramsADefinir) != NbTramsADefinir)
    {
        return false;
    }
    for (i = 0; i < NbTrams; i++)
    {
        while ((LigneTram[NumLigneCrt].NbArrets == 0) ||
               (LigneTram[NumLigneCrt].NbTrams == (1 + ((LigneTram[NumLigneCrt].NbArrets - 1) / 3))))
        {
            NumLigneCrt = (NumLigneCrt + 1) % NBMAXLIGNESBUSTRAM;
        }
        DefinitTram(i + 1, NumLigneCrt + 1);
        NumLigneCrt = (NumLigneCrt + 1) % NBMAXLIGNESBUSTRAM;
    }
    // On place les trams sur les arrêts...
    for (ArretTrv = true, i = 0; ArretTrv && (i < NbTrams); i++)
    {
        for (ArretTrv = false, j = 0; (!ArretTrv) && (j < LigneTram[Tram[i].NumLigne - 1].NbArrets); j++)
        { // Pour chaque tram, on cherche le premier arrêt libre dans l'ordre de leur ligne respective
            NumArret = LigneTram[Tram[i].NumLigne - 1].NumArret[j];
            NumProchArret =
                LigneTram[Tram[i].NumLigne - 1].NumArret[(j + 1) % LigneTram[Tram[i].NumLigne - 1].NbArrets];
            ArretTrv = ArretTram[NumArret - 1].Defini() &&
                       (v[x = ArretTram[NumArret - 1].x][y = ArretTram[NumArret - 1].y].NumTram == 0);
        }
        if (ArretTrv)
        {
            Voie = &v[x][y];
            // On trouve dans quel sens on va
            for (DistProchArret = DIST_INDEFINIE, dp = 0; dp < Voie->NbDirPossSensVoieTram(indefinie); dp++)
            {
                if (Voie->DistArretTram()[NumProchArret - 1][dp] < DistProchArret)
                {
                    DistProchArret = Voie->DistArretTram()[NumProchArret - 1][dp];
                    Tram[i].Dir = Tram[i].DirDrnDepl = Tram[i].DirAvtDrnDepl = Voie->DirSensVoieTram(indefinie, dp);
                    Tram[i].NumArretSvt = NumArret;
                }
            }
            // Et ENFIN on place le tram
            PlaceTram(x, y, i + 1);
        }
    }
    if (!ArretTrv)
    {
        vue->AfficheMessage("\n\
Il n'est pas possible de placer tous les trams sur un arrêt de leur ligne respective !\n\
Réorganisez votre réseau de trams (les lignes ou les arrêts) pour que le réseau puisse démarrer, puis réessayez !",
                            "Impossible de placer les trams", tmAvertissement, 0, 0); // portage : vue->AfficheMessage
        return false;
    }
    return true;
}
//-----------------------------------------------------------------------------
bool centre_ville::PrepareTaxis() // v3.6. Les taxis ainsi que leurs places et leurs files
{
    int NbTaxisADefinir = 0;
    int NumTaxi = 0;
    for (int i = 0; i < NbPlacesTaxi; i++)
    {
        NbTaxisADefinir += static_cast<int>(PlaceTaxi[i].Definie());
    }
    if (DefinitNbTaxis(NbTaxisADefinir) != NbTaxisADefinir)
    {
        return false;
    }
    for (int i = 0; i < NbPlacesTaxi; i++)
    {
        if (PlaceTaxi[i].Definie())
        {
            Taxi[NumTaxi].DefinitEtPlace(PlaceTaxi[i].NumFile, i);
            NumTaxi++;
        }
    }
    return true;
}
//-----------------------------------------------------------------------------
bool centre_ville::PrepareVehlibs() // v5.3. Les vehlibs ainsi que leurs places
{
    if (NbPlacesVehlib == 0)
    {
        NbVehlibs = 0;
        return true;
    }
    const auto NumVehlibPlace = [this](int x) {
        return static_cast<int>(std::lround(x * (TauxRemplissageVehlib / 100.0)));
    };
    const int NbVehlibsADefinir = 1 + NumVehlibPlace(NbPlacesVehlib - 1);
    int // On calcule le n° de vehlib pour la dernière place (NbPlacesVehlib-1) auquel on rajoute 1 pour avoir le nombre
        // de vehlibs à créer.
        NumVehlib = 0;

    // On alloue les vehlibs...
    if (DefinitNbVehlibs(NbVehlibsADefinir) != NbVehlibsADefinir)
    {
        return false;
    }

    // On détermine la place de chaque vehlib pour répartir les places libres
    std::vector<int> NumPlace(NbVehlibsADefinir, 0);
    for (int i = 0; i < NbPlacesVehlib; i++)
    {
        if (NumPlace[NumVehlibPlace(i)] == 0)
        {
            NumPlace[NumVehlibPlace(i)] = i;
        }
    }

    // On définit et place les vehlibs.
    for (int i = 0; i < NbVehlibsADefinir; i++)
    {
        Vehlib[NumVehlib].DefinitEtPlace(NumPlace[i]);
        NumVehlib++;
    }
    return true;
}
//-----------------------------------------------------------------------------
bool centre_ville::PreparePietons() // v2.0
{
    int x = 0;
    int y = 0;
    int i = 0;
    const int NbPietonsAvantCreation = NbPietons;
    direction Dir{};
    if (DefinitNbPietons(NbPietonsAvantCreation) != NbPietonsAvantCreation)
    {
        return false;
    }
    for (i = 0; i < NbPietons; i++)
    {
        do
        {
            x = alea::Rand() % NbX;
            y = alea::Rand() % NbY;
            Dir = static_cast<direction>(1 + (alea::Rand() % 4));
        } while (!v[x][y].PietonPeutAller(Dir));
        Pieton[i].Definit(x, y, Dir, 0);
        v[x][y].AjoutePieton(i);
    }
    return true;
}
//-----------------------------------------------------------------------------
void centre_ville::DeplacePieton(int NumPieton, int x, int y, direction d) // v2.0
{
    v[Pieton[NumPieton].x][Pieton[NumPieton].y].RetirePieton(NumPieton);
    Pieton[NumPieton].Place(x, y);
    Pieton[NumPieton].DemiCasesParcourues++; // v3.0
    if (Pieton[NumPieton].Dir != d)
    {
        Pieton[NumPieton].Dir = d;
    }
    Pieton[NumPieton].TourDrnDepl = TourCrt;
    if (Pieton[NumPieton].PsgPtTrv && // Si le piéton se dirige vers un passage piéton ET
        v[x][y].PassagePietons)
    {                                       // Si le passage piéton est trouvé ALORS
        Pieton[NumPieton].PsgPtTrv = false; // On réactive les recherches de passages piétons
    }
    v[x][y].AjoutePieton(NumPieton);
    vue->RedessineCase(x, y, false); // portage
}

//-----------------------------------------------------------------------------
bool centre_ville::ChangeDirectionSiPossible(int x, int y, dirposs dp) // ATTENTION : dp = N|E|S|O
{                                                                      // On change si...
    voie *vv = &v[x][y];
    if ((!vv->PassagePietons) && // v3.5
        (vv->NumArretBus == 0) && (TourCrt == 0) && ((vv->DirPoss != dp) || (!vv->PassageBus)) &&
        (((vv->DirPoss & dp) == 0) || (vv->Priorite[Dirdp[0][dp] - 1] == 0)))
    {
        vv->DirPoss = static_cast<dirposs>(vv->DirPoss ^ dp);
        vue->RedessineCase(x, y, true); // portage
        return true;
    }
    return false;
}
//-----------------------------------------------------------------------------
void centre_ville::CalculeDistances() // v3.0 : inclut également celles des arrêts de bus. v3.5... Et des trams !
                                      // v3.6... Et des files de taxi !
{
    int i = 0;
    vue->AfficheStatut(0, "Calcul des distances aux parkings..."); // portage
    vue->RepeintStatut();                                          // portage
    for (i = 0; i < NbParkings; i++)
    {
        vue->AfficheStatut(3, Format("%d/%d", TableauDeConstantes(i + 1, NbParkings))); // portage
        vue->RepeintStatut();                                                           // portage
        if (Parking[i].Defini())
        {
            v[Parking[i].x][Parking[i].y].CalculeDistance(DistancesParking.Element(i), 0, 0, indefinie);
        }
    }
    // v3.0
    vue->AfficheStatut(0, "Calcul des distances aux arrêts de bus..."); // portage
    vue->RepeintStatut();                                               // portage
    for (i = 0; i < NbArretsBus; i++)
    {
        vue->AfficheStatut(3, Format("%d/%d", TableauDeConstantes(i + 1, NbArretsBus))); // portage
        vue->RepeintStatut();                                                            // portage
        if (ArretBus[i].Defini())
        {
            v[ArretBus[i].x][ArretBus[i].y].CalculeDistance(DistancesArretBus.Element(i), 0, indefinie);
        }
    }
    // v3.5
    vue->AfficheStatut(0, "Calcul des distances aux arrêts de tram..."); // portage
    vue->RepeintStatut();                                                // portage
    for (i = 0; i < NbArretsTram; i++)
    {
        vue->AfficheStatut(3, Format("%d/%d", TableauDeConstantes(i + 1, NbArretsTram))); // portage
        vue->RepeintStatut();                                                             // portage
        if (ArretTram[i].Defini())
        {
            v[ArretTram[i].x][ArretTram[i].y].CalculeDistanceArretTram(DistancesArretTram.Element(i), 0, indefinie);
        }
    }
    // v3.6
    vue->AfficheStatut(0, "Calcul des distances aux files de taxi..."); // portage
    vue->RepeintStatut();                                               // portage
    for (i = 0; i < NbFilesTaxi; i++)
    {
        vue->AfficheStatut(3, Format("%d/%d", TableauDeConstantes(i + 1, NbFilesTaxi))); // portage
        vue->RepeintStatut();                                                            // portage
        v[PlaceTaxi[FileTaxi[i].NumPlaceQueue].x][PlaceTaxi[FileTaxi[i].NumPlaceQueue].y].CalculeDistance(
            DistancesFileTaxi.Element(i), 0, indefinie);
    }
    // v5.0
    vue->AfficheStatut(0, "Calcul des distances aux files de parkings..."); // portage
    vue->RepeintStatut();                                                   // portage
    for (i = 0; i < NbFilesPark; i++)
    {
        vue->AfficheStatut(3, Format("%d/%d", TableauDeConstantes(i + 1, NbFilesPark))); // portage
        vue->RepeintStatut();                                                            // portage
        v[PlacePark[FilePark[i].NumPlaceQueue].x][PlacePark[FilePark[i].NumPlaceQueue].y].CalculeDistance(
            DistancesFilePark.Element(i), 0, 0, indefinie);
    }
    // v5.3
    vue->AfficheStatut(0, "Calcul des distances aux places de vehlib..."); // portage
    vue->RepeintStatut();                                                  // portage
    for (i = 0; i < NbPlacesVehlib; i++)
    {
        vue->AfficheStatut(3, Format("%d/%d", TableauDeConstantes(i + 1, NbPlacesVehlib))); // portage
        vue->RepeintStatut();                                                               // portage
        v[PlaceVehlib[i].x][PlaceVehlib[i].y].CalculeDistance(DistancesPlaceVehlib.Element(i), 0, indefinie);
    }
}
//-----------------------------------------------------------------------------
void centre_ville::RessuscitePietons() // v4.3.1
{
    for (int i = 0; i < NbPietons; i++)
    {
        pieton *p = &Pieton[i];
        if (p->Ecrase)
        {
            p->Ecrase = false;
            vue->RedessineCase(p->x, p->y, true); // portage
        }
        if (p->Mort) // v5.4.1
        {
            p->SupprimeInfection();
            vue->RedessineCase(p->x, p->y, true); // portage
        }
    }
}
//-----------------------------------------------------------------------------
void centre_ville::PlaceVehiculesSurPlacesPark() // v5.0
{
    for (int i = 0; i < NbVehicules; i++)
    {
        vehicule *Veh = &Vehicule[i];
        if (Veh->NumFileParkDepart != 0)
        {
            v[Veh->x][Veh->y].NumVehicule = i + 1;
            PlaceVehicule(Veh->x, Veh->y, i + 1);      // v2.2.7. Inclut les stats d'Attente/Trafic
            vue->RedessineCase(Veh->x, Veh->y, false); // portage
            Vehicule[i].TourDepart = cv->TourCrt;
            NbVehiculesSortis++;
        }
    }
}
//-----------------------------------------------------------------------------
bool centre_ville::ExisteFeuPietons(int x, int y) const // v5.2
{ // Utilisé pour empêcher de supprimer un passage piéton si des feux de piétons existent (à supprimer avant le passage
  // piéton)
    bool Existe = false;
    int k = 0;
    for (int d = nord; (!Existe) && (d <= ouest); d++)
    {
        for (k = 1; Voisine(x, y, k, d).PassagePietons; k++)
        {
            ;
        }
        Existe = (Voisine(x, y, k, d).NumFeuP[InvDir[d] - 1] > 0);
    }
    return Existe;
}
//-----------------------------------------------------------------------------
void centre_ville::PrepareFeuxPietons() // v5.2
{
    // On réinitialise d'abord
    for (int i = 0; i < NbFeux; i++)
    {
        if (Feu[i].NbFeuxPtn != 0)
        {
            Feu[i].NumFeuxPtn.clear(); // v6.0.7 : la liste est refaite (l'original la libérait deux fois)
            Feu[i].NbFeuxPtn = 0;
        }
    }
    // On charge les feux de piétons
    for (int i = 0; i < NbFeuxPieton; i++)
    {
        for (int j = 0; j < FeuP[i].NbFeux; j++)
        {
            Feu[FeuP[i].NumFeux[j] - 1].AjouteFeuPtn(i + 1);
        }
    }
}
//-----------------------------------------------------------------------------
bool centre_ville::FeuPietonsPossible(int x, int y, direction d) const // v5.2
{ // Utilisé pour savoir si l'on peut poser un feu piéton opposé (à l'extrémité du passage piétons)
    bool Possible = true; // Optimiste !
    int k = 0;
    for (k = 1; Voisine(x, y, k, d).PassagePietons; k++)
    {
        ;
    }
    Possible = (k > 1) && Voisine(x, y, k, d).FeuPietonsPossible(); // Au moins un passage piétons et il mène sur un
                                                                    // trottoir libre pour les piétons (sans décors ni
                                                                    // voies de circulation)
    return Possible;
}
//-----------------------------------------------------------------------------
bool centre_ville::PassagePietonFranchissable(int x, int y, direction d) const // v5.2
{ // Permet de savoir si un feu piéton n'est pas au rouge dans le cas où en (x,y) il y a un passage piéton
    if (Voisine(x, y, InvDir[d]).PassagePietons)
    {                // Si le piéton n'était pas au bord du passage piétons alors
        return true; // il a déjà commencé à traverser.
    }
    bool Possible = true; // Optimiste
    int k = 0;
    int NumFeuP = 0;
    for (k = 1; Voisine(x, y, k, d).PassagePietons; k++)
    {
        ;
    }
    NumFeuP = Voisine(x, y, k, d).NumFeuP[InvDir[d] - 1];
    if (NumFeuP != 0)
    {
        Possible = (FeuP[NumFeuP - 1].Etat != rouge);
    }
    return Possible;
}
//-----------------------------------------------------------------------------
void /*__fastcall*/ centre_ville::EffectueTourSimulation() // portage
{
    int i = 0;
    int j = 0;
    TourEnCours = true;
    NbVehiculesDeplaces = 0;
    if (vue->ResurrectionDemandee()) // v4.3.1 // portage
    {
        RessuscitePietons();
        vue->DefinitResurrectionDemandee(false); // portage
    }
    // 0. On déplace les piétons
    for (i = 0; i < NbX; i++)
    {
        for (j = 0; j < NbY; j++)
        {
            if (v[i][j].NbPietons != 0)
            {
                v[i][j].DeplacePietons();
            }
        }
    }
    NbPietonsGueris = NbPietonsInfectes = NbPietonsMorts = 0L;
    // 0bis. v5.4.1 : On calcule les stats épidémiques de piétons si mode épidémie
    if (vue->ActionEpidemieCochee())
    { // portage
        for (i = 0; i < NbPietons; i++)
        {
            if (Pieton[i].Mort)
            {
                NbPietonsMorts++;
            }
            else
            {
                if (Pieton[i].EstContagieux())
                {
                    NbPietonsInfectes++;
                }
                else if (Pieton[i].NbInfections() > 0)
                {
                    NbPietonsGueris++;
                }
            }
        }
    }
    // 1. On calcule les priorités de passage sur chaque case de la voie
    for (i = 0; i < NbX; i++)
    {
        for (j = 0; j < NbY; j++)
        {
            v[i][j].CalculeProchainVehiculeOuBusOuTramOuTaxiOuVehlib();
        }
    }

    // 2. On déplace les véhicules
    for (i = 0; i < NbX; i++)
    {
        for (j = 0; j < NbY; j++)
        {
            v[i][j].DeplaceVehiculeOuBusOuTramOuTaxi();
        }
    }

    // 3. On fait tourner les feux...
    for (i = 0; i < NbFeuxPieton; i++)
    {
        FeuP[i].Evolue(); // v5.2
    }
    for (i = 0; i < NbFeux; i++)
    {
        Feu[i].Evolue();
    }
    for (i = 0; i < NbFeuxPieton; i++)
    {
        FeuP[i].ChangeEtat(); // v5.2
    }
    for (i = 0; i < NbFeux; i++)
    {
        Feu[i].ChangeEtat();
    }

    // 4. On sort les voitures des parkings sur la voie
    for (i = 0; i < NbParkings; i++)
    {
        Parking[i].PlaceProchainVehicule();
    }
    // 4bis. Ainsi que les bus du dépôt. v3.0
    DepotBus.PlaceProchainBus();
    // 4ter. v5.0 : On place les véhicules sur les places de parkings
    if (TourCrt == 0)
    {
        PlaceVehiculesSurPlacesPark();
    }
    // 5a. On calcule la prochaine direction de chaque véhicule
    for (i = 0; i < NbVehicules; i++)
    {
        if (Vehicule[i].Defini())
        {
            Vehicule[i].CalculeProchaineDirection(); // Vehicule[i].Dir défini
        }
    }
    // 5b. Ainsi que celle des bus. v3.0
    for (i = 0; i < NbBus; i++)
    {
        if (Bus[i].Defini())
        {
            Bus[i].CalculeProchaineDirection(); // Bus[i].Dir défini
        }
    }
    // 5c. Ainsi que celle des trams. v3.5
    for (i = 0; i < NbTrams; i++)
    {
        if (Tram[i].Defini())
        {
            Tram[i].CalculeProchaineDirection(); // Tram[i].Dir défini
        }
    }
    // 5d. Ainsi que celle des taxis. v3.6
    for (i = 0; i < NbTaxis; i++)
    {
        Taxi[i].CalculeProchaineDirection(); // Taxi[i].Dir défini
    }
    // 5e. Ainsi que celle des vehlibs. v5.3
    for (i = 0; i < NbVehlibs; i++)
    {
        Vehlib[i].CalculeProchaineDirection(); // Vehlib[i].Dir défini
    }

    // 6a. On rafraîchit les véhicules (la direction a pu changé)
    for (i = 0; i < 2 * NbParkings; i++)
    {
        const int NumVeh = i * (NbVehicules / NbParkings / 2);
        if (Vehicule[NumVeh].Defini())
        {
            vue->RedessineCase(Vehicule[NumVeh].x, Vehicule[NumVeh].y, false); // portage
        }
    }
    // 6b. Et les bus (la direction a pu changé). v3.0
    for (i = 0; i < NbBus; i++)
    {
        if (Bus[i].Defini())
        {
            vue->RedessineCase(Bus[i].x, Bus[i].y, false); // portage
        }
    }
    // 6c. Et les trams (la direction a pu changé). v3.5
    for (i = 0; i < NbTrams; i++)
    {
        if (Tram[i].Defini())
        {
            vue->RedessineCase(Tram[i].x, Tram[i].y, false); // portage
        }
    }
    // 6d. Et les taxis (la direction a pu changé). v3.6
    for (i = 0; i < NbTaxis; i++)
    {
        vue->RedessineCase(Taxi[i].x, Taxi[i].y, false); // portage
    }
    // 6e. Et les vehlibs (la direction a pu changé). v5.3
    for (i = 0; i < NbVehlibs; i++)
    {
        vue->RedessineCase(Vehlib[i].x, Vehlib[i].y, false); // portage
    }

    // 7. On affiche les stats...
    Stats[TourCrt % NbToursStats].Definit();
    if (vue->FicheVisible(fiStatistiques))
    {
        vue->RafraichitFiche(fiStatistiques); // portage
    }
    if (vue->FicheVisible(fiStatsVehicules))
    {
        vue->RafraichitFiche(fiStatsVehicules); // portage
    }
    if (vue->FicheVisible(fiInfosVehicule))
    {
        vue->RafraichitFiche(fiInfosVehicule); // portage
    }
    if (vue->FicheVisible(fiInfosBus))
    {
        vue->RafraichitFiche(fiInfosBus); // v3.0 // portage
    }
    if (vue->FicheVisible(fiInfosTram))
    {
        vue->RafraichitFiche(fiInfosTram); // v3.5 // portage
    }
    if (vue->FicheVisible(fiInfosTaxi))
    {
        vue->RafraichitFiche(fiInfosTaxi); // v3.6 // portage
    }
    if (vue->FicheVisible(fiInfosVehlib))
    {
        vue->RafraichitFiche(fiInfosVehlib); // v5.3 // portage
    }
    if (vue->FicheVisible(fiInfosPieton))
    {
        vue->RafraichitFiche(fiInfosPieton); // v3.0 // portage
    }
    if (vue->RechercheVisible()) // portage
    {
        if (vue->RechercheCochee(orVehicule))
        {                                        // portage
            vue->RechercheActualise(orVehicule); // portage
        }
        if (vue->RechercheCochee(orPieton))
        {                                      // portage
            vue->RechercheActualise(orPieton); // portage
        }
        if (vue->RechercheCochee(orBus))
        {                                   // v3.0 // portage
            vue->RechercheActualise(orBus); // portage
        }
        if (vue->RechercheCochee(orTram))
        {                                    // v3.5 // portage
            vue->RechercheActualise(orTram); // portage
        }
        if (vue->RechercheCochee(orTaxi))
        {                                    // v3.6 // portage
            vue->RechercheActualise(orTaxi); // portage
        }
        if (vue->RechercheCochee(orVehlib))
        {                                      // v5.3 // portage
            vue->RechercheActualise(orVehlib); // portage
        }
    }

    TourCrt += static_cast<int>(NbVehiculesArrives < NbVehicules);
    TourEnCours = false;
}
//-----------------------------------------------------------------------------
bool centre_ville::Charge(int hfile)
{
    int i = 0;
    int j = 0;
    int NvNbParkings = 0;
    int NvNbPlacesPark = 0;   // v5.1
    int NvNbPlacesVehlib = 0; // v5.1 // v5.3
    bool Ok = false;
    const int Position = FileSeek(hfile, 0, 1); // v6.0.2 : taille du fichier, pour NombreCredible
    TailleFichierCharge = FileSeek(hfile, 0, 2);
    FileSeek(hfile, Position, 0);
    NbPietons = -1;
    Ok = LitChamp(hfile, &NbFeux, sizeof(NbFeux));
    if (NbFeux >= VERSIONMIN) // v3.0
    {                         // Version stockée
        VersionFichier = NbFeux;
        Ok = LitChamp(hfile, &NbFeux, sizeof(NbFeux)); // On relit le nombre de feux
    }
    else
    { // Version non stockée compatible (2.0 <= version < 3.0)
        VersionFichier = VERSIONANC;
    }
    if (VersionFichier > VERSION)
    {
        return false; // v3.5 : n'ouvre pas les fichier de simulation générés par une version ultérieure de la version
                      // actuelle (format de fichier)
    }
    if (VersionFichier < 0x400) // On charge les dimensions de la zone
    {
        NbX = NBXDEF;
        NbY = NBYDEF;
    }
    else
    {
        Ok = Ok && LitChamp(hfile, &NbX, sizeof(NbX)) && // v6.0.2 : Ok&& (une erreur de lecture du nombre de feux était
                                                         // oubliée)
             LitChamp(hfile, &NbY, sizeof(NbY)) && (NbX > 0) && (NbY > 0) &&
             NombreCredible(static_cast<long long>(NbX) * NbY); // v6.0.2
    }
    if (Ok)
    {
        CreeZone(); // v6.0.2 : si Ok, sinon NbX et NbY peuvent être indéterminés
    }
    if (Ok)
    {
        Ok = ((VersionFichier < 0x520) || // v5.2
              LitChamp(hfile, &NbFeuxPieton, sizeof(NbFeuxPieton))) &&
             LitChamp(hfile, &NbParkings, sizeof(NbParkings)) && LitChamp(hfile, &NbVehicules, sizeof(NbVehicules)) &&
             LitChamp(hfile, &NbPietons, sizeof(NbPietons)) &&
             ((VersionFichier < 0x300) || // v3.0
              (LitChamp(hfile, &NbArretsBus, sizeof(NbArretsBus)) &&
               LitChamp(hfile, &NbLignesBus, sizeof(NbLignesBus)))) &&
             ((VersionFichier < 0x350) || // v3.5
              (LitChamp(hfile, &NbArretsTram, sizeof(NbArretsTram)) &&
               LitChamp(hfile, &NbLignesTram, sizeof(NbLignesTram)))) &&
             ((VersionFichier < 0x360) || // v3.6
              (LitChamp(hfile, &NbPlacesTaxi, sizeof(NbPlacesTaxi)) && LitChamp(hfile, &NbTaxis, sizeof(NbTaxis)))) &&
             ((VersionFichier < 0x500) || // v5.0
              LitChamp(hfile, &NbPlacesPark, sizeof(NbPlacesPark))) &&
             ((VersionFichier < 0x530) || // v5.3
              (LitChamp(hfile, &NbPlacesVehlib, sizeof(NbPlacesVehlib)) &&
               LitChamp(hfile, &NbVehlibs, sizeof(NbVehlibs)))) &&
             LitChamp(hfile, &DureeVert, sizeof(DureeVert)) && LitChamp(hfile, &DureeOrange, sizeof(DureeOrange)) &&
             LitChamp(hfile, &DureeRouge, sizeof(DureeRouge)) &&
             LitChamp(hfile, &AttenteMaxVeh, sizeof(AttenteMaxVeh)) &&
             LitChamp(hfile, &NbToursParSeconde, sizeof(NbToursParSeconde)) &&
             LitChamp(hfile, &NbToursStats, sizeof(NbToursStats)) &&
             LitChamp(hfile, &QuadrillageStats, sizeof(QuadrillageStats)) &&
             LitChamp(hfile, &TypeStats, sizeof(TypeStats)) && LitChamp(hfile, &NbVitesses, sizeof(NbVitesses)) &&
             LitChamp(hfile, &NbMaxCasesDetectPsgPt, sizeof(NbMaxCasesDetectPsgPt)) &&
             LitChamp(hfile, &ProbaDirPsgPt, sizeof(ProbaDirPsgPt)) &&
             ((VersionFichier < 0x300) || // v3.0
              (LitChamp(hfile, &NbMaxCasesDetectArretBus, sizeof(NbMaxCasesDetectArretBus)) &&
               LitChamp(hfile, &ProbaDirArretBus, sizeof(ProbaDirArretBus)) &&
               LitChamp(hfile, &ProbaArretBusDmd, sizeof(ProbaArretBusDmd)) &&
               LitChamp(hfile, &NbMaxPersonnesBus, sizeof(NbMaxPersonnesBus)) &&
               LitChamp(hfile, &NbToursStopArretBus, sizeof(NbToursStopArretBus)) &&
               LitChamp(hfile, &NbMaxToursAttenteBus, sizeof(NbMaxToursAttenteBus)))) &&
             ((VersionFichier < 0x350) || // v3.5
              (LitChamp(hfile, &NbMaxCasesDetectArretTram, sizeof(NbMaxCasesDetectArretTram)) &&
               LitChamp(hfile, &ProbaDirArretTram, sizeof(ProbaDirArretTram)) &&
               LitChamp(hfile, &ProbaArretTramDmd, sizeof(ProbaArretTramDmd)) &&
               LitChamp(hfile, &NbMaxPersonnesTram, sizeof(NbMaxPersonnesTram)) &&
               LitChamp(hfile, &NbToursStopArretTram, sizeof(NbToursStopArretTram)) &&
               LitChamp(hfile, &NbMaxToursAttenteTram, sizeof(NbMaxToursAttenteTram)))) &&
             ((VersionFichier < 0x360) || // v3.6
              (LitChamp(hfile, &NbMaxCasesDetectFileTaxi, sizeof(NbMaxCasesDetectFileTaxi)) &&
               LitChamp(hfile, &ProbaDirFileTaxi, sizeof(ProbaDirFileTaxi)) &&
               LitChamp(hfile, &ProbaArretTaxiDmd, sizeof(ProbaArretTaxiDmd)) &&
               LitChamp(hfile, &NbMaxPersonnesTaxi, sizeof(NbMaxPersonnesTaxi)) &&
               LitChamp(hfile, &NbMaxToursAttenteTaxi, sizeof(NbMaxToursAttenteTaxi)))) &&
             ((VersionFichier < 0x530) || // v5.3
              (LitChamp(hfile, &NbMaxPersonnesVehlib, sizeof(NbMaxPersonnesVehlib)) &&
               LitChamp(hfile, &NbMaxCasesDetectFileVehlib, sizeof(NbMaxCasesDetectFileVehlib)) &&
               LitChamp(hfile, &ProbaDirFileVehlib, sizeof(ProbaDirFileVehlib)) &&
               LitChamp(hfile, &TauxRemplissageVehlib, sizeof(TauxRemplissageVehlib))));
    }
    Ok = Ok && NombreCredible(NbFeux) && NombreCredible(NbFeuxPieton) && NombreCredible(NbParkings) && // v6.0.2
         NombreCredible(NbArretsBus) && NombreCredible(NbArretsTram) && NombreCredible(NbPlacesTaxi) &&
         NombreCredible(NbPlacesPark) && NombreCredible(NbPlacesVehlib);
    if (Ok && (NbFeux != 0))
    {
        Feu = std::vector<feu>(NbFeux);
        for (i = 0; Ok && (i < NbFeux); i++)
        {
            Ok = Feu[i].Charge(hfile);
            Feu[i].AffecteNumero(i + 1);
        }
    }
    if ((VersionFichier >= 0x520) && (Ok && (NbFeuxPieton != 0))) // v5.2
    {
        FeuP = std::vector<feu_pieton>(NbFeuxPieton);
        for (i = 0; Ok && (i < NbFeuxPieton); i++)
        {
            Ok = FeuP[i].Charge(hfile);
            FeuP[i].AffecteNumero(i + 1);
        }
    }

    if (Ok && (NbParkings != 0))
    {
        Parking = std::vector<parking>(NbParkings);
        for (i = 0; Ok && (i < NbParkings); i++)
        {
            Ok = Parking[i].Charge(hfile);
            Parking[i].AffecteNumero(i + 1);
            if (Parking[i].Defini())
            {
                NvNbParkings++; // v5.1 : on teste s'il manque des parkings souterrains par rapport à NbParkings
            }
        }
    }
    if (!Vehicule.empty())
    {
        Vehicule.clear();
    }
    if (!Vehlib.empty())
    {
        Vehlib.clear();
    } // v5.3
    if (!Pieton.empty())
    {
        Pieton.clear();
    }
    if (VersionFichier >= 0x300)           // v3.0
    {                                      // Dépot, arrêts et lignes de bus
        Ok = Ok && DepotBus.Charge(hfile); // v5.2 : Manquait Ok&&
        if (Ok && (NbArretsBus != 0))
        {
            ArretBus = std::vector<arret_bus>(NbArretsBus);
            for (i = 0; Ok && (i < NbArretsBus); i++)
            {
                Ok = ArretBus[i].Charge(hfile);
                ArretBus[i].AffecteNumero(i + 1);
            }
        }
        if (Ok)
        {
            for (i = 0; Ok && (i < NBMAXLIGNESBUSTRAM); i++)
            {
                Ok = LigneBus[i].Charge(hfile);
                LigneBus[i].AffecteNumero(i + 1);
            }
        }
    }
    if (VersionFichier >= 0x350) // v3.5
    {                            // Arrêts et lignes de tram
        if (Ok && (NbArretsTram != 0))
        {
            ArretTram = std::vector<arret_tram>(NbArretsTram);
            for (i = 0; Ok && (i < NbArretsTram); i++)
            {
                Ok = ArretTram[i].Charge(hfile);
                ArretTram[i].AffecteNumero(i + 1);
            }
        }
        if (Ok)
        {
            for (i = 0; Ok && (i < NBMAXLIGNESBUSTRAM); i++)
            {
                Ok = LigneTram[i].Charge(hfile);
                LigneTram[i].AffecteNumero(i + 1);
            }
        }
    }

    if ((VersionFichier >= 0x360) && (Ok && (NbPlacesTaxi != 0))) // v3.6
                                                                  // Places de
                                                                  // Taxis
    {
        PlaceTaxi = std::vector<place_taxi>(NbPlacesTaxi);
        for (i = 0; Ok && (i < NbPlacesTaxi); i++)
        {
            Ok = PlaceTaxi[i].Charge(hfile);
            PlaceTaxi[i].AffecteNumero(i + 1);
        }
    }

    if ((VersionFichier >= 0x500) && (Ok && (NbPlacesPark != 0))) // v5.0
                                                                  // Places de
                                                                  // parkings
    {
        PlacePark = std::vector<place_park>(NbPlacesPark);
        for (i = 0; Ok && (i < NbPlacesPark); i++)
        {
            Ok = PlacePark[i].Charge(hfile);
            PlacePark[i].AffecteNumero(i + 1);
            if (PlacePark[i].Definie())
            {
                NvNbPlacesPark++; // v5.1 : on teste s'il manque des places de parkings par rapport à NbPlacesPark
            }
        }
    }

    if ((VersionFichier >= 0x530) && (Ok && (NbPlacesVehlib != 0))) // v5.3

    // Places de vehlib
    {
        PlaceVehlib = std::vector<place_vehlib>(NbPlacesVehlib);
        for (i = 0; Ok && (i < NbPlacesVehlib); i++)
        {
            Ok = PlaceVehlib[i].Charge(hfile);
            PlaceVehlib[i].AffecteNumero(i + 1);
            if (PlaceVehlib[i].Definie())
            {
                NvNbPlacesVehlib++; // on teste s'il manque des places de vehlib par rapport à NbPlacesVehlib
            }
        }
    }

    for (i = 0; Ok && (i < NbX); i++)
    {
        for (j = 0; Ok && (j < NbY); j++)
        {
            Ok = v[i][j].Charge(hfile);
        }
    }

    // v5.1 : on retire les parkings indéfinis
    if (Ok && (NvNbParkings < NbParkings))
    {
        std::vector<parking> NvParking(NvNbParkings);
        {
            NvNbParkings = 0;
            for (i = 0; i < NbParkings; i++)
            {
                if (Parking[i].Defini())
                {
                    NvParking[NvNbParkings].Copie(&Parking[i]);              // NvNbParkings++
                    NvParking[NvNbParkings].AffecteNumero(NvNbParkings + 1); // On renumérote
                    cv->v[NvParking[NvNbParkings].x][NvParking[NvNbParkings].y].NumParking = NvNbParkings + 1;
                    NvNbParkings++;
                }
            }
            NbParkings = NvNbParkings;
            Parking.clear();
            Parking = std::move(NvParking);
        }
    }

    // v5.1 : on retire les places de parkings indéfinies
    if (Ok && (VersionFichier >= 0x500) && (NvNbPlacesPark < NbPlacesPark))
    {
        std::vector<place_park> NvPlacePark(NvNbPlacesPark);
        {
            NvNbPlacesPark = 0;
            for (i = 0; i < NbPlacesPark; i++)
            {
                if (PlacePark[i].Definie())
                {
                    NvPlacePark[NvNbPlacesPark].Copie(&PlacePark[i]);
                    NvPlacePark[NvNbPlacesPark].AffecteNumero(NvNbPlacesPark + 1); // On renumérote
                    cv->v[NvPlacePark[NvNbPlacesPark].x][NvPlacePark[NvNbPlacesPark].y].NumPlacePark =
                        NvNbPlacesPark + 1;
                    NvNbPlacesPark++;
                }
            }
            NbPlacesPark = NvNbPlacesPark;
            PlacePark.clear();
            PlacePark = std::move(NvPlacePark);
        }
    }

    // v5.3 : on retire les places de vehlib indéfinies
    if (Ok && (VersionFichier >= 0x530) && (NvNbPlacesVehlib < NbPlacesVehlib))
    {
        std::vector<place_vehlib> NvPlaceVehlib(NvNbPlacesVehlib);
        {
            NvNbPlacesVehlib = 0;
            for (i = 0; i < NbPlacesVehlib; i++)
            {
                if (PlaceVehlib[i].Definie())
                {
                    NvPlaceVehlib[NvNbPlacesVehlib].Copie(&PlaceVehlib[i]);
                    NvPlaceVehlib[NvNbPlacesVehlib].AffecteNumero(NvNbPlacesVehlib + 1); // On renumérote
                    cv->v[NvPlaceVehlib[NvNbPlacesVehlib].x][NvPlaceVehlib[NvNbPlacesVehlib].y].NumPlaceVehlib =
                        NvNbPlacesVehlib + 1;
                    NvNbPlacesVehlib++;
                }
            }
            NbPlacesVehlib = NvNbPlacesVehlib;
            PlaceVehlib.clear();
            PlaceVehlib = std::move(NvPlaceVehlib);
        }
    }

    if (NbPietons == -1)
    {
        NbPietons = 0; // Si version sans piétons alors pas de piéton !
    }
    return Ok;
}
//-----------------------------------------------------------------------------
bool centre_ville::Sauve(int hfile)
{
    int i = 0;
    int j = 0;
    int VersionCourante = VERSION;
    bool Ok = false;
    Ok = (FileWrite(hfile, &VersionCourante, sizeof(VersionCourante)) != 0) && // v3.0
         (FileWrite(hfile, &NbFeux, sizeof(NbFeux)) != 0) &&
         (FileWrite(hfile, CopieEnCours ? &NbXc : &NbX, sizeof(NbX)) != 0) && // v4.0.1 (CopieEnCours)
         (FileWrite(hfile, CopieEnCours ? &NbYc : &NbY, sizeof(NbY)) != 0) && // v4.0.1 (CopieEnCours)
         (FileWrite(hfile, &NbFeuxPieton, sizeof(NbFeuxPieton)) != 0) &&      // v5.2
         (FileWrite(hfile, &NbParkings, sizeof(NbParkings)) != 0) &&
         (FileWrite(hfile, &NbVehicules, sizeof(NbVehicules)) != 0) &&
         (FileWrite(hfile, &NbPietons, sizeof(NbPietons)) != 0) &&           // v2.0
         (FileWrite(hfile, &NbArretsBus, sizeof(NbArretsBus)) != 0) &&       // v3.0
         (FileWrite(hfile, &NbLignesBus, sizeof(NbLignesBus)) != 0) &&       // v3.0
         (FileWrite(hfile, &NbArretsTram, sizeof(NbArretsTram)) != 0) &&     // v3.5
         (FileWrite(hfile, &NbLignesTram, sizeof(NbLignesTram)) != 0) &&     // v3.5
         (FileWrite(hfile, &NbPlacesTaxi, sizeof(NbPlacesTaxi)) != 0) &&     // v3.6
         (FileWrite(hfile, &NbTaxis, sizeof(NbTaxis)) != 0) &&               // v3.6
         (FileWrite(hfile, &NbPlacesPark, sizeof(NbPlacesPark)) != 0) &&     // v5.0
         (FileWrite(hfile, &NbPlacesVehlib, sizeof(NbPlacesVehlib)) != 0) && // v5.3
         (FileWrite(hfile, &NbVehlibs, sizeof(NbVehlibs)) != 0) &&           // v5.3
         (FileWrite(hfile, &DureeVert, sizeof(DureeVert)) != 0) &&
         (FileWrite(hfile, &DureeOrange, sizeof(DureeOrange)) != 0) &&
         (FileWrite(hfile, &DureeRouge, sizeof(DureeRouge)) != 0) &&
         (FileWrite(hfile, &AttenteMaxVeh, sizeof(AttenteMaxVeh)) != 0) &&
         (FileWrite(hfile, &NbToursParSeconde, sizeof(NbToursParSeconde)) != 0) &&
         (FileWrite(hfile, &NbToursStats, sizeof(NbToursStats)) != 0) &&
         (FileWrite(hfile, &QuadrillageStats, sizeof(QuadrillageStats)) != 0) &&
         (FileWrite(hfile, &TypeStats, sizeof(TypeStats)) != 0) &&
         (FileWrite(hfile, &NbVitesses, sizeof(NbVitesses)) != 0) &&
         (FileWrite(hfile, &NbMaxCasesDetectPsgPt, sizeof(NbMaxCasesDetectPsgPt)) != 0) &&
         (FileWrite(hfile, &ProbaDirPsgPt, sizeof(ProbaDirPsgPt)) != 0) &&
         (FileWrite(hfile, &NbMaxCasesDetectArretBus, sizeof(NbMaxCasesDetectArretBus)) != 0) &&     // v3.0
         (FileWrite(hfile, &ProbaDirArretBus, sizeof(ProbaDirArretBus)) != 0) &&                     // v3.0
         (FileWrite(hfile, &ProbaArretBusDmd, sizeof(ProbaArretBusDmd)) != 0) &&                     // v3.0
         (FileWrite(hfile, &NbMaxPersonnesBus, sizeof(NbMaxPersonnesBus)) != 0) &&                   // v3.0
         (FileWrite(hfile, &NbToursStopArretBus, sizeof(NbToursStopArretBus)) != 0) &&               // v3.0
         (FileWrite(hfile, &NbMaxToursAttenteBus, sizeof(NbMaxToursAttenteBus)) != 0) &&             // v3.0
         (FileWrite(hfile, &NbMaxCasesDetectArretTram, sizeof(NbMaxCasesDetectArretTram)) != 0) &&   // v3.5
         (FileWrite(hfile, &ProbaDirArretTram, sizeof(ProbaDirArretTram)) != 0) &&                   // v3.5
         (FileWrite(hfile, &ProbaArretTramDmd, sizeof(ProbaArretTramDmd)) != 0) &&                   // v3.5
         (FileWrite(hfile, &NbMaxPersonnesTram, sizeof(NbMaxPersonnesTram)) != 0) &&                 // v3.5
         (FileWrite(hfile, &NbToursStopArretTram, sizeof(NbToursStopArretTram)) != 0) &&             // v3.5
         (FileWrite(hfile, &NbMaxToursAttenteTram, sizeof(NbMaxToursAttenteTram)) != 0) &&           // v3.5
         (FileWrite(hfile, &NbMaxCasesDetectFileTaxi, sizeof(NbMaxCasesDetectFileTaxi)) != 0) &&     // v3.5
         (FileWrite(hfile, &ProbaDirFileTaxi, sizeof(ProbaDirFileTaxi)) != 0) &&                     // v3.5
         (FileWrite(hfile, &ProbaArretTaxiDmd, sizeof(ProbaArretTaxiDmd)) != 0) &&                   // v3.5
         (FileWrite(hfile, &NbMaxPersonnesTaxi, sizeof(NbMaxPersonnesTaxi)) != 0) &&                 // v3.5
         (FileWrite(hfile, &NbMaxToursAttenteTaxi, sizeof(NbMaxToursAttenteTaxi)) != 0) &&           // v3.5
         (FileWrite(hfile, &NbMaxPersonnesVehlib, sizeof(NbMaxPersonnesVehlib)) != 0) &&             // v5.3
         (FileWrite(hfile, &NbMaxCasesDetectFileVehlib, sizeof(NbMaxCasesDetectFileVehlib)) != 0) && // v5.3
         (FileWrite(hfile, &ProbaDirFileVehlib, sizeof(ProbaDirFileVehlib)) != 0) &&                 // v5.3
         (FileWrite(hfile, &TauxRemplissageVehlib, sizeof(TauxRemplissageVehlib)) != 0);             // v5.3

    for (i = 0; Ok && (i < NbFeux); i++)
    {
        Ok = Feu[i].Sauve(hfile);
    }
    for (i = 0; Ok && (i < NbFeuxPieton); i++)
    {
        Ok = FeuP[i].Sauve(hfile); // v5.2
    }
    for (i = 0; Ok && (i < NbParkings); i++)
    {
        Ok = Parking[i].Sauve(hfile);
    }
    Ok = Ok && DepotBus.Sauve(hfile); // v6.0.1 : Manquait Ok&& (comme dans Charge avant la v5.2)
    for (i = 0; Ok && (i < NbArretsBus); i++)
    {
        Ok = ArretBus[i].Sauve(hfile); // v3.0
    }
    for (i = 0; Ok && (i < NBMAXLIGNESBUSTRAM); i++)
    {
        Ok = LigneBus[i].Sauve(hfile); // v3.0. Attention : tableau statique
    }
    for (i = 0; Ok && (i < NbArretsTram); i++)
    {
        Ok = ArretTram[i].Sauve(hfile); // v3.5
    }
    for (i = 0; Ok && (i < NBMAXLIGNESBUSTRAM); i++)
    {
        Ok = LigneTram[i].Sauve(hfile); // v3.5. Attention : tableau statique
    }
    for (i = 0; Ok && (i < NbPlacesTaxi); i++)
    {
        Ok = PlaceTaxi[i].Sauve(hfile); // v3.6
    }
    for (i = 0; Ok && (i < NbPlacesPark); i++)
    {
        Ok = PlacePark[i].Sauve(hfile); // v5.0
    }
    for (i = 0; Ok && (i < NbPlacesVehlib); i++)
    {
        Ok = PlaceVehlib[i].Sauve(hfile); // v5.3
    }
    if (CopieEnCours)
    { // v4.0.1
        for (i = Xc; Ok && (i < Xc + NbXc); i++)
        {
            for (j = Yc; Ok && (j < Yc + NbYc); j++)
            {
                Ok = v[i][j].Sauve(hfile);
            }
        }
    }
    else
    {
        for (i = 0; Ok && (i < NbX); i++)
        {
            for (j = 0; Ok && (j < NbY); j++)
            {
                Ok = v[i][j].Sauve(hfile);
            }
        }
    }
    return Ok && FileFlush(hfile); // v6.0.1 : FileWrite renvoie -1 (vrai) en cas d'erreur et écrit par un tampon :
                                   // erreurs vues ici
}
//-----------------------------------------------------------------------------
void centre_ville::Verifie(AnsiString &asMsgErr)
{ // v5.0 : Calcul préalable pour le NbVehicules
    const int NbMinVehicules = (NbParkings * (NbParkings - 1)) + (2 * NbPlacesPark);
    int // v5.0 : 1 véh départ + 1 véh arrivée par place de parking
        const NbMaxVehicules =
            NbParkings < 2 ? 0 : // v6.0.3 : division par zéro dans l'original (signalé plus bas)
                ((NBMAXVEHICULES / (NbParkings * (NbParkings - 1))) * NbParkings * (NbParkings - 1)) +
                    (2 * NbPlacesPark); // v5.0 : 1 véh départ + 1 véh arrivée par place de parking
    int i = 0;
    int j = 0;
    asMsgErr = "";
    if (NbFeux < 0)
    {
        asMsgErr += Format("Le nombre de feux est incorrect (%d)\n", TableauDeConstantes(NbFeux));
    }
    if (NbParkings < 2)
    {
        asMsgErr += Format("Le nombre de parkings est incorrect (%d)\n", TableauDeConstantes(NbParkings));
    }
    if (NbVehicules <= 0)
    {
        asMsgErr += Format("Le nombre de véhicules est incorrect (%d)\n", TableauDeConstantes(NbVehicules));
    }
    else if ((NbParkings == 0) || (NbParkings == 1) || ((NbVehicules % NbMinVehicules) != 0) || // Incrément incorrect
             (NbVehicules < NbMinVehicules) || (NbVehicules > NbMaxVehicules))
    {
        asMsgErr += Format("Le nombre de véhicules ne permet pas une simulation équilibrée (%d)\n",
                           TableauDeConstantes(NbVehicules));
    }
    if (NbPietons < 0)
    {
        asMsgErr += Format("Le nombre de piétons est incorrect (%d)\n", TableauDeConstantes(NbPietons));
    }
    if (DureeVert < 0)
    { // v6.0.7 : DureeVert, l'original affichait DureeOrange
        asMsgErr += Format("La durée du feu vert par défaut est incorrecte (%d)\n", TableauDeConstantes(DureeVert));
    }
    if (DureeOrange < 0)
    {
        asMsgErr += Format("La durée du feu orange est incorrecte (%d)\n", TableauDeConstantes(DureeOrange));
    }
    if (DureeRouge < 0)
    { // v6.0.7 : DureeRouge, l'original affichait DureeOrange
        asMsgErr += Format("La durée du feu rouge par défaut est incorrecte (%d)\n", TableauDeConstantes(DureeRouge));
    }
    if (AttenteMaxVeh < 0)
    {
        asMsgErr += AnsiString("La durée d'attente maximale d'un véhicule est incorrecte\n");
    }
    if (NbToursParSeconde < 0)
    {
        asMsgErr += AnsiString("Le nombre de tours par seconde est incorrect\n");
    }
    if (NbToursStats < 0)
    {
        asMsgErr += Format("Le nombre de tours soumis aux statistiques est incorrect (%d)\n",
                           TableauDeConstantes(NbToursStats));
    }
    if ((TypeStats < par_nombre) || (TypeStats > par_pourcentage))
    {
        asMsgErr +=
            Format("Le type de statistique est incorrect (%d)\n", TableauDeConstantes(static_cast<int>(TypeStats)));
    }
    if ((NbVitesses < 1) || (NbVitesses > 3))
    {
        asMsgErr += Format("Le nombre de vitesses est incorrect (%d)\n", TableauDeConstantes(NbVitesses));
    }
    if ((NbMaxCasesDetectPsgPt < 0) || (NbMaxCasesDetectPsgPt > 5))
    {
        asMsgErr += Format("Le nombre de case maximum pour la détection de passage piéton est incorrect (%d)\n",
                           TableauDeConstantes(NbMaxCasesDetectPsgPt));
    }
    if ((ProbaDirPsgPt < 0) || (ProbaDirPsgPt > 100))
    {
        asMsgErr += Format("La probabilité qu'un piéton se dirige vers un passage piéton détecté est incorrecte (%d)\n",
                           TableauDeConstantes(ProbaDirPsgPt));
    }
    if (NbArretsBus < 0)
    {
        asMsgErr += Format("Le nombre d'arrêts de bus est incorrect (%d)\n", TableauDeConstantes(NbArretsBus));
    }
    if (NbLignesBus < 0)
    {
        asMsgErr += Format("Le nombre de lignes de bus est incorrect (%d)\n", TableauDeConstantes(NbLignesBus));
    }
    if (NbArretsTram < 0)
    {
        asMsgErr += Format("Le nombre d'arrêts de tram est incorrect (%d)\n", TableauDeConstantes(NbArretsTram));
    }
    if (NbLignesTram < 0)
    {
        asMsgErr += Format("Le nombre de lignes de tram est incorrect (%d)\n", TableauDeConstantes(NbLignesTram));
    }
    if (NbPlacesTaxi < 0)
    {
        asMsgErr += Format("Le nombre de places de taxi est incorrect (%d)\n", TableauDeConstantes(NbPlacesTaxi));
    }
    if (NbTaxis < 0)
    {
        asMsgErr += Format("Le nombre de taxis est incorrect (%d)\n", TableauDeConstantes(NbTaxis));
    }
    if (NbPlacesVehlib < 0)
    { // v5.3
        asMsgErr += Format("Le nombre de places de vehlib est incorrect (%d)\n", TableauDeConstantes(NbPlacesVehlib));
    }
    if (NbVehlibs < 0)
    { // v5.3
        asMsgErr += Format("Le nombre de vehlibs est incorrect (%d)\n", TableauDeConstantes(NbVehlibs));
    }
    for (i = 0; i < NbX; i++)
    {
        for (j = 0; j < NbY; j++)
        {
            v[i][j].Verifie(asMsgErr);
        }
    }
    for (i = 0; i < NbFeux; i++)
    {
        if (Feu[i].Defini())
        { // v6.0.1 : un feu supprimé laisse une place libre (NumeroFeuLibre), qui n'est pas une erreur
            Feu[i].Verifie(asMsgErr, Format("Feu n°%d", TableauDeConstantes(i + 1)));
        }
    }
    for (i = 0; i < NbParkings; i++)
    {
        Parking[i].Verifie(asMsgErr);
    }
    if (!Vehicule.empty())
    {
        for (i = 0; i < NbVehicules; i++)
        {
            Vehicule[i].Verifie(asMsgErr);
        }
    }
    if (!Pieton.empty())
    {
        for (i = 0; i < NbPietons; i++)
        {
            Pieton[i].Verifie(asMsgErr);
        }
    }
    if (!Bus.empty())
    { // v3.0.4 (un oubli)
        for (i = 0; i < NbBus; i++)
        {
            Bus[i].Verifie(asMsgErr);
        }
    }
    if (!ArretBus.empty())
    { // v3.0
        for (i = 0; i < NbArretsBus; i++)
        {
            ArretBus[i].Verifie(asMsgErr);
        }
    }
    for (i = 0; i < NBMAXLIGNESBUSTRAM; i++)
    { // v3.0
        LigneBus[i].Verifie(asMsgErr);
    }
    if (!Tram.empty())
    { // v3.5
        for (i = 0; i < NbTrams; i++)
        {
            Tram[i].Verifie(asMsgErr);
        }
    }
    if (!ArretTram.empty())
    { // v3.5
        for (i = 0; i < NbArretsTram; i++)
        {
            ArretTram[i].Verifie(asMsgErr);
        }
    }
    for (i = 0; i < NBMAXLIGNESBUSTRAM; i++)
    { // v3.5
        LigneTram[i].Verifie(asMsgErr);
    }
    if (!Taxi.empty())
    { // v5.0 (oubli)
        for (i = 0; i < NbTaxis; i++)
        {
            Taxi[i].Verifie(asMsgErr);
        }
    }
    if (!PlaceTaxi.empty())
    {
        for (i = 0; i < NbPlacesTaxi; i++)
        {
            PlaceTaxi[i].Verifie(asMsgErr);
        }
    }
    for (i = 0; i < NbPlacesPark; i++)
    { // v5.0
        PlacePark[i].Verifie(asMsgErr);
    }
    if (!Vehlib.empty())
    { // v5.3
        for (i = 0; i < NbVehlibs; i++)
        {
            Vehlib[i].Verifie(asMsgErr);
        }
    }
    if (!PlaceVehlib.empty())
    { // v5.3
        for (i = 0; i < NbPlacesVehlib; i++)
        {
            PlaceVehlib[i].Verifie(asMsgErr);
        }
    }
    DepotBus.Verifie(asMsgErr); // v3.03
}
//-----------------------------------------------------------------------------
inline bool centre_ville::TourDeDeplacementVehicule(int NumeroVehicule) // v3.0.4
{
    if ((TourCrt - Vehicule[NumeroVehicule].TourDepart) % NbVitesses <=
        ((Vehicule[NumeroVehicule].NivPriorite != 0) ? NbVitesses - 1 : NumeroVehicule % NbVitesses))
    {
        return true;
    }

    Vehicule[NumeroVehicule].NbToursRepos++; // v2.2.4
    return false;
}
//-----------------------------------------------------------------------------
inline bool centre_ville::TourDeDeplacementVehlib(int NumeroVehlib) // v5.3
{
    if ((TourCrt - Vehlib[NumeroVehlib].TourDepart) % NbVitesses <= NbVitesses - 1)
    {
        return true;
    }

    Vehlib[NumeroVehlib].NbToursRepos++;
    return false;
}
//-----------------------------------------------------------------------------
inline bool centre_ville::TourDeDeplacementBus(int NumeroBus) // v3.0
{
    if (((TourCrt - Bus[NumeroBus].TourDepart) % NbVitesses) == 0)
    {
        return true;
    }

    Bus[NumeroBus].NbToursRepos++;
    return false;
}
//-----------------------------------------------------------------------------
inline bool centre_ville::TourDeDeplacementTram(int NumeroTram) // v3.5
{
    if ((TourCrt % NbVitesses) == 0)
    {
        return true;
    }

    Tram[NumeroTram].NbToursRepos++;
    return false;
}
//-----------------------------------------------------------------------------
inline bool centre_ville::TourDeDeplacementTaxi(int NumeroTaxi) // v3.6
{
    if ((TourCrt % NbVitesses) == 0)
    {
        return true;
    }

    Taxi[NumeroTaxi].NbToursRepos++;
    return false;
}
//-----------------------------------------------------------------------------
bool centre_ville::CopieValide() // v4.2.1
{
    AnsiString asMsg;
    for (int i = 0; i < NbParkings; i++)
    {
        if (Parking[i].Defini() && (!DansSelectionEnCours(&Parking[i])))
        {
            asMsg = asMsg + "le parking n°" + IntToStr(i + 1) + " " + asCoordonnees(&Parking[i]) + ", ";
        }
    }
    if (DepotBus.Defini() && (!DansSelectionEnCours(&DepotBus)))
    {
        asMsg = asMsg + "le dépôt de bus n°" + asCoordonnees(&DepotBus) + ", ";
    }
    for (int i = 0; i < NbFeux; i++)
    {
        if (Feu[i].Defini() && (!DansSelectionEnCours(&Feu[i])))
        {
            asMsg = asMsg + "le feu n°" + IntToStr(i + 1) + " " + asCoordonnees(&Feu[i]) + ", ";
        }
    }
    for (int i = 0; i < NbArretsBus; i++)
    {
        if (ArretBus[i].Defini() && (!DansSelectionEnCours(&ArretBus[i])))
        {
            asMsg = asMsg + "l'arrêt de bus n°" + IntToStr(i + 1) + " " + asCoordonnees(&ArretBus[i]) + ", ";
        }
    }
    for (int i = 0; i < NbPlacesTaxi; i++)
    {
        if (PlaceTaxi[i].Definie() && (!DansSelectionEnCours(&PlaceTaxi[i])))
        {
            asMsg = asMsg + "la place de taxi n°" + IntToStr(i + 1) + " " + asCoordonnees(&PlaceTaxi[i]) + ", ";
        }
    }
    for (int i = 0; i < NbPlacesPark; i++)
    { // v5.0
        if (PlacePark[i].Definie() && (!DansSelectionEnCours(&PlacePark[i])))
        {
            asMsg = asMsg + "la place de parking n°" + IntToStr(i + 1) + " " + asCoordonnees(&PlacePark[i]) + ", ";
        }
    }
    for (int i = 0; i < NbPlacesVehlib; i++)
    { // v5.3
        if (PlaceVehlib[i].Definie() && (!DansSelectionEnCours(&PlaceVehlib[i])))
        {
            asMsg = asMsg + "la place de vehlib n°" + IntToStr(i + 1) + " " + asCoordonnees(&PlaceVehlib[i]) + ", ";
        }
    }
    if (!asMsg.IsEmpty())
    {
        vue->AfficheMessage(
            "Les éléments suivants DOIVENT ÊTRE dans la sélection :\n" + asMsg +
                "\n\nVeuillez soit les supprimer, soit les inclure dans la sélection avant de réessayer.",
            "Commande de copie impossible", tmAvertissement, 0, 0); // portage
        return false;
    }

    return true;
}
//-----------------------------------------------------------------------------
void centre_ville::Genere(int x, int y, int d) // v4.2
{                                              // ATTENTION : d peut changer à l'intérieur de Genere(...)
    int k = 0;
    bool Ok = true;
    std::array<bool, NBDIR + 1> DejaFait = {false, false, false, false, false};
    bool AutreDirection = false;
    do
    { /* LE CODE EN COMMENTAIRE sert pour le débogage
     frmSimulation->DrawGridSimul->Col=x;
     frmSimulation->DrawGridSimul->Row=y;
     frmSimulation->RedessineCase(x,y,true); */
        const int l =
            1 + ((LgrMinSgmtRoute * ((((d - 1) % 2) != 0) ? NbX : NbY)) / 10) + // Longueur du nouveau segment de route
            (alea::Rand() % (1 + (((LgrMaxSgmtRoute - LgrMinSgmtRoute) * ((((d - 1) % 2) != 0) ? NbX : NbY)) / 10)));
        // AfficheMessage("Départ du segment","("+IntToStr(x)+","+IntToStr(y)+"),"+asDir[d], tmInformation,0,0);
        bool DirOpposee = false;
        bool EntreeIntersection = true;
        const auto TireEntreeIntersection = [&] { // ProbaEntreeIntersection chances sur 10
            EntreeIntersection = (ProbaEntreeIntersection > alea::Rand() % 10);
            return EntreeIntersection;
        };
        const auto VoitDirOpposee = [&](const voie *vc) {
            DirOpposee = ((vc->DirPoss & InvPossDir[d]) != 0);
            return DirOpposee;
        };
        for (k = 0, Ok = true, AutreDirection = false; Ok && (k <= l); k++)
        {
            voie *vc = &Voisine(x, y, k, d);                      // Case suivante
            Ok = ((/*MemeDir=*/vc->DirPoss & PossDir[d]) == 0) && // S'il s'agit d'une direction possible = d
                 (!VoitDirOpposee(vc)) &&                         // OU une direction opposée à d
                 ((vc->DirPoss == 0) ||
                  /*TraverseeRoute=*/(ProbaTraverseeRoute > alea::Rand() % 10) || // OU une traversée de route refusée,
                                                                                  // ALORS on NE TRACE pas le segment de
                                                                                  // route sur la case.
                  TireEntreeIntersection());                                      // OU une entrée d'intersection ?
            if (Ok)
            { // v5.4 : Conversion BCB6>BCBX pas de a+=b sur énumérés
                vc->DirPoss =
                    static_cast<dirposs>((static_cast<int>(vc->DirPoss)) + (static_cast<int>(PossDir[d]))); // On trace
                                                                                                            // la route
                                                                                                            // jusqu'à
                                                                                                            // la case
                                                                                                            // courante
                // frmSimulation->RedessineCase((x+k*dx[d])%NbX, (y+k*dy[d])%NbY, true);
                // AfficheMessage("Tracé de
                // segment","("+IntToStr((x+k*dx[d])%NbX)+","+IntToStr((y+k*dy[d])%NbY)+"),"+IntToStr(k)+"/"+IntToStr(l)+","+asDir[d],
                // tmInformation,0,0);
            }
        }
        if (!Ok) // Le segment N'a PAS été tracé jusqu'au bout
        {
            k--;                       // On revient sur la case précédente TRACÉE
            if (DirOpposee ||          // SI direction OPPOSEE,
                (!EntreeIntersection)) // OU SI arrivée sur une route refusée
            {                          // ALORS on part A GAUCHE OU A DROITE (on évite les deux problèmes)
                // AfficheMessage("DirOpposée||(!EntreeIntersection)","("+IntToStr((x+k*dx[d])%NbX)+","+IntToStr((y+k*dy[d])%NbY)+"),Pas
                // Ok,"+IntToStr(k)+"/"+IntToStr(l)+","+asDir[d], tmInformation,0,0);
                const int dd = 1 + ((d + (2 * (alea::Rand() % 2))) % NBDIR);
                if (k == 0)
                {
                    DejaFait[dd] = true;
                }
                else
                {
                    for (bool &i : DejaFait)
                    {
                        i = false;
                    }
                }
                if ((k != 0) || (!DejaFait[dd])) // On teste si on n'a pas déjà fait cette direction sur la même case...
                {
                    x = (x + (k * dx[d])) % NbX;
                    y = (y + (k * dy[d])) % NbY;
                    d = dd;
                    AutreDirection = true;
                }
            }
        /*    else
               AfficheMessage("MêmeDir||(!TraverséeRoute)","("+IntToStr((x+k*dx[d])%NbX)+","+IntToStr((y+k*dy[d])%NbY)+"),Pas
           Ok,"+IntToStr(k)+"/"+IntToStr(l)+","+asDir[d], tmInformation,0,0);
        */   }
        else // Le segment a été tracé jusqu'au bout : MAIS ATTENTION V(x,y,k,d) N'EST PAS TRACÉ !!!
        {
            if ((Voisine(x, y, k, d).DirPoss & (NESO - InvPossDir[d])) != NESO - InvPossDir[d]) // peut-on continuer à
                                                                                                // créer des segments ?
            {
                if (ProbaSortieIntersection > alea::Rand() % 10) // Crée-t-on une sortie d'intersection ?
                {
                    // AfficheMessage("SortieIntersection","("+IntToStr((x+k*dx[d])%NbX)+","+IntToStr((y+k*dy[d])%NbY)+"),Ok,"+IntToStr(k)+"/"+IntToStr(l)+","+asDir[d],
                    // tmInformation,0,0);
                    int dd = 0;
                    do
                    {
                        dd = 1 + (alea::Rand() % NBDIR);
                    } while (((Voisine(x, y, k, d).DirPoss & PossDir[dd]) != 0) || (abs(d - dd) == 2));
                    Genere((x + (k * dx[d])) % NbX, (y + (k * dy[d])) % NbY, dd);
                    // frmSimulation->DrawGridSimul->Col=x;
                    // frmSimulation->DrawGridSimul->Row=y;
                    // frmSimulation->RedessineCase(x,y,true);
                    // AfficheMessage("RetourCreationNvBranche","("+IntToStr((x+k*dx[d])%NbX)+","+IntToStr((y+k*dy[d])%NbY)+"),Ok,"+IntToStr(k)+"/"+IntToStr(l)+","+asDir[d],
                    // tmInformation,0,0);
                }
                int dd = 0;
                // AfficheMessage("PoursuiteSegment","("+IntToStr((x+k*dx[d])%NbX)+","+IntToStr((y+k*dy[d])%NbY)+"),Ok,"+IntToStr(k)+"/"+IntToStr(l)+","+asDir[d],
                // tmInformation,0,0);
                do
                {
                    dd = 1 + (alea::Rand() % NBDIR);
                } while (((Voisine(x, y, k, d).DirPoss & PossDir[dd]) != 0) || (abs(d - dd) == 2));
                x = (x + (k * dx[d])) % NbX;
                y = (y + (k * dy[d])) % NbY;
                d = dd;
                AutreDirection = true;
            }
        }
    } while (AutreDirection);
}
//-----------------------------------------------------------------------------
