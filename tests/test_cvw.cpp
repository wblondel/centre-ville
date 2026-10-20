//---------------------------------------------------------------------------
// test_cvw.cpp - Format .cvw : round-trip Charge -> Sauve et Verifie()
//---------------------------------------------------------------------------
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include <string>
#include "cartes.h"
#include "format_reference.h"
#include "harnais.h"
#include "sortie.h"
#include "vue.h"
#include "compat/fichier.h"
#include "compat/alea.h"
//---------------------------------------------------------------------------
namespace
 {
  // Premier octet différent entre deux contenus (-1 si identiques)
  long long PremierEcart(const std::string &a, const std::string &b)
   {
    std::size_t n=a.size()<b.size()?a.size():b.size();
    for(std::size_t i=0; i<n; i++)
     if (a[i]!=b[i]) return (long long)i;
    return (a.size()==b.size())?-1:(long long)n;
   }
  //-------------------------------------------------------------------------
  const champ_cvw &Champ(const conversion_cvw &c, const std::string &Nom)
   {
    for(const champ_cvw &ch : c.Champs)
     if (ch.Nom==Nom) return ch;
    throw std::runtime_error("champ introuvable : "+Nom);
   }
  //-------------------------------------------------------------------------
  int LitEntier(const std::string &o, const champ_cvw &ch)
   {
    unsigned u=0;
    for(int i=3; i>=0; i--) u=(u<<8)|(unsigned char)o[ch.Debut+(std::size_t)i];
    return (int)u;
   }
  //-------------------------------------------------------------------------
  void EcritEntier(std::string &o, const champ_cvw &ch, int v)
   {
    for(int i=0; i<4; i++) o[ch.Debut+(std::size_t)i]=(char)(((unsigned)v>>(8*i))&0xFF);
   }
  //-------------------------------------------------------------------------
  void EcritFichier(const std::string &asChemin, const std::string &asContenu)
   {
    int h=FileCreate(asChemin.c_str());
    REQUIRE(h>0);
    REQUIRE(FileWrite(h, asContenu.data(), (unsigned)asContenu.size())==(int)asContenu.size());
    FileClose(h);
   }
  //-------------------------------------------------------------------------
  // Lignes d'écarts attendus : lignes du fichier de référence hors commentaires (#)
  std::string EcartsAttendus(const std::string &asFichier)
   {
    std::string asLignes, l;
    for(std::size_t i=0; i<=asFichier.size(); i++)
     if ((i==asFichier.size())||(asFichier[i]=='\n'))
      {
       if (!l.empty()&&(l.back()=='\r')) l.pop_back();
       if (!l.empty()&&(l[0]!='#')) asLignes+=l+"\n";
       l.clear();
      }
     else
      l+=asFichier[i];
    return asLignes;
   }
 }
//---------------------------------------------------------------------------
TEST_CASE("Round-trip Charge -> Sauve octet par octet sur les 6 cartes", "[cvw][roundtrip]")
 {
  VueNulle Vue;
  vue=&Vue;
  for(const carte &c : Cartes)
   {
    DYNAMIC_SECTION(c.Nom)
     {
      const std::string asOrigine=harnais::LitFichier(CheminCarte(c));
      REQUIRE(!asOrigine.empty());
      // Spécification indépendante du moteur : conversion naïve en 0x530
      const conversion_cvw Reference=ConvertitEn530(asOrigine);
      INFO("Conversion de référence : " << Reference.Erreur);
      REQUIRE(Reference.Ok);
      if (Reference.Version==VERSION)
       { // Au format courant, la spécification doit reproduire le fichier à l'identique
        const bool SpecificationExacte=(Reference.Octets==asOrigine);
        CHECK(SpecificationExacte);
       }
      // Charge -> Sauve par le moteur
      REQUIRE(harnais::Ouvre(CheminCarte(c)));
      const std::string asCopie=FichierSortie(std::string("roundtrip_")+c.Nom+".cvw");
      REQUIRE(harnais::SauveSansChaos(asCopie));
      harnais::Ferme();
      const std::string asSauve=harnais::LitFichier(asCopie);
      // Écarts champ par champ avec la référence : ils doivent être exactement
      // ceux, expliqués, de tests/golden/roundtrip/<carte>.txt
      std::string asEcarts;
      for(const std::string &l : EcartsParChamp(Reference, asSauve)) asEcarts+=l+"\n";
      const std::string asRelatif=std::string("roundtrip/")+c.Nom+".txt";
      const std::string asFichierAttendu=harnais::LitFichier(std::string(CV_DOSSIER_GOLDEN)+"/"+asRelatif);
      REQUIRE_FALSE(asFichierAttendu.empty());
      const std::string asAttendu=EcartsAttendus(asFichierAttendu);
      INFO("Carte " << c.Fichier << " (version 0x" << std::hex << Reference.Version << std::dec << ") : "
           << asOrigine.size() << " octets lus, " << asSauve.size() << " octets écrits, premier écart avec l'original à l'octet "
           << PremierEcart(asOrigine, asSauve) << "\nÉcarts obtenus avec la référence :\n" << asEcarts
           << "Écarts attendus (tests/golden/" << asRelatif << ") :\n" << asAttendu);
      CHECK(asEcarts==asAttendu);
      if (Reference.Version==VERSION)
       {
        const bool Identique=(asSauve==asOrigine);
        CHECK(Identique);
       }
      // Point fixe : relire et réenregistrer la copie redonne la même copie
      REQUIRE(harnais::Ouvre(asCopie));
      const std::string asCopie2=FichierSortie(std::string("roundtrip2_")+c.Nom+".cvw");
      REQUIRE(harnais::SauveSansChaos(asCopie2));
      harnais::Ferme();
      const bool PointFixe=(harnais::LitFichier(asCopie2)==asSauve);
      CHECK(PointFixe);
     }
   }
  CHECK(Vue.Messages.empty());
  vue=nullptr;
 }
//---------------------------------------------------------------------------
TEST_CASE("Verifie() sur les 6 cartes : messages figés", "[cvw][verifie]")
 { // Aucun message attendu, sauf écarts du réseau d'origine expliqués dans
   // tests/golden/verifie/<carte>.txt
  VueNulle Vue;
  vue=&Vue;
  alea::FixeGraine(1);
  for(const carte &c : Cartes)
   {
    DYNAMIC_SECTION(c.Nom)
     {
      AnsiString asMsgErr;
      std::string asObtenu;
      REQUIRE(harnais::Ouvre(CheminCarte(c)));
      // Comme VerifierApresOuverture
      cv->Verifie(asMsgErr);
      asObtenu+="[apres ouverture]\n"+asMsgErr.str();
      // Comme VerifierAvantSimulation (après la préparation de la simulation)
      AnsiString asMsgDemarrage;
      REQUIRE(harnais::Demarre(asMsgDemarrage));
      cv->Verifie(asMsgErr);
      asObtenu+="[avant simulation]\n"+asMsgErr.str();
      harnais::Ferme();
      const std::string asRelatif=std::string("verifie/")+c.Nom+".txt";
      const std::string asFichier=harnais::LitFichier(std::string(CV_DOSSIER_GOLDEN)+"/"+asRelatif);
      REQUIRE_FALSE(asFichier.empty());
      const std::string asAttendu=EcartsAttendus(asFichier);
      INFO("Obtenu :\n" << asObtenu << "Attendu (tests/golden/" << asRelatif << ") :\n" << asAttendu);
      CHECK(asObtenu==asAttendu);
     }
   }
  CHECK(Vue.Messages.empty());
  alea::LibereGraine();
  vue=nullptr;
 }
//---------------------------------------------------------------------------
TEST_CASE("VerifieReseau() accepte les 6 cartes", "[cvw][verifie]")
 {
  VueNulle Vue;
  vue=&Vue;
  for(const carte &c : Cartes)
   {
    DYNAMIC_SECTION(c.Nom)
     {
      AnsiString asMsgErr;
      REQUIRE(harnais::Ouvre(CheminCarte(c)));
      CHECK(harnais::VerifieReseau(asMsgErr));
      INFO(asMsgErr.str());
      CHECK(asMsgErr.IsEmpty());
      CHECK(cv->VerifOk);
      CHECK(!Vue.Modifiee);
      harnais::Ferme();
     }
   }
  vue=nullptr;
 }
//---------------------------------------------------------------------------
TEST_CASE("Chargement : élimination des feux fantômes (v5.1.1)", "[cvw][roundtrip]")
 { // Une case dont NumFeu[] désigne un feu situé ailleurs : Charge remet NumFeu à 0
  VueNulle Vue;
  vue=&Vue;
  std::string asCarte=harnais::LitFichier(CheminCarte(Cartes[4])); // Géant.cvw, format 0x530, 109 feux
  conversion_cvw Ref=ConvertitEn530(asCarte);
  REQUIRE(Ref.Ok);
  REQUIRE(Ref.Version==VERSION);
  const int NbX=LitEntier(asCarte, Champ(Ref, "NbX")), NbY=LitEntier(asCarte, Champ(Ref, "NbY"));
  // Premier feu défini et première case sans feu au nord, ailleurs que ce feu
  int NumFeu=0, xf=-1, yf=-1;
  for(int f=1; (!NumFeu)&&(f<=LitEntier(asCarte, Champ(Ref, "NbFeux"))); f++)
   if ((xf=LitEntier(asCarte, Champ(Ref, "feu"+std::to_string(f)+".x")))>=0)
    {
     NumFeu=f;
     yf=LitEntier(asCarte, Champ(Ref, "feu"+std::to_string(f)+".y"));
    }
  REQUIRE(NumFeu>0);
  std::string asNomChamp;
  for(int x=0; asNomChamp.empty()&&(x<NbX); x++)
   for(int y=0; asNomChamp.empty()&&(y<NbY); y++)
    {
     std::string n="case("+std::to_string(x)+","+std::to_string(y)+").NumFeu[0]";
     if (((x!=xf)||(y!=yf))&&!LitEntier(asCarte, Champ(Ref, n))) asNomChamp=n;
    }
  REQUIRE_FALSE(asNomChamp.empty());
  EcritEntier(asCarte, Champ(Ref, asNomChamp), NumFeu); // feu fantôme
  const std::string asFantome=FichierSortie("feu_fantome.cvw");
  EcritFichier(asFantome, asCarte);
  REQUIRE(harnais::Ouvre(asFantome));
  const std::string asCopie=FichierSortie("feu_fantome_sauve.cvw");
  REQUIRE(harnais::SauveSansChaos(asCopie));
  harnais::Ferme();
  std::vector<std::string> Ecarts=EcartsParChamp(ConvertitEn530(asCarte), harnais::LitFichier(asCopie));
  REQUIRE(Ecarts.size()==1);
  CHECK(Ecarts[0]==asNomChamp+" : "+std::to_string(NumFeu)+" -> 0");
  vue=nullptr;
 }
//---------------------------------------------------------------------------
TEST_CASE("Chargement : parkings indéfinis retirés et renumérotés (v5.1)", "[cvw][roundtrip]")
 { // Parking n°2 sans coordonnées : il disparaît, les suivants sont renumérotés,
   // seules leurs cases sont mises à jour (la case de l'ancien n°2 garde 2)
  VueNulle Vue;
  vue=&Vue;
  std::string asCarte=harnais::LitFichier(CheminCarte(Cartes[0])); // 8x8.cvw : 4 parkings
  conversion_cvw Ref=ConvertitEn530(asCarte);
  REQUIRE(Ref.Ok);
  REQUIRE(LitEntier(asCarte, Champ(Ref, "NbParkings"))==4);
  int x[5], y[5];
  for(int p=1; p<=4; p++)
   {
    x[p]=LitEntier(asCarte, Champ(Ref, "parking"+std::to_string(p)+".x"));
    y[p]=LitEntier(asCarte, Champ(Ref, "parking"+std::to_string(p)+".y"));
    REQUIRE(x[p]>=0);
   }
  EcritEntier(asCarte, Champ(Ref, "parking2.x"), -1);
  EcritEntier(asCarte, Champ(Ref, "parking2.y"), -1);
  const std::string asIndefini=FichierSortie("parking_indefini.cvw");
  EcritFichier(asIndefini, asCarte);
  REQUIRE(harnais::Ouvre(asIndefini));
  REQUIRE(cv->NbParkings==3);
  const int Ancien[4]={0, 1, 3, 4}; // nouveau n° -> ancien n°
  for(int p=1; p<=3; p++)
   {
    CHECK(cv->Parking[p-1].x==x[Ancien[p]]);
    CHECK(cv->Parking[p-1].y==y[Ancien[p]]);
    CHECK(cv->v[x[Ancien[p]]][y[Ancien[p]]].NumParking==p);
   }
  CHECK(cv->v[x[2]][y[2]].NumParking==2); // case de l'ancien parking n°2 non mise à jour
  harnais::Ferme();
  vue=nullptr;
 }
//---------------------------------------------------------------------------
TEST_CASE("Chargement : refus d'un fichier de version ultérieure", "[cvw]")
 {
  VueNulle Vue;
  vue=&Vue;
  std::string asCarte=harnais::LitFichier(CheminCarte(Cartes[0]));
  conversion_cvw Ref=ConvertitEn530(asCarte);
  REQUIRE(Ref.Ok);
  EcritEntier(asCarte, Champ(Ref, "Version"), VERSION+0x10);
  const std::string asFutur=FichierSortie("version_future.cvw");
  EcritFichier(asFutur, asCarte);
  CHECK_FALSE(harnais::Ouvre(asFutur));
  CHECK(VersionFichier==VERSION+0x10);
  harnais::Ferme();
  vue=nullptr;
 }
//---------------------------------------------------------------------------
