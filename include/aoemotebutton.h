#ifndef AOEMOTEBUTTON_H
#define AOEMOTEBUTTON_H

// src
#include "datatypes.h"

class AOApplication;

// qt
#include <QPushButton>

class QLabel;

class AOEmoteButton : public QPushButton
{
  Q_OBJECT

public:
  AOEmoteButton(QWidget *p_parent, AOApplication *p_ao_app, int p_x, int p_y);

  int get_emote_number();
  void set_emote_number(int emote_number);
  void set_image(DREmote emote, bool enabled);

signals:
  void emote_clicked(int emote_number);
  void tooltip_requested(int emote_number, QPoint global_pos);
  void mouse_left(int emote_number);

  // QPushButton interface
protected:
  bool event(QEvent *event) override;

private:
  AOApplication *ao_app = nullptr;

  int m_index = 0;

  QLabel *ui_selected = nullptr;

private slots:
  void on_clicked();
};

#endif // AOEMOTEBUTTON_H
