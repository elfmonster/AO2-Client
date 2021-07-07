#ifndef AOBUTTON_H
#define AOBUTTON_H

class AOApplication;

#include <QPushButton>

class AOButton : public QPushButton
{
  Q_OBJECT

public:
  AOButton(QWidget *parent, AOApplication *p_ao_app);

  QString get_image();
  bool has_image();
  void set_image(QString p_image);
  void refresh_image();

private:
  AOApplication *ao_app = nullptr;
  QString m_image;
  QString m_image_stem;
};

#endif // AOBUTTON_H
