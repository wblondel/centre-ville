//---------------------------------------------------------------------------
// etat.cpp - Tests du moteur : export de l'état complet de la simulation
//---------------------------------------------------------------------------
#include "etat.h"
#include <format>
#include <cstdint>
#include <string>
#include "compat/alea.h"
#include "simul.h"
//---------------------------------------------------------------------------
const char *NomPartieEtat[NB_PARTIES_ETAT] = {
    "centre", "feux",   "feuxp",   "parkings", "vehicules", "bus",       "trams",
    "taxis",  "places", "vehlibs", "pietons",  "cases",     "distances",
};
//---------------------------------------------------------------------------
std::string Hexa(std::uint64_t v)
{
    return std::format("{:016x}", v);
}
//---------------------------------------------------------------------------
// Puits texte
//---------------------------------------------------------------------------
void PuitsTexte::Section(const char *Nom)
{
    Texte += '[';
    Texte += Nom;
    Texte += "]\n";
}
//---------------------------------------------------------------------------
void PuitsTexte::Objet(const char *Type, int Numero, bool Ignorable)
{
    Ignore = Ignorable;
    if (Ignore)
    {
        return;
    }
    Texte += Type;
    if (Numero >= 0)
    {
        Texte += " " + std::to_string(Numero);
    }
    Texte += ':';
}
//---------------------------------------------------------------------------
void PuitsTexte::Champ(const char *Nom, long long Valeur)
{
    if (Ignore)
    {
        return;
    }
    Texte += ' ';
    Texte += Nom;
    Texte += '=';
    Texte += std::to_string(Valeur);
}
//---------------------------------------------------------------------------
void PuitsTexte::FinObjet()
{
    if (!Ignore)
    {
        Texte += '\n';
    }
    Ignore = false;
}
//---------------------------------------------------------------------------
// Parcours de l'état
//---------------------------------------------------------------------------
namespace
{
// Un champ : nom (texte) et valeur (empreinte)
template <class P, class T> void C(P &s, const char *Nom, const T &Valeur)
{
    s.Champ(Nom, static_cast<long long>(Valeur));
}
// Un tableau de n champs : nom0, nom1...
template <class P, class T> void CT(P &s, const char *Nom, const T &Tableau, int n)
{
    for (int k = 0; k < n; k++)
    {
        std::string NomCase; // NOLINT(misc-const-correctness) : modifié dans une branche if constexpr
        if constexpr (P::Noms)
        {
            NomCase = Nom + std::to_string(k);
        }
        s.Champ(NomCase.c_str(), static_cast<long long>(Tableau[k]));
    }
}
} // namespace
//---------------------------------------------------------------------------
template <class P> void ExportEtat::ExporteInfections(const pieton *p, P &s)
{
    int k = 0;
    for (const infection *i = p->Infection; i; i = i->Svt, k++)
    {
        const int Valeurs[8] = {
            i->TourDeb,      i->TourFin, i->TourFatal, i->NumPrmPieton,
            i->NumDrnPieton, i->PrmRang, i->DrnRang,   i->ChargeVirale,
        };
        const char *Noms[8] = {"deb", "fin", "fatal", "prm", "drn", "prmrang", "drnrang", "charge"};
        for (int j = 0; j < 8; j++)
        {
            std::string n; // NOLINT(misc-const-correctness) : modifié dans une branche if constexpr
            if constexpr (P::Noms)
            {
                n = std::format("inf{}.{}", k, Noms[j]);
            }
            s.Champ(n.c_str(), Valeurs[j]);
        }
    }
    C(s, "nbinf", k);
}
//---------------------------------------------------------------------------
template <class P> void ExportEtat::Exporte(const centre_ville *cv, partie_etat Partie, P &s, bool Detail)
{
    int i = 0;
    s.Section(NomPartieEtat[Partie]);
    switch (Partie)
    {
    case peCentreVille:
        s.Objet("centre_ville", -1);
        C(s, "TourCrt", cv->TourCrt);
        C(s, "TourEnCours", cv->TourEnCours);
        C(s, "CopieEnCours", cv->CopieEnCours);
        C(s, "NbX", cv->NbX);
        C(s, "NbY", cv->NbY);
        C(s, "NbFeux", cv->NbFeux);
        C(s, "NbFeuxPieton", cv->NbFeuxPieton);
        C(s, "NbParkings", cv->NbParkings);
        C(s, "NbVehicules", cv->NbVehicules);
        C(s, "NbVehlibs", cv->NbVehlibs);
        C(s, "NbBus", cv->NbBus);
        C(s, "NbTrams", cv->NbTrams);
        C(s, "NbPietons", cv->NbPietons);
        C(s, "NbArretsBus", cv->NbArretsBus);
        C(s, "NbLignesBus", cv->NbLignesBus);
        C(s, "NbArretsTram", cv->NbArretsTram);
        C(s, "NbLignesTram", cv->NbLignesTram);
        C(s, "NbPlacesTaxi", cv->NbPlacesTaxi);
        C(s, "NbFilesTaxi", cv->NbFilesTaxi);
        C(s, "NbTaxis", cv->NbTaxis);
        C(s, "NbPlacesPark", cv->NbPlacesPark);
        C(s, "NbFilesPark", cv->NbFilesPark);
        C(s, "NbPlacesVehlib", cv->NbPlacesVehlib);
        C(s, "DureeVert", cv->DureeVert);
        C(s, "DureeOrange", cv->DureeOrange);
        C(s, "DureeRouge", cv->DureeRouge);
        C(s, "NbVehiculesDeplaces", cv->NbVehiculesDeplaces);
        C(s, "NbVehiculesArrives", cv->NbVehiculesArrives);
        C(s, "NbVehiculesSortis", cv->NbVehiculesSortis);
        C(s, "NbPietonsGueris", cv->NbPietonsGueris);
        C(s, "NbPietonsInfectes", cv->NbPietonsInfectes);
        C(s, "NbPietonsMorts", cv->NbPietonsMorts);
        C(s, "NbToursStats", cv->NbToursStats);
        C(s, "NbVitesses", cv->NbVitesses);
        C(s, "NbMaxCasesDetectPsgPt", cv->NbMaxCasesDetectPsgPt);
        C(s, "ProbaDirPsgPt", cv->ProbaDirPsgPt);
        C(s, "NbMaxCasesDetectArretBus", cv->NbMaxCasesDetectArretBus);
        C(s, "NbMaxCasesDetectArretTram", cv->NbMaxCasesDetectArretTram);
        C(s, "NbMaxCasesDetectFileTaxi", cv->NbMaxCasesDetectFileTaxi);
        C(s, "NbMaxCasesDetectFileVehlib", cv->NbMaxCasesDetectFileVehlib);
        C(s, "ProbaDirArretBus", cv->ProbaDirArretBus);
        C(s, "ProbaDirArretTram", cv->ProbaDirArretTram);
        C(s, "ProbaDirFileTaxi", cv->ProbaDirFileTaxi);
        C(s, "ProbaDirFileVehlib", cv->ProbaDirFileVehlib);
        C(s, "ProbaArretBusDmd", cv->ProbaArretBusDmd);
        C(s, "ProbaArretTramDmd", cv->ProbaArretTramDmd);
        C(s, "ProbaArretTaxiDmd", cv->ProbaArretTaxiDmd);
        C(s, "TauxRemplissageVehlib", cv->TauxRemplissageVehlib);
        C(s, "NbMaxPersonnesBus", cv->NbMaxPersonnesBus);
        C(s, "NbMaxPersonnesTram", cv->NbMaxPersonnesTram);
        C(s, "NbMaxPersonnesTaxi", cv->NbMaxPersonnesTaxi);
        C(s, "NbMaxPersonnesVehlib", cv->NbMaxPersonnesVehlib);
        C(s, "NbToursStopArretBus", cv->NbToursStopArretBus);
        C(s, "NbToursStopArretTram", cv->NbToursStopArretTram);
        C(s, "NbMaxToursAttenteBus", cv->NbMaxToursAttenteBus);
        C(s, "NbMaxToursAttenteTram", cv->NbMaxToursAttenteTram);
        C(s, "NbMaxToursAttenteTaxi", cv->NbMaxToursAttenteTaxi);
        C(s, "AttenteMaxVeh", cv->AttenteMaxVeh);
        C(s, "NbToursParSeconde", cv->NbToursParSeconde);
        C(s, "NbCasesParcourues", cv->NbCasesParcourues);
        C(s, "NbToursParcourus", cv->NbToursParcourus);
        C(s, "Xc", cv->Xc);
        C(s, "Yc", cv->Yc);
        C(s, "NbXc", cv->NbXc);
        C(s, "NbYc", cv->NbYc);
        C(s, "LgrMinSgmtRoute", cv->LgrMinSgmtRoute);
        C(s, "LgrMaxSgmtRoute", cv->LgrMaxSgmtRoute);
        C(s, "ProbaSortieIntersection", cv->ProbaSortieIntersection);
        C(s, "ProbaEntreeIntersection", cv->ProbaEntreeIntersection);
        C(s, "ProbaTraverseeRoute", cv->ProbaTraverseeRoute);
        C(s, "EpidemieInfectiosite", cv->EpidemieInfectiosite);
        C(s, "EpidemieIterationPatientZero", cv->EpidemieIterationPatientZero);
        C(s, "EpidemieChargeFatale", cv->EpidemieChargeFatale);
        C(s, "EpidemieReinfections", cv->EpidemieReinfections);
        C(s, "VerifOk", cv->VerifOk);
        C(s, "DistancesCalculees", cv->DistancesCalculees);
        C(s, "QuadrillageStats", cv->QuadrillageStats);
        C(s, "TypeStats", cv->TypeStats);
        C(s, "graine", alea::Graine()); // état du générateur aléatoire
        s.FinObjet();
        if (!cv->Stats.empty())
        {
            for (i = 0; i < cv->NbToursStats; i++)
            {
                const stats *st = &cv->Stats[i];
                s.Objet("stats", i);
                C(s, "deplaces", st->NbVehiculesDeplaces);
                C(s, "arrives", st->NbVehiculesArrives);
                C(s, "sortis", st->NbVehiculesSortis);
                C(s, "gueris", st->NbPietonsGueris);
                C(s, "infectes", st->NbPietonsInfectes);
                C(s, "morts", st->NbPietonsMorts);
                s.FinObjet();
            }
        }
        break;
    case peFeux:
        for (i = 0; i < cv->NbFeux; i++)
        {
            const feu *f = &cv->Feu[i];
            s.Objet("feu", i + 1);
            C(s, "x", f->x);
            C(s, "y", f->y);
            C(s, "Dir", f->Dir);
            C(s, "Etat", f->Etat);
            C(s, "ProchainEtat", f->ProchainEtat);
            C(s, "NbFeuxDmd", f->NbFeuxDmd);
            CT(s, "Dmd", f->NumFeuxDmd, f->NbFeuxDmd);
            C(s, "NbFeuxAtt", f->NbFeuxAtt);
            CT(s, "Att", f->NumFeuxAtt, f->NbFeuxAtt);
            C(s, "NbFeuxPtn", f->NbFeuxPtn);
            CT(s, "Ptn", f->NumFeuxPtn, f->NbFeuxPtn);
            C(s, "DureeRouge", f->DureeRouge);
            C(s, "DureeVert", f->DureeVert);
            C(s, "TourDrnChgEtat", f->TourDrnChgEtat);
            C(s, "TypeDmdVert", f->TypeDmdVert);
            C(s, "TypePsgRouge", f->TypePsgRouge);
            C(s, "DemandeVert", f->DemandeVert);
            C(s, "FeuxTousRouges", f->FeuxTousRouges);
            C(s, "RougeDemande", f->RougeDemande);
            C(s, "ProchainVert", f->ProchainVert);
            C(s, "VerifEnCours", f->VerifEnCours);
            s.FinObjet();
        }
        break;
    case peFeuxPietons:
        for (i = 0; i < cv->NbFeuxPieton; i++)
        {
            const feu_pieton *f = &cv->FeuP[i];
            s.Objet("feup", i + 1);
            CT(s, "x", f->x, NBFEUXPIETONSPARNUMERO);
            CT(s, "y", f->y, NBFEUXPIETONSPARNUMERO);
            CT(s, "Dir", f->Dir, NBFEUXPIETONSPARNUMERO);
            CT(s, "iFeuDir", f->iFeuDir, NBDIR);
            C(s, "Etat", f->Etat);
            C(s, "ProchainEtat", f->ProchainEtat);
            C(s, "NbFeux", f->NbFeux);
            CT(s, "Feu", f->NumFeux, f->NbFeux);
            C(s, "DureeRouge", f->DureeRouge);
            C(s, "DureeVert", f->DureeVert);
            C(s, "DureeTraversee", f->DureeTraversee);
            C(s, "TourDrnChgEtat", f->TourDrnChgEtat);
            C(s, "TypeDmdVert", f->TypeDmdVert);
            C(s, "TypePsgRouge", f->TypePsgRouge);
            C(s, "DemandeVert", f->DemandeVert);
            C(s, "FeuxTousRouges", f->FeuxTousRouges);
            C(s, "RougeDemande", f->RougeDemande);
            C(s, "ProchainVert", f->ProchainVert);
            C(s, "VerifEnCours", f->VerifEnCours);
            s.FinObjet();
        }
        break;
    case peParkings:
        for (i = 0; i < cv->NbParkings; i++)
        {
            s.Objet("parking", i + 1);
            C(s, "x", cv->Parking[i].x);
            C(s, "y", cv->Parking[i].y);
            s.FinObjet();
        }
        break;
    case peVehicules:
        if (!cv->Vehicule.empty())
        {
            for (i = 0; i < cv->NbVehicules; i++)
            {
                const vehicule *v = &cv->Vehicule[i];
                s.Objet("vehicule", i + 1);
                C(s, "x", v->x);
                C(s, "y", v->y);
                C(s, "Dir", v->Dir);
                C(s, "DirDrnDepl", v->DirDrnDepl);
                C(s, "NivPriorite", v->NivPriorite);
                C(s, "PkgDep", v->NumParkingDepart);
                C(s, "PkgArr", v->NumParkingArrivee);
                C(s, "FilePkDep", v->NumFileParkDepart);
                C(s, "FilePkArr", v->NumFileParkArrivee);
                C(s, "TourDrnDepl", v->TourDrnDepl);
                C(s, "TourDepart", v->TourDepart);
                C(s, "NbCasesParcourues", v->NbCasesParcourues);
                C(s, "NbToursRepos", v->NbToursRepos);
                s.FinObjet();
            }
        }
        break;
    case peBus:
        s.Objet("depot_bus", -1);
        C(s, "x", cv->DepotBus.x);
        C(s, "y", cv->DepotBus.y);
        C(s, "NumDrnBusSorti", cv->DepotBus.NumDrnBusSorti);
        s.FinObjet();
        if (!cv->Bus.empty())
        {
            for (i = 0; i < cv->NbBus; i++)
            {
                const bus *b = &cv->Bus[i];
                s.Objet("bus", i + 1);
                C(s, "x", b->x);
                C(s, "y", b->y);
                C(s, "Dir", b->Dir);
                C(s, "DirDrnDepl", b->DirDrnDepl);
                C(s, "DirAvtDrnDepl", b->DirAvtDrnDepl);
                C(s, "NumLigne", b->NumLigne);
                C(s, "NumDrnArret", b->NumDrnArret);
                C(s, "NumArretSvt", b->NumArretSvt);
                C(s, "NbPersonnes", b->NbPersonnes);
                C(s, "TourDrnDepl", b->TourDrnDepl);
                C(s, "TourDepart", b->TourDepart);
                C(s, "TourArriveDrnArret", b->TourArriveDrnArret);
                C(s, "TourDepartArretSvt", b->TourDepartArretSvt);
                C(s, "NbCasesParcourues", b->NbCasesParcourues);
                C(s, "NbCasesDepuisDrnArret", b->NbCasesParcouruesDepuisDrnArret);
                C(s, "NbToursRepos", b->NbToursRepos);
                s.FinObjet();
            }
        }
        for (i = 0; i < cv->NbArretsBus; i++)
        {
            const arret_bus *a = &cv->ArretBus[i];
            s.Objet("arret_bus", i + 1);
            C(s, "x", a->x);
            C(s, "y", a->y);
            CT(s, "Ligne", a->Ligne, NBMAXLIGNESBUSTRAM);
            C(s, "NbLignes", a->NbLignes);
            CT(s, "NumLigne", a->NumLigne, NBMAXLIGNESBUSTRAM);
            s.FinObjet();
        }
        for (i = 0; i < NBMAXLIGNESBUSTRAM; i++)
        {
            const ligne_bus *l = cv->LigneBus + i;
            s.Objet("ligne_bus", i + 1);
            C(s, "NbArrets", l->NbArrets);
            CT(s, "Arret", l->NumArret, NBMAXARRETSBUSTRAMPARLIGNE);
            C(s, "NbBus", l->NbBus);
            CT(s, "Bus", l->NumBus, NBMAXBUSTRAMPARLIGNE);
            s.FinObjet();
        }
        break;
    case peTrams:
        if (!cv->Tram.empty())
        {
            for (i = 0; i < cv->NbTrams; i++)
            {
                const tram *t = &cv->Tram[i];
                s.Objet("tram", i + 1);
                C(s, "x", t->x);
                C(s, "y", t->y);
                C(s, "Dir", t->Dir);
                C(s, "DirDrnDepl", t->DirDrnDepl);
                C(s, "DirAvtDrnDepl", t->DirAvtDrnDepl);
                C(s, "NumLigne", t->NumLigne);
                C(s, "NumDrnArret", t->NumDrnArret);
                C(s, "NumArretSvt", t->NumArretSvt);
                C(s, "NbPersonnes", t->NbPersonnes);
                C(s, "TourDrnDepl", t->TourDrnDepl);
                C(s, "TourArriveDrnArret", t->TourArriveDrnArret);
                C(s, "TourDepartArretSvt", t->TourDepartArretSvt);
                C(s, "NbCasesParcourues", t->NbCasesParcourues);
                C(s, "NbCasesDepuisDrnArret", t->NbCasesParcouruesDepuisDrnArret);
                C(s, "NbToursRepos", t->NbToursRepos);
                s.FinObjet();
            }
        }
        for (i = 0; i < cv->NbArretsTram; i++)
        {
            const arret_tram *a = &cv->ArretTram[i];
            s.Objet("arret_tram", i + 1);
            C(s, "x", a->x);
            C(s, "y", a->y);
            CT(s, "Ligne", a->Ligne, NBMAXLIGNESBUSTRAM);
            C(s, "NbLignes", a->NbLignes);
            CT(s, "NumLigne", a->NumLigne, NBMAXLIGNESBUSTRAM);
            s.FinObjet();
        }
        for (i = 0; i < NBMAXLIGNESBUSTRAM; i++)
        {
            const ligne_tram *l = cv->LigneTram + i;
            s.Objet("ligne_tram", i + 1);
            C(s, "NbArrets", l->NbArrets);
            CT(s, "Arret", l->NumArret, NBMAXARRETSBUSTRAMPARLIGNE);
            C(s, "NbTrams", l->NbTrams);
            CT(s, "Tram", l->NumTram, NBMAXBUSTRAMPARLIGNE);
            s.FinObjet();
        }
        break;
    case peTaxis:
        for (i = 0; i < cv->NbPlacesTaxi; i++)
        {
            s.Objet("place_taxi", i + 1);
            C(s, "x", cv->PlaceTaxi[i].x);
            C(s, "y", cv->PlaceTaxi[i].y);
            C(s, "NumFile", cv->PlaceTaxi[i].NumFile);
            s.FinObjet();
        }
        if (!cv->FileTaxi.empty())
        {
            for (i = 0; i < cv->NbFilesTaxi; i++)
            {
                s.Objet("file_taxi", i + 1);
                C(s, "NumPlaceQueue", cv->FileTaxi[i].NumPlaceQueue);
                s.FinObjet();
            }
        }
        if (!cv->Taxi.empty())
        {
            for (i = 0; i < cv->NbTaxis; i++)
            {
                const taxi *t = &cv->Taxi[i];
                s.Objet("taxi", i + 1);
                C(s, "Situation", t->Situation);
                C(s, "x", t->x);
                C(s, "y", t->y);
                C(s, "NumFile", t->NumFile);
                C(s, "NbPersonnes", t->NbPersonnes);
                C(s, "NbCourses", t->NbCourses);
                C(s, "TourDrnDepl", t->TourDrnDepl);
                C(s, "TourDepartDrnCourse", t->TourDepartDrnCourse);
                C(s, "TourDepotDrnCourse", t->TourDepotDrnCourse);
                C(s, "TourRetourDrnCourse", t->TourRetourDrnCourse);
                C(s, "NbCasesParcourues", t->NbCasesParcourues);
                C(s, "NbCasesDrnCourse", t->NbCasesParcouruesDrnCourse);
                C(s, "NbToursRepos", t->NbToursRepos);
                C(s, "Dir", t->Dir);
                C(s, "DirDrnDepl", t->DirDrnDepl);
                s.FinObjet();
            }
        }
        break;
    case peParkingsPlaces:
        for (i = 0; i < cv->NbPlacesPark; i++)
        {
            s.Objet("place_park", i + 1);
            C(s, "x", cv->PlacePark[i].x);
            C(s, "y", cv->PlacePark[i].y);
            C(s, "NumFile", cv->PlacePark[i].NumFile);
            s.FinObjet();
        }
        if (!cv->FilePark.empty())
        {
            for (i = 0; i < cv->NbFilesPark; i++)
            {
                s.Objet("file_park", i + 1);
                C(s, "NumPlaceQueue", cv->FilePark[i].NumPlaceQueue);
                s.FinObjet();
            }
        }
        break;
    case peVehlibs:
        for (i = 0; i < cv->NbPlacesVehlib; i++)
        {
            s.Objet("place_vehlib", i + 1);
            C(s, "x", cv->PlaceVehlib[i].x);
            C(s, "y", cv->PlaceVehlib[i].y);
            C(s, "NumVehlibReserve", cv->PlaceVehlib[i].NumVehlibReserve);
            s.FinObjet();
        }
        if (!cv->Vehlib.empty())
        {
            for (i = 0; i < cv->NbVehlibs; i++)
            {
                const vehlib *v = &cv->Vehlib[i];
                s.Objet("vehlib", i + 1);
                C(s, "Situation", v->Situation);
                C(s, "x", v->x);
                C(s, "y", v->y);
                C(s, "Dir", v->Dir);
                C(s, "DirDrnDepl", v->DirDrnDepl);
                C(s, "NumPlaceDepart", v->NumPlaceDepart);
                C(s, "NumPlaceArrivee", v->NumPlaceArrivee);
                C(s, "NbPersonnes", v->NbPersonnes);
                C(s, "TourDrnDepl", v->TourDrnDepl);
                C(s, "TourDepart", v->TourDepart);
                C(s, "NbCasesParcourues", v->NbCasesParcourues);
                C(s, "NbToursRepos", v->NbToursRepos);
                s.FinObjet();
            }
        }
        break;
    case pePietons:
        if (!cv->Pieton.empty())
        {
            for (i = 0; i < cv->NbPietons; i++)
            {
                const pieton *p = &cv->Pieton[i];
                s.Objet("pieton", i + 1);
                C(s, "x", p->x);
                C(s, "y", p->y);
                C(s, "Dir", p->Dir);
                C(s, "DemiCase", p->DemiCase);
                C(s, "NumArretBus", p->NumArretBus);
                C(s, "NumArretTram", p->NumArretTram);
                C(s, "NumFileTaxi", p->NumFileTaxi);
                C(s, "NumPlaceVehlib", p->NumPlaceVehlib);
                C(s, "NumBus", p->NumBus);
                C(s, "NumTram", p->NumTram);
                C(s, "NumTaxi", p->NumTaxi);
                C(s, "NumVehlib", p->NumVehlib);
                C(s, "NumFeuP", p->NumFeuP);
                C(s, "TourDrnDepl", p->TourDrnDepl);
                C(s, "TourDrnAff", p->TourDrnAff);
                C(s, "TourAttBus", p->TourAttBus);
                C(s, "TourAttTram", p->TourAttTram);
                C(s, "TourAttTaxi", p->TourAttTaxi);
                C(s, "DemiCasesParcourues", p->DemiCasesParcourues);
                C(s, "PsgPtTrv", p->PsgPtTrv);
                C(s, "ArretBusTrv", p->ArretBusTrv);
                C(s, "AttendBus", p->AttendBus);
                C(s, "ArretTramTrv", p->ArretTramTrv);
                C(s, "AttendTram", p->AttendTram);
                C(s, "AttendFeuP", p->AttendFeuP);
                C(s, "FileTaxiTrv", p->FileTaxiTrv);
                C(s, "PlaceVehlibTrv", p->PlaceVehlibTrv);
                C(s, "AttendTaxi", p->AttendTaxi);
                C(s, "AttendVehlib", p->AttendVehlib);
                C(s, "VeutDescendre", p->VeutDescendre);
                C(s, "Ecrase", p->Ecrase);
                C(s, "Mort", p->Mort);
                ExporteInfections(p, s);
                s.FinObjet();
            }
        }
        break;
    case peCases:
        for (int x = 0; x < cv->NbX; x++)
        {
            for (int y = 0; y < cv->NbY; y++)
            {
                const voie *v = &cv->v[x][y];
                const int ne = cv->NbParkings;
                bool Vide = false; // NOLINT(misc-const-correctness) : modifié dans la branche if constexpr des noms
                if constexpr (P::Noms)
                { // Cases vides omises du texte (toujours dans l'empreinte)
                    bool AttenteTraficNuls = true;
                    if (!v->Attente.empty() && !v->Trafic.empty())
                    {
                        for (int a = 0; AttenteTraficNuls && (a <= ne); a++)
                        {
                            for (int b = 0; AttenteTraficNuls && (b <= ne); b++)
                            {
                                AttenteTraficNuls = (!v->Attente[a][b]) && (!v->Trafic[a][b]);
                            }
                        }
                    }
                    Vide = (!v->DirPoss) && (!v->SensVoieTram) && (v->Nature == voies) && (!v->ToutDroit) &&
                           (!v->PassagePietons) && (!v->PassageBus) && (!v->NumVehicule) && (!v->NumVehlib) &&
                           (!v->NumBus) && (!v->NumTram) && (!v->NumParking) && (!v->NumProchVeh) && (!v->NumProchVL) &&
                           (!v->NumProchBus) && (!v->NumProchTram) && (!v->NumProchTaxi) && (!v->NbPietons) &&
                           (!v->NumArretBus) && (!v->NumArretTram) && (!v->NumTaxi) && (!v->NumPlaceTaxi) &&
                           (!v->NumPlacePark) && (!v->NumPlaceVehlib) && (!v->NumSpotVehlib) && (!v->NumPieton[0]) &&
                           (!v->NumPieton[1]) && AttenteTraficNuls;
                    for (int k = 0; k < NBDIR; k++)
                    {
                        Vide = Vide && (!v->Priorite[k]) && (!v->NumFeu[k]) && (!v->NumFeuP[k]);
                    }
                    for (int k = 0; k < NBMAXLIGNESBUSTRAM; k++)
                    {
                        Vide = Vide && (!v->LigneBus[k]) && (!v->LigneTram[k]);
                    }
                }
                s.Objet("case", (x * 1000) + y, Vide);
                C(s, "x", x);
                C(s, "y", y);
                C(s, "DirPoss", v->DirPoss);
                C(s, "SensVoieTram", v->SensVoieTram);
                CT(s, "Priorite", v->Priorite, NBDIR);
                C(s, "Nature", v->Nature);
                C(s, "ToutDroit", v->ToutDroit);
                C(s, "PassagePietons", v->PassagePietons);
                C(s, "PassageBus", v->PassageBus);
                CT(s, "LigneBus", v->LigneBus, NBMAXLIGNESBUSTRAM);
                CT(s, "LigneTram", v->LigneTram, NBMAXLIGNESBUSTRAM);
                CT(s, "NumFeu", v->NumFeu, NBFEUXPARCASE);
                CT(s, "NumFeuP", v->NumFeuP, NBFEUXPARCASE);
                C(s, "NumVehicule", v->NumVehicule);
                C(s, "NumVehlib", v->NumVehlib);
                C(s, "NumBus", v->NumBus);
                C(s, "NumTram", v->NumTram);
                C(s, "NumParking", v->NumParking);
                C(s, "NumProchVeh", v->NumProchVeh);
                C(s, "NumProchVL", v->NumProchVL);
                C(s, "NumProchBus", v->NumProchBus);
                C(s, "NumProchTram", v->NumProchTram);
                C(s, "NumProchTaxi", v->NumProchTaxi);
                C(s, "NbPietons", v->NbPietons);
                C(s, "NumArretBus", v->NumArretBus);
                C(s, "NumArretTram", v->NumArretTram);
                C(s, "NumTaxi", v->NumTaxi);
                C(s, "NumPlaceTaxi", v->NumPlaceTaxi);
                C(s, "NumPlacePark", v->NumPlacePark);
                C(s, "NumPlaceVehlib", v->NumPlaceVehlib);
                C(s, "NumSpotVehlib", v->NumSpotVehlib);
                CT(s, "NumPieton", v->NumPieton, NBMAXPIETONSPARCASE);
                if (!v->Attente.empty() && !v->Trafic.empty())
                {
                    if (Detail)
                    {
                        for (int a = 0; a <= ne; a++)
                        {
                            for (int b = 0; b <= ne; b++)
                            {
                                if (v->Attente[a][b] || v->Trafic[a][b])
                                {
                                    std::string n; // NOLINT(misc-const-correctness) : modifié dans une branche if
                                                   // constexpr
                                    if constexpr (P::Noms)
                                    {
                                        n = std::format("att{}.{}", a, b);
                                    }
                                    s.Champ(n.c_str(), v->Attente[a][b]);
                                    if constexpr (P::Noms)
                                    {
                                        n = std::format("tra{}.{}", a, b);
                                    }
                                    s.Champ(n.c_str(), v->Trafic[a][b]);
                                    if constexpr (!P::Noms)
                                    {
                                        s.Champ(n.c_str(), (a * 1000) + b); // position dans l'empreinte
                                    }
                                }
                            }
                        }
                    }
                    else
                    { // [0][0] = totaux tous véhicules, incrémentés à chaque mise à jour
                        C(s, "att0.0", v->Attente[0][0]);
                        C(s, "tra0.0", v->Trafic[0][0]);
                    }
                }
                s.FinObjet();
            }
        }
        break;
    case peDistances:
        for (int x = 0; x < cv->NbX; x++)
        {
            for (int y = 0; y < cv->NbY; y++)
            {
                const voie *v = &cv->v[x][y];
                const int nd = NbDir[v->DirPoss];
                const int ndt = v->NbDirPossSensVoieTram(indefinie);
                s.Objet("distances", (x * 1000) + y);
                if (!v->DistParking.empty())
                {
                    for (int e = 0; e < cv->NbParkings; e++)
                    {
                        for (int d = 0; d < nd; d++)
                        {
                            C(s, "pk", v->DistParking[e][d][0]);
                            C(s, "pk", v->DistParking[e][d][1]);
                        }
                    }
                }
                if (!v->DistArretBus.empty())
                {
                    for (int e = 0; e < cv->NbArretsBus; e++)
                    {
                        for (int d = 0; d < nd; d++)
                        {
                            C(s, "ab", v->DistArretBus[e][d]);
                        }
                    }
                }
                if (!v->DistArretTram.empty())
                {
                    for (int e = 0; e < cv->NbArretsTram; e++)
                    {
                        for (int d = 0; d < ndt; d++)
                        {
                            C(s, "at", v->DistArretTram[e][d]);
                        }
                    }
                }
                if (!v->DistFileTaxi.empty())
                {
                    for (int e = 0; e < cv->NbFilesTaxi; e++)
                    {
                        for (int d = 0; d < nd; d++)
                        {
                            C(s, "ft", v->DistFileTaxi[e][d]);
                        }
                    }
                }
                if (!v->DistFilePark.empty())
                {
                    for (int e = 0; e < cv->NbFilesPark; e++)
                    {
                        for (int d = 0; d < nd; d++)
                        {
                            C(s, "fp", v->DistFilePark[e][d][0]);
                            C(s, "fp", v->DistFilePark[e][d][1]);
                        }
                    }
                }
                if (!v->DistPlaceVehlib.empty())
                {
                    for (int e = 0; e < cv->NbPlacesVehlib; e++)
                    {
                        for (int d = 0; d < nd; d++)
                        {
                            C(s, "pv", v->DistPlaceVehlib[e][d]);
                        }
                    }
                }
                s.FinObjet();
            }
        }
        break;
    default:
        break;
    }
}
//---------------------------------------------------------------------------
std::string ExportEtat::Texte(const centre_ville *cv, bool Detail)
{
    PuitsTexte p;
    for (int i = 0; i < peDistances; i++)
    {
        Exporte(cv, static_cast<partie_etat>(i), p, Detail);
    }
    return p.Texte;
}
//---------------------------------------------------------------------------
std::uint64_t ExportEtat::Empreinte(const centre_ville *cv, partie_etat Partie, bool Detail)
{
    PuitsEmpreinte p;
    Exporte(cv, Partie, p, Detail);
    return p.Empreinte;
}
//---------------------------------------------------------------------------
