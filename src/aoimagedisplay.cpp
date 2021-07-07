#include "aoimagedisplay.h"

#include "aoapplication.h"
#include "aopixmap.h"
#include "file_functions.h"

/*!
 * @class AOImageDisplay
 * @brief Represents a static theme-dependent image.
 */
AOImageDisplay::AOImageDisplay(QWidget *parent, AOApplication *p_ao_app) : QLabel(parent)
{
  ao_app = p_ao_app;
}

QString AOImageDisplay::get_image()
{
  return m_image;
}

void AOImageDisplay::set_image(QString p_image)
{
  const QString l_path = ao_app->find_theme_asset_path(p_image);
  AOPixmap f_pixmap(l_path);
  this->setPixmap(f_pixmap.scale(size()));

  // Store final path if the path exists
  if (file_exists(l_path))
    m_image = l_path;
  else
    m_image = "";
}

void AOImageDisplay::set_image_from_path(QString p_path)
{
  QString default_path = ao_app->find_theme_asset_path("chatmed.png");

  QString final_path;

  if (file_exists(p_path))
    final_path = p_path;
  else
    final_path = default_path;

  AOPixmap f_pixmap(final_path);
  this->setPixmap(f_pixmap.scale(size()));

  // Store final path if the path exists
  if (file_exists(final_path))
    m_image = final_path;
  else
    m_image = "";
}
