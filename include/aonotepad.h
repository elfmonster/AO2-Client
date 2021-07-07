#ifndef AONOTEPAD_H
#define AONOTEPAD_H

#include "drtextedit.h"

class AOApplication;

class AONotepad : public DRTextEdit
{
  Q_OBJECT

public:
  AONotepad(QWidget *p_parent, AOApplication *p_ao_app);

private:
  AOApplication *ao_app = nullptr;
};

#endif // AONOTEPAD_H
