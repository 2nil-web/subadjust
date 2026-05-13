
#include "my_ask.h"

class MyBox : public Fl_Box {
    Fl_Font     _font;
    Fl_Fontsize _size;
public:
    MyBox(int x, int y, int w, int h, const char* l, Fl_Font font, Fl_Fontsize size)
        : Fl_Box(x, y, w, h, l), _font(font), _size(size) {}

    void draw() override {
        // Dessiner le fond
        draw_box();
        // Forcer la police et dessiner le texte DIRECTEMENT sans passer par le mécanisme de label FLTK
        fl_font(_font, _size);
        fl_color(labelcolor());
        fl_draw(label(), x(), y(), w(), h(), align(), nullptr, 0);
    }
};

// ─────────────────────────────────────────────
// Fonction interne commune
// ─────────────────────────────────────────────

// Retourne l'index du bouton cliqué, ou -1 si fermé/Escape
// Si labels est vide, aucun bouton n'est créé (mode my_message)
static int my_dialog(Fl_Font font, Fl_Fontsize size, const char* msg, const std::vector<std::string>& labels)
{
    const int MAX_W    = 800;
    const int MARGIN   = 20;
    const int BTN_H    = 20;
    const int BTN_W    = 40;
    const int PADDING  = 10;
    const int BTN_GAP = 10;

    // --- Mesurer le texte ---
    fl_font(font, size);
    int text_w = MAX_W - 2 * MARGIN;
    int text_h = 0;
    fl_measure(msg, text_w, text_h, 1);

    // --- Dimensions fenêtre ---
    int total_btn_w = labels.empty() ? BTN_W  // bouton OK par défaut
                    : (int)labels.size() * BTN_W
                    + ((int)labels.size() - 1) * BTN_GAP;

    int win_w = max(text_w + 2 * MARGIN, total_btn_w + 2 * MARGIN);
    int win_h = PADDING + text_h + PADDING + BTN_H + PADDING;
    win_w     = max(win_w, 300);
    win_h     = max(win_h, 150);

    // --- Données partagées des callbacks ---
    struct CallbackData {
        int        result;
        Fl_Window* win;
    };
    CallbackData cbd = { -1, nullptr };

    // --- Fenêtre ---
    Fl_Window* win = new Fl_Window(win_w, win_h, "");
    cbd.win = win;
    win->set_modal();

    // --- Boîte de message ---
    MyBox* box = new MyBox(MARGIN, PADDING, text_w, text_h, msg, font, size);
    box->align(FL_ALIGN_WRAP | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);

    // --- Boutons ---
    // Utiliser les labels fournis, ou un bouton "OK" par défaut
    const std::vector<std::string> effective_labels = labels.empty() ? std::vector<std::string>{ "OK" } : labels;

    int total_w = (int)effective_labels.size() * BTN_W + ((int)effective_labels.size() - 1) * BTN_GAP;
    int start_x = (win_w - total_w) / 2;
    int btn_y   = PADDING + text_h + PADDING;

    struct BtnData {
        CallbackData* cbd;
        int           index;
    };

    for (int i = 0; i < (int)effective_labels.size(); i++) {
        int btn_x = start_x + i * (BTN_W + BTN_GAP);

        Fl_Hover_Button* btn = new Fl_Hover_Button(btn_x, btn_y, BTN_W, BTN_H, nullptr);
        btn->box(FL_BORDER_BOX);
        btn->color(FL_BACKGROUND_COLOR);
        btn->selection_color(FL_BACKGROUND_COLOR);
        btn->labeltype(FL_NORMAL_LABEL);
        btn->labelfont(font);
        btn->labelsize(size);
        btn->labelcolor(FL_FOREGROUND_COLOR);
        btn->align(Fl_Align(FL_ALIGN_CENTER));
        btn->when(FL_WHEN_RELEASE);

        btn->copy_label(effective_labels[i].c_str());

        BtnData* bd = new BtnData{ &cbd, i };
        btn->callback([](Fl_Widget*, void* data) {
            BtnData* bd = (BtnData*)data;
            bd->cbd->result = bd->index;
            bd->cbd->win->hide();
            delete bd;
        }, bd);
    }

    // --- Callback fermeture fenêtre (Escape, croix) ---
    win->callback([](Fl_Widget* w, void* data) {
        CallbackData* cbd = (CallbackData*)data;
        cbd->result = -1;
        w->hide();
    }, &cbd);

    win->end();
    win->show();

    while (win->shown())
        Fl::wait();

    delete win;
    return cbd.result;
}

void my_message(const char* msg, Fl_Font font, Fl_Fontsize size)
{
    my_dialog(font, size, msg, {});
}


// Retourne l'index du bouton cliqué (0, 1, 2...), ou -1 si fermé
// Equivalent de fl_choice mais avec police personnalisée
// Utilisation : my_choice(font, size, "Message", "Oui", "Non", "Annuler", nullptr)
int my_choice(Fl_Font font, Fl_Fontsize size, const char* msg, ...)
{
    std::vector<std::string> labels;
    va_list args;
    va_start(args, msg);
    const char* lbl;
    while ((lbl = va_arg(args, const char*)) != nullptr)
        labels.push_back(lbl);
    va_end(args);

    return my_dialog(font, size, msg, labels);
}
