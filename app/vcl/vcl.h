//---------------------------------------------------------------------------
// vcl/vcl.h - Portage de Centre-Ville : sous-ensemble de la VCL au-dessus de Qt
//
// Remplace <vcl.h> et les en-têtes *.hpp de C++Builder pour le code des
// fiches. Seul ce qu'utilisent les fiches de Centre-Ville est reproduit, avec
// la sémantique de la VCL (événements implicites compris, PLAN_PORTAGE.md
// §4.4), pour que le code porté reste lisible ligne à ligne face à l'original.
//
// Correspondances de base :
//   TObject, TComponent -> QObject ; String -> AnsiString (UTF-8) étendue ;
//   TColor -> entier 0x00BBGGRR (vcl/couleurs.h) ; __property -> Propriete<T>.
//---------------------------------------------------------------------------
#ifndef vcl_vclH
#define vcl_vclH
//---------------------------------------------------------------------------
#include <QObject>
#include <QString>
#include <functional>
#include <vector>
#include "compat/chaine.h"
#include "compat/fichier.h"
#include "compat/temps.h"
#include "proprietes.h"
//---------------------------------------------------------------------------
typedef QObject TObject;
typedef QObject TComponent;
typedef unsigned short WORD;
//---------------------------------------------------------------------------
// String (UnicodeString de C++Builder X) : AnsiString en UTF-8, plus les
// quelques méthodes utilisées par les fiches
//---------------------------------------------------------------------------
class String : public AnsiString
 {
  public:
  String() {}
  String(const char *s) : AnsiString(s) {}
  String(const AnsiString &as) : AnsiString(as) {}
  String(const std::string &s) : AnsiString(s) {}
  String UpperCase() const;
  String LowerCase() const;
  String Trim() const;
 };
typedef String UnicodeString;
//---------------------------------------------------------------------------
// Conversions avec Qt
//---------------------------------------------------------------------------
inline QString Q(const AnsiString &as) { return QString::fromUtf8(as.c_str()); }
inline AnsiString A(const QString &qs) { return AnsiString(qs.toUtf8().toStdString()); }
//---------------------------------------------------------------------------
// Événements (TNotifyEvent et compagnie) : fonctions appelées avec Sender
//---------------------------------------------------------------------------
typedef std::function<void(TObject *Sender)> TNotifyEvent;
//---------------------------------------------------------------------------
// Connecte(Evenement, objet, &Classe::Gestionnaire) : affecte un gestionnaire
// (méthode de fiche) à un événement, comme l'inspecteur d'objets de l'EDI
//---------------------------------------------------------------------------
template <class R, class... P, class C, class O>
void Connecte(std::function<R(P...)> &Evenement, O *Objet, R (C::*Gestionnaire)(P...))
 {
  Evenement=[Objet, Gestionnaire](P... a) -> R { return (Objet->*Gestionnaire)(std::forward<P>(a)...); };
 }
//---------------------------------------------------------------------------
// try/__finally de C++Builder -> garde RAII : le code est exécuté en sortie de bloc
//---------------------------------------------------------------------------
class Finalement
 {
  std::function<void()> Code;
  public:
  explicit Finalement(std::function<void()> c) : Code(std::move(c)) {}
  Finalement(const Finalement &)=delete;
  ~Finalement() { Code(); }
 };
//---------------------------------------------------------------------------
// TShiftState
//---------------------------------------------------------------------------
enum TShiftStateElement { ssShift, ssAlt, ssCtrl, ssLeft, ssRight, ssMiddle, ssDouble };
class TShiftState
 {
  unsigned Bits=0;
  public:
  TShiftState() {}
  TShiftState &operator<<(TShiftStateElement e) { Bits|=1u<<e; return *this; }
  TShiftState &operator>>(TShiftStateElement e) { Bits&=~(1u<<e); return *this; }
  bool Contains(TShiftStateElement e) const { return (Bits>>e)&1u; }
 };
//---------------------------------------------------------------------------
// Ensembles de la VCL (TGridOptions, TMsgDlgButtons...) : Options << x, >> x
//---------------------------------------------------------------------------
template <class E> class TEnsemble
 {
  unsigned long long Bits=0;
  public:
  TEnsemble() {}
  TEnsemble &operator<<(E e) { Bits|=1ull<<(int)e; return *this; }
  TEnsemble &operator>>(E e) { Bits&=~(1ull<<(int)e); return *this; }
  bool Contains(E e) const { return (Bits>>(int)e)&1ull; }
  bool operator==(const TEnsemble &t) const { return Bits==t.Bits; }
 };
//---------------------------------------------------------------------------
// Divers
//---------------------------------------------------------------------------
enum TWindowState { wsNormal, wsMinimized, wsMaximized };
enum TCursor { crDefault=0, crHourGlass=-11, crHandPoint=-21 };
enum TModalResult { mrNone=0, mrOk=1, mrCancel=2, mrAbort=3, mrRetry=4, mrIgnore=5, mrYes=6, mrNo=7,
                    mrAll=8, mrNoToAll=9, mrYesToAll=10, mrClose=11 };
//---------------------------------------------------------------------------
// SysUtils / System
//---------------------------------------------------------------------------
int ParamCount();
String ParamStr(int Index);
String FormatDateTime(const AnsiString &asFormat, const TDateTime &dt);
String ExtractFilePath(const AnsiString &asNomFichier);
String ExtractFileName(const AnsiString &asNomFichier);
int StrToInt(const AnsiString &asTexte); // texte invalide : exception (EConvertError)
// Sous-ensemble utilisé par les fiches : « 0 », « 0.0 », « 0.00 »... (séparateur
// décimal de FormatSettings, arrondi au plus proche)
String FormatFloat(const AnsiString &asFormat, long double Valeur);
//---------------------------------------------------------------------------
// TScreen (Screen->PixelsPerInch) : résolution de l'écran principal comme la
// donne Windows, 96 × facteur d'échelle (192 sur un écran Retina)
//---------------------------------------------------------------------------
class TScreen
 {
  public:
  Propriete<int> PixelsPerInch;
  TScreen();
 };
extern TScreen *Screen;
//---------------------------------------------------------------------------
// TApplication (Application->Title...)
//---------------------------------------------------------------------------
class TApplication : public QObject
 {
  AnsiString asTitre;
  public:
  TApplication();
  Propriete<AnsiString> Title;
  void ProcessMessages();
 };
extern TApplication *Application;
//---------------------------------------------------------------------------
#endif
