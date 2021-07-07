#include "draudiodevice.h"

DRAudioDevice::DRAudioDevice() : m_name("<unknown>")
{}

DRAudioDevice::DRAudioDevice(DWORD p_device, BASS_DEVICEINFO p_device_info)
    : m_id(p_device), m_name(p_device_info.name), m_driver(p_device_info.driver), m_states(State(p_device_info.flags))
{}

std::optional<DWORD> DRAudioDevice::get_id() const
{
  return m_id;
}

QString DRAudioDevice::get_name() const
{
  return m_name;
}

QString DRAudioDevice::get_driver() const
{
  return m_driver;
}

DRAudioDevice::States DRAudioDevice::get_states() const
{
  return m_states;
}

bool DRAudioDevice::is_state(DRAudioDevice::State p_state) const
{
  return m_states.testFlag(p_state);
}

bool DRAudioDevice::is_enabled() const
{
  return is_state(SEnabled);
}

bool DRAudioDevice::is_default() const
{
  return is_state(SDefault);
}

bool DRAudioDevice::is_init() const
{
  return is_state(SInit);
}

bool DRAudioDevice::merge(DRAudioDevice &p_device)
{
  bool is_changed = false;
  auto check_and_set = [&](auto &a, auto &b) {
    if (a == b)
      return;
    is_changed = true;
    a = b;
  };

  check_and_set(m_name, p_device.m_name);
  check_and_set(m_driver, p_device.m_driver);
  check_and_set(m_states, p_device.m_states);
  return is_changed;
}
