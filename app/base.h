//---------------------------------------------------------------------------
// base.h - Portage de Centre-Ville : port de src/base.h
//---------------------------------------------------------------------------
#ifndef baseH
#define baseH
//---------------------------------------------------------------------------
#include <type_traits>
#include "vcl/vcl.h" // portage : remplace <vcl.h>
#include "vue.h" // portage : type_message et type_reponse sont déclarés avec IVueSimulation
//---------------------------------------------------------------------------
class QWidget;
//---------------------------------------------------------------------------
// Propriétés de TCSpinEdit
#define MINVALUE MinValue
#define MAXVALUE MaxValue
// Propriétés de TOpen/CloseDialog
#define FILENAME(x) ((x)->FileName)
// Types
#define TBITMAP TBitmap // portage : Graphics::TBitmap -> vcl/graphiques.h
#define CONTROLCOUNT ControlCount
//---------------------------------------------------------------------------
// portage : macros min/max de src/simul.h, retirées du moteur (conflit avec la
// STL) et utilisées par 5 fiches. Fonctions plutôt que macros, pour ne pas
// casser les en-têtes de Qt et de la STL ; même résultat que
// (((x)<(y))?(x):(y)) avec les conversions arithmétiques usuelles.
//---------------------------------------------------------------------------
template <class X, class Y> inline std::common_type_t<X, Y> min(X x, Y y) { return (x<y)?x:y; }
template <class X, class Y> inline std::common_type_t<X, Y> max(X x, Y y) { return (x>y)?x:y; }
//---------------------------------------------------------------------------
// Types énumérés type_message et type_reponse : voir engine/vue.h // portage
//---------------------------------------------------------------------------
// v5.3: DEBOGAGE dans cvw.log
extern bool Debug;
extern void DebugOuvre();
extern void DebugEcrit(const AnsiString asMessage);
extern void DebugFerme();
//---------------------------------------------------------------------------
extern const AnsiString asDebutNomSpinEdit; // v5.4
//---------------------------------------------------------------------------
extern String stRepLocalAppData(); // v5.4
extern type_reponse AfficheMessage(const AnsiString asMessage, const AnsiString asTitre, type_message Type, int OrdreBoutonDefaut, int OrdreBoutonEchap);
extern void AfficheAideContextuelle(QWidget *Form); // portage : TForm* -> QWidget* (HelpContext lu sur la fiche)
extern void AfficheIndexAide(QWidget *Form);
extern void AfficheTutoriel(); // v5.4 (appelée par TfrmSimulation::ActionTutorielExecute
//---------------------------------------------------------------------------
// portage : MessageDlg de l'unité Dialogs de la VCL
//---------------------------------------------------------------------------
enum TMsgDlgType { mtWarning, mtError, mtInformation, mtConfirmation, mtCustom };
enum TMsgDlgBtn { mbYes, mbNo, mbOK, mbCancel, mbAbort, mbRetry, mbIgnore, mbAll, mbNoToAll, mbYesToAll, mbHelp, mbClose };
typedef TEnsemble<TMsgDlgBtn> TMsgDlgButtons;
extern int MessageDlg(const AnsiString asMessage, TMsgDlgType Type, TMsgDlgButtons Boutons, int HelpCtx);
//---------------------------------------------------------------------------
#endif
