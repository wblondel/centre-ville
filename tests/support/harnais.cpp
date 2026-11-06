//---------------------------------------------------------------------------
// harnais.cpp - Tests du moteur : pilotage de la simulation sans interface
//---------------------------------------------------------------------------
#include "harnais.h"
#include <array>
#include <cstdio>
#include "compat/chaine.h"
#include <string>
#include <vector>
#include "compat/fichier.h"
#include "simul.h"
#include "vue.h"
//---------------------------------------------------------------------------
constexpr bool AUCUN_ELEMENT_PAR_DEFAUT = false; // f_simul.cpp
//---------------------------------------------------------------------------
bool harnais::Ouvre(const std::string &asNomFichier)
{ // f_simul.cpp, TfrmSimulation::ActionOuvrirExecute (partie moteur)
    int hfile = 0;
    AppelleAuChaos();
    hfile = FileOpen(asNomFichier.c_str(), fmOpenRead);
    if (hfile < 0)
    {
        return false; // harnais : -1 si échec
    }
    FileSeek(hfile, 0, 0);
    if (cv)
    {
        delete cv;
        cv = nullptr;
    }
    cv = new centre_ville(AUCUN_ELEMENT_PAR_DEFAUT, 0, 0);
    const bool Ok = cv->Charge(hfile);
    FileClose(hfile);
    return Ok;
}
//---------------------------------------------------------------------------
bool harnais::SauveSansChaos(const std::string &asNomFichier)
{
    int hfile = 0;
    hfile = FileCreate(asNomFichier.c_str());
    if (hfile < 0)
    {
        return false;
    }
    const bool Ok = cv->Sauve(hfile);
    FileClose(hfile);
    return Ok;
}
//---------------------------------------------------------------------------
bool harnais::Sauve(const std::string &asNomFichier)
{ // f_simul.cpp, TfrmSimulation::ActionEnregistrerExecute (partie moteur)
    AppelleAuChaos();
    return SauveSansChaos(asNomFichier);
}
//---------------------------------------------------------------------------
bool harnais::VerifieReseau(AnsiString &asMsgErr) // harnais : f_simul.cpp, TfrmSimulation::VerifieReseau()
{
    bool Ok = true;
    int i = 0;
    int j = 0;
    int d = 0;
    int NbDirPossSansPsgBus = 0;
    int NbCasesPietonPeutAller = 0; // v3.5 // v3.8
    const AnsiString asMsgOk = "Réseau correct.";
    asMsgErr = ""; // harnais : variable locale devenue paramètre
    dirposs dp{};
    voie *v = nullptr;
    voie *vv = nullptr;
    cv->VerifOk = false;
    // On enleve le clignotement si actif. v3.5
    // harnais : if (TimerClignotementErreur->Enabled) TimerClignotementErreur->Enabled=false;
    // harnais : if ((xErr>-1)&&(yErr>-1)) RedessineCase(xErr, yErr, true);
    int xErr = -1;
    int yErr = -1; // harnais : membres de TfrmSimulation
    for (i = 0; Ok && (i < cv->NbParkings); i++)
    { // v3.0
        Ok = cv->Parking[i].Defini();
        if (!(Ok))
        {
            asMsgErr = Format("Parking n°%d non défini. Pour renuméroter : sauvez puis rouvrez le réseau.", // v5.1
                              TableauDeConstantes(i + 1, cv->NbParkings));
        }
    }

    for (i = 0; Ok && (i < cv->NbPlacesPark); i++)
    { // v5.0
        Ok = cv->PlacePark[i].Definie();
        if (!(Ok))
        {
            asMsgErr =
                Format("Place de parking n°%d non définie. Pour renuméroter : sauvez puis rouvrez le réseau.", // v5.1
                       TableauDeConstantes(i + 1, cv->NbPlacesPark));
        }
    }

    for (i = 0; Ok && (i < cv->NbPlacesVehlib); i++)
    { // v6.0.7 : oubliées par la v5.3, CalculeDistances lisait alors la case (-1,-1)
        Ok = cv->PlaceVehlib[i].Definie();
        if (!(Ok))
        {
            asMsgErr = Format("Place de vehlib n°%d non définie. Pour renuméroter : sauvez puis rouvrez le réseau.",
                              TableauDeConstantes(i + 1));
        }
    }

    for (i = 0; Ok && (i < NBMAXLIGNESBUSTRAM); i++)
    {
        for (j = 0; Ok && (j < NBMAXARRETSBUSTRAMPARLIGNE); j++)
        {
            if ((j < cv->LigneBus[i].NbArrets) &&
                (!(Ok = cv->ArretBus[cv->LigneBus[i].NumArret[j] - 1].Defini()))) // v3.5 : Oubli du test du nombre
                                                                                  // d'arrêts
            {
                asMsgErr = Format("La ligne de bus n°%d utilise l'arrêt n°%d qui n'est pas défini.",
                                  TableauDeConstantes(i + 1, cv->LigneBus[i].NumArret[j]));
                break;
            }

            if ((j < cv->LigneTram[i].NbArrets) &&
                (!(Ok = cv->ArretTram[cv->LigneTram[i].NumArret[j] - 1].Defini()))) // v3.5 : Oubli du test du nombre
                                                                                    // d'arrêts
            {
                asMsgErr =
                    Format("La ligne de tram n°%s utilise l'arrêt n°%d qui n'est pas défini.",
                           TableauDeConstantes(AnsiString(static_cast<char>('A' + i)), cv->LigneTram[i].NumArret[j]));
                break;
            }
        }
    }
    for (i = 0; Ok && (i < cv->NbArretsBus); i++)
    {
        if (cv->ArretBus[i].Defini() && (!(Ok = (cv->v[cv->ArretBus[i].x][cv->ArretBus[i].y].NumArretBus == i + 1))))
        { // v3.0.3
            xErr = cv->ArretBus[i].x;
            yErr = cv->ArretBus[i].y;
            asMsgErr = Format("La tête de l'arrêt de bus n°%d n'est pas définie ou est incorrecte.",
                              TableauDeConstantes(i + 1));
            break;
        }
    }
    for (i = 0; Ok && (i < cv->NbArretsTram); i++)
    {
        if (cv->ArretTram[i].Defini() &&
            (!(Ok = (cv->v[cv->ArretTram[i].x][cv->ArretTram[i].y].NumArretTram == i + 1))))
        { // v3.0.3
            xErr = cv->ArretTram[i].x;
            yErr = cv->ArretTram[i].y;
            asMsgErr = Format("La tête de l'arrêt de tram n°%d n'est pas définie ou est incorrecte.",
                              TableauDeConstantes(i + 1));
            break;
        }
    }
    if (Ok && (cv->NbArretsBus > 0) && (cv->NbLignesBus > 0)) // v3.0
    {
        Ok = cv->DepotBus.Defini();
        if (!(Ok))
        {
            asMsgErr = "Si des arrêts et des lignes de bus sont définis, un dépôt de bus est obligatoire.";
        }
        else
        {
            Ok = (cv->v[cv->DepotBus.x][cv->DepotBus.y].DirPoss > aucune);
            if (!(Ok))
            {
                xErr = cv->DepotBus.x;
                yErr = cv->DepotBus.y;
                asMsgErr = "Un dépôt de bus doit avoir au moins une direction possible vers l'extérieur.";
            }
            for (d = nord; Ok && (d <= ouest); d++)
            {
                Ok = ((cv->Voisine(cv->DepotBus.x, cv->DepotBus.y, d).DirPoss & InvPossDir[d]) == 0);
                if (!(Ok))
                {
                    xErr = (cv->DepotBus.x + dx[d]) % cv->NbX;
                    yErr = (cv->DepotBus.y + dy[d]) % cv->NbY;
                    asMsgErr = "On ne peut diriger une case vers le dépôt de bus (il est en SORTIE SEULE).";
                }
            }
        }
    }
    for (i = 0; Ok && (i < cv->NbX); i++)
    {
        for (j = 0; Ok && (j < cv->NbY); j++)
        {
            v = &cv->v[i][j];
            NbDirPossSansPsgBus = 0; // v3.5
            for (d = nord; Ok && (d <= ouest); d++)
            { // v5.2.2
                if ((v->DirPoss & PossDir[d]) == PossDir[d])
                {
                    vv = &cv->Voisine(i, j, d);
                    NbDirPossSansPsgBus += static_cast<int>(!vv->PassageBus);
                    dp = vv->DirPoss;
                    if ((dp == InvPossDir[d]) && (vv->NumPlaceVehlib == 0)) // v5.3 sauf Places vehlib pour lesquelles
                                                                            // il faut pouvoir entrer et sortir (pas de
                                                                            // file de vehlib)
                    {
                        Ok = false;
                        xErr = i;
                        yErr = j;
                        asMsgErr = Format("Demi-tour obligé entre les cases (%d, %d) et (%d, %d).",
                                          TableauDeConstantes(i, j, (i + dx[d]) % cv->NbX, (j + dy[d]) % cv->NbY));
                        break;
                    }
                    if (dp == 0)
                    {
                        Ok = false;
                        xErr = i;
                        yErr = j;
                        asMsgErr = Format("La case (%d, %d) pointe sur (%d, %d) qui n'a aucune direction possible.",
                                          TableauDeConstantes(i, j, (i + dx[d]) % cv->NbX, (j + dy[d]) % cv->NbY));
                        break;
                    }
                    if ((!v->PassageBus) && // Si ce n'est pas un passage réservé aux bus ET
                        v->ToutDroit &&     // il y a obligation d'aller tout droit ET
                        vv->PassageBus &&   // la case voisine est réservée aux bus ET
                        ((cv->Voisine(i, j, InvDir[d]).DirPoss & PossDir[d]) != 0) && // On peut venir de la case
                                                                                      // opposée
                        (!cv->Voisine(i, j, InvDir[d]).PassageBus)) // qui n'est pas réservé aux bus ALORS...
                    {                                               // ...Il y a possibilité de blocage. v3.5
                        Ok = false;
                        xErr = i;
                        yErr = j;
                        asMsgErr = Format("L'obligation d'aller tout droit sur la case (%d, %d) oblige à aller sur le "
                                          "passage bus %s.",
                                          TableauDeConstantes(i, j, asDir[d]));
                        break;
                    }
                }
                // v5.2.2 : On vérifie les voies de tram !
                if (v->EstDirPossSensVoieTram(indefinie, direction(d)) &&
                    (!cv->Voisine(i, j, d).EstDirPossSensVoieTram(indefinie, InvDir[d])))
                { // Il y a une voie de tram allant/venant de d MAIS dans cette direction s il N'y a PAS de voie de tram
                  // dans la direction opposée
                    Ok = false;
                    xErr = i;
                    yErr = j;
                    asMsgErr = Format("La voie de tram (%d, %d) est un cul-de-sac dans la direction %s. Le réseau des "
                                      "voies de tram doit toujours boucler.",
                                      TableauDeConstantes(i, j, asDir[d]));
                    break;
                }
            } // v5.2.2
            if (!Ok)
            {
                break; // v3.5 (oubli !)
            }
            if ((v->DirPoss != 0) && (!v->PassageBus) && (NbDirPossSansPsgBus == 0)) // v3.5
            {
                Ok = false;
                xErr = i;
                yErr = j;
                asMsgErr = Format("La case (%d, %d) n'a aucune direction possible en dehors des passages de bus.",
                                  TableauDeConstantes(i, j));
                break;
            }
            if (v->NumParking != 0)
            {
                if (v->DirPoss == 0)
                {
                    Ok = false;
                    xErr = i;
                    yErr = j;
                    asMsgErr = Format("La case (%d, %d) est un parking mais n'a aucune direction possible.",
                                      TableauDeConstantes(i, j));
                    break;
                }
                for (d = nord; (d <= ouest) && ((cv->Voisine(i, j, d).DirPoss & InvPossDir[d]) == 0); d++)
                {
                    ;
                }
                if (d > ouest)
                {
                    Ok = false;
                    xErr = i;
                    yErr = j;
                    asMsgErr = Format("La case (%d, %d) est un parking mais il est impossible d'y accéder.",
                                      TableauDeConstantes(i, j));
                    break;
                }
            }
            if (v->NumArretBus != 0) // v3.0
            {
                for (Ok = false, d = nord; (!Ok) && (d <= ouest); d++)
                {
                    Ok = (cv->Voisine(i, j, d).NumArretBus == v->NumArretBus);
                }
                if (!Ok)
                {
                    xErr = i;
                    yErr = j;
                    asMsgErr = Format("L'arrêt de bus n°%d n'est défini que sur la case (%d, %d).",
                                      TableauDeConstantes(v->NumArretBus, i, j));
                    break;
                }
            }
            if (v->NumArretTram != 0) // v3.5
            {
                for (Ok = false, d = nord; (!Ok) && (d <= ouest); d++)
                {
                    Ok = (cv->Voisine(i, j, d).NumArretTram == v->NumArretTram);
                }
                if (!Ok)
                {
                    xErr = i;
                    yErr = j;
                    asMsgErr = Format("L'arrêt de tram n°%d n'est défini que sur la case (%d, %d).",
                                      TableauDeConstantes(v->NumArretTram, i, j));
                    break;
                }
            }
            if (v->PassagePietons && (cv->SensPassagePietonsPossible(i, j) == 0))
            {
                Ok = false;
                xErr = i;
                yErr = j;

                asMsgErr = Format("Le passage piéton sur la case (%d, %d) ne peut avoir de sens. Il a été retiré.",
                                  TableauDeConstantes(i, j));
                v->PassagePietons = false;
                vue->DefinitSimulationModifiee(true); // harnais : SimulationModifiee=true;
                break;
            }
            if ((v->NumPlaceTaxi != 0) && v->ExistePlusieursQueuesFileTaxis()) // v3.6
            {
                Ok = false;
                xErr = i;
                yErr = j;
                asMsgErr = Format("Plusieurs files de taxi sont dirigées sur la place de taxi de la case (%d, %d).",
                                  TableauDeConstantes(i, j));
                break;
            }
            if ((v->NumPlaceTaxi != 0) && v->ExistePlusieursTetesFileTaxis()) // v3.6
            {
                Ok = false;
                xErr = i;
                yErr = j;
                asMsgErr = Format("La place de taxi de la case (%d, %d) pointe sur plus d'une file de taxis.",
                                  TableauDeConstantes(i, j));
                break;
            }
            if ((v->NumPlacePark != 0) && v->ExistePlusieursQueuesFilePark()) // v5.0
            {
                Ok = false;
                xErr = i;
                yErr = j;
                asMsgErr =
                    Format("Plusieurs files de parkings sont dirigées sur la place de parking de la case (%d, %d).",
                           TableauDeConstantes(i, j));
                break;
            }
            if ((v->NumPlacePark != 0) && v->ExistePlusieursTetesFilePark()) // v5.0
            {
                Ok = false;
                xErr = i;
                yErr = j;
                asMsgErr = Format("La place de parking de la case (%d, %d) pointe sur plus d'une file de parking.",
                                  TableauDeConstantes(i, j));
                break;
            }
            // Vérification du nombre de cases potentielles avec 1 piéton par case minimal. v3.8
            for (d = nord; (d <= ouest) && (!v->PietonPeutAller(static_cast<direction>(d))); d++)
            {
                ; // v6.0.2 : borne testée avant, comme f_simul.cpp
            }
            NbCasesPietonPeutAller += static_cast<int>(d <= ouest);
        }
    }
    if (Ok && (NbCasesPietonPeutAller < cv->NbPietons)) // v4.2
    {
        Ok = false;
        asMsgErr = Format("Il n'y a pas assez de place pour %d piétons (%d possibles). Ajoutez des trottoirs.",
                          TableauDeConstantes(cv->NbPietons, NbCasesPietonPeutAller));
    }
    cv->VerifOk = Ok; // harnais : StatusBar->Panels->Items[0]->Text=(cv->VerifOk=Ok)?asMsgOk:asMsgErr;
    (void)xErr;
    (void)yErr;
    (void)asMsgOk; // harnais : if ((xErr>-1)&&(yErr>-1))
    // harnais :  TimerClignotementErreur->Enabled=true;
    // harnais : Cursor=crDefault;
    return Ok;
}
//---------------------------------------------------------------------------
bool harnais::Demarre(AnsiString &asMsgErr)
{ // f_simul.cpp, TfrmSimulation::ActionMarcheExecute (cas !Pause)
    type_distance TypeDistance = type_indefini;
    int i = 0;
    asMsgErr = "";
    if (!VerifieReseau(asMsgErr))
    {
        return false;
    }
    for (i = 0; (i < cv->NbParkings) && cv->Parking[i].Defini(); i++)
    {
        ;
    }
    if (i < cv->NbParkings)
    {
        asMsgErr = "Nombre de parkings insuffisant";
        return false;
    }
    cv->DefinitFilesTaxi();
    cv->DefinitFilesPark();
    if (!cv->PrepareCalculDistances())
    {
        asMsgErr = "PrepareCalculDistances";
        return false;
    }
    if (!cv->PrepareCalculAttenteTrafic())
    {
        asMsgErr = "PrepareCalculAttenteTrafic";
        return false;
    }
    cv->CalculeDistances();
    if (!cv->VerifieCalculDistances(TypeDistance))
    {
        asMsgErr = Format("Distance indéfinie (type %d) en (%d, %d)",
                          TableauDeConstantes(static_cast<int>(TypeDistance), cv->xErr, cv->yErr));
        return false;
    }
    cv->InitialiseInfosTrajetLignesBus();
    cv->CalculeTrajetLignesBus();
    cv->CalculeTrajetLignesTram();
    cv->DistancesCalculees = true;
    if (!cv->PrepareVehicules())
    {
        asMsgErr = "PrepareVehicules";
        return false;
    }
    if (!cv->PrepareBus())
    {
        asMsgErr = "PrepareBus";
        return false;
    }
    if (!cv->PrepareTrams())
    {
        asMsgErr = "PrepareTrams";
        return false;
    }
    if (!cv->PrepareTaxis())
    {
        asMsgErr = "PrepareTaxis";
        return false;
    }
    if (!cv->PrepareVehlibs())
    {
        asMsgErr = "PrepareVehlibs";
        return false;
    }
    if (!cv->PreparePietons())
    {
        asMsgErr = "PreparePietons";
        return false;
    }
    cv->PrepareFeuxPietons();
    cv->NbVehiculesSortis = cv->NbVehiculesDeplaces = cv->NbVehiculesArrives = 0L;
    cv->NbPietonsGueris = cv->NbPietonsInfectes = cv->NbPietonsMorts = 0L;
    // VerifierAvantSimulation : faux par défaut
    // ActionEnregistrer->Execute() : seul effet sur le moteur
    AppelleAuChaos();
    cv->Stats = std::vector<stats>(cv->NbToursStats);
    cv->Stats[0].Definit();
    return true;
}
//---------------------------------------------------------------------------
bool harnais::Tick()
{ // f_simul.cpp, TfrmSimulation::TimerSimulTimer
    if (cv->TourEnCours)
    {
        return true;
    }
    if (cv->NbVehiculesArrives == cv->NbVehicules)
    {
        return false; // « Simulation terminée » : TypeDemande=tdArret
    }
    cv->EffectueTourSimulation();
    return true;
}
//---------------------------------------------------------------------------
void harnais::Ferme()
{
    if (cv)
    {
        delete cv;
        cv = nullptr;
    }
}
//---------------------------------------------------------------------------
std::string harnais::LitFichier(const std::string &asNomFichier)
{
    std::string Contenu;
    const int hfile = FileOpen(asNomFichier.c_str(), fmOpenRead);
    if (hfile < 0)
    {
        return Contenu;
    }
    std::array<char, 65536> Tampon{};
    int n = 0;
    while ((n = FileRead(hfile, Tampon.data(), sizeof(Tampon))) > 0)
    {
        Contenu.append(Tampon.data(), static_cast<std::size_t>(n));
    }
    FileClose(hfile);
    return Contenu;
}
//---------------------------------------------------------------------------
