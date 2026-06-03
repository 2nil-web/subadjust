
#include "my_ask.h"

static Fl_Font my_font_ = FL_HELVETICA;
static Fl_Font my_fontsize_ = 14;
static int my_x = -1, my_y = -1;

void my_message_position(int x, int y)
{
  my_x = x;
  my_y = y;
}

void my_font(Fl_Font f, Fl_Fontsize s)
{
  my_font_ = f;
  my_fontsize_ = s;
}

void my_fontsize(Fl_Fontsize s)
{
  my_fontsize_ = s;
}

class MyBox : public Fl_Box
{
  Fl_Font _font;
  Fl_Fontsize _size;

public:
  MyBox(int x, int y, int w, int h, const char *l, Fl_Font font = my_font_, Fl_Fontsize size = my_fontsize_) : Fl_Box(x, y, w, h, l), _font(font), _size(size)
  {
  }

  void draw() override
  {
    // Dessiner le fond
    draw_box();
    // Forcer la police et dessiner le texte DIRECTEMENT sans passer par le mécanisme de label FLTK
    fl_font(_font, _size);
    fl_color(labelcolor());
    fl_draw(label(), x(), y(), w(), h(), align(), nullptr, 0);
  }
};

// Fonction interne commune qui retourne l'index du bouton cliqué, ou -1 si fermé/Escape
// Si labels est vide, un seul bouton "OK" est créé (mode my_message)
static int my_dialog(const char *msg, const std::vector<std::string> &labels = {})
{
  const int MAX_W = 800;
  const int MARGIN = 20;
  const int BTN_H = 20;
  const int BTN_PAD = 10;
  const int BTN_GAP = 20;

  // --- Mesurer le texte du message ---
  fl_font(my_font_, my_fontsize_);
  int text_w = MAX_W - 2 * MARGIN;
  int text_h = 0;
  fl_measure(msg, text_w, text_h, 1);

  // Compenser la descente superflue incluse par fl_measure
  int line_h = fl_height();
  int descent = fl_descent();
  text_h -= descent;

  // Padding vertical proportionnel à la police
  const int PAD_TOP = line_h / 2;
  const int PAD_MID = line_h / 2;
  const int PAD_BOT = line_h / 2;

  // --- Mesurer chaque bouton ---
  const std::vector<std::string> effective_labels = labels.empty() ? std::vector<std::string>{"OK"} : labels;

  std::vector<int> btn_widths;
  for (const auto &lbl : effective_labels)
  {
    int bw = 0, bh = 0;
    fl_measure(lbl.c_str(), bw, bh, 0);
    bw += 2 * BTN_PAD;
    bw = std::max(bw, 60);
    btn_widths.push_back(bw);
  }

  // --- Largeur totale des boutons ---
  int total_btn_w = BTN_GAP * ((int)effective_labels.size() - 1);
  for (int w : btn_widths)
    total_btn_w += w;

  // --- Dimensions fenêtre ---
  int win_w = std::max(text_w + 2 * MARGIN, total_btn_w + 2 * MARGIN);
  int win_h = 2 * PAD_TOP + text_h + PAD_MID + BTN_H + PAD_BOT;
  win_w = std::max(win_w, 300);
  win_h = std::max(win_h, 60);

  // --- Positions verticales ---
  int box_y = PAD_TOP;
  int btn_y = 2 * PAD_TOP + text_h + PAD_MID;

  // --- Données partagées des callbacks ---
  struct CallbackData
  {
    int result;
    Fl_Window *win;
  };
  CallbackData cbd = {-1, nullptr};

  // --- Fenêtre ---
  extern options myopt; // Pour récupérer Progname
  Fl_Window *win = new Fl_Window(win_w, win_h, myopt.Progname.c_str());
  // extern Fl_SVG_Image *app_icon; // Defined and populated in subadjust.cpp
  win->icon(app_icon);

  if (my_x > -1 && my_y > -1)
    win->resize(my_x, my_y, win_w, win_h);
  cbd.win = win;

  // --- Boîte de message ---
  MyBox *box = new MyBox(MARGIN, box_y, text_w, text_h, msg);
  box->align(FL_ALIGN_WRAP | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);

  // --- Boutons ---
  int start_x = (win_w - total_btn_w) / 2;
  int cursor_x = start_x;

  struct BtnData
  {
    CallbackData *cbd;
    int index;
  };

  for (int i = 0; i < (int)effective_labels.size(); i++)
  {
    Fl_Hover<Fl_Button> *btn = new Fl_Hover<Fl_Button>(cursor_x, btn_y, btn_widths[i], BTN_H, nullptr);
    btn->copy_label(effective_labels[i].c_str());

    BtnData *bd = new BtnData{&cbd, i};
    btn->callback(
        [](Fl_Widget *, void *data) {
          BtnData *bd = (BtnData *)data;
          bd->cbd->result = bd->index;
          bd->cbd->win->hide();
          delete bd;
        },
        bd);

    cursor_x += btn_widths[i] + BTN_GAP;
  }

  // --- Callback fermeture fenêtre ---
  win->callback(
      [](Fl_Widget *w, void *data) {
        CallbackData *cbd = (CallbackData *)data;
        cbd->result = -1;
        w->hide();
      },
      &cbd);

  win->set_non_modal();
  win->end();
  win->show();
  win->take_focus();

  while (win->shown())
    Fl::wait();

  delete win;
  return cbd.result;
}

void my_message(const char *msg)
{
  my_dialog(msg);
}

// Retourne l'index du bouton cliqué (0, 1, 2...), ou -1 si fermé
// Equivalent de fl_choice mais avec police personnalisée
// Utilisation : my_choice("Message", "Oui", "Non", "Annuler", nullptr)
int my_choice(const char *msg, ...)
{
  std::vector<std::string> labels;
  va_list args;
  va_start(args, msg);
  const char *lbl;
  while ((lbl = va_arg(args, const char *)) != nullptr)
    labels.push_back(lbl);
  va_end(args);

  return my_dialog(msg, labels);
}
