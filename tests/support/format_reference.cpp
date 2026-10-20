//---------------------------------------------------------------------------
// format_reference.cpp - Tests du moteur : spécification exécutable du .cvw
//
// Toutes les valeurs sont petit-boutistes. int = 4 octets, bool = 1 octet,
// énumérés = 1 octet, nature = 4 octets.
//---------------------------------------------------------------------------
#include "format_reference.h"
#include <cstdint>
#include <cstring>
#include <stdexcept>
//---------------------------------------------------------------------------
namespace
 {
  const int NB_LIGNES=9, NB_ARRETS_PAR_LIGNE=9, NB_DIR=4;
  //-------------------------------------------------------------------------
  class lecteur
   {
    const std::string &s;
    std::size_t Pos=0;
    public:
    explicit lecteur(const std::string &Contenu) : s(Contenu) {}
    std::string Octets(std::size_t n)
     {
      if (Pos+n>s.size()) throw std::runtime_error("fin de fichier prématurée à l'octet "+std::to_string(Pos));
      std::string r=s.substr(Pos, n);
      Pos+=n;
      return r;
     }
    std::int32_t Entier()
     {
      std::string o=Octets(4);
      return (std::int32_t)((std::uint32_t)(unsigned char)o[0]|((std::uint32_t)(unsigned char)o[1]<<8)|
                            ((std::uint32_t)(unsigned char)o[2]<<16)|((std::uint32_t)(unsigned char)o[3]<<24));
     }
    std::string Octet() { return Octets(1); }
    bool Fin() const { return Pos==s.size(); }
    std::size_t Position() const { return Pos; }
   };
  //-------------------------------------------------------------------------
  class ecrivain
   {
    public:
    conversion_cvw &c;
    explicit ecrivain(conversion_cvw &Conversion) : c(Conversion) {}
    void Brut(const std::string &Nom, const std::string &o)
     {
      c.Champs.push_back(champ_cvw{c.Octets.size(), o.size(), Nom});
      c.Octets+=o;
     }
    void Entier(const std::string &Nom, std::int32_t v)
     {
      std::uint32_t u=(std::uint32_t)v;
      std::string o;
      for(int i=0; i<4; i++) o+=(char)((u>>(8*i))&0xFF);
      Brut(Nom, o);
     }
    void Octet(const std::string &Nom, int v) { Brut(Nom, std::string(1, (char)v)); }
   };
  //-------------------------------------------------------------------------
  std::string Indice(const std::string &Nom, int i)
   {
    return Nom+"["+std::to_string(i)+"]";
   }
 }
//---------------------------------------------------------------------------
conversion_cvw ConvertitEn530(const std::string &asOrigine)
 {
  conversion_cvw c;
  lecteur l(asOrigine);
  ecrivain e(c);
  try
   {
    // --- En-tête (centre_ville::Charge / Sauve)
    int v=l.Entier();
    if (v<0x300) throw std::runtime_error("version non stockée (< 0x300) : non prise en charge");
    if (v>0x530) throw std::runtime_error("version ultérieure à 0x530");
    c.Version=v;
    e.Entier("Version", 0x530);
    int NbFeux=l.Entier();
    e.Entier("NbFeux", NbFeux);
    int NbX=99, NbY=63;
    if (v>=0x400) { NbX=l.Entier(); NbY=l.Entier(); }
    e.Entier("NbX", NbX);
    e.Entier("NbY", NbY);
    int NbFeuxPieton=(v>=0x520)?l.Entier():0;
    e.Entier("NbFeuxPieton", NbFeuxPieton);
    int NbParkings=l.Entier();
    e.Entier("NbParkings", NbParkings);
    e.Entier("NbVehicules", l.Entier());
    e.Entier("NbPietons", l.Entier());
    int NbArretsBus=l.Entier();
    e.Entier("NbArretsBus", NbArretsBus);
    e.Entier("NbLignesBus", l.Entier());
    int NbArretsTram=0, NbLignesTram=0;
    if (v>=0x350) { NbArretsTram=l.Entier(); NbLignesTram=l.Entier(); }
    e.Entier("NbArretsTram", NbArretsTram);
    e.Entier("NbLignesTram", NbLignesTram);
    int NbPlacesTaxi=0, NbTaxis=0;
    if (v>=0x360) { NbPlacesTaxi=l.Entier(); NbTaxis=l.Entier(); }
    e.Entier("NbPlacesTaxi", NbPlacesTaxi);
    e.Entier("NbTaxis", NbTaxis);
    int NbPlacesPark=(v>=0x500)?l.Entier():0;
    e.Entier("NbPlacesPark", NbPlacesPark);
    int NbPlacesVehlib=0,
        NbVehlibs=0; // < 0x530 : champ non initialisé par le constructeur (0 dans les tests)
    if (v>=0x530) { NbPlacesVehlib=l.Entier(); NbVehlibs=l.Entier(); }
    e.Entier("NbPlacesVehlib", NbPlacesVehlib);
    e.Entier("NbVehlibs", NbVehlibs);
    const char *Parametres[]={"DureeVert", "DureeOrange", "DureeRouge", "AttenteMaxVeh", "NbToursParSeconde", "NbToursStats"};
    for(const char *p : Parametres) e.Entier(p, l.Entier());
    e.Brut("QuadrillageStats", l.Octet());
    e.Brut("TypeStats", l.Octet());
    e.Entier("NbVitesses", l.Entier());
    e.Entier("NbMaxCasesDetectPsgPt", l.Entier());
    e.Entier("ProbaDirPsgPt", l.Entier());
    const char *ParamBus[]={"NbMaxCasesDetectArretBus", "ProbaDirArretBus", "ProbaArretBusDmd",
                            "NbMaxPersonnesBus", "NbToursStopArretBus", "NbMaxToursAttenteBus"};
    for(const char *p : ParamBus) e.Entier(p, l.Entier());
    // Valeurs par défaut du constructeur de centre_ville pour les champs absents
    const char *ParamTram[]={"NbMaxCasesDetectArretTram", "ProbaDirArretTram", "ProbaArretTramDmd",
                             "NbMaxPersonnesTram", "NbToursStopArretTram", "NbMaxToursAttenteTram"};
    const int DefautTram[]={3, 50, 10, 100, 3, 25};
    for(int i=0; i<6; i++) e.Entier(ParamTram[i], (v>=0x350)?l.Entier():DefautTram[i]);
    const char *ParamTaxi[]={"NbMaxCasesDetectFileTaxi", "ProbaDirFileTaxi", "ProbaArretTaxiDmd",
                             "NbMaxPersonnesTaxi", "NbMaxToursAttenteTaxi"};
    const int DefautTaxi[]={3, 50, 10, 3, 25};
    for(int i=0; i<5; i++) e.Entier(ParamTaxi[i], (v>=0x360)?l.Entier():DefautTaxi[i]);
    const char *ParamVehlib[]={"NbMaxPersonnesVehlib", "NbMaxCasesDetectFileVehlib", "ProbaDirFileVehlib",
                               "TauxRemplissageVehlib"};
    const int DefautVehlib[]={3, 3, 50, 80};
    for(int i=0; i<4; i++) e.Entier(ParamVehlib[i], (v>=0x530)?l.Entier():DefautVehlib[i]);
    // --- Feux (feu::Charge / Sauve)
    for(int f=1; f<=NbFeux; f++)
     {
      std::string n="feu"+std::to_string(f)+".";
      e.Entier(n+"Numero", l.Entier());
      e.Entier(n+"x", l.Entier());
      e.Entier(n+"y", l.Entier());
      e.Entier(n+"Dir", l.Entier());
      e.Brut(n+"Etat", l.Octet());
      int NbDmd=l.Entier(), NbAtt=l.Entier();
      e.Entier(n+"NbFeuxDmd", NbDmd);
      e.Entier(n+"NbFeuxAtt", NbAtt);
      e.Brut(n+"TypeDmdVert", l.Octet());
      e.Brut(n+"TypePsgRouge", l.Octet());
      e.Entier(n+"DureeRouge", l.Entier());
      e.Entier(n+"DureeVert", l.Entier());
      e.Brut(n+"DemandeVert", l.Octet());
      e.Brut(n+"RougeDemande", l.Octet());
      for(int i=0; i<NbDmd; i++) e.Entier(Indice(n+"NumFeuxDmd", i), l.Entier());
      for(int i=0; i<NbAtt; i++) e.Entier(Indice(n+"NumFeuxAtt", i), l.Entier());
     }
    // --- Feux piétons (feu_pieton::Charge / Sauve), v5.2
    for(int f=1; f<=NbFeuxPieton; f++)
     {
      std::string n="feup"+std::to_string(f)+".";
      e.Entier(n+"Numero", l.Entier());
      for(int i=0; i<2; i++) e.Entier(Indice(n+"x", i), l.Entier());
      for(int i=0; i<2; i++) e.Entier(Indice(n+"y", i), l.Entier());
      for(int i=0; i<2; i++) e.Entier(Indice(n+"Dir", i), l.Entier());
      for(int i=0; i<NB_DIR; i++) e.Entier(Indice(n+"iFeuDir", i), l.Entier());
      e.Brut(n+"Etat", l.Octet());
      int Nb=l.Entier();
      e.Entier(n+"NbFeux", Nb);
      e.Brut(n+"TypeDmdVert", l.Octet());
      e.Brut(n+"TypePsgRouge", l.Octet());
      e.Entier(n+"DureeRouge", l.Entier());
      e.Entier(n+"DureeVert", l.Entier());
      e.Brut(n+"DemandeVert", l.Octet());
      e.Brut(n+"RougeDemande", l.Octet());
      for(int i=0; i<Nb; i++) e.Entier(Indice(n+"NumFeux", i), l.Entier());
     }
    // --- Parkings
    for(int p=1; p<=NbParkings; p++)
     {
      std::string n="parking"+std::to_string(p)+".";
      e.Entier(n+"Numero", l.Entier());
      e.Entier(n+"x", l.Entier());
      e.Entier(n+"y", l.Entier());
     }
    // --- Dépôt, arrêts et lignes de bus (v3.0)
    e.Entier("depot.x", l.Entier());
    e.Entier("depot.y", l.Entier());
    for(int a=1; a<=NbArretsBus; a++)
     {
      std::string n="arret_bus"+std::to_string(a)+".";
      e.Entier(n+"Numero", l.Entier());
      e.Entier(n+"x", l.Entier());
      e.Entier(n+"y", l.Entier());
      for(int i=0; i<NB_LIGNES; i++) e.Brut(Indice(n+"Ligne", i), l.Octet());
      e.Entier(n+"NbLignes", l.Entier());
      for(int i=0; i<NB_LIGNES; i++) e.Entier(Indice(n+"NumLigne", i), l.Entier());
     }
    for(int k=1; k<=NB_LIGNES; k++)
     {
      std::string n="ligne_bus"+std::to_string(k)+".";
      e.Entier(n+"Numero", l.Entier());
      e.Entier(n+"NbArrets", l.Entier());
      for(int i=0; i<NB_ARRETS_PAR_LIGNE; i++) e.Entier(Indice(n+"NumArret", i), l.Entier());
     }
    // --- Arrêts et lignes de tram (v3.5)
    for(int a=1; a<=NbArretsTram; a++)
     {
      std::string n="arret_tram"+std::to_string(a)+".";
      e.Entier(n+"Numero", l.Entier());
      e.Entier(n+"x", l.Entier());
      e.Entier(n+"y", l.Entier());
      for(int i=0; i<NB_LIGNES; i++) e.Brut(Indice(n+"Ligne", i), l.Octet());
      e.Entier(n+"NbLignes", l.Entier());
      for(int i=0; i<NB_LIGNES; i++) e.Entier(Indice(n+"NumLigne", i), l.Entier());
     }
    for(int k=1; k<=NB_LIGNES; k++)
     {
      std::string n="ligne_tram"+std::to_string(k)+".";
      if (v>=0x350)
       {
        e.Entier(n+"Numero", l.Entier());
        e.Entier(n+"NbArrets", l.Entier());
        for(int i=0; i<NB_ARRETS_PAR_LIGNE; i++) e.Entier(Indice(n+"NumArret", i), l.Entier());
       }
      else
       { // Lignes initialisées par le constructeur (AffecteNumero + Initialise)
        e.Entier(n+"Numero", k);
        e.Entier(n+"NbArrets", 0);
        for(int i=0; i<NB_ARRETS_PAR_LIGNE; i++) e.Entier(Indice(n+"NumArret", i), 0);
       }
     }
    // --- Places de taxi (v3.6), de parking (v5.0), de vehlib (v5.3)
    const char *Places[]={"place_taxi", "place_park", "place_vehlib"};
    const int NbPlaces[]={NbPlacesTaxi, NbPlacesPark, NbPlacesVehlib};
    for(int t=0; t<3; t++)
     for(int p=1; p<=NbPlaces[t]; p++)
      {
       std::string n=std::string(Places[t])+std::to_string(p)+".";
       e.Entier(n+"Numero", l.Entier());
       e.Entier(n+"x", l.Entier());
       e.Entier(n+"y", l.Entier());
      }
    // --- Cases (voie::Charge / Sauve), x puis y
    for(int x=0; x<NbX; x++)
     for(int y=0; y<NbY; y++)
      {
       std::string n="case("+std::to_string(x)+","+std::to_string(y)+").";
       e.Entier(n+"x", l.Entier());
       e.Entier(n+"y", l.Entier());
       e.Brut(n+"DirPoss", l.Octet());
       if (v>=0x350) e.Brut(n+"SensVoieTram", l.Octet()); else e.Octet(n+"SensVoieTram", 0);
       e.Entier(n+"Nature", (v>=0x380)?l.Entier():0);
       e.Entier(n+"NumParking", l.Entier());
       e.Brut(n+"ToutDroit", l.Octet());
       e.Brut(n+"PassagePietons", l.Octet());
       e.Brut(n+"PassageBus", l.Octet());
       e.Entier(n+"NumArretBus", l.Entier());
       e.Entier(n+"NumArretTram", (v>=0x350)?l.Entier():0);
       e.Entier(n+"NumPlaceTaxi", (v>=0x360)?l.Entier():0);
       e.Entier(n+"NumPlacePark", (v>=0x500)?l.Entier():0);
       e.Entier(n+"NumPlaceVehlib", (v>=0x530)?l.Entier():0);
       e.Entier(n+"NumSpotVehlib", (v>=0x530)?l.Entier():0);
       for(int i=0; i<NB_DIR; i++)
        {
         e.Brut(Indice(n+"Priorite", i), l.Octet());
         e.Entier(Indice(n+"NumFeu", i), l.Entier());
        }
       for(int i=0; i<NB_DIR; i++)
        e.Entier(Indice(n+"NumFeuP", i), (v>=0x520)?l.Entier():0);
      }
    if (!l.Fin())
     throw std::runtime_error(std::to_string(asOrigine.size()-l.Position())+" octet(s) inattendu(s) en fin de fichier");
    c.Ok=true;
   }
  catch(const std::exception &x)
   {
    c.Erreur=x.what();
   }
  return c;
 }
//---------------------------------------------------------------------------
std::vector<std::string> EcartsParChamp(const conversion_cvw &Reference, const std::string &asObtenu)
 {
  std::vector<std::string> Ecarts;
  auto Valeur=[](const std::string &s, std::size_t Debut, std::size_t Taille)->std::string
   {
    if (Debut+Taille>s.size()) return "<absent>";
    if (Taille==4)
     {
      std::uint32_t u=0;
      for(int i=3; i>=0; i--) u=(u<<8)|(unsigned char)s[Debut+(std::size_t)i];
      return std::to_string((std::int32_t)u);
     }
    if (Taille==1) return std::to_string((int)(signed char)s[Debut]);
    return "<"+std::to_string(Taille)+" octets>";
   };
  for(const champ_cvw &c : Reference.Champs)
   if ((c.Debut+c.Taille>asObtenu.size())||
       std::memcmp(Reference.Octets.data()+c.Debut, asObtenu.data()+c.Debut, c.Taille))
    Ecarts.push_back(c.Nom+" : "+Valeur(Reference.Octets, c.Debut, c.Taille)+" -> "+Valeur(asObtenu, c.Debut, c.Taille));
  if (asObtenu.size()!=Reference.Octets.size())
   Ecarts.push_back("taille : "+std::to_string(Reference.Octets.size())+" -> "+std::to_string(asObtenu.size()));
  return Ecarts;
 }
//---------------------------------------------------------------------------
