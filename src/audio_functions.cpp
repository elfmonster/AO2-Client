#include "courtroom.h"

#include "draudioengine.h"

bool Courtroom::is_audio_suppressed() const
{
  return is_audio_muted;
}

void Courtroom::suppress_audio(bool p_enabled)
{
  if (is_audio_muted == p_enabled)
    return;
  is_audio_muted = p_enabled;

  // suppress audio
  for (auto &family : DRAudioEngine::get_family_list())
    family->set_suppressed(is_audio_muted);
}

void Courtroom::stop_all_audio()
{
  for (auto &family : DRAudioEngine::get_family_list())
    for (auto &stream : family->get_stream_list())
      stream->stop();
}
