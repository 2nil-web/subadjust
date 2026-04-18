#ifndef FL_TIME_PICKER_H
#define FL_TIME_PICKER_H
// Fl_Time_Picker.H
// Popup à colonnes déroulantes. Colonnes alignées sur celles du widget parent.
// Fermeture sans validation : clic extérieur ou Échap (via Fl::grab).
// Validation : clic OK ou Entrée.
#include <FL/Fl_Window.H>
#include <functional>
#include "TimeFontMetrics.H"

class Fl_Time_Picker : public Fl_Window {
public:
    struct Time { int h, m, s, ms; bool pm; };

    std::function<void(Time)> on_accept;

    Fl_Time_Picker();

    // Synchronise métriques + configuration avant chaque ouverture.
    void set_config(const TimeFontMetrics& fm,
                    bool use24, bool show_ms,
                    const int* col_x, const int* col_w, int ncols);

    void show_at(int screen_x, int screen_y, Time t);

private:
    static const int ROWS_V = 5;

    TimeFontMetrics fm_;
    bool            use24_;    // true = 24h, false = 12h AM/PM
    bool            show_ms_;  // true = affiche les millisecondes

    Time cur_;
    int  sc_[5]; // index sélectionné par colonne (max 5 : H M S MS AMPM)

    // Colonnes : positions/largeurs alignées sur le widget parent.
    // ncols_ vaut 3 (HH MM SS), 4 (+AMPM ou +MS), ou 5 (+MS +AMPM).
    int ncols_;
    int col_x_[5];
    int col_w_[5];

    int header_h_;
    int footer_h_;
    int btn_h_;
    int btn_w_;

    // Nombre d'items par colonne selon configuration.
    int col_count(int col) const;
    // Index de la colonne logique : 0=H 1=M 2=S 3=MS(opt) 4=AMPM(opt)
    // Retourne -1 si la colonne n'existe pas dans la config courante.
    int ampm_col() const;
    int ms_col()   const;

    void layout();

    void clamp_scroll(int col);
    void update_cur_from_scroll();
    void accept_and_hide();
    void cancel_and_hide();

    void draw()         override;
    int  handle(int ev) override;
    void draw_col(int col, int cx, int cw);

    int clip_top()    const { return header_h_; }
    int clip_bottom() const { return header_h_ + ROWS_V * fm_.ch; }
    int total_w()     const;
    int col_at(int mx) const;
    void item_label(int col, int idx, char* buf, int bufsz) const;

    int ok_x()  const { return (total_w() - btn_w_) / 2; }
    int btn_y() const { return h() - footer_h_ + (footer_h_ - btn_h_) / 2; }
};

#endif // FL_TIME_PICKER_H
