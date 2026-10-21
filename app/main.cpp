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
#include <QMessageBox>
#include <exception>
#include "application.h"
#include "f_simul.h"
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
