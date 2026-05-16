#ifndef FL_I18N_H
#define FL_I18N_H

// To avoid warning C5260 in FL/Enumerations.H with Visual Studio.
// See from line 989 and around 70 that follow in the Enumeration.h file of fltk 1.4
#ifdef _MSVC_LANG
#pragma warning(push)
#pragma warning(disable : 5260)
#include <FL/Fl.H>
#pragma warning(pop)
#endif

#include <FL/Fl_File_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Input_.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Value_Output.H>
#include <libintl.h>
#include <type_traits>
#include <vector>

#include "utils.h" // Contient le define _() gettext()

// ========================================================================================================================
//  Classe template ajoutant un menu contextuel localisé (i18n/l10n) à tout widget héritier de
//  Fl_Input_, Fl_Spinner, Fl_Input_Choice, Fl_Text_Display ou Fl_Text_Editor.
// ========================================================================================================================

//  Fl_I18n_Config
//
//  Paramètres globaux partagés par toutes les instances.
//  Modifiez-les avant de créer les widgets, ou utilisez
//  set_selection_color() sur chaque instance individuelle.
// ============================================================

struct Fl_I18n_Config
{
  // Couleur de fond du texte sélectionné.
  // FL_BLUE est plus visible que le gris par défaut de FLTK.
  static Fl_Color selection_color;
};

// Définition de la valeur par défaut.
// Si ce header est inclus depuis plusieurs unités de traduction,
// déplacez cette ligne dans un fichier .cxx pour éviter
// les définitions multiples (ODR).
inline Fl_Color Fl_I18n_Config::selection_color = FL_BLUE;

// ============================================================
//  Traits de détection des trois familles supportées

template <class T> struct fl_is_input_family : std::is_base_of<Fl_Input_, T>
{
};

template <class T> struct fl_is_spinner_family : std::is_base_of<Fl_Spinner, T>
{
};

template <class T> struct fl_is_input_choice_family : std::is_base_of<Fl_Input_Choice, T>
{
};

template <class T> struct fl_is_text_display_family : std::is_base_of<Fl_Text_Display, T>
{
};

// ============================================================
//  I18nContextMenu<Derived>
//
//  Mixin CRTP pur : logique d'affichage du menu contextuel.
//  Délègue toutes les opérations réelles à Derived via *_impl().
//
//  Points d'extension attendus dans Derived :
//    bool has_selection_impl()  const
//    bool is_readonly_impl()    const
//    bool allows_copy_impl()    const
//    void cut_impl()
//    void copy_impl()
//    void paste_impl()
//    void select_all_impl()
// ============================================================

template <class Derived> class I18nContextMenu
{
protected:
  void show_context_menu(Fl_Widget *target)
  {
    Derived *self = static_cast<Derived *>(this);

    const bool has_selection = self->has_selection_impl();
    const bool is_readonly = self->is_readonly_impl();
    const bool can_copy = self->allows_copy_impl();
    const bool has_clipboard = (Fl::clipboard_contains(Fl::clipboard_plain_text) != 0);

    std::vector<Fl_Menu_Item> items;
    items.reserve(5);

    // Couper : sélection non vide ET modifiable ET copie autorisée
    if (has_selection && !is_readonly && can_copy)
      items.push_back({_("Cut"), 0, cut_cb, target, {}, {}, {}, {}, {}});

    // Copier : sélection non vide ET copie autorisée
    if (has_selection && can_copy)
      items.push_back({_("Copy"), 0, copy_cb, target, {}, {}, {}, {}, {}});

    // Coller : presse-papier non vide ET modifiable
    if (has_clipboard && !is_readonly)
      items.push_back({_("Paste"), 0, paste_cb, target, {}, {}, {}, {}, {}});

    // Tout sélectionner : toujours présent
    items.push_back({_("Select all"), 0, select_all_cb, target, {}, {}, {}, {}, {}});

    // Sentinel obligatoire pour Fl_Menu_Item::popup()
    items.push_back({{}, {}, {}, {}, {}, {}, {}, {}, {}});

    // N'afficher le menu que s'il y a au moins un item utile
    // (autre que "Select all" et le sentinel)
    const Fl_Menu_Item *choice = items.data()->popup(Fl::event_x(), Fl::event_y());
    if (choice && choice->callback_)
      choice->do_callback(target);
  }

private:
  static void cut_cb(Fl_Widget *, void *v)
  {
    static_cast<Derived *>(v)->cut_impl();
  }
  static void copy_cb(Fl_Widget *, void *v)
  {
    static_cast<Derived *>(v)->copy_impl();
  }
  static void paste_cb(Fl_Widget *, void *v)
  {
    static_cast<Derived *>(v)->paste_impl();
  }
  static void select_all_cb(Fl_Widget *, void *v)
  {
    static_cast<Derived *>(v)->select_all_impl();
  }
};

// ============================================================
//  Fl_I18n<Base>  — déclaration primaire (non définie)
//
//  Seules les trois spécialisations ci-dessous sont valides.
//  Utiliser Fl_I18n avec une Base non supportée provoque une
//  erreur de compilation explicite.
// ============================================================

template <class Base, class Enable = void> class Fl_I18n
{
  static_assert(fl_is_input_family<Base>::value || fl_is_spinner_family<Base>::value || fl_is_input_choice_family<Base>::value || fl_is_text_display_family<Base>::value, "Fl_I18n : Base doit hériter de Fl_Input_, "
                                                                                                                                                                          "Fl_Spinner, Fl_Input_Choice ou Fl_Text_Display.");
};

// ============================================================
//  Spécialisation 1 — famille Fl_Input_
//  Couvre : Fl_Input, Fl_Multiline_Input, Fl_Float_Input,
//           Fl_Int_Input, et toute sous-classe.
//  (Fl_Secret_Input a sa propre spécialisation explicite.)
// ============================================================

template <class Base> class Fl_I18n<Base, std::enable_if_t<fl_is_input_family<Base>::value>> : public Base, public I18nContextMenu<Fl_I18n<Base>>
{
  friend class I18nContextMenu<Fl_I18n<Base>>;

public:
  // Transmission parfaite de tous les constructeurs de Base
  template <typename... Args> explicit Fl_I18n(Args &&...args) : Base(std::forward<Args>(args)...)
  {
    this->selection_color(Fl_I18n_Config::selection_color);
  }

  // Permet de changer la couleur de sélection après construction.
  void set_selection_color(Fl_Color c)
  {
    this->selection_color(c);
  }

  int handle(int event) override
  {
    if (event == FL_PUSH && Fl::event_button() == FL_RIGHT_MOUSE)
    {
      this->show_context_menu(this);
      return 1;
    }
    return Base::handle(event);
  }

protected:
  // Points d'extension — comportement par défaut
  bool has_selection_impl() const
  {
    return this->insert_position() != this->mark();
  }
  bool is_readonly_impl() const
  {
    return this->readonly();
  }
  // Surcharger dans une sous-classe pour interdire la copie
  // (ex. Fl_Secret_Input — voir spécialisation explicite plus bas)
  bool allows_copy_impl() const
  {
    return true;
  }
  void cut_impl()
  {
    this->copy(1);
    this->cut();
  }
  void copy_impl()
  {
    this->copy(1);
  }
  void paste_impl()
  {
    Fl::paste(*this, 1);
  }
  void select_all_impl()
  {
    // mark = size → position = 0 : tout est sélectionné
    this->insert_position(this->size(), 0);
  }
};

// ============================================================
//  Spécialisation explicite — Fl_Secret_Input
//
//  Un champ mot de passe interdit Couper et Copier.
//  La couleur de sélection reste appliquée (surligne la zone
//  de saisie sans révéler le contenu).
// ============================================================

template <> class Fl_I18n<Fl_Secret_Input, std::enable_if_t<fl_is_input_family<Fl_Secret_Input>::value>> : public Fl_Secret_Input, public I18nContextMenu<Fl_I18n<Fl_Secret_Input>>
{
  friend class I18nContextMenu<Fl_I18n<Fl_Secret_Input>>;

public:
  template <typename... Args> explicit Fl_I18n(Args &&...args) : Fl_Secret_Input(std::forward<Args>(args)...)
  {
    this->selection_color(Fl_I18n_Config::selection_color);
  }

  void set_selection_color(Fl_Color c)
  {
    this->selection_color(c);
  }

  int handle(int event) override
  {
    if (event == FL_PUSH && Fl::event_button() == FL_RIGHT_MOUSE)
    {
      this->show_context_menu(this);
      return 1;
    }
    return Fl_Secret_Input::handle(event);
  }

protected:
  bool has_selection_impl() const
  {
    return insert_position() != mark();
  }
  bool is_readonly_impl() const
  {
    return readonly();
  }
  bool allows_copy_impl() const
  {
    return false;
  } // ← interdit
  void cut_impl()
  { /* jamais appelé : allows_copy_impl() == false */
  }
  void copy_impl()
  { /* jamais appelé : allows_copy_impl() == false */
  }
  void paste_impl()
  {
    Fl::paste(*this, 1);
  }
  void select_all_impl()
  {
    insert_position(size(), 0);
  }
};

// ============================================================
//  Spécialisation 2 — famille Fl_Spinner
//  Fl_Spinner hérite de Fl_Group. Son Fl_Input interne est
//  accessible via child(0) car Fl_Spinner::input() est protected.
//  La couleur de sélection est appliquée sur ce widget interne.
//  Le menu n'apparaît que si le clic est dans la zone texte,
//  pas sur les boutons haut/bas.
// ============================================================

template <class Base> class Fl_I18n<Base, std::enable_if_t<fl_is_spinner_family<Base>::value>> : public Base, public I18nContextMenu<Fl_I18n<Base>>
{
  friend class I18nContextMenu<Fl_I18n<Base>>;

public:
  template <typename... Args> explicit Fl_I18n(Args &&...args) : Base(std::forward<Args>(args)...)
  {
    input_widget()->selection_color(Fl_I18n_Config::selection_color);
  }

  void set_selection_color(Fl_Color c)
  {
    input_widget()->selection_color(c);
  }

  int handle(int event) override
  {
    if (event == FL_PUSH && Fl::event_button() == FL_RIGHT_MOUSE)
    {
      // N'afficher le menu que si le clic est dans la zone texte,
      // pas sur les boutons haut/bas du spinner.
      if (Fl::event_inside(input_widget()))
      {
        this->show_context_menu(this);
        return 1;
      }
    }
    return Base::handle(event);
  }

protected:
  // child(0) est toujours le Fl_Input dans l'implémentation FLTK
  // de référence (Fl_Spinner::input() étant protected).
  Fl_Input *input_widget() const
  {
    return static_cast<Fl_Input *>(this->child(0));
  }

  bool has_selection_impl() const
  {
    const Fl_Input *i = input_widget();
    return i->insert_position() != i->mark();
  }
  bool is_readonly_impl() const
  {
    return input_widget()->readonly();
  }
  bool allows_copy_impl() const
  {
    return true;
  }
  void cut_impl()
  {
    Fl_Input *i = input_widget();
    i->copy(1);
    i->cut();
  }
  void copy_impl()
  {
    input_widget()->copy(1);
  }
  void paste_impl()
  {
    Fl::paste(*input_widget(), 1);
  }
  void select_all_impl()
  {
    Fl_Input *i = input_widget();
    i->insert_position(i->size(), 0);
  }
};

// ============================================================
//  Spécialisation 3 — famille Fl_Input_Choice
//
//  Fl_Input_Choice hérite de Fl_Group. Contrairement à
//  Fl_Spinner, elle expose une méthode publique input() qui
//  retourne son Fl_Input interne (API officielle FLTK 1.3+) :
//    Fl_Input* Fl_Input_Choice::input()
//  On l'utilise directement, sans recourir à child(N).
//  Le menu n'apparaît que si le clic est dans la zone texte,
//  pas sur le bouton de menu déroulant à droite.
// ============================================================

template <class Base> class Fl_I18n<Base, std::enable_if_t<fl_is_input_choice_family<Base>::value>> : public Base, public I18nContextMenu<Fl_I18n<Base>>
{
  friend class I18nContextMenu<Fl_I18n<Base>>;

public:
  template <typename... Args> explicit Fl_I18n(Args &&...args) : Base(std::forward<Args>(args)...)
  {
    // Fl_Input_Choice::input() n'est pas const dans FLTK 1.4 :
    // on met en cache le pointeur à la construction pour pouvoir
    // l'utiliser sans contrainte de constance dans les méthodes const.
    inp_ = this->input();
    inp_->selection_color(Fl_I18n_Config::selection_color);
  }

  void set_selection_color(Fl_Color c)
  {
    inp_->selection_color(c);
  }

  int handle(int event) override
  {
    if (event == FL_PUSH && Fl::event_button() == FL_RIGHT_MOUSE)
    {
      // N'afficher le menu que si le clic est dans la zone texte,
      // pas sur le bouton de menu déroulant à droite.
      if (Fl::event_inside(inp_))
      {
        this->show_context_menu(this);
        return 1;
      }
    }
    return Base::handle(event);
  }

protected:
  bool has_selection_impl() const
  {
    return inp_->insert_position() != inp_->mark();
  }
  bool is_readonly_impl() const
  {
    return inp_->readonly();
  }
  bool allows_copy_impl() const
  {
    return true;
  }
  void cut_impl()
  {
    inp_->copy(1);
    inp_->cut();
  }
  void copy_impl()
  {
    inp_->copy(1);
  }
  void paste_impl()
  {
    Fl::paste(*inp_, 1);
  }
  void select_all_impl()
  {
    inp_->insert_position(inp_->size(), 0);
  }

private:
  // Pointeur mis en cache au constructeur.
  // Fl_Input_Choice::input() n'étant pas const, le stocker ici
  // permet de l'utiliser depuis les méthodes const sans const_cast.
  Fl_Input *inp_ = nullptr;
};

// ============================================================
//  Spécialisation 4 — famille Fl_Text_Display / Fl_Text_Editor
//
//  Ces deux widgets héritent de Fl_Group et utilisent une
//  architecture radicalement différente des Fl_Input_ :
//    - La sélection est gérée par le Fl_Text_Buffer associé
//      via buffer()->selected() et buffer()->selection_text().
//    - Les opérations cut/copy/paste/select_all passent par
//      les fonctions statiques kf_*() de Fl_Text_Editor.
//    - Fl_Text_Display est en lecture seule : Couper et Coller
//      n'y sont jamais disponibles ; seul Copier l'est.
//    - Fl_Text_Editor est éditable : toutes les opérations
//      sont disponibles selon le contexte habituel.
//    - La couleur de sélection se règle via
//      selection_color() directement sur le widget.
//
//  Détection de la sous-famille (display vs editor) :
//    is_editor_impl() retourne true uniquement pour les
//    descendants de Fl_Text_Editor.
// ============================================================

template <class Base> class Fl_I18n<Base, std::enable_if_t<fl_is_text_display_family<Base>::value>> : public Base, public I18nContextMenu<Fl_I18n<Base>>
{
  friend class I18nContextMenu<Fl_I18n<Base>>;

public:
  template <typename... Args> explicit Fl_I18n(Args &&...args) : Base(std::forward<Args>(args)...)
  {
    this->selection_color(Fl_I18n_Config::selection_color);
  }

  void set_selection_color(Fl_Color c)
  {
    this->selection_color(c);
  }

  int handle(int event) override
  {
    if (event == FL_PUSH && Fl::event_button() == FL_RIGHT_MOUSE)
    {
      this->show_context_menu(this);
      return 1;
    }
    return Base::handle(event);
  }

protected:
  // Détecte si Base est un Fl_Text_Editor (éditable) ou
  // un simple Fl_Text_Display (lecture seule).
  bool is_editor_impl() const
  {
    return std::is_base_of<Fl_Text_Editor, Base>::value;
  }

  bool has_selection_impl() const
  {
    // La sélection est portée par le Fl_Text_Buffer associé.
    // buffer() peut retourner nullptr si aucun buffer n'est attaché.
    const Fl_Text_Buffer *buf = this->buffer();
    return buf && buf->selected();
  }

  bool is_readonly_impl() const
  {
    // Fl_Text_Display est toujours en lecture seule ;
    // Fl_Text_Editor hérite de Fl_Text_Display mais est éditable.
    return !is_editor_impl();
  }

  bool allows_copy_impl() const
  {
    return true; // Copier est autorisé sur display et editor
  }

  void cut_impl()
  {
    // Uniquement appelé si is_editor_impl() == true (via is_readonly_impl())
    Fl_Text_Editor *ed = static_cast<Fl_Text_Editor *>(this);
    Fl_Text_Editor::kf_cut(0, ed);
  }

  void copy_impl()
  {
    // kf_copy est défini sur Fl_Text_Editor, mais la sélection
    // et le buffer sont accessibles depuis Fl_Text_Display.
    // On passe par le buffer directement pour être compatible
    // avec les instances purement Fl_Text_Display.
    Fl_Text_Buffer *buf = this->buffer();
    if (!buf)
      return;
    char *sel = buf->selection_text();
    if (sel)
    {
      Fl::copy(sel, (int)strlen(sel), 1);
      free(sel);
    }
  }

  void paste_impl()
  {
    // Uniquement appelé si is_editor_impl() == true
    Fl_Text_Editor *ed = static_cast<Fl_Text_Editor *>(this);
    Fl_Text_Editor::kf_paste(0, ed);
  }

  void select_all_impl()
  {
    Fl_Text_Buffer *buf = this->buffer();
    if (buf)
      buf->select(0, buf->length());
  }
};

// ============================================================
//  Alias prêts à l'emploi
// ============================================================

using Fl_Input_L10n = Fl_I18n<Fl_Input>;
using Fl_Int_Input_L10n = Fl_I18n<Fl_Int_Input>;
using Fl_Multiline_Input_L10n = Fl_I18n<Fl_Multiline_Input>;
using Fl_Float_Input_L10n = Fl_I18n<Fl_Float_Input>;
using Fl_Multiline_Output_L10n = Fl_I18n<Fl_Multiline_Output>;
using Fl_Output_L10n = Fl_I18n<Fl_Output>;
using Fl_Secret_Input_L10n = Fl_I18n<Fl_Secret_Input>;
using Fl_Spinner_L10n = Fl_I18n<Fl_Spinner>;
using Fl_Value_Input_L10n = Fl_I18n<Fl_Value_Input>;
using Fl_Value_Output_L10n = Fl_I18n<Fl_Value_Output>;
using Fl_Input_Choice_L10n = Fl_I18n<Fl_Input_Choice>;
using Fl_Text_Display_L10n = Fl_I18n<Fl_Text_Display>;
using Fl_Text_Editor_L10n = Fl_I18n<Fl_Text_Editor>;

// ============================================================
//  Tableau récapitulatif des items affichés selon le contexte
//
//  has_sel  readonly  can_copy  clipboard  │ Couper Copier Coller  ToutSél
//  ─────────────────────────────────────────┼──────────────────────────────
//    non      *          *        non       │                        ✓
//    non      *          *        oui       │                ✓       ✓
//    oui      oui        oui      non       │         ✓              ✓
//    oui      oui        oui      oui       │         ✓      ✓       ✓
//    oui      non        oui      non       │  ✓      ✓              ✓
//    oui      non        oui      oui       │  ✓      ✓      ✓       ✓
//    oui      *          non      non       │                        ✓  (Secret)
//    oui      *          non      oui       │                ✓       ✓  (Secret)
//
//  Widgets de type Fl_Group (Fl_Spinner, Fl_Input_Choice) :
//    le menu n'apparaît que si le clic est dans la zone texte,
//    pas sur les boutons haut/bas ou le menu déroulant.
//
//  Fl_Text_Display (lecture seule) :
//    is_readonly_impl() == true → Couper et Coller jamais affichés.
//    Copier disponible si sélection non vide.
//
//  Fl_Text_Editor (éditable) :
//    Comportement identique aux Fl_Input_ éditables.
//    cut/paste passent par kf_cut()/kf_paste() de Fl_Text_Editor.
//    copy() passe par Fl_Text_Buffer::selection_text() + Fl::copy()
//    pour rester compatible avec Fl_Text_Display.
//    Requiert qu'un Fl_Text_Buffer soit attaché (buffer() != nullptr).
// ============================================================
#endif /* FL_I18N_H */
