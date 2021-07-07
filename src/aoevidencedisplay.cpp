#include "aoevidencedisplay.h"

#include "aoapplication.h"
#include "aopixmap.h"
#include "aosfxplayer.h"
#include "commondefs.h"
#include "courtroom.h"
#include "datatypes.h"
#include "file_functions.h"
#include "misc_functions.h"

#include <QMovie>

AOEvidenceDisplay::AOEvidenceDisplay(QWidget *p_parent, AOApplication *p_ao_app) : QLabel(p_parent)
{
  ao_app = p_ao_app;

  m_movie = new QMovie(this);
  ui_icon = new QLabel(this);
  dr_sfx = new AOSfxPlayer(ao_app, this);

  connect(m_movie, SIGNAL(frameChanged(int)), this, SLOT(frame_change(int)));
}

void AOEvidenceDisplay::show_evidence(QString p_evidence_image, bool is_left_side)
{
  this->reset();

  QString l_evidence = ao_app->get_evidence_path(p_evidence_image);

  AOPixmap l_pixmap(l_evidence);

  QString l_icon_animation;
  QString l_icon_identifier;

  if (is_left_side)
  {
    l_icon_identifier = "left_evidence_icon";
    l_icon_animation = "evidence_appear_left.gif";
  }
  else
  {
    l_icon_identifier = "right_evidence_icon";
    l_icon_animation = "evidence_appear_right.gif";
  }

  pos_size_type l_icon_dimensions = ao_app->get_element_dimensions(l_icon_identifier, COURTROOM_DESIGN_INI);

  ui_icon->move(l_icon_dimensions.x, l_icon_dimensions.y);
  ui_icon->resize(l_icon_dimensions.width, l_icon_dimensions.height);
  ui_icon->setPixmap(l_pixmap.scale(ui_icon->size()));

  QString f_path = ao_app->find_theme_asset_path(l_icon_animation);
  m_movie->setFileName(f_path);
  if (m_movie->frameCount() < 1)
    return;

  this->setMovie(m_movie);

  m_movie->start();
  dr_sfx->play_effect(ao_app->get_sfx("evidence_present"));
}

void AOEvidenceDisplay::frame_change(int p_frame)
{
  if (p_frame == (m_movie->frameCount() - 1))
  {
    // we need this or else the last frame wont show
    delay(m_movie->nextFrameDelay());

    m_movie->stop();
    this->clear();

    ui_icon->show();
  }
}

void AOEvidenceDisplay::reset()
{
  m_movie->stop();
  ui_icon->hide();
  this->clear();
}
