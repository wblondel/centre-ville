//---------------------------------------------------------------------------
// compat/fichier.cpp - Portage de Centre-Ville hors de C++Builder
//---------------------------------------------------------------------------
#include "fichier.h"
#include <atomic>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <cstdio>
#include <mutex>
#include <vector>
#ifdef _WIN32
 #include <fcntl.h>
 #include <io.h>
 #include <process.h>
 #include <share.h>
 #include <sys/stat.h>
#else
 #include <fcntl.h>
 #include <sys/stat.h>
 #include <unistd.h>
#endif
//---------------------------------------------------------------------------
namespace
 {
  // Poignée VCL = indice+1 dans la table (jamais 0). Les lectures et écritures
  // passent par un tampon (le moteur lit et écrit champ par champ : un appel
  // système par champ serait très lent) ; le comportement observable reste
  // celui de SysUtils (nombre d'octets, fin de fichier, position).
  const std::size_t TAILLE_TAMPON=1<<16;
  struct fichier
   {
    int fd=-1;
    std::vector<char> Lecture; // octets lus d'avance
    std::size_t PosLecture=0;  // prochain octet à rendre dans Lecture
    std::vector<char> Ecriture; // octets en attente d'écriture
    bool Erreur=false; // une écriture a échoué : les suivantes échouent aussi
    std::filesystem::path Temporaire, Cible; // v6.0.2 : FileCreateRemplacement
   };
  std::mutex Verrou;
  std::vector<fichier> Fichiers;
  //-------------------------------------------------------------------------
  int Enregistre(int fd)
   {
    if (fd<0) return -1;
    std::lock_guard<std::mutex> g(Verrou);
    for(std::size_t i=0; i<Fichiers.size(); i++)
     if (Fichiers[i].fd<0)
      {
       Fichiers[i]=fichier();
       Fichiers[i].fd=fd;
       return (int)i+1;
      }
    Fichiers.push_back(fichier());
    Fichiers.back().fd=fd;
    return (int)Fichiers.size();
   }
  //-------------------------------------------------------------------------
  fichier *Trouve(int Handle)
   { // Appelé sous le verrou
    if ((Handle<1)||((std::size_t)Handle>Fichiers.size())) return nullptr;
    fichier *f=&Fichiers[(std::size_t)Handle-1];
    return (f->fd<0)?nullptr:f;
   }
  //-------------------------------------------------------------------------
  long long LitSysteme(int fd, char *Tampon, std::size_t n)
   {
#ifdef _WIN32
    return _read(fd, Tampon, (unsigned)n);
#else
    return (long long)read(fd, Tampon, n);
#endif
   }
  //-------------------------------------------------------------------------
  bool EcritSysteme(int fd, const char *Tampon, std::size_t n)
   {
    std::size_t Total=0;
    while(Total<n)
     {
#ifdef _WIN32
      int k=_write(fd, Tampon+Total, (unsigned)(n-Total));
#else
      ssize_t k=write(fd, Tampon+Total, n-Total);
#endif
      if (k<=0) return false;
      Total+=(std::size_t)k;
     }
    return true;
   }
  //-------------------------------------------------------------------------
  long long DeplaceSysteme(int fd, long long Decalage, int Origine)
   {
#ifdef _WIN32
    return _lseeki64(fd, Decalage, Origine);
#else
    return (long long)lseek(fd, (off_t)Decalage, Origine);
#endif
   }
  //-------------------------------------------------------------------------
  // Vide le tampon d'écriture et rend au système les octets lus d'avance
  bool Synchronise(fichier *f)
   {
    bool Ok=true;
    if (!f->Ecriture.empty())
     {
      Ok=EcritSysteme(f->fd, f->Ecriture.data(), f->Ecriture.size());
      if (!Ok) f->Erreur=true;
      f->Ecriture.clear();
     }
    std::size_t Restant=f->Lecture.size()-f->PosLecture;
    if (Restant)
     DeplaceSysteme(f->fd, -(long long)Restant, SEEK_CUR);
    f->Lecture.clear();
    f->PosLecture=0;
    return Ok;
   }
  //-------------------------------------------------------------------------
  std::filesystem::path Chemin(const AnsiString &asNomFichier)
   { // Les noms de fichiers sont en UTF-8
    const std::string &s=asNomFichier.str();
    return std::filesystem::path(std::u8string(s.begin(), s.end()));
   }
  //-------------------------------------------------------------------------
  int OuvreChemin(const std::filesystem::path &p, int Drapeaux)
   {
#ifdef _WIN32
    int fd=-1;
    if (_wsopen_s(&fd, p.c_str(), Drapeaux|_O_BINARY, _SH_DENYNO, _S_IREAD|_S_IWRITE)) fd=-1;
    return fd;
#else
    return open(p.c_str(), Drapeaux, 0666);
#endif
   }
  //-------------------------------------------------------------------------
  int Ouvre(const AnsiString &asNomFichier, int Drapeaux)
   {
    return OuvreChemin(Chemin(asNomFichier), Drapeaux);
   }
  //-------------------------------------------------------------------------
  bool FermeSysteme(int fd)
   {
#ifdef _WIN32
    return _close(fd)==0;
#else
    return close(fd)==0;
#endif
   }
  //-------------------------------------------------------------------------
  // Fichier temporaire créé à côté de Cible (sans tirage : alea reste celui de la simulation)
  int CreeTemporaire(const std::filesystem::path &Dossier, std::filesystem::path &Temporaire)
   {
    static std::atomic<unsigned> Numero{0};
#ifdef _WIN32
    const int Drapeaux=_O_RDWR|_O_CREAT|_O_EXCL;
    const long Processus=_getpid();
#else
    const int Drapeaux=O_RDWR|O_CREAT|O_EXCL;
    const long Processus=getpid();
#endif
    int fd=-1;
    for(int Essai=0; (fd<0)&&(Essai<100); Essai++)
     {
      Temporaire=Dossier/(".centre-ville-"+std::to_string(Processus)+"-"+std::to_string(Numero++)+".tmp");
      fd=OuvreChemin(Temporaire, Drapeaux);
      if ((fd<0)&&(errno!=EEXIST)) break;
     }
    return fd;
   }
 }
//---------------------------------------------------------------------------
int FileOpen(const AnsiString &asNomFichier, unsigned Mode)
 { // Les modes de partage (fmShare*) n'ont pas d'équivalent portable : ignorés
  int Drapeaux;
  switch(Mode&3)
   {
#ifdef _WIN32
    case fmOpenWrite:     Drapeaux=_O_WRONLY; break;
    case fmOpenReadWrite: Drapeaux=_O_RDWR; break;
    default:              Drapeaux=_O_RDONLY; break;
#else
    case fmOpenWrite:     Drapeaux=O_WRONLY; break;
    case fmOpenReadWrite: Drapeaux=O_RDWR; break;
    default:              Drapeaux=O_RDONLY; break;
#endif
   }
  return Enregistre(Ouvre(asNomFichier, Drapeaux));
 }
//---------------------------------------------------------------------------
int FileCreate(const AnsiString &asNomFichier)
 { // Comme la VCL : création ou troncature, accès en lecture/écriture
#ifdef _WIN32
  return Enregistre(Ouvre(asNomFichier, _O_RDWR|_O_CREAT|_O_TRUNC));
#else
  return Enregistre(Ouvre(asNomFichier, O_RDWR|O_CREAT|O_TRUNC));
#endif
 }
//---------------------------------------------------------------------------
int FileRead(int Handle, void *Tampon, unsigned NbOctets)
 { // Comme ReadFile : jusqu'à NbOctets, moins en fin de fichier
  std::lock_guard<std::mutex> g(Verrou);
  fichier *f=Trouve(Handle);
  if (!f) return -1;
  if (!f->Ecriture.empty()&&!Synchronise(f)) return -1;
  char *Destination=(char*)Tampon;
  unsigned Total=0;
  while(Total<NbOctets)
   {
    if (f->PosLecture==f->Lecture.size())
     {
      if (NbOctets-Total>=TAILLE_TAMPON)
       { // Gros bloc : lecture directe
        long long n=LitSysteme(f->fd, Destination+Total, NbOctets-Total);
        if (n<0) return -1;
        if (n==0) break;
        Total+=(unsigned)n;
        continue;
       }
      f->Lecture.resize(TAILLE_TAMPON);
      long long n=LitSysteme(f->fd, f->Lecture.data(), TAILLE_TAMPON);
      if (n<0) { f->Lecture.clear(); f->PosLecture=0; return -1; }
      f->Lecture.resize((std::size_t)n);
      f->PosLecture=0;
      if (n==0) break;
     }
    std::size_t k=f->Lecture.size()-f->PosLecture;
    if (k>NbOctets-Total) k=NbOctets-Total;
    std::memcpy(Destination+Total, f->Lecture.data()+f->PosLecture, k);
    f->PosLecture+=k;
    Total+=(unsigned)k;
   }
  return (int)Total;
 }
//---------------------------------------------------------------------------
int FileWrite(int Handle, const void *Tampon, unsigned NbOctets)
 {
  std::lock_guard<std::mutex> g(Verrou);
  fichier *f=Trouve(Handle);
  if ((!f)||f->Erreur) return -1;
  if ((f->PosLecture!=f->Lecture.size())||!f->Lecture.empty()) Synchronise(f);
  f->Ecriture.insert(f->Ecriture.end(), (const char*)Tampon, (const char*)Tampon+NbOctets);
  if ((f->Ecriture.size()>=TAILLE_TAMPON)&&!Synchronise(f)) return -1;
  return (int)NbOctets;
 }
//---------------------------------------------------------------------------
int FileSeek(int Handle, int Decalage, int Origine)
 {
  std::lock_guard<std::mutex> g(Verrou);
  fichier *f=Trouve(Handle);
  if (!f) return -1;
  if (!Synchronise(f)) return -1;
  long long p=DeplaceSysteme(f->fd, Decalage, Origine);
  return (p<0)?-1:(int)p;
 }
//---------------------------------------------------------------------------
bool FileFlush(int Handle)
 {
  std::lock_guard<std::mutex> g(Verrou);
  fichier *f=Trouve(Handle);
  if (!f) return false;
  Synchronise(f);
  return !f->Erreur;
 }
//---------------------------------------------------------------------------
void FileClose(int Handle)
 {
  int fd=-1;
   {
    std::lock_guard<std::mutex> g(Verrou);
    fichier *f=Trouve(Handle);
    if (!f) return;
    Synchronise(f);
    fd=f->fd;
    *f=fichier();
   }
#ifdef _WIN32
  _close(fd);
#else
  close(fd);
#endif
 }
//---------------------------------------------------------------------------
int FileCreateRemplacement(const AnsiString &asNomFichier, bool *Direct)
 {
  namespace fs=std::filesystem;
  std::error_code ec;
  fs::path Cible=Chemin(asNomFichier);
  if (Direct) *Direct=false;
  if (fs::is_symlink(Cible, ec))
   { // Le lien reste : c'est sa cible qui est remplacée
    fs::path Resolu=fs::canonical(Cible, ec);
    if (ec)
     { // Lien vers rien : à traiter comme FileCreate
      if (Direct) *Direct=true;
      return FileCreate(asNomFichier);
     }
    Cible=Resolu;
   }
  const bool Existe=fs::exists(Cible, ec);
  if (Existe)
   { // Mêmes refus que FileCreate (fichier en lecture seule, dossier) : renommer les ignorerait
#ifdef _WIN32
    int fd=OuvreChemin(Cible, _O_WRONLY);
#else
    int fd=OuvreChemin(Cible, O_WRONLY);
#endif
    if (fd<0) return -1;
    FermeSysteme(fd);
   }
  fs::path Dossier=Cible.parent_path();
  if (Dossier.empty()) Dossier=".";
  fs::path Temporaire;
  int fd=CreeTemporaire(Dossier, Temporaire);
  if (fd<0)
   {
    int Erreur=errno;
#ifdef _WIN32
    const bool Interdit=(Erreur==EACCES);
#else
    const bool Interdit=(Erreur==EACCES)||(Erreur==EPERM)||(Erreur==EROFS);
#endif
    if (!(Existe&&Interdit)) return -1; // Dossier plein, notamment : réécrire le fichier le détruirait
    if (Direct) *Direct=true;
    return FileCreate(asNomFichier);
   }
#ifndef _WIN32
  struct stat Etat;
  if (Existe&&(stat(Cible.c_str(), &Etat)==0))
   {
    fchmod(fd, Etat.st_mode&07777);
    (void)!fchown(fd, Etat.st_uid, Etat.st_gid); // Refusé si le fichier est à un autre : il reste à l'utilisateur
   }
#endif
  int Handle=Enregistre(fd);
  std::lock_guard<std::mutex> g(Verrou);
  fichier *f=Trouve(Handle);
  if (f)
   {
    f->Temporaire=Temporaire;
    f->Cible=Cible;
   }
  return Handle;
 }
//---------------------------------------------------------------------------
bool FileCommitRemplacement(int Handle)
 {
  namespace fs=std::filesystem;
  int fd=-1;
  bool Ok;
  fs::path Temporaire, Cible;
   {
    std::lock_guard<std::mutex> g(Verrou);
    fichier *f=Trouve(Handle);
    if (!f) return false;
    Synchronise(f);
    Ok=!f->Erreur;
    fd=f->fd;
    Temporaire=f->Temporaire;
    Cible=f->Cible;
    *f=fichier();
   }
  // Les octets d'abord sur le disque : une coupure de courant ne doit pas laisser le nouveau nom sur un fichier vide
#ifdef _WIN32
  Ok=(_commit(fd)==0)&&Ok;
#else
  Ok=(fsync(fd)==0)&&Ok;
#endif
  Ok=FermeSysteme(fd)&&Ok;
  if (Temporaire.empty()) return Ok; // Écriture directe
  std::error_code ec;
  if (Ok)
   {
    fs::rename(Temporaire, Cible, ec);
    Ok=!ec;
   }
  if (!Ok) fs::remove(Temporaire, ec);
  return Ok;
 }
//---------------------------------------------------------------------------
void FileAbandonRemplacement(int Handle)
 {
  std::filesystem::path Temporaire;
   {
    std::lock_guard<std::mutex> g(Verrou);
    fichier *f=Trouve(Handle);
    if (!f) return;
    Temporaire=f->Temporaire;
    f->Ecriture.clear(); // Inutile de tenter d'écrire ce qui attend encore
   }
  FileClose(Handle);
  std::error_code ec;
  if (!Temporaire.empty()) std::filesystem::remove(Temporaire, ec);
 }
//---------------------------------------------------------------------------
