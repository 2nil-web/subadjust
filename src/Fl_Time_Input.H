#ifndef FL_TIME_INPUT_H
#define FL_TIME_INPUT_H
// Fl_Time_Input.H
// Widget FLTK imitant <input type="time"> de Chrome.
// Dimensions proportionnelles à la police (labelfont/labelsize).
// Système horaire et millisecondes configurables.
//
// Compilation :
//   g++ main.cpp Fl_Time_Picker.cpp Fl_Time_Input.cpp \
//       -o demo $(fltk-config --cxxflags --ldflags)
#include <string>
#include <FL/Fl_Group.H>
#include "TimeFontMetrics.H"
#include "Fl_Time_Picker.H"

// Segments logiques. SEG_MS et SEG_AMPM peuvent être absents selon config.
enum TimeSegment {
    SEG_HOUR = 0,
    SEG_MIN  = 1,
    SEG_SEC  = 2,
    SEG_MS   = 3,   // millisecondes (optionnel)
    SEG_AMPM = 4,   // AM/PM (optionnel, absent en mode 24h)
    SEG_NONE = 5
};

class Fl_Time_Input : public Fl_Group {
public:
    // use24  : true = horloge 24h (défaut), false = 12h AM/PM
    // show_ms: true = affiche HH:MM:SS.mmm
    Fl_Time_Input(int x, int y, int w, int h,
                  const char* label = nullptr,
                  bool use24   = true,
                  bool show_ms = true);
    ~Fl_Time_Input();

    // Reconfiguration dynamique (appelle fit() en interne).
    void set_mode(bool use24, bool show_ms);

    // Valeur — h est toujours 0..23 en mode 24h, 1..12 en mode 12h.
    void set_time_ms(int ms);
    void set_time(int hour, int min, int sec, int ms = 0, bool pm = false);
    // Set time with a string of the form HH:MM:SS./,SSS AM/PM
    void set_time_str(std::string s);

    // Get time by returning int for hours (1->12), minutes, seconds and milliseconds and a boolean to true to indicate PM or false for AM
    //void get_time(int&, int&, int&, int&, bool&);
    // Get time by returning int for hours(0->23), minutes, seconds and milliseconds
    void get_time(int&, int&, int&, int&);
    int get_time_ms();

    // Get time by returning a string of the form HH:MM:SS./,SSS AM/PM
    std::string get_time_str();

    const char* value() { return get_time_str().c_str(); };
    void value(const char *p) { if (p) set_time_str(std::string(p)); };
    void value(const std::string s) { set_time_str(s); };

    int  hour()   const { return h_; }
    int  minute() const { return m_; }
    int  second() const { return s_; }
    int  millis() const { return ms_; }
    bool is_pm()  const { return pm_; }
    // Toujours en 24h quelle que soit la config.
    int  hour24() const;

    // Copie la valeur courante dans le presse-papier (format HH:MM:SS[.mmm]).
    void copy_value();
    // Tente de coller depuis le presse-papier.
    void paste_value();

    void preferred_size(int& pw, int& ph) const;
    void fit();
    void resize(int x, int y, int w, int h) override;

    void callback(Fl_Callback* cb, void* data = nullptr);

private:
    int         h_, m_, s_, ms_;
    bool        pm_;
    bool        use24_;
    bool        show_ms_;
    TimeSegment active_seg_;
    // Pendant la saisie numérique : chiffres déjà entrés (0..3).
    int         digit_count_;
    int         digit_accum_;

    Fl_Time_Picker* picker_;
    Fl_Callback*    user_cb_;
    void*           user_data_;

    TimeFontMetrics fm_;

    // Nombre de segments visibles (3, 4 ou 5).
    int nseg() const;
    // Valeur maximale du segment actif.
    int seg_max(TimeSegment s) const;
    // Prochain / précédent segment valide.
    TimeSegment next_seg(TimeSegment s) const;
    TimeSegment prev_seg(TimeSegment s) const;

    struct Rects {
        int fx, fy, fw, fh;
        int sx, sy, sw, sh;
        int cx, cy, cw, ch;
        int seg_x[5], seg_w[5];
        int nseg;
    } r_;

    void measure_font();
    void compute_rects();

    void draw()         override;
    int  handle(int ev) override;

    void draw_segment(const char* txt, int x, int y, int w, int h, bool active);
    void draw_spin_button(int x, int y, int w, int h);
    void draw_clock_button(int x, int y, int w, int h);

    void activate_segment(TimeSegment s);
    void increment(int delta);
    void commit_digit(int digit);
    void flush_digit();
    void open_picker();
    void fire_callback();
    void clamp_values();

    // Formate la valeur courante en chaîne pour le presse-papier.
    void format_clipboard(char* buf, int bufsz) const;
    // Parse une chaîne du presse-papier et met à jour h_..ms_, pm_.
    bool parse_clipboard(const char* str);
};

#endif // FL_TIME_INPUT_H
