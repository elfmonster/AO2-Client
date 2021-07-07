#pragma once

#include "draudio.h"
#include "draudiodevice.h"
#include "draudioerror.h"

#include <bass.h>

#include <QObject>
#include <QStack>

#include <optional>

class DRAudioEngine;
class DRAudioEnginePrivate;
class DRAudioStreamFamily;

class DRAudioStreamSync
{
public:
  HSYNC sync;
  DWORD type;
};

class DRAudioStream : public QObject
{
  Q_OBJECT

  DRAudioStream(DRAudio::Family p_family);

public:
  using ptr = QSharedPointer<DRAudioStream>;

  ~DRAudioStream();

  DRAudio::Family get_family() const;
  std::optional<QString> get_file() const;

  // state
  bool is_playing() const;

public slots:
  std::optional<DRAudioError> set_file(QString m_file);
  void set_volume(float p_volume);

  void play();
  void stop();

signals:
  void file_changed(QString p_file);
  void finished();

public:
  static void CALLBACK on_sync_callback(HSYNC hsync, DWORD ch, DWORD data, void *userdata);

private:
  friend class DRAudioEngine;
  friend class DRAudioEnginePrivate;
  friend class DRAudioStreamFamily;

  // static method
  DRAudio::Family m_family;
  std::optional<QString> m_file;
  float m_volume;

  // bass
  std::optional<HSTREAM> m_hstream;
  QStack<DRAudioStreamSync> m_hsync_stack;
  std::optional<DWORD> m_position;

  void cache_position();
  void update_device();

private slots:
  void on_device_error();

signals:
  void device_error(QPrivateSignal);
};
