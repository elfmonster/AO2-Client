#include "aosystemplayer.h"

#include "aoapplication.h"
#include "draudioengine.h"
#include "file_functions.h"

AOSystemPlayer::AOSystemPlayer(AOApplication *p_ao_app, QObject *p_parent) : AOObject(p_ao_app, p_parent)
{}

void AOSystemPlayer::play(QString p_name)
{
  const QString file = ao_app->find_asset_path({ao_app->get_sounds_path(p_name)}, audio_extensions());
  DRAudioEngine::get_family(DRAudio::Family::FSystem)->play_stream(file);
}
