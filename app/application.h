//---------------------------------------------------------------------------
// application.h - Portage de Centre-Ville : démarrage de l'application
//---------------------------------------------------------------------------
#ifndef applicationH
#define applicationH
//---------------------------------------------------------------------------
#include <QString>
class QApplication;
//---------------------------------------------------------------------------
void InitialiseApplication(QApplication &App); // locale, traductions, polices
void CreeFiches(const QString &asFichierFinder=QString()); // fiches de cvw.cpp, dans le même ordre
void DetruitFiches();
//---------------------------------------------------------------------------
#endif
