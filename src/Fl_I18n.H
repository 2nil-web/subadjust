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
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Value_Output.H>
#include <libintl.h>
#include <type_traits>
#include <vector>

#include "utils.h" // Contient le define _() gettext()

// ========================================================================================================================
//  Classe template ajoutant un menu contextuel localisé (i18n/l10n) à tout widget héritier de Fl_Input_ ou de Fl_Spinner.
// ========================================================================================================================

//  Traits de détection des deux familles supportées

template <class T> struct fl_is_input_family : std::is_base_of<Fl_Input_, T>
{
};

template <class T> struct fl_is_spinner_family : std::is_base_of<Fl_Spinner, T>
{
};

// ============================================================
//  I18nContextMenu<Derived>
//
//  Mixin CRTP pur : contient toute la logique d'affichage du
//  menu contextuel. Délègue les opérations réelles à la classe
//  dérivée via les méthodes *_impl().
//
//  Points d'extension attendus dans Derived :
//    bool has_selection_impl() const
//    bool is_readonly_impl()   const
//    bool allows_copy_impl()   const   (facultatif, défaut true)
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
      items.push_back({_("Cut"), 0, cut_cb, target});

    // Copier : sélection non vide ET copie autorisée
    if (has_selection && can_copy)
      items.push_back({_("Copy"), 0, copy_cb, target});

    // Coller : presse-papier non vide ET modifiable
    if (has_clipboard && !is_readonly)
      items.push_back({_("Paste"), 0, paste_cb, target});

    // Tout sélectionner : toujours présent
    items.push_back({_("Select all"), 0, select_all_cb, target});

    // Sentinel obligatoire pour Fl_Menu_Item::popup()
    items.push_back({0});

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
//  Seules les deux spécialisations ci-dessous sont valides.
//  Utiliser Fl_I18n avec une Base non supportée provoque une
//  erreur de compilation explicite.
// ============================================================

template <class Base, class Enable = void> class Fl_I18n
{
  static_assert(fl_is_input_family<Base>::value || fl_is_spinner_family<Base>::value, "Fl_I18n : Base doit hériter de Fl_Input_ ou de Fl_Spinner.");
};

// ============================================================
//  Spécialisation 1 — famille Fl_Input_
//  Couvre : Fl_Input, Fl_Multiline_Input, Fl_Float_Input,
//           Fl_Int_Input, Fl_Secret_Input, et toute sous-classe.
// ============================================================

template <class Base> class Fl_I18n<Base, std::enable_if_t<fl_is_input_family<Base>::value>> : public Base, public I18nContextMenu<Fl_I18n<Base>>
{
  friend class I18nContextMenu<Fl_I18n<Base>>;

public:
  // Transmission parfaite de tous les constructeurs de Base
  template <typename... Args> explicit Fl_I18n(Args &&...args) : Base(std::forward<Args>(args)...)
  {
    //    Base::selection_color(FL_BLUE);
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
//  Spécialisation 2 — famille Fl_Spinner
//  Fl_Spinner hérite de Fl_Group, pas de Fl_Input_.
//  Son Fl_Input interne est accessible via child(0).
// ============================================================

template <class Base> class Fl_I18n<Base, std::enable_if_t<fl_is_spinner_family<Base>::value>> : public Base, public I18nContextMenu<Fl_I18n<Base>>
{
  friend class I18nContextMenu<Fl_I18n<Base>>;

public:
  template <typename... Args> explicit Fl_I18n(Args &&...args) : Base(std::forward<Args>(args)...)
  {
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
  // Fl_Spinner::input() est protected → on accède via child(0),
  // qui est toujours le Fl_Input dans l'implémentation de référence.
  // Si vous avez accès aux sources FLTK, vous pouvez sous-classer
  // Fl_Spinner pour exposer input() et remplacer ce cast.
  Fl_Input *input_widget() const
  {
    // Fl_Spinner::input() est protected → on passe par les enfants du groupe
    // child(0) est le Fl_Input dans l'implémentation de référence FLTK
    this->child(0)->selection_color(FL_SELECTION_COLOR);
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
//  Spécialisation explicite — Fl_Secret_Input
//
//  Un champ mot de passe ne doit jamais autoriser Couper/Copier.
//  On surcharge uniquement allows_copy_impl() ; tout le reste
//  est hérité de la spécialisation Fl_Input_.
// ============================================================

template <> class Fl_I18n<Fl_Secret_Input, std::enable_if_t<fl_is_input_family<Fl_Secret_Input>::value>> : public Fl_Secret_Input, public I18nContextMenu<Fl_I18n<Fl_Secret_Input>>
{
  friend class I18nContextMenu<Fl_I18n<Fl_Secret_Input>>;

public:
  template <typename... Args> explicit Fl_I18n(Args &&...args) : Fl_Secret_Input(std::forward<Args>(args)...)
  {
    //    Fl_Secret_Input::selection_color(FL_BLUE);
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
//  Utilisation :
//    #include "Fl_I18n.H"
//    using Fl_Input_L10n  = Fl_I18n<Fl_Input>;
//    using Fl_Spinner_L10n = Fl_I18n<Fl_Spinner>;
//
using Fl_Float_Input_L10n = Fl_I18n<Fl_Float_Input>;
// using Fl_Input_Choice_L10n = Fl_I18n<Fl_Input_Choice>;
using Fl_Input_L10n = Fl_I18n<Fl_Input>;
using Fl_Int_Input_L10n = Fl_I18n<Fl_Int_Input>;
using Fl_Multiline_Input_L10n = Fl_I18n<Fl_Multiline_Input>;
using Fl_Multiline_Output_L10n = Fl_I18n<Fl_Multiline_Output>;
using Fl_Output_L10n = Fl_I18n<Fl_Output>;
using Fl_Secret_Input_L10n = Fl_I18n<Fl_Secret_Input>;
using Fl_Spinner_L10n = Fl_I18n<Fl_Spinner>;
using Fl_Value_Input_L10n = Fl_I18n<Fl_Value_Input>;
using Fl_Value_Output_L10n = Fl_I18n<Fl_Value_Output>;

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
//    oui      *          non      non       │                        ✓  (Fl_Secret_Input)
//    oui      *          non      oui       │                ✓       ✓  (Fl_Secret_Input)
// ============================================================
#endif /* FL_I18N_H */
