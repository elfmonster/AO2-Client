#ifndef AOCHARMOVIE_H
#define AOCHARMOVIE_H

class AOApplication;

#include <QLabel>

class QMovie;
class QTimer;

class AOCharMovie : public QLabel
{
  Q_OBJECT

public:
  AOCharMovie(QWidget *p_parent, AOApplication *p_ao_app);

  bool play(QString character, QString emote, QString prefix, bool play_once);
  bool play(QString character, QString emote, bool play_once);
  bool play_pre(QString character, QString emote);
  bool play_talk(QString character, QString emote);
  bool play_idle(QString character, QString emote);
  void set_mirrored(bool enabled);
  void combo_resize(QSize size);
  void stop();

signals:
  void done();

private:
  AOApplication *ao_app = nullptr;

  QMovie *m_movie = nullptr;
  QTimer *m_frame_timer = nullptr;
  QImage m_current_frame;

  bool is_mirrored = false;
  bool is_play_once = false;

  void paint_frame();

private slots:
  void on_frame_changed(int n_frame);
  void on_timer_timeout();
};

#endif // AOCHARMOVIE_H
