//---------------------------------------------------------------------------
// application.h - Portage de Centre-Ville : démarrage de l'application
//---------------------------------------------------------------------------
#ifndef applicationH
#define applicationH
//---------------------------------------------------------------------------
#include <QObject>
#include <QString>
class QApplication;
//---------------------------------------------------------------------------
// macOS : ouverture d'un .cvw depuis le Finder (QFileOpenEvent). Avant la
// création de frmSimulation, le fichier est mis en attente (EnAttente) ;
// ensuite, il est ouvert comme par Ouvrir (TfrmSimulation::OuvreFichierDemande).
class OuvertureFichiers : public QObject
 {
  public:
  QString EnAttente;
  bool eventFilter(QObject *o, QEvent *e) override;
 };
//---------------------------------------------------------------------------
void InitialiseApplication(QApplication &App); // locale, traductions, polices
void CreeFiches(const QString &asFichierFinder=QString()); // fiches de cvw.cpp, dans le même ordre
void DetruitFiches();
//---------------------------------------------------------------------------
#endif
