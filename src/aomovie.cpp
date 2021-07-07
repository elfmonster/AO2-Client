#include "aomovie.h"

#include "aoapplication.h"
#include "file_functions.h"
#include "misc_functions.h"

#include <QDebug>

AOMovie::AOMovie(QWidget *p_parent, AOApplication *p_ao_app) : QLabel(p_parent)
{
  ao_app = p_ao_app;

  m_movie = new QMovie();

  this->setMovie(m_movie);

  connect(m_movie, SIGNAL(frameChanged(int)), this, SLOT(frame_change(int)));
}

AOMovie::~AOMovie()
{
  delete m_movie;
}

void AOMovie::set_play_once(bool p_play_once)
{
  is_play_once = p_play_once;
}

void AOMovie::play(QString p_file, QString p_char)
{
  m_movie->stop();
  QString file_path = "";

  QString char_p_file;
  // FIXME: When looking in the character folder, append "_bubble" except for
  // custom We probably should drop this
  if (p_file != "custom")
    char_p_file = p_file + "_bubble";
  else
    char_p_file = p_file;

  // Asset lookup order
  // 1. If p_char is not empty, in the character folder, look for
  // `char_p_file` + extensions in `exts` in order
  // 2. If p_char is not empty, in the character folder, look for
  // `overlay/char_p_file` + extensions in `exts` in order
  // 3. In the theme folder (gamemode-timeofday/main/default), look for
  // `p_file` + extensions in `exts` in order
  // 4. In the theme folder (gamemode-timeofday/main/default), look for
  // "placeholder" + extensions in `exts` in order

  if (!p_char.isEmpty())
  {
    file_path = ao_app->find_asset_path(
        {
            ao_app->get_character_path(p_char, char_p_file),
            ao_app->get_character_path(p_char, "overlay/" + char_p_file),
        },
        animated_or_static_extensions());
  }

  if (file_path.isEmpty())
  {
    file_path = ao_app->find_theme_asset_path(p_file, animated_or_static_extensions());
    if (file_path.isEmpty())
      file_path = ao_app->find_theme_asset_path("placeholder", animated_or_static_extensions());
  }

  qDebug() << "playing" << file_path;
  m_movie->setFileName(file_path);

  this->show();
  m_movie->setScaledSize(this->size());
  m_movie->start();
}

///
/// \brief Searches and play the first interjection file it can find based on
/// the provided character name and interjection name.
///
void AOMovie::play_interjection(QString p_char_name, QString p_interjection_name)
{
  m_movie->stop();

  QString p_char_interjection_name;
  // FIXME: When looking in the character folder, append "_bubble" except for
  // custom We probably should drop this
  if (p_interjection_name.toLower() != "custom")
    p_char_interjection_name = p_interjection_name + "_bubble";
  else
    p_char_interjection_name = p_interjection_name;

  // Asset lookup order
  // 1. In the character folder, look for
  // `p_char_interjection_name`
  // 2. In the theme folder (gamemode-timeofday/main/default), look for
  // `p_char_name`

  QString interjection_filepath = ao_app->find_asset_path(
      {ao_app->get_character_path(p_char_name, p_char_interjection_name)}, animated_extensions());
  if (interjection_filepath.isEmpty())
    interjection_filepath = ao_app->find_theme_asset_path(p_interjection_name, animated_extensions());

  if (interjection_filepath.isEmpty())
  {
    Q_EMIT done();
    return;
  }

  qDebug() << "playing interjection" << (p_interjection_name.isEmpty() ? "(none)" : p_interjection_name)
           << "for character" << (p_char_name.isEmpty() ? "(none)" : p_char_name) << "at"
           << (interjection_filepath.isEmpty() ? "(not found)" : interjection_filepath);
  m_movie->setFileName(interjection_filepath);
  this->show();
  m_movie->setScaledSize(this->size());
  m_movie->start();
}

void AOMovie::restart()
{
  m_movie->stop();
  m_movie->start();
}

void AOMovie::stop()
{
  m_movie->stop();
  m_movie->setFileName(""); // Properly free up resources
  // If the file is not cleared after the movie is stopped, the movie will
  // continue using as much memory as it requested, which leads to invisible
  // memory hogging.
  this->hide();
}

void AOMovie::frame_change(int n_frame)
{
  const int l_real_frame = n_frame + 1;
  if (is_play_once && l_real_frame == m_movie->frameCount())
  {
    delay(m_movie->nextFrameDelay());
    m_movie->stop();
    Q_EMIT done();
  }
}

void AOMovie::combo_resize(int w, int h)
{
  QSize f_size(w, h);
  this->resize(f_size);
  m_movie->setScaledSize(f_size);
}

/*
 * @brief Returns the state of the current movie. Refer to QMovie::state()
 * for more details.
 * @returns Current movie status
 */
QMovie::MovieState AOMovie::state()
{
  return m_movie->state();
}
