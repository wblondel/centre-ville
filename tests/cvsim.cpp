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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include "cartes.h"
#include "etat.h"
#include "golden.h"
#include "harnais.h"
#include "vue.h"
#include "compat/alea.h"
//---------------------------------------------------------------------------
namespace
 {
  int Usage()
   {
    std::cerr<<"Usage : cvsim CARTE.cvw [--tours N] [--graine G] [--sanglante] [--epidemie] [--resurrection T]\n"
               "                    [--sauve SORTIE.cvw] [--etat]\n"
               "        cvsim --golden DOSSIER_MAPS DOSSIER_GOLDEN\n";
    return 2;
   }
  //-------------------------------------------------------------------------
  std::filesystem::path Chemin(const std::string &s)
   {
    return std::filesystem::path(std::u8string(s.begin(), s.end()));
   }
  //-------------------------------------------------------------------------
  int RegenereGolden(const std::string &asMaps, const std::string &asGolden)
   {
    for(const carte &c : Cartes)
     for(int k=0; k<NB_CONFIGS_GOLDEN; k++)
      {
       std::string Texte=ProduitGolden(asMaps+"/"+c.Fichier, c.Nom, ConfigsGolden[k]);
       std::filesystem::path p=Chemin(asGolden)/Chemin(FichierGolden(c.Nom, ConfigsGolden[k]));
       std::filesystem::create_directories(p.parent_path());
       std::ofstream f(p, std::ios::binary);
       f<<Texte;
       std::cout<<p.string()<<" ("<<Texte.size()<<" octets)\n";
      }
    std::filesystem::path Temp=std::filesystem::temp_directory_path()/"cvsim";
    std::filesystem::create_directories(Temp);
    std::string asTemp=Temp.string();
    for(int k=0; k<NB_CONFIGS_GENERATION; k++)
     {
      std::string Texte=ProduitGoldenGeneration(ConfigsGeneration[k], asTemp+"/generation.cvw");
      std::filesystem::path p=Chemin(asGolden)/"generation"/(std::string(ConfigsGeneration[k].Nom)+".txt");
      std::filesystem::create_directories(p.parent_path());
      std::ofstream(p, std::ios::binary)<<Texte;
      std::cout<<p.string()<<" ("<<Texte.size()<<" octets)\n";
     }
    for(const carte &c : Cartes)
     {
      std::string Texte=ProduitGoldenCopie(asMaps+"/"+c.Fichier, c.Nom, asTemp);
      std::filesystem::path p=Chemin(asGolden)/"copie"/(std::string(c.Nom)+".txt");
      std::filesystem::create_directories(p.parent_path());
      std::ofstream(p, std::ios::binary)<<Texte;
      std::cout<<p.string()<<" ("<<Texte.size()<<" octets)\n";
     }
    return 0;
   }
 }
//---------------------------------------------------------------------------
int main(int argc, char **argv)
 {
  if (argc<2) return Usage();
  if (!std::strcmp(argv[1], "--golden"))
   {
    if (argc!=4) return Usage();
    return RegenereGolden(argv[2], argv[3]);
   }
  std::string asCarte=argv[1], asSortie;
  int NbTours=100, TourResurrection=0;
  bool Etat=false, GraineFixee=false;
  unsigned Graine=0;
  VueNulle Vue;
  for(int i=2; i<argc; i++)
   {
    if (!std::strcmp(argv[i], "--tours")&&(i+1<argc)) NbTours=std::atoi(argv[++i]);
    else if (!std::strcmp(argv[i], "--graine")&&(i+1<argc)) { Graine=(unsigned)std::strtoul(argv[++i], nullptr, 0); GraineFixee=true; }
    else if (!std::strcmp(argv[i], "--sanglante")) Vue.Sanglante=true;
    else if (!std::strcmp(argv[i], "--epidemie")) Vue.Epidemie=true;
    else if (!std::strcmp(argv[i], "--resurrection")&&(i+1<argc)) TourResurrection=std::atoi(argv[++i]);
    else if (!std::strcmp(argv[i], "--sauve")&&(i+1<argc)) asSortie=argv[++i];
    else if (!std::strcmp(argv[i], "--etat")) Etat=true;
    else return Usage();
   }
  vue=&Vue;
  if (GraineFixee) alea::FixeGraine(Graine);
  if (!harnais::Ouvre(asCarte))
   {
    std::cerr<<"Chargement impossible : "<<asCarte<<"\n";
    return 1;
   }
  std::cout<<"Carte "<<asCarte<<" : "<<cv->NbX<<"x"<<cv->NbY<<", version 0x"<<std::hex<<VersionFichier<<std::dec
           <<", "<<cv->NbVehicules<<" véhicules, "<<cv->NbPietons<<" piétons, "<<cv->NbParkings<<" parkings, "
           <<cv->NbFeux<<" feux\n";
  AnsiString asMsgErr;
  if (!harnais::Demarre(asMsgErr))
   {
    std::cerr<<"Démarrage impossible : "<<asMsgErr<<"\n";
    return 1;
   }
  int t;
  for(t=0; t<NbTours; t++)
   {
    if (TourResurrection&&(t+1==TourResurrection)) Vue.Resurrection=true; // comme le golden master
    if (!harnais::Tick())
     {
      std::cout<<"Simulation terminée ("<<cv->NbVehicules<<" véhicules)\n";
      break;
     }
    if (!((t+1)%10)||(t+1==NbTours))
     std::cout<<"Tour "<<cv->TourCrt<<" : "<<cv->NbVehiculesSortis<<" sortis, "<<cv->NbVehiculesArrives
              <<" arrivés, "<<cv->NbVehiculesDeplaces<<" déplacés\n";
   }
  for(const VueNulle::message &m : Vue.Messages)
   std::cout<<"Message : "<<m.asTitre<<" | "<<m.asMessage<<"\n";
  if (Etat) std::cout<<ExportEtat::Texte(cv, true);
  int Retour=0;
  if (!asSortie.empty())
   {
    if (harnais::Sauve(asSortie))
     std::cout<<"Enregistré : "<<asSortie<<"\n";
    else
     {
      std::cerr<<"Enregistrement impossible : "<<asSortie<<"\n";
      Retour=1;
     }
   }
  harnais::Ferme();
  vue=nullptr;
  return Retour;
 }
//---------------------------------------------------------------------------
