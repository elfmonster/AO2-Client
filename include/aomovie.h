#ifndef AOMOVIE_H
#define AOMOVIE_H

class AOApplication;

#include <QLabel>
#include <QMovie>

class AOMovie : public QLabel
{
  Q_OBJECT

public:
  AOMovie(QWidget *p_parent, AOApplication *p_ao_app);
  ~AOMovie();

  void set_play_once(bool p_play_once);
  void play(QString p_file, QString p_char = "");
  void play_interjection(QString p_char_name, QString p_interjection_name);
  void restart();
  void combo_resize(int w, int h);
  QMovie::MovieState state();
  void stop();

signals:
  void done();

private:
  AOApplication *ao_app = nullptr;

  QMovie *m_movie = nullptr;

  bool is_play_once = true;

private slots:
  void frame_change(int n_frame);
};

#endif // AOMOVIE_H
