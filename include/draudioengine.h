#pragma once

#include "draudiostreamfamily.h"

class DRAudioEngine : public QObject
{
  Q_OBJECT

public:
  static std::optional<DRAudioDevice> get_device();
  static std::optional<DRAudioDevice> get_favorite_device();
  static QList<DRAudioDevice> get_device_list();
  static DRAudioStreamFamily::ptr get_family(DRAudio::Family p_family);
  static QList<DRAudioStreamFamily::ptr> get_family_list();
  static int32_t get_volume();
  static DRAudio::Options get_options();
  // option get
  static bool is_option(DRAudio::Option p_option);
  static bool is_suppressed();
  static bool is_suppress_background_audio();

  DRAudioEngine(QObject *p_parent = nullptr);
  ~DRAudioEngine();

public slots:
  void set_device(DRAudioDevice p_device);
  void set_favorite_device(DRAudioDevice p_device);
  void set_favorite_device_by_driver(QString p_device_driver);
  void set_volume(int32_t p_volume);
  void set_options(DRAudio::Options p_options);
  // option set
  void set_option(DRAudio::Option p_option, bool p_enabled);
  void set_suppressed(bool p_enabled);
  void set_suppress_background_audio(bool p_enabled);

signals:
  void device_changed(DRAudioDevice);
  void favorite_device_changed(DRAudioDevice);
  void device_list_changed(QList<DRAudioDevice>);
  void volume_changed(int32_t);
  void options_changed(DRAudio::Options);

private:
  friend class DRAudioStream;

  static void check_stream_error();
};
