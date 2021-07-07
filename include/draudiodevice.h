#pragma once

#include <bass.h>

#include <QMetaType>
#include <QString>

#include <optional>

class DRAudioEngine;
class DRAudioEnginePrivate;

class DRAudioDevice
{
public:
  enum State : DWORD
  {
    SEnabled = BASS_DEVICE_ENABLED,
    SDefault = BASS_DEVICE_DEFAULT,
    SInit = BASS_DEVICE_INIT,
  };
  Q_DECLARE_FLAGS(States, State)

  DRAudioDevice();
  DRAudioDevice(DWORD p_device, BASS_DEVICEINFO p_device_info);

  std::optional<DWORD> get_id() const;
  QString get_name() const;
  QString get_driver() const;
  States get_states() const;
  // condition check
  bool is_state(State p_state) const;
  bool is_enabled() const;
  bool is_default() const;
  bool is_init() const;

private:
  friend class DRAudioEngine;
  friend class DRAudioEnginePrivate;

  std::optional<DWORD> m_id;
  QString m_name;
  QString m_driver;
  States m_states;

  bool merge(DRAudioDevice &p_device);
};
Q_DECLARE_METATYPE(DRAudioDevice)
