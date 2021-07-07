#ifndef AOEVIDENCEBUTTON_H
#define AOEVIDENCEBUTTON_H

class AOApplication;
class AOImageDisplay;

#include <QPushButton>

class AOEvidenceButton : public QPushButton
{
  Q_OBJECT

public:
  AOEvidenceButton(QWidget *p_parent, AOApplication *p_ao_app, int p_x, int p_y);

  void reset();
  void set_image(QString p_image);
  void set_theme_image(QString p_image);
  void set_id(int p_id);

  void set_selected(bool p_selected);

signals:
  void evidence_clicked(int p_id);
  void evidence_double_clicked(int p_id);
  void on_hover(int p_id, bool p_state);

protected:
  void enterEvent(QEvent *e);
  void leaveEvent(QEvent *e);
  void mouseDoubleClickEvent(QMouseEvent *e);

private:
  AOApplication *ao_app = nullptr;

  int m_index = 0;

  AOImageDisplay *ui_selected = nullptr;
  AOImageDisplay *ui_selector = nullptr;

private slots:
  void on_clicked();
};

#endif // AOEVIDENCEBUTTON_H
