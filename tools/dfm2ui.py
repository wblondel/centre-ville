#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Conversion des fiches VCL (.dfm) de Centre-Ville en fichiers Qt Designer (.ui).

Phase 6 de PLAN_PORTAGE.md (outillage), utilisée dès la phase 5 pour la fiche
principale. Pour chaque fiche src/<fiche>.dfm, l'outil écrit :

  - app/ui/<fiche>.ui : la fiche au format Qt Designer, en géométrie absolue
    (Left/Top/Width/Height), avec Caption, Hint, TabOrder, Enabled/Visible,
    bornes et valeurs des spins, éléments des TRadioGroup, polices, couleurs,
    boutons Default/Cancel et ModalResult. Les menus (TMainMenu, TPopupMenu,
    TMenuItem), les actions (TActionList/TAction) et les barres d'outils
    (TToolBar/TToolButton) sont convertis aussi ;
  - app/ui/<fiche>_composants.h : deux listes en X-macros, les composants de
    la fiche (pour lier les membres de la classe portée aux objets créés par
    uic) et les gestionnaires d'événements à connecter.

Correspondance des classes : chaque contrôle VCL devient un widget « promu »
du même nom (TLabel, TCSpinEdit, TCheckBox...) déclaré dans app/vcl/controles.h
et dérivé de la classe Qt la plus proche (QLabel, QAbstractSpinBox, QCheckBox...) : uic
instancie directement ces classes, qui reproduisent l'API et les événements
implicites de la VCL (PLAN_PORTAGE.md, §4.4). Ce que Qt Designer ne sait pas
représenter est conservé en propriétés dynamiques préfixées par « vcl » (par
exemple vclAlign, vclModalResult, vclFontHeight), appliquées à l'exécution.

Composants non visuels :
  - TAction -> <action> (catégorie dans la propriété dynamique « categorie ») ;
  - TMenuItem -> <action> (feuille, séparateur compris) ou QMenu (sous-menu) ;
  - TPopupMenu -> QMenu ; TMainMenu -> QMenuBar ;
  - TTimer, TOpenDialog, TSaveDialog -> propriété dynamique « vcl_<Nom> » de la
    fiche (liste « Propriété=valeur ») ;
  - TImageCollection, TVirtualImageList : images extraites par
    tools/extrait_images.py ; les ImageIndex sont résolus avec
    app/ressources/index_images.json.

Usage :
  python3 tools/dfm2ui.py [--src src] [--sortie app/ui] [fiche ...]
  python3 tools/dfm2ui.py --verifie   # échoue si la sortie n'est pas à jour

Aucune dépendance hors bibliothèque standard.
"""
import argparse
import json
import os
import sys
import xml.etree.ElementTree as ET
from collections import OrderedDict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from extrait_images import lit_dfm, Binaire, Identifiant  # noqa: E402

RACINE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# ---------------------------------------------------------------------------
# Correspondances
# ---------------------------------------------------------------------------

# Classe VCL -> (classe promue dans le .ui, classe Qt de base, en-tête)
WIDGETS = OrderedDict([
    ("TLabel", ("TLabel", "QLabel", "vcl/controles.h")),
    ("TCSpinEdit", ("TCSpinEdit", "QAbstractSpinBox", "vcl/controles.h")),
    ("TButton", ("TButton", "QPushButton", "vcl/controles.h")),
    ("TBitBtn", ("TBitBtn", "QPushButton", "vcl/controles.h")),
    ("TCheckBox", ("TCheckBox", "QCheckBox", "vcl/controles.h")),
    ("TRadioButton", ("TRadioButton", "QRadioButton", "vcl/controles.h")),
    ("TGroupBox", ("TGroupBox", "QGroupBox", "vcl/controles.h")),
    ("TRadioGroup", ("TRadioGroup", "QGroupBox", "vcl/controles.h")),
    ("TSpeedButton", ("TSpeedButton", "QToolButton", "vcl/controles.h")),
    ("TShape", ("TShape", "QWidget", "vcl/controles.h")),
    ("TImage", ("TImage", "QLabel", "vcl/controles.h")),
    ("TStringGrid", ("TStringGrid", "QAbstractScrollArea", "vcl/controles.h")),
    ("TTrackBar", ("TTrackBar", "QSlider", "vcl/controles.h")),
    ("TPanel", ("TPanel", "QFrame", "vcl/controles.h")),
    ("TBevel", ("TBevel", "QFrame", "vcl/controles.h")),
    ("TPaintBox", ("TPaintBox", "QWidget", "vcl/controles.h")),
    ("TTabControl", ("TTabControl", "QWidget", "vcl/controles.h")),
    ("TStatusBar", ("TStatusBar", "QStatusBar", "vcl/controles.h")),
    ("TToolBar", ("TToolBar", "QWidget", "vcl/controles.h")),
    ("TToolButton", ("TToolButton", "QToolButton", "vcl/controles.h")),
    ("TDrawGrid", ("GrilleSimulation", "QAbstractScrollArea", "grille/GrilleSimulation.h")),
])

NON_VISUELS_PROPRIETE = ("TTimer", "TOpenDialog", "TSaveDialog")
IGNORES = ("TImageCollection", "TVirtualImageList")
FOCALISABLES = ("TCSpinEdit", "TButton", "TBitBtn", "TCheckBox", "TRadioButton", "TStringGrid",
                "TTrackBar", "TTabControl", "TDrawGrid")
CONTENEURS = ("TGroupBox", "TRadioGroup", "TPanel", "TToolBar", "TTabControl")

# Propriétés traduites en propriétés Qt (les autres sont gardées en « vcl... »)
GEOMETRIE = ("Left", "Top", "Width", "Height")
NE_PAS_CONSERVER = {
    # Propriétés sans effet ou propres à l'EDI / à Windows
    "ExplicitLeft", "ExplicitTop", "ExplicitWidth", "ExplicitHeight", "Ctl3D", "ParentCtl3D",
    "Font.Charset", "Font.Pitch", "OldCreateOrder", "PixelsPerInch", "Locked", "DragKind",
    "ParentFont", "ParentShowHint", "ShowHint", "ParentColor", "ParentBackground",
    "DesignSize", "Images",
}

# Codes de touches virtuelles Windows -> nom de touche Qt (texte portable)
TOUCHES = {
    0x08: "Backspace", 0x09: "Tab", 0x0D: "Return", 0x1B: "Esc", 0x20: "Space",
    0x21: "PgUp", 0x22: "PgDown", 0x23: "End", 0x24: "Home",
    0x25: "Left", 0x26: "Up", 0x27: "Right", 0x28: "Down", 0x2D: "Ins", 0x2E: "Del",
    0x6A: "*", 0x6B: "+", 0x6D: "-", 0x6E: ".", 0x6F: "/",
    0xBA: ";", 0xBB: "=", 0xBC: ",", 0xBD: "-", 0xBE: ".", 0xBF: "/", 0xC0: "`",
    0xDB: "[", 0xDC: "\\", 0xDD: "]", 0xDE: "'",
}
for _c in range(0x30, 0x3A):
    TOUCHES[_c] = chr(_c)
for _c in range(0x41, 0x5B):
    TOUCHES[_c] = chr(_c)
for _c in range(0x60, 0x6A):
    TOUCHES[_c] = "Num+" + chr(0x30 + _c - 0x60)
for _c in range(0x70, 0x88):
    TOUCHES[_c] = "F%d" % (_c - 0x6F)

SC_SHIFT, SC_CTRL, SC_ALT = 0x2000, 0x4000, 0x8000

# Couleurs nommées de la VCL : valeur TColor (0x00BBGGRR) ou couleur système
COULEURS = {
    "clBlack": 0x000000, "clMaroon": 0x000080, "clGreen": 0x008000, "clOlive": 0x008080,
    "clNavy": 0x800000, "clPurple": 0x800080, "clTeal": 0x808000, "clGray": 0x808080,
    "clSilver": 0xC0C0C0, "clRed": 0x0000FF, "clLime": 0x00FF00, "clYellow": 0x00FFFF,
    "clBlue": 0xFF0000, "clFuchsia": 0xFF00FF, "clAqua": 0xFFFF00, "clWhite": 0xFFFFFF,
    "clMoneyGreen": 0xC0DCC0, "clSkyBlue": 0xF0CAA6, "clCream": 0xF0FBFF, "clMedGray": 0xA4A0A0,
}


def raccourci_qt(n):
    """TShortCut (VCL) -> séquence de touches Qt en texte portable ("Ctrl+Alt+D")."""
    if not n:
        return ""
    touche = n & 0xFF
    nom = TOUCHES.get(touche)
    if nom is None:
        raise ValueError("touche virtuelle 0x%02X sans équivalent" % touche)
    mods = []
    if n & SC_CTRL:
        mods.append("Ctrl")
    if n & SC_ALT:
        mods.append("Alt")
    if n & SC_SHIFT:
        mods.append("Shift")
    return "+".join(mods + [nom])


def couleur(valeur):
    """Valeur VCL (identifiant ou entier) -> texte gardé dans vclColor."""
    if isinstance(valeur, int):
        return "0x%08X" % (valeur & 0xFFFFFFFF)
    return str(valeur)


def hint_court_long(hint):
    """« court|long » -> (court, long), comme GetShortHint/GetLongHint."""
    if "|" in hint:
        court, long_ = hint.split("|", 1)
        return court, long_
    return hint, hint


def taille_points(hauteur, ppp=96):
    """Font.Height négatif = hauteur des caractères en pixels à PixelsPerInch."""
    if hauteur < 0:
        return max(1, int(round(-hauteur * 72.0 / ppp)))
    return max(1, int(round(hauteur * 72.0 / ppp * 0.85)))


# ---------------------------------------------------------------------------
# Construction XML
# ---------------------------------------------------------------------------

def sous(parent, balise, texte=None, **attributs):
    e = ET.SubElement(parent, balise, {k.rstrip("_"): v for k, v in attributs.items()})
    if texte is not None:
        e.text = texte
    return e


def prop(parent, nom, genre, valeur, dynamique=False):
    p = sous(parent, "property", name=nom)
    if dynamique:
        p.set("stdset", "0")
    if genre == "rect":
        r = sous(p, "rect")
        for k, v in zip(("x", "y", "width", "height"), valeur):
            sous(r, k, str(v))
    elif genre == "bool":
        sous(p, "bool", "true" if valeur else "false")
    elif genre == "number":
        sous(p, "number", str(int(valeur)))
    elif genre == "string":
        s = sous(p, "string", valeur)
        if dynamique:
            s.set("notr", "true")
    elif genre == "cstring":
        sous(p, "cstring", valeur)
    elif genre == "stringlist":
        sl = sous(p, "stringlist")
        sl.set("notr", "true")
        for v in valeur:
            sous(sl, "string", v)
    elif genre == "enum":
        sous(p, "enum", valeur)
    elif genre == "set":
        sous(p, "set", valeur)
    elif genre == "size":
        t = sous(p, "size")
        sous(t, "width", str(valeur[0]))
        sous(t, "height", str(valeur[1]))
    elif genre == "icon":
        i = sous(p, "iconset", resource="../ressources/images.qrc")
        sous(i, "normaloff", valeur)
    elif genre == "pixmap":
        sous(p, "pixmap", valeur, resource="../ressources/images.qrc")
    elif genre == "font":
        f = sous(p, "font")
        for k, v in valeur:
            sous(f, k, v)
    else:
        raise ValueError(genre)
    return p


def indente(e, niveau=0):
    """Mise en forme stable (ET.indent n'existe qu'à partir de Python 3.9)."""
    i = "\n" + niveau * " "
    if len(e):
        if not e.text or not e.text.strip():
            e.text = i + " "
        for enfant in e:
            indente(enfant, niveau + 1)
        if not enfant.tail or not enfant.tail.strip():
            enfant.tail = i
    if niveau and (not e.tail or not e.tail.strip()):
        e.tail = i


# ---------------------------------------------------------------------------
# Conversion d'une fiche
# ---------------------------------------------------------------------------

class Conversion:
    def __init__(self, chemin, index_images):
        self.chemin = chemin
        self.fiche = os.path.splitext(os.path.basename(chemin))[0]
        self.racine = lit_dfm(chemin)
        self.index = index_images
        self.rapport = []
        self.composants = []  # (classe VCL, nom)
        self.evenements = []  # (composant, événement, gestionnaire)
        self.classes_utilisees = OrderedDict()
        self.actions = OrderedDict()  # nom -> objet TAction
        self.images_actions = {}  # nom de TActionList -> nom de liste d'images
        self.objets = {o.nom: o for o in self.racine.parcours()}
        self.parents = {}
        for o in self.racine.parcours():
            for e in o.enfants:
                self.parents[e.nom] = o
        self.ppp = self.racine.props.get("PixelsPerInch", 96)
        self.hauteur_texte = self.racine.props.get("TextHeight", 13)
        self.principale = any(o.classe == "TMainMenu" for o in self.racine.parcours())
        for o in self.racine.parcours():
            if o.classe == "TActionList":
                for a in o.enfants:
                    self.actions[a.nom] = a
                if "Images" in o.props:
                    self.images_actions[o.nom] = str(o.props["Images"])

    # --- utilitaires -----------------------------------------------------------

    def note(self, texte):
        self.rapport.append(texte)

    def image(self, liste, indice):
        """Indice d'une TVirtualImageList de la fiche -> chemin de ressource Qt."""
        if indice is None or indice < 0:
            return None
        try:
            images = self.index["listes"][self.fiche][liste]["images"]
        except KeyError:
            self.note("  ! liste d'images %s inconnue" % liste)
            return None
        if indice >= len(images):
            self.note("  ! %s : indice %d hors liste" % (liste, indice))
            return None
        return ":/" + images[indice]["fichier"]

    def image_composant(self, nom):
        for c in self.index["composants"]:
            if c["fiche"] == self.fiche and c["composant"] == nom:
                return c
        return None

    def montre_bulles(self, o):
        """ShowHint effectif (ParentShowHint remonte jusqu'à la fiche)."""
        while o is not None:
            if "ShowHint" in o.props:
                return bool(o.props["ShowHint"] == "True")
            if o.props.get("ParentShowHint") == "False":
                return False
            o = self.parents.get(o.nom)
        return False

    def evenements_de(self, o, nom=None):
        for k, v in o.props.items():
            if k.startswith("On") and isinstance(v, str):
                self.evenements.append((nom or o.nom, k, str(v)))

    def police(self, e, o):
        p = o.props
        if not any(k.startswith("Font.") for k in p):
            return
        elements = []
        if "Font.Name" in p:
            elements.append(("family", p["Font.Name"]))
        if "Font.Height" in p:
            elements.append(("pointsize", str(taille_points(p["Font.Height"], self.ppp))))
            prop(e, "vclFontHeight", "number", p["Font.Height"], True)
        styles = p.get("Font.Style", set())
        elements.append(("bold", "true" if "fsBold" in styles else "false"))
        elements.append(("italic", "true" if "fsItalic" in styles else "false"))
        elements.append(("underline", "true" if "fsUnderline" in styles else "false"))
        elements.append(("strikeout", "true" if "fsStrikeOut" in styles else "false"))
        prop(e, "font", "font", elements)
        if "Font.Color" in p and p["Font.Color"] not in ("clWindowText", "clBtnText"):
            prop(e, "vclFontColor", "string", couleur(p["Font.Color"]), True)

    def bulle(self, e, o, hint):
        if not hint:
            return
        court, long_ = hint_court_long(hint)
        if self.montre_bulles(o):
            prop(e, "toolTip", "string", court)
        prop(e, "statusTip", "string", long_)

    def proprietes_communes(self, e, o, deja):
        p = o.props
        if all(k in p for k in GEOMETRIE):
            prop(e, "geometry", "rect", [p[k] for k in GEOMETRIE])
        if p.get("Enabled") == "False" and "Enabled" not in deja:
            prop(e, "enabled", "bool", False)
        if p.get("Visible") == "False" and "Visible" not in deja:
            prop(e, "visible", "bool", False)
        self.police(e, o)
        if "Hint" in p and "Hint" not in deja:
            self.bulle(e, o, p["Hint"])
        for k, v in p.items():
            if (k in deja or k in GEOMETRIE or k in NE_PAS_CONSERVER or k.startswith("Font.")
                    or k.startswith("On") or k in ("Enabled", "Visible", "Hint")
                    or isinstance(v, Binaire)):
                continue
            self.dynamique(e, k, v)
        self.evenements_de(o)

    def dynamique(self, e, k, v):
        nom = "vcl" + k.replace(".", "")
        if isinstance(v, bool) or v in ("True", "False"):
            prop(e, nom, "bool", v == "True", True)
        elif isinstance(v, int):
            prop(e, nom, "number", v, True)
        elif isinstance(v, float):
            prop(e, nom, "string", repr(v), True)
        elif isinstance(v, set):
            prop(e, nom, "string", ",".join(sorted(str(x) for x in v)), True)
        elif isinstance(v, list):
            if all(isinstance(x, (str, int, float)) for x in v):
                prop(e, nom, "stringlist", [str(x) for x in v], True)
            else:
                prop(e, nom, "stringlist", [";".join("%s=%s" % (kk, vv) for kk, vv in el.items())
                                            for el in v], True)
        else:
            prop(e, nom, "string", str(v), True)

    def widget(self, parent_xml, o):
        classe, base, entete = WIDGETS[o.classe]
        self.classes_utilisees[classe] = (base, entete)
        e = sous(parent_xml, "widget", class_=classe, name=o.nom)
        deja = set()
        p = o.props
        methode = getattr(self, "w_" + o.classe, None)
        if methode:
            deja = methode(e, o) or set()
        self.proprietes_communes(e, o, deja)
        self.composants.append((o.classe, o.nom))
        if o.classe not in ("TRadioGroup",):
            self.enfants(e, o)
        return e

    def enfants(self, e, o):
        for enfant in o.enfants:
            self.objet(e, enfant)

    def objet(self, parent_xml, o):
        if o.classe in WIDGETS:
            if o.classe == "TStatusBar" and self.principale:
                self.widget(self.xml_racine, o)
            else:
                self.widget(parent_xml, o)
        elif o.classe in ("TMainMenu", "TPopupMenu", "TActionList", "TMenuItem", "TAction"):
            pass  # traités à part
        elif o.classe in NON_VISUELS_PROPRIETE:
            valeurs = ["%s=%s" % (k, (",".join(sorted(map(str, v))) if isinstance(v, set) else v))
                       for k, v in o.props.items() if k not in ("Left", "Top")]
            prop(self.xml_racine, "vcl_" + o.nom, "stringlist", ["Classe=" + o.classe] + valeurs, True)
            self.composants.append((o.classe, o.nom))
            self.evenements_de(o)
        elif o.classe in IGNORES:
            self.composants.append((o.classe, o.nom))
        else:
            self.note("  ! composant %s: %s non converti" % (o.nom, o.classe))

    # --- contrôles ---------------------------------------------------------------

    def w_TLabel(self, e, o):
        p = o.props
        prop(e, "text", "string", p.get("Caption", ""))
        alignement = {"taCenter": "Qt::AlignHCenter", "taRightJustify": "Qt::AlignRight"}.get(
            p.get("Alignment"), "Qt::AlignLeft")
        vertical = {"tlCenter": "Qt::AlignVCenter", "tlBottom": "Qt::AlignBottom"}.get(
            p.get("Layout"), "Qt::AlignTop")
        prop(e, "alignment", "set", alignement + "|" + vertical)
        if p.get("WordWrap") == "True":
            prop(e, "wordWrap", "bool", True)
        if "FocusControl" in p:
            prop(e, "buddy", "cstring", str(p["FocusControl"]))
        return {"Caption", "Alignment", "Layout", "WordWrap", "FocusControl"}

    def w_TCSpinEdit(self, e, o):
        # CSPIN.pas : MinValue, MaxValue et Increment sont de simples champs, et
        # Value écrit le texte (borné si MinValue <> MaxValue) ; tout est appliqué
        # par TCSpinEdit::PrepareVcl, Value après les bornes comme au chargement
        p = o.props
        prop(e, "vclMinValue", "number", p.get("MinValue", 0), True)
        prop(e, "vclMaxValue", "number", p.get("MaxValue", 0), True)
        if "Value" in p:
            prop(e, "vclValue", "number", p["Value"], True)
        if "Increment" in p:
            prop(e, "vclIncrement", "number", p["Increment"], True)
        return {"MinValue", "MaxValue", "Value", "Increment"}

    def boutons(self, e, o):
        p = o.props
        if p.get("Default") == "True":
            prop(e, "default", "bool", True)
        prop(e, "autoDefault", "bool", False)
        return {"Default"}

    def w_TButton(self, e, o):
        prop(e, "text", "string", o.props.get("Caption", ""))
        return {"Caption"} | self.boutons(e, o)

    def glyphe(self, e, o):
        c = self.image_composant(o.nom)
        if c is None:
            return
        chemin = ":/" + c["fichier"]
        n = c.get("glyphes", 1) or 1
        prop(e, "icon", "icon", chemin)
        prop(e, "iconSize", "size", (c["largeur"] // n, c["hauteur"]))
        prop(e, "vclGlyph", "string", chemin, True)

    def w_TBitBtn(self, e, o):
        prop(e, "text", "string", o.props.get("Caption", ""))
        self.glyphe(e, o)
        return {"Caption"} | self.boutons(e, o)

    def w_TCheckBox(self, e, o):
        p = o.props
        prop(e, "text", "string", p.get("Caption", ""))
        if p.get("Checked") == "True" or p.get("State") == "cbChecked":
            prop(e, "checked", "bool", True)
        if p.get("Alignment") == "taLeftJustify":
            prop(e, "layoutDirection", "enum", "Qt::RightToLeft")
        return {"Caption", "Checked", "State"}

    def w_TRadioButton(self, e, o):
        p = o.props
        prop(e, "text", "string", p.get("Caption", ""))
        if p.get("Checked") == "True":
            prop(e, "checked", "bool", True)
        return {"Caption", "Checked"}

    def w_TGroupBox(self, e, o):
        prop(e, "title", "string", o.props.get("Caption", ""))
        return {"Caption"}

    def w_TRadioGroup(self, e, o):
        """Les éléments deviennent des QRadioButton disposés comme
        TCustomRadioGroup.ArrangeButtons."""
        p = o.props
        prop(e, "title", "string", p.get("Caption", ""))
        elements = p.get("Items.Strings", [])
        colonnes = max(1, p.get("Columns", 1))
        indice = p.get("ItemIndex", -1)
        if "Font.Height" in p:
            hauteur_texte = int(round(-p["Font.Height"] * 13.0 / 11.0))
        else:
            hauteur_texte = self.hauteur_texte
        largeur, hauteur = p.get("Width", 0), p.get("Height", 0)
        if elements:
            par_colonne = (len(elements) + colonnes - 1) // colonnes
            largeur_bouton = (largeur - 10) // colonnes
            reste = hauteur - hauteur_texte - 5
            hauteur_bouton = reste // par_colonne
            marge = hauteur_texte + 1 + (reste % par_colonne) // 2
            for i, texte in enumerate(elements):
                b = sous(e, "widget", class_="QRadioButton", name="%s_%d" % (o.nom, i))
                prop(b, "geometry", "rect", [(i // par_colonne) * largeur_bouton + 8,
                                             (i % par_colonne) * hauteur_bouton + marge,
                                             largeur_bouton, hauteur_bouton])
                prop(b, "text", "string", texte)
                if i == indice:
                    prop(b, "checked", "bool", True)
        return {"Caption", "Items.Strings"}

    def w_TSpeedButton(self, e, o):
        p = o.props
        self.glyphe(e, o)
        if p.get("GroupIndex", 0) > 0:
            prop(e, "checkable", "bool", True)
            if p.get("Down") == "True":
                prop(e, "checked", "bool", True)
        if p.get("Flat") == "True":
            prop(e, "autoRaise", "bool", True)
        if p.get("Caption"):
            prop(e, "text", "string", p["Caption"])
        return {"Caption", "Down", "Flat"}

    def w_TShape(self, e, o):
        return set()

    def w_TImage(self, e, o):
        c = self.image_composant(o.nom)
        if c is not None:
            prop(e, "pixmap", "pixmap", ":/" + c["fichier"])
        if o.props.get("Stretch") == "True":
            prop(e, "scaledContents", "bool", True)
        return set()

    def w_TStringGrid(self, e, o):
        p = o.props
        prop(e, "rowCount", "number", p.get("RowCount", 5))
        prop(e, "columnCount", "number", p.get("ColCount", 5))
        return set()

    def w_TTrackBar(self, e, o):
        p = o.props
        prop(e, "orientation", "enum", "Qt::Vertical" if p.get("Orientation") == "trVertical"
             else "Qt::Horizontal")
        prop(e, "minimum", "number", p.get("Min", 0))
        prop(e, "maximum", "number", p.get("Max", 10))
        prop(e, "value", "number", p.get("Position", 0))
        prop(e, "pageStep", "number", p.get("PageSize", 2))
        prop(e, "tickInterval", "number", p.get("Frequency", 1))
        prop(e, "tickPosition", "enum", {"tmTopLeft": "QSlider::TicksAbove",
                                         "tmBoth": "QSlider::TicksBothSides"}.get(
            p.get("TickMarks"), "QSlider::TicksBelow"))
        return {"Orientation", "Min", "Max", "Position", "PageSize", "Frequency"}

    def w_TPanel(self, e, o):
        p = o.props
        if p.get("BevelOuter") == "bvNone" and p.get("BevelInner", "bvNone") == "bvNone":
            prop(e, "frameShape", "enum", "QFrame::NoFrame")
        else:
            prop(e, "frameShape", "enum", "QFrame::Panel")
            prop(e, "frameShadow", "enum", "QFrame::Sunken" if p.get("BevelOuter") == "bvLowered"
                 else "QFrame::Raised")
        return set()

    def w_TBevel(self, e, o):
        p = o.props
        forme = {"bsTopLine": "QFrame::HLine", "bsBottomLine": "QFrame::HLine",
                 "bsLeftLine": "QFrame::VLine", "bsRightLine": "QFrame::VLine"}.get(
            p.get("Shape"), "QFrame::Box")
        prop(e, "frameShape", "enum", forme)
        prop(e, "frameShadow", "enum", "QFrame::Raised" if p.get("Style") == "bsRaised"
             else "QFrame::Sunken")
        return set()

    def w_TPaintBox(self, e, o):
        return set()

    def w_TTabControl(self, e, o):
        return set()

    def w_TStatusBar(self, e, o):
        p = o.props
        panneaux = []
        for el in p.get("Panels", []):
            panneaux.append(";".join("%s=%s" % (k, v) for k, v in el.items()))
        prop(e, "vclPanels", "stringlist", panneaux, True)
        if self.principale:
            prop(e, "sizeGripEnabled", "bool", p.get("SizeGrip") != "False")
        return {"Panels"}

    def w_TToolBar(self, e, o):
        return set()

    def w_TToolButton(self, e, o):
        """Bouton lié à une action (Action) ou à un menu (MenuItem)."""
        p = o.props
        largeur, hauteur = 23, 22
        barre = self.parents[o.nom].props
        largeur = barre.get("ButtonWidth", largeur)
        hauteur = barre.get("ButtonHeight", hauteur)
        prop(e, "geometry", "rect", [p.get("Left", 0), p.get("Top", 0), largeur, hauteur])
        source = None
        if "Action" in p:
            source = self.actions.get(str(p["Action"]))
        elif "MenuItem" in p:
            source = self.objets.get(str(p["MenuItem"]))
        sp = source.props if source is not None else {}
        texte = p.get("Caption", sp.get("Caption", ""))
        prop(e, "text", "string", texte)
        indice = p.get("ImageIndex", sp.get("ImageIndex", -1))
        images = str(self.parents[o.nom].props.get("Images", ""))
        chemin = self.image(images, indice) if images else None
        if chemin:
            prop(e, "icon", "icon", chemin)
        hint = p.get("Hint", sp.get("Hint", ""))
        self.bulle(e, o, hint)
        if p.get("Style") == "tbsCheck":
            prop(e, "checkable", "bool", True)
            if sp.get("Checked") == "True" or p.get("Down") == "True":
                prop(e, "checked", "bool", True)
        enabled = p.get("Enabled", sp.get("Enabled", "True"))
        if enabled == "False":
            prop(e, "enabled", "bool", False)
        return {"Caption", "ImageIndex", "Hint", "Enabled", "Left", "Top", "Width", "Height"}

    def w_TDrawGrid(self, e, o):
        prop(e, "frameShape", "enum", "QFrame::NoFrame")
        return set()

    # --- menus et actions --------------------------------------------------------

    def action(self, parent_xml, o):
        p = o.props
        e = sous(parent_xml, "action", name=o.nom)
        prop(e, "text", "string", p.get("Caption", ""))
        indice = p.get("ImageIndex", -1)
        liste = self.images_actions.get(self.parents[o.nom].nom)
        chemin = self.image(liste, indice) if liste else None
        if chemin:
            prop(e, "icon", "icon", chemin)
        if "Hint" in p:
            court, long_ = hint_court_long(p["Hint"])
            prop(e, "toolTip", "string", court)
            prop(e, "statusTip", "string", long_)
        if p.get("ShortCut"):
            prop(e, "shortcut", "string", raccourci_qt(p["ShortCut"]))
        if p.get("Checked") == "True" or p.get("AutoCheck") == "True":
            prop(e, "checkable", "bool", True)
        if p.get("Checked") == "True":
            prop(e, "checked", "bool", True)
        if p.get("Enabled") == "False":
            prop(e, "enabled", "bool", False)
        if p.get("Visible") == "False":
            prop(e, "visible", "bool", False)
        prop(e, "categorie", "string", p.get("Category", ""), True)
        prop(e, "vclClasse", "string", "TAction", True)
        for k, v in p.items():
            if k in ("Caption", "Hint", "Category", "Checked", "Enabled", "Visible") or k.startswith("On"):
                continue
            self.dynamique(e, k, v)
        self.composants.append(("TAction", o.nom))
        self.evenements_de(o)

    def feuille_menu(self, o, images):
        """TMenuItem sans sous-éléments -> <action> (séparateur compris)."""
        p = o.props
        e = sous(self.xml_racine, "action", name=o.nom)
        source = self.actions.get(str(p.get("Action", ""))) if "Action" in p else None
        sp = source.props if source is not None else {}
        caption = p.get("Caption", sp.get("Caption", ""))
        if caption == "-":
            prop(e, "vclSeparateur", "bool", True, True)
        prop(e, "text", "string", caption)
        indice = p.get("ImageIndex", sp.get("ImageIndex", -1))
        if source is not None and "ImageIndex" not in p:
            liste = self.images_actions.get(self.parents[source.nom].nom)
        else:
            liste = images
        chemin = self.image(liste, indice) if liste else None
        if chemin:
            prop(e, "icon", "icon", chemin)
        hint = p.get("Hint", sp.get("Hint", ""))
        if hint:
            court, long_ = hint_court_long(hint)
            prop(e, "toolTip", "string", court)
            prop(e, "statusTip", "string", long_)
        raccourci = p.get("ShortCut", sp.get("ShortCut", 0))
        if raccourci:
            prop(e, "shortcut", "string", raccourci_qt(raccourci))
            prop(e, "shortcutContext", "enum", "Qt::WidgetShortcut")
        coche = p.get("Checked", sp.get("Checked", "False")) == "True"
        if coche or p.get("RadioItem") == "True" or p.get("AutoCheck") == "True":
            prop(e, "checkable", "bool", True)
        if coche:
            prop(e, "checked", "bool", True)
        if p.get("Enabled", sp.get("Enabled", "True")) == "False":
            prop(e, "enabled", "bool", False)
        if p.get("Visible", sp.get("Visible", "True")) == "False":
            prop(e, "visible", "bool", False)
        prop(e, "vclClasse", "string", "TMenuItem", True)
        for k, v in p.items():
            if k in ("Hint",) or k.startswith("On"):
                continue
            self.dynamique(e, k, v)
        self.composants.append(("TMenuItem", o.nom))
        self.evenements_de(o)

    def menu(self, parent_xml, o, images, popup=False):
        """TMenuItem à sous-éléments ou TPopupMenu -> QMenu."""
        p = o.props
        e = sous(parent_xml, "widget", class_="QMenu", name=o.nom)
        prop(e, "title", "string", "" if popup else p.get("Caption", ""))
        if not popup:
            source = self.actions.get(str(p.get("Action", ""))) if "Action" in p else None
            sp = source.props if source is not None else {}
            chemin = self.image(images, p.get("ImageIndex", sp.get("ImageIndex", -1))) if images else None
            if chemin:
                prop(e, "icon", "icon", chemin)
            if p.get("Enabled", "True") == "False":
                prop(e, "enabled", "bool", False)
            if "Hint" in p:
                prop(e, "statusTip", "string", hint_court_long(p["Hint"])[1])
        prop(e, "vclClasse", "string", "TPopupMenu" if popup else "TMenuItem", True)
        for k, v in p.items():
            if k in ("Caption", "Enabled", "Hint", "Images") or k.startswith("On") or k in ("Left", "Top"):
                continue
            self.dynamique(e, k, v)
        self.composants.append(("TPopupMenu" if popup else "TMenuItem", o.nom))
        self.evenements_de(o)
        self.elements_menu(e, o, images)
        return e

    def elements_menu(self, e, o, images):
        for enfant in o.enfants:
            if enfant.enfants:
                self.menu(e, enfant, images)
                sous(e, "addaction", name=enfant.nom)
            else:
                self.feuille_menu(enfant, images)
                sous(e, "addaction", name=enfant.nom)

    # --- fiche --------------------------------------------------------------------

    def convertit(self):
        r = self.racine
        p = r.props
        ui = ET.Element("ui", version="4.0")
        sous(ui, "comment", "GÉNÉRÉ par tools/dfm2ui.py depuis src/%s.dfm : ne pas modifier à la main "
                            "avant la fin du portage de la fiche" % self.fiche)
        sous(ui, "class", r.nom)
        classe = "QMainWindow" if self.principale else "QDialog"
        racine = sous(ui, "widget", class_=classe, name=r.nom)
        self.xml_racine = racine
        largeur = p.get("ClientWidth", p.get("Width", 0))
        hauteur = p.get("ClientHeight", p.get("Height", 0))
        prop(racine, "geometry", "rect", [p.get("Left", 0), p.get("Top", 0), largeur, hauteur])
        prop(racine, "windowTitle", "string", p.get("Caption", ""))
        self.police(racine, r)
        deja = {"Left", "Top", "ClientWidth", "ClientHeight", "Width", "Height", "Caption", "Menu"}
        for k, v in p.items():
            if k in deja or k in NE_PAS_CONSERVER or k.startswith("Font.") or k.startswith("On"):
                continue
            self.dynamique(racine, k, v)
        self.evenements_de(r, "this")

        if self.principale:
            centre = sous(racine, "widget", class_="QWidget", name="centralwidget")
            barre_etat = next((o for o in r.enfants if o.classe == "TStatusBar"), None)
            h = hauteur - (barre_etat.props.get("Height", 0) if barre_etat is not None else 0)
            prop(centre, "geometry", "rect", [0, 0, largeur, h])
        else:
            centre = racine
        self.xml_centre = centre

        for o in r.enfants:
            self.objet(centre, o)
        # Menus, menus contextuels et actions
        for o in r.enfants:
            if o.classe == "TActionList":
                self.composants.append(("TActionList", o.nom))
                for a in o.enfants:
                    self.action(racine, a)
        for o in r.enfants:
            images = str(o.props.get("Images", "")) or None
            if o.classe == "TMainMenu":
                barre = sous(racine, "widget", class_="QMenuBar", name=o.nom)
                prop(barre, "geometry", "rect", [0, 0, largeur, 22])
                self.composants.append(("TMainMenu", o.nom))
                for m in o.enfants:
                    if m.enfants:
                        self.menu(barre, m, images)
                    else:
                        self.feuille_menu(m, images)
                    sous(barre, "addaction", name=m.nom)
            elif o.classe == "TPopupMenu":
                self.menu(centre, o, images, popup=True)

        self.ordre_tabulation(ui)
        if self.classes_utilisees:
            cw = sous(ui, "customwidgets")
            for classe, (base, entete) in self.classes_utilisees.items():
                c = sous(cw, "customwidget")
                sous(c, "class", classe)
                sous(c, "extends", base)
                sous(c, "header", entete)
        res = sous(ui, "resources")
        sous(res, "include", location="../ressources/images.qrc")
        sous(ui, "connections")
        indente(ui)
        texte = '<?xml version="1.0" encoding="UTF-8"?>\n' + ET.tostring(ui, encoding="unicode") + "\n"
        return texte

    def ordre_tabulation(self, ui):
        """TabOrder (par conteneur) -> <tabstops> (chaîne globale de Qt)."""
        ordre = []

        def parcourt(o):
            enfants = [e for e in o.enfants if "TabOrder" in e.props]
            enfants.sort(key=lambda e: e.props["TabOrder"])
            for e in enfants:
                if e.classe in FOCALISABLES and e.props.get("TabStop") != "False":
                    if not (e.classe == "TRadioButton" and e.props.get("TabStop") != "True"
                            and e.props.get("Checked") != "True"):
                        ordre.append(e.nom)
                if e.classe in CONTENEURS:
                    if e.classe == "TRadioGroup":
                        n = len(e.props.get("Items.Strings", []))
                        ordre.extend("%s_%d" % (e.nom, i) for i in range(n))
                    parcourt(e)
        parcourt(self.racine)
        if len(ordre) > 1:
            ts = sous(ui, "tabstops")
            for nom in ordre:
                sous(ts, "tabstop", nom)

    def entete_composants(self):
        nom = self.racine.nom
        garde = self.fiche + "_composantsH"
        lignes = ["//---------------------------------------------------------------------------",
                  "// %s_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/%s.dfm : ne pas modifier"
                  % (self.fiche, self.fiche),
                  "//",
                  "// COMPOSANTS_%s(X) : X(classe VCL, nom) pour chaque composant de la fiche,"
                  % nom,
                  "// dans l'ordre du .dfm (membres à lier aux objets créés par uic).",
                  "// EVENEMENTS_%s(X) : X(composant, événement, gestionnaire) pour chaque"
                  % nom,
                  "// gestionnaire d'événement à connecter (« this » = la fiche).",
                  "//---------------------------------------------------------------------------",
                  "#ifndef " + garde, "#define " + garde, ""]
        lignes.append("#define COMPOSANTS_%s(X) \\" % nom)
        for classe, n in self.composants:
            lignes.append("  X(%s, %s) \\" % (classe, n))
        lignes.append("")
        lignes.append("#define EVENEMENTS_%s(X) \\" % nom)
        for c, ev, g in self.evenements:
            lignes.append("  X(%s, %s, %s) \\" % (c, ev, g))
        lignes.append("")
        lignes.append("#endif")
        return "\n".join(lignes) + "\n"


# ---------------------------------------------------------------------------

def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--src", default=os.path.join(RACINE, "src"))
    p.add_argument("--sortie", default=os.path.join(RACINE, "app", "ui"))
    p.add_argument("--index", default=os.path.join(RACINE, "app", "ressources", "index_images.json"))
    p.add_argument("--verifie", action="store_true", help="n'écrit rien ; échoue si la sortie n'est pas à jour")
    p.add_argument("fiches", nargs="*", help="noms de fiches (f_simul...) ; toutes par défaut")
    args = p.parse_args()

    with open(args.index, encoding="utf-8") as f:
        index = json.load(f)
    fiches = args.fiches or sorted(os.path.splitext(n)[0] for n in os.listdir(args.src) if n.endswith(".dfm"))
    sorties = OrderedDict()
    rapport = ["Rapport de tools/dfm2ui.py (GÉNÉRÉ : ne pas modifier)", ""]
    for fiche in fiches:
        c = Conversion(os.path.join(args.src, fiche + ".dfm"), index)
        sorties[fiche + ".ui"] = c.convertit().encode("utf-8")
        sorties[fiche + "_composants.h"] = c.entete_composants().encode("utf-8")
        rapport.append("%s : %s (%s), %d composants, %d gestionnaires d'événements"
                       % (fiche, c.racine.nom, "QMainWindow" if c.principale else "QDialog",
                          len(c.composants), len(c.evenements)))
        rapport.extend(c.rapport)
    if not args.fiches:
        sorties["rapport_dfm2ui.txt"] = ("\n".join(rapport) + "\n").encode("utf-8")

    a_jour = True
    for chemin, octets in sorties.items():
        complet = os.path.join(args.sortie, chemin)
        if args.verifie:
            try:
                with open(complet, "rb") as f:
                    if f.read() != octets:
                        a_jour = False
                        print("différent : " + chemin)
            except OSError:
                a_jour = False
                print("absent : " + chemin)
        else:
            os.makedirs(os.path.dirname(complet), exist_ok=True)
            with open(complet, "wb") as f:
                f.write(octets)
    print("\n".join(rapport))
    if args.verifie and not a_jour:
        print("La sortie n'est pas à jour : relancer tools/dfm2ui.py", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
