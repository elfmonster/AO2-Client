#ifndef DATATYPES_H
#define DATATYPES_H

#include <QDateTime>

class DREmote
{
public:
  QString character;
  QString key;
  QString comment;
  QString anim;
  QString dialog;
  int modifier = 0;
  int desk_modifier = -1;
  QString sound_file;
  int sound_delay = 0;
};

class DRChatRecord
{
public:
  using list = QVector<DRChatRecord>;

  DRChatRecord(QString p_name, QString p_message) : name(p_name), message(p_message)
  {}

  QDateTime get_timestamp() const
  {
    return timestamp;
  }
  QString get_name() const
  {
    return name;
  }
  QString get_message() const
  {
    return message;
  }
  bool is_self() const
  {
    return self;
  }
  bool is_system() const
  {
    return system;
  }
  bool is_music() const
  {
    return music;
  }

  // set
  void set_self(const bool p_enabled)
  {
    if (self == p_enabled)
      return;
    self = p_enabled;
  }
  void set_system(bool p_enabled)
  {
    if (system == p_enabled)
      return;
    system = p_enabled;
  }
  void set_music(bool p_enabled)
  {
    if (music == p_enabled)
      return;
    music = p_enabled;
  }

private:
  QDateTime timestamp = QDateTime::currentDateTime();
  QString name;
  QString message;
  bool self = false;
  bool system = false;
  bool music = false;
};

struct DRSfx
{
public:
  DRSfx() = default;
  DRSfx(QString p_name, QString p_file, bool p_is_found = false)
      : name(p_name.trimmed()), file(p_file.trimmed()), is_found(p_is_found)
  {}

  QString name;
  QString file;
  bool is_found;
};

struct server_type
{
  QString name;
  QString desc;
  QString ip;
  int port;

  QString to_info() const
  {
    QString r_info;

    if (!name.isEmpty())
    {
      r_info = name;
    }
    else if (!ip.isEmpty())
    {
      r_info = ip + ":" + QString::number(port);
    }

    return r_info;
  }
};

struct char_type
{
  QString name;
  bool taken = false;
};

struct evi_type
{
  QString name;
  QString description;
  QString image;
};

struct pos_size_type
{
  int x = 0;
  int y = 0;
  int width = 0;
  int height = 0;
};

enum ChatMessage : int32_t
{
  CMDeskModifier = 0,
  CMPreAnim,
  CMChrName,
  CMEmote,
  CMMessage,
  CMPosition,
  CMSoundName,
  CMEmoteModifier,
  CMChrId,
  CMSoundDelay,
  CMShoutModifier,
  CMEvidenceId,
  CMFlipState,
  CMEffectState,
  CMTextColor,
  CMShowName,
};

namespace DR
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
using SplitBehavior = QString::SplitBehavior;
#else
using SplitBehavior = Qt::SplitBehaviorFlags;
#endif
const SplitBehavior KeepEmptyParts = SplitBehavior::KeepEmptyParts;
const SplitBehavior SkipEmptyParts = SplitBehavior::SkipEmptyParts;

enum VAlign : int32_t
{
  Top,
  Middle,
  Bottom,
};

enum HAlign : int32_t
{
  Left,
  Center,
  Right,
};

enum Color : int32_t
{
  CDefault,
  CGreen,
  CRed,
  COrange,
  CBlue,
  CYellow,
  CPurple,
  CPink,
  CRainbow,

  // aliases
  CWhite = CDefault,
};

struct ColorInfo
{
public:
  ColorInfo() = default;
  ColorInfo(QString p_showname, QString p_code) : name(p_showname.toLower()), showname(p_showname), code(p_code)
  {}

  QString name;
  QString showname;
  QString code;
};

QMap<Color, ColorInfo> get_default_color_map();
} // namespace DR

#endif // DATATYPES_H
