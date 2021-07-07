#ifndef AOCONFIGPANEL_H
#define AOCONFIGPANEL_H

#include "draudioengine.h"

class AOApplication;
class AOConfig;

#include <QWidget>

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSlider;
class QSpinBox;

class AOConfigPanel : public QWidget
{
  Q_OBJECT

public:
  AOConfigPanel(AOApplication *p_ao_app, QWidget *p_parent = nullptr);

public slots:
  void on_config_reload_theme_requested();

signals:
  void reload_theme();

protected:
  void showEvent(QShowEvent *event) override;

private:
  void refresh_theme_list();
  void refresh_gamemode_list();
  void refresh_timeofday_list();
  void update_audio_device_list();

private slots:
  void on_reload_theme_clicked();
  void on_theme_changed(QString);
  void on_gamemode_changed(QString);
  void on_timeofday_changed(QString);
  void on_gamemode_index_changed(QString p_text);
  void on_timeofday_index_changed(QString p_text);
  void on_showname_placeholder_changed(QString p_text);
  void on_log_is_topdown_changed(bool p_enabled);
  void on_device_current_index_changed(int p_index);
  void on_audio_device_changed(DRAudioDevice p_device);
  void on_favorite_audio_device_changed(DRAudioDevice p_device);
  void on_audio_device_list_changed(QList<DRAudioDevice> p_device_list);
  void on_master_value_changed(int p_num);
  void on_system_value_changed(int p_num);
  void on_effect_value_changed(int p_num);
  void on_music_value_changed(int p_num);
  void on_blip_value_changed(int p_num);

private:
  // FIXME This dependency shouldn't have come to exist.
  AOApplication *ao_app = nullptr;
  // driver
  AOConfig *m_config = nullptr;
  DRAudioEngine *m_engine = nullptr;
  // behaviour
  QPushButton *ui_save = nullptr;
  QPushButton *ui_close = nullptr;
  QCheckBox *ui_autosave = nullptr;
  // general
  QLineEdit *ui_username = nullptr;
  QLineEdit *ui_callwords = nullptr;
  QCheckBox *ui_server_alerts = nullptr;
  QGroupBox *ui_discord_presence = nullptr;
  QCheckBox *ui_discord_hide_server = nullptr;
  QCheckBox *ui_discord_hide_character = nullptr;
  // game
  QComboBox *ui_theme = nullptr;
  QPushButton *ui_reload_theme = nullptr;
  QComboBox *ui_gamemode = nullptr;
  QCheckBox *ui_manual_gamemode = nullptr;
  QComboBox *ui_timeofday = nullptr;
  QCheckBox *ui_manual_timeofday = nullptr;
  QLineEdit *ui_showname = nullptr;
  QCheckBox *ui_always_pre = nullptr;
  QSpinBox *ui_chat_tick_interval = nullptr;
  // IC Chatlog
  QSpinBox *ui_log_max_lines = nullptr;
  QCheckBox *ui_log_display_timestamp = nullptr;
  QCheckBox *ui_log_display_self_highlight = nullptr;
  QCheckBox *ui_log_format_use_newline = nullptr;
  QCheckBox *ui_log_display_empty_messages = nullptr;
  QCheckBox *ui_log_display_music_switch = nullptr;
  QRadioButton *ui_log_orientation_top_down = nullptr;
  QRadioButton *ui_log_orientation_bottom_up = nullptr;
  QCheckBox *ui_log_is_recording = nullptr;
  // audio
  QComboBox *ui_device = nullptr;
  QCheckBox *ui_favorite_device = nullptr;
  QSlider *ui_master = nullptr;
  QLabel *ui_master_value = nullptr;
  QGroupBox *ui_suppress_background_audio = nullptr;
  QSlider *ui_system = nullptr;
  QLabel *ui_system_value = nullptr;
  QSlider *ui_effect = nullptr;
  QCheckBox *ui_effect_ignore_suppression = nullptr;
  QLabel *ui_effect_value = nullptr;
  QSlider *ui_music = nullptr;
  QCheckBox *ui_music_ignore_suppression = nullptr;
  QLabel *ui_music_value = nullptr;
  QSlider *ui_blip = nullptr;
  QCheckBox *ui_blip_ignore_suppression = nullptr;
  QLabel *ui_blip_value = nullptr;
  QSpinBox *ui_blip_rate = nullptr;
  QCheckBox *ui_blank_blips = nullptr;

private slots:
  void username_editing_finished();
  void showname_editing_finished();
  void callwords_editing_finished();
};

#endif // AOCONFIGPANEL_H
