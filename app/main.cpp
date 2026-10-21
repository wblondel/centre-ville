//---------------------------------------------------------------------------
// main.cpp - Portage de Centre-Ville : port de src/cvw.cpp
//
// Comme Application->CreateForm, les fiches sont toutes créées au démarrage,
// dans l'ordre de cvw.cpp (application.cpp). frmSimulation (Visible = True
// dans le .dfm) est affichée dès sa création, donc AVANT la création des
// autres fiches : son OnShow (À propos, ouverture du fichier passé en
// paramètre) s'exécute alors que frmDefFeux & co. valent encore NULL, d'où
// les tests « frmDefFeux&& » du code d'origine.
//---------------------------------------------------------------------------
#include <QApplication>
#include <QFileOpenEvent>
#include <QMessageBox>
#include <exception>
#include "application.h"
#include "f_simul.h"
//---------------------------------------------------------------------------
namespace
 {
  // macOS : ouverture d'un .cvw depuis le Finder (QFileOpenEvent). Avant la
  // création de frmSimulation, le fichier est mis en attente.
  class OuvertureFichiers : public QObject
   {
    public:
    QString EnAttente;
    bool eventFilter(QObject *o, QEvent *e) override
     {
      if (e->type()==QEvent::FileOpen)
       {
        QString f=static_cast<QFileOpenEvent *>(e)->file();
        if (frmSimulation&&frmSimulation->isVisible())
         frmSimulation->OuvreFichierDemande(A(f));
        else
         EnAttente=f;
        return true;
       }
      return QObject::eventFilter(o, e);
     }
   };
 }
//---------------------------------------------------------------------------
int main(int argc, char *argv[])
{
 QApplication App(argc, argv);
 InitialiseApplication(App);
 OuvertureFichiers Ouverture;
 App.installEventFilter(&Ouverture);
 int Resultat=0;
 try
	{
		CreeFiches(Ouverture.EnAttente);
		Resultat=App.exec(); // Application->Run();
		DetruitFiches();
	}
 catch (std::exception &exception)
	{ // Application->ShowException(&exception);
		QMessageBox::critical(nullptr, "Centre-Ville", QString::fromUtf8(exception.what()));
	}
 catch (...)
	{
		QMessageBox::critical(nullptr, "Centre-Ville", "Exception");
	}
 return Resultat;
}
//---------------------------------------------------------------------------
