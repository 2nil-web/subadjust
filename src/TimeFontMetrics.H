#ifndef TIME_FONT_METRICS_H
#define TIME_FONT_METRICS_H
// TimeFontMetrics.H
// Dimensions dérivées de la police, partagées par Fl_Time_Input et Fl_Time_Picker.
#include <FL/fl_draw.H>
#include <FL/Enumerations.H>

struct TimeFontMetrics {
    Fl_Font font;
    int     size;
    int     ch;      // hauteur de cellule (fl_height + 2*pad)
    int     cw_num;  // largeur de "00" + 2*pad
    int     cw_ms;   // largeur de "000" + 2*pad  (millisecondes)
    int     cw_ampm; // largeur de "AM/PM" + 2*pad
    int     sep_w;   // largeur de ":" + pad
    int     dot_w;   // largeur de "." + pad  (séparateur millisecondes)
    int     pad;     // marge interne (~fl_height/3)

    void measure(Fl_Font f, int s) {
        font = f;
        size = s;
        fl_font(f, s);
        int fh = fl_height();
        pad     = fh / 4;
        ch      = fh + pad;
        cw_num  = (int)fl_width("00")    + pad;
        cw_ms   = (int)fl_width("000")   + pad;
        cw_ampm = (int)fl_width("AM/PM") + pad;
        sep_w   = (int)fl_width(":")     + pad;
        dot_w   = (int)fl_width(".")     + pad;
    }
};

#endif // TIME_FONT_METRICS_H
