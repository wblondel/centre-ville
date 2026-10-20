//---------------------------------------------------------------------------
// test_compat.cpp - Tests de la couche engine/compat
//---------------------------------------------------------------------------
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include <string>
#include "sortie.h"
#include "compat/chaine.h"
#include "compat/fichier.h"
#include "compat/temps.h"
#include "compat/alea.h"
//---------------------------------------------------------------------------
TEST_CASE("AnsiString : sous-ensemble utilisé par le moteur", "[compat][chaine]")
 {
  AnsiString as;
  CHECK(as.IsEmpty());
  as="Arrêt";
  as+=" n°";
  as=as+IntToStr(3);
  CHECK(as==AnsiString("Arrêt n°3"));
  CHECK("("+IntToStr(-1)+", "+IntToStr(2)+")"==AnsiString("(-1, 2)"));
  CHECK(AnsiString((char)('A'+2))==AnsiString("C"));
  AnsiString asListe="";
  for(int i=1; i<=3; i++)
   if (asListe.IsEmpty()) asListe=IntToStr(i); else asListe=asListe+","+IntToStr(i);
  CHECK(asListe==AnsiString("1,2,3"));
  // Indices à partir de 1
  AnsiString asTitre="Centre-Ville*";
  CHECK(asTitre[asTitre.Length()]=='*');
  CHECK(asTitre.SubString(1, asTitre.Length()-1)==AnsiString("Centre-Ville"));
  CHECK(asTitre.Pos("Ville")==8);
  CHECK(asTitre.Pos("Tram")==0);
 }
//---------------------------------------------------------------------------
TEST_CASE("Format/ARRAYOFCONST : spécificateurs de SysUtils", "[compat][chaine]")
 {
  CHECK(Format("Piéton n°%d (%d,%d)", ARRAYOFCONST((7, -1, 12)))==AnsiString("Piéton n°7 (-1,12)"));
  CHECK(Format("%d/%d", ARRAYOFCONST((3, 10)))==AnsiString("3/10"));
  CHECK(Format("feu %s mais (%d)", ARRAYOFCONST((AnsiString("Nord"), (int)3)))==AnsiString("feu Nord mais (3)"));
  CHECK(Format("%s", ARRAYOFCONST(("abc")))==AnsiString("abc"));
  CHECK(Format("100%%", ARRAYOFCONST((0)))==AnsiString("100%"));
  // Arguments surnuméraires ignorés (cf. depot_bus::Verifie)
  CHECK(Format("Dépot bus : ", ARRAYOFCONST((1, 2)))==AnsiString("Dépot bus : "));
  // Largeur, justification, précision
  CHECK(Format("[%5d]", ARRAYOFCONST((42)))==AnsiString("[   42]"));
  CHECK(Format("[%-5d]", ARRAYOFCONST((42)))==AnsiString("[42   ]"));
  CHECK(Format("[%.4d]", ARRAYOFCONST((-42)))==AnsiString("[-0042]"));
  CHECK(Format("[%*d]", ARRAYOFCONST((4, 7)))==AnsiString("[   7]"));
  CHECK(Format("%x", ARRAYOFCONST((255)))==AnsiString("FF"));
  CHECK(Format("%.2f", ARRAYOFCONST((3.14159)))==AnsiString("3.14"));
  CHECK(Format("%f", ARRAYOFCONST((2.5)))==AnsiString("2.50"));
  CHECK(Format("%.3e", ARRAYOFCONST((1234.5)))==AnsiString("1.23E+003"));
  CHECK(Format("%.2s", ARRAYOFCONST(("abc")))==AnsiString("ab"));
  // Index explicite
  CHECK(Format("%1:s-%0:s", ARRAYOFCONST(("a", "b")))==AnsiString("b-a"));
  // Erreurs (EConvertError dans la VCL)
  CHECK_THROWS_AS(Format("%d", ARRAYOFCONST(("texte"))), std::invalid_argument);
  CHECK_THROWS_AS(Format("%d %d", ARRAYOFCONST((1))), std::invalid_argument);
 }
//---------------------------------------------------------------------------
TEST_CASE("rand()/srand() : LCG de la RTL Borland", "[compat][alea]")
 {
  alea::LibereGraine();
  // Suite connue de rand() Borland/Turbo C après srand(1)
  srand(1);
  const int Attendu[10]={346, 130, 10982, 1090, 11656, 7117, 17595, 6415, 22948, 31126};
  for(int i=0; i<10; i++)
   CHECK(rand()==Attendu[i]);
  // Bornes : RAND_MAX Borland = 32767
  srand(0xDEADBEEF);
  bool Max15Bits=true;
  for(int i=0; i<100000; i++)
   {
    int r=rand();
    Max15Bits=Max15Bits&&(r>=0)&&(r<=alea::RAND_MAX_BORLAND);
   }
  CHECK(Max15Bits);
  // L'état est bien graine = graine*22695477+1 sur 32 bits
  srand(123);
  rand();
  CHECK(alea::Graine()==123u*22695477u+1u);
 }
//---------------------------------------------------------------------------
TEST_CASE("rand()/srand() : graine injectable", "[compat][alea]")
 {
  alea::FixeGraine(42);
  int a=rand();
  srand(987654); // ignorée tant que la graine est fixée (AppelleAuChaos)
  int b=rand();
  CHECK(a==b);
  alea::LibereGraine();
  srand(987654);
  CHECK(alea::Graine()==987654u);
 }
//---------------------------------------------------------------------------
TEST_CASE("FileCreate/FileWrite/FileRead/FileSeek/FileClose", "[compat][fichier]")
 {
  const AnsiString asNom=FichierSortie("compat_fichier_é.bin");
  int h=FileCreate(asNom);
  REQUIRE(h>0);
  int Valeurs[3]={1, -2, 0x530};
  bool Booleen=true;
  signed char Octet=-3;
  CHECK(FileWrite(h, Valeurs, sizeof(Valeurs))==(int)sizeof(Valeurs));
  CHECK(FileWrite(h, &Booleen, sizeof(Booleen))==1);
  CHECK(FileWrite(h, &Octet, sizeof(Octet))==1);
  FileClose(h);
  h=FileOpen(asNom, fmOpenRead);
  REQUIRE(h>0);
  CHECK(FileSeek(h, 0, 0)==0);
  int Lus[3]={0, 0, 0};
  bool BooleenLu=false;
  signed char OctetLu=0;
  CHECK(FileRead(h, Lus, sizeof(Lus))==(int)sizeof(Lus));
  CHECK(FileRead(h, &BooleenLu, sizeof(BooleenLu))==1);
  CHECK(FileRead(h, &OctetLu, sizeof(OctetLu))==1);
  CHECK(FileRead(h, &OctetLu, sizeof(OctetLu))==0); // fin de fichier : 0 (faux)
  FileClose(h);
  CHECK(Lus[0]==1);
  CHECK(Lus[1]==-2);
  CHECK(Lus[2]==0x530);
  CHECK(BooleenLu);
  CHECK(OctetLu==-3);
  // Fichier absent : -1, comme INVALID_HANDLE_VALUE
  CHECK(FileOpen(AnsiString(CV_DOSSIER_SORTIE)+"/absent/absent.cvw", fmOpenRead)==-1);
  CHECK(FileRead(9999, Lus, 4)==-1);
 }
//---------------------------------------------------------------------------
TEST_CASE("TDateTime/Time() : chronométrage", "[compat][temps]")
 {
  TDateTime dtTop, dtTemps;
  dtTop=dtTemps=0;
  CHECK((double)dtTemps==0.0);
  dtTop=Time();
  CHECK((double)dtTop>=0.0);
  CHECK((double)dtTop<1.0);
  dtTemps+=(Time()-dtTop);
  CHECK((double)dtTemps>-1.0/86400); // à minuit près
  CHECK((double)Now()>45000.0); // après 2023
 }
//---------------------------------------------------------------------------
TEST_CASE("FileRead/FileWrite : tampon transparent (gros volumes, lecture/écriture mêlées)", "[compat][fichier]")
 {
  const AnsiString asNom=FichierSortie("compat_tampon.bin");
  int h=FileCreate(asNom);
  REQUIRE(h>0);
  const int N=300000; // > 4 tampons de 64 Ko, écrit par petits morceaux
  bool EcrituresOk=true;
  for(int i=0; i<N; i++)
   {
    unsigned char o=(unsigned char)(i*7+3);
    EcrituresOk=EcrituresOk&&(FileWrite(h, &o, 1)==1);
   }
  CHECK(EcrituresOk);
  CHECK(FileSeek(h, 0, 2)==N); // fin de fichier : tout a été écrit
  CHECK(FileSeek(h, 0, 0)==0);
  std::string Gros((std::size_t)N, '\0');
  CHECK(FileRead(h, &Gros[0], 100)==100);
  CHECK(FileRead(h, &Gros[100], (unsigned)N)==N-100); // lecture directe d'un gros bloc, tronquée en fin
  bool Identiques=true;
  for(int i=0; i<N; i++) Identiques=Identiques&&((unsigned char)Gros[(std::size_t)i]==(unsigned char)(i*7+3));
  CHECK(Identiques);
  FileClose(h);
  // Lecture puis écriture au milieu (fmOpenReadWrite) : la position est respectée
  h=FileOpen(asNom, fmOpenReadWrite);
  REQUIRE(h>0);
  int Lu=0;
  CHECK(FileRead(h, &Lu, 4)==4);
  int Ecrit=-1;
  CHECK(FileWrite(h, &Ecrit, 4)==4); // octets 4 à 7
  unsigned char Suivant=0;
  CHECK(FileRead(h, &Suivant, 1)==1); // octet 8
  CHECK(Suivant==(unsigned char)(8*7+3));
  FileClose(h);
  h=FileOpen(asNom, fmOpenRead);
  REQUIRE(h>0);
  int Valeurs[2]={0, 0};
  CHECK(FileRead(h, Valeurs, 8)==8);
  CHECK(Valeurs[0]==Lu);
  CHECK(Valeurs[1]==-1);
  CHECK(FileSeek(h, 0, 2)==N);
  FileClose(h);
 }
//---------------------------------------------------------------------------
