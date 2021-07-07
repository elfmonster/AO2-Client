#include "aobutton.h"

#include "aoapplication.h"
#include "debug_functions.h"
#include "file_functions.h"

#include <QDebug>

AOButton::AOButton(QWidget *parent, AOApplication *p_ao_app) : QPushButton(parent)
{
  ao_app = p_ao_app;
}

QString AOButton::get_image()
{
  return m_image;
}

bool AOButton::has_image()
{
  return (!m_image.isEmpty());
}

void AOButton::set_image(QString p_image)
{
  m_image_stem = p_image;
  m_image = ao_app->find_theme_asset_path(p_image);
  // Get the path of the found image without the extension
  const QString l_image_name = p_image.left(p_image.lastIndexOf(QChar('.')));
  const QString l_hover_image = ao_app->find_theme_asset_path(l_image_name + "_hover.png");

  if (file_exists(m_image))
  {
    if (file_exists(l_hover_image))
      this->setStyleSheet("QPushButton {border-image:url(\"" + m_image +
                          "\");}"
                          "QPushButton:hover {border-image:url(\"" +
                          l_hover_image + "\");}");
    else
      this->setStyleSheet("border-image:url(\"" + m_image + "\")");
  }
  else
  {
    m_image = "";
    this->setStyleSheet("border-image:url(\"" + m_image + "\")");
  }
}

void AOButton::refresh_image()
{
  set_image(m_image_stem);
}
