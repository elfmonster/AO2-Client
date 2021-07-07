#ifndef AOLABEL_HPP
#define AOLABEL_HPP

class AOApplication;

#include <QLabel>

class AOLabel : public QLabel
{
public:
  AOLabel(QWidget *parent, AOApplication *p_ao_app);

  void set_image(QString p_image);

private:
  AOApplication *ao_app = nullptr;
};

#endif // AOLABEL_HPP
