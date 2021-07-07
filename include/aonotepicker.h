#ifndef AONOTEPICKER_HPP
#define AONOTEPICKER_HPP

class AOApplication;
class AOButton;

#include <QLabel>

class QHBoxLayout;
class QLineEdit;

class AONotePicker : public QLabel
{
  Q_OBJECT

public:
  AONotePicker(QWidget *p_parent, AOApplication *p_ao_app);

  QString m_file;

  QLineEdit *ui_line = nullptr;
  AOButton *ui_button = nullptr;
  AOButton *ui_delete_button = nullptr;
  AOButton *ui_hover = nullptr;
  QHBoxLayout *ui_layout = nullptr;

private:
  AOApplication *ao_app = nullptr;
};

#endif // AONOTEPICKER_HPP
