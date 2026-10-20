//---------------------------------------------------------------------------
// golden.cpp - Tests du moteur : golden master de la simulation
//---------------------------------------------------------------------------
#include "golden.h"
#include <cstdio>
#include <filesystem>
#include "etat.h"
#include "harnais.h"
#include "vue.h"
#include "compat/alea.h"
//---------------------------------------------------------------------------
namespace
 {
  std::string Hexa8(std::uint64_t v)
   { // Empreinte 64 bits repliée sur 32 bits pour la ligne de chaque tour
    char Tampon[12];
    std::snprintf(Tampon, sizeof(Tampon), "%08x", (unsigned)((v>>32)^(v&0xFFFFFFFFu)));
    return Tampon;
   }
  //-------------------------------------------------------------------------
  std::string LigneTour(const centre_ville *cv)
   {
    char Tampon[256];
    std::snprintf(Tampon, sizeof(Tampon),
                  "tour %d graine=%08x sortis=%d arrives=%d deplaces=%d gueris=%d infectes=%d morts=%d |",
                  cv->TourCrt, alea::Graine(), cv->NbVehiculesSortis, cv->NbVehiculesArrives,
                  cv->NbVehiculesDeplaces, cv->NbPietonsGueris, cv->NbPietonsInfectes, cv->NbPietonsMorts);
    std::string Ligne(Tampon);
    // Clés courtes, dans l'ordre de partie_etat (voir l'en-tête des fichiers)
    static const char *Cles[peDistances]={"c", "f", "fp", "pk", "v", "b", "t", "x", "pl", "vl", "p", "k"};
    for(int i=0; i<peDistances; i++)
     Ligne+=std::string(" ")+Cles[i]+"="+Hexa8(ExportEtat::Empreinte(cv, (partie_etat)i, false));
    return Ligne+"\n";
   }
 }
//---------------------------------------------------------------------------
std::string FichierGolden(const std::string &asNomCarte, const config_golden &Config)
 {
  return std::string(Config.Nom)+"/"+asNomCarte+".txt";
 }
//---------------------------------------------------------------------------
std::string ProduitGolden(const std::string &asCheminCarte, const std::string &asNomCarte,
                          const config_golden &Config)
 {
  std::string Texte;
  VueNulle Vue;
  IVueSimulation *VuePrc=vue;
  vue=&Vue;
  Vue.Sanglante=Config.Sanglante;
  Vue.Epidemie=Config.Epidemie;
  alea::FixeGraine(GRAINE_GOLDEN);
  Texte+="# Golden master du moteur Centre-Ville (tests/support/golden.cpp)\n";
  Texte+="# carte="+asNomCarte+" config="+Config.Nom+
         " graine="+std::to_string(GRAINE_GOLDEN)+" tours="+std::to_string(NB_TOURS_GOLDEN)+
         " sanglante="+std::to_string(Config.Sanglante)+" epidemie="+std::to_string(Config.Epidemie)+
         " resurrection="+std::to_string(Config.TourResurrection)+"\n";
  Texte+="# Une ligne par tour : état du générateur aléatoire, compteurs, puis empreinte (64 bits repliée\n"
         "# sur 32 bits) de chaque partie de l'état : c=centre_ville et statistiques, f=feux, fp=feux piétons,\n"
         "# pk=parkings, v=véhicules, b=dépôt/bus/arrêts/lignes, t=trams/arrêts/lignes, x=taxis/places/files,\n"
         "# pl=places et files de parking, vl=vehlibs/places, p=piétons et infections, k=cases.\n"
         "# Diagnostic d'un écart : cvsim <carte> --graine "+std::to_string(GRAINE_GOLDEN)+" --tours <N> --etat"+
         std::string(Config.Sanglante?" --sanglante":"")+std::string(Config.Epidemie?" --epidemie":"")+
         (Config.TourResurrection?" --resurrection "+std::to_string(Config.TourResurrection):std::string())+"\n";
  Texte+="# Régénération (après validation d'un changement VOULU) : cvsim --golden maps tests/golden\n";
  AnsiString asMsgErr;
  if (!harnais::Ouvre(asCheminCarte))
   Texte+="ERREUR chargement\n";
  else
   if (!harnais::Demarre(asMsgErr))
    Texte+="ERREUR demarrage : "+asMsgErr.str()+"\n";
   else
    {
     Texte+="distances="+Hexa(ExportEtat::Empreinte(cv, peDistances, true))+"\n";
     Texte+=LigneTour(cv);
     int t;
     for(t=1; t<=NB_TOURS_GOLDEN; t++)
      {
       if (Config.TourResurrection&&(t==Config.TourResurrection))
        Vue.Resurrection=true; // ActionResurrection (fiche principale)
       if (!harnais::Tick())
        {
         Texte+="simulation terminee\n";
         break;
        }
       Texte+=LigneTour(cv);
      }
     Texte+="messages="+std::to_string(Vue.Messages.size())+"\n";
     for(const VueNulle::message &m : Vue.Messages)
      Texte+="message: "+m.asTitre.str()+" | "+m.asMessage.str()+"\n";
     Texte+="etat final (attente/trafic detailles)="+Hexa(ExportEtat::Empreinte(cv, peCases, true))+"\n";
    }
  harnais::Ferme();
  alea::LibereGraine();
  vue=VuePrc;
  return Texte;
 }
//---------------------------------------------------------------------------
namespace
 {
  std::uint64_t EmpreinteOctets(const std::string &asContenu)
   {
    std::uint64_t h=14695981039346656037ULL;
    for(unsigned char c : asContenu) { h^=c; h*=1099511628211ULL; }
    return h;
   }
  //-------------------------------------------------------------------------
  std::string DescriptionFichier(const std::string &asChemin)
   {
    std::string asContenu=harnais::LitFichier(asChemin);
    return "taille="+std::to_string(asContenu.size())+" empreinte="+Hexa(EmpreinteOctets(asContenu));
   }
 }
//---------------------------------------------------------------------------
std::string ProduitGoldenGeneration(const config_generation &Config, const std::string &asFichierCvw)
 { // f_simul.cpp, ActionNouveauExecute + f_genres.cpp, TfrmGeneration::ButtonOkClick
  std::string Texte;
  VueNulle Vue;
  IVueSimulation *VuePrc=vue;
  vue=&Vue;
  alea::FixeGraine(Config.Graine);
  Texte+="# Golden master de la génération aléatoire de réseau (tests/support/golden.cpp)\n";
  Texte+="# config="+std::string(Config.Nom)+" graine="+std::to_string(Config.Graine)+
         " NbX="+std::to_string(Config.NbX)+" NbY="+std::to_string(Config.NbY)+"\n";
  AppelleAuChaos();
  harnais::Ferme();
  cv=new centre_ville(true, Config.NbX, Config.NbY); // NB_ELEMENTS_PAR_DEFAUT
  // Positions par défaut des TTrackBar de f_genres.dfm
  cv->PrepareGenerationAleatoire(0, 3, 8, 8, 8);
  // f_genres.cpp : cv->Genere(rand()%cv->NbX, rand()%cv->NbY, (direction)(1+rand()%NBDIR));
  // L'ordre d'évaluation des arguments n'est pas spécifié en C++ : on fixe ici
  // x, puis y, puis la direction (à vérifier contre le binaire d'origine).
  int x=rand()%cv->NbX;
  int y=rand()%cv->NbY;
  direction d=(direction)(1+rand()%NBDIR);
  Texte+="depart x="+std::to_string(x)+" y="+std::to_string(y)+" d="+std::to_string((int)d)+"\n";
  cv->Genere(x, y, d);
  char Tampon[16];
  std::snprintf(Tampon, sizeof(Tampon), "%08x", alea::Graine());
  Texte+="graine apres generation="+std::string(Tampon)+"\n";
  Texte+="DirPoss (une ligne par ordonnée, un chiffre hexadécimal par case) :\n";
  for(int j=0; j<cv->NbY; j++)
   {
    for(int i=0; i<cv->NbX; i++)
     {
      std::snprintf(Tampon, sizeof(Tampon), "%x", (int)cv->v[i][j].DirPoss);
      Texte+=Tampon;
     }
    Texte+="\n";
   }
  Texte+="etat="+Hexa(ExportEtat::Empreinte(cv, peCases, true))+"\n";
  if (harnais::SauveSansChaos(asFichierCvw))
   Texte+="fichier "+DescriptionFichier(asFichierCvw)+"\n";
  else
   Texte+="ERREUR enregistrement\n";
  Texte+="messages="+std::to_string(Vue.Messages.size())+"\n";
  harnais::Ferme();
  alea::LibereGraine();
  vue=VuePrc;
  return Texte;
 }
//---------------------------------------------------------------------------
std::string ProduitGoldenCopie(const std::string &asCheminCarte, const std::string &asNomCarte,
                               const std::string &asDossierSortie)
 { // f_simul.cpp, TfrmSimulation::ActionCopierVersExecute (partie moteur)
  std::string Texte;
  VueNulle Vue;
  IVueSimulation *VuePrc=vue;
  vue=&Vue;
  alea::FixeGraine(GRAINE_GOLDEN);
  Texte+="# Golden master de « Copier vers... » (tests/support/golden.cpp)\n";
  Texte+="# carte="+asNomCarte+"\n";
  if (!harnais::Ouvre(asCheminCarte))
   Texte+="ERREUR chargement\n";
  else
   {
    const std::string asNormal=asDossierSortie+"/copie_"+asNomCarte+"_normal.cvw";
    for(const char *Suffixe : {"_complete.cvw", "_reduite.cvw"})
     { // Pas de copie d'une exécution précédente
      std::string asAncienne=asDossierSortie+"/copie_"+asNomCarte+Suffixe;
      std::error_code e;
      std::filesystem::remove(std::filesystem::path(std::u8string(asAncienne.begin(), asAncienne.end())), e);
     }
    harnais::SauveSansChaos(asNormal);
    struct selection { const char *Nom; int x, y, nx, ny; };
    const selection Selections[2]=
     {
      {"complete", 0, 0, cv->NbX, cv->NbY},
      {"reduite", 1, 1, cv->NbX-2, cv->NbY-2},
     };
    for(const selection &s : Selections)
     {
      std::size_t NbMessages=Vue.Messages.size();
      cv->PrepareCopie(DEBUT, s.x, s.y, s.nx, s.ny);
      bool Valide=cv->CopieValide();
      Texte+=std::string("selection ")+s.Nom+" ("+std::to_string(s.x)+","+std::to_string(s.y)+","+
             std::to_string(s.nx)+","+std::to_string(s.ny)+") valide="+std::to_string(Valide)+"\n";
      const std::string asCopie=asDossierSortie+"/copie_"+asNomCarte+"_"+s.Nom+".cvw";
      if (Valide)
       {
        harnais::Sauve(asCopie); // ActionEnregistrer->Execute()
        Texte+="  copie "+DescriptionFichier(asCopie)+"\n";
        Texte+="  identique a Sauve="+std::to_string(harnais::LitFichier(asCopie)==harnais::LitFichier(asNormal))+"\n";
       }
      cv->PrepareCopie(FIN, 0, 0, 0, 0);
      Texte+="  apres FIN : CopieEnCours="+std::to_string(cv->CopieEnCours)+" Xc="+std::to_string(cv->Xc)+
             " Yc="+std::to_string(cv->Yc)+" NbXc="+std::to_string(cv->NbXc)+" NbYc="+std::to_string(cv->NbYc)+"\n";
      for(std::size_t m=NbMessages; m<Vue.Messages.size(); m++)
       Texte+="  message: "+Vue.Messages[m].asTitre.str()+" | "+Vue.Messages[m].asMessage.str()+"\n";
     }
    // La copie réduite se recharge-t-elle sans erreur d'intégrité ?
    const std::string asReduite=asDossierSortie+"/copie_"+asNomCarte+"_reduite.cvw";
    std::string asContenu=harnais::LitFichier(asReduite);
    if (!asContenu.empty())
     {
      harnais::Ferme();
      bool Ok=harnais::Ouvre(asReduite);
      Texte+="rechargement copie reduite="+std::to_string(Ok);
      if (Ok)
       {
        AnsiString asMsgErr;
        cv->Verifie(asMsgErr);
        Texte+=" NbX="+std::to_string(cv->NbX)+" NbY="+std::to_string(cv->NbY)+
               " verifie="+(asMsgErr.IsEmpty()?std::string("ok"):asMsgErr.str());
       }
      Texte+="\n";
     }
   }
  harnais::Ferme();
  alea::LibereGraine();
  vue=VuePrc;
  return Texte;
 }
//---------------------------------------------------------------------------
