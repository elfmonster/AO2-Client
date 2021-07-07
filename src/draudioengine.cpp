#include "draudioengine.h"

#include "draudioengine_p.h"

#include <QTimer>

static const int EVENT_TIMER_INTERVAL_DEFAULT = 100;

static class DRAudioEngineData
{
public:
  using ptr = DRAudioEnginePrivate::ptr;

  ptr &operator->()
  {
    if (d == nullptr)
    {
      d = ptr(new DRAudioEnginePrivate);

      // create engine shortcut
      d->engine = new DRAudioEngine;

      // create families
      d->family_map.insert(DRAudio::Family::FSystem,
                           DRAudioStreamFamily::ptr(new DRAudioStreamFamily(DRAudio::Family::FSystem)));
      d->family_map.insert(DRAudio::Family::FEffect,
                           DRAudioStreamFamily::ptr(new DRAudioStreamFamily(DRAudio::Family::FEffect)));
      d->family_map.insert(DRAudio::Family::FMusic,
                           DRAudioStreamFamily::ptr(new DRAudioStreamFamily(DRAudio::Family::FMusic)));
      d->family_map.insert(DRAudio::Family::FBlip,
                           DRAudioStreamFamily::ptr(new DRAudioStreamFamily(DRAudio::Family::FBlip)));

      // set family-specific options
      d->family_map.value(DRAudio::Family::FSystem)->set_ignore_suppression(true);

      // create and init event loop
      d->event_timer->setInterval(EVENT_TIMER_INTERVAL_DEFAULT);
      d->event_timer->setSingleShot(false);

      QObject::connect(d->event_timer, SIGNAL(timeout()), d, SLOT(on_event_tick()));

      d->event_timer->start();

      d->on_event_tick();
    }

    return d;
  }

private:
  ptr d;
} d;

DRAudioEngine::DRAudioEngine(QObject *p_parent) : QObject(p_parent)
{
  d->children.append(this);
}

DRAudioEngine::~DRAudioEngine()
{
  d->children.removeAll(this);
}

std::optional<DRAudioDevice> DRAudioEngine::get_device()
{
  return d->device;
}

std::optional<DRAudioDevice> DRAudioEngine::get_favorite_device()
{
  return d->favorite_device;
}

QList<DRAudioDevice> DRAudioEngine::get_device_list()
{
  return d->device_map.values();
}

DRAudioStreamFamily::ptr DRAudioEngine::get_family(DRAudio::Family p_family)
{
  return d->family_map.value(p_family);
}

QList<DRAudioStreamFamily::ptr> DRAudioEngine::get_family_list()
{
  return d->family_map.values();
}

int32_t DRAudioEngine::get_volume()
{
  return d->volume;
}

DRAudio::Options DRAudioEngine::get_options()
{
  return d->options;
}

bool DRAudioEngine::is_option(DRAudio::Option p_option)
{
  return d->options.testFlag(p_option);
}

bool DRAudioEngine::is_suppressed()
{
  return d->options.testFlag(DRAudio::OSuppressed);
}

bool DRAudioEngine::is_suppress_background_audio()
{
  return is_option(DRAudio::OEngineSuppressBackgroundAudio);
}

void DRAudioEngine::set_device(DRAudioDevice p_device)
{
  if (!d->device_map.contains(p_device.get_driver()))
    return;
  if (d->device.has_value() && d->device->get_driver() == p_device.get_driver())
    return;
  d->previous_device = d->device;
  d->device = d->device_map.value(p_device.get_driver());
  d->update_device();
  Q_EMIT d->invoke_signal("device_changed", Q_ARG(DRAudioDevice, d->device.value()));
}

void DRAudioEngine::set_favorite_device(DRAudioDevice p_device)
{
  if (d->favorite_device.has_value() && d->favorite_device.value().get_driver() == p_device.get_driver())
    return;
  d->favorite_device = p_device;
  Q_EMIT d->invoke_signal("favorite_device_changed", Q_ARG(DRAudioDevice, d->favorite_device.value()));
}

void DRAudioEngine::set_favorite_device_by_driver(QString p_device_driver)
{
  DRAudioDevice favorite_device;
  favorite_device.m_driver = p_device_driver;
  d->check_device = true;
  set_favorite_device(favorite_device);
}

void DRAudioEngine::set_volume(int32_t p_volume)
{
  p_volume = std::clamp(p_volume, 0, 100);
  if (d->volume == p_volume)
    return;
  d->volume = p_volume;
  d->update_volume();
  Q_EMIT d->invoke_signal("volume_changed", Q_ARG(int32_t, d->volume));
}

void DRAudioEngine::set_options(DRAudio::Options p_options)
{
  if (d->options == p_options)
    return;
  d->options = p_options;
  d->update_options();
  Q_EMIT d->invoke_signal("options_changed", Q_ARG(DRAudio::Options, d->options));
}

void DRAudioEngine::set_option(DRAudio::Option p_option, bool p_enabled)
{
  DRAudio::Options new_options = d->options;
  new_options.setFlag(p_option, p_enabled);
  set_options(new_options);
}

void DRAudioEngine::set_suppressed(bool p_enabled)
{
  set_option(DRAudio::OSuppressed, p_enabled);
}

void DRAudioEngine::set_suppress_background_audio(bool p_enabled)
{
  set_option(DRAudio::OEngineSuppressBackgroundAudio, p_enabled);
}

void DRAudioEngine::check_stream_error()
{
  d->check_stream_error();
}
