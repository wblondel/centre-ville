//---------------------------------------------------------------------------
#ifndef simulH
#define simulH
//---------------------------------------------------------------------------
// Directives de compilation
//---------------------------------------------------------------------------
// #pragma warn -pia // Inhibe "Possibly Incorrect Assignment" (v5.2) // portage : pragma C++Builder neutralisé
//---------------------------------------------------------------------------
#include <array>
#include <vector>
#include "compat/chaine.h" // portage : remplace <dstring.h> (AnsiString, Format, IntToStr)
#include "compat/temps.h"  // portage : remplace <systdate.h> (TDateTime, Time)
//---------------------------------------------------------------------------
constexpr int NBXDEF = 99;  // v4.0 (NBX->NBXDEF)
constexpr int NBYDEF = 63;  // v4.0 (NBY->NBYDEF)
constexpr int NBXMIN = 8;   // v4.0
constexpr int NBYMIN = 8;   // v4.0
constexpr int NBXMAX = 200; // v6.0.3 : nouvelle zone seulement (mémoire, et pile du calcul récursif des distances)
constexpr int NBYMAX = 200; // v6.0.3
//---------------------------------------------------------------------------
constexpr int NBDIR = 4;
constexpr int NBFEUXDEFAUT = 2;       // v5.1 : anciennement 30
constexpr int NBFEUXPIETONDEFAUT = 2; // v5.2
constexpr int NBETAPESDEFAUT = 2;     // v5.1 : anciennement 4
constexpr int NBVEHICULESDEFAUT = 120;
constexpr int NBPIETONSDEFAUT = 30;  // v2.0 (25% des véhicules = 120/4 = 30)
constexpr int NBNIVEAUXPRIORITE = 3; // v2.2
constexpr int NBMAXETAPES = 32;
constexpr int NBMAXVEHICULES = 9999;                                                // v5.0.3 - Au lieu de 1000
constexpr int NBMAXPIETONSPARCASE = 2;                                              // v2.0
constexpr int NBMAXLIGNESBUSTRAM = 9;                                               // v3.5
constexpr int NBMAXARRETSBUSTRAMPARLIGNE = 9;                                       // v3.5
constexpr int NBMAXBUSTRAMPARLIGNE = 3;                                             // v3.5
constexpr int NBMAXARRETSBUSTRAM = NBMAXLIGNESBUSTRAM * NBMAXARRETSBUSTRAMPARLIGNE; // v3.5
constexpr int NBMAXBUSTRAM = NBMAXLIGNESBUSTRAM * NBMAXBUSTRAMPARLIGNE;             // v3.5
constexpr int NBSENSVOIETRAMBASE = 7;                                               // v3.5
constexpr int NBSENSVOIETRAM = 64;        // v3.5 (2^NBSENSVOIETRAMBASE-1=2^6=64)
constexpr int NBTYPESVOIE = 2;            // v3.5
constexpr int NBSITUATIONSTAXI = 4;       // v3.6
constexpr int NBFEUXPARCASE = 4;          // v5.2
constexpr int NBFEUXPIETONSPARNUMERO = 2; // v5.2
//---------------------------------------------------------------------------
// valeurs de distance particulières
constexpr int DIST_INDEFINIE = 32767;
//---------------------------------------------------------------------------
// Filtre pour ne donner que la nature (sans les décors) dans la propriété Nature (poids fort) :
// NatureSansDecor(), après la classe voie
//---------------------------------------------------------------------------
// 3ème index de voie::DistParking[][][]
constexpr int SANSPSGBUS = 0;
constexpr int AVECPSGBUS = 1;
//---------------------------------------------------------------------------
// Index de RotPossDir
constexpr int R90D = 1;
constexpr int R180 = 2;
constexpr int R90G = 3;
//---------------------------------------------------------------------------
constexpr int VERSION = 0x530;    // v3.0. La version change SSI le format de fichiers change (v5.3)
constexpr int VERSIONMIN = 0x300; // v3.0. Version minimale stockée : NE PAS MODIFIER
constexpr int VERSIONANC = 0x200; // v3.0. Version non stockée compatible : NE PAS MODIFIER
//--------------------------------------------------------------------------- Voie
// Case voisine dans la direction d, ou à n cases : centre_ville::Voisine() (6.0.17 : anciennes macros V, _V, V_, _V_)
//--------------------------------------------------------------------------- Feux Piétons (v5.2)
// voie::FeuxPietons() (6.0.17 : anciennes macros FEUXPIETONS et FeuxPietons())
//---------------------------------------------------------------------------
// #define min(x,y) (((x)<(y))?(x):(y)) // portage : inutilisée par le moteur, retirée (conflit avec la STL)
// #define max(x,y) (((x)>(y))?(x):(y)) // portage : inutilisée par le moteur, retirée (conflit avec la STL)
//---------------------------------------------------------------------------
// Pour centre_ville::PrepareCopie(...)
constexpr bool DEBUT = true;
constexpr bool FIN = false;
//---------------------------------------------------------------------------
// v5.4.1 : Pour infection::DonneTourDeb(bool Dernier) et DonneNumPieton(bool Dernier)
constexpr bool PREMIER = false;
constexpr bool DERNIER = true;
//---------------------------------------------------------------------------
// Version 1.5 : Ajout de la vitesse
//  - Ajout de la propriété NbVitesses dans la classe centre_ville.
//  - Il peut y avoir de 1 à 3 vitesses différentes : 1 case/tour /2 tours ou /3 tours.

// Version 2.0 : Ajout des piétons
//  - Les piétons se déplacent en dehors des voies (endroits noirs) ou sur les passages piétons.
//  - classe pieton
//  - propriété PassagePieton, FeuOpposePietons dans la classe voie.

// Version 2.2 : Ajout des véhicules d'urgence et de police
//  - type enuméré niveau_priorite
//  - propriété niveau_priorite NivPriorite dans la classe vehicule

// Version 3.0 : Ajout des bus
//  - Classes arret_bus, ligne_bus, bus
//  - Propriétés voie::NumArretBus, centre_ville::ArretBus, centre_ville::LigneBus

// Version 3.1 : Activation des couloirs de bus
//  - Utilisation du bouton et de la rubrique Couloir Bus
//  - Utilisation de la propriété PassageBus de la classe voie

// Version 3.5 : Ajout des trams
//  - Classes arret_tram, ligne_tram, tram
//  - Propriétés voie::NumArretTram, centre_ville::ArretTram, centre_ville::LigneTram

// Version 3.6 : Ajout des taxis
//  - Classes place_taxi, file_taxi, taxi
//  - Propriétés voie::NumProchTaxi, voie::NumPlaceTaxi, centre_ville::PlaceTaxi, centre_ville::FileTaxi,
//  centre_ville::Taxi

// Version 3.8 : Zones délimitées en vue des décors
//  - Zones de circulation, Zones habitables, Zones piétonnes

// Version 3.9 : Premiers décors suivant les zones
//  - toitures, trottoirs, pelouses, arbres

// Version 4.0 : Dimensions de la zone modifiable

// Version 4.2 : Génération aléatoire d'un réseau (voies automobiles seulement)

// version 5.0 : - Naissance des places de parking. Les parkings deviennent des parkings (souterrains)
//               - nouvelles classes place_park, file_park

// version 5.1 : Disparition dans les paramètres du nombre de feux, parkings, arrêts de bus/tram, places de
// taxis/parkings.
//                 -> Ajout méthode copie et incrémentation automatique transparente.

// version 5.3 : Ajout éléments vehlib, place_vehlib
// version 5.4 : Version 32 et 64 bits
// version 5.4.1 : Ajout de la notion d'épidémie (spécial Covid-19)
//----------------------------------------------------------------------------------------------------------------------
// Types énumérés. /!\ v5.4 : le type char a été forcé pour que l'énuméré ne prenne pas la taille d'un entier (4 octets)
// // portage : signed char, car char est non signé sur certaines plateformes (Linux ARM64) ; tailles vérifiées dans
// format_cvw.h
//----------------------------------------------------------------------------------------------------------------------
enum priorite : signed char { a_droite, cedez_le_passage, stop, feux }; // portage : signed char
enum direction : signed char { indefinie, nord, est, sud, ouest };      // portage : signed char
enum sens : signed char { indefini, horizontal, vertical }; // v2.0. Pour les passages piétons // portage : signed char
enum dirposs : signed char {
    aucune,
    N,
    E,
    NE,
    S,
    NS,
    SE,
    NES,
    O,
    NO,
    EO,
    NEO,
    SO,
    NSO,
    ESO,
    NESO
}; // portage : signed char
enum diraprendre : signed char { gauche, devant, droite };                             // portage : signed char
enum etat_feu : signed char { inactif, vert, orange, rouge };                          // portage : signed char
enum dmd_vert : signed char { arrivee_vehicule, immediate };                           // portage : signed char
enum psg_rouge : signed char { prm_dmd, immediat };                                    // portage : signed char
enum type_stats : signed char { par_nombre, par_pourcentage };                         // portage : signed char
enum compatibilite : signed char { version_inf_16, version_inf_20, version_courante }; // v2.0 // portage : signed char
enum niveau_priorite : signed char { normal, urgence, police };                        // v2.2 // portage : signed char
enum position_arret_bus_tram : signed char {
    tete,
    milieu,
    queue
}; // v3.5 (ajout de "_tram" au nom du type car communs aux deux types d'arrêt) // portage : signed char
enum sens_voie_tram : signed char {
    aucune_voie = 0,
    nord_est = 1,
    nord_sud = 2,
    nord_ouest = 4,
    sud_est = 8,
    est_ouest = 16,
    sud_ouest = 32
}; // v3.5 // portage : signed char
enum sens_voie_tram_base : signed char { aucune_voie_base, ne, ns, no, se, eo, so }; // portage : signed char
enum type_distance : signed char {
    type_indefini,
    parkings,
    arrets_bus,
    arrets_tram,
    files_taxi,
    files_park,
    files_vehlib
}; // v3.5. v3.6 : Ajout de files_taxi. v5.0 : Ajout de files_park. v5.3 : Ajout de files_vehlib // portage : signed
   // char
enum type_voie : signed char {
    tvAucun,
    tvRoute,
    tvVoieTram,
    tvTout
}; // pour la suppression des routes et voies de tram. v3.5 // portage : signed char
enum situation_taxi : signed char { stAttente, stDepart, stDepot, stRetour }; // v3.6 // portage : signed char
enum situation_vehlib : signed char { svAttente, svDepart };                  // v5.3 // portage : signed char
enum nature {
    voies = 0,
    trottoirs = 0x10000,
    arbre = 0x10010,
    pelouse = 0x10020, // Décors zone piétonne (v3.9)
    immeubles = 0x20000,
};
enum coin : signed char {
    aucun_coin = 0,
    haut_droite = 1,
    bas_droite = 2,
    bas_gauche = 4,
    haut_gauche = 8,
    quatre_coins = 15
}; // v3.8 // portage : signed char
//---------------------------------------------------------------------------
class centre_ville;
//---------------------------------------------------------------------------
class infection // v5.4.1
{
    int TourDeb, TourFin,           // Contagieux si TourDeb < cv->TourCrt < TourFin
        TourFatal,                  // Si > 0 alors Mort programmée à TourFatal + cv->EpidemieInfectiosite
        NumPrmPieton, NumDrnPieton, // Pas de patient source car patient zéro donc... vaut zéro !
        PrmRang, DrnRang,           // 0 = patient zéro.
        ChargeVirale;               // Est incrémenté à chaque (ré)infection dans la période contagieuse
public:
    infection(const infection &) = delete;
    infection &operator=(const infection &) = delete;
    friend class ExportEtat; // portage : lecture de l'état complet (tests du moteur)
    infection *Svt;
    explicit infection(const centre_ville *cv); // Patient zéro
    infection(int Tour, int NumPietonOrigine, int Rang);
    ~infection() { delete Svt; }
    [[nodiscard]] bool EstContagieux(int Tour) const
    {
        if ((Tour > TourDeb) && (Tour < TourFin))
        {
            return true;
        }
        if (Svt)
        {
            return Svt->EstContagieux(Tour);
        }

        return false;
    }
    bool Reinfecte(int Tour, int NumPieton, int Rang); // comme le constructeur mais pour une personne qui a déjà été
                                                       // infectée
    [[nodiscard]] int DonneRang(int Tour) const
    {
        if ((Tour > TourDeb) && (Tour < TourFin))
        {
            return DrnRang; // v5.4.2 : Au lieu de PrmRang
        }
        if (Svt)
        {
            return Svt->DonneRang(Tour);
        }

        return -1; // Non contagieux au moment du tour
    }
    [[nodiscard]] int DonneChargeVirale(int Tour) const
    {
        if ((Tour > TourDeb) && (Tour < TourFin))
        {
            return ChargeVirale;
        }
        if (Svt)
        {
            return Svt->DonneChargeVirale(Tour);
        }

        return 0;
    }
    [[nodiscard]] bool DoitSuccomber(int Tour) const;
    [[nodiscard]] int NbInfections() const
    {
        if (Svt)
        {
            return 1 + Svt->NbInfections();
        }
        return 1;
    }
    [[nodiscard]] int DonnePrmRang() const { return PrmRang; } // v5.4.2 : au lieu de DonneDrnRang()
    [[nodiscard]] int DonneTourFin() const
    {
        if (Svt)
        {
            return Svt->DonneTourFin();
        }
        return TourFin;
    }
    [[nodiscard]] int DonneTourDeb(bool Dernier) const
    {
        if (Dernier && Svt)
        {
            return Svt->DonneTourDeb(true);
        }
        return TourDeb;
    }
    [[nodiscard]] bool EstCondamne() const
    {
        if (Svt)
        {
            return Svt->EstCondamne();
        }
        return (TourFatal > 0);
    }
};
//---------------------------------------------------------------------------
class pieton // v2.0
{
    int Numero;

public:
    pieton(const pieton &) = delete;
    pieton &operator=(const pieton &) = delete;
    int x{}, y{}, Dir{}, DemiCase{}, NumArretBus{}, // v3.0. Arrêt de bus d'attente (si >0). Le piéton se trouve à côté
                                                    // de cet arrêt (à droite de la tête de l'arrêt de bus)
        NumArretTram{}, // v3.5. Arrêt de tram d'attente (si >0). Le piéton se trouve à côté de cet arrêt (à droite de
                        // la tête de l'arrêt de tram)
        NumFileTaxi{}, // v3.6. File de taxi d'attente (si >0). Le piéton se trouve à côté de cette file (à droite de la
                       // tête de file de taxi)
        NumPlaceVehlib{}, // v5.3. Place vehlib d'attente (si >0). Le piéton se trouve à côté de cette place avant de
                          // monter dans le vehlib
        NumBus{},         // v3.0. Bus dans lequel se trouve le piéton (si >0. Dans ce cas, x=-1 et y=-1)
        NumTram{},        // v3.5. Tram dans lequel se trouve le piéton (si >0. Dans ce cas, x=-1 et y=-1)
        NumTaxi{},        // v3.6. Tram dans lequel se trouve le piéton (si >0. Dans ce cas, x=-1 et y=-1)
        NumVehlib{},      // v5.3/ Vehlib dans lequel se trouve le piéton (si >0. Dans ce cas, x=-1 et y=-1)
        NumFeuP{},        // v5.2. Feu piéton au pied duquel le piéton attend pour traverser (>0 si AttendFeuP)
        TourDrnDepl{}, TourDrnAff{}, TourAttBus{}, // v3.0. Tour à partir duquel le piéton attend le bus
        TourAttTram{},                             // v3.5. Tour à partir duquel le piéton attend le tram
        TourAttTaxi{},                             // v3.6. Tour à partir duquel le piéton attend le taxi
        DemiCasesParcourues{};                     // v3.0. L'unité de longueur piétonne est la demi-case, pas la case !
    bool PsgPtTrv{}, ArretBusTrv{},                // v3.0. Arrêt de bus trouvé
        AttendBus{},                               // v3.0. Attend un bus à l'arret NumArretBus
        ArretTramTrv{},                            // v3.5. Arrêt de tram trouvé
        AttendTram{},                              // v3.5. Attend un tram à l'arret NumArretTram
        AttendFeuP{},     // v5.2. Attend que le feu piéton NumFeuP l'autorise à traverser la voie (tram/route).
        FileTaxiTrv{},    // v3.5. File de taxi trouvée
        PlaceVehlibTrv{}, // v5.3. Place de vehlib trouvée
        AttendTaxi{},     // v3.5. Attend un taxi à la file NumFileTaxi
        AttendVehlib{},   // v5.3. Attend un vehlib à la file NumFileVehlib
        VeutDescendre{},  // v3.0. Indique qu'il veut descendre du bus, du tram ou du taxi
        Ecrase{}, // v4.3. Indique si ce piéton a été écrasé : Si c'est le cas les coordonnées sont l'endroit du crime
        Mort{};   // v5.4.1. Indique si ce piéton est victime de l'épidémie
    infection *Infection{}; // v5.4.1
    void Initialise()
    {
        x = y = -1; // v5.4.1 (xm, ym)
        Dir = indefinie;
        NumVehlib = NumPlaceVehlib = NumFeuP = NumTaxi = NumTram = NumBus = NumFileTaxi = NumArretTram = NumArretBus =
            DemiCase = 0; // v3.6 (NumTaxi, NumFileTaxi), v5.2 (NumFeuP), v5.3 (vehlib)
        DemiCasesParcourues = TourDrnAff = TourDrnDepl = 0L;
        TourAttTaxi = TourAttTram = TourAttBus = 0L; // v6.0.1 : indéterminés tant que le piéton n'a rien attendu
        Mort = Ecrase = VeutDescendre = AttendFeuP = AttendVehlib = AttendTaxi = AttendTram = AttendBus =
            PlaceVehlibTrv = FileTaxiTrv = ArretTramTrv = ArretBusTrv = PsgPtTrv = false; // v4.3 (Ecrase), v5.2
                                                                                          // (AttendFeuP), v5.3
                                                                                          // (vehlib), v5.4.1 (Mort)
        Infection = nullptr;
    }
    pieton()
    {
        Numero = 0;
        Initialise();
    }
    void AffecteNumero(int Num) { Numero = Num; }
    void Place(int x0, int y0)
    {
        x = x0;
        y = y0;
        DemiCase = 0;
    }
    void Definit(int x0, int y0, direction Dir0, int DemiCase0)
    {
        x = x0;
        y = y0;
        Dir = Dir0;
        DemiCase = DemiCase0;
        TourDrnAff = TourDrnDepl = 0L;
        PsgPtTrv = false;
    }
    void AnnuleRechercheEnCours()
    {
        if (PsgPtTrv)
        {
            PsgPtTrv = false;
        }
        if (ArretBusTrv)
        {
            ArretBusTrv = false;
        }
        if (ArretTramTrv)
        {
            ArretTramTrv = false;
        }
        if (FileTaxiTrv)
        {
            FileTaxiTrv = false; // v3.6
        }
        if (PlaceVehlibTrv)
        {
            PlaceVehlibTrv = false; // v5.3
        }
    }
    ~pieton()
    {
        delete Infection; // v5.4.1
    }
    void ChercheFileTaxi(); // v3.6
    [[nodiscard]] bool Defini() const { return (x > -1) && (y > -1); }
    void Verifie(AnsiString &asMsgErr) const;
    void MonteDansBus(int b)
    {
        NumBus = b;
        AttendBus = false;
        x = -1;
        y = -1;
    } // v3.6
    void MonteDansTram(int t)
    {
        NumTram = t;
        AttendTram = false;
        x = -1;
        y = -1;
    } // v3.6
    void MonteDansTaxi(int t)
    {
        NumTaxi = t;
        AttendTaxi = false;
        NumFileTaxi = 0;
        x = -1;
        y = -1;
    } // v3.6
    void MonteDansVehlib(int v)
    {
        NumVehlib = v;
        AttendVehlib = false;
        NumPlaceVehlib = 0;
        x = -1;
        y = -1;
    } // v5.3
    void SeFaitEcraser()
    {
        Ecrase = true;
        PsgPtTrv = ArretBusTrv = ArretTramTrv = PlaceVehlibTrv = FileTaxiTrv = AttendVehlib = AttendBus = AttendTram =
            AttendTaxi = VeutDescendre = false;
    } // v4.3 (création) v5.3 (vehlib)
    // v5.4.1 : gestion de l'épidémie
    void SuccombeInfection()
    {
        Mort = true;
        PsgPtTrv = ArretBusTrv = ArretTramTrv = PlaceVehlibTrv = FileTaxiTrv = AttendVehlib = AttendBus = AttendTram =
            AttendTaxi = VeutDescendre = false;
    } // v5.4.1
    [[nodiscard]] bool EstVivant() const { return (!Ecrase) && (!Mort); }
    bool EstContagieux();
    void EstContaminePar(int NumPieton, int Rang); // v5.4.1. Utilisation :
                                                   // UnPieton->EstContaminePar(PietonContagieux->Numero+1,
                                                   // PietonContagieux->DonneRang()+1)
    [[nodiscard]] int DonneRang() const;
    [[nodiscard]] int DonneChargeVirale() const;
    [[nodiscard]] bool DoitSuccomber() const;
    [[nodiscard]] int NbInfections() const
    {
        if (Infection)
        {
            return Infection->NbInfections();
        }
        return 0;
    }
    [[nodiscard]] int DonnePrmRang() const
    {
        if (Infection)
        {
            return Infection->DonnePrmRang();
        }
        return -1;
    }
    [[nodiscard]] int DonneTourFin() const
    {
        if (Infection)
        {
            return Infection->DonneTourFin();
        }
        return -1;
    }
    [[nodiscard]] int DonneTourDeb(bool Dernier) const
    {
        if (Infection)
        {
            return Infection->DonneTourDeb(Dernier);
        }
        return -1;
    }
    [[nodiscard]] bool EstCondamne() const
    {
        if (Infection)
        {
            return Infection->EstCondamne();
        }
        return false;
    }
    void SupprimeInfection()
    {
        if (Infection)
        {
            delete Infection;
            Infection = nullptr;
            Mort = false;
        }
    } // Utilisé par la résurrection !
    bool EstGueri() { return (!EstContagieux()) && (NbInfections() > 0); } // v5.4.2
};
//---------------------------------------------------------------------------
class feu
{
    int Numero; // v5.1 : devenu public
public:
    feu(const feu &) = delete;
    feu &operator=(const feu &) = delete;
    int x{}, y{}, Dir{}; // Orientation. Mais il peut y avoir max. 2 feux par case;
    etat_feu Etat{}, ProchainEtat{};
    int NbFeuxDmd{}, // Nbre de feux demandant le rouge
        NbFeuxAtt{}, // Nbre de feux devant être rouge pour que celui-ci (this) passe au vert
        NbFeuxPtn{}, // Nbre de feux piétons liés (v5.2). ATTENTION: défini au lancement de la simulation car le lien
                     // est établi depuis les feux de piétons.
        DureeRouge{}, DureeVert{}, TourDrnChgEtat{}; // Tour du dernier changement d'état
    std::vector<int> NumFeuxDmd,                     // Liste des feux demandant le rouge (NbFeuxDmd premiers)
        NumFeuxAtt, // Liste des feux devant être rouge pour que celui-ci (this) passe au vert (NbFeuxAtt premiers)
        NumFeuxPtn; // Liste des feux de piétons (v5.2), NbFeuxPtn premiers. ATTENTION: définie au lancement de la
                    // simulation car le lien est établi depuis les feux de piétons.
    dmd_vert TypeDmdVert{};   // Type de demande du vert : dès l'arrivée d'un vehicule ou dès le passage au rouge
    psg_rouge TypePsgRouge{}; // Type de passage au rouge : dès la première demande ou à l'unanimité des feux demandeurs
    bool DemandeVert{},       // Indique qu'il demande le rouge (à ceux qui ont Numero dans FeuxDmd[])
        FeuxTousRouges{},     // Indique que les feux (FeuxAtt) sont tous rouges
        RougeDemande{},       // Indique qu'un feu demande le rouge
        ProchainVert{},       // Indique qu'il est autorisé à passer au vert
        VerifEnCours{};       // Drapeau pour Verifie()
    void Initialise()
    {
        x = -1;
        y = -1;
        Dir = indefinie;
        ProchainEtat = Etat = inactif;
        TypeDmdVert = arrivee_vehicule;
        TypePsgRouge = prm_dmd;
        DureeVert = DureeRouge = NbFeuxPtn = NbFeuxAtt = NbFeuxDmd = 0;
        TourDrnChgEtat = 0L;
        ProchainVert = RougeDemande = FeuxTousRouges = DemandeVert = false;
        VerifEnCours = false;
    }
    feu()
    {
        Numero = 0;
        Initialise();
    }
    void Copie(feu *f) // v5.1 (f est ensuite détruit : ses listes lui sont prises)
    {
        Numero = f->Numero;
        x = f->x;
        y = f->y;
        Dir = f->Dir;
        Etat = f->Etat;
        ProchainEtat = f->ProchainEtat;
        NbFeuxDmd = f->NbFeuxDmd;
        NbFeuxAtt = f->NbFeuxAtt;
        NbFeuxPtn = f->NbFeuxPtn; // v5.2
        NumFeuxDmd = std::move(f->NumFeuxDmd);
        NumFeuxAtt = std::move(f->NumFeuxAtt);
        NumFeuxPtn = std::move(f->NumFeuxPtn); // v5.2
        DureeRouge = f->DureeRouge;
        DureeVert = f->DureeVert;
        TourDrnChgEtat = f->TourDrnChgEtat;
        TypeDmdVert = f->TypeDmdVert;
        TypePsgRouge = f->TypePsgRouge;
        DemandeVert = f->DemandeVert;
        FeuxTousRouges = f->FeuxTousRouges;
        RougeDemande = f->RougeDemande;
        ProchainVert = f->ProchainVert;
        VerifEnCours = f->VerifEnCours;
    }
    void AffecteNumero(int Num) { Numero = Num; }
    void Definit(int x0, int y0, int Dir0)
    {
        x = x0;
        y = y0;
        Dir = Dir0;
        Etat = inactif;
    }
    [[nodiscard]] bool Defini() const { return (x > -1) && (y > -1) && (Dir > aucune); }
    void AjouteFeuDmd(int NumNvFeu);
    void AjouteFeuAtt(int NumNvFeu);
    void AjouteFeuPtn(int NumNvFeu); // v5.2
    void RetireFeuDmd(int NumFeu);
    void RetireFeuAtt(int NumFeu);
    void ChangeEtat();
    void DefinitProchainEtat();
    void Evolue();
    bool Charge(int hfile);
    bool Sauve(int hfile);
    void Verifie(AnsiString &asMsgErr, const AnsiString &asSource);
};
//---------------------------------------------------------------------------
class feu_pieton // v5.2
{
    int Numero;

public:
    feu_pieton(const feu_pieton &) = delete;
    feu_pieton &operator=(const feu_pieton &) = delete;
    std::array<int, NBFEUXPIETONSPARNUMERO> x{};
    std::array<int, NBFEUXPIETONSPARNUMERO> y{};
    std::array<int, NBFEUXPIETONSPARNUMERO> Dir{}; // Deux feux pitons sont positionnés de part et d'autre d'un passage
                                                   // piétons. NumFeuDir
    std::array<int, NBDIR> iFeuDir{}; // (Arch, qu'est-ce qu'y feut dire ?) ...Permet d'avoir directement les
                                      // coordonnées du feu en fonction de la direction Attention à l'accès :
                                      // iFeuDir[(direction)d-1]
    etat_feu Etat{}, ProchainEtat{};
    int NbFeux{},                                    // Nbre de feux de circulation liés
        DureeRouge{}, DureeVert{}, DureeTraversee{}, // Soit 1+2*Cases Passage piéton. Défini quand le feu opposé est
                                                     // placé
        TourDrnChgEtat{};                            // Tour du dernier changement d'état
    std::vector<int> NumFeux;                        // Liste des feux de circulation liés (NbFeux premiers)
    dmd_vert TypeDmdVert{};   // Type de demande du vert : dès l'arrivée d'un vehicule ou dès le passage au rouge
    psg_rouge TypePsgRouge{}; // Type de passage au rouge : dès la première demande ou à l'unanimité des feux demandeurs
    bool DemandeVert{},       // Indique qu'il demande le rouge (à ceux qui ont Numero dans NumFeuxPtn[])
        FeuxTousRouges{},     // Indique que les feux (NumFeux) sont tous rouges
        RougeDemande{},       // Indique qu'un feu demande le rouge
        ProchainVert{},       // Indique qu'il est autorisé à passer au vert
        VerifEnCours{};       // Drapeau pour Verifie()
    void Initialise();
    feu_pieton()
    {
        Numero = 0;
        Initialise();
    }
    void Copie(feu_pieton *f); // f est ensuite détruit : sa liste lui est prise
    void AffecteNumero(int Num) { Numero = Num; }
    void Definit(int x0, int y0, int Dir0)
    {
        x[0] = x0;
        y[0] = y0;
        Dir[0] = Dir0;
        Etat = inactif;
    }
    void DefinitFeuOppose();
    [[nodiscard]] bool Defini() const
    {
        return (x[0] > -1) && (y[0] > -1) && (Dir[0] > aucune) && (x[1] > -1) && (y[1] > -1) && (Dir[1] > aucune);
    }
    void AjouteFeu(int NumNvFeu);
    void RetireFeu(int NumFeu);
    void ChangeEtat();
    void DefinitProchainEtat();
    bool PietonAttend();
    void Evolue();
    bool Charge(int hfile);
    bool Sauve(int hfile);
    void Verifie(AnsiString &asMsgErr, AnsiString asSource);
};
//---------------------------------------------------------------------------
class parking
{
    int Numero;

public:
    int x{}, y{};
    void Initialise()
    {
        x = -1;
        y = -1;
    }
    parking()
    {
        Numero = 0;
        Initialise();
    }
    void Copie(parking *p) // v5.1
    {
        Numero = p->Numero;
        x = p->x;
        y = p->y;
    }
    void AffecteNumero(int Num) { Numero = Num; }
    void Definit(int x0, int y0)
    {
        x = x0;
        y = y0;
    }
    ~parking() = default;
    [[nodiscard]] bool Defini() const { return (x > -1) && (y > -1); }
    void PlaceProchainVehicule();
    bool Charge(int hfile);
    bool Sauve(int hfile);
    void Verifie(AnsiString &asMsgErr) const;
};
//---------------------------------------------------------------------------
class voie; // v5.0 : utilisée en paramètre d'une méthode de vehicule
//---------------------------------------------------------------------------
class vehicule // instances créées au lancement de la simulation
{
    int Numero;

public:
    int x{}, y{};
    direction Dir{}, DirDrnDepl{};
    niveau_priorite NivPriorite{};                 // v2.2
    int NumParkingDepart{}, NumParkingArrivee{},   // Index-1 de centre_ville::Parking[]
        NumFileParkDepart{}, NumFileParkArrivee{}, // v5.0. NumParking{Depart|Arrivee} exclusif avec
                                                   // NumFilePark{Depart|Arrivee}. Index-1 de centre_ville.FilePark[]
        TourDrnDepl{},                             // Tour du dernier déplacement effectué
        TourDepart{},                              // Tour pdt lequel il a été placé sur le parking de départ
        NbCasesParcourues{},                       // Nombre de cases parcourues
        NbToursRepos{}; // v2.2.4 : Tours pendant lesquels le véhicule ne s'est pas déplacé du fait de la vitesse
                        // réduite
    void Initialise()
    {
        x = -1;
        y = -1;
        DirDrnDepl = Dir = indefinie;
        NivPriorite = normal;
        NbToursRepos /*v2.2.4*/ = NbCasesParcourues = TourDepart = TourDrnDepl = 0L;
        NumParkingDepart = NumParkingArrivee = 0;
        NumFileParkDepart = NumFileParkArrivee = 0; // v5.0
    }
    vehicule()
    {
        Numero = 0;
        Initialise();
    }
    void AffecteNumero(int Num) { Numero = Num; }
    void Place(int x0, int y0)
    {
        x = x0;
        y = y0;
    }
    void Definit(int x0, int y0, direction Dir0, int NumDepart, int NumArrivee, int NumFileDepart,
                 int NumFileArrivee,            // v5.0
                 niveau_priorite NivPriorite0); // v2.2
    ~vehicule() = default;
    [[nodiscard]] bool Defini() const { return (x > -1) && (y > -1); }
    bool EstDestination(const voie *v) const; // v5.0 : indique si v est la case destination (parking ou place de
                                              // parking)
    void CalculeProchaineDirection();
    [[nodiscard]] direction ProchaineDirectionAuCarrefour() const;
    [[nodiscard]] bool PeutPasser() const;
    void Verifie(AnsiString &asMsgErr) const;
};
//---------------------------------------------------------------------------
// Classes de GESTION DES BUS
//---------------------------------------------------------------------------
class depot_bus // v3.0 ATTENTION : il n'y en a qu'un !
{
public:
    int x{}, y{}, NumDrnBusSorti{};
    void Initialise()
    {
        x = -1;
        y = -1;
        NumDrnBusSorti = 0;
    }
    depot_bus() { Initialise(); }
    void Definit(int x0, int y0)
    {
        x = x0;
        y = y0;
        NumDrnBusSorti = 0;
    }
    ~depot_bus() = default;
    [[nodiscard]] bool Defini() const { return (x > -1) && (y > -1); }
    void PlaceProchainBus();
    bool Charge(int hfile);
    bool Sauve(int hfile);
    void Verifie(AnsiString &asMsgErr) const;
};
//---------------------------------------------------------------------------
class bus // v3.0 instances créées au lancement de la simulation
{
    int Numero;

public:
    direction Dir{}, DirDrnDepl{}, DirAvtDrnDepl{};
    int x{}, y{}, NumLigne{}, NumDrnArret{}, NumArretSvt{}, // Index-1 de centre_ville::ArretBus
        NbPersonnes{},        // Correspond aux nombre de Pieton ayant NumBus = Numero (piétons dans le bus)
        TourDrnDepl{},        // Tour du dernier déplacement effectué
        TourDepart{},         // Tour pdt lequel il a été placé sur la case du dépot
        TourArriveDrnArret{}, // Tour lorsque le bus est arrêté au dernier arrêt
        TourDepartArretSvt{}, // Tour pour le départ vers l'arrêt suivant
        NbCasesParcourues{},  // Nombre de cases parcourues
        NbCasesParcouruesDepuisDrnArret{}, // Idem mais remis à 0 à chaque arrêt de bus
        NbToursRepos{}; // Tours pendant lesquels le bus ne s'est pas déplacé du fait de la vitesse réduite
    void Initialise()
    {
        x = -1;
        y = -1;
        DirAvtDrnDepl = DirDrnDepl = Dir = indefinie;
        NbPersonnes = NbToursRepos = NbCasesParcouruesDepuisDrnArret = NbCasesParcourues = TourArriveDrnArret =
            TourDepartArretSvt = TourDepart = TourDrnDepl = 0L;
        NumDrnArret = NumArretSvt = 0;
    }
    bus()
    {
        Numero = 0;
        Initialise();
    }
    void AffecteNumero(int Num) { Numero = Num; }
    void Place(int x0, int y0)
    {
        x = x0;
        y = y0;
    }
    void Definit(int NumLigne0, int NumArretSvt0)
    {
        NumLigne = NumLigne0;
        NumArretSvt = NumArretSvt0;
    }
    ~bus() = default;
    [[nodiscard]] bool Defini() const { return (x > -1) && (y > -1); }
    void FaitMonterPieton(int NumPietonAFaireMonter);
    void FaitMonterEtDescendre(); // v3.0
    void CalculeProchaineDirection();
    [[nodiscard]] direction ProchaineDirectionAuCarrefour() const;
    [[nodiscard]] bool PeutPasser() const;
    [[nodiscard]] int NumeroArretSuivant() const;
    void Verifie(AnsiString &asMsgErr) const;
};
//---------------------------------------------------------------------------
class arret_bus // v3.0
{
    int Numero;

public:
    int x{}, y{}; // Position de la tête de l'arrêt de bus (indiquant les couleurs de lignes)
    std::array<bool, NBMAXLIGNESBUSTRAM> Ligne{}; // Indique si une ligne de bus passe par cet arrêt (Index-1)
    int NbLignes{};
    std::array<int, NBMAXLIGNESBUSTRAM> NumLigne{}; // Liste des lignes de bus passant par cet arrêt (Index-1)
    void Initialise();
    arret_bus()
    {
        Numero = 0;
        Initialise();
    }
    void Copie(arret_bus *a); // v5.1
    void AffecteNumero(int Num) { Numero = Num; }
    void Definit(int x0, int y0)
    {
        x = x0;
        y = y0;
    }
    ~arret_bus() = default;
    [[nodiscard]] bool Defini() const { return (x > -1) && (y > -1); }
    void InitialiseInfosLignesBus();
    void AjouteLigneBus(int n);
    AnsiString asListeLignesBus(); // v3.0.5
    bool Charge(int hfile);
    bool Sauve(int hfile);
    void Verifie(AnsiString &asMsgErr);
};
//---------------------------------------------------------------------------
class ligne_bus // v3.0
{
    int Numero; // Numéro de la ligne (CoulLigneBus[Numero]=Couleur de la ligne)
public:
    int NbArrets{};                                         // Nb arrêts de la ligne
    int NbBus{};                                            // Nb bus de la ligne
    std::array<int, NBMAXARRETSBUSTRAMPARLIGNE> NumArret{}; // Ordre des arrêts à effectuer
    std::array<int, NBMAXBUSTRAMPARLIGNE> NumBus{};         // N° des bus de la ligne
    void Initialise();
    ligne_bus()
    {
        Numero = 0;
        Initialise();
    }
    void AffecteNumero(int Num) { Numero = Num; }
    void AjouteBus(int NumeroBus);
    ~ligne_bus() = default;
    bool Charge(int hfile);
    bool Sauve(int hfile);
    void Verifie(AnsiString &asMsgErr);
};
//---------------------------------------------------------------------------
// Classes de GESTION DES TRAMS
//---------------------------------------------------------------------------
class tram // v3.5 instances créées au lancement de la simulation
{
    int Numero;

public:
    int x{}, y{}, NumLigne{};
    direction Dir{}, DirDrnDepl{}, DirAvtDrnDepl{};
    int NumDrnArret{}, NumArretSvt{}, // Index-1 de centre_ville::ArretTram
        NbPersonnes{},                // Correspond aux nombre de Pieton ayant NumTram = Numero (piétons dans le tram)
        TourDrnDepl{},                // Tour du dernier déplacement effectué
        TourArriveDrnArret{},         // Tour lorsque le tram est arrêté au dernier arrêt
        TourDepartArretSvt{},         // Tour pour le départ vers l'arrêt suivant
        NbCasesParcourues{},          // Nombre de cases parcourues
        NbCasesParcouruesDepuisDrnArret{}, // Idem mais remis à 0 à chaque arrêt de tram
        NbToursRepos{}; // Tours pendant lesquels le tram ne s'est pas déplacé du fait de la vitesse réduite
    void Initialise()
    {
        x = -1;
        y = -1;
        DirAvtDrnDepl = DirDrnDepl = Dir = indefinie;
        NbPersonnes = NbToursRepos = NbCasesParcouruesDepuisDrnArret = NbCasesParcourues = TourArriveDrnArret =
            TourDepartArretSvt = TourDrnDepl = 0L;
        NumDrnArret = NumArretSvt = 0;
    }
    tram()
    {
        Numero = 0;
        Initialise();
    }
    void AffecteNumero(int Num) { Numero = Num; }
    void Place(int x0, int y0)
    {
        x = x0;
        y = y0;
    }
    void Definit(int NumLigne0, int NumArretSvt0)
    {
        NumLigne = NumLigne0;
        NumArretSvt = NumArretSvt0;
    }
    ~tram() = default;
    [[nodiscard]] bool Defini() const { return (x > -1) && (y > -1); }
    void FaitMonterPieton(int NumPietonAFaireMonter);
    void FaitMonterEtDescendre();
    void CalculeProchaineDirection();
    [[nodiscard]] direction ProchaineDirectionAuCarrefour() const;
    [[nodiscard]] bool PeutPasser() const;
    [[nodiscard]] int NumeroArretSuivant() const;
    void Verifie(AnsiString &asMsgErr) const;
};
//---------------------------------------------------------------------------
class arret_tram // v3.5
{
    int Numero;

public:
    int x{}, y{}; // Position de la tête de l'arrêt de tram (indiquant les couleurs de lignes)
    std::array<bool, NBMAXLIGNESBUSTRAM> Ligne{}; // Indique si une ligne de tram passe par cet arrêt (Index-1)
    int NbLignes{};
    std::array<int, NBMAXLIGNESBUSTRAM> NumLigne{}; // Liste des lignes de tram passant par cet arrêt (Index-1)
    void Initialise();
    arret_tram()
    {
        Numero = 0;
        Initialise();
    }
    void Copie(arret_tram *a); // v5.1
    void AffecteNumero(int Num) { Numero = Num; }
    void Definit(int x0, int y0)
    {
        x = x0;
        y = y0;
    }
    ~arret_tram() = default;
    [[nodiscard]] bool Defini() const { return (x > -1) && (y > -1); }
    void InitialiseInfosLignesTram();
    void AjouteLigneTram(int n);
    AnsiString asListeLignesTram();
    bool Charge(int hfile);
    bool Sauve(int hfile);
    void Verifie(AnsiString &asMsgErr);
};
//---------------------------------------------------------------------------
class ligne_tram // v3.5
{
    int Numero; // Numéro de la ligne (CoulLigneTram[Numero]=Couleur de la ligne)
public:
    int NbArrets{};                                         // Nb arrêts de la ligne
    int NbTrams{};                                          // Nb tram de la ligne
    std::array<int, NBMAXARRETSBUSTRAMPARLIGNE> NumArret{}; // Ordre des arrêts à effectuer
    std::array<int, NBMAXBUSTRAMPARLIGNE> NumTram{};        // N° des tram de la ligne
    void Initialise();
    ligne_tram()
    {
        Numero = 0;
        Initialise();
    }
    void AffecteNumero(int Num) { Numero = Num; }
    void AjouteTram(int NumeroTram);
    ~ligne_tram() = default;
    bool Charge(int hfile);
    bool Sauve(int hfile);
    void Verifie(AnsiString &asMsgErr);
};
//---------------------------------------------------------------------------
// Classes de GESTION DES TAXIS
//---------------------------------------------------------------------------
class place_taxi // v3.6
{
    int Numero;

public:
    int x{}, y{}, NumFile{}; // 0 <= NumFile < NbFilesTaxi
    void Initialise()
    {
        x = -1;
        y = -1;
        NumFile = -1;
    }
    place_taxi()
    {
        Numero = 0;
        Initialise();
    }
    void Copie(place_taxi *p)
    {
        Numero = p->Numero;
        x = p->x;
        y = p->y;
        NumFile = p->NumFile;
    }
    void AffecteNumero(int Num) { Numero = Num; }
    [[nodiscard]] bool Definie() const { return (x > -1) && (y > -1); }
    void Definit(int x0, int y0)
    {
        x = x0;
        y = y0;
    }
    void DefinitFile(int n) { NumFile = n; }
    ~place_taxi() = default;
    bool Charge(int hfile);
    bool Sauve(int hfile);
    void Verifie(AnsiString &asMsgErr);
    [[nodiscard]] bool EstQueueFile() const;             // v3.6
    [[nodiscard]] bool EstTeteFile() const;              // v3.6
    [[nodiscard]] int PlaceQueueFile();                  // Retourne 1+NumPlaceTaxi si elle existe, 0 sinon.
    [[nodiscard]] int PlaceTeteFile();                   // Retourne 1+NumPlaceTaxi si elle existe, 0 sinon.
    [[nodiscard]] direction DirPlaceSvtMemeFile() const; // Retourne la direction vers laquelle il y a une place de la
                                                         // même file
};
//---------------------------------------------------------------------------
class file_taxi // v3.6 instances créées au lancement de la simulation
{
    int Numero;

public:
    int NumPlaceQueue; // 0 <= NumPlaceQueue < NbPlacesTaxi
    file_taxi()
    {
        Numero = 0;
        NumPlaceQueue = -1;
    }
    void AffecteNumero(int Num) { Numero = Num; }
    void Definit(int NumPlace) { NumPlaceQueue = NumPlace; }
    ~file_taxi() = default;
};
//---------------------------------------------------------------------------
class taxi // v3.6 instances créées au lancement de la simulation
{
    int Numero;

public:
    situation_taxi Situation{};
    int x{}, y{}, NumFile{},                       // 0 <= NumFile < NbTaxis
        NbPersonnes{}, NbCourses{}, TourDrnDepl{}, // Tour du dernier déplacement effectué
        TourDepartDrnCourse{},                     // Tour pour le départ d'une course
        TourDepotDrnCourse{},                      // Tour lors duquel le ou les derniers clients ont été déposés
        TourRetourDrnCourse{},                     // Tour du retour à la file (pour attente nouvelle course)
        NbCasesParcourues{},                       // Nombre de cases parcourues
        NbCasesParcouruesDrnCourse{},              // Idem mais remis à 0 à chaque départ d'une course
        NbToursRepos{}; // Tours pendant lesquels le bus ne s'est pas déplacé du fait de la vitesse réduite
    direction Dir{}, DirDrnDepl{};
    void Initialise()
    {
        NumFile = x = y = -1;
        DirDrnDepl = Dir = indefinie;
        NbCourses = NbPersonnes = 0;
        Situation = stAttente;
        TourDrnDepl = TourDepartDrnCourse = TourDepotDrnCourse = TourRetourDrnCourse = NbCasesParcourues =
            NbCasesParcouruesDrnCourse = NbToursRepos = 0;
    }
    taxi()
    {
        Numero = 0;
        Initialise();
    }
    ~taxi() = default;
    void AffecteNumero(int Num) { Numero = Num; }
    void Definit(int f) { NumFile = f; }                               // 0 <= f < NbFilesTaxi
    [[nodiscard]] bool Defini() const { return (x > -1) && (y > -1); } // v6.0.2 (Rechercher)
    void Place(int x0, int y0)
    {
        x = x0;
        y = y0;
    }
    void Place(int p); // 0 <= p < NbPlacesTaxi
    void CalculeProchaineDirection();
    [[nodiscard]] direction ProchaineDirectionAuCarrefour() const;
    [[nodiscard]] bool PeutPasser() const;
    void DefinitEtPlace(int f, int p)
    {
        Definit(f);
        Place(p);
    }
    void FaitDescendrePietons();
    void FaitMonterPieton(int NumPietonAFaireMonter);
    int NbPietonsQuiSontMontes(); // retourne true si des piétons attendaient et sont montés
    void Verifie(AnsiString &asMsgErr) const;
};
//---------------------------------------------------------------------------
class place_park // v5.0
{
    int Numero;

public:
    int x{}, y{}, NumFile{}; // 0 <= NumFile < NbFilesPark. -1 si non défini
    void Initialise()
    {
        x = -1;
        y = -1;
        NumFile = -1;
    }
    place_park()
    {
        Numero = 0;
        Initialise();
    }
    void Copie(place_park *p)
    {
        Numero = p->Numero;
        x = p->x;
        y = p->y;
        NumFile = p->NumFile;
    }
    void AffecteNumero(int Num) { Numero = Num; }
    [[nodiscard]] bool Definie() const { return (x > -1) && (y > -1); }
    void Definit(int x0, int y0)
    {
        x = x0;
        y = y0;
    }
    void DefinitFile(int n) { NumFile = n; }
    ~place_park() = default;
    bool Charge(int hfile);
    bool Sauve(int hfile);
    void Verifie(AnsiString &asMsgErr) const;
    [[nodiscard]] bool EstQueueFile() const; // v3.6
    bool EstTeteFile();                      // v3.6
    [[nodiscard]] int PlaceQueueFile();      // Retourne 1+NumPlace si elle existe, 0 sinon.
    int PlaceTeteFile();                     // Retourne 1+NumPlace si elle existe, 0 sinon.
};
//---------------------------------------------------------------------------
class file_park // v5.0 instances créées au lancement de la simulation
{
    int Numero;

public:
    int NumPlaceQueue; // 0 <= NumPlaceQueue < NbPlacesPark. -1 : Place non encore définie
    file_park()
    {
        Numero = 0;
        NumPlaceQueue = -1;
    }
    void AffecteNumero(int Num) { Numero = Num; }
    void Definit(int NumPlace) { NumPlaceQueue = NumPlace; }
    ~file_park() = default;
};
//---------------------------------------------------------------------------
class place_vehlib // v5.3
{
    int Numero;

public:
    int x{}, y{};
    int NumVehlibReserve{}; // si réservé alors > 0 et c'est le n° vehlib (+1) qui a réservé, sinon 0
    void Initialise()
    {
        x = -1;
        y = -1;
        NumVehlibReserve = 0;
    }
    place_vehlib()
    {
        Numero = 0;
        Initialise();
    }
    void Copie(place_vehlib *p)
    {
        Numero = p->Numero;
        x = p->x;
        y = p->y;
        NumVehlibReserve = p->NumVehlibReserve;
    }
    void AffecteNumero(int Num) { Numero = Num; }
    [[nodiscard]] bool Definie() const { return (x > -1) && (y > -1); }
    void Definit(int x0, int y0)
    {
        x = x0;
        y = y0;
    }
    ~place_vehlib() = default;
    bool Charge(int hfile);
    bool Sauve(int hfile);
    void Verifie(AnsiString &asMsgErr) const;
    [[nodiscard]] bool EstLibre() const;
};
//---------------------------------------------------------------------------
class vehlib // v5.3 instances créées au lancement de la simulation
{
    int Numero;

public:
    situation_vehlib Situation{};
    int x{}, y{};
    direction Dir{}, DirDrnDepl{};
    int NumPlaceDepart{},  // Place de départ. 0 = pas placé sinon index+1 de centre_ville.PlaceVehlib[].
        NumPlaceArrivee{}, // Place réservée. 0 = pas encore réservé sinon Index+1 de centre_ville.PlaceVehlib[].
        NbPersonnes{}, TourDrnDepl{}, // Tour du dernier déplacement effectué
        TourDepart{},                 // Tour pdt lequel il a été placé sur le parking de départ
        NbCasesParcourues{},          // Nombre de cases parcourues
        NbToursRepos{}; // Tours pendant lesquels le véhicule ne s'est pas déplacé du fait de la vitesse réduite
    void Initialise()
    {
        Situation = svAttente;
        x = -1;
        y = -1;
        DirDrnDepl = Dir = indefinie;
        NbToursRepos = NbCasesParcourues = TourDepart = TourDrnDepl = 0L;
        NumPlaceArrivee = NumPlaceDepart = 0;
        NbPersonnes = 0;
    }
    vehlib()
    {
        Numero = 0;
        Initialise();
    }
    void AffecteNumero(int Num) { Numero = Num; }
    void Place(int x0, int y0)
    {
        x = x0;
        y = y0;
    }
    void Place(int p);                              // 0 <= p < NbPlacesVehlib
    void Definit(int p) { NumPlaceDepart = p + 1; } // ATTENTION: 0 <= p < NbPlacesVehlib
    void DefinitEtPlace(int p)
    {
        Definit(p);
        Place(p);
    } // ATTENTION: 0 <= p < NbPlacesVehlib
    ~vehlib() = default;
    [[nodiscard]] bool Defini() const { return (x > -1) && (y > -1); }
    bool EstDestination(const voie *v) const;
    void CalculeProchaineDirection();
    [[nodiscard]] direction ProchaineDirectionAuCarrefour() const;
    [[nodiscard]] bool PeutPasser() const;
    void FaitDescendrePietons();
    bool ReservePlaceVehlibCible(); // v6.0.1 : false si aucune place n'est libre
    void FaitMonterPieton(int NumPietonAFaireMonter);
    int NbPietonsQuiSontMontes(); // retourne true si des piétons attendaient et sont montés
    void Verifie(AnsiString &asMsgErr) const;
};
//---------------------------------------------------------------------------
// Classes de GESTION GENERALE de la SIMULATION
//---------------------------------------------------------------------------
class voie
{
    int x, y;

public:
    voie(const voie &) = delete;
    voie &operator=(const voie &) = delete;
    dirposs DirPoss{};
    sens_voie_tram SensVoieTram{}; // v3.5. 0 = aucune voie. Sinon 0<SensVoieTram<64.
    std::array<priorite, 4> Priorite{};
    nature Nature{}; // v3.8. 0 = rien (fond noir), poids fort = type (voies,trottoirs,immeubles), poid faible = décors
                     // (arbre, pelouse, etc.)
    bool ToutDroit{};
    bool PassagePietons{}; // v2.0. Si PassagePieton, est perpendiculaire au sens de la route
    bool PassageBus{};     // v3.0. Si PassageBus, seuls les bus peuvent franchir la case (et véhicules prioritaires)
    std::array<bool, NBMAXLIGNESBUSTRAM> LigneBus{};  // v3.0. Si Ligne[i] alors la ligne n°i+1 passe par cette case
                                                      // (visualisation par la fiche paramètres du réseau de bus
                                                      // UNIQUEMENT)
    std::array<bool, NBMAXLIGNESBUSTRAM> LigneTram{}; // v3.5. Si Ligne[i] alors la ligne n°i+1 passe par cette case
                                                      // (visualisation par la fiche paramètres du réseau de tram
                                                      // UNIQUEMENT)
    int NumVehicule{};                        // >0 si véhicule éventuel (=index+1 de cv->Vehicule)
    int NumVehlib{};                          // v5.3 si vehlib éventuel (=index+1 de cv->Vehlib)
    int NumBus{};                             // v3.0. >0 si bus éventuel (=index+1 de cv->Bus)
    int NumTram{};                            // v3.5. >0 si tram éventuel (=index+1 de cv->Tram)
    int NumParking{};                         // >0 si c'est un parking (=index+1 de cv->parking)
    int NumProchVeh{};                        // >0 si un véhicule doit passer sur la case lors du prochain mouvement
    int NumProchVL{};                         // >0 si un vehlib doit passer sur la case lors du prochain mouvement
    int NumProchBus{};                        // v3.0. >0 si un bus doit passer sur la case lors du prochain mouvement
    int NumProchTram{};                       // v3.5. >0 si un tram doit passer sur la case lors du prochain mouvement
    int NumProchTaxi{};                       // v3.6. >0 si un taxi doit passer sur la case lors du prochain mouvement
    int NbPietons{};                          // v2.0. Nombre de pietons sur cette case
    int NumArretBus{};                        // v3.0. >0 si arrêt de bus (=index+1 de cv->ArretBus)
    int NumArretTram{};                       // v3.5. >0 si arrêt de tram (=index+1 de cv->ArretTram)
    int NumTaxi{};                            // v3.6. >0 si taxi (=index+1 de cv->Taxi)
    int NumPlaceTaxi{};                       // v3.6. >0 si taxi (=index+1 de cv->PlaceTaxi)
    int NumPlacePark{};                       // v5.0. >0 si place de parking (=index+1 de cv->PlacePark)
    int NumPlaceVehlib{};                     // v5.3. >0 si place de vehlib (=index+1 de cv->PlaceVehlib)
    int NumSpotVehlib{};                      // v5.3. >0 si spot vehlib (=index+1 de cv->SpotVehlib)
    std::array<int, NBFEUXPARCASE> NumFeu{};  // >0 si feu éventuel (=index+1 de cv->Feu) (v5.2: NBFEUXPARCASE)
    std::array<int, NBFEUXPARCASE> NumFeuP{}; // v5.2. >0 si feu piéton éventuel (=index+1 de cv->FeuP)
    std::array<int, NBMAXPIETONSPARCASE> NumPieton{}; // v2.0. >0 si un piéton est sur la case. Max: 2 piétons/case.
    // Distances (en cases) aux éléments, par élément puis par direction possible de la case. DistParking et
    // DistFilePark ont deux valeurs par direction : sans et avec les passages de bus (SANSPSGBUS, AVECPSGBUS).
    // Vides tant que la case n'a pas de direction possible ou que le réseau n'a pas l'élément.
    std::vector<std::vector<std::array<int, 2>>> DistParking, // v3.5. Distances aux parkings
        DistFilePark;                                         // v5.0. Distance jusqu'à la queue de la file de parkings
    std::vector<std::vector<int>> DistArretBus,               // v3.0. Distance à un arrêt de bus
        DistArretTram,   // v3.5. Distance à un arrêt de tram (uniquement quand SensVoieTram>aucune_voie)
        DistFileTaxi,    // v3.6. Distance jusqu'à la queue de la file de taxis
        DistPlaceVehlib; // v5.3. Distance jusqu'à une place de vehlib (sans notion de priorité)
    // v2.2.7. Statistiques sur les passages de véhicules : Attente[ed][ea] et Trafic[ed][ea] avec
    // ed=ParkingDépart, ea=ParkingArrivée. Si ed=0, par ParkingArrivée (toute ParkingDépart confondue) ;
    // si ea=0, par ParkingDépart (toute ParkingArrivée confondue) ; si ed=ea=0, tous véhicules.
    std::vector<std::vector<int>> Attente, // v2.2.7. Nombre de tours pendant lesquels un véhicule est resté sur la case
        Trafic;                            // v2.2.7. Nombre de véhicules passés par la case
    // v6.0.1 : dimensions des tableaux à leur allocation
    int NbDirDist{}, NbParkingsDist{}, NbArretsBusDist{}, NbArretsTramDist{}, NbFilesTaxiDist{}, NbFilesParkDist{},
        NbPlacesVehlibDist{}, NbParkingsAttente{};
    voie();
    void Initialise();
    void Initialise(type_voie TypeVoie); // v3.5
    void Definit(int x0, int y0)
    {
        x = x0;
        y = y0;
        Initialise();
    }
    [[nodiscard]] voie *v(direction d) const;                // v3.0
    bool PrepareCalculDistances();                           // v3.0 : inclut également celui des arrêts de bus/tram
    void LibereDistances();                                  // v6.0.1
    bool PrepareCalculAttenteTrafic();                       // v2.2.7
    void LibereAttenteTrafic();                              // v6.0.1
    void CalculeProchainVehiculeOuBusOuTramOuTaxiOuVehlib(); // v3.6 (changement de nom : +OuTaxi) v5.3 (changement de
                                                             // nom : +OuVehlib)
    void DeplaceVehiculeOuBusOuTramOuTaxi();                 // v3.6 (changement de nom : +OuTaxi)
    bool PietonPeutAller(direction DirPieton);               // v2.0
    bool FaitDescendrePieton(direction DirDescente, int NumPietonAFaireDescendre); // v3.6 (taxis+DirDescente)
    bool AjoutePieton(int NumPietonAAjouter);                                      // v2.0
    bool RetirePieton(int NumPietonARetirer);                                      // v2.0
    void DeplacePietons();                                                         // v2.0
    void RetireArretBus();                                                         // v3.0
    void RetireArretTram();                                                        // v3.5
    void RetirePlaceTaxi();                                                        // v3.6
    void RetirePlacePark();                                                        // v5.0
    void RetirePlaceVehlib();                                                      // v5.3
    [[nodiscard]] bool EstTeteArretBus() const;                                    // v3.0
    [[nodiscard]] bool EstTeteArretBus(int NumeroArretBus) const;                  // v3.0
    [[nodiscard]] bool EstTeteArretTram() const;                                   // v3.5
    [[nodiscard]] bool EstTeteArretTram(int NumeroArretTram) const;                // v3.5
    [[nodiscard]] bool EstTeteBus() const;                                         // v3.0
    [[nodiscard]] bool EstTeteTram() const;                                        // v3.5

    [[nodiscard]] bool EstQueueFileTaxis() const;              // v3.6
    [[nodiscard]] bool EstTeteFileTaxis() const;               // v3.6
    [[nodiscard]] int PlaceQueueFileTaxi();                    // v3.6. Retourne 1+NumPlaceTaxi si elle existe, 0 sinon.
    int PlaceTeteFileTaxi();                                   // v3.6. Retourne 1+NumPlaceTaxi si elle existe, 0 sinon.
    [[nodiscard]] bool ExistePlusieursQueuesFileTaxis() const; // v3.6
    [[nodiscard]] bool ExistePlusieursTetesFileTaxis() const;  // v3.6

    [[nodiscard]] bool EstQueueFilePark() const;              // v5.0
    [[nodiscard]] bool EstTeteFilePark() const;               // v5.0
    [[nodiscard]] int PlaceQueueFilePark();                   // v5.0. Retourne 1+NumPlace si elle existe, 0 sinon.
    [[nodiscard]] bool ExistePlusieursQueuesFilePark() const; // v5.0
    [[nodiscard]] bool ExistePlusieursTetesFilePark() const;  // v5.0

    [[nodiscard]] int OrientationPlaceVehlib() const; // v5.3 : Permet d'orienter l'image de la place selon
                                                      // l'orientation de la file !

    void DefinitArretBus(int n);                // v3.0
    void DefinitArretTram(int n);               // v3.5
    void RedessineCasesArretBus(direction d0);  // v3.0
    void RedessineCasesArretTram(direction d0); // v3.5
    [[nodiscard]] bool EstDepotBus() const;     // v3.0

    void CalculeDistanceParking(int e, int DistSansPsgBus, int DistAvecPsgBus, direction DirOrig);
    void CalculeDistanceArretBus(int ab, int Distance, direction DirOrig);                           // v3.0
    void CalculeDistanceArretTram(int at, int Distance, direction DirOrig);                          // v3.5
    void CalculeDistanceFileTaxi(int ft, int Distance, direction DirOrig);                           // v3.6
    void CalculeDistanceFilePark(int fp, int DistSansPsgBus, int DistAvecPsgBus, direction DirOrig); // v5.0
    void CalculeDistancePlaceVehlib(int pv, int Distance, direction DirOrig);                        // v5.3
    void InitialiseInfosLignesBus();                                                                 // v3.0
    void ModifieSensVoieTram(direction DirOrig, direction DirDest);                                  // v3.5
    direction DirSensVoieTram(direction DirOrigine, int iDir);                                       // v3.5
    [[nodiscard]] bool EstDirPossSensVoieTram(direction DirOrigine, direction DirDestination) const; // v3.5
    int NumDirPossSensVoieTram(direction DirOrigine, direction DirDestination);                      // v3.5
    [[nodiscard]] int NbDirPossSensVoieTram(direction DirOrigine) const;                             // v3.5
    bool SupprimePassagePietonsInvalide();                                                           // v3.5
    bool ExisteVoieDansDirection(type_voie TypeVoie, direction d); // v3.5 // v5.4 : pas de inline pour éviter
                                                                   // avertissement "non trouvé"
    void DefinitNature(nature NvNature, bool Ecraser);
    void TuePietons(); // v4.3. Appelée par centre_ville::PlaceXXX (XXX = Vehicule, Bus, Tram, VehiculeTaxi ou
                       // VehiculeVehlib v5.3)
    [[nodiscard]] bool FeuPietonsPossible() const; // v5.2
    void RedessineCase(bool EffaceFond);           // v3.0
    bool Charge(int hfile);
    bool Sauve(int hfile);
    void Verifie(AnsiString &asMsgErr);
    [[nodiscard]] bool FeuxPietons() const // v5.2 (macros FEUXPIETONS et FeuxPietons())
    {
        return NumFeuP[0] + NumFeuP[1] + NumFeuP[2] + NumFeuP[3] > 0;
    }
};
//---------------------------------------------------------------------------
// Nature sans les décors (poids fort de voie::Nature ; macro NATURE)
inline int NatureSansDecor(const voie *v)
{
    return v->Nature & 0x7FFF0000;
}
//---------------------------------------------------------------------------
class stats
{
public:
    int NbVehiculesDeplaces, NbVehiculesArrives, NbVehiculesSortis;
    // v5.4.1 : stats piétons (épidémie)
    int NbPietonsGueris, NbPietonsInfectes, NbPietonsMorts;
    stats() // v3.5 : manquait un s !!!
    {
        NbVehiculesDeplaces = NbVehiculesArrives = NbVehiculesSortis = 0L;
        NbPietonsGueris = NbPietonsInfectes = NbPietonsMorts = 0L; // v5.4.1 : stats épidémiques des piétons
    }
    void Definit();
};
//---------------------------------------------------------------------------
class centre_ville
{
    void IncrementeAttente(int NumVehicule); // v2.2.7
    void IncrementeTrafic(int NumVehicule);  // v2.2.7
public:
    centre_ville(const centre_ville &) = delete;
    centre_ville &operator=(const centre_ville &) = delete;
    bool TourEnCours,
        CopieEnCours;         // v4.0.1 Copie en cours de la sélection dans un fichier
    TDateTime dtTop, dtTemps; // v3.9.2
    int NbX, NbY,             // v4.0
        TourCrt,
        NbFeux,         // Nombre de feux max. pouvant être utilisés
        NbFeuxPieton,   // v5.2. Nombre de feux pietons max. pouvant être utilisés
        NbParkings,     // Nombre de parkings max. pouvant être utilisés
        NbVehicules,    // Nombre de véhicules générés pour la simulation
        NbVehlibs,      // v5.3. Nombre de vehlib générés pour la simulation
        NbBus,          // v3.0. Nombre de bus générés pour la simulation
        NbTrams,        // v3.5. Nombre de trams
        NbPietons{},    // v2.0. Nombre de piétons générés pour la simulation
        NbArretsBus,    // v3.0. Nombre d'arrêts de bus à utiliser
        NbLignesBus,    // v3.0. Nombre de lignes de bus
        NbArretsTram,   // v3.5. Nombre d'arrêts de tram
        NbLignesTram,   // v3.5. Nombre de lignes de tram
        NbPlacesTaxi,   // v3.6. Nombre de places de taxi
        NbFilesTaxi,    // v3.6. Nombre de file de taxi
        NbTaxis,        // v3.6. Nombre de taxis (<= Nb.Places de taxi)
        NbPlacesPark,   // v5.0. Nombre de places de parking
        NbFilesPark,    // v5.0. Nombre de files de parkings
        NbPlacesVehlib, // v5.3. Nombre de places de vehlib
        DureeVert,      // Durée du vert PAR DEFAUT (à la création d'un feu)
        DureeOrange,    // Duree de l'orange dans la simulation;
        DureeRouge,     // Durée du rouge PAR DEFAUT (à la création d'un feu)
        xErr{}, yErr{}, // Coordonnées de la case concernant la dernière erreur
                        // stats véhicules
        NbVehiculesDeplaces, NbVehiculesArrives, NbVehiculesSortis,
        // stats épidémiques des piétons
        NbPietonsGueris, NbPietonsInfectes, NbPietonsMorts,
        NbToursStats,          // Nombre de tours soumis à la statistiques (derniers)
        NbVitesses,            // v1.5. Vitesse
        NbMaxCasesDetectPsgPt, // v2.0. Maximum de cases qu'un piéton peut contrôler pour détecter un passage piéton
        ProbaDirPsgPt,         // v2.0. Probabilité pour un piéton de se diriger vers un passage piéton en changeant de
                               // direction
        NbMaxCasesDetectArretBus,  // v3.0. Maximum de cases qu'un piéton peut contrôler pour détecter un arret de bus
        NbMaxCasesDetectArretTram, // v3.5. Maximum de cases qu'un piéton peut contrôler pour détecter un arret de tram
        NbMaxCasesDetectFileTaxi,  // v3.6. Maximum de cases qu'un piéton peut contrôler pour détecter une file de taxis
        NbMaxCasesDetectFileVehlib, // v5.3. Maximum de cases qu'un piéton peut contrôler pour détecter une file de
                                    // vehlibs
        ProbaDirArretBus,           // v3.0. Probabilité pour un piéton de se diriger vers un arrêt de bus
        ProbaDirArretTram,          // v3.5. Probabilité pour un piéton de se diriger vers un arrêt de tram
        ProbaDirFileTaxi,           // v3.6. Probabilité pour un piéton de se diriger vers une file de taxis
        ProbaDirFileVehlib,         // v5.3. Probabilité pour un piéton de se diriger vers une file de vehlibs
        ProbaArretBusDmd,           // v3.0. Probabilité pour un piéton de demander à descendre au prochain arrêt de bus
        ProbaArretTramDmd,     // v3.5. Probabilité pour un piéton de demander à descendre au prochain arrêt de tram
        ProbaArretTaxiDmd,     // v3.6. Probabilité pour un piéton de demander à descendre du taxi
        TauxRemplissageVehlib, // v5.3. Taux de remplissage des places de vehlib au début de la simulation (88% par
                               // défaut : 1/1 jusqu'à 4/4 puis 4/5, 5/6 pour que des places soient libres pour les
                               // premiers vehlibs pris...)
        NbMaxPersonnesBus,     // v3.0. Capacité d'un bus
        NbMaxPersonnesTram,    // v3.5. Capacité d'un tram
        NbMaxPersonnesTaxi,    // v3.6. Capacité d'un taxi
        NbMaxPersonnesVehlib,  // v5.3. Capacité d'un vehlib
        NbToursStopArretBus,   // v3.0. Arrêt obligatoire (tours) à chaque arrêt de bus
        NbToursStopArretTram,  // v3.5. Arrêt obligatoire (tours) à chaque arrêt de tram
        NbMaxToursAttenteBus,  // v3.0. Nombre max d'attente d'un bus à l'arrêt
        NbMaxToursAttenteTram, // v3.5. Nombre max d'attente d'un tram à l'arrêt
        NbMaxToursAttenteTaxi, // v3.6. Nombre max d'attente d'un taxi
        AttenteMaxVeh, // Attente max d'un vehicule bloqué avant choix d'un autre trajet. AttenteMaxVeh/2 pour un piéton
                       // et 5*AttenteMaxVeh pour un bus.
        NbToursParSeconde,                // Vitesse de simulation
        NbCasesParcourues,                // Nb total de cases parcourues par tous les véhicules
        NbToursParcourus,                 // Nb Total de tours tous véhicules confondus
        Xc, Yc, NbXc, NbYc;               // v4.0.1 Coordonnées et taille de la sélection à copier
                                          // DEBUT Propriétés pour la génération aléatoire de réseau (v4.2)
    int LgrMinSgmtRoute, LgrMaxSgmtRoute, // Longueur de segment (min/max) de route dans une dir. donnée
        ProbaSortieIntersection, // en dixième : probabilité de créer une nouvelle direction (sortie d'intersection)
                                 // après chaque segment tracé
        ProbaEntreeIntersection, // en dixième : probabilité de déboucher sur une route (entrée d'intersection) après
                                 // chaque segment tracé
        ProbaTraverseeRoute;     // en dixième : probabilité qu'un segment traverse une route (entrée + franchissement +
                                 // sortie d'intersection)
                                 // v5.4.1 paramètres de l'épidémie (si active)
    int EpidemieInfectiosite,    // Nb tours
        EpidemieIterationPatientZero, // Nb piétons
        EpidemieChargeFatale,         // en % piétons infectés au bout de la période d'infectiosité
        EpidemieReinfections; // v5.4.2 : Si vrai, alors même guéri, un piéton peut être réinfecté, si faux, il ne peut
                              // plus l'être (faux par défaut)

    // FIN Propriétés pour la génération aléatoire de réseau
    bool VerifOk, DistancesCalculees, QuadrillageStats;
    type_stats TypeStats;

    std::vector<std::vector<voie>> v; // v[x][y]. Dimensions modifiables (v4.0)
    // Case voisine dans la direction d, ou à n cases dans la direction d, sur le tore (macros V, _V, V_, _V_)
    [[nodiscard]] voie &Voisine(int x, int y, int d);
    [[nodiscard]] const voie &Voisine(int x, int y, int d) const;
    [[nodiscard]] voie &Voisine(int x, int y, int n, int d);
    [[nodiscard]] const voie &Voisine(int x, int y, int n, int d) const;
    std::vector<feu> Feu;                                   // Tableau de feux
    std::vector<feu_pieton> FeuP;                           // v5.2 Tableau de feux piéton
    std::vector<parking> Parking;                           // Tableau des parkings
    std::vector<vehicule> Vehicule;                         // Tableau de voitures
    std::vector<vehlib> Vehlib;                             // v5.3. Tableau de vehlib
    std::vector<bus> Bus;                                   // v3.0. Tableau de bus
    std::vector<tram> Tram;                                 // v3.5. Tableau de trams
    std::vector<pieton> Pieton;                             // v2.0. Tableau de piétons
    depot_bus DepotBus;                                     // v3.0 : Attention INSTANCE STATIQUE car Dépôt unique !
    std::vector<arret_bus> ArretBus;                        // v3.0. Tableau d'arrêts de bus
    std::array<ligne_bus, NBMAXLIGNESBUSTRAM> LigneBus{};   // v3.0. Tableau (statique) de lignes de bus
    std::vector<arret_tram> ArretTram;                      // v3.5. Tableau d'arrêts de tram
    std::array<ligne_tram, NBMAXLIGNESBUSTRAM> LigneTram{}; // v3.5. Tableau (statique) de lignes de tram
    std::vector<taxi> Taxi;                                 // v3.6. Tableau de taxis
    std::vector<place_taxi> PlaceTaxi;                      // v3.6. Tableau de places de taxis
    std::vector<file_taxi> FileTaxi;                        // v3.6. Tableau de files de taxi
    std::vector<place_park> PlacePark;                      // v5.0. Tableau de places de parking
    std::vector<file_park> FilePark;                        // v5.0. Tableau de files de parkings
    std::vector<place_vehlib> PlaceVehlib;                  // v5.3. Tableau de places de vehlib
    std::vector<stats> Stats;                               // Tableau de statistiques
    void CreeZone();                                        // v4.0 (allocation dynamique si NbX*NbY>0)
    void RetablitVoisinages() const; // v6.0.2 : dx et dy (globales) suivent le dernier réseau créé : à rappeler quand
                                     // on revient à un réseau existant
    centre_ville(bool NbElementsParDefaut, int NbX, int NbY); // v4.0 (NbX, NbY)
    ~centre_ville();
    int DefinitNbFeux(int n);
    int DefinitNbFeuxPietons(int n); // v5.2
    int DefinitNbParkings(int n);
    int DefinitNbVehicules(int n);
    int DefinitNbVehlibs(int n);    // v5.3
    int DefinitNbBus(int n);        // v3.0
    int DefinitNbTrams(int n);      // v3.5
    int DefinitNbPietons(int n);    // v2.0
    int DefinitNbLignesBus(int n);  // v3.0
    int DefinitNbLignesTram(int n); // v3.5

    int DefinitNbFilesTaxi(int n); // v3.6
    int DefinitNbTaxis(int n);     // v3.6
    int DefinitFilesTaxi();        // v3.6. Renvoie le nombre de files créées. Défini à partir de PlaceTaxi.

    int DefinitNbFilesPark(int n); // v5.0
    int DefinitFilesPark();        // v5.0. Renvoie le nombre de files créées. Défini à partir de PlacePark.

    [[nodiscard]] int NbFeuxLibres() const;
    [[nodiscard]] int NumeroFeuLibre() const;
    [[nodiscard]] int NumeroFeuPietonsLibre() const; // v5.2
    [[nodiscard]] int NumeroParkingLibre() const;
    [[nodiscard]] int ArretBusExistant(int x, int y);        // v3.0
    [[nodiscard]] int ArretTramExistant(int x, int y) const; // v3.5
    int NumeroArretBusLibre(int x, int y);                   // v3.0
    int NumeroArretTramLibre(int x, int y);                  // v3.5
    [[nodiscard]] int NumeroPlaceTaxiLibre() const;          // v3.6
    [[nodiscard]] int NumeroPlaceParkLibre() const;   // v5.0. v5.3 renommée de NumeroPlaceLibre pour ne pas confondre
                                                      // avec NumeroPlaceVehlibLibre()
    [[nodiscard]] int NumeroPlaceVehlibLibre() const; // v5.3
    int AjouteFeu(int x, int y, direction d);
    int AjouteFeuPietons(int x, int y, direction d); // v5.2
    void SupprimeFeu(int n);
    void SupprimeFeuPietons(int n);                          // v5.2
    void SupprimeFeuxVoie(int x, int y, type_voie TypeVoie); // v3.5 (TypeVoie)
    void SupprimeParkingVoie(int x, int y);                  // v3.6
    void SupprimeArretBusVoie(int x, int y);                 // v3.6
    void SupprimeDepotBusVoie(int x, int y);                 // v3.6
    void SupprimeArretTramVoie(int x, int y);                // v3.6
    void SupprimePlaceTaxiVoie(int x, int y);                // v3.6
    void SupprimePlaceParkVoie(int x, int y);   // v5.0. v5.3 : Renommée de SupprimePlaceVoie pour ne pas confondre
                                                // avec SupprimePlaceVehlibVoie(x, y)
    void SupprimePlaceVehlibVoie(int x, int y); // v5.3
    int AjouteParking(int x, int y);
    void SupprimeParking(int n);
    int AjouteArretBus(int x, int y);                     // v3.0
    int AjouteArretTram(int x, int y);                    // v3.5
    int AjoutePlaceTaxi(int x, int y);                    // v3.6
    int AjoutePlacePark(int x, int y);                    // v5.0, v5.1 : ancien nom AjoutePlace
    int AjoutePlaceVehlib(int x, int y);                  // v5.3
    [[nodiscard]] bool ExisteCaseArretBus(int x, int y);  // v3.0
    [[nodiscard]] bool ExisteCaseArretTram(int x, int y); // v3.5
    void SupprimeArretBus(int x, int y);                  // v3.0. Supprime sur une case seulement
    void SupprimeArretBus(int n);                         // v3.0. Supprime complètement l'arrêt (sur TOUTES les cases)
    void SupprimeArretTram(int x, int y);                 // v3.5. Supprime sur une case seulement
    void SupprimeArretTram(int n);                        // v3.5. Supprime complètement l'arrêt (sur TOUTES les cases)
    void SupprimePlaceTaxi(int n);                        // v3.6
    void SupprimePlacePark(int n);   // v5.0. v5.3 renommée de SupprimePlace(n) pour distinguer avec
                                     // SupprimePlaceVehlib(n)
    void SupprimePlaceVehlib(int n); // v5.3
    [[nodiscard]] sens SensPassagePietonsPossible(int x, int y) const;                       // v3.5
    [[nodiscard]] direction CherchePassagePietons(int x, int y, sens SensRecherche);         // v2.0
    [[nodiscard]] direction ChercheArretBus(int x, int y, sens SensRecherche);               // v3.0
    [[nodiscard]] direction ChercheArretTram(int x, int y, sens SensRecherche);              // v3.5
    [[nodiscard]] direction CherchePlaceTaxi(int x, int y, sens SensRecherche);              // v3.6
    [[nodiscard]] direction CherchePlaceVehlib(int x, int y, sens SensRecherche);            // v5.3
    void DefinitArretBus(int n, int x, int y);                                               // v3.0
    void DefinitArretTram(int n, int x, int y);                                              // v3.5
    [[nodiscard]] direction DirectionArretBus(int x, int y) const;                           // v3.0
    [[nodiscard]] position_arret_bus_tram PositionArretBus(int x, int y, direction d) const; // v3.0
    [[nodiscard]] bool ArretsBusVoisinsDifferents(int x, int y) const;                       // v3.0
    [[nodiscard]] bool ArretsTramVoisinsDifferents(int x, int y) const;                      // v3.5
    [[nodiscard]] direction DirArretBusVoisin(int x, int y) const;                           // v3.0
    [[nodiscard]] direction DirArretTramVoisin(int x, int y) const;                          // v3.5
    bool AjoutArretBusPossible(int x, int y);                                                // v3.0
    bool AjoutArretTramPossible(int x, int y);                                               // v3.5
    [[nodiscard]] bool SuppressionArretBusPossible(int x, int y) const;                      // v3.0
    [[nodiscard]] bool SuppressionArretTramPossible(int x, int y) const;                     // v3.5
    void InitialiseInfosLignesBus();                                                         // v3.0
    void InitialiseInfosLignesTram();                                                        // v3.5
    void InitialiseInfosTrajetLignesBus();                                                   // v3.0
    void CalculeTrajetLignesBus();                                                           // v3.0
    void CalculeTrajetLignesTram();                                                          // v3.5
    bool VerifieCalculDistances(type_distance &TypeDistance); // v3.5 : inclut également celui des arrêts de bus & tram
    [[nodiscard]] bool PrepareCalculDistances();              // v3.5 : inclut également celui des arrêts de bus & tram
    void LibereDistances(); // v6.0.1 : distances obsolètes (réseau modifié après un échec de Marche)
    [[nodiscard]] bool PrepareCalculAttenteTrafic();   // v2.2.7
    void PlaceVehicule(int x, int y, int NumVehicule); // v2.2.7
    bool PrepareVehicules();
    void PlaceBus(int x, int y, int NumBus);               // v3.0
    void DefinitBus(int NumBus, int NumLigne);             // v3.0
    bool PrepareBus();                                     // v3.0
    void PlaceTram(int x, int y, int NumTram);             // v3.5
    void DefinitTram(int NumTram, int NumLigne);           // v3.5
    void PlaceVehiculeTaxi(int x, int y, int NumTaxi);     // v3.6
    void PlaceVehiculeVehlib(int x, int y, int NumVehlib); // v5.3
    bool PrepareTrams();                                   // v3.5
    bool PrepareTaxis();                                   // v3.6. Les taxis ainsi que leurs places et leurs files
    bool PrepareVehlibs();                                 // v5.3. Les vehlibs ainsi que leurs places et leurs files
    bool PreparePietons();                                 // v2.0
    void DeplacePieton(int NumPieton, int x, int y, direction d); // v2.0
    bool ChangeDirectionSiPossible(int x, int y, dirposs dp);
    void CalculeDistances();
    void RessuscitePietons();                                                       // v4.3.1
    void PlaceVehiculesSurPlacesPark();                                             // v5.0
    [[nodiscard]] bool ExisteFeuPietons(int x, int y) const;                        // v5.2
    void PrepareFeuxPietons();                                                      // v5.2
    [[nodiscard]] bool FeuPietonsPossible(int x, int y, direction d) const;         // v5.2
    [[nodiscard]] bool PassagePietonFranchissable(int x, int y, direction d) const; // v5.2
    void /*__fastcall*/ EffectueTourSimulation(); // portage : convention d'appel C++Builder retirée
    bool Charge(int hfile);
    bool Sauve(int hfile);
    void Verifie(AnsiString &asMsgErr);
    [[nodiscard]] inline bool TourDeDeplacementVehicule(int NumeroVehicule);
    [[nodiscard]] inline bool TourDeDeplacementVehlib(int NumeroVehlib);
    [[nodiscard]] inline bool TourDeDeplacementBus(int NumeroBus);
    [[nodiscard]] inline bool TourDeDeplacementTram(int NumeroTram); // v3.5
    [[nodiscard]] inline bool TourDeDeplacementTaxi(int NumeroTaxi); // v3.6
    void TopChrono()                                                 // v3.9.2
    {
        dtTop = Time();
    }
    void StopChrono() // v3.9.2
    {
        dtTemps += (Time() - dtTop);
    }
    bool CopieValide();                                             // v4.2.1
    void PrepareCopie(bool EnCours, int x, int y, int NbX, int NbY) // v4.0.1
    {
        if ((CopieEnCours = EnCours))
        {
            Xc = x;
            Yc = y;
            NbXc = NbX;
            NbYc = NbY;
        }
        else
        {
            Xc = Yc = NbXc = NbYc = 0; // v6.0.1 : NbY (le paramètre) dans l'original
        }
    }
    template <class C> bool DansSelectionEnCours(C *c) // v4.0.1
    {
        return (c->x >= Xc) && (c->y >= Yc) && (c->x < Xc + NbXc) && (c->y < Yc + NbYc);
    }
    template <class C> AnsiString asCoordonnees(C *c) // v4.2.1
    {
        if ((c->x > -1) && (c->y > -1))
        {
            return "(" + IntToStr(c->x) + ", " + IntToStr(c->y) + ")";
        }
        return "(?, ?)";
    }
    void PrepareGenerationAleatoire(int LgrMinSgmtRoute, int LgrMaxSgmtRoute, int ProbaSortieIntersection,
                                    int ProbaEntreeIntersection,
                                    int ProbaTraverseeRoute) // v4.2
    {
        this->LgrMinSgmtRoute = LgrMinSgmtRoute;
        this->LgrMaxSgmtRoute = LgrMaxSgmtRoute;
        this->ProbaSortieIntersection = ProbaSortieIntersection;
        this->ProbaEntreeIntersection = ProbaEntreeIntersection;
        this->ProbaTraverseeRoute = ProbaTraverseeRoute;
    }
    void Genere(int x, int y, int d); // v4.2
};
//---------------------------------------------------------------------------
// v3.5 : remplacement des [5] par [NBDIR+1] et des [4] par [NBDIR]
inline constexpr std::array<dirposs, NBDIR + 1> PossDir = {aucune, N, E, S, O};
inline constexpr std::array<dirposs, NBDIR + 1> InvPossDir = {aucune, S, O, N, E};
// 1=90°droite, 2=180°, 3=90°gauche
inline constexpr std::array<std::array<dirposs, 16>, NBDIR> RotPossDir = {{
    {aucune, N, E, NE, S, NS, SE, NES, O, NO, EO, NEO, SO, NSO, ESO, NESO},
    {aucune, E, S, SE, O, EO, SO, ESO, N, NE, NS, NES, NO, NEO, NSO, NESO},
    {aucune, S, O, SO, N, NS, NO, NSO, E, SE, EO, ESO, NE, NES, NEO, NESO},
    {aucune, O, N, NO, E, EO, NE, NEO, S, SO, NS, NSO, SE, ESO, NES, NESO},
}};
inline constexpr std::array<int, NBDIR + 1> dxr = {0, 0, 1, 0, -1};
inline constexpr std::array<int, NBDIR + 1> dyr = {0, -1, 0, 1, 0};
inline constexpr std::array<int, 16> NbDir = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
// NumDirdp[direction][dirposs] = Ordre de direction dans dirposs
// 0 <= NumDirdp[direction][dirposs] < NbDir[direction][dirposs]
inline constexpr std::array<std::array<int, 16>, NBDIR + 1> NumDirdp = {{
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // indefinie
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // nord
    {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1}, // est
    {0, 0, 0, 0, 0, 1, 1, 2, 0, 0, 0, 0, 0, 1, 1, 2}, // sud
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 1, 2, 2, 3},
}};
// ouest
// SensPsgPt[dirposs] = indefini|horizontal|vertical. v2.0
inline constexpr std::array<sens, 16> SensPsgPt = {
    indefini, horizontal, vertical, indefini, horizontal, horizontal, indefini, indefini,
    vertical, indefini,   vertical, indefini, indefini,   indefini,   indefini, indefini,
};
// Dirdp[x][y] = (x+1)ème direction pour DirPoss=y
inline constexpr std::array<std::array<direction, 16>, NBDIR> Dirdp = {{
    {
        indefinie,
        nord,
        est,
        nord,
        sud,
        nord,
        est,
        nord,
        ouest,
        nord,
        est,
        nord,
        sud,
        nord,
        est,
        nord,
    },
    {
        indefinie,
        indefinie,
        indefinie,
        est,
        indefinie,
        sud,
        sud,
        est,
        indefinie,
        ouest,
        ouest,
        est,
        ouest,
        sud,
        sud,
        est,
    },
    {
        indefinie,
        indefinie,
        indefinie,
        indefinie,
        indefinie,
        indefinie,
        indefinie,
        sud,
        indefinie,
        indefinie,
        indefinie,
        ouest,
        indefinie,
        ouest,
        ouest,
        sud,
    },
    {
        indefinie,
        indefinie,
        indefinie,
        indefinie,
        indefinie,
        indefinie,
        indefinie,
        indefinie,
        indefinie,
        indefinie,
        indefinie,
        indefinie,
        indefinie,
        indefinie,
        indefinie,
        ouest,
    },
}};
inline constexpr std::array<direction, NBDIR + 1> InvDir = {indefinie, sud, ouest, nord, est};
inline constexpr std::array<direction, NBDIR + 1> AGaucheDe = {indefinie, ouest, nord, est, sud};
inline constexpr std::array<direction, NBDIR + 1> ADroiteDe = {indefinie, est, sud, ouest, nord};
// v3.5. SensVoieTramDir[DirOrig][DirDest] (indefinie, nord, est, sud, ouest)
inline constexpr std::array<std::array<sens_voie_tram, NBDIR + 1>, NBDIR + 1> SensVoieTramDir = {{
    {aucune_voie, nord_sud, est_ouest, nord_sud, est_ouest}, // indefinie
    {aucune_voie, nord_sud, nord_est, nord_sud, nord_ouest}, // nord
    {aucune_voie, nord_est, est_ouest, sud_est, est_ouest},  // est
    {aucune_voie, nord_sud, sud_est, nord_sud, sud_ouest},   // sud
    {aucune_voie, nord_ouest, est_ouest, sud_ouest, est_ouest},
}};
// ouest
// ouest
inline constexpr std::array<coin, NBDIR + 1> CoinDir = {aucun_coin, haut_droite, bas_droite, bas_gauche, haut_gauche};
// v3.8
inline constexpr std::array<const char *, NBDIR + 1> asDir = {"", "Nord", "Est", "Sud", "Ouest"};
inline constexpr std::array<const char *, 16> asDirPoss = {
    "",          "Nord",           "Est",           "Nord+Est",           "Sud",       "Nord+Sud",
    "Est+Sud",   "Nord+Est+Sud",   "Ouest",         "Nord+Ouest",         "Est+Ouest", "Nord+Est+Ouest",
    "Sud+Ouest", "Nord+Sud+Ouest", "Est+Sud+Ouest", "Nord+Est+Sud+Ouest",
};
//---------------------------------------------------------------------------
extern bool VerifierApresOuverture, VerifierAvantSimulation;
extern std::array<int, NBDIR + 1> dx, dy;
extern int VersionFichier, // v3.5
    SvgNbFeux, SvgNbParkings, SvgNbVehicules, SvgNbPietons, SvgNbArretsBus,
    SvgNbArretsTram, // v3.5
    SvgNbPlacesTaxi; // v3.6
extern centre_ville *cv;
//---------------------------------------------------------------------------
inline voie &centre_ville::Voisine(int x, int y, int d)
{
    return v[(x + dx[d]) % NbX][(y + dy[d]) % NbY];
}
inline const voie &centre_ville::Voisine(int x, int y, int d) const
{
    return v[(x + dx[d]) % NbX][(y + dy[d]) % NbY];
}
inline voie &centre_ville::Voisine(int x, int y, int n, int d)
{
    return v[(x + (n * dx[d])) % NbX][(y + (n * dy[d])) % NbY];
}
inline const voie &centre_ville::Voisine(int x, int y, int n, int d) const
{
    return v[(x + (n * dx[d])) % NbX][(y + (n * dy[d])) % NbY];
}
//---------------------------------------------------------------------------
extern void AppelleAuChaos();
//---------------------------------------------------------------------------
// v5.4 : conversion BCB6>BCBX (extern après template)
template <class E> extern int IncrementeNombre(std::vector<E> &e, int &n); // v5.1
//---------------------------------------------------------------------------
#endif
