#include "aoevidencedescription.h"

AOEvidenceDescription::AOEvidenceDescription(QWidget *parent) : QPlainTextEdit(parent)
{
  this->setReadOnly(true);
}

void AOEvidenceDescription::mouseDoubleClickEvent(QMouseEvent *e)
{
  QPlainTextEdit::mouseDoubleClickEvent(e);

  this->setReadOnly(false);
}

void AOEvidenceDescription::on_enter_pressed()
{
  this->setReadOnly(true);
}
