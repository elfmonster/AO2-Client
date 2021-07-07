#include "aomusicplayer.h"

#include "aoapplication.h"

AOMusicPlayer::AOMusicPlayer(AOApplication *p_ao_app, QObject *p_parent) : AOObject(p_ao_app, p_parent)
{
  m_family = DRAudioEngine::get_family(DRAudio::Family::FMusic);
  m_family->set_capacity(1); // a single song is needed
}

void AOMusicPlayer::play(QString p_song)
{
  stop();
  m_song = p_song;
  m_family->play_stream(ao_app->get_music_path(p_song));
}

void AOMusicPlayer::stop()
{
  for (auto &song : m_family->get_stream_list())
    song->stop();
}
