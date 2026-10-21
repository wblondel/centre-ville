//---------------------------------------------------------------------------
// vcl/vcl.cpp - Portage de Centre-Ville : String, SysUtils, TApplication
//---------------------------------------------------------------------------
#include "vcl.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QGuiApplication>
#include <QScreen>
#include <QStringList>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
//---------------------------------------------------------------------------
String String::UpperCase() const { return A(Q(*this).toUpper()); }
String String::LowerCase() const { return A(Q(*this).toLower()); }
String String::Trim() const { return A(Q(*this).trimmed()); }
//---------------------------------------------------------------------------
int ParamCount()
 {
  return (int)QCoreApplication::arguments().size()-1;
 }
//---------------------------------------------------------------------------
String ParamStr(int Index)
 {
  QStringList l=QCoreApplication::arguments();
  if ((Index<0)||(Index>=l.size())) return String();
  if (Index==0) return A(QCoreApplication::applicationFilePath());
  return A(l[Index]);
 }
//---------------------------------------------------------------------------
String FormatDateTime(const AnsiString &asFormat, const TDateTime &dt)
 { // Sous-ensemble utilisé par les fiches : hh, nn, ss, zzz et caractères littéraux
  double Jours=(double)dt;
  long long ms=(long long)std::floor((Jours-std::floor(Jours))*86400000.0+0.5);
  int h=(int)((ms/3600000)%24), n=(int)((ms/60000)%60), s=(int)((ms/1000)%60), z=(int)(ms%1000);
  const std::string &f=asFormat.str();
  std::string r;
  char Tampon[8];
  for(std::size_t i=0; i<f.size();)
   {
    auto Commence=[&](const char *m) { return f.compare(i, std::char_traits<char>::length(m), m)==0; };
    if (Commence("hh")) { std::snprintf(Tampon, sizeof(Tampon), "%02d", h); r+=Tampon; i+=2; }
    else if (Commence("nn")) { std::snprintf(Tampon, sizeof(Tampon), "%02d", n); r+=Tampon; i+=2; }
    else if (Commence("ss")) { std::snprintf(Tampon, sizeof(Tampon), "%02d", s); r+=Tampon; i+=2; }
    else if (Commence("zzz")) { std::snprintf(Tampon, sizeof(Tampon), "%03d", z); r+=Tampon; i+=3; }
    else r+=f[i++];
   }
  return String(r);
 }
//---------------------------------------------------------------------------
String ExtractFilePath(const AnsiString &asNomFichier)
 {
  QString s=Q(asNomFichier);
  int i=(int)qMax(s.lastIndexOf('/'), s.lastIndexOf('\\'));
  return A(s.left(i+1));
 }
//---------------------------------------------------------------------------
String ExtractFileName(const AnsiString &asNomFichier)
 {
  QString s=Q(asNomFichier);
  int i=(int)qMax(s.lastIndexOf('/'), s.lastIndexOf('\\'));
  return A(s.mid(i+1));
 }
//---------------------------------------------------------------------------
int StrToInt(const AnsiString &asTexte)
 {
  bool Ok=false;
  int n=Q(asTexte).trimmed().toInt(&Ok);
  if (!Ok) throw std::invalid_argument("'"+asTexte.str()+"' n'est pas une valeur entière correcte");
  return n;
 }
//---------------------------------------------------------------------------
String FormatFloat(const AnsiString &asFormat, long double Valeur)
 { // SysUtils::FormatFloat, une seule section : « 0 » chiffre obligatoire, « # »
   // chiffre facultatif, « . » séparateur décimal (omis sans décimale affichée),
   // texte entre guillemets ou apostrophes et autres caractères (« % »...)
   // recopiés à leur place, signe en tête. Arrondi comme FloatToDecimal : à
   // partir de 17 chiffres significatifs, la moitié vers le haut. NAN, INF et
   // -INF comme la VCL (FloatToTextFmt).
  if (std::isnan(Valeur)) return String("NAN");
  if (std::isinf(Valeur)) return String((Valeur<0)?"-INF":"INF");
  const std::string &f=asFormat.str();
  // Analyse du format
  int NbAvant=0, PremierZero=-1, Decimales=0, DecimalesMin=0;
  bool Point=false;
  for(std::size_t i=0; i<f.size(); i++)
   {
    char c=f[i];
    if ((c=='"')||(c=='\'')) { std::size_t j=f.find(c, i+1); i=(j==std::string::npos)?f.size():j; }
    else if ((c=='.')&&!Point) Point=true;
    else if ((c=='0')||(c=='#'))
     {
      if (!Point) { if ((c=='0')&&(PremierZero<0)) PremierZero=NbAvant; NbAvant++; }
      else { Decimales++; if (c=='0') DecimalesMin=Decimales; }
     }
   }
  int EntiersMin=(PremierZero<0)?0:NbAvant-PremierZero;
  // Chiffres : Valeur = 0,Chiffres x 10^(Exposant+1), arrondis à Decimales
  char Tampon[64];
  std::snprintf(Tampon, sizeof(Tampon), "%.16Le", std::fabs(Valeur));
  std::string Chiffres=std::string(1, Tampon[0])+std::string(Tampon+2, 16);
  int Exposant=std::atoi(std::strchr(Tampon, 'e')+1);
  int Garde=Exposant+1+Decimales;
  if (Garde<0) Chiffres.clear();
  else if (Garde<(int)Chiffres.size())
   {
    bool Haut=Chiffres[(std::size_t)Garde]>='5';
    Chiffres.resize((std::size_t)Garde);
    if (Haut)
     {
      int i=Garde-1;
      while((i>=0)&&(Chiffres[(std::size_t)i]=='9')) Chiffres[(std::size_t)i--]='0';
      if (i<0) { Chiffres.insert(0, "1"); Exposant++; }
      else Chiffres[(std::size_t)i]++;
     }
   }
  bool Zero=Chiffres.find_first_not_of('0')==std::string::npos;
  std::string Entier, Fraction;
  for(int i=0; i<Exposant+1; i++) Entier+=(i<(int)Chiffres.size())?Chiffres[(std::size_t)i]:'0';
  for(int i=0; i<Decimales; i++)
   {
    int j=Exposant+1+i;
    Fraction+=((j>=0)&&(j<(int)Chiffres.size()))?Chiffres[(std::size_t)j]:'0';
   }
  Entier.erase(0, qMin(Entier.find_first_not_of('0'), Entier.size()));
  if ((int)Entier.size()<EntiersMin) Entier.insert(0, (std::size_t)(EntiersMin-(int)Entier.size()), '0');
  while(((int)Fraction.size()>DecimalesMin)&&(Fraction.back()=='0')) Fraction.pop_back();
  // Résultat
  std::string r=((Valeur<0)&&!Zero)?"-":"";
  bool EntierEcrit=false, PointVu=false;
  std::size_t Decimale=0;
  for(std::size_t i=0; i<f.size(); i++)
   {
    char c=f[i];
    if ((c=='"')||(c=='\''))
     {
      std::size_t j=f.find(c, i+1);
      if (j==std::string::npos) j=f.size();
      r+=f.substr(i+1, j-i-1);
      i=j;
     }
    else if ((c=='.')&&!PointVu)
     {
      PointVu=true;
      if (!EntierEcrit) { r+=Entier; EntierEcrit=true; }
      if (!Fraction.empty()) r+=FormatSettings.DecimalSeparator;
     }
    else if ((c=='0')||(c=='#'))
     {
      if (!PointVu) { if (!EntierEcrit) { r+=Entier; EntierEcrit=true; } }
      else if (Decimale<Fraction.size()) r+=Fraction[Decimale++];
     }
    else r+=c;
   }
  if (!EntierEcrit) r+=Entier;
  return String(r);
 }
//---------------------------------------------------------------------------
// TScreen
//---------------------------------------------------------------------------
namespace
 {
  TScreen EcranVcl;
 }
TScreen *Screen=&EcranVcl;
//---------------------------------------------------------------------------
TScreen::TScreen()
 {
  PixelsPerInch.Definit([]()
                         {
                          QScreen *e=QGuiApplication::primaryScreen();
                          return e?qRound(96*e->devicePixelRatio()):96;
                         },
                        [](const int &) {});
 }
//---------------------------------------------------------------------------
// TApplication
//---------------------------------------------------------------------------
TApplication *Application=nullptr;
//---------------------------------------------------------------------------
TApplication::TApplication()
 { // Avec MainFormOnTaskBar, Title n'apparaît nulle part : il est seulement mémorisé
  Title.Definit([this]() { return asTitre; }, [this](const AnsiString &as) { asTitre=as; });
 }
//---------------------------------------------------------------------------
void TApplication::ProcessMessages()
 {
  QCoreApplication::processEvents();
 }
//---------------------------------------------------------------------------
