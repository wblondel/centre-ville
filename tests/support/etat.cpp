//---------------------------------------------------------------------------
// etat.cpp - Tests du moteur : export de l'état complet de la simulation
//---------------------------------------------------------------------------
#include "etat.h"
#include <cstdio>
#include "compat/alea.h"
//---------------------------------------------------------------------------
const char *NomPartieEtat[NB_PARTIES_ETAT]=
 {
  "centre", "feux", "feuxp", "parkings", "vehicules", "bus",
  "trams", "taxis", "places", "vehlibs", "pietons", "cases", "distances"
 };
//---------------------------------------------------------------------------
std::string Hexa(std::uint64_t v)
 {
  char Tampon[20];
  std::snprintf(Tampon, sizeof(Tampon), "%016llx", (unsigned long long)v);
  return Tampon;
 }
//---------------------------------------------------------------------------
// Puits texte
//---------------------------------------------------------------------------
void PuitsTexte::Section(const char *Nom)
 {
  Texte+="[";
  Texte+=Nom;
  Texte+="]\n";
 }
//---------------------------------------------------------------------------
void PuitsTexte::Objet(const char *Type, int Numero, bool Ignorable)
 {
  Ignore=Ignorable;
  if (Ignore) return;
  Texte+=Type;
  if (Numero>=0) Texte+=" "+std::to_string(Numero);
  Texte+=":";
 }
//---------------------------------------------------------------------------
void PuitsTexte::Champ(const char *Nom, long long Valeur)
 {
  if (Ignore) return;
  Texte+=" ";
  Texte+=Nom;
  Texte+="=";
  Texte+=std::to_string(Valeur);
 }
//---------------------------------------------------------------------------
void PuitsTexte::FinObjet()
 {
  if (!Ignore) Texte+="\n";
  Ignore=false;
 }
//---------------------------------------------------------------------------
// Parcours de l'état
//---------------------------------------------------------------------------
#define C(nom, valeur) s.Champ(nom, (long long)(valeur))
#define CT(nom, tab, n) do { for(int k_=0; k_<(n); k_++) { char n_[48]; if constexpr (P::Noms) std::snprintf(n_, sizeof(n_), "%s%d", nom, k_); else n_[0]=0; s.Champ(n_, (long long)((tab)[k_])); } } while(0)
//---------------------------------------------------------------------------
template <class P> void ExportEtat::ExporteInfections(const pieton *p, P &s)
 {
  int k=0;
  for(const infection *i=p->Infection; i; i=i->Svt, k++)
   {
    const int Valeurs[8]={i->TourDeb, i->TourFin, i->TourFatal, i->NumPrmPieton,
                          i->NumDrnPieton, i->PrmRang, i->DrnRang, i->ChargeVirale};
    const char *Noms[8]={"deb", "fin", "fatal", "prm", "drn", "prmrang", "drnrang", "charge"};
    for(int j=0; j<8; j++)
     {
      char n[48];
      if constexpr (P::Noms) std::snprintf(n, sizeof(n), "inf%d.%s", k, Noms[j]); else n[0]=0;
      s.Champ(n, Valeurs[j]);
     }
   }
  C("nbinf", k);
 }
//---------------------------------------------------------------------------
template <class P> void ExportEtat::Exporte(const centre_ville *cv, partie_etat Partie, P &s, bool Detail)
 {
  int i;
  s.Section(NomPartieEtat[Partie]);
  switch(Partie)
   {
    case peCentreVille:
      s.Objet("centre_ville", -1);
      C("TourCrt", cv->TourCrt); C("TourEnCours", cv->TourEnCours); C("CopieEnCours", cv->CopieEnCours);
      C("NbX", cv->NbX); C("NbY", cv->NbY);
      C("NbFeux", cv->NbFeux); C("NbFeuxPieton", cv->NbFeuxPieton); C("NbParkings", cv->NbParkings);
      C("NbVehicules", cv->NbVehicules); C("NbVehlibs", cv->NbVehlibs); C("NbBus", cv->NbBus);
      C("NbTrams", cv->NbTrams); C("NbPietons", cv->NbPietons);
      C("NbArretsBus", cv->NbArretsBus); C("NbLignesBus", cv->NbLignesBus);
      C("NbArretsTram", cv->NbArretsTram); C("NbLignesTram", cv->NbLignesTram);
      C("NbPlacesTaxi", cv->NbPlacesTaxi); C("NbFilesTaxi", cv->NbFilesTaxi); C("NbTaxis", cv->NbTaxis);
      C("NbPlacesPark", cv->NbPlacesPark); C("NbFilesPark", cv->NbFilesPark); C("NbPlacesVehlib", cv->NbPlacesVehlib);
      C("DureeVert", cv->DureeVert); C("DureeOrange", cv->DureeOrange); C("DureeRouge", cv->DureeRouge);
      C("NbVehiculesDeplaces", cv->NbVehiculesDeplaces); C("NbVehiculesArrives", cv->NbVehiculesArrives);
      C("NbVehiculesSortis", cv->NbVehiculesSortis);
      C("NbPietonsGueris", cv->NbPietonsGueris); C("NbPietonsInfectes", cv->NbPietonsInfectes);
      C("NbPietonsMorts", cv->NbPietonsMorts);
      C("NbToursStats", cv->NbToursStats); C("NbVitesses", cv->NbVitesses);
      C("NbMaxCasesDetectPsgPt", cv->NbMaxCasesDetectPsgPt); C("ProbaDirPsgPt", cv->ProbaDirPsgPt);
      C("NbMaxCasesDetectArretBus", cv->NbMaxCasesDetectArretBus); C("NbMaxCasesDetectArretTram", cv->NbMaxCasesDetectArretTram);
      C("NbMaxCasesDetectFileTaxi", cv->NbMaxCasesDetectFileTaxi); C("NbMaxCasesDetectFileVehlib", cv->NbMaxCasesDetectFileVehlib);
      C("ProbaDirArretBus", cv->ProbaDirArretBus); C("ProbaDirArretTram", cv->ProbaDirArretTram);
      C("ProbaDirFileTaxi", cv->ProbaDirFileTaxi); C("ProbaDirFileVehlib", cv->ProbaDirFileVehlib);
      C("ProbaArretBusDmd", cv->ProbaArretBusDmd); C("ProbaArretTramDmd", cv->ProbaArretTramDmd);
      C("ProbaArretTaxiDmd", cv->ProbaArretTaxiDmd); C("TauxRemplissageVehlib", cv->TauxRemplissageVehlib);
      C("NbMaxPersonnesBus", cv->NbMaxPersonnesBus); C("NbMaxPersonnesTram", cv->NbMaxPersonnesTram);
      C("NbMaxPersonnesTaxi", cv->NbMaxPersonnesTaxi); C("NbMaxPersonnesVehlib", cv->NbMaxPersonnesVehlib);
      C("NbToursStopArretBus", cv->NbToursStopArretBus); C("NbToursStopArretTram", cv->NbToursStopArretTram);
      C("NbMaxToursAttenteBus", cv->NbMaxToursAttenteBus); C("NbMaxToursAttenteTram", cv->NbMaxToursAttenteTram);
      C("NbMaxToursAttenteTaxi", cv->NbMaxToursAttenteTaxi); C("AttenteMaxVeh", cv->AttenteMaxVeh);
      C("NbToursParSeconde", cv->NbToursParSeconde);
      C("NbCasesParcourues", cv->NbCasesParcourues); C("NbToursParcourus", cv->NbToursParcourus);
      C("Xc", cv->Xc); C("Yc", cv->Yc); C("NbXc", cv->NbXc); C("NbYc", cv->NbYc);
      C("LgrMinSgmtRoute", cv->LgrMinSgmtRoute); C("LgrMaxSgmtRoute", cv->LgrMaxSgmtRoute);
      C("ProbaSortieIntersection", cv->ProbaSortieIntersection); C("ProbaEntreeIntersection", cv->ProbaEntreeIntersection);
      C("ProbaTraverseeRoute", cv->ProbaTraverseeRoute);
      C("EpidemieInfectiosite", cv->EpidemieInfectiosite); C("EpidemieIterationPatientZero", cv->EpidemieIterationPatientZero);
      C("EpidemieChargeFatale", cv->EpidemieChargeFatale); C("EpidemieReinfections", cv->EpidemieReinfections);
      C("VerifOk", cv->VerifOk); C("DistancesCalculees", cv->DistancesCalculees);
      C("QuadrillageStats", cv->QuadrillageStats); C("TypeStats", cv->TypeStats);
      C("graine", alea::Graine()); // état du générateur aléatoire
      s.FinObjet();
      if (cv->Stats)
       for(i=0; i<cv->NbToursStats; i++)
        {
         const stats *st=cv->Stats+i;
         s.Objet("stats", i);
         C("deplaces", st->NbVehiculesDeplaces); C("arrives", st->NbVehiculesArrives);
         C("sortis", st->NbVehiculesSortis); C("gueris", st->NbPietonsGueris);
         C("infectes", st->NbPietonsInfectes); C("morts", st->NbPietonsMorts);
         s.FinObjet();
        }
      break;
    case peFeux:
      for(i=0; i<cv->NbFeux; i++)
       {
        const feu *f=cv->Feu+i;
        s.Objet("feu", i+1);
        C("x", f->x); C("y", f->y); C("Dir", f->Dir); C("Etat", f->Etat); C("ProchainEtat", f->ProchainEtat);
        C("NbFeuxDmd", f->NbFeuxDmd); CT("Dmd", f->NumFeuxDmd, f->NbFeuxDmd);
        C("NbFeuxAtt", f->NbFeuxAtt); CT("Att", f->NumFeuxAtt, f->NbFeuxAtt);
        C("NbFeuxPtn", f->NbFeuxPtn); CT("Ptn", f->NumFeuxPtn, f->NbFeuxPtn);
        C("DureeRouge", f->DureeRouge); C("DureeVert", f->DureeVert); C("TourDrnChgEtat", f->TourDrnChgEtat);
        C("TypeDmdVert", f->TypeDmdVert); C("TypePsgRouge", f->TypePsgRouge);
        C("DemandeVert", f->DemandeVert); C("FeuxTousRouges", f->FeuxTousRouges);
        C("RougeDemande", f->RougeDemande); C("ProchainVert", f->ProchainVert); C("VerifEnCours", f->VerifEnCours);
        s.FinObjet();
       }
      break;
    case peFeuxPietons:
      for(i=0; i<cv->NbFeuxPieton; i++)
       {
        const feu_pieton *f=cv->FeuP+i;
        s.Objet("feup", i+1);
        CT("x", f->x, NBFEUXPIETONSPARNUMERO); CT("y", f->y, NBFEUXPIETONSPARNUMERO);
        CT("Dir", f->Dir, NBFEUXPIETONSPARNUMERO); CT("iFeuDir", f->iFeuDir, NBDIR);
        C("Etat", f->Etat); C("ProchainEtat", f->ProchainEtat);
        C("NbFeux", f->NbFeux); CT("Feu", f->NumFeux, f->NbFeux);
        C("DureeRouge", f->DureeRouge); C("DureeVert", f->DureeVert); C("DureeTraversee", f->DureeTraversee);
        C("TourDrnChgEtat", f->TourDrnChgEtat); C("TypeDmdVert", f->TypeDmdVert); C("TypePsgRouge", f->TypePsgRouge);
        C("DemandeVert", f->DemandeVert); C("FeuxTousRouges", f->FeuxTousRouges);
        C("RougeDemande", f->RougeDemande); C("ProchainVert", f->ProchainVert); C("VerifEnCours", f->VerifEnCours);
        s.FinObjet();
       }
      break;
    case peParkings:
      for(i=0; i<cv->NbParkings; i++)
       {
        s.Objet("parking", i+1);
        C("x", cv->Parking[i].x); C("y", cv->Parking[i].y);
        s.FinObjet();
       }
      break;
    case peVehicules:
      if (cv->Vehicule)
       for(i=0; i<cv->NbVehicules; i++)
        {
         const vehicule *v=cv->Vehicule+i;
         s.Objet("vehicule", i+1);
         C("x", v->x); C("y", v->y); C("Dir", v->Dir); C("DirDrnDepl", v->DirDrnDepl); C("NivPriorite", v->NivPriorite);
         C("PkgDep", v->NumParkingDepart); C("PkgArr", v->NumParkingArrivee);
         C("FilePkDep", v->NumFileParkDepart); C("FilePkArr", v->NumFileParkArrivee);
         C("TourDrnDepl", v->TourDrnDepl); C("TourDepart", v->TourDepart);
         C("NbCasesParcourues", v->NbCasesParcourues); C("NbToursRepos", v->NbToursRepos);
         s.FinObjet();
        }
      break;
    case peBus:
      s.Objet("depot_bus", -1);
      C("x", cv->DepotBus.x); C("y", cv->DepotBus.y); C("NumDrnBusSorti", cv->DepotBus.NumDrnBusSorti);
      s.FinObjet();
      if (cv->Bus)
       for(i=0; i<cv->NbBus; i++)
        {
         const bus *b=cv->Bus+i;
         s.Objet("bus", i+1);
         C("x", b->x); C("y", b->y); C("Dir", b->Dir); C("DirDrnDepl", b->DirDrnDepl); C("DirAvtDrnDepl", b->DirAvtDrnDepl);
         C("NumLigne", b->NumLigne); C("NumDrnArret", b->NumDrnArret); C("NumArretSvt", b->NumArretSvt);
         C("NbPersonnes", b->NbPersonnes); C("TourDrnDepl", b->TourDrnDepl); C("TourDepart", b->TourDepart);
         C("TourArriveDrnArret", b->TourArriveDrnArret); C("TourDepartArretSvt", b->TourDepartArretSvt);
         C("NbCasesParcourues", b->NbCasesParcourues); C("NbCasesDepuisDrnArret", b->NbCasesParcouruesDepuisDrnArret);
         C("NbToursRepos", b->NbToursRepos);
         s.FinObjet();
        }
      for(i=0; i<cv->NbArretsBus; i++)
       {
        const arret_bus *a=cv->ArretBus+i;
        s.Objet("arret_bus", i+1);
        C("x", a->x); C("y", a->y); CT("Ligne", a->Ligne, NBMAXLIGNESBUSTRAM);
        C("NbLignes", a->NbLignes); CT("NumLigne", a->NumLigne, NBMAXLIGNESBUSTRAM);
        s.FinObjet();
       }
      for(i=0; i<NBMAXLIGNESBUSTRAM; i++)
       {
        const ligne_bus *l=cv->LigneBus+i;
        s.Objet("ligne_bus", i+1);
        C("NbArrets", l->NbArrets); CT("Arret", l->NumArret, NBMAXARRETSBUSTRAMPARLIGNE);
        C("NbBus", l->NbBus); CT("Bus", l->NumBus, NBMAXBUSTRAMPARLIGNE);
        s.FinObjet();
       }
      break;
    case peTrams:
      if (cv->Tram)
       for(i=0; i<cv->NbTrams; i++)
        {
         const tram *t=cv->Tram+i;
         s.Objet("tram", i+1);
         C("x", t->x); C("y", t->y); C("Dir", t->Dir); C("DirDrnDepl", t->DirDrnDepl); C("DirAvtDrnDepl", t->DirAvtDrnDepl);
         C("NumLigne", t->NumLigne); C("NumDrnArret", t->NumDrnArret); C("NumArretSvt", t->NumArretSvt);
         C("NbPersonnes", t->NbPersonnes); C("TourDrnDepl", t->TourDrnDepl);
         C("TourArriveDrnArret", t->TourArriveDrnArret); C("TourDepartArretSvt", t->TourDepartArretSvt);
         C("NbCasesParcourues", t->NbCasesParcourues); C("NbCasesDepuisDrnArret", t->NbCasesParcouruesDepuisDrnArret);
         C("NbToursRepos", t->NbToursRepos);
         s.FinObjet();
        }
      for(i=0; i<cv->NbArretsTram; i++)
       {
        const arret_tram *a=cv->ArretTram+i;
        s.Objet("arret_tram", i+1);
        C("x", a->x); C("y", a->y); CT("Ligne", a->Ligne, NBMAXLIGNESBUSTRAM);
        C("NbLignes", a->NbLignes); CT("NumLigne", a->NumLigne, NBMAXLIGNESBUSTRAM);
        s.FinObjet();
       }
      for(i=0; i<NBMAXLIGNESBUSTRAM; i++)
       {
        const ligne_tram *l=cv->LigneTram+i;
        s.Objet("ligne_tram", i+1);
        C("NbArrets", l->NbArrets); CT("Arret", l->NumArret, NBMAXARRETSBUSTRAMPARLIGNE);
        C("NbTrams", l->NbTrams); CT("Tram", l->NumTram, NBMAXBUSTRAMPARLIGNE);
        s.FinObjet();
       }
      break;
    case peTaxis:
      for(i=0; i<cv->NbPlacesTaxi; i++)
       {
        s.Objet("place_taxi", i+1);
        C("x", cv->PlaceTaxi[i].x); C("y", cv->PlaceTaxi[i].y); C("NumFile", cv->PlaceTaxi[i].NumFile);
        s.FinObjet();
       }
      if (cv->FileTaxi)
       for(i=0; i<cv->NbFilesTaxi; i++)
        {
         s.Objet("file_taxi", i+1);
         C("NumPlaceQueue", cv->FileTaxi[i].NumPlaceQueue);
         s.FinObjet();
        }
      if (cv->Taxi)
       for(i=0; i<cv->NbTaxis; i++)
        {
         const taxi *t=cv->Taxi+i;
         s.Objet("taxi", i+1);
         C("Situation", t->Situation); C("x", t->x); C("y", t->y); C("NumFile", t->NumFile);
         C("NbPersonnes", t->NbPersonnes); C("NbCourses", t->NbCourses); C("TourDrnDepl", t->TourDrnDepl);
         C("TourDepartDrnCourse", t->TourDepartDrnCourse); C("TourDepotDrnCourse", t->TourDepotDrnCourse);
         C("TourRetourDrnCourse", t->TourRetourDrnCourse); C("NbCasesParcourues", t->NbCasesParcourues);
         C("NbCasesDrnCourse", t->NbCasesParcouruesDrnCourse); C("NbToursRepos", t->NbToursRepos);
         C("Dir", t->Dir); C("DirDrnDepl", t->DirDrnDepl);
         s.FinObjet();
        }
      break;
    case peParkingsPlaces:
      for(i=0; i<cv->NbPlacesPark; i++)
       {
        s.Objet("place_park", i+1);
        C("x", cv->PlacePark[i].x); C("y", cv->PlacePark[i].y); C("NumFile", cv->PlacePark[i].NumFile);
        s.FinObjet();
       }
      if (cv->FilePark)
       for(i=0; i<cv->NbFilesPark; i++)
        {
         s.Objet("file_park", i+1);
         C("NumPlaceQueue", cv->FilePark[i].NumPlaceQueue);
         s.FinObjet();
        }
      break;
    case peVehlibs:
      for(i=0; i<cv->NbPlacesVehlib; i++)
       {
        s.Objet("place_vehlib", i+1);
        C("x", cv->PlaceVehlib[i].x); C("y", cv->PlaceVehlib[i].y);
        C("NumVehlibReserve", cv->PlaceVehlib[i].NumVehlibReserve);
        s.FinObjet();
       }
      if (cv->Vehlib)
       for(i=0; i<cv->NbVehlibs; i++)
        {
         const vehlib *v=cv->Vehlib+i;
         s.Objet("vehlib", i+1);
         C("Situation", v->Situation); C("x", v->x); C("y", v->y); C("Dir", v->Dir); C("DirDrnDepl", v->DirDrnDepl);
         C("NumPlaceDepart", v->NumPlaceDepart); C("NumPlaceArrivee", v->NumPlaceArrivee);
         C("NbPersonnes", v->NbPersonnes); C("TourDrnDepl", v->TourDrnDepl); C("TourDepart", v->TourDepart);
         C("NbCasesParcourues", v->NbCasesParcourues); C("NbToursRepos", v->NbToursRepos);
         s.FinObjet();
        }
      break;
    case pePietons:
      if (cv->Pieton)
       for(i=0; i<cv->NbPietons; i++)
        {
         const pieton *p=cv->Pieton+i;
         s.Objet("pieton", i+1);
         C("x", p->x); C("y", p->y); C("Dir", p->Dir); C("DemiCase", p->DemiCase);
         C("NumArretBus", p->NumArretBus); C("NumArretTram", p->NumArretTram); C("NumFileTaxi", p->NumFileTaxi);
         C("NumPlaceVehlib", p->NumPlaceVehlib); C("NumBus", p->NumBus); C("NumTram", p->NumTram);
         C("NumTaxi", p->NumTaxi); C("NumVehlib", p->NumVehlib); C("NumFeuP", p->NumFeuP);
         C("TourDrnDepl", p->TourDrnDepl); C("TourDrnAff", p->TourDrnAff);
         C("TourAttBus", p->TourAttBus); C("TourAttTram", p->TourAttTram); C("TourAttTaxi", p->TourAttTaxi);
         C("DemiCasesParcourues", p->DemiCasesParcourues);
         C("PsgPtTrv", p->PsgPtTrv); C("ArretBusTrv", p->ArretBusTrv); C("AttendBus", p->AttendBus);
         C("ArretTramTrv", p->ArretTramTrv); C("AttendTram", p->AttendTram); C("AttendFeuP", p->AttendFeuP);
         C("FileTaxiTrv", p->FileTaxiTrv); C("PlaceVehlibTrv", p->PlaceVehlibTrv); C("AttendTaxi", p->AttendTaxi);
         C("AttendVehlib", p->AttendVehlib); C("VeutDescendre", p->VeutDescendre);
         C("Ecrase", p->Ecrase); C("Mort", p->Mort);
         ExporteInfections(p, s);
         s.FinObjet();
        }
      break;
    case peCases:
      for(int x=0; x<cv->NbX; x++)
       for(int y=0; y<cv->NbY; y++)
        {
         const voie *v=&cv->v[x][y];
         int ne=cv->NbParkings;
         bool Vide=false;
         if constexpr (P::Noms)
          { // Cases vides omises du texte (toujours dans l'empreinte)
         bool AttenteTraficNuls=true;
         if (v->Attente&&v->Trafic)
          for(int a=0; AttenteTraficNuls&&(a<=ne); a++)
           for(int b=0; AttenteTraficNuls&&(b<=ne); b++)
            AttenteTraficNuls=(!v->Attente[a][b])&&(!v->Trafic[a][b]);
         Vide=(!v->DirPoss)&&(!v->SensVoieTram)&&(v->Nature==voies)&&(!v->ToutDroit)&&
                   (!v->PassagePietons)&&(!v->PassageBus)&&
                   (!v->NumVehicule)&&(!v->NumVehlib)&&(!v->NumBus)&&(!v->NumTram)&&(!v->NumParking)&&
                   (!v->NumProchVeh)&&(!v->NumProchVL)&&(!v->NumProchBus)&&(!v->NumProchTram)&&(!v->NumProchTaxi)&&
                   (!v->NbPietons)&&(!v->NumArretBus)&&(!v->NumArretTram)&&(!v->NumTaxi)&&(!v->NumPlaceTaxi)&&
                   (!v->NumPlacePark)&&(!v->NumPlaceVehlib)&&(!v->NumSpotVehlib)&&
                   (!v->NumPieton[0])&&(!v->NumPieton[1])&&AttenteTraficNuls;
         for(int k=0; k<NBDIR; k++)
          Vide=Vide&&(!v->Priorite[k])&&(!v->NumFeu[k])&&(!v->NumFeuP[k]);
         for(int k=0; k<NBMAXLIGNESBUSTRAM; k++)
          Vide=Vide&&(!v->LigneBus[k])&&(!v->LigneTram[k]);
          }
         s.Objet("case", x*1000+y, Vide);
         C("x", x); C("y", y);
         C("DirPoss", v->DirPoss); C("SensVoieTram", v->SensVoieTram); CT("Priorite", v->Priorite, NBDIR);
         C("Nature", v->Nature); C("ToutDroit", v->ToutDroit); C("PassagePietons", v->PassagePietons);
         C("PassageBus", v->PassageBus); CT("LigneBus", v->LigneBus, NBMAXLIGNESBUSTRAM);
         CT("LigneTram", v->LigneTram, NBMAXLIGNESBUSTRAM);
         CT("NumFeu", v->NumFeu, NBFEUXPARCASE); CT("NumFeuP", v->NumFeuP, NBFEUXPARCASE);
         C("NumVehicule", v->NumVehicule); C("NumVehlib", v->NumVehlib); C("NumBus", v->NumBus);
         C("NumTram", v->NumTram); C("NumParking", v->NumParking);
         C("NumProchVeh", v->NumProchVeh); C("NumProchVL", v->NumProchVL); C("NumProchBus", v->NumProchBus);
         C("NumProchTram", v->NumProchTram); C("NumProchTaxi", v->NumProchTaxi);
         C("NbPietons", v->NbPietons); C("NumArretBus", v->NumArretBus); C("NumArretTram", v->NumArretTram);
         C("NumTaxi", v->NumTaxi); C("NumPlaceTaxi", v->NumPlaceTaxi); C("NumPlacePark", v->NumPlacePark);
         C("NumPlaceVehlib", v->NumPlaceVehlib); C("NumSpotVehlib", v->NumSpotVehlib);
         CT("NumPieton", v->NumPieton, NBMAXPIETONSPARCASE);
         if (v->Attente&&v->Trafic)
          {
           if (Detail)
            {
             for(int a=0; a<=ne; a++)
              for(int b=0; b<=ne; b++)
               if (v->Attente[a][b]||v->Trafic[a][b])
                {
                 char n[48];
                 if constexpr (P::Noms) std::snprintf(n, sizeof(n), "att%d.%d", a, b); else n[0]=0;
                 s.Champ(n, v->Attente[a][b]);
                 if constexpr (P::Noms) std::snprintf(n, sizeof(n), "tra%d.%d", a, b);
                 s.Champ(n, v->Trafic[a][b]);
                 if constexpr (!P::Noms) s.Champ(n, a*1000+b); // position dans l'empreinte
                }
            }
           else
            { // [0][0] = totaux tous véhicules, incrémentés à chaque mise à jour
             C("att0.0", v->Attente[0][0]);
             C("tra0.0", v->Trafic[0][0]);
            }
          }
         s.FinObjet();
        }
      break;
    case peDistances:
      for(int x=0; x<cv->NbX; x++)
       for(int y=0; y<cv->NbY; y++)
        {
         const voie *v=&cv->v[x][y];
         int nd=NbDir[v->DirPoss],
             ndt=const_cast<voie*>(v)->NbDirPossSensVoieTram(indefinie);
         s.Objet("distances", x*1000+y);
         if (v->DistParking)
          for(int e=0; e<cv->NbParkings; e++)
           for(int d=0; d<nd; d++)
            { C("pk", v->DistParking[e][d][0]); C("pk", v->DistParking[e][d][1]); }
         if (v->DistArretBus)
          for(int e=0; e<cv->NbArretsBus; e++)
           for(int d=0; d<nd; d++) C("ab", v->DistArretBus[e][d]);
         if (v->DistArretTram)
          for(int e=0; e<cv->NbArretsTram; e++)
           for(int d=0; d<ndt; d++) C("at", v->DistArretTram[e][d]);
         if (v->DistFileTaxi)
          for(int e=0; e<cv->NbFilesTaxi; e++)
           for(int d=0; d<nd; d++) C("ft", v->DistFileTaxi[e][d]);
         if (v->DistFilePark)
          for(int e=0; e<cv->NbFilesPark; e++)
           for(int d=0; d<nd; d++)
            { C("fp", v->DistFilePark[e][d][0]); C("fp", v->DistFilePark[e][d][1]); }
         if (v->DistPlaceVehlib)
          for(int e=0; e<cv->NbPlacesVehlib; e++)
           for(int d=0; d<nd; d++) C("pv", v->DistPlaceVehlib[e][d]);
         s.FinObjet();
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
  for(int i=0; i<peDistances; i++)
   Exporte(cv, (partie_etat)i, p, Detail);
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
