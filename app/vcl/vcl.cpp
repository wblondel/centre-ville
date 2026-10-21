//---------------------------------------------------------------------------
// vcl/vcl.cpp - Portage de Centre-Ville : String, SysUtils, TApplication
//---------------------------------------------------------------------------
#include "vcl.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QStringList>
#include <cmath>
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
