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
#include <QDir>
#include <QMessageBox>
#include <exception>
#include "application.h"
#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
#include "base.h" // stRepLocalAppData, CopieCartesExemple
#include "vcl.h"  // Q
#endif
//---------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QApplication App(argc, argv);
    InitialiseApplication(App);
#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
    // Cartes d'exemple copiées au premier lancement de chaque utilisateur dans le
    // dossier de stRepLocalAppData (app/CMakeLists.txt) : sous macOS (v6.0.1),
    // embarquées dans l'exécutable ; sous Windows (6.0.21), installées dans
    // {app}\cartes, l'installeur n'écrivant plus dans le dossier de l'utilisateur
    // qui l'exécute en administrateur. Sous Linux, l'AppImage les copie avant le
    // lancement (linux/apprun-hooks). stRepLocalAppData se rabat sur le dossier de
    // l'exécutable s'il ne peut créer le sien : on n'y écrit pas (paquet signé,
    // Program Files)
#ifdef Q_OS_MACOS
    const QString stCartes = ":/cartes";
#else
    const QString stCartes = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../cartes");
#endif
    if (Q(stRepLocalAppData()) != QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "/"))
    {
        CopieCartesExemple(stCartes, Q(stRepLocalAppData()));
    }
#endif
    OuvertureFichiers Ouverture;
    App.installEventFilter(&Ouverture);
    int Resultat = 0;
    try
    {
        CreeFiches(Ouverture.EnAttente);
        Resultat = QApplication::exec(); // Application->Run();
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
