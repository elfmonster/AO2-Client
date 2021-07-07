#ifndef AOEVIDENCEDISPLAY_H
#define AOEVIDENCEDISPLAY_H

class AOApplication;
class AOSfxPlayer;

#include <QLabel>

class AOEvidenceDisplay : public QLabel
{
  Q_OBJECT

public:
  AOEvidenceDisplay(QWidget *p_parent, AOApplication *p_ao_app);

  void show_evidence(QString p_evidence_image, bool is_left_side);
  void reset();

private:
  AOApplication *ao_app = nullptr;
  AOSfxPlayer *dr_sfx = nullptr;

  QMovie *m_movie = nullptr;

  QLabel *ui_icon = nullptr;

private slots:
  void frame_change(int p_frame);
};

#endif // AOEVIDENCEDISPLAY_H
