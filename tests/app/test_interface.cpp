//---------------------------------------------------------------------------
// test_interface.cpp - Tests de l'application Qt (phases 4 à 6)
//
// Lance l'interface comme « cvw <carte> » (copie de maps/Démo.cvw dans un
// dossier temporaire, car Marche enregistre le fichier) puis la pilote comme
// un utilisateur : boutons de la barre d'outils, éléments de menu, raccourcis
// clavier. Les boîtes de message (bilan...) reçoivent la réponse par défaut
// de l'utilisateur pressé : « Non ». Vérifie :
//   - le chargement par la ligne de commande, le titre, la taille de la grille ;
//   - les raccourcis selon le contexte (Ctrl+B : passage de bus en édition,
//     détail d'un bus en simulation) ;
//   - Marche / Interruption / Arrêt, par la barre d'outils et par les menus,
//     et la mise à jour de la barre d'état ;
//   - l'enregistrement : enregistrer puis rouvrir et réenregistrer donne le
//     même fichier, identique à la conversion de référence du format ; un
//     fichier impossible à créer ou à écrire est signalé (v6.0.1) ;
//   - la vérification d'intégrité après ouverture affiche les erreurs des
//     contrôles de champs (v6.0.1) ;
//   - fichier impossible à ouvrir : message, réseau affiché gardé (v6.0.1) ;
//   - Marche sur un fichier en lecture seule (v6.0.1) : non modifié, simulation
//     sans enregistrement, avec les mêmes tirages, et Arrêt recharge le réseau
//     d'origine ; modifié, question puis « Enregistrer sous », sinon pas de
//     démarrage ;
//   - démarrage interrompu (v6.0.1) : erreurs d'intégrité, trams impossibles à
//     placer, réseau non connexe, enregistrement annulé ou en échec, copie du
//     réseau impossible. Plusieurs clics sur Marche, sous une limite de temps
//     (jusqu'à la 6.0.0, le 2e clic tournait sans fin dans PrepareBus), puis un
//     démarrage sur le réseau rétabli ;
//   - Arrêt quand le fichier n'est plus accessible : réseau rétabli (v6.0.1) ;
//   - recette : carte d'un vrai volume en lecture seule, s'il est désigné par
//     CV_CARTE_LECTURE_SEULE (sinon sauté) ;
//   - Nouveau puis génération aléatoire (fiches Dimensions et Génération) à
//     graine fixe : même réseau que le golden master de la génération ;
//   - lot 1 des fiches secondaires (paramètres et définitions), sur une copie
//     neuve de Démo : chaque fiche est ouverte par son menu, son bouton ou son
//     action, un paramètre y est modifié et validé, puis son effet sur le
//     moteur (cv->...) est vérifié ; Annuler ne change rien ; Paramètres et
//     Carrefours : enregistrer puis rouvrir conserve les valeurs. L'écran
//     « À propos » du démarrage s'anime puis se ferme seul ;
//   - lot 2 (distances et informations), en simulation à graine fixe : les
//     six tableaux de distances ouverts par leurs menus, actions et raccourcis
//     affichent les distances du moteur pour la case courante (Géant, qui a
//     des arrêts, des files et des places de chaque sorte, puis Démo), se redimensionnent
//     d'après le nombre de lignes et se ferment à l'arrêt ; les six fiches
//     d'informations (non modales, au premier plan) suivent tour après tour
//     l'objet de la case courante, l'utilisateur cliquant sur lui quand il se
//     déplace ; à l'arrêt, elles gardent leur contenu jusqu'au clic suivant
//     sur la carte (comme l'original), qui les vide ;
//   - lot 3 (statistiques et recherche), sur des copies de Géant : composition
//     du réseau (modale) calculée sur la carte ; « Évolution » redessinée par le
//     moteur à chaque tour, courbes et quadrillage aux positions que donnent
//     cv->Stats ; statistiques des véhicules d'une étape à l'autre d'après les
//     véhicules arrivés ; fluidité et trafic (modale, proposée par le bilan de
//     l'interruption) : couleur de chaque case de la carte égale au calcul sur
//     l'attente et le trafic du moteur, totaux, trajets, boutons et touches de
//     déplacement ; Rechercher : chaque sorte d'élément fixe retrouvée, et
//     « Suivre » un véhicule, un piéton, un bus, un tram, un taxi, un vehlib :
//     curseur clignotant sur l'objet et fiche d'informations qui le suit, tour
//     après tour, sans clic ; numéros tapés hors bornes (Fluidité, Rechercher),
//     ramenés dans les bornes en quittant le champ ;
//   - couche VCL : TCSpinEdit comme CSPIN.pas (MinValue/MaxValue sans effet ni
//     OnChange, Value= borné, frappe non bornée jusqu'à la sortie du champ,
//     flèches, EditorEnabled) ;
//   - aide (phase 7) : F1 sur la fiche principale, une fiche non modale et une
//     fiche modale, bouton Aide, Alt+Maj+F1, menus Sommaire et Tutoriel
//     (QDesktopServices::openUrl intercepté ; la table des rubriques et les
//     ancres sont vérifiées par test_aide.cpp).
// Les fiches modales sont pilotées par des étapes (QuandModale) : l'action
// prévue est exécutée quand la fiche attendue devient la fiche modale active.
// Simulations pas à pas (Demarre) : le minuteur de la simulation est arrêté et
// le test joue lui-même chaque tour (JoueTour), pour qu'aucun tour ne tombe
// entre ce qu'il observe et ce qu'il vérifie ; marcheInterruptionArret seule
// laisse tourner le minuteur (temps réel).
// Les captures de la fenêtre principale et des fiches sont écrites dans le
// dossier CV_DOSSIER_CAPTURES (Démo au tour 0 et vers le tour 200, fiche_*.png,
// carte_*.png pour la carte en statistiques de fluidité et de trafic).
//---------------------------------------------------------------------------
#include <QAbstractButton>
#include <QApplication>
#include <QCheckBox>
#include <QDesktopServices>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QFileOpenEvent>
#include <QFocusEvent>
#include <QFontDatabase>
#include <QFontInfo>
#include <QLineEdit>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QScrollBar>
#include <QTabBar>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <QUrl>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <mutex>
#include <sstream>
#include <thread>
#ifndef _WIN32
 #include <csignal>
 #include <sys/resource.h>
#endif
#include "application.h"
#include "f_afdistab.h"
#include "f_afdistat.h"
#include "f_afdistfp.h"
#include "f_afdistft.h"
#include "f_afdistp.h"
#include "f_afdistpv.h"
#include "f_carfours.h"
#include "f_deff.h"
#include "f_deffp.h"
#include "f_defn.h"
#include "f_defv.h"
#include "f_defvp.h"
#include "f_dimzone.h"
#include "f_epidemie.h"
#include "f_genres.h"
#include "f_infbus.h"
#include "f_infptn.h"
#include "f_inftaxi.h"
#include "f_inftram.h"
#include "f_infveh.h"
#include "f_infvehlib.h"
#include "f_prmbus.h"
#include "f_prmsim.h"
#include "f_prmtaxi.h"
#include "f_prmtram.h"
#include "f_prmvehlib.h"
#include "f_propos.h"
#include "f_rech.h"
#include "f_simul.h"
#include "f_stats.h"
#include "f_statsat.h"
#include "f_statsc.h"
#include "f_statsv.h"
#include "format_reference.h"
#include "compat/fichier.h"
#include "compat/alea.h" // après les en-têtes système
//---------------------------------------------------------------------------
extern TfrmGeneration *frmGeneration; // défini dans f_genres.cpp (non déclaré par f_genres.h d'origine)
extern TfrmStatsComposition *frmStatsComposition; // défini dans f_statsc.cpp (déclaration en commentaire dans f_statsc.h)
//---------------------------------------------------------------------------
namespace
 {
  QString DossierCaptures;
  QStringList ArgumentsTest;
  //-------------------------------------------------------------------------
  QByteArray Contenu(const QString &asFichier)
   {
    QFile f(asFichier);
    return f.open(QIODevice::ReadOnly)?f.readAll():QByteArray();
   }
  //-------------------------------------------------------------------------
  // f_carfours.cpp : DirPossC4x4[x-xd][y-yd]
  const dirposs DirPossC4x4[4][4]=
   {{aucune,      O,      E, aucune},
    {     S,     SO,     SE,      S},
    {     N,     NO,     NE,      N},
    {aucune,      O,      E, aucune}};
  //-------------------------------------------------------------------------
  // Case libre : sans voie, nature, parking, arrêt, place ni feu
  bool CaseLibre(int x, int y)
   {
    voie &v=cv->v[(x+cv->NbX)%cv->NbX][(y+cv->NbY)%cv->NbY];
    return !v.DirPoss&&!v.SensVoieTram&&!NATURE(&v)&&!v.NumParking&&!v.NumArretBus&&!v.NumArretTram&&
           !v.NumPlaceTaxi&&!v.NumPlacePark&&!v.NumPlaceVehlib&&!v.PassagePietons&&!v.EstDepotBus()&&
           !v.NumFeu[0]&&!v.NumFeu[1]&&!v.NumFeu[2]&&!v.NumFeu[3]&&
           !v.NumFeuP[0]&&!v.NumFeuP[1]&&!v.NumFeuP[2]&&!v.NumFeuP[3];
   }
  //-------------------------------------------------------------------------
  // Zone libre de l à h cases entourée d'une marge libre, en haut à gauche de
  // la carte (visible sans défilement), hors des zones déjà prises
  bool ZoneLibre(int l, int h, int &xz, int &yz, const std::vector<QRect> &Prises={})
   {
    for(int y=1; y+h<24; y++)
     for(int x=1; x+l<36; x++)
      {
       QRect r(x-1, y-1, l+2, h+2);
       bool Ok=true;
       for(const QRect &p : Prises) Ok=Ok&&!p.intersects(r);
       for(int i=-1; Ok&&(i<=l); i++)
        for(int j=-1; Ok&&(j<=h); j++)
         Ok=CaseLibre(x+i, y+j);
       if (Ok) { xz=x; yz=y; return true; }
      }
    return false;
   }
  //-------------------------------------------------------------------------
  int NbFeuxPietonsDefinis()
   {
    int n=0;
    for(int i=0; i<cv->NbFeuxPieton; i++) n+=cv->FeuP[i].Defini();
    return n;
   }
  //-------------------------------------------------------------------------
  // Limite de temps d'un clic sur Marche : le fil principal, s'il tourne sans
  // fin, ne rendrait jamais la main à QTest ; seul un autre fil peut l'arrêter
  class LimiteDeTemps
   {
    std::mutex Verrou;
    std::condition_variable Fin;
    bool Termine=false;
    std::thread Fil;
    public:
    LimiteDeTemps(const char *Quoi, int Secondes) : Fil([this, Quoi, Secondes]()
     {
      std::unique_lock<std::mutex> v(Verrou);
      if (!Fin.wait_for(v, std::chrono::seconds(Secondes), [this]() { return Termine; }))
       {
        std::fprintf(stderr, "FAIL! : blocage de plus de %d s : %s\n", Secondes, Quoi);
        std::fflush(stderr);
        std::_Exit(3);
       }
     }) {}
    ~LimiteDeTemps()
     {
      { std::lock_guard<std::mutex> v(Verrou); Termine=true; }
      Fin.notify_all();
      Fil.join();
     }
   };
  //-------------------------------------------------------------------------
  // Fichier en lecture seule le temps d'un test, droits rétablis même s'il
  // échoue (QVERIFY sort de la fonction) : le dossier temporaire reste effaçable
  class LectureSeule
   {
    QString Fichier;
    QFile::Permissions Droits;
    public:
    explicit LectureSeule(const QString &f) : Fichier(f), Droits(QFile::permissions(f))
     { QFile::setPermissions(f, QFile::ReadOwner|QFile::ReadUser|QFile::ReadGroup|QFile::ReadOther); }
    ~LectureSeule() { QFile::setPermissions(Fichier, Droits); }
    bool Effective() const { return !QFile(Fichier).open(QIODevice::ReadWrite); } // Faux pour root
   };
  //-------------------------------------------------------------------------
  // Réseau tel qu'il serait enregistré, pour le comparer à un autre
  QByteArray OctetsReseau()
   {
    QTemporaryDir Dossier;
    QString f=Dossier.path()+"/reseau.cvw";
    int hfile=FileCreate(A(QDir::toNativeSeparators(f)));
    if (hfile==-1) return QByteArray();
    bool Ok=cv->Sauve(hfile);
    FileClose(hfile);
    return Ok?Contenu(f):QByteArray();
   }
  //-------------------------------------------------------------------------
  // Position de tout ce qui se déplace : deux simulations aux mêmes tirages ont la même
  QString EmpreinteSimulation()
   {
    QString e=QString("%1 %2 %3:").arg(cv->TourCrt).arg(cv->NbVehiculesSortis).arg(cv->NbVehiculesArrives);
    for(int i=0; i<cv->NbVehicules; i++) e+=QString(" %1,%2").arg(cv->Vehicule[i].x).arg(cv->Vehicule[i].y);
    for(int i=0; i<cv->NbPietons; i++) e+=QString(" %1,%2").arg(cv->Pieton[i].x).arg(cv->Pieton[i].y);
    for(int i=0; i<cv->NbBus; i++) e+=QString(" %1,%2").arg(cv->Bus[i].x).arg(cv->Bus[i].y);
    for(int i=0; i<cv->NbTrams; i++) e+=QString(" %1,%2").arg(cv->Tram[i].x).arg(cv->Tram[i].y);
    for(int i=0; i<cv->NbTaxis; i++) e+=QString(" %1,%2").arg(cv->Taxi[i].x).arg(cv->Taxi[i].y);
    for(int i=0; i<cv->NbVehlibs; i++) e+=QString(" %1,%2").arg(cv->Vehlib[i].x).arg(cv->Vehlib[i].y);
    return e;
   }
  //-------------------------------------------------------------------------
  QWidget *BoiteFichier()
   {
    return qobject_cast<QFileDialog *>(QApplication::activeModalWidget());
   }
  void ChoisitFichier(const QString &asFichier)
   {
    QFileDialog *d=qobject_cast<QFileDialog *>(QApplication::activeModalWidget());
    // (selectFile n'écrit pas dans le champ du nom quand celui-ci a le focus)
    d->findChild<QLineEdit *>("fileNameEdit")->setText(asFichier);
    QMetaObject::invokeMethod(d, "accept"); // créneau protégé
   }
  void AnnuleBoiteFichier()
   {
    qobject_cast<QFileDialog *>(QApplication::activeModalWidget())->reject();
   }
#ifndef _WIN32
  //-------------------------------------------------------------------------
  // Taille des fichiers limitée à 1000 octets, comme un disque plein. Quand la
  // barre d'état se redessine avec asEtape (donc au milieu de Marche), si elle est donnée
  class LimiteFichiers : public QObject
   {
    struct rlimit Ancienne;
    void (*AncienSignal)(int);
    QString Etape;
    void Pose()
     {
      struct rlimit Limite=Ancienne;
      Limite.rlim_cur=1000;
      setrlimit(RLIMIT_FSIZE, &Limite);
     }
    public:
    explicit LimiteFichiers(const QString &asEtape=QString()) : Etape(asEtape)
     {
      getrlimit(RLIMIT_FSIZE, &Ancienne);
      AncienSignal=std::signal(SIGXFSZ, SIG_IGN);
      if (Etape.isEmpty()) Pose();
      else frmSimulation->StatusBar->installEventFilter(this);
     }
    ~LimiteFichiers() override
     {
      setrlimit(RLIMIT_FSIZE, &Ancienne);
      std::signal(SIGXFSZ, AncienSignal);
     }
    bool eventFilter(QObject *o, QEvent *e) override
     {
      if ((e->type()==QEvent::Paint)&&(Q(frmSimulation->StatusBar->Panels->Items[0]->Text)==Etape)) Pose();
      return QObject::eventFilter(o, e);
     }
   };
#endif
 }
//---------------------------------------------------------------------------
class TestInterface : public QObject
 {
  Q_OBJECT
  QString Carte;
  QStringList Messages; // boîtes de message affichées (titre : texte)
  QTimer Repondeur;
  QMessageBox::StandardButton ReponseQuestions=QMessageBox::No; // réponse aux boîtes Oui/Non
  // Fiches modales à piloter, dans l'ordre : quand Fiche() devient la fiche
  // modale active, Action est exécutée (hors du minuteur, car elle peut ouvrir
  // une autre fiche modale)
  struct Etape { std::function<QWidget *()> Fiche; std::function<void()> Action; };
  std::deque<Etape> Etapes;
  QPointer<QWidget> ModaleSansReponse;
  QElapsedTimer DureeSansReponse;
  void QuandModale(std::function<QWidget *()> Fiche, std::function<void()> Action)
   {
    Etapes.push_back({std::move(Fiche), std::move(Action)});
   }
  void CaptureFiche(QWidget *Fiche, const QString &asNom)
   {
    if (DossierCaptures.isEmpty()) return;
    QDir().mkpath(DossierCaptures);
    QApplication::processEvents();
    QVERIFY(Fiche->grab().save(DossierCaptures+"/"+asNom));
   }
  void Clique(QWidget *w)
   { // Cases à cocher et boutons radio : clic à l'extrémité droite, hors du
     // texte (la VCL y réagit ; Qt, seulement sur la case et le texte)
    if (qobject_cast<QCheckBox *>(w)||qobject_cast<QRadioButton *>(w))
     QTest::mouseClick(w, Qt::LeftButton, Qt::KeyboardModifiers(), QPoint(w->width()-3, w->height()/2));
    else
     QTest::mouseClick(w, Qt::LeftButton);
   }
  void Touche(QWidget *w, Qt::Key Touche)
   { // Touche frappée dans le contrôle actif (qui reçoit le clavier)
    w->window()->activateWindow();
    w->setFocus(Qt::OtherFocusReason);
    QTest::keyClick(w, Touche);
   }
  void CliqueChoix(TRadioGroup *g, int i)
   { // Clic sur le i-ème bouton d'un TRadioGroup (<Nom>_<i> créés par dfm2ui)
    QRadioButton *b=g->findChild<QRadioButton *>(g->objectName()+"_"+QString::number(i));
    QVERIFY(b);
    Clique(b);
   }
  void PlaceCurseur(int x, int y)
   { // Case courante (et ancre), puis OnDrawCell de la case : état des actions
    frmSimulation->DrawGridSimul->Col=x;
    frmSimulation->DrawGridSimul->Row=y;
    frmSimulation->DrawGridSimul->Repaint();
   }
  void Selectionne(int x1, int y1, int x2, int y2)
   { // Clic sur (x1, y1) puis Maj+clic sur (x2, y2), comme l'utilisateur
    GrilleSimulation *g=frmSimulation->DrawGridSimul;
    g->LeftCol=0;
    g->TopRow=0;
    auto Centre=[g](int x, int y) { TRect r=g->CellRect(x, y); return QPoint((r.Left+r.Right)/2, (r.Top+r.Bottom)/2); };
    QTest::mouseClick(g->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(), Centre(x1, y1));
    QTest::mouseClick(g->viewport(), Qt::LeftButton, Qt::ShiftModifier, Centre(x2, y2));
    g->Repaint();
    QCOMPARE(g->Selection.Left, qMin(x1, x2));
    QCOMPARE(g->Selection.Top, qMin(y1, y2));
    QCOMPARE(g->Selection.Right, qMax(x1, x2));
    QCOMPARE(g->Selection.Bottom, qMax(y1, y2));
   }
  void OuvreFichier(const QString &asFichier)
   { // Ouverture comme depuis le Finder (QFileOpenEvent, macOS)
    OuvertureFichiers Ouverture;
    qApp->installEventFilter(&Ouverture);
    QFileOpenEvent Evenement(asFichier);
    QCoreApplication::sendEvent(qApp, &Evenement);
    qApp->removeEventFilter(&Ouverture);
   }
  QString OuvreDemo(const QString &asNom)
   { // Copie neuve de maps/Démo.cvw, ouverte
    QString f=QFileInfo(Carte).absolutePath()+"/"+asNom;
    QFile::remove(f);
    if (!QFile::copy(ArgumentsTest.value(2), f)) return QString();
    OuvreFichier(f);
    return f;
   }
  void Enregistre(const QString &asFichier)
   {
    frmSimulation->SaveDialog->FileName=A(QDir::toNativeSeparators(asFichier));
    frmSimulation->ActionEnregistrer->Execute();
   }
  void Capture(const QString &asNom)
   {
    if (DossierCaptures.isEmpty()) return;
    QDir().mkpath(DossierCaptures);
    QVERIFY(frmSimulation->grab().save(DossierCaptures+"/"+asNom));
   }
  bool AttendTour(long Tour, int DelaiMs)
   {
    return QTest::qWaitFor([Tour]() { return cv&&(cv->TourCrt>=Tour); }, DelaiMs);
   }
  // Simulation pas à pas : un tic du minuteur de la simulation (TimerSimulTimer,
  // par OnTimer comme le QTimer), puis les événements en attente (dessins), comme
  // dans la boucle de l'application ; le minuteur, arrêté par Demarre, ne joue
  // aucun tour pendant que le test clique et vérifie
  void JoueTour()
   {
    frmSimulation->TimerSimul->OnTimer(frmSimulation->TimerSimul);
    QApplication::processEvents();
   }
  // Tours joués jusqu'à ce que Fin() soit vraie (au plus NbMax tours)
  bool JoueTours(std::function<bool()> Fin, int NbMax)
   {
    for(int i=0; (i<NbMax)&&!Fin()&&frmSimulation->SimulationEnCours; i++) JoueTour();
    return Fin();
   }
  bool JoueJusquAuTour(long Tour)
   {
    return JoueTours([Tour]() { return cv->TourCrt>=Tour; }, 10000);
   }
  // Arrêt et Interruption ne sont traités qu'au tic suivant du minuteur
  // (TypeDemande) : relancé après la demande, son premier tic la traite sans
  // jouer de tour
  void RelanceMinuteur()
   {
    if (frmSimulation->SimulationEnCours) frmSimulation->TimerSimul->Enabled=true;
   }
  QString OuvreCarte(const QString &asCarte, const QString &asNom)
   { // Copie neuve de maps/<asCarte>, ouverte
    QString f=QFileInfo(Carte).absolutePath()+"/"+asNom;
    QFile::remove(f);
    if (!QFile::copy(QFileInfo(ArgumentsTest.value(2)).absolutePath()+"/"+asCarte, f)) return QString();
    OuvreFichier(f);
    return f;
   }
  bool Demarre()
   { // Marche par la barre d'outils : vérification, calcul des distances, enregistrement.
     // Graine fixe (srand de AppelleAuChaos) : simulation reproductible. Jusqu'à la
     // 6.0.0, elle évitait aussi le blocage des vehlibs de Géant (sans place libre,
     // vehlib::ReservePlaceVehlibCible tirait sans fin), corrigé en 6.0.1
    // Simulation pas à pas ensuite : minuteur arrêté, tours joués par JoueTour
    alea::FixeGraine(1);
    Messages.clear();
    QTest::mouseClick(frmSimulation->ToolButtonMarche, Qt::LeftButton);
    frmSimulation->TimerSimul->Enabled=false;
    return frmSimulation->SimulationEnCours&&cv->DistancesCalculees;
   }
  // Marche sous une limite de temps (Demarre, comme l'utilisateur)
  bool DemarreSansBlocage(const char *Quoi)
   {
    LimiteDeTemps Limite(Quoi, 120);
    return Demarre();
   }
  // Marche n'a rien démarré : les boutons et le menu d'Arrêt sont grisés, Marche
  // est disponible et le réseau est celui d'avant le clic (Octets) ; aucun bus
  // ni tram (les Prepare* les comptent sur leur ligne, à l'origine du blocage)
  void VerifieNonDemarre(const QByteArray &Octets)
   {
    QVERIFY(!frmSimulation->SimulationEnCours);
    QVERIFY(!frmSimulation->Pause);
    QVERIFY(frmSimulation->ActionMarche->Enabled);
    QVERIFY(!frmSimulation->ActionArret->Enabled);
    QVERIFY(!frmSimulation->ToolButtonArret->isEnabled());
    QVERIFY(!frmSimulation->MenuItemArret->ElementQt()->isEnabled());
    QVERIFY(frmSimulation->DrawGridSimul->PopupMenu==frmSimulation->PopupMenuEditionCase);
    QVERIFY(!cv->DistancesCalculees);
    QCOMPARE((long)cv->TourCrt, 0L);
    for(int i=0; i<NBMAXLIGNESBUSTRAM; i++)
     {
      QCOMPARE(cv->LigneBus[i].NbBus, 0);
      QCOMPARE(cv->LigneTram[i].NbTrams, 0);
     }
    QVERIFY(!cv->Vehicule);
    QVERIFY(OctetsReseau()==Octets);
   }
  // Réseau de deux boucles à sens unique, chacune avec son parking, sans lien
  // entre elles : Marche le trouve correct, puis non connexe (distances indéfinies)
  QString CreeReseauNonConnexe(const QString &asNom)
   {
    QString f=QFileInfo(Carte).absolutePath()+"/"+asNom;
    centre_ville *Precedent=cv;
    cv=new centre_ville(true, 20, 12); // éléments par défaut, à ajuster
    cv->DefinitNbParkings(2);
    cv->DefinitNbVehicules(8);
    cv->DefinitNbPietons(0);
    for(int x0 : {2, 10})
     {
      const int x1=x0+4, y0=2, y1=6;
      for(int x=x0; x<x1; x++) cv->v[x][y0].DirPoss=E;
      for(int y=y0; y<y1; y++) cv->v[x1][y].DirPoss=S;
      for(int x=x1; x>x0; x--) cv->v[x][y1].DirPoss=O;
      for(int y=y1; y>y0; y--) cv->v[x0][y].DirPoss=N;
      cv->AjouteParking(x0+2, y0);
     }
    int hfile=FileCreate(A(QDir::toNativeSeparators(f)));
    bool Ok=(hfile!=-1)&&cv->Sauve(hfile);
    if (hfile!=-1) FileClose(hfile);
    delete cv;
    cv=Precedent;
    if (!Ok) return QString();
    OuvreFichier(f);
    return f;
   }
  bool Arrete()
   { // Arrêt par la barre d'outils : bilan, puis rechargement du réseau
    QTest::mouseClick(frmSimulation->ToolButtonArret, Qt::LeftButton);
    RelanceMinuteur();
    return QTest::qWaitFor([]() { return !frmSimulation->SimulationEnCours&&!frmSimulation->Pause&&(cv->TourCrt==0); }, 10000);
   }
  void CliqueCase(int x, int y)
   { // Clic de l'utilisateur sur la case (x, y), amenée dans la vue, puis le
     // dessin qui le suit avant toute autre action de l'utilisateur : OnDrawCell
     // de la case courante règle l'état des actions (« Suivre... »)
    GrilleSimulation *g=frmSimulation->DrawGridSimul;
    g->LeftCol=qMax(0, x-4);
    g->TopRow=qMax(0, y-4);
    TRect r=g->CellRect(x, y);
    QTest::mouseClick(g->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(), QPoint((r.Left+r.Right)/2, (r.Top+r.Bottom)/2));
    g->Repaint();
   }
  int TrouveCase(std::function<int(voie *)> Numero)
   { // Premier objet trouvé sur la carte (Numero(v) > 0), cliqué par l'utilisateur
    for(int y=0; y<cv->NbY; y++)
     for(int x=0; x<cv->NbX; x++)
      if (int n=Numero(&cv->v[x][y]))
       {
        Focalise();
        CliqueCase(x, y);
        return n;
       }
    return 0;
   }
  void VerifieFichesDistances(const QString &asCarte, bool Captures);
  void SuitObjet(const char *Type, std::function<QString(long)> Ecart, std::function<bool(int &, int &)> Position, int NbTours);
  void Focalise()
   {
    frmSimulation->activateWindow();
    (void)QTest::qWaitForWindowActive(frmSimulation, 2000);
    frmSimulation->DrawGridSimul->setFocus();
    QApplication::processEvents();
   }
  QList<QUrl> UrlsOuvertes; // QDesktopServices::openUrl, intercepté par UrlOuverte
  QUrl AdresseAide(const QUrl &Url)
   { // Aide contextuelle : adresse que charge la page de redirection (base.cpp)
    QRegularExpressionMatch m=QRegularExpression("url='([^']*)'").match(QString::fromUtf8(Contenu(Url.toLocalFile())));
    return m.hasMatch()?QUrl(m.captured(1)):Url;
   }
  public slots:
  void UrlOuverte(const QUrl &Url) { UrlsOuvertes<<Url; }
  private slots:
  void initTestCase();
  void init();
  void aProposAuDemarrage();
  void ouvertureParLigneDeCommande();
  void raccourcisSelonLeMode();
  void marcheInterruptionArret();
  void enregistrementIdentique();
  void enregistrementImpossible();
  void verificationApresOuverture();
  void ouvertureImpossible();
  void marcheLectureSeuleNonModifie();
  void marcheLectureSeuleModifie();
  void interruptionErreursIntegrite();
  void interruptionTramsImpossibles();
  void interruptionReseauNonConnexe();
  void interruptionEnregistrementAnnule();
  void interruptionEnregistrementEnEchec();
  void arretFichierInaccessible();
  void carteDUnVolumeEnLectureSeule();
  void nouveauEtGeneration();
  // Lot 1 des fiches secondaires (phase 6)
  void ficheAPropos();
  void ficheEpidemie();
  void ficheNature();
  void fichesPrioritesEtFeux();
  void fichesPrioritesEtFeuxPietons();
  void fichesParametres();
  void ficheCarrefours();
  // Lot 2 des fiches secondaires (phase 6)
  void fichesDistances();
  void fichesInfos();
  // Lot 3 des fiches secondaires (phase 6)
  void ficheComposition();
  void fichesEvolutionEtVehicules();
  void ficheFluiditeTrafic();
  void ficheRecherche();
  // Couche VCL
  void controleCSpinEdit();
  // Services système (phase 7)
  void aideContextuelle();
  // Affichage natif de Windows (recette)
  void policesEtStyleWindows();
  void cleanupTestCase();
 };
//---------------------------------------------------------------------------
void TestInterface::initTestCase()
 {
  // Les boîtes de message reçoivent « Non » (ou OK) et sont mémorisées
  Repondeur.setInterval(20);
  QObject::connect(&Repondeur, &QTimer::timeout, this, [this]()
   {
    QWidget *Modale=QApplication::activeModalWidget();
    if (Modale&&!Etapes.empty()&&(Modale==Etapes.front().Fiche()))
     {
      std::function<void()> Action=Etapes.front().Action;
      Etapes.pop_front();
      ModaleSansReponse=nullptr;
      QTimer::singleShot(0, this, Action);
      return;
     }
    if (frmDimensionsZone&&(Modale==frmDimensionsZone))
     { // Nouveau : dimensions proposées, génération aléatoire, sans les paramètres
      frmDimensionsZone->CheckBoxAfficherParametres->Checked=false;
      frmDimensionsZone->CheckBoxGenererReseau->Checked=true;
      QTest::mouseClick(frmDimensionsZone->ButtonCreer, Qt::LeftButton);
      return;
     }
    if (frmGeneration&&(Modale==frmGeneration))
     {
      QTest::mouseClick(frmGeneration->ButtonOk, Qt::LeftButton);
      return;
     }
    QMessageBox *b=qobject_cast<QMessageBox *>(Modale);
    if (!b)
     { // Fiche modale sans étape prévue : fermée au bout de 10 s (le test échoue sans rester bloqué)
      if (Modale!=ModaleSansReponse) { ModaleSansReponse=Modale; DureeSansReponse.start(); }
      else if (Modale&&DureeSansReponse.hasExpired(10000))
       {
        Messages<<("Fiche modale sans réponse : "+Modale->windowTitle());
        ModaleSansReponse=nullptr;
        Modale->close();
       }
      return;
     }
    Messages<<(b->windowTitle()+" : "+b->text());
    QAbstractButton *r=b->button(ReponseQuestions);
    if (!r) r=b->button(QMessageBox::Ok);
    if (!r) r=b->buttons().value(0);
    if (r) r->click();
   });
  Repondeur.start();
  Carte=ArgumentsTest.value(1);
  QVERIFY2(QFile::exists(Carte), qPrintable(Carte));
  CreeFiches();
  QVERIFY(frmSimulation);
 }
//---------------------------------------------------------------------------
void TestInterface::init()
 {
  Etapes.clear();
  Messages.clear();
  ReponseQuestions=QMessageBox::No;
  // Simulation laissée en cours (ou en pause : Arrêt y est désactivé) par un test en échec : arrêtée
  if (frmSimulation->Pause) QTest::mouseClick(frmSimulation->ToolButtonMarche, Qt::LeftButton);
  if (frmSimulation->SimulationEnCours||frmSimulation->Pause) (void)Arrete();
  alea::LibereGraine();
  Messages.clear();
 }
//---------------------------------------------------------------------------
void TestInterface::aProposAuDemarrage()
 { // f_simul FormShow : « À propos » affichée (non modale), feu vert puis texte
   // en vert au premier tic du minuteur, fermeture automatique au deuxième (2 s)
  QVERIFY(frmAPropos);
  QVERIFY(frmAPropos->isVisible());
  QVERIFY(!QApplication::activeModalWidget());
  QVERIFY(frmAPropos->FermetureAutomatique);
  QVERIFY(frmAPropos->Timer->Enabled);
  QCOMPARE(Q(frmAPropos->LabelAlignement->Caption), QString("version 64 bits"));
  // Version de project(VERSION) : ImageVersion (« majeure,mineure », dessinée
  // comme la bitmap d'origine « 5,4 », redessinée ici à l'identique) et
  // LabelSousVersion (« ,corrective »)
  auto ImageARGB=[](const QPixmap &p)
   { // pixels seulement (QImage::operator== compare aussi l'espace colorimétrique du PNG)
    QImage i=p.toImage().convertToFormat(QImage::Format_ARGB32);
    QList<QRgb> Pixels;
    for(int y=0; y<i.height(); y++)
     for(int x=0; x<i.width(); x++) Pixels<<i.pixel(x, y);
    return QPair<QSize, QList<QRgb>>(i.size(), Pixels);
   };
  QCOMPARE(ImageARGB(ImageDeVersion(5, 4)), ImageARGB(QPixmap(":/images/f_propos/ImageVersion.png")));
  QCOMPARE(ImageARGB(frmAPropos->ImageVersion->pixmap()), ImageARGB(ImageDeVersion(CV_VERSION_MAJEURE, CV_VERSION_MINEURE)));
  QCOMPARE(Q(frmAPropos->LabelSousVersion->Caption), QString(",%1").arg(CV_VERSION_CORRECTIVE));
  QVERIFY2(QRegularExpression("^HDPI x\\d,\\d$").match(Q(frmAPropos->LabelHDPI->Caption)).hasMatch(),
           qPrintable(Q(frmAPropos->LabelHDPI->Caption)));
  QVERIFY(frmAPropos->SpeedButtonOk->Glyph.Ref().Image.toImage()==frmAPropos->VirtualImageList->Pixmap(0).toImage());
  QVERIFY(!frmAPropos->SpeedButtonOk->icon().isNull());
  QVERIFY(QTest::qWaitFor([]() { return frmAPropos->LabelTexte->Font->Color==clLime; }, 3000));
  CaptureFiche(frmAPropos, "fiche_propos_demarrage.png");
  QVERIFY(QTest::qWaitFor([]() { return !frmAPropos->isVisible(); }, 4000));
  QVERIFY(!frmAPropos->FermetureAutomatique);
  QVERIFY(!frmAPropos->Timer->Enabled);
 }
//---------------------------------------------------------------------------
void TestInterface::ouvertureParLigneDeCommande()
 { // cvw <carte> : ouverture dans FormShow, sans boîte de dialogue
  QCOMPARE(Q(frmSimulation->Caption), QString("Centre-Ville - ")+Carte);
  QVERIFY(cv);
  QCOMPARE((int)frmSimulation->DrawGridSimul->ColCount, cv->NbX);
  QCOMPARE((int)frmSimulation->DrawGridSimul->RowCount, cv->NbY);
  QCOMPARE(Q(frmSimulation->StatusBar->Panels->Items[0]->Text), QString::fromUtf8("Chargement effectué."));
  // Ouvrir et Enregistrer démarrent dans le dossier des fichiers de Centre-Ville
  // (où l'installeur Windows et l'AppImage copient les cartes d'exemple)
  QCOMPARE(Q(frmSimulation->OpenDialog->InitialDir), Q(stRepLocalAppData()));
  QCOMPARE(Q(frmSimulation->SaveDialog->InitialDir), Q(stRepLocalAppData()));
  QVERIFY(!frmSimulation->SimulationModifiee);
  QVERIFY(frmSimulation->ActionMarche->Enabled);
  QVERIFY(!frmSimulation->ActionArret->Enabled);
  QVERIFY(Messages.isEmpty());
  QTest::qWait(200);
  frmSimulation->DrawGridSimul->Repaint();
  Capture("demo_tour0.png");
 }
//---------------------------------------------------------------------------
void TestInterface::raccourcisSelonLeMode()
 { // Ctrl+B : « Passage de bus » (menu contextuel d'édition) avant la simulation
  int x=-1, y=-1;
  for(int i=0; (x<0)&&(i<cv->NbX); i++)
   for(int j=0; j<cv->NbY; j++)
    if (cv->v[i][j].DirPoss&&!cv->v[i][j].PassageBus&&!cv->v[i][j].Priorite[0]&&!cv->v[i][j].NumParking)
     { x=i; y=j; break; }
  QVERIFY(x>=0);
  Focalise();
  frmSimulation->DrawGridSimul->Col=x;
  frmSimulation->DrawGridSimul->Row=y;
  frmSimulation->DrawGridSimul->Repaint(); // OnDrawCell de la case courante : état des actions
  QVERIFY(frmSimulation->ActionPassageBus->Enabled);
  QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_B, Qt::ControlModifier);
  QVERIFY(cv->v[x][y].PassageBus);
  QVERIFY(frmSimulation->SimulationModifiee);
  QVERIFY(Q(frmSimulation->Caption).endsWith('*'));
  QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_B, Qt::ControlModifier);
  QVERIFY(!cv->v[x][y].PassageBus);
  QVERIFY(!frmInfosBus->isVisible());
 }
//---------------------------------------------------------------------------
void TestInterface::marcheInterruptionArret()
 {
  // Marche par la barre d'outils : vérification, distances, enregistrement automatique
  QTest::mouseClick(frmSimulation->ToolButtonMarche, Qt::LeftButton);
  QVERIFY2(frmSimulation->SimulationEnCours, qPrintable(Messages.join('\n')));
  QVERIFY(!frmSimulation->SimulationModifiee); // enregistré avant la simulation
  QVERIFY(!frmSimulation->ActionMarche->Enabled);
  QVERIFY(frmSimulation->ActionInterruption->Enabled);
  QVERIFY(frmSimulation->ActionArret->Enabled);
  QVERIFY(frmSimulation->ToolButtonArret->isEnabled());
  QVERIFY(frmSimulation->DrawGridSimul->PopupMenu==frmSimulation->PopupMenuSimulation);
  QVERIFY(AttendTour(20, 30000));
  QVERIFY(Q(frmSimulation->StatusBar->Panels->Items[0]->Text).startsWith("Sur "));
  QVERIFY(Q(frmSimulation->Caption).startsWith("Centre-Ville - "+Carte+" ["));
  // Ctrl+B en simulation : « Détail sur un bus » (menu contextuel de simulation)
  Focalise();
  QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_B, Qt::ControlModifier);
  QVERIFY(frmInfosBus->isVisible());
  QVERIFY(frmSimulation->ActionDetailBus->Checked);
  Focalise(); // la fiche affichée est devenue active, comme avec Visible=true dans la VCL
  QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_B, Qt::ControlModifier);
  QVERIFY(!frmInfosBus->isVisible());
  // Interruption par le menu Simulation : bilan, puis pause
  QVERIFY(AttendTour(100, 60000));
  Messages.clear();
  frmSimulation->MenuItemPause->ElementQt()->trigger();
  QVERIFY(QTest::qWaitFor([]() { return frmSimulation->Pause; }, 5000));
  QVERIFY(!frmSimulation->SimulationEnCours);
  QVERIFY(frmSimulation->ActionMarche->Enabled);
  QVERIFY(!frmSimulation->ActionInterruption->Enabled);
  QVERIFY(QTest::qWaitFor([this]() { return !Messages.isEmpty(); }, 5000));
  // (pas de titre sous macOS : QMessageBox suit les règles d'Apple)
  QVERIFY2(Messages.first().contains(QString::fromUtf8("Bilan au tour n°")), qPrintable(Messages.first()));
  QVERIFY2(Messages.first().contains(QRegularExpression("\\d,\\d\\d")), qPrintable(Messages.first())); // séparateur décimal ','
  long TourPause=cv->TourCrt;
  QTest::qWait(300);
  QCOMPARE((long)cv->TourCrt, TourPause);
  // Reprise par la barre d'outils, jusque vers le tour 200 : capture
  QTest::mouseClick(frmSimulation->ToolButtonMarche, Qt::LeftButton);
  QVERIFY(frmSimulation->SimulationEnCours);
  QVERIFY(AttendTour(200, 60000));
  Capture("demo_tour200.png");
  // Arrêt par la barre d'outils : bilan, rechargement du réseau enregistré
  Messages.clear();
  QTest::mouseClick(frmSimulation->ToolButtonArret, Qt::LeftButton);
  QVERIFY(QTest::qWaitFor([]() { return !frmSimulation->SimulationEnCours&&!frmSimulation->Pause&&(cv->TourCrt==0); }, 5000));
  QVERIFY(QTest::qWaitFor([this]() { return !Messages.isEmpty(); }, 5000));
  QVERIFY(Messages.first().contains(QString::fromUtf8("Bilan au tour n°")));
  QCOMPARE(Q(frmSimulation->StatusBar->Panels->Items[0]->Text), QString::fromUtf8("Chargement effectué."));
  QVERIFY(frmSimulation->ActionMarche->Enabled);
  QVERIFY(!frmSimulation->ActionArret->Enabled);
  QVERIFY(frmSimulation->DrawGridSimul->PopupMenu==frmSimulation->PopupMenuEditionCase);
 }
//---------------------------------------------------------------------------
void TestInterface::enregistrementIdentique()
 { // Enregistrer, rouvrir, réenregistrer sous un autre nom : mêmes octets
  QString Dossier=QFileInfo(Carte).absolutePath();
  QString f1=Dossier+"/enregistrement1.cvw", f2=Dossier+"/enregistrement2.cvw";
  frmSimulation->SaveDialog->FileName=A(QDir::toNativeSeparators(f1));
  frmSimulation->ActionEnregistrer->Execute();
  QCOMPARE(Q(frmSimulation->StatusBar->Panels->Items[0]->Text), QString::fromUtf8("Enregistrement effectué."));
  // Réouverture comme depuis le Finder (QFileOpenEvent, macOS)
  OuvertureFichiers Ouverture;
  qApp->installEventFilter(&Ouverture);
  QFileOpenEvent Evenement(f1);
  QCoreApplication::sendEvent(qApp, &Evenement);
  qApp->removeEventFilter(&Ouverture);
  QCOMPARE(Q(frmSimulation->Caption), QString("Centre-Ville - ")+QDir::toNativeSeparators(f1));
  QCOMPARE(Q(frmSimulation->StatusBar->Panels->Items[0]->Text), QString::fromUtf8("Chargement effectué."));
  frmSimulation->SaveDialog->FileName=A(QDir::toNativeSeparators(f2));
  frmSimulation->ActionEnregistrer->Execute();
  QByteArray o1=Contenu(f1), o2=Contenu(f2);
  QVERIFY(!o1.isEmpty());
  QVERIFY(o1==o2);
  // ... et identique à la conversion de référence de la carte d'origine au format 0x530
  QString Origine=ArgumentsTest.value(2);
  conversion_cvw Reference=ConvertitEn530(Contenu(Origine).toStdString());
  QVERIFY(Reference.Ok);
  QVERIFY(o1.toStdString()==Reference.Octets);
 }
//---------------------------------------------------------------------------
void TestInterface::enregistrementImpossible()
 { // Une erreur d'enregistrement est signalée : jusqu'à la 6.0.0 (comme
   // l'original), les erreurs d'écriture étaient ignorées et la barre d'état
   // annonçait « Enregistrement effectué. »
  QString Dossier=QFileInfo(Carte).absolutePath();
  AnsiString asPrecedent=frmSimulation->SaveDialog->FileName;
  // Dossier inexistant : le fichier ne peut pas être créé
  Messages.clear();
  Enregistre(Dossier+"/inexistant/enregistrement.cvw");
  QCOMPARE(Messages.size(), 1);
  QVERIFY2(Messages.first().contains("Impossible de créer le fichier"), qPrintable(Messages.first())); // jusqu'à la 6.0.1-rc1 : « Erreur de données »
  QVERIFY2(Q(frmSimulation->StatusBar->Panels->Items[0]->Text).startsWith(QString::fromUtf8("Enregistrement non effectué"), Qt::CaseInsensitive),
           qPrintable(Q(frmSimulation->StatusBar->Panels->Items[0]->Text)));
#ifndef _WIN32
  // Taille de fichier limitée à 1000 octets, comme un disque plein : l'écriture échoue en cours de route
  QString f=Dossier+"/enregistrement_limite.cvw";
  QFile::remove(f);
  struct rlimit Ancienne, Limite;
  QVERIFY(getrlimit(RLIMIT_FSIZE, &Ancienne)==0);
  Limite=Ancienne;
  Limite.rlim_cur=1000;
  void (*AncienSignal)(int)=std::signal(SIGXFSZ, SIG_IGN);
  Messages.clear();
  QVERIFY(setrlimit(RLIMIT_FSIZE, &Limite)==0);
  Enregistre(f);
  setrlimit(RLIMIT_FSIZE, &Ancienne);
  std::signal(SIGXFSZ, AncienSignal);
  QCOMPARE(Messages.size(), 1);
  QVERIFY2(Messages.first().contains(QString::fromUtf8("Erreur de données dans le fichier")), qPrintable(Messages.first()));
  QCOMPARE(Q(frmSimulation->StatusBar->Panels->Items[0]->Text), QString::fromUtf8("Enregistrement NON effectué."));
  QVERIFY(frmSimulation->SimulationModifiee); // le fichier est tronqué : le réseau n'est plus que dans la mémoire
  frmSimulation->SimulationModifiee=false;
#endif
  QCOMPARE(Q(frmSimulation->SaveDialog->FileName), Q(asPrecedent)); // en échec, le nom courant ne change pas
 }
//---------------------------------------------------------------------------
void TestInterface::verificationApresOuverture()
 { // « Vérifier intégrité après ouverture » : les erreurs trouvées par les
   // contrôles de champs (ici VerifieDmdVert) sont affichées. Jusqu'à la
   // 6.0.0 (comme l'original), ces contrôles recevaient le message par valeur
   // et rien n'était affiché
  conversion_cvw Reference=ConvertitEn530(Contenu(ArgumentsTest.value(2)).toStdString());
  QVERIFY(Reference.Ok);
  std::string Octets=Reference.Octets;
  auto Debut=[&Reference](const std::string &Nom) -> long long
   {
    for(const champ_cvw &ch : Reference.Champs) if (ch.Nom==Nom) return (long long)ch.Debut;
    return -1;
   };
  int NumFeu=1; // premier feu défini (x>=0) : les places libres ne sont pas vérifiées
  for(long long d; ((d=Debut("feu"+std::to_string(NumFeu)+".x"))>=0)&&(Octets[(std::size_t)d+3]&0x80); NumFeu++);
  const long long DmdVert=Debut("feu"+std::to_string(NumFeu)+".TypeDmdVert");
  QVERIFY(DmdVert>=0);
  Octets[(std::size_t)DmdVert]=9;
  QString f=QFileInfo(Carte).absolutePath()+"/verification.cvw";
  QFile Fichier(f);
  QVERIFY(Fichier.open(QIODevice::WriteOnly|QIODevice::Truncate));
  QCOMPARE(Fichier.write(Octets.data(), (qint64)Octets.size()), (qint64)Octets.size());
  Fichier.close();
  QVERIFY(!VerifierApresOuverture);
  frmSimulation->ActionVerifierApresOuverture->Execute();
  QVERIFY(VerifierApresOuverture);
  Messages.clear();
  OuvreFichier(f);
  frmSimulation->ActionVerifierApresOuverture->Execute();
  QVERIFY(!VerifierApresOuverture);
  QCOMPARE(Messages.size(), 1);
  QVERIFY2(Messages.first().contains(QString::fromUtf8("Feu n°%1: type de demande du vert incorrect (9)").arg(NumFeu)), qPrintable(Messages.first()));
  QCOMPARE(Q(frmSimulation->StatusBar->Panels->Items[0]->Text), QString::fromUtf8("Chargement effectué MAIS erreur(s) d'intégrité trouvée(s)"));
 }
//---------------------------------------------------------------------------
void TestInterface::ouvertureImpossible()
 { // Jusqu'à la 6.0.1-rc1 (comme l'original), FileOpen en échec (-1) passait pour
   // une réussite : carte vide de 0 case ou 99×63, titre au nom du fichier
  QString f=OuvreCarte("Démo.cvw", "ouverture.cvw");
  QVERIFY(!f.isEmpty());
  centre_ville *Reseau=cv;
  AnsiString asTitre=frmSimulation->Caption, asOuvrir=frmSimulation->OpenDialog->FileName, asEnregistrer=frmSimulation->SaveDialog->FileName;
  auto VerifieReseauGarde=[&]()
   {
    QVERIFY(cv==Reseau);
    QCOMPARE(Messages.size(), 1);
    QVERIFY2(Messages.first().contains("Impossible d'ouvrir le fichier"), qPrintable(Messages.first()));
    QCOMPARE(Q(frmSimulation->StatusBar->Panels->Items[0]->Text), QString::fromUtf8("Chargement NON effectué."));
    QCOMPARE(Q(frmSimulation->Caption), Q(asTitre));
    QCOMPARE(Q(frmSimulation->OpenDialog->FileName), Q(asOuvrir));
    QCOMPARE(Q(frmSimulation->SaveDialog->FileName), Q(asEnregistrer));
    QVERIFY(!frmSimulation->SimulationModifiee);
   };
  Messages.clear();
  OuvreFichier(QFileInfo(f).absolutePath()+"/inexistant.cvw");
  VerifieReseauGarde();
  if (QTest::currentTestFailed()) return;
#ifndef _WIN32
  // Fichier sans droit de lecture
  QString Protege=QFileInfo(f).absolutePath()+"/protege.cvw";
  QFile::remove(Protege);
  QVERIFY(QFile::copy(f, Protege));
   {
    LectureSeule Droits(Protege);
    QVERIFY(QFile::setPermissions(Protege, QFile::Permissions()));
    if (!QFile(Protege).open(QIODevice::ReadOnly))
     {
      Messages.clear();
      OuvreFichier(Protege);
      VerifieReseauGarde();
      if (QTest::currentTestFailed()) return;
     }
   }
#endif
  // Le réseau gardé reste utilisable
  QVERIFY2(DemarreSansBlocage("Marche après une ouverture impossible"), qPrintable(Messages.join('\n')));
  QVERIFY(Arrete());
 }
//---------------------------------------------------------------------------
void TestInterface::marcheLectureSeuleNonModifie()
 { // Carte d'exemple ouverte depuis le .dmg : jusqu'à la 6.0.1-rc1, « Erreur de
   // données dans le fichier » (FileCreate en échec), simulation non démarrée, et le
   // clic suivant tournait sans fin. Le fichier est à jour : la simulation démarre
   // Référence : même carte dans un fichier inscriptible, mêmes tirages
  QVERIFY(!OuvreCarte("Géant.cvw", "inscriptible.cvw").isEmpty());
  QVERIFY2(DemarreSansBlocage("Marche, fichier inscriptible"), qPrintable(Messages.join('\n')));
  QVERIFY(JoueJusquAuTour(50));
  QString Reference=EmpreinteSimulation();
  QVERIFY(Arrete());

  QString f=OuvreCarte("Géant.cvw", "lecture_seule.cvw");
  QVERIFY(!f.isEmpty());
  QByteArray Origine=Contenu(f), Reseau=OctetsReseau();
  LectureSeule Droits(f);
  if (!Droits.Effective()) QSKIP("Droits de fichier sans effet (root)");
  for(int Essai=0; Essai<2; Essai++)
   {
    QVERIFY2(DemarreSansBlocage("Marche, fichier en lecture seule"), qPrintable(Messages.join('\n')));
    QVERIFY2(Messages.isEmpty(), qPrintable(Messages.join('\n')));
    QVERIFY(!frmSimulation->SimulationModifiee);
    QVERIFY(frmSimulation->ToolButtonArret->isEnabled());
    QVERIFY(frmSimulation->MenuItemArret->ElementQt()->isEnabled());
    QVERIFY(JoueJusquAuTour(50));
    QCOMPARE(EmpreinteSimulation(), Reference);
    QVERIFY(Contenu(f)==Origine);
    // Arrêt : bilan, puis le réseau d'origine rechargé, sans autre message
    QVERIFY(Arrete());
    QVERIFY(QTest::qWaitFor([this]() { return !Messages.isEmpty(); }, 5000));
    QCOMPARE(Messages.size(), 1);
    QVERIFY2(Messages.first().contains(QString::fromUtf8("Bilan au tour n°")), qPrintable(Messages.first()));
    QCOMPARE(Q(frmSimulation->StatusBar->Panels->Items[0]->Text), QString::fromUtf8("Chargement effectué."));
    QCOMPARE(Q(frmSimulation->Caption), QString("Centre-Ville - ")+QDir::toNativeSeparators(f));
    QVERIFY(!frmSimulation->SimulationModifiee);
    QVERIFY(!cv->DistancesCalculees);
    QVERIFY(OctetsReseau()==Reseau);
    QVERIFY(Contenu(f)==Origine);
   }
 }
//---------------------------------------------------------------------------
void TestInterface::marcheLectureSeuleModifie()
 { // Le fichier n'a pas les modifications : question, « Enregistrer sous », sinon pas de démarrage
  QString f=OuvreCarte("Géant.cvw", "lecture_seule_modifie.cvw");
  QVERIFY(!f.isEmpty());
  QString Dossier=QFileInfo(f).absolutePath(), Copie=Dossier+"/lecture_seule_copie.cvw", Refuse=Dossier+"/lecture_seule_refuse.cvw";
  QFile::remove(Copie);
  QFile::remove(Refuse);
  QVERIFY(QFile::copy(f, Refuse));
  QByteArray Origine=Contenu(f);
  LectureSeule Droits(f), DroitsRefuse(Refuse);
  if (!Droits.Effective()) QSKIP("Droits de fichier sans effet (root)");
  cv->AttenteMaxVeh=77; // modification de l'utilisateur
  frmSimulation->SimulationModifiee=true;
  QByteArray Reseau=OctetsReseau();
  AnsiString asNom=frmSimulation->SaveDialog->FileName;
  auto VerifieNonDemarreModifie=[&]()
   {
    VerifieNonDemarre(Reseau);
    if (QTest::currentTestFailed()) return;
    QCOMPARE(cv->AttenteMaxVeh, 77L);
    QVERIFY(frmSimulation->SimulationModifiee);
    QVERIFY(Contenu(f)==Origine);
    QCOMPARE(Q(frmSimulation->SaveDialog->FileName), Q(asNom));
   };
  // Réponse « Non », plusieurs fois
  for(int Essai=0; Essai<3; Essai++)
   {
    QVERIFY(!DemarreSansBlocage("Marche, fichier en lecture seule modifié, réponse Non"));
    QCOMPARE(Messages.size(), 1);
    QVERIFY2(Messages.first().contains("lecture seule"), qPrintable(Messages.first()));
    VerifieNonDemarreModifie();
    if (QTest::currentTestFailed()) return;
   }
  // « Oui », boîte annulée
  ReponseQuestions=QMessageBox::Yes;
  QuandModale(BoiteFichier, AnnuleBoiteFichier);
  QVERIFY(!DemarreSansBlocage("Marche, fichier en lecture seule modifié, boîte annulée"));
  QVERIFY(Etapes.empty());
  VerifieNonDemarreModifie();
  if (QTest::currentTestFailed()) return;
  // « Oui », nom d'un autre fichier en lecture seule : refusé, comme tout échec
  QuandModale(BoiteFichier, []() { ChoisitFichier(QFileInfo(frmSimulation->OpenDialog->FileName.c_str()).absolutePath()+"/lecture_seule_refuse.cvw"); });
  QVERIFY(!DemarreSansBlocage("Marche, fichier en lecture seule modifié, autre fichier refusé"));
  QVERIFY(Etapes.empty());
  QVERIFY2(Messages.last().contains("Impossible de créer le fichier"), qPrintable(Messages.join('\n')));
  VerifieNonDemarreModifie();
  if (QTest::currentTestFailed()) return;
  // « Oui », autre nom : le réseau y est enregistré, la simulation démarre, et Arrêt recharge ce fichier
  QuandModale(BoiteFichier, [Copie]() { ChoisitFichier(Copie); });
  QVERIFY2(DemarreSansBlocage("Marche, fichier en lecture seule modifié, enregistré sous"), qPrintable(Messages.join('\n')));
  QVERIFY(Etapes.empty());
  QVERIFY(QFile::exists(Copie));
  QCOMPARE(Q(frmSimulation->SaveDialog->FileName), QDir::toNativeSeparators(Copie));
  QVERIFY(!frmSimulation->SimulationModifiee);
  QVERIFY(JoueJusquAuTour(20));
  ReponseQuestions=QMessageBox::No;
  QVERIFY(Arrete());
  QCOMPARE(Q(frmSimulation->Caption), QString("Centre-Ville - ")+QDir::toNativeSeparators(Copie));
  QCOMPARE(Q(frmSimulation->StatusBar->Panels->Items[0]->Text), QString::fromUtf8("Chargement effectué."));
  QCOMPARE(cv->AttenteMaxVeh, 77L);
  QVERIFY(Contenu(f)==Origine);
 }
//---------------------------------------------------------------------------
void TestInterface::interruptionErreursIntegrite()
 { // « Vérifier intégrité avant simulation » : la vérification porte sur le réseau
   // préparé (véhicules, bus...), elle ne peut pas venir avant les Prepare*
  QString f=QFileInfo(Carte).absolutePath()+"/integrite.cvw";
  conversion_cvw Reference=ConvertitEn530(Contenu(QFileInfo(ArgumentsTest.value(2)).absolutePath()+"/Géant.cvw").toStdString());
  QVERIFY(Reference.Ok);
  auto Debut=[&Reference](const std::string &Nom) -> long long
   {
    for(const champ_cvw &ch : Reference.Champs) if (ch.Nom==Nom) return (long long)ch.Debut;
    return -1;
   };
  int NumFeu=1;
  for(long long d; ((d=Debut("feu"+std::to_string(NumFeu)+".x"))>=0)&&(Reference.Octets[(std::size_t)d+3]&0x80); NumFeu++);
  const long long DmdVert=Debut("feu"+std::to_string(NumFeu)+".TypeDmdVert");
  QVERIFY(DmdVert>=0);
  std::string Octets=Reference.Octets;
  const char Bon=Octets[(std::size_t)DmdVert];
  Octets[(std::size_t)DmdVert]=9;
  QFile::remove(f);
  QFile Fichier(f);
  QVERIFY(Fichier.open(QIODevice::WriteOnly));
  QCOMPARE(Fichier.write(Octets.data(), (qint64)Octets.size()), (qint64)Octets.size());
  Fichier.close();
  OuvreFichier(f);
  QVERIFY(!VerifierAvantSimulation);
  frmSimulation->ActionVerifierAvantSimulation->Execute();
  QVERIFY(VerifierAvantSimulation);
  Finalement Decoche([]() { if (VerifierAvantSimulation) frmSimulation->ActionVerifierAvantSimulation->Execute(); });
  QByteArray Reseau=OctetsReseau();
  QByteArray Disque=Contenu(f);
  for(int Essai=0; Essai<3; Essai++)
   {
    QVERIFY(!DemarreSansBlocage("Marche après des erreurs d'intégrité"));
    QVERIFY2(Messages.size()==1, qPrintable(Messages.join('\n')));
    QVERIFY2(Messages.first().contains(QString::fromUtf8("type de demande du vert incorrect (9)")), qPrintable(Messages.first()));
    VerifieNonDemarre(Reseau);
    if (QTest::currentTestFailed()) return;
    QVERIFY(!frmSimulation->SimulationModifiee);
    QVERIFY(Contenu(f)==Disque); // rien d'enregistré
   }
  // Ouvrir ne doit pas prendre ces erreurs pour celles du fichier ouvert
  frmSimulation->ActionVerifierAvantSimulation->Execute();
  QVERIFY(!OuvreCarte("Démo.cvw", "integrite_demo.cvw").isEmpty());
  QCOMPARE(Q(frmSimulation->StatusBar->Panels->Items[0]->Text), QString::fromUtf8("Chargement effectué."));
  // Le réseau corrigé démarre, sur le réseau rétabli
  OuvreFichier(f);
  cv->Feu[NumFeu-1].TypeDmdVert=(dmd_vert)Bon;
  frmSimulation->SimulationModifiee=true;
  QVERIFY2(DemarreSansBlocage("Marche, réseau corrigé"), qPrintable(Messages.join('\n')));
  QVERIFY(JoueJusquAuTour(5));
  QVERIFY(Arrete());
  QVERIFY(Contenu(f)==QByteArray(Reference.Octets.data(), (qsizetype)Reference.Octets.size()));
 }
//---------------------------------------------------------------------------
void TestInterface::interruptionTramsImpossibles()
 { // Plus de trams à placer que d'arrêts sur leur ligne : tous les Prepare* jusqu'à
   // PrepareTrams ont tourné, et un seul message (pas de « mémoire insuffisante »)
  QString f=OuvreCarte("Géant.cvw", "trams.cvw");
  QVERIFY(!f.isEmpty());
  int Ligne=0;
  while((Ligne<NBMAXLIGNESBUSTRAM)&&(cv->LigneTram[Ligne].NbArrets<2)) Ligne++;
  QVERIFY(Ligne<NBMAXLIGNESBUSTRAM);
  const int Arret1=cv->LigneTram[Ligne].NumArret[0], Arret2=cv->LigneTram[Ligne].NumArret[1];
  // Trois lignes d'un tram chacune sur les deux mêmes arrêts
  struct ligne { int NbArrets; std::vector<int> NumArret; };
  std::vector<ligne> Lignes;
  for(int i=0; i<NBMAXLIGNESBUSTRAM; i++)
   Lignes.push_back({cv->LigneTram[i].NbArrets, std::vector<int>(cv->LigneTram[i].NumArret, cv->LigneTram[i].NumArret+NBMAXARRETSBUSTRAMPARLIGNE)});
  auto Retablit=[&Lignes]()
   {
    for(int i=0; i<NBMAXLIGNESBUSTRAM; i++)
     {
      cv->LigneTram[i].NbArrets=Lignes[i].NbArrets;
      for(int a=0; a<NBMAXARRETSBUSTRAMPARLIGNE; a++) cv->LigneTram[i].NumArret[a]=Lignes[i].NumArret[a];
     }
   };
  for(int i : {0, 1, 2})
   {
    cv->LigneTram[i].NbArrets=2;
    cv->LigneTram[i].NumArret[0]=Arret1;
    cv->LigneTram[i].NumArret[1]=Arret2;
   }
  for(int i=3; i<NBMAXLIGNESBUSTRAM; i++) cv->LigneTram[i].NbArrets=0;
  Enregistre(f);
  QCOMPARE(Q(frmSimulation->StatusBar->Panels->Items[0]->Text), QString::fromUtf8("Enregistrement effectué."));
  QByteArray Reseau=OctetsReseau();
  for(int Essai=0; Essai<3; Essai++)
   {
    QVERIFY(!DemarreSansBlocage("Marche après des trams impossibles à placer"));
    QVERIFY2(Messages.size()==1, qPrintable(Messages.join('\n')));
    QVERIFY2(Messages.first().contains("placer tous les trams"), qPrintable(Messages.first()));
    VerifieNonDemarre(Reseau);
    if (QTest::currentTestFailed()) return;
   }
  // L'utilisateur rétablit ses lignes : la simulation démarre, avec les bus et les trams de chaque ligne
  Retablit();
  frmSimulation->SimulationModifiee=true;
  QVERIFY2(DemarreSansBlocage("Marche, trams corrigés"), qPrintable(Messages.join('\n')));
  for(int i=0; i<NBMAXLIGNESBUSTRAM; i++)
   {
    QCOMPARE(cv->LigneBus[i].NbBus, cv->LigneBus[i].NbArrets?1+(cv->LigneBus[i].NbArrets-1)/3:0);
    QCOMPARE(cv->LigneTram[i].NbTrams, cv->LigneTram[i].NbArrets?1+(cv->LigneTram[i].NbArrets-1)/3:0);
   }
  QVERIFY(JoueJusquAuTour(5));
  QVERIFY(Arrete());
 }
//---------------------------------------------------------------------------
void TestInterface::interruptionReseauNonConnexe()
 { // Seule interruption qui garde son état : les fiches affichent les distances
   // calculées, pour trouver la case fautive. Puis l'utilisateur répare le réseau,
   // ce qui change le nombre de directions : les distances, allouées d'après l'ancien,
   // étaient libérées d'après le nouveau (hors du tableau, jusqu'à la 6.0.1-rc1)
  QString f=CreeReseauNonConnexe("non_connexe.cvw");
  QVERIFY(!f.isEmpty());
  QByteArray Reseau=OctetsReseau();
  for(int Essai=0; Essai<3; Essai++)
   {
    QVERIFY(!DemarreSansBlocage("Marche, réseau non connexe"));
    QVERIFY2(Messages.size()==1, qPrintable(Messages.join('\n')));
    QVERIFY2(Messages.first().contains(QString::fromUtf8("n'est pas connexe")), qPrintable(Messages.first()));
    QVERIFY(!frmSimulation->SimulationEnCours);
    QVERIFY(!frmSimulation->ActionArret->Enabled);
    QVERIFY(!frmSimulation->ToolButtonArret->isEnabled());
    QVERIFY(!frmSimulation->MenuItemArret->ElementQt()->isEnabled());
    QVERIFY(frmAffDistParkings->isVisible());
    QVERIFY(cv->v[4][2].DistParking);
    QVERIFY(OctetsReseau()==Reseau);
    QVERIFY(Contenu(f)==Reseau);
   }
  // Réparation : deux routes à sens unique relient les boucles, deux cases gagnent une direction
  PlaceCurseur(6, 3);
  frmSimulation->ActionDirectionDroite->Execute();
  QCOMPARE((int)cv->v[6][3].DirPoss, (int)(S|E));
  QVERIFY(frmSimulation->SimulationModifiee);
  QVERIFY(!cv->v[4][2].DistParking); // distances obsolètes libérées...
  frmSimulation->DrawGridSimulClick(frmSimulation->DrawGridSimul); // ... et la fiche encore ouverte le voit
  for(int x=7; x<=9; x++) cv->v[x][3].DirPoss=E;
  PlaceCurseur(10, 5);
  frmSimulation->ActionDirectionGauche->Execute();
  QCOMPARE((int)cv->v[10][5].DirPoss, (int)(N|O));
  for(int x=7; x<=9; x++) cv->v[x][5].DirPoss=O;
  QVERIFY2(DemarreSansBlocage("Marche, réseau réparé"), qPrintable(Messages.join('\n')));
  QVERIFY(frmSimulation->ToolButtonArret->isEnabled());
  QVERIFY(JoueJusquAuTour(5));
  QVERIFY(Arrete());
  QCOMPARE((int)cv->v[6][3].DirPoss, (int)(S|E));
  if (frmAffDistParkings->isVisible()) frmAffDistParkings->Hide();
 }
//---------------------------------------------------------------------------
void TestInterface::interruptionEnregistrementAnnule()
 { // Réseau sans nom : Marche demande où l'enregistrer (après les Prepare*, l'enregistrement les écrit)
  // (Géant sans nom de fichier, comme après Nouveau : un réseau généré n'a pas de parkings définis)
  QVERIFY(!OuvreCarte("Géant.cvw", "sans_nom_source.cvw").isEmpty());
  frmSimulation->SaveDialog->FileName="";
  frmSimulation->OpenDialog->FileName="";
  frmSimulation->Caption="Centre-Ville";
  QString f=QFileInfo(Carte).absolutePath()+"/sans_nom.cvw";
  QFile::remove(f);
  QByteArray Reseau=OctetsReseau();
  for(int Essai=0; Essai<3; Essai++)
   {
    QuandModale(BoiteFichier, AnnuleBoiteFichier);
    QVERIFY(!DemarreSansBlocage("Marche, enregistrement annulé"));
    QVERIFY(Etapes.empty());
    VerifieNonDemarre(Reseau);
    if (QTest::currentTestFailed()) return;
    QVERIFY(frmSimulation->SaveDialog->FileName.IsEmpty());
    QVERIFY2(Messages.isEmpty(), qPrintable(Messages.join('\n')));
   }
  // Ouvrir après une interruption : pas d'ouverture du fichier de Marche sans demander
  QuandModale(BoiteFichier, AnnuleBoiteFichier);
  frmSimulation->ActionOuvrir->Execute();
  QVERIFY(Etapes.empty());
  // Nom choisi : la simulation démarre et Arrêt recharge ce fichier, sans demander lequel
  QuandModale(BoiteFichier, [f]() { ChoisitFichier(f); });
  QVERIFY2(DemarreSansBlocage("Marche, enregistrement accepté"), qPrintable(Messages.join('\n')));
  QVERIFY(Etapes.empty());
  QVERIFY(JoueJusquAuTour(5));
  QVERIFY(Arrete());
  QVERIFY(!QApplication::activeModalWidget());
  QCOMPARE(Q(frmSimulation->StatusBar->Panels->Items[0]->Text), QString::fromUtf8("Chargement effectué."));
  QCOMPARE(Q(frmSimulation->Caption), QString("Centre-Ville - ")+QDir::toNativeSeparators(f));
  QVERIFY2(!Messages.join('\n').contains("sans réponse"), qPrintable(Messages.join('\n')));
 }
//---------------------------------------------------------------------------
void TestInterface::interruptionEnregistrementEnEchec()
 {
#ifdef _WIN32
  QSKIP("Limite de taille des fichiers : pas d'équivalent sous Windows");
#else
  QString f=OuvreCarte("Géant.cvw", "disque_plein.cvw");
  QVERIFY(!f.isEmpty());
  QByteArray Origine=Contenu(f), Reseau=OctetsReseau();
  // Copie du réseau impossible (dossier temporaire plein) : aucun démarrage, rien de préparé
  for(int Essai=0; Essai<2; Essai++)
   {
    LimiteFichiers Limite;
    QVERIFY(!DemarreSansBlocage("Marche, copie du réseau impossible"));
    QVERIFY2(Messages.size()==1, qPrintable(Messages.join('\n')));
    QVERIFY2(Messages.first().contains("dossier temporaire"), qPrintable(Messages.first()));
   }
  VerifieNonDemarre(Reseau);
  if (QTest::currentTestFailed()) return;
  QVERIFY(Contenu(f)==Origine);
  // Disque plein pendant l'enregistrement, le fichier est tronqué : message, pas de démarrage
  for(int Essai=0; Essai<2; Essai++)
   {
    LimiteFichiers Limite(QString::fromUtf8("Préparation des feux de piétons..."));
    QVERIFY(!DemarreSansBlocage("Marche, écriture en échec"));
    QVERIFY2(Messages.size()==1, qPrintable(Messages.join('\n')));
    QVERIFY2(Messages.first().contains(QString::fromUtf8("Erreur de données dans le fichier")), qPrintable(Messages.first()));
    frmSimulation->StatusBar->removeEventFilter(&Limite);
   }
  VerifieNonDemarre(Reseau);
  if (QTest::currentTestFailed()) return;
  QVERIFY(frmSimulation->SimulationModifiee); // Le réseau n'est plus que dans la mémoire...
  QVERIFY(Contenu(f)!=Origine);
  // ... et l'espace revenu, Marche l'enregistre puis démarre
  QVERIFY2(DemarreSansBlocage("Marche, écriture de nouveau possible"), qPrintable(Messages.join('\n')));
  QVERIFY(!frmSimulation->SimulationModifiee);
  QVERIFY(JoueJusquAuTour(5));
  QVERIFY(Arrete());
  QVERIFY(OctetsReseau()==Reseau);
#endif
 }
//---------------------------------------------------------------------------
void TestInterface::arretFichierInaccessible()
 { // Volume du .dmg éjecté pendant la simulation : Arrêt ne peut plus recharger le
   // fichier ; le réseau préparé bloquerait le clic suivant, il est rétabli
  QString f=OuvreCarte("Géant.cvw", "ejecte.cvw");
  QVERIFY(!f.isEmpty());
  QByteArray Reseau=OctetsReseau();
  QVERIFY2(DemarreSansBlocage("Marche, fichier bientôt inaccessible"), qPrintable(Messages.join('\n')));
  QVERIFY(JoueJusquAuTour(10));
  QVERIFY(QFile::remove(f));
  QVERIFY(Arrete());
  QVERIFY(QTest::qWaitFor([this]() { return Messages.size()>=2; }, 5000));
  QVERIFY2(Messages.last().contains("Impossible d'ouvrir le fichier"), qPrintable(Messages.join('\n')));
  VerifieNonDemarre(Reseau);
  if (QTest::currentTestFailed()) return;
  QVERIFY(frmSimulation->SimulationModifiee);
  QCOMPARE(Q(frmSimulation->Caption), QString("Centre-Ville - ")+QDir::toNativeSeparators(f)+"*");
  // Dossier toujours là : Marche réécrit le fichier
  QVERIFY2(DemarreSansBlocage("Marche, après l'arrêt sans fichier"), qPrintable(Messages.join('\n')));
  QVERIFY(QFile::exists(f));
  QVERIFY(JoueJusquAuTour(5));
  QVERIFY(Arrete());
 }
//---------------------------------------------------------------------------
void TestInterface::carteDUnVolumeEnLectureSeule()
 { // Recette (docs/RECETTE.md) : carte du dossier Cartes du .dmg monté, désignée par
   // CV_CARTE_LECTURE_SEULE, sur un vrai système de fichiers en lecture seule et
   // de préférence dans l'affichage natif. En temps réel, comme marcheInterruptionArret
  QString f=qEnvironmentVariable("CV_CARTE_LECTURE_SEULE");
  if (f.isEmpty()) QSKIP("CV_CARTE_LECTURE_SEULE non définie");
  QVERIFY2(!QFile(f).open(QIODevice::ReadWrite), "La carte n'est pas en lecture seule");
  OuvreFichier(f);
  QCOMPARE(Q(frmSimulation->StatusBar->Panels->Items[0]->Text), QString::fromUtf8("Chargement effectué."));
  QByteArray Origine=Contenu(f), Reseau=OctetsReseau();
  for(int Essai=0; Essai<2; Essai++)
   {
    Messages.clear();
    {
     LimiteDeTemps Limite("Marche, carte d'un volume en lecture seule", 120);
     QTest::mouseClick(frmSimulation->ToolButtonMarche, Qt::LeftButton);
    }
    QVERIFY2(frmSimulation->SimulationEnCours, qPrintable(Messages.join('\n')));
    QVERIFY2(Messages.isEmpty(), qPrintable(Messages.join('\n')));
    QVERIFY(AttendTour(100, 120000));
    Capture(Essai?"volume_lecture_seule_2.png":"volume_lecture_seule_1.png");
    QVERIFY(Arrete());
    QVERIFY(QTest::qWaitFor([this]() { return !Messages.isEmpty(); }, 5000));
    QVERIFY2(Messages.first().contains(QString::fromUtf8("Bilan au tour n°")), qPrintable(Messages.first()));
    QCOMPARE(Q(frmSimulation->StatusBar->Panels->Items[0]->Text), QString::fromUtf8("Chargement effectué."));
    QVERIFY(OctetsReseau()==Reseau);
    QVERIFY(Contenu(f)==Origine);
   }
 }
//---------------------------------------------------------------------------
void TestInterface::nouveauEtGeneration()
 { // f_simul : ActionNouveauExecute -> f_dimzone -> f_genres (ButtonOkClick), graine 12345
  QVERIFY(!OuvreDemo("avant_generation.cvw").isEmpty()); // Dimensions proposées : celles du réseau ouvert
  alea::FixeGraine(12345);
  Messages.clear();
  QTest::mouseClick(frmSimulation->ToolButtonNouveau, Qt::LeftButton);
  alea::LibereGraine();
  QVERIFY2(Messages.isEmpty(), qPrintable(Messages.join('\n')));
  QCOMPARE(Q(frmSimulation->Caption), QString("Centre-Ville"));
  QCOMPARE(cv->NbX, NBXDEF);
  QCOMPARE(cv->NbY, NBYDEF);
  QVERIFY(!frmSimulation->SimulationModifiee);
  QVERIFY(!frmDimensionsZone);
  QVERIFY(!frmGeneration);
  // Réseau généré = golden master « defaut_12345 » (tests/golden/generation)
  std::ifstream f(std::string(CV_DOSSIER_GOLDEN)+"/generation/defaut_12345.txt");
  QVERIFY(f.good());
  std::string Ligne, Attendu, Obtenu;
  bool Carte=false;
  while(std::getline(f, Ligne))
   {
    if (Ligne.rfind("DirPoss", 0)==0) { Carte=true; continue; }
    if (Carte&&(Ligne.rfind("etat=", 0)==0)) break;
    if (Carte) Attendu+=Ligne+"\n";
   }
  char Tampon[4];
  for(int j=0; j<cv->NbY; j++)
   {
    for(int i=0; i<cv->NbX; i++)
     {
      std::snprintf(Tampon, sizeof(Tampon), "%x", (int)cv->v[i][j].DirPoss);
      Obtenu+=Tampon;
     }
    Obtenu+="\n";
   }
  QVERIFY(!Attendu.empty());
  QVERIFY(Obtenu==Attendu);
  frmSimulation->DrawGridSimul->Repaint();
  Capture("generation_12345.png");
 }
//---------------------------------------------------------------------------
// Lot 1 des fiches secondaires
//---------------------------------------------------------------------------
void TestInterface::ficheAPropos()
 { // Aide > À propos : fiche modale ; feu vert, orange puis rouge (minuteur
   // accéléré) ; Échap = clic sur le feu = boîte « Licence »
  QVERIFY(!OuvreDemo("lot1_apropos.cvw").isEmpty());
  frmAPropos->Timer->Interval=20;
  bool Vert=false, Jaune=false, Rouge=false, GlypheRouge=false, Modale=false;
  Messages.clear();
  QuandModale([]() { return frmAPropos; }, [&]()
   {
    Modale=frmAPropos->isModal()&&!frmAPropos->FermetureAutomatique;
    Vert=QTest::qWaitFor([]() { return frmAPropos->LabelTexte->Font->Color==clLime; }, 2000);
    Jaune=QTest::qWaitFor([]() { return frmAPropos->LabelTexte->Font->Color==clYellow; }, 2000);
    Rouge=QTest::qWaitFor([]() { return frmAPropos->LabelTexte->Font->Color==clRed; }, 2000);
    GlypheRouge=frmAPropos->SpeedButtonOk->Glyph.Ref().Image.toImage()==frmAPropos->VirtualImageList->Pixmap(2).toImage();
    frmAPropos->Timer->Enabled=false; // capture au feu rouge
    CaptureFiche(frmAPropos, "fiche_propos.png");
    frmAPropos->Timer->Enabled=true;
    frmAPropos->activateWindow();
    QTest::keyClick(frmAPropos, Qt::Key_Escape); // aucun contrôle actif : FormKeyPress -> SpeedButtonOk->Click() -> LabelClick
    frmAPropos->Close();
   });
  frmSimulation->MenuItemAPropos->ElementQt()->trigger();
  frmAPropos->Timer->Interval=1000;
  QVERIFY(Etapes.empty());
  QVERIFY(Modale);
  QVERIFY(Vert&&Jaune&&Rouge);
  QVERIFY(GlypheRouge);
  QVERIFY(!frmAPropos->isVisible());
  QVERIFY(!frmAPropos->Timer->Enabled);
  QCOMPARE(Messages.size(), 1); // (titre « Licence » absent sous macOS)
  QVERIFY2(Messages.first().contains(QString::fromUtf8(" : Centre-Ville est sous Licence Publique Générale")), qPrintable(Messages.first()));
 }
//---------------------------------------------------------------------------
void TestInterface::ficheEpidemie()
 { // Menu Simulation > Épidémie (AutoCheck) : la fiche modale s'ouvre quand l'action se coche
  QVERIFY(!OuvreDemo("lot1_epidemie.cvw").isEmpty());
  QVERIFY(!frmSimulation->ActionEpidemie->Checked);
  int Infectiosite=cv->EpidemieInfectiosite, PatientZero=cv->EpidemieIterationPatientZero,
      Charge=cv->EpidemieChargeFatale;
  bool Reinfections=cv->EpidemieReinfections, Affichage=false;
  QuandModale([]() { return frmEpidemie; }, [&]()
   {
    Affichage=frmEpidemie->CheckBoxActiver->Checked&&frmEpidemie->GroupBox->Enabled&&
              (frmEpidemie->CSpinEditInfectiosite->Value==Infectiosite)&&
              (frmEpidemie->CSpinEditChargeViraleFatale->Value==Charge);
    frmEpidemie->CSpinEditInfectiosite->Value=Infectiosite+11;
    frmEpidemie->CSpinEditPatientZero->Value=PatientZero+7;
    frmEpidemie->CSpinEditChargeViraleFatale->Value=Charge+3;
    Clique(frmEpidemie->CheckBoxReinfections);
    CaptureFiche(frmEpidemie, "fiche_epidemie.png");
    Clique(frmEpidemie->ButtonOk);
   });
  frmSimulation->Epidemie1->ElementQt()->trigger();
  QVERIFY(Etapes.empty());
  QVERIFY(Affichage);
  QVERIFY(frmSimulation->ActionEpidemie->Checked);
  QVERIFY(!frmEpidemie->isVisible());
  QCOMPARE(cv->EpidemieInfectiosite, Infectiosite+11);
  QCOMPARE(cv->EpidemieIterationPatientZero, PatientZero+7);
  QCOMPARE(cv->EpidemieChargeFatale, Charge+3);
  QCOMPARE(cv->EpidemieReinfections, !Reinfections);
  // Décocher n'ouvre pas la fiche ; recocher puis Annuler (ou Échap) : rien ne change
  frmSimulation->Epidemie1->ElementQt()->trigger();
  QVERIFY(!frmSimulation->ActionEpidemie->Checked);
  QVERIFY(!frmEpidemie->isVisible());
  bool Relu=false;
  QuandModale([]() { return frmEpidemie; }, [&]()
   {
    Relu=(frmEpidemie->CSpinEditInfectiosite->Value==Infectiosite+11)&&(frmEpidemie->CheckBoxReinfections->Checked==!Reinfections);
    frmEpidemie->CSpinEditInfectiosite->Value=Infectiosite+50;
    Clique(frmEpidemie->ButtonAnnuler);
   });
  frmSimulation->Epidemie1->ElementQt()->trigger();
  QVERIFY(Etapes.empty());
  QVERIFY(Relu);
  QCOMPARE(cv->EpidemieInfectiosite, Infectiosite+11);
  frmSimulation->Epidemie1->ElementQt()->trigger();
  QuandModale([]() { return frmEpidemie; }, [&]()
   {
    frmEpidemie->CSpinEditPatientZero->Value=PatientZero+50;
    Touche(frmEpidemie->CSpinEditPatientZero, Qt::Key_Escape);
   });
  frmSimulation->Epidemie1->ElementQt()->trigger();
  QVERIFY(Etapes.empty());
  QCOMPARE(cv->EpidemieIterationPatientZero, PatientZero+7);
  frmSimulation->Epidemie1->ElementQt()->trigger(); // épidémie désactivée pour la suite
  QVERIFY(!frmSimulation->ActionEpidemie->Checked);
 }
//---------------------------------------------------------------------------
void TestInterface::ficheNature()
 { // Sélection de 4 x 2 cases libres, bouton « Nature » : fiche non modale
  QVERIFY(!OuvreDemo("lot1_nature.cvw").isEmpty());
  int xz, yz;
  QVERIFY(ZoneLibre(4, 2, xz, yz));
  Focalise();
  Selectionne(xz, yz, xz+3, yz+1);
  QVERIFY(frmSimulation->ActionDefinirNature->Enabled);
  Clique(frmSimulation->ToolButtonDefinirNature);
  QVERIFY(frmDefNature->isVisible());
  QVERIFY(frmSimulation->ActionDefinirNature->Checked);
  QVERIFY(!QApplication::activeModalWidget());
  QCOMPARE((int)frmDefNature->RadioGroupNature->ItemIndex, -1);
  QVERIFY(!frmDefNature->ButtonAppliquer->Enabled);
  QVERIFY(!frmDefNature->CheckBoxPelouse->Enabled);
  CliqueChoix(frmDefNature->RadioGroupNature, 1); // Trottoirs
  QVERIFY(frmDefNature->ButtonAppliquer->Enabled);
  QVERIFY(frmDefNature->CheckBoxPelouse->Enabled);
  CaptureFiche(frmDefNature, "fiche_nature.png");
  Clique(frmDefNature->ButtonAppliquer);
  for(int i=0; i<4; i++)
   for(int j=0; j<2; j++)
    QCOMPARE((int)NATURE(&cv->v[xz+i][yz+j]), (int)trottoirs);
  QVERIFY(frmSimulation->SimulationModifiee);
  QVERIFY(frmDefNature->isVisible()); // Appliquer laisse la fiche ouverte
  // Pelouse puis Fermer (bouton Cancel) : rien ne change, fiche cachée
  CliqueChoix(frmDefNature->RadioGroupNature, 2); // Immeubles : la pelouse est décochée et grisée
  QVERIFY(!frmDefNature->CheckBoxPelouse->Enabled);
  Clique(frmDefNature->ButtonFermer);
  QVERIFY(!frmDefNature->isVisible());
  QVERIFY(!frmSimulation->ActionDefinirNature->Checked);
  QCOMPARE((int)NATURE(&cv->v[xz][yz]), (int)trottoirs);
  // Réouverture par le menu : fiche réinitialisée (FormShow)
  frmSimulation->MenuItemDefinirNature->ElementQt()->trigger();
  QVERIFY(frmDefNature->isVisible());
  QCOMPARE((int)frmDefNature->RadioGroupNature->ItemIndex, -1);
  CliqueChoix(frmDefNature->RadioGroupNature, 2);
  Touche(frmDefNature->ButtonAppliquer, Qt::Key_Escape); // Échap = Fermer
  QVERIFY(!frmDefNature->isVisible());
  QCOMPARE((int)NATURE(&cv->v[xz+1][yz+1]), (int)trottoirs);
 }
//---------------------------------------------------------------------------
void TestInterface::fichesPrioritesEtFeux()
 { // Case de route : Priorités (non modale), Stop, puis Feu et fiche Feux (non modale)
  QVERIFY(!OuvreDemo("lot1_priorites.cvw").isEmpty());
  int x=-1, y=-1, d=0;
  for(int i=0; (x<0)&&(i<cv->NbX); i++)
   for(int j=0; (x<0)&&(j<cv->NbY); j++)
    {
     voie &v=cv->v[i][j];
     int n=((v.DirPoss&N)==N)+((v.DirPoss&E)==E)+((v.DirPoss&S)==S)+((v.DirPoss&O)==O);
     if ((n>=2)&&!v.SensVoieTram&&!v.NumParking&&!v.EstDepotBus()&&
         !v.Priorite[0]&&!v.Priorite[1]&&!v.Priorite[2]&&!v.Priorite[3])
      { x=i; y=j; }
    }
  QVERIFY(x>=0);
  voie *v=&cv->v[x][y];
  for(int k=nord; (!d)&&(k<=ouest); k++)
   if ((v->DirPoss&PossDir[k])==PossDir[k]) d=k;
  TRadioGroup *Groupes[NBDIR]={frmDefVoie->RadioGroupPrioriteNord, frmDefVoie->RadioGroupPrioriteEst,
                               frmDefVoie->RadioGroupPrioriteSud, frmDefVoie->RadioGroupPrioriteOuest};
  TBitBtn *BoutonsFeu[NBDIR]={frmDefVoie->BitBtnDefFeuN, frmDefVoie->BitBtnDefFeuE,
                              frmDefVoie->BitBtnDefFeuS, frmDefVoie->BitBtnDefFeuO};
  TRadioGroup *Groupe=Groupes[d-1];
  Focalise();
  PlaceCurseur(x, y);
  QVERIFY(frmSimulation->ActionPriorites->Enabled);
  Clique(frmSimulation->ToolButtonPriorites);
  QVERIFY(frmDefVoie->isVisible());
  QVERIFY(frmSimulation->ActionPriorites->Checked);
  QCOMPARE(Q(frmDefVoie->LabelCoordonnees->Caption), QString::asprintf("%02d, %02d", x, y));
  QVERIFY(Groupe->Enabled);
  QCOMPARE((int)Groupe->ItemIndex, (int)a_droite);
  QVERIFY(!frmDefVoie->ButtonAppliquer->Enabled);
  // Stop, OK
  CliqueChoix(Groupe, stop);
  QVERIFY(frmDefVoie->ButtonAppliquer->Enabled);
  CaptureFiche(frmDefVoie, "fiche_priorites.png");
  Clique(frmDefVoie->ButtonOk);
  QVERIFY(!frmDefVoie->isVisible());
  QVERIFY(!frmSimulation->ActionPriorites->Checked);
  QCOMPARE((int)v->Priorite[d-1], (int)stop);
  // Cédez-le-passage, Annuler : rien ne change
  Clique(frmSimulation->ToolButtonPriorites);
  QCOMPARE((int)Groupe->ItemIndex, (int)stop);
  CliqueChoix(Groupe, cedez_le_passage);
  Clique(frmDefVoie->ButtonAnnuler);
  QVERIFY(!frmDefVoie->isVisible());
  QCOMPARE((int)v->Priorite[d-1], (int)stop);
  // Feu, Appliquer : le feu est créé et son bouton de définition s'active
  frmSimulation->MenuItemPriorites->ElementQt()->trigger();
  QVERIFY(frmDefVoie->isVisible());
  QVERIFY(!BoutonsFeu[d-1]->Enabled);
  int NbFeux=0;
  for(int i=0; i<cv->NbFeux; i++) NbFeux+=cv->Feu[i].Defini();
  CliqueChoix(Groupe, feux);
  Clique(frmDefVoie->ButtonAppliquer);
  QCOMPARE((int)v->Priorite[d-1], (int)feux);
  int n=v->NumFeu[d-1];
  QVERIFY(n>0);
  int NbFeuxApres=0;
  for(int i=0; i<cv->NbFeux; i++) NbFeuxApres+=cv->Feu[i].Defini();
  QCOMPARE(NbFeuxApres, NbFeux+1);
  QVERIFY(BoutonsFeu[d-1]->Enabled);
  QVERIFY(!BoutonsFeu[d-1]->icon().isNull());
  // Fiche Feux : durée du vert et état, OK
  Clique(BoutonsFeu[d-1]);
  QVERIFY(frmDefFeux->isVisible());
  QVERIFY(frmDefVoie->isVisible());
  QCOMPARE((int)frmDefFeux->Direction, d);
  QCOMPARE(Q(frmDefFeux->LabelNumero->Caption), QString::number(n));
  QCOMPARE(Q(frmDefFeux->LabelFeuxPietons->Caption), QString("Aucun"));
  frmDefFeux->CSpinEditDureeVert->Value=7;
  CliqueChoix(frmDefFeux->RadioGroupEtat, rouge);
  QVERIFY(frmDefFeux->ButtonAppliquer->Enabled);
  CaptureFiche(frmDefFeux, "fiche_feux.png");
  Clique(frmDefFeux->ButtonOk);
  QVERIFY(!frmDefFeux->isVisible());
  QCOMPARE(cv->Feu[n-1].DureeVert, 7);
  QCOMPARE((int)cv->Feu[n-1].Etat, (int)rouge);
  // Feux, Annuler : rien ne change
  Clique(BoutonsFeu[d-1]);
  QVERIFY(frmDefFeux->isVisible());
  QCOMPARE((int)frmDefFeux->CSpinEditDureeVert->Value, 7);
  frmDefFeux->CSpinEditDureeVert->Value=9;
  CliqueChoix(frmDefFeux->RadioGroupEtat, vert);
  Clique(frmDefFeux->ButtonAnnuler);
  QVERIFY(!frmDefFeux->isVisible());
  QCOMPARE(cv->Feu[n-1].DureeVert, 7);
  QCOMPARE((int)cv->Feu[n-1].Etat, (int)rouge);
  Clique(frmDefVoie->ButtonOk);
  QVERIFY(!frmDefVoie->isVisible());
  QCOMPARE((int)v->Priorite[d-1], (int)feux);
 }
//---------------------------------------------------------------------------
void TestInterface::fichesPrioritesEtFeuxPietons()
 { // Route est-ouest (k voies) entre deux trottoirs, passage piétons :
   // Priorités piétons (non modale) et fiche Feux de piétons (non modale)
  QVERIFY(!OuvreDemo("lot1_pietons.cvw").isEmpty());
  int x=-1, y=-1, k=0;
  for(int i=1; (x<0)&&(i<cv->NbX-1); i++)
   for(int j=1; (x<0)&&(j<cv->NbY-4); j++)
    for(int l=1; (x<0)&&(l<=3); l++)
     {
      bool Ok=CaseLibre(i, j-1)&&CaseLibre(i, j+l);
      for(int m=0; Ok&&(m<l); m++)
       {
        voie &v=cv->v[i][j+m];
        Ok=v.DirPoss&&!(v.DirPoss&(N|S))&&!v.SensVoieTram&&!v.NumParking&&!v.EstDepotBus()&&!v.NumArretBus&&
           !v.Priorite[0]&&!v.Priorite[1]&&!v.Priorite[2]&&!v.Priorite[3];
       }
      if (Ok) { x=i; y=j; k=l; }
     }
  QVERIFY(x>=0);
  int yn=y-1, ys=y+k; // trottoirs nord et sud
  Focalise();
  // Trottoirs (Nature > Trottoir), passage piétons sur chaque voie
  for(int j : {yn, ys})
   {
    PlaceCurseur(x, j);
    QVERIFY(frmSimulation->ActionNatureTrottoir->Enabled);
    frmSimulation->ActionNatureTrottoir->Execute();
    QCOMPARE((int)NATURE(&cv->v[x][j]), (int)trottoirs);
   }
  for(int j=y; j<ys; j++)
   {
    PlaceCurseur(x, j);
    QVERIFY(frmSimulation->ActionPassagePietons->Enabled);
    frmSimulation->ActionPassagePietons->Execute();
    QVERIFY(cv->v[x][j].PassagePietons);
   }
  // Priorités piétons sur le trottoir nord : feu vers le sud possible
  PlaceCurseur(x, yn);
  QVERIFY(frmSimulation->ActionPrioritesPietons->Enabled);
  frmSimulation->MenuItemPrioritesPietons->ElementQt()->trigger();
  QVERIFY(frmDefVoiePietons->isVisible());
  QVERIFY(frmSimulation->ActionPrioritesPietons->Checked);
  QCOMPARE(Q(frmDefVoiePietons->LabelCoordonnees->Caption), QString::asprintf("%02d, %02d", x, yn));
  QVERIFY(frmDefVoiePietons->SpeedButtonS->Enabled);
  QVERIFY(!frmDefVoiePietons->SpeedButtonN->Enabled);
  QVERIFY(!frmDefVoiePietons->SpeedButtonS->Down);
  QVERIFY(frmDefVoiePietons->ImageSS->Visible);
  QVERIFY(!frmDefVoiePietons->ImageNN->Visible);
  QVERIFY(!frmDefVoiePietons->SpeedButtonS->icon().isNull());
  CaptureFiche(frmDefVoiePietons, "fiche_priorites_pietons.png");
  int NbFeuxP=NbFeuxPietonsDefinis();
  // Clic sur le feu sud : feu créé (et son opposé), fiche Feux de piétons ; puis Annuler partout
  Clique(frmDefVoiePietons->SpeedButtonS);
  QVERIFY(frmDefVoiePietons->SpeedButtonS->Down);
  QVERIFY(frmDefFeuxPietons->isVisible());
  int n=cv->v[x][yn].NumFeuP[sud-1];
  QVERIFY(n>0);
  QCOMPARE((int)cv->v[x][ys].NumFeuP[nord-1], n);
  QCOMPARE(Q(frmDefFeuxPietons->LabelCoordonneesFeuPietonsOppose->Caption), QString::asprintf("%02d, %02d", x, ys));
  QCOMPARE(Q(frmDefFeuxPietons->LabelDureeTraversee->Caption), QString::number(cv->FeuP[n-1].DureeTraversee));
  QCOMPARE(NbFeuxPietonsDefinis(), NbFeuxP+1);
  Clique(frmDefFeuxPietons->ButtonAnnuler);
  QVERIFY(!frmDefFeuxPietons->isVisible());
  QVERIFY(frmDefVoiePietons->SpeedButtonS->Down); // le feu existe tant que la priorité n'est pas annulée
  Clique(frmDefVoiePietons->ButtonAnnuler);
  QVERIFY(!frmDefVoiePietons->isVisible());
  QVERIFY(!frmSimulation->ActionPrioritesPietons->Checked);
  QCOMPARE((int)cv->v[x][yn].NumFeuP[sud-1], 0);
  QCOMPARE((int)cv->v[x][ys].NumFeuP[nord-1], 0);
  QCOMPARE(NbFeuxPietonsDefinis(), NbFeuxP);
  // Même chose, état vert et durée du vert, OK partout
  frmSimulation->MenuItemPrioritesPietons->ElementQt()->trigger();
  QVERIFY(frmDefVoiePietons->isVisible());
  Clique(frmDefVoiePietons->SpeedButtonS);
  QVERIFY(frmDefFeuxPietons->isVisible());
  n=cv->v[x][yn].NumFeuP[sud-1];
  QVERIFY(n>0);
  CliqueChoix(frmDefFeuxPietons->RadioGroupEtat, 1); // Vert
  frmDefFeuxPietons->CSpinEditDureeVert->Value=6;
  QVERIFY(frmDefFeuxPietons->ButtonAppliquer->Enabled);
  CaptureFiche(frmDefFeuxPietons, "fiche_feux_pietons.png");
  Messages.clear();
  Clique(frmDefFeuxPietons->ButtonOk); // assistant (aucun feu de circulation lié) : « Non »
  QVERIFY(!frmDefFeuxPietons->isVisible());
  QCOMPARE(Messages.size(), 1);
  QVERIFY2(Messages.first().contains(QString::fromUtf8("Voulez-vous affecter l'état opposé à tous les feux de circulation liés ?")), qPrintable(Messages.first()));
  QCOMPARE((int)cv->FeuP[n-1].Etat, (int)vert);
  QCOMPARE(cv->FeuP[n-1].DureeVert, 6);
  Clique(frmDefVoiePietons->ButtonOk);
  QVERIFY(!frmDefVoiePietons->isVisible());
  QCOMPARE((int)cv->v[x][yn].NumFeuP[sud-1], n);
  QCOMPARE((int)cv->v[x][ys].NumFeuP[nord-1], n);
  QCOMPARE(NbFeuxPietonsDefinis(), NbFeuxP+1);
 }
//---------------------------------------------------------------------------
void TestInterface::fichesParametres()
 { // Bouton Paramètres (modale) et ses fiches filles modales (bus, tram, taxis,
   // vehlibs), après avoir posé un arrêt de bus, un arrêt de tram, une place de
   // taxi et une place de vehlib ; puis Annuler, enregistrer et rouvrir
  QString Fichier=OuvreDemo("lot1_parametres.cvw");
  QVERIFY(!Fichier.isEmpty());
  Focalise();
  // Arrêt de bus et places de taxi et de vehlib sur trois cases de route
  int xb=-1, yb=-1;
  for(int i=0; (xb<0)&&(i<cv->NbX); i++)
   for(int j=0; (xb<0)&&(j<cv->NbY); j++)
    if (cv->AjoutArretBusPossible(i, j)&&!cv->v[i][j].NumParking&&!cv->v[i][j].EstDepotBus()) { xb=i; yb=j; }
  QVERIFY(xb>=0);
  PlaceCurseur(xb, yb);
  QVERIFY(frmSimulation->ActionArretBusTram->Enabled);
  frmSimulation->ActionArretBusTram->Execute();
  int NumArretBus=cv->v[xb][yb].NumArretBus;
  QVERIFY(NumArretBus>0);
  std::vector<std::pair<int, int>> Routes;
  for(int i=0; (Routes.size()<2)&&(i<cv->NbX); i++)
   for(int j=0; (Routes.size()<2)&&(j<cv->NbY); j++)
    if (cv->v[i][j].DirPoss&&!cv->v[i][j].NumParking&&!cv->v[i][j].EstDepotBus()&&!cv->v[i][j].NumArretBus&&
        ((i!=xb)||(j!=yb))&&(Routes.empty()||(Routes[0]!=std::make_pair(i, j))))
     Routes.push_back({i, j});
  QCOMPARE((int)Routes.size(), 2);
  PlaceCurseur(Routes[0].first, Routes[0].second);
  QVERIFY(frmSimulation->ActionPlaceTaxi->Enabled);
  frmSimulation->ActionPlaceTaxi->Execute();
  QVERIFY(cv->NbPlacesTaxi>0);
  PlaceCurseur(Routes[1].first, Routes[1].second);
  QVERIFY(frmSimulation->ActionPlaceVehlib->Enabled);
  frmSimulation->ActionPlaceVehlib->Execute();
  QVERIFY(cv->NbPlacesVehlib>0);
  // Voie de tram d'une case (de l'ouest vers l'est) dans une zone libre, et son arrêt
  int xt, yt;
  QVERIFY(ZoneLibre(3, 1, xt, yt));
  xt++;
  PlaceCurseur(xt-1, yt);
  PlaceCurseur(xt, yt);
  QVERIFY(frmSimulation->ActionDirectionDroiteTram->Enabled);
  frmSimulation->ActionDirectionDroiteTram->Execute();
  QVERIFY(cv->v[xt][yt].SensVoieTram);
  PlaceCurseur(xt, yt);
  QVERIFY(frmSimulation->ActionArretBusTram->Enabled);
  frmSimulation->ActionArretBusTram->Execute();
  int NumArretTram=cv->v[xt][yt].NumArretTram;
  QVERIFY(NumArretTram>0);
  // Paramètres : durées des feux, vitesses, piétons ; puis chaque fiche fille
  int DureeVert=cv->DureeVert, DureeOrange=cv->DureeOrange, NbTours=cv->NbToursParSeconde,
      ProbaPsgPt=cv->ProbaDirPsgPt, NbMaxBus=cv->NbMaxPersonnesBus, NbMaxTram=cv->NbMaxPersonnesTram,
      NbMaxTaxi=cv->NbMaxPersonnesTaxi, TauxVehlib=cv->TauxRemplissageVehlib;
  QVERIFY(cv->NbVitesses!=3);
  bool Affichage=false, CacheeBus=false, Revenue=false, CacheeTram=false, CacheeTaxi=false, CacheeVehlib=false;
  QuandModale([]() { return frmParamSimul; }, [&]()
   {
    Affichage=Q(frmParamSimul->Caption).endsWith(QString(" [%1x%2]").arg(cv->NbX).arg(cv->NbY))&&
              frmParamSimul->BitBtnDefLignesBus->Enabled&&frmParamSimul->BitBtnDefLignesTram->Enabled&&
              frmParamSimul->BitBtnDefTaxis->Enabled&&frmParamSimul->BitBtnDefVehlibs->Enabled&&
              !frmParamSimul->ButtonAppliquer->Enabled&&(frmParamSimul->CSpinEditDureeVert->Value==DureeVert)&&
              !frmParamSimul->BitBtnDefLignesBus->icon().isNull();
    // Saisie perdue au retour des fiches filles : la fiche réaffichée repasse
    // par FormShow, qui recharge les valeurs du moteur (comme dans la VCL)
    frmParamSimul->CSpinEditDureeVert->Value=DureeVert+1;
    QVERIFY(frmParamSimul->ButtonAppliquer->Enabled);
    Clique(frmParamSimul->BitBtnDefLignesBus);
   });
  QuandModale([]() { return frmParamBus; }, [&]()
   { // Ligne 1 : un arrêt, le n° NumArretBus
    CacheeBus=!frmParamSimul->isVisible();
    frmParamBus->CSpinEdit1->Value=1;
    frmParamBus->CSpinEdit11->Value=NumArretBus;
    frmParamBus->CSpinEditNbMaxPersonnesBus->Value=NbMaxBus-10;
    CaptureFiche(frmParamBus, "fiche_parametres_bus.png");
    Clique(frmParamBus->ButtonOk);
   });
  QuandModale([]() { return frmParamSimul; }, [&]()
   {
    Revenue=frmParamSimul->isVisible()&&(frmParamSimul->CSpinEditDureeVert->Value==DureeVert)&&!frmParamBus;
    Clique(frmParamSimul->BitBtnDefLignesTram);
   });
  QuandModale([]() { return frmParamTram; }, [&]()
   {
    CacheeTram=!frmParamSimul->isVisible();
    frmParamTram->CSpinEdit1->Value=1;
    frmParamTram->CSpinEdit11->Value=NumArretTram;
    frmParamTram->CSpinEditNbMaxPersonnesTram->Value=NbMaxTram-20;
    CaptureFiche(frmParamTram, "fiche_parametres_tram.png");
    Clique(frmParamTram->ButtonOk);
   });
  QuandModale([]() { return frmParamSimul; }, [&]() { Clique(frmParamSimul->BitBtnDefTaxis); });
  QuandModale([]() { return frmParamTaxi; }, [&]()
   {
    CacheeTaxi=!frmParamSimul->isVisible();
    frmParamTaxi->CSpinEditNbMaxPersonnesTaxi->Value=NbMaxTaxi+1;
    CaptureFiche(frmParamTaxi, "fiche_parametres_taxis.png");
    Clique(frmParamTaxi->ButtonOk);
   });
  QuandModale([]() { return frmParamSimul; }, [&]() { Clique(frmParamSimul->BitBtnDefVehlibs); });
  QuandModale([]() { return frmParamVehlib; }, [&]()
   {
    CacheeVehlib=!frmParamSimul->isVisible();
    frmParamVehlib->CSpinEditTauxRemplissage->Value=TauxVehlib-20;
    CaptureFiche(frmParamVehlib, "fiche_parametres_vehlibs.png");
    Clique(frmParamVehlib->ButtonOk);
   });
  QuandModale([]() { return frmParamSimul; }, [&]()
   {
    frmParamSimul->CSpinEditDureeVert->Value=DureeVert+2;
    frmParamSimul->CSpinEditDureeOrange->Value=DureeOrange+1;
    frmParamSimul->CSpinEditNbToursParSeconde->Value=NbTours-9;
    frmParamSimul->CSpinEditProbaPsgPt->Value=ProbaPsgPt-10;
    CliqueChoix(frmParamSimul->RadioGroupVitesses, 2);
    CaptureFiche(frmParamSimul, "fiche_parametres.png");
    Clique(frmParamSimul->ButtonOk);
   });
  Clique(frmSimulation->ToolButtonParametres);
  QVERIFY2(Etapes.empty(), qPrintable(Messages.join('\n')));
  QVERIFY(!frmParamSimul);
  QVERIFY(Affichage);
  QVERIFY(CacheeBus&&CacheeTram&&CacheeTaxi&&CacheeVehlib);
  QVERIFY(Revenue);
  QCOMPARE(cv->DureeVert, DureeVert+2);
  QCOMPARE(cv->DureeOrange, DureeOrange+1);
  QCOMPARE(cv->NbToursParSeconde, NbTours-9);
  QCOMPARE(cv->ProbaDirPsgPt, ProbaPsgPt-10);
  QCOMPARE(cv->NbVitesses, 3);
  QCOMPARE(cv->LigneBus[0].NbArrets, 1);
  QCOMPARE(cv->LigneBus[0].NumArret[0], NumArretBus);
  QCOMPARE(cv->NbLignesBus, 1);
  QCOMPARE(cv->NbMaxPersonnesBus, NbMaxBus-10);
  QCOMPARE(cv->LigneTram[0].NbArrets, 1);
  QCOMPARE(cv->LigneTram[0].NumArret[0], NumArretTram);
  QCOMPARE(cv->NbMaxPersonnesTram, NbMaxTram-20);
  QCOMPARE(cv->NbMaxPersonnesTaxi, NbMaxTaxi+1);
  QCOMPARE(cv->TauxRemplissageVehlib, TauxVehlib-20);
  QVERIFY(frmSimulation->SimulationModifiee);
  // Annuler (et Échap dans une fiche fille) : rien ne change
  QuandModale([]() { return frmParamSimul; }, [&]()
   {
    frmParamSimul->CSpinEditDureeVert->Value=DureeVert+5;
    Clique(frmParamSimul->BitBtnDefTaxis);
   });
  QuandModale([]() { return frmParamTaxi; }, [&]()
   {
    frmParamTaxi->CSpinEditNbMaxPersonnesTaxi->Value=NbMaxTaxi+3;
    Touche(frmParamTaxi->CSpinEditNbMaxPersonnesTaxi, Qt::Key_Escape);
   });
  QuandModale([]() { return frmParamSimul; }, [&]() { Clique(frmParamSimul->ButtonAnnuler); });
  frmSimulation->MenuItemParametres->ElementQt()->trigger();
  QVERIFY(Etapes.empty());
  QCOMPARE(cv->DureeVert, DureeVert+2);
  QCOMPARE(cv->NbMaxPersonnesTaxi, NbMaxTaxi+1);
  // Enregistrer, rouvrir : valeurs conservées, et affichées par la fiche
  Enregistre(Fichier);
  QCOMPARE(Q(frmSimulation->StatusBar->Panels->Items[0]->Text), QString::fromUtf8("Enregistrement effectué."));
  QVERIFY(!OuvreDemo("lot1_autre.cvw").isEmpty());
  QCOMPARE(cv->DureeVert, DureeVert);
  OuvreFichier(Fichier);
  QCOMPARE(Q(frmSimulation->Caption), QString("Centre-Ville - ")+QDir::toNativeSeparators(Fichier));
  QCOMPARE(cv->DureeVert, DureeVert+2);
  QCOMPARE(cv->DureeOrange, DureeOrange+1);
  QCOMPARE(cv->NbToursParSeconde, NbTours-9);
  QCOMPARE(cv->ProbaDirPsgPt, ProbaPsgPt-10);
  QCOMPARE(cv->NbVitesses, 3);
  QCOMPARE(cv->LigneBus[0].NumArret[0], NumArretBus);
  QCOMPARE(cv->NbMaxPersonnesBus, NbMaxBus-10);
  QCOMPARE(cv->LigneTram[0].NumArret[0], NumArretTram);
  QCOMPARE(cv->NbMaxPersonnesTram, NbMaxTram-20);
  QCOMPARE(cv->NbMaxPersonnesTaxi, NbMaxTaxi+1);
  QCOMPARE(cv->TauxRemplissageVehlib, TauxVehlib-20);
  bool Relu=false, ReluBus=false;
  QuandModale([]() { return frmParamSimul; }, [&]()
   {
    Relu=(frmParamSimul->CSpinEditDureeVert->Value==DureeVert+2)&&(frmParamSimul->CSpinEditDureeOrange->Value==DureeOrange+1)&&
         (frmParamSimul->CSpinEditNbToursParSeconde->Value==NbTours-9)&&(frmParamSimul->CSpinEditProbaPsgPt->Value==ProbaPsgPt-10)&&
         (frmParamSimul->RadioGroupVitesses->ItemIndex==2)&&!frmParamSimul->ButtonAppliquer->Enabled;
    Clique(frmParamSimul->BitBtnDefLignesBus);
   });
  QuandModale([]() { return frmParamBus; }, [&]()
   {
    ReluBus=(frmParamBus->CSpinEdit1->Value==1)&&(frmParamBus->CSpinEdit11->Value==NumArretBus)&&
            (frmParamBus->CSpinEditNbMaxPersonnesBus->Value==NbMaxBus-10)&&(Q(frmParamBus->LabelNbBus->Caption)=="1")&&
            !frmParamBus->ButtonAppliquer->Enabled;
    Clique(frmParamBus->ButtonAnnuler);
   });
  QuandModale([]() { return frmParamSimul; }, [&]() { Clique(frmParamSimul->ButtonAnnuler); });
  Clique(frmSimulation->ToolButtonParametres);
  QVERIFY(Etapes.empty());
  QVERIFY(Relu);
  QVERIFY(ReluBus);
 }
//---------------------------------------------------------------------------
void TestInterface::ficheCarrefours()
 { // Sélection de 4 x 4 cases libres, Carrefours (modale) : carrefour à feux ;
   // Annuler ; enregistrer et rouvrir
  QString Fichier=OuvreDemo("lot1_carrefours.cvw");
  QVERIFY(!Fichier.isEmpty());
  int xd, yd, xa, ya;
  QVERIFY(ZoneLibre(4, 4, xd, yd));
  QVERIFY(ZoneLibre(4, 4, xa, ya, {QRect(xd-1, yd-1, 6, 6)}));
  Focalise();
  Selectionne(xd, yd, xd+3, yd+3);
  QVERIFY(frmSimulation->ActionCarrefours->Enabled);
  int NbFeux=0;
  for(int i=0; i<cv->NbFeux; i++) NbFeux+=cv->Feu[i].Defini();
  bool Active=false;
  ReponseQuestions=QMessageBox::Yes; // « Êtes-vous bien sûr(e) de vouloir effacer... »
  Messages.clear();
  QuandModale([]() { return frmCarrefours; }, [&]()
   {
    Active=frmCarrefours->GroupBox44->isEnabled()&&frmCarrefours->SpeedButton44a->isEnabled()&&
           frmCarrefours->SpeedButton44b->isEnabled()&&!frmCarrefours->GroupBox34->isEnabled()&&
           !frmCarrefours->SpeedButton34a1->isEnabled()&&!frmCarrefours->SpeedButton88->isEnabled()&&
           !frmCarrefours->SpeedButton44b->icon().isNull();
    CaptureFiche(frmCarrefours, "fiche_carrefours.png");
    Clique(frmCarrefours->SpeedButton44b); // 4x4 à feux (Tag 4402)
   });
  Clique(frmSimulation->ToolButtonCarrefours);
  ReponseQuestions=QMessageBox::No;
  QVERIFY2(Etapes.empty(), qPrintable(Messages.join('\n')));
  QVERIFY(Active);
  QVERIFY(!frmCarrefours->isVisible());
  QCOMPARE(Messages.size(), 1);
  QVERIFY2(Messages.first().contains(QString::fromUtf8("Êtes-vous bien sûr(e) de vouloir effacer")), qPrintable(Messages.first()));
  auto VerifieCarrefour=[&]()
   {
    for(int i=0; i<4; i++)
     for(int j=0; j<4; j++)
      QCOMPARE((int)cv->v[xd+i][yd+j].DirPoss, (int)DirPossC4x4[i][j]);
    const int xf[NBDIR]={xd+1, xd+3, xd+2, xd}, yf[NBDIR]={yd, yd+1, yd+3, yd+2};
    const direction df[NBDIR]={sud, ouest, nord, est};
    int NumFeux[NBDIR];
    for(int i=0; i<NBDIR; i++)
     {
      QCOMPARE((int)cv->v[xf[i]][yf[i]].Priorite[df[i]-1], (int)feux);
      QVERIFY((NumFeux[i]=cv->v[xf[i]][yf[i]].NumFeu[df[i]-1])>0);
     }
    for(int i=0; i<NBDIR; i++)
     {
      feu &f=cv->Feu[NumFeux[i]-1];
      QCOMPARE((int)f.Etat, 1+2*((1+2+i)%2)); // Type=2
      QCOMPARE(f.NbFeuxDmd, 2);
      QCOMPARE(f.NbFeuxAtt, 2);
      QCOMPARE(f.NumFeuxDmd[0], NumFeux[(i+1)%NBDIR]);
      QCOMPARE(f.NumFeuxDmd[1], NumFeux[(i+3)%NBDIR]);
     }
    int NbFeuxApres=0;
    for(int i=0; i<cv->NbFeux; i++) NbFeuxApres+=cv->Feu[i].Defini();
    QCOMPARE(NbFeuxApres, NbFeux+4);
   };
  VerifieCarrefour();
  if (QTest::currentTestFailed()) return;
  QVERIFY(frmSimulation->SimulationModifiee);
  // Annuler sur une autre zone : rien ne change
  Selectionne(xa, ya, xa+3, ya+3);
  QuandModale([]() { return frmCarrefours; }, [&]() { Clique(frmCarrefours->ButtonAnnuler); });
  frmSimulation->MenuItemCarrefours->ElementQt()->trigger();
  QVERIFY(Etapes.empty());
  for(int i=0; i<4; i++)
   for(int j=0; j<4; j++)
    QVERIFY(CaseLibre(xa+i, ya+j));
  // Enregistrer, rouvrir : carrefour et feux conservés
  Enregistre(Fichier);
  QVERIFY(!OuvreDemo("lot1_autre.cvw").isEmpty());
  QVERIFY(CaseLibre(xd+1, yd+1));
  OuvreFichier(Fichier);
  QCOMPARE(Q(frmSimulation->Caption), QString("Centre-Ville - ")+QDir::toNativeSeparators(Fichier));
  VerifieCarrefour();
 }
//---------------------------------------------------------------------------
// Lot 2 des fiches secondaires : distances et informations
//---------------------------------------------------------------------------
namespace
 {
  // asDir1 des fiches de distances : trois espaces et la flèche de la police
  // Symbol (0xAD haut, 0xAE droite, 0xAF bas, 0xAC gauche)
  QString Fleche(direction d)
   {
    const char16_t Symboles[NBDIR+1]={0, 0xAD, 0xAE, 0xAF, 0xAC};
    return d?QString("   ")+QChar(Symboles[d]):QString();
   }
  //-------------------------------------------------------------------------
  // Tableau attendu d'une fiche de distances pour une case, d'après le moteur
  struct TableDistances
   {
    QString Titre;
    int Nb=0, NbDir=0;
    bool Definie=false;
    std::function<direction(int)> Dir; // direction de la colonne d+1
    std::function<int(int, int)> Distance; // ligne k+1, colonne d+1
   };
  struct FicheDistances
   {
    const char *Nom; // capture fiche_distances_<Nom>.png
    TForm *Fiche;
    TStringGrid *Grille;
    TAction *Action;
    TCheckBox *Couloirs; // case « Avec couloirs de bus » (parkings, files de parkings)
    std::function<TableDistances(voie *, bool)> Table;
    std::function<void()> Ouvre; // menu, raccourci ou action
   };
  TableDistances TableRoute(voie *v)
   { // Colonnes : directions possibles de la case (Dirdp)
    TableDistances t;
    t.NbDir=NbDir[v->DirPoss];
    t.Dir=[v](int d) { return Dirdp[d][v->DirPoss]; };
    return t;
   }
  //-------------------------------------------------------------------------
  // Différences entre deux textes, pour les fiches d'informations
  void Compare(QStringList &Ecarts, const char *Nom, const AnsiString &asObtenu, const QString &Attendu)
   {
    if (Q(asObtenu)!=Attendu) Ecarts<<QString::fromUtf8("%1 : « %2 » au lieu de « %3 »").arg(QString::fromUtf8(Nom), Q(asObtenu), Attendu);
   }
  QString NumeroCoordonnees(int n, int x, int y) { return QString::asprintf("%d (%02d, %02d)", n, x, y); }
  QString Cadre(int n, int x, int y) { return QString(27, ' ')+NumeroCoordonnees(n, x, y)+" "; }
  // Pourcentages (FormatFloat « #.000% » ou « 0.000% » de la VCL)
  const QRegularExpression Pourcentage("^(-?\\d*,\\d{3}%|NAN|-?INF)$");
 }
//---------------------------------------------------------------------------
void TestInterface::VerifieFichesDistances(const QString &asCarte, bool Captures)
 { // Sur une copie de la carte, en simulation : chaque fiche de distances ouverte
   // sur une case de route (de voie de tram pour les arrêts de tram), puis sur une
   // autre case cliquée ; tableau = distances du moteur, hauteur de la fiche
   // selon le nombre de lignes (FormShow), fermeture par la case de la fenêtre
  QString Copie=OuvreCarte(asCarte, "lot2_distances_"+QFileInfo(asCarte).completeBaseName()+".cvw");
  QVERIFY(!Copie.isEmpty());
  Focalise();
  QVERIFY2(Demarre(), qPrintable(Messages.join('\n')));
  QVERIFY(JoueJusquAuTour(2));
  TfrmSimulation *s=frmSimulation;
  std::vector<FicheDistances> Fiches=
   {
    {"parkings", frmAffDistParkings, frmAffDistParkings->StringGridDistances, s->ActionDistancesParkings,
     frmAffDistParkings->CheckBoxCouloirsBus,
     [](voie *v, bool c)
      {
       TableDistances t=TableRoute(v);
       t.Titre="Parkings"; t.Nb=cv->NbParkings; t.Definie=v->DistParking;
       t.Distance=[v, c](int k, int d) { return v->DistParking[k][d][c]; };
       return t;
      },
     [s]() { s->MenuItemInfosDistancesEtapes->ElementQt()->trigger(); }},
    {"arrets_bus", frmAffDistArretsBus, frmAffDistArretsBus->StringGridDistances, s->ActionDistancesArretsBus, nullptr,
     [](voie *v, bool)
      {
       TableDistances t=TableRoute(v);
       t.Titre=QString::fromUtf8("Arrêts bus"); t.Nb=cv->NbArretsBus; t.Definie=v->DistArretBus;
       t.Distance=[v](int k, int d) { return v->DistArretBus[k][d]; };
       return t;
      },
     [s]() { s->MenuItemDistancesArretsBus->ElementQt()->trigger(); }}, // menu contextuel de simulation
    {"arrets_tram", frmAffDistArretsTram, frmAffDistArretsTram->StringGridDistances, s->ActionDistancesArretsTram, nullptr,
     [](voie *v, bool)
      {
       TableDistances t;
       t.Titre=QString::fromUtf8("Arrêts tram"); t.Nb=cv->NbArretsTram; t.Definie=v->DistArretTram;
       t.NbDir=v->NbDirPossSensVoieTram(indefinie);
       t.Dir=[v](int d) { return v->DirSensVoieTram(indefinie, d); };
       t.Distance=[v](int k, int d) { return v->DistArretTram[k][d]; };
       return t;
      },
     []() { QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_R, Qt::ControlModifier); }},
    {"files_taxi", frmAffDistFilesTaxi, frmAffDistFilesTaxi->StringGridDistances, s->ActionDistancesFilesTaxi, nullptr,
     [](voie *v, bool)
      {
       TableDistances t=TableRoute(v);
       t.Titre="Files taxi"; t.Nb=cv->NbFilesTaxi; t.Definie=v->DistFileTaxi;
       t.Distance=[v](int k, int d) { return v->DistFileTaxi[k][d]; };
       return t;
      },
     [s]() { s->MenuItemInfosDistancesFilesTaxi->ElementQt()->trigger(); }},
    {"files_parkings", frmAffDistFilesPark, frmAffDistFilesPark->StringGridDistances, s->ActionDistancesFilesPark,
     frmAffDistFilesPark->CheckBoxCouloirsBus,
     [](voie *v, bool c)
      {
       TableDistances t=TableRoute(v);
       t.Titre="Files parking"; t.Nb=cv->NbFilesPark; t.Definie=v->DistFilePark;
       t.Distance=[v, c](int k, int d) { return v->DistFilePark[k][d][c]; };
       return t;
      },
     []() { QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_K, Qt::ControlModifier); }},
    {"places_vehlib", frmAffDistPlacesVehlib, frmAffDistPlacesVehlib->StringGridDistances, s->ActionDistancesPlacesVehlib, nullptr,
     [](voie *v, bool)
      {
       TableDistances t=TableRoute(v);
       t.Titre="Places vehlib"; t.Nb=cv->NbPlacesVehlib; t.Definie=v->DistPlaceVehlib;
       t.Distance=[v](int k, int d) { return v->DistPlaceVehlib[k][d]; };
       return t;
      },
     [s]() { s->ActionDistancesPlacesVehlib->Execute(); }},
   };
  // Tableau affiché pour la case courante
  auto Verifie=[](const FicheDistances &f) -> QString
   {
    int x=frmSimulation->DrawGridSimul->Col, y=frmSimulation->DrawGridSimul->Row;
    voie *v=&cv->v[x][y];
    TableDistances t=f.Table(v, f.Couloirs&&f.Couloirs->Checked);
    TStringGrid *g=f.Grille;
    QStringList e;
    if (!t.Definie)
     { // EffaceInfos seul
      for(int c=0; c<g->ColCount; c++)
       for(int r=0; r<g->RowCount; r++)
        if (!Q(g->Cells[c][r]).isEmpty()) e<<QString("case [%1][%2] non vide").arg(c).arg(r);
      return e.join('\n');
     }
    Compare(e, "titre", f.Fiche->Caption, QString("%1 (%2, %3)").arg(t.Titre).arg(x).arg(y));
    if (g->RowCount!=1+t.Nb) e<<QString("RowCount %1 au lieu de %2").arg((int)g->RowCount).arg(1+t.Nb);
    Compare(e, "[0][0]", g->Cells[0][0], QString());
    for(int d=0; d<t.NbDir; d++) Compare(e, "en-tête", g->Cells[d+1][0], Fleche(t.Dir(d)));
    for(int k=1; k<=t.Nb; k++)
     {
      Compare(e, "numéro", g->Cells[0][k], QString::asprintf("%5d", k));
      for(int d=0; d<t.NbDir; d++)
       {
        int de=t.Distance(k-1, d);
        Compare(e, "distance", g->Cells[d+1][k], (de==DIST_INDEFINIE)?QString("     "):QString::asprintf("%5d", de));
       }
      for(int c=t.NbDir+1; c<g->ColCount; c++) Compare(e, "colonne vide", g->Cells[c][k], QString());
     }
    return e.join('\n');
   };
  // Case de route à au moins deux directions (de voie de tram pour les arrêts de
  // tram) où le tableau est défini, dans la partie visible de la carte, hors de (xh, yh)
  auto CaseDe=[](const FicheDistances &f, int xh, int yh, int &xc, int &yc)
   {
    int Meilleur=-1;
    for(int y=1; y<22; y++)
     for(int x=1; x<34; x++)
      {
       voie *v=&cv->v[x][y];
       if (((x==xh)&&(y==yh))||v->NumParking) continue;
       TableDistances t=f.Table(v, false);
       if (!(f.Fiche==frmAffDistArretsTram?(v->SensVoieTram>aucune_voie):(v->DirPoss>aucune))) continue;
       int Note=2*t.Definie+(t.NbDir>=2);
       if (Note>Meilleur) { Meilleur=Note; xc=x; yc=y; }
      }
    return Meilleur>=0;
   };
  for(const FicheDistances &f : Fiches)
   {
    int x=0, y=0, x2=0, y2=0;
    if ((f.Fiche==frmAffDistArretsTram)&&!cv->NbArretsTram) continue; // pas de voie de tram (Démo)
    QVERIFY2(CaseDe(f, -1, -1, x, y), f.Nom);
    QVERIFY2(CaseDe(f, x, y, x2, y2), f.Nom);
    Focalise();
    PlaceCurseur(x, y);
    QVERIFY2(f.Action->Enabled, f.Nom);
    QVERIFY(!f.Fiche->isVisible());
    f.Ouvre();
    QVERIFY2(f.Fiche->isVisible(), f.Nom);
    QVERIFY(f.Action->Checked);
    QVERIFY(!f.Fiche->isModal()&&!QApplication::activeModalWidget());
    QVERIFY(f.Fiche->windowFlags().testFlag(Qt::WindowStaysOnTopHint));
    QString e=Verifie(f);
    QVERIFY2(e.isEmpty(), qPrintable(QString("%1 (%2, %3) :\n%4").arg(f.Nom).arg(x).arg(y).arg(e)));
    // FormShow : hauteur de la fiche (cadre compris) d'après le nombre de lignes,
    // grille de RowCount lignes de 20 pixels et un trait, sans défilement jusqu'à 10
    TableDistances t=f.Table(&cv->v[x][y], false);
    // (seule f_afdistp compte le panneau de sa case à cocher : 43 et 300 ; f_afdistfp
    // garde 26 et 283, sa grille est donc plus courte de 17 pixels)
    int Panneau=(f.Fiche==frmAffDistParkings)?17:0, Lignes=1+t.Nb;
    int Hauteur=(t.Nb>10)?283+Panneau:26+Panneau+21*Lignes;
    QCOMPARE((int)f.Fiche->Height, Hauteur);
    QCOMPARE(f.Fiche->height(), Hauteur-22);
    QCOMPARE(f.Fiche->width(), (f.Fiche==frmAffDistArretsTram)?156:125); // Constraints 172 / 141, cadre compris
    int Grille=Hauteur-22-(f.Couloirs?17:0)-4; // zone cliente, case à cocher, bord de la grille
    QCOMPARE(f.Grille->viewport()->height(), Grille);
    if (t.Nb>0)
     QCOMPARE(f.Grille->verticalScrollBar()->maximum()>0, 21*Lignes>Grille+1); // dernier trait coupé toléré
    else
     { // RowCount=1 : TCustomGrid::SetRowCount ramène FixedRows à 0, et pour la suite
      QCOMPARE(f.Grille->verticalScrollBar()->maximum(), 0);
      QCOMPARE((int)f.Grille->FixedRows, 0);
     }
    if (Captures) CaptureFiche(f.Fiche, QString("fiche_distances_%1.png").arg(f.Nom));
    if (t.Nb>10)
     { // Clavier (TCustomGrid::KeyDown) : la case courante descend d'une page, la
       // grille défile pour la garder visible ; Ctrl+Début revient en haut
      QCOMPARE((int)f.Grille->Row, 1);
      Touche(f.Grille, Qt::Key_PageDown);
      QVERIFY((int)f.Grille->Row>1);
      QVERIFY((int)f.Grille->TopRow>1);
      QCOMPARE(f.Grille->verticalScrollBar()->value(), (int)f.Grille->TopRow-1);
      QTest::keyClick(f.Grille, Qt::Key_Home, Qt::ControlModifier);
      QCOMPARE((int)f.Grille->Row, 1);
      QCOMPARE((int)f.Grille->TopRow, 1);
     }
    // Couloirs de bus : distances prioritaires
    if (f.Couloirs)
     {
      Clique(f.Couloirs);
      QVERIFY(f.Couloirs->Checked);
      e=Verifie(f);
      QVERIFY2(e.isEmpty(), qPrintable(QString("%1 couloirs :\n%2").arg(f.Nom, e)));
      Clique(f.Couloirs);
      QVERIFY(!f.Couloirs->Checked);
     }
    // Clic sur une autre case : le tableau la suit (DrawGridSimulClick)
    Focalise();
    CliqueCase(x2, y2);
    QCOMPARE((int)frmSimulation->DrawGridSimul->Col, x2);
    QCOMPARE((int)frmSimulation->DrawGridSimul->Row, y2);
    e=Verifie(f);
    QVERIFY2(e.isEmpty(), qPrintable(QString("%1 (%2, %3) :\n%4").arg(f.Nom).arg(x2).arg(y2).arg(e)));
    // Case de la fenêtre : fiche cachée, action décochée (FormHide)
    f.Fiche->Close();
    QVERIFY(!f.Fiche->isVisible());
    QVERIFY(!f.Action->Checked);
   }
  // Fiches ouvertes à l'arrêt : refermées (TimerSimulTimer, tdArret)
  for(const FicheDistances &f : Fiches)
   {
    int x=0, y=0;
    if ((f.Fiche==frmAffDistArretsTram)&&!cv->NbArretsTram) continue;
    QVERIFY(CaseDe(f, -1, -1, x, y));
    Focalise();
    PlaceCurseur(x, y);
    f.Action->Execute();
    QVERIFY2(f.Fiche->isVisible(), f.Nom);
   }
  QVERIFY(Arrete());
  for(const FicheDistances &f : Fiches)
   {
    QVERIFY2(!f.Fiche->isVisible(), f.Nom);
    QVERIFY(!f.Action->Checked);
   }
 }
//---------------------------------------------------------------------------
void TestInterface::fichesDistances()
 { // Géant (arrêts, files et places de chaque sorte, plus de 10 lignes pour les
   // arrêts et les places) puis Démo (4 parkings, aucun arrêt, file ni place :
   // tableaux vides, ce qui laisse ensuite ces grilles sans ligne fixe)
  VerifieFichesDistances(QString::fromUtf8("Géant.cvw"), true);
  if (QTest::currentTestFailed()) return;
  VerifieFichesDistances(QString::fromUtf8("Démo.cvw"), false);
 }
//---------------------------------------------------------------------------
void TestInterface::SuitObjet(const char *Type, std::function<QString(long)> Ecart, std::function<bool(int &, int &)> Position, int NbTours)
 { // À chaque tour, la fiche montre l'objet de la case courante (rafraîchissement
   // demandé par le moteur) ; si l'objet suivi s'est déplacé, l'utilisateur clique
   // sur sa nouvelle case et la fiche le retrouve
  for(int i=0; i<NbTours; i++)
   {
    long Tour=cv->TourCrt;
    JoueTour();
    QCOMPARE((long)cv->TourCrt, Tour+1);
    QString e=Ecart(cv->TourCrt-1); // rafraîchissement en fin de tour, avant TourCrt++
    QVERIFY2(e.isEmpty(), qPrintable(QString("%1, tour %2 :\n%3").arg(Type).arg(cv->TourCrt).arg(e)));
    int x, y;
    if (!Position(x, y)) break; // arrivé ou monté dans un véhicule
    if ((x!=frmSimulation->DrawGridSimul->Col)||(y!=frmSimulation->DrawGridSimul->Row))
     {
      Focalise();
      CliqueCase(x, y);
      e=Ecart(cv->TourCrt);
      QVERIFY2(e.isEmpty(), qPrintable(QString("%1, tour %2, après le clic sur (%3, %4) :\n%5").arg(Type).arg(cv->TourCrt).arg(x).arg(y).arg(e)));
     }
   }
 }
//---------------------------------------------------------------------------
void TestInterface::fichesInfos()
 { // Géant en simulation : chaque fiche d'informations ouverte par son menu, son
   // raccourci ou le menu contextuel sur un objet de la case courante, suivie
   // pendant quelques tours, puis fermée par « Vu » ou Échap
  QVERIFY(!OuvreCarte(QString::fromUtf8("Géant.cvw"), "lot2_infos.cvw").isEmpty());
  Focalise();
  QVERIFY2(Demarre(), qPrintable(Messages.join('\n')));
  QVERIFY(JoueJusquAuTour(15));
  auto Case=[]() { return &cv->v[frmSimulation->DrawGridSimul->Col][frmSimulation->DrawGridSimul->Row]; };
  auto Coordonnees=[]() { return QString::asprintf("%02d, %02d", (int)frmSimulation->DrawGridSimul->Col, (int)frmSimulation->DrawGridSimul->Row); };
  // Premier objet trouvé sur la carte (Numero(v) > 0), case courante
  auto Trouve=[this](std::function<int(voie *)> Numero) -> int
   {
    for(int y=0; y<cv->NbY; y++)
     for(int x=0; x<cv->NbX; x++)
      if (int n=Numero(&cv->v[x][y]))
       {
        Focalise();
        CliqueCase(x, y);
        return n;
       }
    return 0;
   };
  //-------------------------------------------------------------------------
  // Véhicule (Ctrl+V)
  {
   int n=Trouve([](voie *v) { return (v->NumVehicule>0)&&!v->NumParking?v->NumVehicule:0; });
   QVERIFY(n>0);
   TfrmInfosVehicule *f=frmInfosVehicule;
   auto Ecart=[f, Case, Coordonnees](long Tour) -> QString
    {
     voie *v=Case();
     QStringList e;
     Compare(e, "coordonnées", f->LabelCoordonnees->Caption, Coordonnees());
     if (v->NumVehicule>0)
      {
       vehicule *Veh=cv->Vehicule+v->NumVehicule-1;
       const char *Priorites[]={"Normal", "Urgences", "Police"};
       if (!f->RadioButtonVehiculeNumero->Checked||f->LabelDirection->isHidden()) e<<QString::fromUtf8("véhicule non affiché");
       Compare(e, "cadre", f->GroupBoxVehicule->Caption, Cadre(v->NumVehicule, Veh->x, Veh->y));
       Compare(e, "direction", f->LabelDirection->Caption, Q(asDir[Veh->Dir]));
       Compare(e, "priorité", f->LabelNivPriorite->Caption, Priorites[Veh->NivPriorite]);
       Compare(e, "sorti au tour", f->LabelTourSortie->Caption, QString::number(Veh->TourDepart));
       Compare(e, "tours", f->LabelTours->Caption, QString::number(Tour-Veh->TourDepart));
       Compare(e, "parcours", f->LabelParcours->Caption, QString::number(Veh->NbCasesParcourues));
       Compare(e, "vitesse", f->LabelVitesse->Caption, QString::asprintf("%d/%d", 1+(v->NumVehicule-1)%cv->NbVitesses, cv->NbVitesses));
       if (Veh->NumParkingArrivee)
        Compare(e, "parking d'arrivée", f->LabelParkingArrivee->Caption,
                NumeroCoordonnees(Veh->NumParkingArrivee, cv->Parking[Veh->NumParkingArrivee-1].x, cv->Parking[Veh->NumParkingArrivee-1].y));
       for(TLabel *l : {f->LabelParcoursEffectue, f->LabelDetour, f->LabelAttente})
        if (!Pourcentage.match(Q(l->Caption)).hasMatch()) e<<("pourcentage : "+Q(l->Caption));
      }
     else
      if (!f->RadioButtonAucunVehicule->Checked||!f->LabelDirection->isHidden()) e<<QString::fromUtf8("« Aucun véhicule » non affiché");
     return e.join('\n');
    };
   QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_V, Qt::ControlModifier);
   QVERIFY(f->isVisible());
   QVERIFY(frmSimulation->ActionDetailVehicule->Checked);
   QVERIFY(!f->isModal()&&!QApplication::activeModalWidget());
   QVERIFY(f->windowFlags().testFlag(Qt::WindowStaysOnTopHint));
   QString e=Ecart(cv->TourCrt);
   QVERIFY2(e.isEmpty(), qPrintable(e));
   QVERIFY(Q(f->GroupBoxVehicule->Caption).contains(QString::number(n)+" ("));
   CaptureFiche(f, "fiche_infos_vehicule.png");
   SuitObjet("véhicule", Ecart, [n](int &x, int &y)
    {
     vehicule *Veh=cv->Vehicule+n-1;
     x=Veh->x; y=Veh->y;
     return (x>=0)&&(y>=0)&&(cv->v[x][y].NumVehicule==n);
    }, 8);
   if (QTest::currentTestFailed()) return;
   Clique(f->ButtonOk); // « Vu »
   QVERIFY(!f->isVisible());
   QVERIFY(!frmSimulation->ActionDetailVehicule->Checked);
  }
  //-------------------------------------------------------------------------
  // Bus (menu Informations)
  {
   // (pas le premier bus : l'original lisait son tour de départ pour tous, et les
   // tours sont joués jusqu'à ce que les deux diffèrent)
   QVERIFY(JoueTours([]()
    {
     for(int i=1; i<cv->NbBus; i++)
      if (cv->Bus[i].Defini()&&(cv->Bus[i].TourDepartArretSvt!=cv->Bus->TourDepartArretSvt)) return true;
     return false;
    }, 2000));
   int n=Trouve([](voie *v) { return ((v->NumBus>1)&&(cv->Bus[v->NumBus-1].TourDepartArretSvt!=cv->Bus->TourDepartArretSvt))?v->NumBus:0; });
   QVERIFY(n>0);
   TfrmInfosBus *f=frmInfosBus;
   auto Ecart=[f, Case, Coordonnees](long Tour) -> QString
    {
     voie *v=Case();
     QStringList e;
     Compare(e, "coordonnées", f->LabelCoordonnees->Caption, Coordonnees());
     if (v->NumBus>0)
      {
       bus *b=cv->Bus+v->NumBus-1;
       if (!f->RadioButtonBusNumero->Checked||f->LabelDirection->isHidden()) e<<"bus non affiché";
       Compare(e, "cadre", f->GroupBoxBus->Caption, Cadre(v->NumBus, b->x, b->y));
       Compare(e, "direction", f->LabelDirection->Caption, Q(asDir[b->Dir]));
       Compare(e, "ligne", f->LabelNumeroLigne->Caption, QString::number(b->NumLigne));
       Compare(e, "sorti au tour", f->LabelTourSortie->Caption, QString::number(b->TourDepart));
       Compare(e, "personnes", f->LabelNbPersonnes->Caption, QString::number(b->NbPersonnes));
       Compare(e, "tours", f->LabelTours->Caption, QString::number(Tour-b->TourDepart));
       Compare(e, "parcours", f->LabelParcours->Caption, QString::number(b->NbCasesParcourues));
       Compare(e, "tours depuis l'arrêt", f->LabelToursEntreArrets->Caption, QString::number(Tour-b->TourArriveDrnArret));
       Compare(e, "parcours depuis l'arrêt", f->LabelParcoursEntreArrets->Caption, QString::number(b->NbCasesParcouruesDepuisDrnArret));
       Compare(e, "prochain arrêt", f->LabelProchainArret->Caption, // v6.0.2 : de ce bus (l'original lisait le premier bus)
               NumeroCoordonnees(b->NumArretSvt, cv->ArretBus[b->NumArretSvt-1].x, cv->ArretBus[b->NumArretSvt-1].y)+
               QString(" depuis le tour %1").arg(b->TourDepartArretSvt));
       Compare(e, "vitesse", f->LabelVitesse->Caption, QString::asprintf("1/%d", cv->NbVitesses));
      }
     else
      if (!f->RadioButtonAucunBus->Checked||!f->LabelDirection->isHidden()) e<<QString::fromUtf8("« Aucun bus » non affiché");
     return e.join('\n');
    };
   frmSimulation->MenuItemInfosBus->ElementQt()->trigger();
   QVERIFY(f->isVisible());
   QVERIFY(frmSimulation->ActionDetailBus->Checked);
   QString e=Ecart(cv->TourCrt);
   QVERIFY2(e.isEmpty(), qPrintable(e));
   CaptureFiche(f, "fiche_infos_bus.png");
   SuitObjet("bus", Ecart, [n](int &x, int &y)
    {
     x=cv->Bus[n-1].x; y=cv->Bus[n-1].y;
     return (x>=0)&&(y>=0)&&(cv->v[x][y].NumBus==n);
    }, 6);
   if (QTest::currentTestFailed()) return;
   Touche(f->ButtonOk, Qt::Key_Escape); // « Vu » est le bouton Cancel
   QVERIFY(!f->isVisible());
   QVERIFY(!frmSimulation->ActionDetailBus->Checked);
  }
  //-------------------------------------------------------------------------
  // Tram (menu contextuel de simulation)
  {
   QVERIFY(JoueTours([]()
    {
     for(int i=1; i<cv->NbTrams; i++)
      if (cv->Tram[i].Defini()&&(cv->Tram[i].TourDepartArretSvt!=cv->Tram->TourDepartArretSvt)) return true;
     return false;
    }, 2000));
   int n=Trouve([](voie *v) { return ((v->NumTram>1)&&(cv->Tram[v->NumTram-1].TourDepartArretSvt!=cv->Tram->TourDepartArretSvt))?v->NumTram:0; });
   QVERIFY(n>0);
   TfrmInfosTram *f=frmInfosTram;
   auto Ecart=[f, Case, Coordonnees](long Tour) -> QString
    {
     voie *v=Case();
     QStringList e;
     Compare(e, "coordonnées", f->LabelCoordonnees->Caption, Coordonnees());
     if (v->NumTram>0)
      {
       tram *t=cv->Tram+v->NumTram-1;
       if (!f->RadioButtonTramNumero->Checked||f->LabelDirection->isHidden()) e<<"tram non affiché";
       Compare(e, "cadre", f->GroupBoxTram->Caption, Cadre(v->NumTram, t->x, t->y));
       Compare(e, "direction", f->LabelDirection->Caption, Q(asDir[t->Dir]));
       Compare(e, "ligne", f->LabelNumeroLigne->Caption, QString(QChar('A'+t->NumLigne-1)));
       Compare(e, "personnes", f->LabelNbPersonnes->Caption, QString::number(t->NbPersonnes));
       Compare(e, "tours", f->LabelTours->Caption, QString::number(Tour)); // Tous les trams partent au tour 0 : tours depuis le départ, comme pour un bus
       Compare(e, "prochain arrêt", f->LabelProchainArret->Caption, // v6.0.2 : de ce tram (l'original lisait le premier tram)
               NumeroCoordonnees(t->NumArretSvt, cv->ArretTram[t->NumArretSvt-1].x, cv->ArretTram[t->NumArretSvt-1].y)+
               QString(" depuis le tour %1").arg(t->TourDepartArretSvt));
       Compare(e, "parcours", f->LabelParcours->Caption, QString::number(t->NbCasesParcourues));
       Compare(e, "tours depuis l'arrêt", f->LabelToursEntreArrets->Caption, QString::number(Tour-t->TourArriveDrnArret));
       for(TLabel *l : {f->LabelParcoursEffectue, f->LabelDetour, f->LabelAttente})
        if (!Pourcentage.match(Q(l->Caption)).hasMatch()) e<<("pourcentage : "+Q(l->Caption));
      }
     else
      if (!f->RadioButtonAucunTram->Checked||!f->LabelDirection->isHidden()) e<<QString::fromUtf8("« Aucun tram » non affiché");
     return e.join('\n');
    };
   frmSimulation->MenuItemDetailTram->ElementQt()->trigger();
   QVERIFY(f->isVisible());
   QVERIFY(frmSimulation->ActionDetailTram->Checked);
   QString e=Ecart(cv->TourCrt);
   QVERIFY2(e.isEmpty(), qPrintable(e));
   CaptureFiche(f, "fiche_infos_tram.png");
   SuitObjet("tram", Ecart, [n](int &x, int &y)
    {
     x=cv->Tram[n-1].x; y=cv->Tram[n-1].y;
     return (x>=0)&&(y>=0)&&(cv->v[x][y].NumTram==n);
    }, 6);
   if (QTest::currentTestFailed()) return;
   Clique(f->ButtonOk);
   QVERIFY(!f->isVisible());
   QVERIFY(!frmSimulation->ActionDetailTram->Checked);
  }
  //-------------------------------------------------------------------------
  // Taxi (Ctrl+I) et vehlib (Ctrl+E)
  const char *Situations[NBSITUATIONSTAXI]={"En attente", "Départ d'une course", "Dépôt de client(s)", "Retour d'une course"};
  {
   int n=Trouve([](voie *v) { return v->NumTaxi; });
   QVERIFY(n>0);
   TfrmInfosTaxi *f=frmInfosTaxi;
   auto Ecart=[f, Case, Coordonnees, Situations](long Tour) -> QString
    {
     voie *v=Case();
     QStringList e;
     Compare(e, "coordonnées", f->LabelCoordonnees->Caption, Coordonnees());
     if (v->NumTaxi>0)
      {
       taxi *t=cv->Taxi+v->NumTaxi-1;
       if (!f->RadioButtonTaxiNumero->Checked||f->LabelDirection->isHidden()) e<<"taxi non affiché";
       Compare(e, "cadre", f->GroupBoxTaxi->Caption, Cadre(v->NumTaxi, t->x, t->y));
       Compare(e, "situation", f->LabelSituation->Caption, QString::fromUtf8(Situations[t->Situation]));
       Compare(e, "direction", f->LabelDirection->Caption, Q(asDir[t->Dir]));
       Compare(e, "file", f->LabelNumeroFile->Caption, QString::number(t->NumFile+1));
       Compare(e, "personnes", f->LabelNbPersonnes->Caption, QString::number(t->NbPersonnes));
       Compare(e, "parcours", f->LabelParcours->Caption, QString::number(t->NbCasesParcourues));
       Compare(e, "tours depuis le départ", f->LabelToursDepuisDepart->Caption, QString::number(Tour-t->TourDepartDrnCourse));
       // v6.0.2 : DistFileTaxi[file][direction] (l'original inversait les indices)
       int Min=0x7FFFFFFF;
       for(int d=0; d<NbDir[v->DirPoss]; d++) Min=qMin(Min, v->DistFileTaxi[t->NumFile][d]);
       Compare(e, "distance à la tête de file", f->LabelDistanceTeteFileDepart->Caption, QString::number(Min));
      }
     else
      if (!f->RadioButtonAucunTaxi->Checked||!f->LabelDirection->isHidden()) e<<QString::fromUtf8("« Aucun taxi » non affiché");
     return e.join('\n');
    };
   QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_I, Qt::ControlModifier);
   QVERIFY(f->isVisible());
   QVERIFY(frmSimulation->ActionDetailTaxi->Checked);
   QString e=Ecart(cv->TourCrt);
   QVERIFY2(e.isEmpty(), qPrintable(e));
   CaptureFiche(f, "fiche_infos_taxi.png");
   SuitObjet("taxi", Ecart, [n](int &x, int &y)
    {
     x=cv->Taxi[n-1].x; y=cv->Taxi[n-1].y;
     return (x>=0)&&(y>=0)&&(cv->v[x][y].NumTaxi==n);
    }, 4);
   if (QTest::currentTestFailed()) return;
   Clique(f->ButtonOk);
   QVERIFY(!f->isVisible());
  }
  {
   int n=Trouve([](voie *v) { return v->NumVehlib; });
   QVERIFY(n>0);
   TfrmInfosVehlib *f=frmInfosVehlib;
   auto Ecart=[f, Case, Coordonnees, Situations](long Tour) -> QString
    {
     voie *v=Case();
     QStringList e;
     Compare(e, "coordonnées", f->LabelCoordonnees->Caption, Coordonnees());
     if (v->NumVehlib>0)
      {
       vehlib *l=cv->Vehlib+v->NumVehlib-1;
       if (!f->RadioButtonVehlibNumero->Checked||f->LabelDirection->isHidden()) e<<"vehlib non affiché";
       Compare(e, "cadre", f->GroupBoxVehlib->Caption, Cadre(v->NumVehlib, l->x, l->y));
       Compare(e, "situation", f->LabelSituation->Caption, QString::fromUtf8(Situations[l->Situation]));
       Compare(e, "direction", f->LabelDirection->Caption, Q(asDir[l->Dir]));
       Compare(e, "place de départ", f->LabelNumeroPlaceDepart->Caption,
               NumeroCoordonnees(l->NumPlaceDepart, cv->PlaceVehlib[l->NumPlaceDepart-1].x, cv->PlaceVehlib[l->NumPlaceDepart-1].y));
       Compare(e, "personnes", f->LabelNbPersonnes->Caption, QString::number(l->NbPersonnes));
       Compare(e, "parcours", f->LabelParcours->Caption, QString::number(l->NbCasesParcourues));
       Compare(e, "tours depuis le départ", f->LabelToursDepuisDepart->Caption,
               (l->Situation==svDepart)?QString::number(Tour-l->TourDepart):QString("N/D"));
      }
     else
      if (!f->RadioButtonAucunVehlib->Checked||!f->LabelDirection->isHidden()) e<<QString::fromUtf8("« Aucun vehlib » non affiché");
     return e.join('\n');
    };
   QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_E, Qt::ControlModifier);
   QVERIFY(f->isVisible());
   QVERIFY(frmSimulation->ActionDetailVehlib->Checked);
   QString e=Ecart(cv->TourCrt);
   QVERIFY2(e.isEmpty(), qPrintable(e));
   CaptureFiche(f, "fiche_infos_vehlib.png");
   SuitObjet("vehlib", Ecart, [n](int &x, int &y)
    {
     x=cv->Vehlib[n-1].x; y=cv->Vehlib[n-1].y;
     return (x>=0)&&(y>=0)&&(cv->v[x][y].NumVehlib==n);
    }, 4);
   if (QTest::currentTestFailed()) return;
   Clique(f->ButtonOk);
   QVERIFY(!f->isVisible());
  }
  //-------------------------------------------------------------------------
  // Piéton (Ctrl+P), à pied
  {
   int n=Trouve([](voie *v) { return v->NumPieton[0]?v->NumPieton[0]:v->NumPieton[1]; });
   QVERIFY(n>0);
   TfrmInfosPieton *f=frmInfosPieton;
   auto Ecart=[f, Case, Coordonnees](long Tour) -> QString
    {
     voie *v=Case();
     QStringList e;
     Compare(e, "coordonnées", f->LabelCoordonnees->Caption, Coordonnees());
     int Num=v->NumPieton[0]?v->NumPieton[0]:v->NumPieton[1];
     if (Num>0)
      {
       pieton *p=&cv->Pieton[Num-1];
       const char *Positions[2]={"Entrée de la case", "Sortie de la case"};
       if (!f->RadioButtonPietonNumero->Checked||f->LabelDirection->isHidden()) e<<QString::fromUtf8("piéton non affiché");
       Compare(e, "cadre", f->GroupBoxPieton->Caption, Cadre(Num, p->x, p->y));
       Compare(e, "direction", f->LabelDirection->Caption, Q(asDir[p->Dir]));
       Compare(e, "parcours", f->LabelParcours->Caption, QString::number(p->DemiCasesParcourues/2.0, 'f', 1).replace('.', ','));
       Compare(e, "position", f->LabelPosition->Caption, QString::fromUtf8(Positions[!!p->DemiCase]));
       Compare(e, "véhicule", f->LabelBusTramTaxiVehlib->Caption, "Aucun");
      }
     else
      if (!f->RadioButtonAucunPieton->Checked||!f->LabelDirection->isHidden()) e<<QString::fromUtf8("« Aucun piéton » non affiché");
     return e.join('\n');
    };
   QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_P, Qt::ControlModifier);
   QVERIFY(f->isVisible());
   QVERIFY(frmSimulation->ActionDetailPieton->Checked);
   QString e=Ecart(cv->TourCrt);
   QVERIFY2(e.isEmpty(), qPrintable(e));
   CaptureFiche(f, "fiche_infos_pieton.png");
   SuitObjet("piéton", Ecart, [n](int &x, int &y)
    {
     pieton *p=&cv->Pieton[n-1];
     x=p->x; y=p->y;
     return (x>=0)&&(y>=0)&&((cv->v[x][y].NumPieton[0]==n)||(cv->v[x][y].NumPieton[1]==n));
    }, 6);
   if (QTest::currentTestFailed()) return;
   Clique(f->ButtonOk);
   QVERIFY(!f->isVisible());
  }
  //-------------------------------------------------------------------------
  // Arrêt : la fiche ouverte garde son contenu (aucun rafraîchissement dans
  // l'original), puis le clic suivant sur la carte rechargée la vide
  int n=Trouve([](voie *v) { return (v->NumVehicule>0)&&!v->NumParking?v->NumVehicule:0; });
  QVERIFY(n>0);
  int x=frmSimulation->DrawGridSimul->Col, y=frmSimulation->DrawGridSimul->Row;
  QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_V, Qt::ControlModifier);
  QVERIFY(frmInfosVehicule->isVisible());
  QVERIFY(frmInfosVehicule->RadioButtonVehiculeNumero->Checked);
  QVERIFY(Arrete());
  QVERIFY(frmInfosVehicule->isVisible());
  QVERIFY(frmSimulation->ActionDetailVehicule->Checked);
  QVERIFY(frmInfosVehicule->RadioButtonVehiculeNumero->Checked);
  QVERIFY(!cv->v[x][y].NumVehicule);
  Focalise();
  CliqueCase(x, y);
  QVERIFY(frmInfosVehicule->RadioButtonAucunVehicule->Checked);
  QVERIFY(frmInfosVehicule->LabelDirection->isHidden());
  QCOMPARE(Q(frmInfosVehicule->GroupBoxVehicule->Caption), QString(27, ' '));
  CaptureFiche(frmInfosVehicule, "fiche_infos_vehicule_arret.png");
  Clique(frmInfosVehicule->ButtonOk);
  QVERIFY(!frmInfosVehicule->isVisible());
 }
//---------------------------------------------------------------------------
// Lot 3 des fiches secondaires : statistiques et recherche
//---------------------------------------------------------------------------
namespace
 {
  // Pourcentage affiché par FormatFloat("###.##") : vide pour 0, virgule décimale
  bool PourcentageEgal(const AnsiString &asTexte, double Attendu)
   {
    QString t=Q(asTexte);
    bool Ok=true;
    double v=t.isEmpty()?0.0:t.replace(',', '.').toDouble(&Ok);
    return Ok&&(qAbs(v-Attendu)<0.0051);
   }
  // Le pixel logique (x, y) d'une capture, ou l'un de ses voisins à Rayon près,
  // a la couleur attendue (HiDPI : pixel du milieu du pixel logique)
  bool CouleurAutour(const QImage &Image, qreal Echelle, int x, int y, TColor Couleur, int Rayon)
   {
    QRgb c=CouleurQt(Couleur).rgb();
    for(int i=-Rayon; i<=Rayon; i++)
     for(int j=-Rayon; j<=Rayon; j++)
      {
       QPoint p((int)((x+i+0.5)*Echelle), (int)((y+j+0.5)*Echelle));
       if (Image.rect().contains(p)&&(Image.pixel(p)==c)) return true;
      }
    return false;
   }
  // Dessins d'un contrôle (QEvent::Paint), pour compter les rafraîchissements
  class CompteurDessins : public QObject
   {
    public:
    int n=0;
    bool eventFilter(QObject *, QEvent *e) override
     {
      if (e->type()==QEvent::Paint) n++;
      return false;
     }
   };
  //-------------------------------------------------------------------------
  // Statistiques de fluidité et de trafic attendues pour les trajets ed -> ea
  // (cases de route hors parkings, comme RecalculeEtVisualise)
  struct TotauxFluidite
   {
    int m[2]={0, 0}, t[2]={0, 0}, n=0;
   };
  TotauxFluidite CalculeFluidite(int ed, int ea)
   {
    TotauxFluidite r;
    for(int x=0; x<cv->NbX; x++)
     for(int y=0; y<cv->NbY; y++)
      {
       voie *v=&cv->v[x][y];
       if ((v->DirPoss<=aucune)||v->NumParking) continue;
       r.n++;
       if (!v->Attente||!v->Trafic) continue;
       r.m[0]=qMax(r.m[0], v->Attente[ed][ea]);
       r.t[0]+=v->Attente[ed][ea];
       r.m[1]=qMax(r.m[1], v->Trafic[ed][ea]);
       r.t[1]+=v->Trafic[ed][ea];
      }
    return r;
   }
  // Couleur d'une case : du vert (0) au rouge (maximum) en passant par le jaune
  TColor CouleurFluidite(const TotauxFluidite &r, int ts, int ed, int ea, voie *v)
   {
    if (!r.m[ts]||!v->Trafic||!v->Attente) return 0;
    int s=(127*(ts?v->Trafic[ed][ea]:v->Attente[ed][ea]))/r.m[ts],
        Rouge=(s<64)?4*s:252, Vert=(s>=64)?252-4*(s-64):252;
    return Rouge|(Vert<<8);
   }
 }
//---------------------------------------------------------------------------
void TestInterface::ficheComposition()
 { // Géant (routes, voies de tram, arrêts, places de chaque sorte) : la fiche
   // modale, ouverte par la barre d'outils puis par le raccourci, montre la
   // composition du réseau ; « Vu » et Échap la ferment
  QVERIFY(!OuvreCarte(QString::fromUtf8("Géant.cvw"), "lot3_composition.cvw").isEmpty());
  // Composition attendue, d'après le réseau
  int Total=0, Routes=0, VoiesTram=0, Parkings=0, CasesArretsBus=0, CasesArretsTram=0,
      Dir=0, Nord=0, Est=0, Sud=0, Ouest=0, ArretsBus=0, ArretsTram=0, PlacesTaxi=0, PlacesPark=0;
  int Priorites[2]={0, 0}, Feux[2]={0, 0}, Clp[2]={0, 0}, Stops[2]={0, 0}; // sur route, sur voie de tram
  for(int i=0; i<cv->NbArretsBus; i++) ArretsBus+=cv->ArretBus[i].Defini();
  for(int i=0; i<cv->NbArretsTram; i++) ArretsTram+=cv->ArretTram[i].Defini();
  for(int i=0; i<cv->NbPlacesTaxi; i++) PlacesTaxi+=cv->PlaceTaxi[i].Definie();
  for(int i=0; i<cv->NbPlacesPark; i++) PlacesPark+=cv->PlacePark[i].Definie();
  auto Compte=[&](int Sur, int p)
   {
    Feux[Sur]+=(p==feux);
    Clp[Sur]+=(p==cedez_le_passage);
    Stops[Sur]+=(p==stop);
    Priorites[Sur]+=(p==feux)||(p==cedez_le_passage)||(p==stop);
   };
  for(int x=0; x<cv->NbX; x++)
   for(int y=0; y<cv->NbY; y++)
    {
     voie *v=&cv->v[x][y];
     Total++;
     Parkings+=(v->NumParking>0);
     CasesArretsBus+=(v->NumArretBus>0);
     if (v->DirPoss)
      {
       Routes++;
       Dir+=NbDir[v->DirPoss];
       Nord+=!!(v->DirPoss&N); Est+=!!(v->DirPoss&E); Sud+=!!(v->DirPoss&S); Ouest+=!!(v->DirPoss&O);
       for(int d=nord; d<=ouest; d++)
        if (v->DirPoss&PossDir[d]) Compte(0, v->Priorite[d-1]);
      }
     if (v->SensVoieTram)
      {
       VoiesTram++;
       CasesArretsTram+=(v->NumArretTram>0);
       for(int d=nord; d<=ouest; d++)
        if (v->EstDirPossSensVoieTram(indefinie, (direction)d)) Compte(1, v->Priorite[d-1]);
      }
    }
  QVERIFY(Routes&&Dir&&VoiesTram&&ArretsBus&&ArretsTram&&PlacesTaxi&&PlacesPark&&Priorites[0]);
  QStringList e;
  bool Modale=false;
  auto Entier=[&e](const char *Nom, TLabel *l, int Attendu) { Compare(e, Nom, l->Caption, QString::number(Attendu)); };
  auto PC=[&e](const char *Nom, TLabel *l, double Attendu)
   {
    if (!PourcentageEgal(l->Caption, Attendu))
     e<<QString::fromUtf8("%1 : « %2 » au lieu de %3").arg(QString::fromUtf8(Nom), Q(l->Caption)).arg(Attendu);
   };
  auto Verifie=[&]()
   {
    TfrmStatsComposition *c=frmStatsComposition;
    Modale=(QApplication::activeModalWidget()==c);
    Entier("total", c->LabelTotal, Total);
    Entier("routes", c->LabelRoutes, Routes);
    PC("routes %", c->LabelRoutesPC, 100.0*Routes/Total);
    Entier("priorités", c->LabelPrioritesR, Priorites[0]);
    PC("priorités %", c->LabelPrioritesRPC, 100.0*Priorites[0]/Routes);
    Entier("feux", c->LabelFeuxR, Feux[0]);
    PC("feux %", c->LabelFeuxRPC, 100.0*Feux[0]/Priorites[0]);
    Entier("feux (maximum)", c->LabelFeuxRMax, cv->NbFeux);
    Entier("feux (reste)", c->LabelFeuxRReste, cv->NbFeux-Feux[1]-Feux[0]);
    Entier("cédez-le-passage", c->LabelClpR, Clp[0]);
    PC("cédez-le-passage %", c->LabelClpRPC, 100.0*Clp[0]/Priorites[0]);
    Entier("stops", c->LabelStopsR, Stops[0]);
    PC("stops %", c->LabelStopsRPC, 100.0*Stops[0]/Priorites[0]);
    Entier("parkings", c->LabelParkings, Parkings);
    PC("parkings %", c->LabelParkingsPC, 100.0*Parkings/Routes);
    Entier("parkings (maximum)", c->LabelParkingsMax, cv->NbParkings);
    Entier("parkings (reste)", c->LabelParkingsReste, cv->NbParkings-Parkings);
    Entier("directions", c->LabelDir, Dir);
    PC("directions %", c->LabelDirPC, 100.0*Dir/Routes);
    Entier("nord", c->LabelNord, Nord);
    PC("nord %", c->LabelNordPC, 100.0*Nord/Dir);
    Entier("est", c->LabelEst, Est);
    PC("est %", c->LabelEstPC, 100.0*Est/Dir);
    Entier("sud", c->LabelSud, Sud);
    PC("sud %", c->LabelSudPC, 100.0*Sud/Dir);
    Entier("ouest", c->LabelOuest, Ouest);
    PC("ouest %", c->LabelOuestPC, 100.0*Ouest/Dir);
    Entier("arrêts de bus", c->LabelArretsBus, ArretsBus);
    PC("arrêts de bus %", c->LabelArretsBusPC, 100.0*CasesArretsBus/Routes);
    Entier("arrêts de bus (maximum)", c->LabelArretsBusMax, cv->NbArretsBus);
    Entier("arrêts de bus (reste)", c->LabelArretsBusReste, cv->NbArretsBus-ArretsBus);
    Entier("voies de tram", c->LabelVoiesTram, VoiesTram);
    PC("voies de tram %", c->LabelVoiesTramPC, 100.0*VoiesTram/Total);
    Entier("arrêts de tram", c->LabelArretsTram, ArretsTram);
    PC("arrêts de tram %", c->LabelArretsTramPC, 100.0*CasesArretsTram/VoiesTram);
    Entier("arrêts de tram (maximum)", c->LabelArretsTramMax, cv->NbArretsTram);
    Entier("places de taxi", c->LabelPlacesTaxi, PlacesTaxi);
    PC("places de taxi %", c->LabelPlacesTaxiPC, 100.0*PlacesTaxi/Routes);
    Entier("places de taxi (maximum)", c->LabelPlacesTaxiMax, cv->NbPlacesTaxi);
    Entier("places de taxi (reste)", c->LabelPlacesTaxiReste, cv->NbPlacesTaxi-PlacesTaxi);
    Entier("places de parking", c->LabelPlacesPark, PlacesPark);
    PC("places de parking %", c->LabelPlacesParkPC, 100.0*PlacesPark/Routes);
    Entier("places de parking (maximum)", c->LabelPlacesParkMax, cv->NbPlacesPark);
    Entier("places de parking (reste)", c->LabelPlacesParkReste, cv->NbPlacesPark-PlacesPark);
    Entier("priorités des trams", c->LabelPrioritesV, Priorites[1]);
    PC("priorités des trams %", c->LabelPrioritesVPC, 100.0*Priorites[1]/VoiesTram);
    if (Priorites[1])
     {
      Entier("feux des trams", c->LabelFeuxV, Feux[1]);
      PC("feux des trams %", c->LabelFeuxVPC, 100.0*Feux[1]/Priorites[1]);
      Entier("cédez-le-passage des trams", c->LabelClpV, Clp[1]);
      Entier("stops des trams", c->LabelStopsV, Stops[1]);
     }
   };
  // Barre d'outils, « Vu »
  QuandModale([]() { return frmStatsComposition; }, [&]()
   {
    Verifie();
    CaptureFiche(frmStatsComposition, "fiche_composition.png");
    Clique(frmStatsComposition->ButtonOk);
   });
  Clique(frmSimulation->ToolButtonComposition);
  QVERIFY2(Etapes.empty(), qPrintable(Messages.join('\n')));
  QVERIFY(Modale);
  QVERIFY2(e.isEmpty(), qPrintable(e.join('\n')));
  QVERIFY(!frmStatsComposition); // Release, comme l'original
  // Raccourci Ctrl+Maj+R, Échap (« Vu » est aussi le bouton Cancel)
  Modale=false;
  QuandModale([]() { return frmStatsComposition; }, [&]()
   {
    Verifie();
    Touche(frmStatsComposition->ButtonAide, Qt::Key_Escape);
   });
  Focalise();
  QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_R, Qt::ControlModifier|Qt::ShiftModifier);
  QVERIFY2(Etapes.empty(), qPrintable(Messages.join('\n')));
  QVERIFY(Modale);
  QVERIFY2(e.isEmpty(), qPrintable(e.join('\n')));
  QVERIFY(!frmStatsComposition);
 }
//---------------------------------------------------------------------------
void TestInterface::fichesEvolutionEtVehicules()
 { // Géant en simulation : « Évolution » (non modale, au premier plan),
   // redessinée par le moteur à chaque tour, courbes et quadrillage aux
   // positions que donnent cv->Stats, par nombre puis par pourcentage ;
   // statistiques des véhicules d'une étape à l'autre (non modale), d'après les
   // véhicules arrivés, sur les deux onglets ; les deux se ferment à l'arrêt
  QVERIFY(!OuvreCarte(QString::fromUtf8("Géant.cvw"), "lot3_evolution.cvw").isEmpty());
  Focalise();
  QVERIFY2(Demarre(), qPrintable(Messages.join('\n')));
  QVERIFY(JoueJusquAuTour(2));
  TfrmSimulation *s=frmSimulation;
  // Statistiques des véhicules avant toute arrivée : rien à afficher
  if (!cv->NbVehiculesArrives)
   {
    Messages.clear();
    Clique(s->ToolButtonAffStatsEtapes);
    QCOMPARE(Messages.size(), 1);
    QVERIFY2(Messages.first().contains(QString::fromUtf8("Aucune information n'est disponible.")), qPrintable(Messages.first()));
    QVERIFY(!frmStatsVehicules->isVisible());
    QVERIFY(!s->ActionStatsParkings->Checked);
   }
  //-------------------------------------------------------------------------
  // Évolution (barre d'outils)
  TfrmStatistiques *f=frmStatistiques;
  TPaintBox *p=f->PaintBoxStats;
  Clique(s->ToolButtonEvolution);
  QVERIFY(f->isVisible());
  QVERIFY(QTest::qWaitForWindowExposed(f)); // affichage natif : la fiche n'est dessinée qu'une fois à l'écran
  QVERIFY(s->ActionEvolution->Checked);
  QVERIFY(!f->isModal()&&!QApplication::activeModalWidget());
  QVERIFY(f->windowFlags().testFlag(Qt::WindowStaysOnTopHint));
  QCOMPARE(p->geometry(), f->rect()); // alClient
  // Rafraîchissement par le moteur (PaintBoxStats->Repaint() à chaque tour)
  {
   CompteurDessins Compteur;
   p->installEventFilter(&Compteur);
   long Tour=cv->TourCrt;
   QVERIFY(JoueJusquAuTour(Tour+5));
   p->removeEventFilter(&Compteur);
   QVERIFY2(Compteur.n>=(int)(cv->TourCrt-Tour), qPrintable(QString("%1 dessins en %2 tours").arg(Compteur.n).arg(cv->TourCrt-Tour)));
  }
  QVERIFY(JoueJusquAuTour(qMin(cv->NbToursStats, 60L)));
  // Courbes : points (x_i, y_i) des tours PrmTour..TourCrt-1 (LineTo n'allume
  // pas le point d'arrivée : le dernier point, celui du tour à venir, n'est pas
  // vérifié), tracés dans l'ordre rouge, jaune, vert ; quadrillage dessous
  auto VerifieCourbes=[p](bool Epidemique) -> QString
   {
    QStringList e;
    QImage Image=p->grab().toImage();
    qreal Echelle=p->devicePixelRatioF();
    bool ParNombre=(cv->TypeStats==par_nombre);
    int NbTours=cv->NbToursStats, DrnTour=cv->TourCrt, PrmTour=(DrnTour<NbTours)?0:(DrnTour-NbTours+1),
        NbVehicules=cv->NbVehicules, NbPietons=cv->NbPietons,
        xMax=p->width(), yMax=p->height(), NbPoints=DrnTour-PrmTour;
    if (Q(p->Hint)!=QString::fromUtf8(Epidemique?"Guéris en vert ; Infectés en jaune et Morts en rouge":
                                                 "Sortis en jaune ; Déplacés en vert et Arrivés en rouge"))
     e<<("légende : "+Q(p->Hint));
    if (NbPoints<5) e<<QString("%1 points seulement").arg(NbPoints);
    int Couleurs[3]={clRed, clYellow, clGreen}, Trouves[3]={0, 0, 0}, Visibles[3]={0, 0, 0};
    for(int i=0; i<NbPoints; i++)
     {
      stats &st=cv->Stats[(PrmTour+i)%NbTours];
      int x=xMax*i/NbTours, y[3];
      if (Epidemique)
       {
        y[0]=yMax-yMax*st.NbPietonsMorts/NbPietons;
        y[1]=yMax-yMax*st.NbPietonsInfectes/NbPietons;
        y[2]=yMax-yMax*st.NbPietonsGueris/NbPietons;
       }
      else
       {
        int NbRestants=ParNombre?NbVehicules:(st.NbVehiculesSortis-st.NbVehiculesArrives);
        if (!NbRestants) NbRestants=100;
        y[0]=yMax-yMax*st.NbVehiculesArrives/NbVehicules;
        y[1]=yMax-yMax*st.NbVehiculesSortis/NbVehicules;
        y[2]=yMax-yMax*st.NbVehiculesDeplaces/NbRestants;
       }
      for(int c=0; c<3; c++)
       if ((y[c]>=0)&&(y[c]<yMax))
        {
         Visibles[c]++;
         Trouves[c]+=CouleurAutour(Image, Echelle, x, y[c], Couleurs[c], 1);
        }
     }
    // Vert, tracé en dernier : tous ses points ; rouge et jaune : au moins la
    // moitié (une courbe tracée après peut passer dessus)
    const char *Noms[3]={"rouge", "jaune", "vert"};
    for(int c=0; c<3; c++)
     if (Visibles[c]&&(Trouves[c]<((c==2)?Visibles[c]:(Visibles[c]+1)/2)))
      e<<QString("courbe %1 : %2 points sur %3").arg(Noms[c]).arg(Trouves[c]).arg(Visibles[c]);
    // Quadrillage : carrés de 2 x 2 pixels, gris sauf toutes les dix marques (blanc)
    int dt=1+9*(NbTours>=100), NbMarques=ParNombre?(NbVehicules/10):10, NbCarres=0, CarresTrouves=0;
    for(int i=0; i<NbTours/dt; i++)
     for(int j=0; j<NbMarques; j++)
      {
       int x=dt*xMax*i/NbTours, y=ParNombre?(10*yMax*j/NbVehicules):(10*yMax*j/100);
       if ((x<1)||(yMax-y-1<0)||(yMax-y-1>=yMax)) continue;
       NbCarres++;
       CarresTrouves+=CouleurAutour(Image, Echelle, x-1, yMax-y-1, ((i%10)&&(j%10))?clGray:clWhite, 0);
      }
    if (cv->QuadrillageStats?(CarresTrouves<NbCarres*3/4):(CarresTrouves>0))
     e<<QString("quadrillage %1 : %2 carrés sur %3").arg(cv->QuadrillageStats?"affiché":"masqué").arg(CarresTrouves).arg(NbCarres);
    return e.join('\n');
   };
  if (!cv->QuadrillageStats) s->MenuItemQuadrillageStatistiques->ElementQt()->trigger();
  QVERIFY(cv->QuadrillageStats);
  QVERIFY(s->ActionStatsAffQuadrillage->Checked);
  if (cv->TypeStats!=par_nombre) s->ActionStatsAffParVehicule->Execute();
  QString e=VerifieCourbes(false);
  QVERIFY2(e.isEmpty(), qPrintable(e));
  CaptureFiche(f, "fiche_evolution.png");
  // Par pourcentage (menu Affichage), sans quadrillage
  s->MenuItemParPourcentage->ElementQt()->trigger();
  QCOMPARE((int)cv->TypeStats, (int)par_pourcentage);
  s->MenuItemQuadrillageStatistiques->ElementQt()->trigger();
  QVERIFY(!cv->QuadrillageStats);
  e=VerifieCourbes(false);
  QVERIFY2(e.isEmpty(), qPrintable(e));
  s->MenuItemQuadrillageStatistiques->ElementQt()->trigger();
  // Affichage épidémique : légende et courbes des piétons
  s->ActionAfficherInfection->Execute();
  QVERIFY(f->AffichageEpidemique);
  e=VerifieCourbes(true);
  QVERIFY2(e.isEmpty(), qPrintable(e));
  s->ActionAfficherInfection->Execute();
  QVERIFY(!f->AffichageEpidemique);
  // Fermeture par la case de la fenêtre, réouverture par le menu
  f->close();
  QVERIFY(!f->isVisible());
  QVERIFY(!s->ActionEvolution->Checked);
  s->MenuItemEvolution->ElementQt()->trigger();
  QVERIFY(f->isVisible());
  QVERIFY(s->ActionEvolution->Checked);
  //-------------------------------------------------------------------------
  // Statistiques des véhicules d'une étape à l'autre (menu Statistiques)
  // (attente d'arrivées : deux véhicules d'un même trajet, aux parcours et aux durées différents)
  auto DeuxArrivees=[]()
   {
    for(int i=0; i<cv->NbVehicules; i++)
     for(int j=i+1; j<cv->NbVehicules; j++)
      {
       vehicule *a=&cv->Vehicule[i], *b=&cv->Vehicule[j];
       if (!a->Defini()&&!b->Defini()&&a->NbCasesParcourues&&b->NbCasesParcourues&&
           (a->NumParkingDepart==b->NumParkingDepart)&&(a->NumParkingArrivee==b->NumParkingArrivee)&&
           (a->NumParkingDepart!=a->NumParkingArrivee)&&
           (a->NbCasesParcourues!=b->NbCasesParcourues)&&a->TourDrnDepl&&b->TourDrnDepl&&(a->TourDrnDepl!=b->TourDrnDepl))
        return true;
      }
    return false;
   };
  QVERIFY(JoueTours(DeuxArrivees, 1500)); // graine 1 : sans blocage des vehlibs sur 1500 tours (Demarre)
  TfrmStatsVehicules *g=frmStatsVehicules;
  Messages.clear();
  s->MenuItemStatistiquesEtapes->ElementQt()->trigger();
  QVERIFY2(g->isVisible(), qPrintable(Messages.join('\n')));
  QVERIFY(s->ActionStatsParkings->Checked);
  QVERIFY(!g->isModal()&&!QApplication::activeModalWidget());
  QVERIFY(g->windowFlags().testFlag(Qt::WindowStaysOnTopHint));
  int ne=cv->NbParkings;
  QCOMPARE(Q(g->Caption), QString::fromUtf8("Statistique des véhicules d'une étape à l'autre [Tour %1]").arg(cv->TourCrt));
  QCOMPARE((int)g->StringGridCoordParkings->RowCount, 1+ne);
  QCOMPARE((int)g->StringGridStatsVehicules->RowCount, 1+ne);
  QCOMPARE((int)g->StringGridStatsVehicules->ColCount, 1+ne);
  QCOMPARE(Q(g->StringGridStatsVehicules->Cells[0][0]), QString("A\\D"));
  QCOMPARE(Q(g->StringGridCoordParkings->Cells[1][0]), QString("Position"));
  for(int i=0; i<ne; i++)
   {
    QCOMPARE(Q(g->StringGridCoordParkings->Cells[0][1+i]), QString::asprintf("%3d", 1+i));
    QCOMPARE(Q(g->StringGridCoordParkings->Cells[1][1+i]), QString("%1,%2").arg(cv->Parking[i].x).arg(cv->Parking[i].y));
    QCOMPARE(Q(g->StringGridStatsVehicules->Cells[1+i][0]), QString::asprintf("%3d", 1+i));
   }
  // Écart entre le plus long et le plus court (onglet 0 : cases ; 1 : tours) des
  // véhicules arrivés, pour chaque départ (colonne) et chaque arrivée (ligne)
  auto Ecart=[g](long Tour) -> QString
   {
    QStringList e;
    Compare(e, "titre", g->Caption, QString::fromUtf8("Statistique des véhicules d'une étape à l'autre [Tour %1]").arg(Tour));
    int n=cv->NbParkings, Onglet=g->TabControl->TabIndex, NbNonNuls=0;
    for(int ed=1; ed<=n; ed++)
     for(int ea=1; ea<=n; ea++)
      if (ed!=ea)
       {
        int Max=0, Min=32768, Nb=0;
        for(int i=0; i<cv->NbVehicules; i++)
         {
          vehicule *v=&cv->Vehicule[i];
          int Valeur=Onglet?v->TourDrnDepl:v->NbCasesParcourues;
          if ((v->NumParkingDepart==ed)&&(v->NumParkingArrivee==ea)&&!v->Defini()&&Valeur)
           {
            Max=qMax(Max, Valeur);
            Min=qMin(Min, Valeur);
            Nb++;
           }
         }
        NbNonNuls+=(Nb&&(Max>Min));
        Compare(e, qPrintable(QString("(%1, %2)").arg(ed).arg(ea)), g->StringGridStatsVehicules->Cells[ed][ea], QString::asprintf("%4d", Nb?Max-Min:0));
       }
    if (!NbNonNuls) e<<QString::fromUtf8("aucun écart non nul");
    return e.join('\n');
   };
  QTabBar *Onglets=g->TabControl->findChild<QTabBar *>();
  QVERIFY(Onglets);
  QCOMPARE(Onglets->count(), 2);
  QCOMPARE((int)g->TabControl->TabIndex, 0);
  for(int i=0; i<3; i++)
   {
    JoueTour();
    e=Ecart(cv->TourCrt-1); // rafraîchi pendant le tour, avant TourCrt++
    QVERIFY2(e.isEmpty(), qPrintable(e));
   }
  CaptureFiche(g, "fiche_stats_vehicules.png");
  // Second onglet (clic) : recalcul immédiat sur les tours
  QTest::mouseClick(Onglets, Qt::LeftButton, Qt::KeyboardModifiers(), Onglets->tabRect(1).center());
  QCOMPARE((int)g->TabControl->TabIndex, 1);
  e=Ecart(cv->TourCrt);
  QVERIFY2(e.isEmpty(), qPrintable(e));
  JoueTour();
  e=Ecart(cv->TourCrt-1);
  QVERIFY2(e.isEmpty(), qPrintable(e));
  CaptureFiche(g, "fiche_stats_vehicules_tours.png");
  // « Vu », réouverture par la barre d'outils
  Clique(g->ButtonFermer);
  QVERIFY(!g->isVisible());
  QVERIFY(!s->ActionStatsParkings->Checked);
  Clique(s->ToolButtonAffStatsEtapes);
  QVERIFY(g->isVisible());
  QVERIFY(s->ActionStatsParkings->Checked);
  // Arrêt : les deux fiches se ferment
  QVERIFY(Arrete());
  QVERIFY(!f->isVisible());
  QVERIFY(!s->ActionEvolution->Checked);
  QVERIFY(!g->isVisible());
  QVERIFY(!s->ActionStatsParkings->Checked);
 }
//---------------------------------------------------------------------------
void TestInterface::ficheFluiditeTrafic()
 { // Géant : refusée pendant la simulation ; à l'interruption, le bilan propose
   // de l'afficher (Oui) : fiche modale, carte colorée case par case comme le
   // calcule le moteur (attente puis trafic), totaux, choix des trajets,
   // déplacement du curseur par les boutons et par les touches du menu
   // contextuel de la fiche ; n° d'étape tapé hors bornes (message, cases
   // sans statistique), ramené dans les bornes en quittant le champ ; puis par
   // le raccourci, toujours en pause
  QVERIFY(!OuvreCarte(QString::fromUtf8("Géant.cvw"), "lot3_fluidite.cvw").isEmpty());
  Focalise();
  QVERIFY2(Demarre(), qPrintable(Messages.join('\n')));
  QVERIFY(JoueJusquAuTour(40));
  TfrmSimulation *s=frmSimulation;
  GrilleSimulation *g=s->DrawGridSimul;
  Messages.clear();
  Clique(s->ToolButtonFluiditeTrafic);
  QCOMPARE(Messages.size(), 1);
  QVERIFY2(Messages.first().contains(QString::fromUtf8("Vous devez interrompre ou arrêter la simulation en cours")), qPrintable(Messages.first()));
  QVERIFY(!frmStatsAttenteTrafic);
  // Fiche, totaux et couleurs pour le type de statistiques ts et les trajets ed -> ea
  auto Verifie=[s, g](int ts, int ed, int ea) -> QString
   {
    TfrmStatsAttenteTrafic *f=frmStatsAttenteTrafic;
    QStringList e;
    TotauxFluidite r=CalculeFluidite(ed, ea);
    auto Moyenne=[&e, &r](const char *Nom, TLabel *l, int t)
     { // FormatFloat("0.0", t/n) ; « N/D » sans case
      if (!r.n) { Compare(e, Nom, l->Caption, "N/D"); return; }
      QString Texte=Q(l->Caption);
      bool Ok=false;
      double v=QString(Texte).replace(',', '.').toDouble(&Ok);
      if (!Ok||!Texte.contains(',')||(qAbs(v-1.0*t/r.n)>0.0501)) e<<QString("%1 : %2 au lieu de %3").arg(Nom, Texte).arg(1.0*t/r.n);
     };
    Compare(e, "maximum de la légende", f->LabelMax->Caption, QString::number(r.m[ts]));
    Compare(e, "attente cumulée", f->LabelAttenteCumulee->Caption, QString::number(r.t[0]));
    Moyenne("attente moyenne", f->LabelAttenteMoyenne, r.t[0]);
    Compare(e, "attente maximale", f->LabelAttenteMaximale->Caption, QString::number(r.m[0]));
    Compare(e, "trafic cumulé", f->LabelTraficCumule->Caption, QString::number(r.t[1]));
    Moyenne("trafic moyen", f->LabelTraficMoyen, r.t[1]);
    Compare(e, "trafic maximal", f->LabelTraficMaximal->Caption, QString::number(r.m[1]));
    // CouleurCase de chaque case de route, et son dessin sur la carte (milieu de
    // la case ; hors parkings, dépôts et case courante, dessinés par-dessus)
    QImage Image=g->viewport()->grab().toImage();
    qreal Echelle=g->viewport()->devicePixelRatioF();
    int NbVues=0, NbColorees=0;
    for(int x=0; x<cv->NbX; x++)
     for(int y=0; y<cv->NbY; y++)
      {
       voie *v=&cv->v[x][y];
       if ((v->DirPoss<=aucune)||v->NumParking) continue;
       TColor c=CouleurFluidite(r, ts, ed, ea, v);
       if (f->CouleurCase(x, y)!=c)
        {
         e<<QString("CouleurCase(%1, %2) = %3 au lieu de %4").arg(x).arg(y).arg(f->CouleurCase(x, y), 6, 16).arg(c, 6, 16);
         continue;
        }
       NbColorees+=(c!=0);
       QRect Case=g->CellRect(x, y).Qt();
       if (!g->viewport()->rect().contains(Case)||v->EstDepotBus()||((x==g->Col)&&(y==g->Row))) continue;
       NbVues++;
       QRgb Obtenu=Image.pixel(QPoint((int)((Case.left()+8.5)*Echelle), (int)((Case.top()+8.5)*Echelle)));
       if (Obtenu!=CouleurQt(c).rgb())
        e<<QString("case (%1, %2) dessinée en %3 au lieu de %4").arg(x).arg(y).arg(Obtenu&0xFFFFFF, 6, 16, QChar('0')).arg(CouleurQt(c).rgb()&0xFFFFFF, 6, 16, QChar('0'));
      }
    if (NbVues<100) e<<QString("%1 cases vues seulement").arg(NbVues);
    if (!ed&&!ea&&!NbColorees) e<<QString::fromUtf8("aucune case colorée");
    // Détail de la case courante dans la barre d'état (asMsgDetailCase, au dessin de la case)
    g->Repaint();
    voie *v=&cv->v[g->Col][g->Row];
    QString Detail;
    if (r.m[ts]&&v->Trafic&&v->Attente)
     {
      int Valeur=ts?v->Trafic[ed][ea]:v->Attente[ed][ea];
      Detail=QString::fromUtf8(ts?"Trafic en (%1, %2) au tour %3 : %4 véhicules (Max. %5) soit %6%":
                                   "Attente en (%1, %2) au tour %3 : %4 tours (Max. %5) soit %6%")
              .arg((int)g->Col).arg((int)g->Row).arg(cv->TourCrt).arg(Valeur).arg(r.m[ts])
              .arg(QString::number(100.0*Valeur/r.m[ts], 'f', 3).replace('.', ','));
     }
    Compare(e, "barre d'état", s->StatusBar->Panels->Items[0]->Text, Detail);
    return e.join('\n');
   };
  // Interruption, « Oui » au bilan : fiche modale
  QPoint Depart;
  QString e;
  bool Modale=false;
  QuandModale([]() { return frmStatsAttenteTrafic; }, [&]()
   {
    TfrmStatsAttenteTrafic *f=frmStatsAttenteTrafic;
    Modale=(QApplication::activeModalWidget()==f);
    QStringList Ecarts;
    auto Etape=[&Ecarts](const QString &Nom, const QString &Ecart) { if (!Ecart.isEmpty()) Ecarts<<(Nom+" :\n"+Ecart); };
    Compare(Ecarts, "titre", f->Caption, QString::fromUtf8("Statistiques de fluidité et de trafic au tour n°%1").arg(cv->TourCrt));
    if (f->AvecMenuContextuel::PopupMenu!=f->PopupMenu) Ecarts<<QString::fromUtf8("menu contextuel de la fiche non relié");
    // Curseur sur une case de route où le trafic n'est pas nul, dans la vue
    for(int y=2; (y<cv->NbY-2)&&Depart.isNull(); y++)
     for(int x=2; (x<cv->NbX-2)&&Depart.isNull(); x++)
      {
       voie *v=&cv->v[x][y];
       if ((v->DirPoss>aucune)&&!v->NumParking&&v->Trafic&&v->Trafic[0][0]&&
           g->viewport()->rect().contains(g->CellRect(x+1, y+1).Qt()))
        Depart=QPoint(x, y);
      }
    if (Depart.isNull()) Ecarts<<QString::fromUtf8("aucune case de route avec du trafic");
    g->Col=Depart.x();
    g->Row=Depart.y();
    Etape("attente", Verifie(0, 0, 0));
    CaptureFiche(f, "fiche_fluidite_trafic.png");
    Capture("carte_fluidite_trafic.png");
    // Trafic
    CliqueChoix(f->RadioGroupType, 1);
    Etape("trafic", Verifie(1, 0, 0));
    Capture("carte_trafic.png");
    // Trajets : parking de départ 1 puis 2, puis parking d'arrivée 1
    Clique(f->CheckBoxParkingDepart);
    if (!f->CSpinEditParkingDepart->isEnabled()) Ecarts<<QString::fromUtf8("parking de départ non modifiable");
    Etape("départ 1", Verifie(1, 1, 0));
    f->CSpinEditParkingDepart->Value=2;
    Etape("départ 2", Verifie(1, 2, 0));
    Clique(f->CheckBoxParkingArrivee);
    Etape("départ 2, arrivée 1", Verifie(1, 2, 1));
    Clique(f->CheckBoxParkingDepart);
    Clique(f->CheckBoxParkingArrivee);
    if (f->CSpinEditParkingDepart->isEnabled()||f->CSpinEditParkingArrivee->isEnabled()) Ecarts<<QString::fromUtf8("parkings encore modifiables");
    CliqueChoix(f->RadioGroupType, 0);
    Etape("attente (retour)", Verifie(0, 0, 0));
    // Boutons de direction : droite, bas, gauche-haut ; dans les coins : bas-droite, haut-gauche
    auto Curseur=[&Ecarts, g](const char *Nom, int x, int y)
     {
      if ((g->Col!=x)||(g->Row!=y))
       Ecarts<<QString("%1 : curseur en (%2, %3) au lieu de (%4, %5)").arg(Nom).arg((int)g->Col).arg((int)g->Row).arg(x).arg(y);
     };
    int x=Depart.x(), y=Depart.y();
    Clique(f->BitBtn32);
    Curseur("BitBtn32", x+1, y);
    Clique(f->BitBtn23);
    Curseur("BitBtn23", x+1, y+1);
    Clique(f->BitBtn11);
    Curseur("BitBtn11", x, y);
    Etape("détail de la case", Verifie(0, 0, 0));
    Clique(f->CheckBoxCoins);
    Clique(f->BitBtn33);
    Curseur("BitBtn33 (coins)", g->ColCount-1, g->RowCount-1);
    Clique(f->BitBtn11);
    Curseur("BitBtn11 (coins)", 0, 0);
    Clique(f->CheckBoxCoins);
    g->Col=x;
    g->Row=y;
    // Touches du menu contextuel de la fiche (flèches, Origine) depuis n'importe quel contrôle ;
    // « Zoom (-/+) » (Z) n'a plus de bouton depuis la v5.4
    Touche(f->CheckBoxCoins, Qt::Key_Right);
    Curseur("flèche droite", x+1, y);
    Touche(f->CheckBoxCoins, Qt::Key_Down);
    Curseur("flèche bas", x+1, y+1);
    Touche(f->CheckBoxCoins, Qt::Key_Home);
    Curseur("Origine (haut gauche)", x, y);
    Touche(f->CheckBoxCoins, Qt::Key_Z);
    Curseur("Z", x, y);
    // N° d'étape tapé hors de 0..NbParkings (frappe non bornée, comme CSPIN.pas) :
    // « Le n° d'étape est incorrect ! » ; touches de la fiche depuis le champ :
    // case sans statistique pour ce trajet (noire, sans détail) ; en quittant le
    // champ, numéro ramené dans les bornes et statistiques recalculées
    Clique(f->CheckBoxParkingDepart);
    if (!f->CSpinEditParkingDepart->hasFocus()) Ecarts<<QString::fromUtf8("parking de départ sans le focus");
    int NbMessages=Messages.size();
    f->CSpinEditParkingDepart->selectAll();
    QTest::keyClicks(f->CSpinEditParkingDepart, QString::number(10*cv->NbParkings));
    if ((Messages.size()!=NbMessages+1)||!Messages.last().contains(QString::fromUtf8("Le n° d'étape est incorrect !")))
     Ecarts<<QString::fromUtf8("n° d'étape hors bornes : ")+Messages.mid(NbMessages).join(" / ");
    while (Messages.size()>NbMessages) Messages.removeLast(); // vérifié ici
    if (f->CSpinEditParkingDepart->Value!=10*cv->NbParkings) Ecarts<<QString::fromUtf8("frappe bornée");
    f->activateWindow(); // de nouveau active après la boîte de message, le champ ayant le focus
    if (!QTest::qWaitFor([f]() { return QApplication::activeWindow()==f; }, 3000)||!f->CSpinEditParkingDepart->hasFocus())
     Ecarts<<QString::fromUtf8("parking de départ sans le focus après le message");
    Touche(f->CSpinEditParkingDepart, Qt::Key_Right);
    Curseur("flèche droite (n° hors bornes)", x+1, y);
    if (f->CouleurCase(x+1, y)!=0) Ecarts<<QString::fromUtf8("case colorée pour un n° hors bornes");
    g->Repaint(); // détail de la case courante, au dessin de la case
    Compare(Ecarts, "barre d'état (n° hors bornes)", s->StatusBar->Panels->Items[0]->Text, QString());
    QTest::keyClick(f->CSpinEditParkingDepart, Qt::Key_Tab);
    if (f->CSpinEditParkingDepart->hasFocus()) Ecarts<<QString::fromUtf8("parking de départ toujours actif après Tab");
    if (f->CSpinEditParkingDepart->Value!=cv->NbParkings) Ecarts<<QString::fromUtf8("n° d'étape non ramené dans les bornes en quittant le champ");
    Etape("départ hors bornes ramené au dernier", Verifie(0, cv->NbParkings, 0));
    Clique(f->CheckBoxParkingDepart);
    e=Ecarts.join('\n');
    f->close(); // case de la fenêtre
   });
  ReponseQuestions=QMessageBox::Yes;
  Messages.clear();
  Clique(s->ToolButtonPause);
  RelanceMinuteur();
  QVERIFY(QTest::qWaitFor([this]() { return Etapes.empty()&&!frmStatsAttenteTrafic&&frmSimulation->Pause; }, 30000));
  ReponseQuestions=QMessageBox::No;
  QVERIFY2(Modale, qPrintable(Messages.join('\n')));
  QVERIFY2(Messages.size()==1&&Messages.first().contains(QString::fromUtf8("Bilan au tour n°")), qPrintable(Messages.join('\n')));
  QVERIFY2(e.isEmpty(), qPrintable(e));
  // Carte redessinée sans les statistiques (affichage d'avant)
  {
   g->Repaint();
   QImage Image=g->viewport()->grab().toImage();
   QRect Case=g->CellRect(Depart.x(), Depart.y()).Qt();
   TotauxFluidite r=CalculeFluidite(0, 0);
   QVERIFY(Image.pixel(QPoint((int)((Case.left()+8.5)*g->viewport()->devicePixelRatioF()), (int)((Case.top()+8.5)*g->viewport()->devicePixelRatioF())))!=
           CouleurQt(CouleurFluidite(r, 0, 0, 0, &cv->v[Depart.x()][Depart.y()])).rgb());
  }
  // En pause, par le raccourci (Ctrl+Maj+F)
  Modale=false;
  e.clear();
  QuandModale([]() { return frmStatsAttenteTrafic; }, [&]()
   {
    Modale=(QApplication::activeModalWidget()==frmStatsAttenteTrafic);
    e=Verifie(0, 0, 0);
    frmStatsAttenteTrafic->close();
   });
  Focalise();
  QTest::keyClick(g, Qt::Key_F, Qt::ControlModifier|Qt::ShiftModifier);
  QVERIFY2(Etapes.empty(), qPrintable(Messages.join('\n')));
  QVERIFY(Modale);
  QVERIFY2(e.isEmpty(), qPrintable(e));
  QVERIFY(!frmStatsAttenteTrafic);
  // Arrêt désactivé en pause (ActionArret->Enabled=EnCours) : reprise, puis arrêt
  QVERIFY(!s->ActionArret->Enabled);
  QTest::mouseClick(s->ToolButtonMarche, Qt::LeftButton);
  QVERIFY(s->SimulationEnCours);
  QVERIFY(Arrete());
 }
//---------------------------------------------------------------------------
void TestInterface::ficheRecherche()
 { // Géant en simulation. Rechercher (non modale, au premier plan), ouverte par
   // la barre d'outils sur un parking : numéros de la case ; chaque sorte
   // d'élément fixe est retrouvée (curseur, barre d'état de la fiche) ;
   // « Suivre » un véhicule, un piéton, un bus, un tram, un taxi, un vehlib : le
   // curseur suit l'objet tour après tour en clignotant, et la fiche
   // d'informations ouverte le suit aussi, sans clic de l'utilisateur ; hors
   // simulation, FormShow ne borne pas un numéro resté trop grand (MaxValue=
   // sans OnChange), et la frappe hors bornes l'est en quittant le champ
  QVERIFY(!OuvreCarte(QString::fromUtf8("Géant.cvw"), "lot3_recherche.cvw").isEmpty());
  Focalise();
  QVERIFY2(Demarre(), qPrintable(Messages.join('\n')));
  QVERIFY(JoueJusquAuTour(15));
  TfrmSimulation *s=frmSimulation;
  TfrmRecherche *r=frmRecherche;
  GrilleSimulation *g=s->DrawGridSimul;
  // Ouverture sur un parking : FormShow reprend la case et ses numéros
  int NumParking=TrouveCase([](voie *v) { return v->NumParking; });
  QVERIFY(NumParking>0);
  int x0=g->Col, y0=g->Row;
  Clique(s->ToolButtonRechercher);
  QVERIFY(r->isVisible());
  QVERIFY(s->ActionRechercher->Checked);
  QVERIFY(!r->isModal()&&!QApplication::activeModalWidget());
  QVERIFY(r->windowFlags().testFlag(Qt::WindowStaysOnTopHint));
  QCOMPARE((int)r->CSpinEditX->Value, x0);
  QCOMPARE((int)r->CSpinEditY->Value, y0);
  QCOMPARE((int)r->CSpinEditParking->Value, NumParking);
  QCOMPARE((int)r->CSpinEditParking->MaxValue, cv->NbParkings);
  QCOMPARE((int)r->CSpinEditVehicule->MaxValue, cv->NbVehicules);
  QCOMPARE((int)r->CSpinEditPieton->MaxValue, cv->NbPietons);
  QCOMPARE((int)r->CSpinEditVehlib->MaxValue, cv->NbVehlibs);
  QCOMPARE(g->Col, x0);
  QCOMPARE(g->Row, y0);
  // Curseur clignotant (TimerClignotementLocalisation, 250 ms)
  QVERIFY(s->TimerClignotementLocalisation->Enabled);
  int Tag=s->TimerClignotementLocalisation->Tag;
  QVERIFY(QTest::qWaitFor([Tag]() { return frmSimulation->TimerClignotementLocalisation->Tag!=Tag; }, 2000));
  //-------------------------------------------------------------------------
  // Éléments fixes : bouton radio, puis numéro
  struct Element
   {
    const char *Nom;
    TRadioButton *Radio;
    TCSpinEdit *Numero;
    int Nb;
    std::function<QString(int, int &, int &)> Attendu; // texte attendu, position (x<0 : curseur inchangé)
   };
  auto Trouve=[](const char *Format, int k, int x, int y) { return QString::fromUtf8(Format).arg(k).arg(x).arg(y); };
  std::vector<Element> Elements=
   {
    {"parking", r->RadioButtonParking, r->CSpinEditParking, cv->NbParkings, [Trouve](int k, int &x, int &y)
      {
       parking *p=cv->Parking+k-1;
       if (!p->Defini()) return QString::fromUtf8("Parking n°%1 non utilisé").arg(k);
       x=p->x; y=p->y;
       return Trouve("Parking n°%1 trouvé en (%2, %3)", k, x, y);
      }},
    {"feu", r->RadioButtonFeu, r->CSpinEditFeu, cv->NbFeux, [Trouve](int k, int &x, int &y)
      {
       feu *f=cv->Feu+k-1;
       if (!f->Defini()) return QString::fromUtf8("Feu n°%1 non utilisé").arg(k);
       x=f->x; y=f->y;
       return Trouve("Feu n°%1 trouvé en (%2, %3)", k, x, y);
      }},
    {"arrêt de bus", r->RadioButtonArretBus, r->CSpinEditArretBus, cv->NbArretsBus, [Trouve](int k, int &x, int &y)
      {
       arret_bus *a=cv->ArretBus+k-1;
       if (!a->Defini()) return QString::fromUtf8("Arrêt de bus n°%1 non utilisé").arg(k);
       x=a->x; y=a->y;
       return Trouve("Arrêt de bus n°%1 trouvé en (%2, %3)", k, x, y);
      }},
    {"arrêt de tram", r->RadioButtonArretTram, r->CSpinEditArretTram, cv->NbArretsTram, [Trouve](int k, int &x, int &y)
      {
       arret_tram *a=cv->ArretTram+k-1;
       if (!a->Defini()) return QString::fromUtf8("Arrêt de tram n°%1 non utilisé").arg(k);
       x=a->x; y=a->y;
       return Trouve("Arrêt de tram n°%1 trouvé en (%2, %3)", k, x, y);
      }},
    {"place de taxi", r->RadioButtonPlaceTaxi, r->CSpinEditPlaceTaxi, cv->NbPlacesTaxi, [Trouve](int k, int &x, int &y)
      {
       place_taxi *p=cv->PlaceTaxi+k-1;
       if (!p->Definie()) return QString(); // texte inchangé (code d'origine)
       x=p->x; y=p->y;
       return Trouve("Place de taxi n°%1 trouvée en (%2, %3)", k, x, y);
      }},
    {"file de taxis", r->RadioButtonFileTaxi, r->CSpinEditFileTaxi, cv->NbFilesTaxi, [Trouve](int k, int &x, int &y)
      {
       place_taxi *p=cv->PlaceTaxi+cv->FileTaxi[k-1].NumPlaceQueue;
       x=p->x; y=p->y;
       return Trouve("file de taxis n°%1 trouvée en (%2, %3)", k, x, y);
      }},
    {"place de parking", r->RadioButtonPlacePark, r->CSpinEditPlacePark, cv->NbPlacesPark, [Trouve](int k, int &x, int &y)
      {
       place_park *p=cv->PlacePark+k-1;
       if (!p->Definie()) return QString();
       x=p->x; y=p->y;
       return Trouve("Place de parking n°%1 trouvée en (%2, %3)", k, x, y);
      }},
    {"file de parkings", r->RadioButtonFilePark, r->CSpinEditFilePark, cv->NbFilesPark, [Trouve](int k, int &x, int &y)
      {
       place_park *p=cv->PlacePark+cv->FilePark[k-1].NumPlaceQueue;
       x=p->x; y=p->y;
       return Trouve("file de parkings n°%1 trouvée en (%2, %3)", k, x, y);
      }},
    {"place de vehlib", r->RadioButtonPlaceVehlib, r->CSpinEditPlaceVehlib, cv->NbPlacesVehlib, [Trouve](int k, int &x, int &y)
      {
       place_vehlib *p=cv->PlaceVehlib+k-1;
       if (!p->Definie()) return QString();
       x=p->x; y=p->y;
       return Trouve("Place de vehlib n°%1 trouvée en (%2, %3)", k, x, y);
      }},
   };
  for(const Element &el : Elements)
   {
    QVERIFY2(el.Nb>0, el.Nom);
    std::vector<int> Numeros={el.Nb, 1, (el.Nb+1)/2};
    for(std::size_t i=0; i<Numeros.size(); i++)
     {
      int k=Numeros[i], x=-1, y=-1;
      QString Avant=Q(r->StatusBar->SimpleText);
      int ColAvant=g->Col, RowAvant=g->Row;
      if (!i)
       { // Le bouton radio active le numéro et recherche l'élément du numéro affiché
        el.Numero->Value=k;
        Clique(el.Radio);
        QVERIFY2(el.Radio->Checked&&el.Numero->isEnabled()&&!r->CSpinEditX->isEnabled(), el.Nom);
       }
      else
       el.Numero->Value=k;
      QString Texte=el.Attendu(k, x, y);
      if (Texte.isEmpty()) { Texte=Avant; x=ColAvant; y=RowAvant; }
      QCOMPARE(Q(r->StatusBar->SimpleText), Texte);
      if (x>=0) { QCOMPARE(g->Col, x); QCOMPARE(g->Row, y); }
     }
   }
  // Numéro hors des éléments définis (MaxValue suit la carte) : sans effet sur le curseur
  // Coordonnées
  Clique(r->RadioButtonCoordonnees);
  QVERIFY(r->CSpinEditX->isEnabled()&&r->CSpinEditY->isEnabled()&&!r->CSpinEditVehlib->isEnabled());
  QCOMPARE(g->Col, (int)r->CSpinEditX->Value);
  QCOMPARE(g->Row, (int)r->CSpinEditY->Value);
  QCOMPARE(Q(r->StatusBar->SimpleText), QString());
  r->CSpinEditX->Value=7;
  r->CSpinEditY->Value=5;
  QCOMPARE(g->Col, 7);
  QCOMPARE(g->Row, 5);
  // Clignotement : sur un immeuble (rien n'y bouge), le curseur n'est dessiné
  // qu'une fois sur deux (Tag du minuteur)
  QPoint Vide;
  for(int y=1; (y<20)&&Vide.isNull(); y++)
   for(int x=1; (x<30)&&Vide.isNull(); x++)
    {
     voie *v=&cv->v[x][y];
     if (!v->DirPoss&&!v->SensVoieTram&&!v->NumParking&&!v->NbPietons&&(NATURE(v)==immeubles)) Vide=QPoint(x, y);
    }
  QVERIFY(!Vide.isNull());
  r->CSpinEditX->Value=Vide.x();
  r->CSpinEditY->Value=Vide.y();
  QVERIFY(g->viewport()->rect().contains(g->CellRect(Vide.x(), Vide.y()).Qt()));
  auto CaseCourante=[g]() { return g->viewport()->grab(g->CellRect(g->Col, g->Row).Qt()).toImage(); };
  QVERIFY(QTest::qWaitFor([]() { return frmSimulation->TimerClignotementLocalisation->Tag==1; }, 2000));
  QImage Allume=CaseCourante();
  QVERIFY(QTest::qWaitFor([]() { return frmSimulation->TimerClignotementLocalisation->Tag==0; }, 2000));
  QImage Eteint=CaseCourante();
  QVERIFY(Allume!=Eteint);
  QVERIFY(QTest::qWaitFor([]() { return frmSimulation->TimerClignotementLocalisation->Tag==1; }, 2000));
  QVERIFY(CaseCourante()==Allume);
  //-------------------------------------------------------------------------
  // Objets mobiles : « Suivre » sur l'objet de la case (raccourci ou menu), puis
  // fiche d'informations ; chaque tour, curseur et fiche sur l'objet suivi
  struct Mobile
   {
    const char *Nom;
    std::function<int(voie *)> Numero; // objet de la case
    std::function<void()> Suivre; // « Suivre... » (case courante)
    TRadioButton *Radio;
    TCSpinEdit *Spin;
    std::function<bool(int, int &, int &, QString &)> Attendu; // position (faux : hors de la carte) et texte attendus
    TForm *Infos;
    std::function<void()> OuvreInfos;
    std::function<QString(int, int, int)> EcartInfos; // la fiche montre l'objet n en (x, y)
    int NbTours;
    const char *Capture;
   };
  auto Touches=[](Qt::Key Touche, Qt::KeyboardModifiers m) { return [Touche, m]() { QTest::keyClick(frmSimulation->DrawGridSimul, Touche, m); }; };
  auto Groupe=[](TGroupBox *b, int n, int x, int y) -> QString
   {
    QStringList e;
    Compare(e, "cadre", b->Caption, Cadre(n, x, y));
    return e.join('\n');
   };
  std::vector<Mobile> Mobiles=
   {
    {"véhicule", [](voie *v) { return (v->NumVehicule>0)&&!v->NumParking?v->NumVehicule:0; },
     Touches(Qt::Key_S, Qt::ControlModifier), r->RadioButtonVehicule, r->CSpinEditVehicule,
     [](int n, int &x, int &y, QString &t)
      {
       vehicule *v=cv->Vehicule+n-1;
       if (!v->Defini()) { t=QString::fromUtf8("Véhicule n°%1 non utilisé").arg(n); return false; }
       x=v->x; y=v->y;
       t=QString::fromUtf8("Véhicule n°%1 trouvé en (%2, %3)").arg(n).arg(x).arg(y);
       return cv->v[x][y].NumVehicule==n;
      },
     frmInfosVehicule, Touches(Qt::Key_V, Qt::ControlModifier),
     [Groupe](int n, int x, int y) { return Groupe(frmInfosVehicule->GroupBoxVehicule, n, x, y); }, 8, nullptr},
    {"piéton", [](voie *v) { return v->NumPieton[0]; },
     Touches(Qt::Key_L, Qt::ControlModifier), r->RadioButtonPieton, r->CSpinEditPieton,
     [](int n, int &x, int &y, QString &t)
      { // Dans un taxi ou un vehlib, le texte (et, pour un vehlib, la position) vient de cv->Bus (code d'origine)
       pieton *p=cv->Pieton+n-1;
       if (p->Defini())
        {
         x=p->x; y=p->y;
         t=QString::fromUtf8("Piéton n°%1 trouvé en (%2, %3)").arg(n).arg(x).arg(y);
         return (cv->v[x][y].NumPieton[0]==n)||(cv->v[x][y].NumPieton[1]==n);
        }
       if (!p->NumBus&&!p->NumTaxi&&!p->NumVehlib) { t=QString::fromUtf8("Piéton n°%1 non utilisé").arg(n); return false; }
       auto Dans=[&t, n](const char *Quoi, int Num, int xb, int yb)
        { t=QString::fromUtf8("Piéton n°%1 trouvé dans le %2 n°%3 (%4, %5)").arg(n).arg(Quoi).arg(Num).arg(xb).arg(yb); };
       if (p->NumBus) { x=cv->Bus[p->NumBus-1].x; y=cv->Bus[p->NumBus-1].y; Dans("bus", p->NumBus, x, y); }
       if (p->NumTaxi)
        {
         x=cv->Taxi[p->NumTaxi-1].x; y=cv->Taxi[p->NumTaxi-1].y;
         if (p->NumTaxi<=cv->NbBus) Dans("taxi", p->NumTaxi, cv->Bus[p->NumTaxi-1].x, cv->Bus[p->NumTaxi-1].y);
         else t.clear(); // lecture hors de cv->Bus dans l'original : texte non vérifié
        }
       if (p->NumVehlib)
        {
         if (p->NumVehlib>cv->NbBus) { t.clear(); return false; }
         x=cv->Bus[p->NumVehlib-1].x; y=cv->Bus[p->NumVehlib-1].y;
         Dans("vehlib", p->NumVehlib, x, y);
        }
       return false; // fiche d'informations non vérifiée (cadre en coordonnées de cv->Bus, lot 2)
      },
     frmInfosPieton, Touches(Qt::Key_P, Qt::ControlModifier),
     [Groupe](int n, int x, int y) { return Groupe(frmInfosPieton->GroupBoxPieton, n, x, y); }, 8, nullptr},
    {"bus", [](voie *v) { return v->NumBus; },
     Touches(Qt::Key_W, Qt::ControlModifier), r->RadioButtonBus, r->CSpinEditBus,
     [](int n, int &x, int &y, QString &t)
      {
       bus *b=cv->Bus+n-1;
       if (!b->Defini()) { t=QString::fromUtf8("Bus n°%1 au dépôt").arg(n); return false; }
       x=b->x; y=b->y;
       t=QString::fromUtf8("Bus n°%1 trouvé en (%2, %3)").arg(n).arg(x).arg(y);
       return cv->v[x][y].NumBus==n;
      },
     frmInfosBus, Touches(Qt::Key_B, Qt::ControlModifier),
     [Groupe](int n, int x, int y) { return Groupe(frmInfosBus->GroupBoxBus, n, x, y); }, 6, nullptr},
    {"tram", [](voie *v) { return v->NumTram; },
     Touches(Qt::Key_X, Qt::ControlModifier), r->RadioButtonTram, r->CSpinEditTram,
     [](int n, int &x, int &y, QString &t)
      {
       tram *m=cv->Tram+n-1;
       if (!m->Defini()) { t=QString::fromUtf8("Tram n°%1 non utilisé").arg(n); return false; }
       x=m->x; y=m->y;
       t=QString::fromUtf8("Tram n°%1 trouvé en (%2, %3)").arg(n).arg(x).arg(y);
       return cv->v[x][y].NumTram==n;
      },
     frmInfosTram, [s]() { s->MenuItemDetailTram->ElementQt()->trigger(); },
     [Groupe](int n, int x, int y) { return Groupe(frmInfosTram->GroupBoxTram, n, x, y); }, 6, nullptr},
    {"taxi", [](voie *v) { return v->NumTaxi; },
     Touches(Qt::Key_Y, Qt::ControlModifier), r->RadioButtonTaxi, r->CSpinEditTaxi,
     [](int n, int &x, int &y, QString &t)
      {
       taxi *m=cv->Taxi+n-1;
       x=m->x; y=m->y;
       t=QString::fromUtf8("Taxi n°%1 trouvé en (%2, %3)").arg(n).arg(x).arg(y);
       return (x>=0)&&(y>=0)&&(cv->v[x][y].NumTaxi==n);
      },
     frmInfosTaxi, Touches(Qt::Key_I, Qt::ControlModifier),
     [Groupe](int n, int x, int y) { return Groupe(frmInfosTaxi->GroupBoxTaxi, n, x, y); }, 6, nullptr},
    {"vehlib", [](voie *v) { return v->NumVehlib; },
     Touches(Qt::Key_E, Qt::ControlModifier|Qt::ShiftModifier), // menu contextuel de la carte, avant Statistiques | Parkings (même raccourci)
     r->RadioButtonVehlib, r->CSpinEditVehlib,
     [](int n, int &x, int &y, QString &t)
      {
       vehlib *m=cv->Vehlib+n-1;
       x=m->x; y=m->y;
       t=QString::fromUtf8("Vehlib n°%1 trouvé en (%2, %3)").arg(n).arg(x).arg(y);
       return (x>=0)&&(y>=0)&&(cv->v[x][y].NumVehlib==n);
      },
     frmInfosVehlib, Touches(Qt::Key_E, Qt::ControlModifier),
     [Groupe](int n, int x, int y) { return Groupe(frmInfosVehlib->GroupBoxVehlib, n, x, y); }, 6, "fiche_recherche.png"},
   };
  for(const Mobile &m : Mobiles)
   {
    // Plus d'objet suivi (coordonnées), puis « Suivre » sur un objet de la carte
    Clique(r->RadioButtonCoordonnees);
    int n=TrouveCase(m.Numero);
    QVERIFY2(n>0, m.Nom);
    m.Suivre();
    QVERIFY2(m.Radio->Checked, m.Nom);
    QCOMPARE((int)m.Spin->Value, n);
    QVERIFY(r->isVisible());
    QVERIFY(m.Spin->isEnabled()&&!r->CSpinEditX->isEnabled());
    // Fiche d'informations ouverte sur l'objet suivi
    Focalise();
    m.OuvreInfos();
    QVERIFY2(m.Infos->isVisible(), m.Nom);
    int NbVerifies=0;
    for(int i=0; i<m.NbTours; i++)
     {
      JoueTour();
      int x=-1, y=-1;
      QString Texte;
      bool SurLaCarte=m.Attendu(n, x, y, Texte);
      QString Contexte=QString("%1 n°%2, tour %3").arg(QString::fromUtf8(m.Nom)).arg(n).arg(cv->TourCrt);
      if (!Texte.isEmpty()) QVERIFY2(Q(r->StatusBar->SimpleText)==Texte, qPrintable(Contexte+" : « "+Q(r->StatusBar->SimpleText)+" » au lieu de « "+Texte+" »"));
      if (x>=0)
       QVERIFY2((g->Col==x)&&(g->Row==y), qPrintable(Contexte+QString(" : curseur en (%1, %2) au lieu de (%3, %4)").arg((int)g->Col).arg((int)g->Row).arg(x).arg(y)));
      if (!SurLaCarte) continue;
      QString e=m.EcartInfos(n, x, y);
      QVERIFY2(e.isEmpty(), qPrintable(Contexte+" (fiche d'informations) :\n"+e));
      NbVerifies++;
     }
    QVERIFY2(NbVerifies>0, m.Nom);
    if (m.Capture) CaptureFiche(r, m.Capture);
    Focalise();
    m.Infos->Close();
    QVERIFY(!m.Infos->isVisible());
   }
  // « Vu » (bouton Cancel : Échap) : fin du clignotement
  Touche(r->ButtonAide, Qt::Key_Escape);
  QVERIFY(!r->isVisible());
  QVERIFY(!s->ActionRechercher->Checked);
  QVERIFY(!s->TimerClignotementLocalisation->Enabled);
  // Suivre un véhicule rouvre la fiche
  int n=TrouveCase([](voie *v) { return (v->NumVehicule>0)&&!v->NumParking?v->NumVehicule:0; });
  QVERIFY(n>0);
  s->MenuItemSuivreCeVehicule->ElementQt()->trigger();
  QVERIFY(r->RadioButtonVehicule->Checked);
  QCOMPARE((int)r->CSpinEditVehicule->Value, n);
  QVERIFY(r->isVisible());
  QVERIFY(s->ActionRechercher->Checked);
  Clique(r->ButtonAnnuler);
  QVERIFY(!r->isVisible());
  QVERIFY(!s->ActionRechercher->Checked);
  QVERIFY(Arrete());
  //-------------------------------------------------------------------------
  // Hors simulation (curseur immobile) : numéro resté au-dessus du nombre
  // d'éléments (carte précédente plus grande) ; FormShow règle MaxValue
  // d'après la carte sans le ramener dans les bornes ni déclencher OnChange,
  // la case courante ne bouge pas. Frappe hors bornes acceptée (« n°
  // incorrect » à chaque chiffre), ramenée dans les bornes en quittant le champ
  int NbParkings=cv->NbParkings;
  r->CSpinEditParking->MaxValue=NbParkings+10;
  r->CSpinEditParking->Value=NbParkings+5;
  QVERIFY(TrouveCase([](voie *v) { return (v->DirPoss>aucune)&&!v->NumParking?1:0; }));
  int xc=g->Col, yc=g->Row;
  int NbChangeParking=0;
  TNotifyEvent ChangeParking=r->CSpinEditParking->OnChange;
  r->CSpinEditParking->OnChange=[&NbChangeParking, ChangeParking](TObject *Sender) { NbChangeParking++; ChangeParking(Sender); };
  s->ActionRechercher->Execute();
  QVERIFY(r->isVisible());
  QCOMPARE((int)r->CSpinEditParking->MaxValue, NbParkings);
  QCOMPARE((int)r->CSpinEditParking->Value, NbParkings+5);
  QCOMPARE(NbChangeParking, 0);
  QCOMPARE(g->Col, xc);
  QCOMPARE(g->Row, yc);
  // Fiche active (qWaitForWindowActive ne suffit pas : une fenêtre outil
  // partage l'activation de la fenêtre principale), pour le focus de ses contrôles
  r->activateWindow();
  QVERIFY(QTest::qWaitFor([r]() { return QApplication::activeWindow()==r; }, 3000));
  Clique(r->RadioButtonParking);
  QVERIFY(r->CSpinEditParking->hasFocus());
  QCOMPARE(Q(r->StatusBar->SimpleText), QString::fromUtf8("Aucune parking défini ou n° incorrect"));
  QCOMPARE(g->Col, xc);
  QCOMPARE(g->Row, yc);
  QString Frappe=QString::number(100*NbParkings);
  r->CSpinEditParking->selectAll();
  NbChangeParking=0;
  QTest::keyClicks(r->CSpinEditParking, Frappe);
  QCOMPARE((int)r->CSpinEditParking->Value, 100*NbParkings);
  QCOMPARE(NbChangeParking, (int)Frappe.size());
  QCOMPARE(Q(r->StatusBar->SimpleText), QString::fromUtf8("Aucune parking défini ou n° incorrect"));
  QTest::keyClick(r->CSpinEditParking, Qt::Key_Tab);
  QVERIFY(!r->CSpinEditParking->hasFocus());
  QCOMPARE((int)r->CSpinEditParking->Value, NbParkings);
  QCOMPARE(NbChangeParking, (int)Frappe.size()+1);
  parking *Dernier=cv->Parking+NbParkings-1;
  QVERIFY(Dernier->Defini());
  QCOMPARE(Q(r->StatusBar->SimpleText), QString::fromUtf8("Parking n°%1 trouvé en (%2, %3)").arg(NbParkings).arg(Dernier->x).arg(Dernier->y));
  QCOMPARE(g->Col, Dernier->x);
  QCOMPARE(g->Row, Dernier->y);
  r->CSpinEditParking->OnChange=ChangeParking;
  Clique(r->ButtonAnnuler);
  QVERIFY(!r->isVisible());
 }
//---------------------------------------------------------------------------
void TestInterface::controleCSpinEdit()
 { // TCSpinEdit comme CSPIN.pas, sur un champ seul : MinValue et MaxValue sont
   // de simples champs (ni le texte ni OnChange ne bougent) ; Value=... est
   // borné si MinValue<>MaxValue, et OnChange suit le texte ; la frappe n'est
   // pas bornée (chiffres et signes), mais l'est en quittant le champ, pas
   // quand la fenêtre perd l'activation ; texte vide : Value vaut MinValue, et
   // le texte reste vide ; flèches : Value±Increment, borné ; sans bornes si
   // MinValue=MaxValue ; EditorEnabled=False : flèches seulement ; texte
   // sélectionné en entrant dans le champ (sauf au clic)
  QWidget Fiche;
  TCSpinEdit *s=new TCSpinEdit(&Fiche);
  TButton *b=new TButton(&Fiche);
  s->setGeometry(8, 8, 60, 22);
  b->setGeometry(8, 40, 75, 25);
  QLineEdit *Texte=s->findChild<QLineEdit *>();
  QVERIFY(Texte);
  QVERIFY(b->focusPolicy()&Qt::ClickFocus); // le clic donne le focus (sous macOS aussi)
  int NbChange=0;
  s->OnChange=[&NbChange](TObject *) { NbChange++; };
  QCOMPARE(Texte->text(), QString("0")); // Text:='0' du constructeur
  // MinValue= et MaxValue= : sans OnChange, valeur laissée hors des bornes
  s->MinValue=1;
  s->MaxValue=10;
  QCOMPARE(Texte->text(), QString("0"));
  QCOMPARE((int)s->Value, 0);
  s->Value=5;
  QCOMPARE(Texte->text(), QString("5"));
  QCOMPARE(NbChange, 1);
  s->MaxValue=3;
  QCOMPARE((int)s->Value, 5);
  QCOMPARE(Texte->text(), QString("5"));
  QCOMPARE(NbChange, 1);
  // Value=... : CheckValue ; OnChange seulement si le texte change
  s->Value=8;
  QCOMPARE(Texte->text(), QString("3"));
  QCOMPARE(NbChange, 2);
  s->Value=3;
  QCOMPARE(NbChange, 2);
  s->Value=-4;
  QCOMPARE((int)s->Value, 1);
  QCOMPARE(NbChange, 3);
  // Frappe hors bornes : acceptée telle quelle, OnChange à chaque caractère
  Fiche.show();
  Fiche.activateWindow();
  QVERIFY(QTest::qWaitForWindowActive(&Fiche));
  s->setFocus(Qt::MouseFocusReason);
  QVERIFY(s->hasFocus());
  s->selectAll();
  NbChange=0;
  QTest::keyClicks(s, "42x");
  QCOMPARE(Texte->text(), QString("42"));
  QCOMPARE((int)s->Value, 42);
  QCOMPARE(NbChange, 2);
  // Fenêtre désactivée : pas de CMExit
  QFocusEvent Desactivation(QEvent::FocusOut, Qt::ActiveWindowFocusReason);
  QCoreApplication::sendEvent(s, &Desactivation);
  QCOMPARE(Texte->text(), QString("42"));
  // Sortie du champ (clic sur le bouton) : ramenée dans les bornes, avec OnChange
  QTest::mouseClick(b, Qt::LeftButton);
  QVERIFY(b->hasFocus());
  QCOMPARE(Texte->text(), QString("3"));
  QCOMPARE(NbChange, 3);
  // Texte vide : Value=MinValue, déjà dans les bornes : le texte reste vide
  s->setFocus(Qt::MouseFocusReason);
  s->selectAll();
  QTest::keyClick(s, Qt::Key_Backspace);
  QCOMPARE(Texte->text(), QString());
  QCOMPARE((int)s->Value, 1);
  QCOMPARE(NbChange, 4);
  b->setFocus(Qt::TabFocusReason);
  QCOMPARE(Texte->text(), QString());
  QCOMPARE(NbChange, 4);
  // Flèches (touches haut et bas, stepBy des boutons) : Value±Increment, borné
  s->MinValue=0;
  s->MaxValue=10;
  s->Increment=4;
  s->Value=5;
  s->setFocus(Qt::MouseFocusReason);
  QTest::keyClick(s, Qt::Key_Up);
  QCOMPARE(Texte->text(), QString("9"));
  QTest::keyClick(s, Qt::Key_Up);
  QCOMPARE(Texte->text(), QString("10"));
  QTest::keyClick(s, Qt::Key_Down);
  QCOMPARE(Texte->text(), QString("6"));
  s->stepBy(-1);
  s->stepBy(-1);
  QCOMPARE(Texte->text(), QString("0"));
  // depuis une valeur tapée hors bornes : Value (non borné) + Increment, puis borné
  s->selectAll();
  QTest::keyClicks(s, "-9");
  QCOMPARE((int)s->Value, -9);
  QTest::keyClick(s, Qt::Key_Up);
  QCOMPARE(Texte->text(), QString("0"));
  // MinValue=MaxValue : aucune borne, ni à l'affectation ni en sortant
  s->MinValue=7;
  s->MaxValue=7;
  s->Value=-12;
  QCOMPARE(Texte->text(), QString("-12"));
  s->selectAll();
  QTest::keyClicks(s, "99999");
  b->setFocus(Qt::TabFocusReason);
  QCOMPARE(Texte->text(), QString("99999"));
  // EditorEnabled=False : saisie impossible, flèches actives
  s->setProperty("vclEditorEnabled", false);
  s->PrepareVcl();
  s->setFocus(Qt::MouseFocusReason);
  s->selectAll();
  QTest::keyClicks(s, "5");
  QCOMPARE(Texte->text(), QString("99999"));
  QTest::keyClick(s, Qt::Key_Down);
  QCOMPARE(Texte->text(), QString("99995"));
  // Entrée dans le champ (CMEnter, AutoSelect) : texte sélectionné, sauf au clic
  // et quand la fenêtre retrouve l'activation (le champ resté contrôle actif)
  b->setFocus(Qt::TabFocusReason);
  s->setFocus(Qt::OtherFocusReason);
  QCOMPARE(Texte->selectedText(), QString("99995"));
  Texte->deselect();
  QFocusEvent Reactivation(QEvent::FocusIn, Qt::ActiveWindowFocusReason);
  QCoreApplication::sendEvent(s, &Desactivation);
  QCoreApplication::sendEvent(s, &Reactivation);
  QVERIFY(!Texte->hasSelectedText());
  b->setFocus(Qt::TabFocusReason);
  QTest::mouseClick(Texte, Qt::LeftButton);
  QVERIFY(s->hasFocus());
  QVERIFY(!Texte->hasSelectedText());
  // Chargement du .dfm : Value lu après MinValue et MaxValue (CheckValue), sans
  // OnChange ; sans Value, le texte reste « 0 », même hors des bornes
  TCSpinEdit *d=new TCSpinEdit(&Fiche);
  d->setProperty("vclMinValue", 1);
  d->setProperty("vclMaxValue", 32);
  d->setProperty("vclValue", 50);
  d->PrepareVcl();
  QCOMPARE((int)d->Value, 32);
  TCSpinEdit *z=new TCSpinEdit(&Fiche);
  z->setProperty("vclMinValue", 1);
  z->setProperty("vclMaxValue", 32);
  z->PrepareVcl();
  QCOMPARE(z->findChild<QLineEdit *>()->text(), QString("0"));
 }
//---------------------------------------------------------------------------
void TestInterface::aideContextuelle()
 { // F1 ouvre l'aide à la rubrique du HelpContext de la fiche (fiche principale,
   // Rechercher non modale, Paramètres modale), comme le bouton Aide des fiches ;
   // Alt+Maj+F1 et le menu Sommaire ouvrent le sommaire, le menu Tutoriel le
   // tutoriel. La rubrique passe par une page de redirection (base.cpp)
  QDesktopServices::setUrlHandler("file", this, "UrlOuverte");
  QString stDossier=QCoreApplication::applicationDirPath();
  auto Rubrique=[&stDossier](const char *Ancre)
   {
    QUrl u=QUrl::fromLocalFile(stDossier+"/html/index.html");
    u.setFragment(Ancre);
    return u;
   };
  QVERIFY(!OuvreDemo("aide.cvw").isEmpty());
  Focalise();
  UrlsOuvertes.clear();
  QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_F1);
  QCOMPARE(UrlsOuvertes.size(), 1);
  QVERIFY(!UrlsOuvertes[0].hasFragment());
  QCOMPARE(AdresseAide(UrlsOuvertes[0]), Rubrique("PDLI")); // HelpContext 30
  UrlsOuvertes.clear();
  QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_F1, Qt::AltModifier|Qt::ShiftModifier);
  frmSimulation->ActionSommaire->Execute();
  frmSimulation->ActionTutoriel->Execute();
  QUrl Sommaire=QUrl::fromLocalFile(stDossier+"/html/index.html");
  QCOMPARE(UrlsOuvertes, QList<QUrl>({Sommaire, Sommaire, QUrl::fromLocalFile(stDossier+"/tutocv/index.html")}));
  // Rechercher (non modale) : F1 dans un champ, puis le bouton Aide ; fermée
  // d'abord, si un test précédent en échec l'a laissée ouverte
  if (frmRecherche->isVisible()) Clique(frmRecherche->ButtonAnnuler);
  frmSimulation->ActionRechercher->Checked=false;
  frmSimulation->ActionRechercher->Execute();
  QVERIFY(frmRecherche->isVisible());
  UrlsOuvertes.clear();
  Touche(frmRecherche->CSpinEditX, Qt::Key_F1);
  Clique(frmRecherche->ButtonAide);
  QCOMPARE(UrlsOuvertes.size(), 2);
  QCOMPARE(AdresseAide(UrlsOuvertes[0]), Rubrique("RDUEDLS")); // HelpContext 65
  QCOMPARE(AdresseAide(UrlsOuvertes[1]), Rubrique("RDUEDLS"));
  Clique(frmRecherche->ButtonAnnuler);
  QVERIFY(!frmRecherche->isVisible());
  // Paramètres (modale) : F1, puis Annuler
  UrlsOuvertes.clear();
  QuandModale([]() { return frmParamSimul; }, [this]()
   {
    Touche(frmParamSimul->CSpinEditDureeVert, Qt::Key_F1);
    Clique(frmParamSimul->ButtonAnnuler);
   });
  Clique(frmSimulation->ToolButtonParametres);
  QVERIFY(!frmParamSimul); // créée à chaque ouverture, libérée à la fermeture
  QCOMPARE(UrlsOuvertes.size(), 1);
  QCOMPARE(AdresseAide(UrlsOuvertes[0]), Rubrique("PDLS")); // HelpContext 311
  QVERIFY(Messages.isEmpty());
  QDesktopServices::unsetUrlHandler("file");
 }
//---------------------------------------------------------------------------
void TestInterface::policesEtStyleWindows()
 { // Affichage natif de Windows (recette, pas la CI en « offscreen ») : les
   // fiches sont en Microsoft Sans Serif, équivalent TrueType de MS Sans Serif
   // (libellés entiers), avec le style « windowsvista » (valeur des champs
   // numériques visible)
  if (QGuiApplication::platformName()!=QLatin1String("windows")) QSKIP("affichage natif de Windows seulement");
  QFont Police=frmRecherche->RadioButtonParking->font();
  QString Diagnostic=QString("police de Rechercher : %1 (%2) ; MS Sans Serif trouvée : %3 ; Microsoft Sans Serif trouvée : %4 ; style : %5")
                     .arg(QFontInfo(Police).family(), Police.families().join(", "))
                     .arg(QFontDatabase::hasFamily("MS Sans Serif")).arg(QFontDatabase::hasFamily("Microsoft Sans Serif"))
                     .arg(QApplication::style()->name());
  QVERIFY2(QFontInfo(Police).family()==QLatin1String("Microsoft Sans Serif"), qPrintable(Diagnostic));
  QVERIFY2(QApplication::style()->name()==QLatin1String("windowsvista"), qPrintable(Diagnostic));
 }
//---------------------------------------------------------------------------
void TestInterface::cleanupTestCase()
 {
  alea::LibereGraine();
  Repondeur.stop();
  DetruitFiches();
 }
//---------------------------------------------------------------------------
int main(int argc, char *argv[])
 { // La carte (variable d'environnement CV_CARTE, en Unicode sur tous les
   // systèmes) est copiée puis passée à l'application comme « cvw <copie> »
  QTemporaryDir Temp;
  QString Origine=qEnvironmentVariable("CV_CARTE");
  QString Copie=Temp.path()+"/carte.cvw";
  QFile::copy(Origine, Copie);
  QByteArray CopieLocale=Copie.toLocal8Bit();
  std::vector<char *> ArgsApplication={argv[0], CopieLocale.data()};
  int NbArgsApplication=2;
  // Boîtes de message de Qt et non natives (NSAlert sous macOS) : le test y répond
  QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
  QApplication App(NbArgsApplication, ArgsApplication.data());
  InitialiseApplication(App);
  // Carte : le chemin tel que passé à l'application (ParamStr(1), affiché tel quel dans le titre)
  ArgumentsTest<<QString::fromLocal8Bit(argv[0])<<Copie<<Origine;
  DossierCaptures=qEnvironmentVariable("CV_DOSSIER_CAPTURES");
  TestInterface t;
  return QTest::qExec(&t, argc, argv);
 }
//---------------------------------------------------------------------------
#include "test_interface.moc"
