#include "aoconfigpanel.h"

#include "aoapplication.h"
#include "aoconfig.h"
#include "aoguiloader.h"
#include "datatypes.h"
#include "drpather.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDir>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>

AOConfigPanel::AOConfigPanel(AOApplication *p_ao_app, QWidget *p_parent)
    : QWidget(p_parent), m_config(new AOConfig(this)), m_engine(new DRAudioEngine(this))
{
  ao_app = p_ao_app;

  setWindowTitle(tr("Config"));
  setWindowFlag(Qt::WindowMinMaxButtonsHint, false);

  AOGuiLoader loader;
  loader.load_from_file(":res/ui/config_panel.ui", this);

  // tab
  QTabWidget *tab_widget = AO_GUI_WIDGET(QTabWidget, "tab_widget");
  setFocusProxy(tab_widget);
  tab_widget->setCurrentIndex(0);

  // behaviour
  ui_save = AO_GUI_WIDGET(QPushButton, "save");
  ui_close = AO_GUI_WIDGET(QPushButton, "close");
  ui_autosave = AO_GUI_WIDGET(QCheckBox, "autosave");

  // general
  ui_username = AO_GUI_WIDGET(QLineEdit, "username");
  ui_callwords = AO_GUI_WIDGET(QLineEdit, "callwords");
  ui_server_alerts = AO_GUI_WIDGET(QCheckBox, "server_alerts");
  ui_discord_presence = AO_GUI_WIDGET(QGroupBox, "discord_presence");
  ui_discord_hide_server = AO_GUI_WIDGET(QCheckBox, "discord_hide_server");
  ui_discord_hide_character = AO_GUI_WIDGET(QCheckBox, "discord_hide_character");

  // game
  ui_theme = AO_GUI_WIDGET(QComboBox, "theme");
  ui_reload_theme = AO_GUI_WIDGET(QPushButton, "theme_reload");
  ui_gamemode = AO_GUI_WIDGET(QComboBox, "gamemode");
  ui_manual_gamemode = AO_GUI_WIDGET(QCheckBox, "manual_gamemode");
  ui_timeofday = AO_GUI_WIDGET(QComboBox, "timeofday");
  ui_manual_timeofday = AO_GUI_WIDGET(QCheckBox, "manual_timeofday");
  ui_showname = AO_GUI_WIDGET(QLineEdit, "showname");
  ui_always_pre = AO_GUI_WIDGET(QCheckBox, "always_pre");
  ui_chat_tick_interval = AO_GUI_WIDGET(QSpinBox, "chat_tick_interval");

  // IC Chatlog
  ui_log_max_lines = AO_GUI_WIDGET(QSpinBox, "log_length");
  ui_log_display_timestamp = AO_GUI_WIDGET(QCheckBox, "log_display_timestamp");
  ui_log_display_self_highlight = AO_GUI_WIDGET(QCheckBox, "log_display_self_highlight");
  ui_log_format_use_newline = AO_GUI_WIDGET(QCheckBox, "log_format_use_newline");
  ui_log_display_empty_messages = AO_GUI_WIDGET(QCheckBox, "log_display_empty_messages");
  ui_log_display_music_switch = AO_GUI_WIDGET(QCheckBox, "log_display_music_switch");
  ui_log_orientation_top_down = AO_GUI_WIDGET(QRadioButton, "log_orientation_top_down");
  ui_log_orientation_bottom_up = AO_GUI_WIDGET(QRadioButton, "log_orientation_bottom_up");
  ui_log_is_recording = AO_GUI_WIDGET(QCheckBox, "log_recording");

  // audio
  ui_device = AO_GUI_WIDGET(QComboBox, "device");
  ui_favorite_device = AO_GUI_WIDGET(QCheckBox, "favorite_device");
  ui_master = AO_GUI_WIDGET(QSlider, "master");
  ui_master_value = AO_GUI_WIDGET(QLabel, "master_value");
  ui_suppress_background_audio = AO_GUI_WIDGET(QGroupBox, "suppress_background_audio");
  ui_system = AO_GUI_WIDGET(QSlider, "system");
  ui_system_value = AO_GUI_WIDGET(QLabel, "system_value");
  ui_effect = AO_GUI_WIDGET(QSlider, "effect");
  ui_effect_ignore_suppression = AO_GUI_WIDGET(QCheckBox, "effect_ignore_suppression");
  ui_effect_value = AO_GUI_WIDGET(QLabel, "effect_value");
  ui_music = AO_GUI_WIDGET(QSlider, "music");
  ui_music_ignore_suppression = AO_GUI_WIDGET(QCheckBox, "music_ignore_suppression");
  ui_music_value = AO_GUI_WIDGET(QLabel, "music_value");
  ui_blip = AO_GUI_WIDGET(QSlider, "blip");
  ui_blip_ignore_suppression = AO_GUI_WIDGET(QCheckBox, "blip_ignore_suppression");
  ui_blip_value = AO_GUI_WIDGET(QLabel, "blip_value");
  ui_blip_rate = AO_GUI_WIDGET(QSpinBox, "blip_rate");
  ui_blank_blips = AO_GUI_WIDGET(QCheckBox, "blank_blips");

  // themes
  refresh_theme_list();
  refresh_gamemode_list();
  refresh_timeofday_list();

  // input
  // meta
  connect(m_config, SIGNAL(autosave_changed(bool)), ui_autosave, SLOT(setChecked(bool)));

  // general
  connect(m_config, SIGNAL(username_changed(QString)), ui_username, SLOT(setText(QString)));
  connect(m_config, SIGNAL(callwords_changed(QString)), ui_callwords, SLOT(setText(QString)));
  connect(m_config, SIGNAL(server_alerts_changed(bool)), ui_server_alerts, SLOT(setChecked(bool)));
  connect(m_config, SIGNAL(discord_presence_changed(bool)), ui_discord_presence, SLOT(setChecked(bool)));
  connect(m_config, SIGNAL(discord_hide_server_changed(bool)), ui_discord_hide_server, SLOT(setChecked(bool)));
  connect(m_config, SIGNAL(discord_hide_character_changed(bool)), ui_discord_hide_character, SLOT(setChecked(bool)));

  // game
  connect(m_config, SIGNAL(theme_changed(QString)), this, SLOT(on_theme_changed(QString)));
  connect(m_config, SIGNAL(gamemode_changed(QString)), this, SLOT(on_gamemode_changed(QString)));
  connect(m_config, SIGNAL(manual_gamemode_changed(bool)), ui_manual_gamemode, SLOT(setChecked(bool)));
  connect(m_config, SIGNAL(timeofday_changed(QString)), this, SLOT(on_theme_changed(QString)));
  connect(m_config, SIGNAL(manual_timeofday_changed(bool)), ui_manual_timeofday, SLOT(setChecked(bool)));
  connect(m_config, SIGNAL(showname_changed(QString)), ui_showname, SLOT(setText(QString)));
  connect(m_config, SIGNAL(showname_placeholder_changed(QString)), this,
          SLOT(on_showname_placeholder_changed(QString)));
  connect(m_config, SIGNAL(always_pre_changed(bool)), ui_always_pre, SLOT(setChecked(bool)));
  connect(m_config, SIGNAL(chat_tick_interval_changed(int)), ui_chat_tick_interval, SLOT(setValue(int)));

  // log
  connect(m_config, SIGNAL(log_max_lines_changed(int)), ui_log_max_lines, SLOT(setValue(int)));
  connect(m_config, SIGNAL(log_display_timestamp_changed(bool)), ui_log_display_timestamp, SLOT(setChecked(bool)));
  connect(m_config, SIGNAL(log_display_self_highlight_changed(bool)), ui_log_display_self_highlight,
          SLOT(setChecked(bool)));
  connect(m_config, SIGNAL(log_format_use_newline_changed(bool)), ui_log_format_use_newline, SLOT(setChecked(bool)));
  connect(m_config, SIGNAL(log_display_empty_messages_changed(bool)), ui_log_display_empty_messages,
          SLOT(setChecked(bool)));
  connect(m_config, SIGNAL(log_display_music_switch_changed(bool)), ui_log_display_music_switch,
          SLOT(setChecked(bool)));
  connect(m_config, SIGNAL(log_is_topdown_changed(bool)), this, SLOT(on_log_is_topdown_changed(bool)));
  connect(m_config, SIGNAL(log_is_recording_changed(bool)), ui_log_is_recording, SLOT(setChecked(bool)));

  // audio
  connect(m_config, SIGNAL(master_volume_changed(int)), ui_master, SLOT(setValue(int)));
  connect(m_config, SIGNAL(suppress_background_audio_changed(bool)), ui_suppress_background_audio,
          SLOT(setChecked(bool)));
  connect(m_config, SIGNAL(system_volume_changed(int)), ui_system, SLOT(setValue(int)));
  connect(m_config, SIGNAL(effect_volume_changed(int)), ui_effect, SLOT(setValue(int)));
  connect(m_config, SIGNAL(effect_ignore_suppression_changed(bool)), ui_effect_ignore_suppression,
          SLOT(setChecked(bool)));
  connect(m_config, SIGNAL(music_volume_changed(int)), ui_music, SLOT(setValue(int)));
  connect(m_config, SIGNAL(music_ignore_suppression_changed(bool)), ui_music_ignore_suppression,
          SLOT(setChecked(bool)));
  connect(m_config, SIGNAL(blip_volume_changed(int)), ui_blip, SLOT(setValue(int)));
  connect(m_config, SIGNAL(blip_ignore_suppression_changed(bool)), ui_blip_ignore_suppression, SLOT(setChecked(bool)));
  connect(m_config, SIGNAL(blip_rate_changed(int)), ui_blip_rate, SLOT(setValue(int)));
  connect(m_config, SIGNAL(blank_blips_changed(bool)), ui_blank_blips, SLOT(setChecked(bool)));

  connect(m_engine, SIGNAL(device_changed(DRAudioDevice)), this, SLOT(on_audio_device_changed(DRAudioDevice)));
  connect(m_engine, SIGNAL(device_list_changed(QList<DRAudioDevice>)), this,
          SLOT(on_audio_device_list_changed(QList<DRAudioDevice>)));
  connect(m_engine, SIGNAL(favorite_device_changed(DRAudioDevice)), this,
          SLOT(on_favorite_audio_device_changed(DRAudioDevice)));

  // output
  // meta
  connect(ui_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(ui_save, SIGNAL(clicked()), m_config, SLOT(save_file()));
  connect(ui_autosave, SIGNAL(toggled(bool)), m_config, SLOT(set_autosave(bool)));

  // general
  connect(ui_username, SIGNAL(editingFinished()), this, SLOT(username_editing_finished()));
  connect(ui_callwords, SIGNAL(editingFinished()), this, SLOT(callwords_editing_finished()));
  connect(ui_server_alerts, SIGNAL(toggled(bool)), m_config, SLOT(set_server_alerts(bool)));
  connect(ui_discord_presence, SIGNAL(toggled(bool)), m_config, SLOT(set_discord_presence(bool)));
  connect(ui_discord_hide_server, SIGNAL(toggled(bool)), m_config, SLOT(set_discord_hide_server(const bool)));
  connect(ui_discord_hide_character, SIGNAL(toggled(bool)), m_config, SLOT(set_discord_hide_character(const bool)));

  // game
  connect(ui_theme, SIGNAL(currentIndexChanged(QString)), m_config, SLOT(set_theme(QString)));
  connect(ui_reload_theme, SIGNAL(clicked()), this, SLOT(on_reload_theme_clicked()));
  connect(ui_gamemode, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_gamemode_index_changed(QString)));
  connect(ui_manual_gamemode, SIGNAL(toggled(bool)), m_config, SLOT(set_manual_gamemode(bool)));
  connect(ui_timeofday, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_timeofday_index_changed(QString)));
  connect(ui_manual_timeofday, SIGNAL(toggled(bool)), m_config, SLOT(set_manual_timeofday(bool)));
  connect(ui_showname, SIGNAL(editingFinished()), this, SLOT(showname_editing_finished()));
  connect(ui_always_pre, SIGNAL(toggled(bool)), m_config, SLOT(set_always_pre(bool)));
  connect(ui_chat_tick_interval, SIGNAL(valueChanged(int)), m_config, SLOT(set_chat_tick_interval(int)));

  // out, log
  connect(ui_log_max_lines, SIGNAL(valueChanged(int)), m_config, SLOT(set_log_max_lines(int)));
  connect(ui_log_display_timestamp, SIGNAL(toggled(bool)), m_config, SLOT(set_log_display_timestamp(bool)));
  connect(ui_log_display_self_highlight, SIGNAL(toggled(bool)), m_config, SLOT(set_log_display_self_highlight(bool)));
  connect(ui_log_format_use_newline, SIGNAL(toggled(bool)), m_config, SLOT(set_log_format_use_newline(bool)));
  connect(ui_log_display_empty_messages, SIGNAL(toggled(bool)), m_config, SLOT(set_log_display_empty_messages(bool)));
  connect(ui_log_display_music_switch, SIGNAL(toggled(bool)), m_config, SLOT(set_log_display_music_switch(bool)));
  connect(ui_log_orientation_top_down, SIGNAL(toggled(bool)), m_config, SLOT(set_log_is_topdown(bool)));
  connect(ui_log_is_recording, SIGNAL(toggled(bool)), m_config, SLOT(set_log_is_recording(bool)));
  connect(ui_suppress_background_audio, SIGNAL(toggled(bool)), m_config, SLOT(set_suppress_background_audio(bool)));
  connect(ui_device, SIGNAL(currentIndexChanged(int)), this, SLOT(on_device_current_index_changed(int)));
  connect(ui_master, SIGNAL(valueChanged(int)), m_config, SLOT(set_master_volume(int)));
  connect(ui_master, SIGNAL(valueChanged(int)), this, SLOT(on_master_value_changed(int)));
  connect(ui_system, SIGNAL(valueChanged(int)), m_config, SLOT(set_system_volume(int)));
  connect(ui_system, SIGNAL(valueChanged(int)), this, SLOT(on_system_value_changed(int)));
  connect(ui_effect, SIGNAL(valueChanged(int)), m_config, SLOT(set_effect_volume(int)));
  connect(ui_effect, SIGNAL(valueChanged(int)), this, SLOT(on_effect_value_changed(int)));
  connect(ui_effect_ignore_suppression, SIGNAL(toggled(bool)), m_config, SLOT(set_effect_ignore_suppression(bool)));
  connect(ui_music, SIGNAL(valueChanged(int)), m_config, SLOT(set_music_volume(int)));
  connect(ui_music, SIGNAL(valueChanged(int)), this, SLOT(on_music_value_changed(int)));
  connect(ui_music_ignore_suppression, SIGNAL(toggled(bool)), m_config, SLOT(set_music_ignore_suppression(bool)));
  connect(ui_blip, SIGNAL(valueChanged(int)), m_config, SLOT(set_blip_volume(int)));
  connect(ui_blip, SIGNAL(valueChanged(int)), this, SLOT(on_blip_value_changed(int)));
  connect(ui_blip_ignore_suppression, SIGNAL(toggled(bool)), m_config, SLOT(set_blip_ignore_suppression(bool)));
  connect(ui_blip_rate, SIGNAL(valueChanged(int)), m_config, SLOT(set_blip_rate(int)));
  connect(ui_blank_blips, SIGNAL(toggled(bool)), m_config, SLOT(set_blank_blips(bool)));

  // set values
  // meta
  ui_autosave->setChecked(m_config->autosave());

  // general
  ui_username->setText(m_config->username());
  ui_callwords->setText(m_config->callwords());
  ui_server_alerts->setChecked(m_config->server_alerts_enabled());

  // game
  ui_theme->setCurrentText(m_config->theme());
  ui_gamemode->setCurrentText(m_config->gamemode());
  ui_manual_gamemode->setChecked(m_config->manual_gamemode_enabled());
  ui_timeofday->setCurrentText(m_config->timeofday());
  ui_manual_timeofday->setChecked(m_config->manual_timeofday_enabled());
  ui_showname->setText(m_config->showname());
  on_showname_placeholder_changed(m_config->showname_placeholder());
  ui_always_pre->setChecked(m_config->always_pre_enabled());
  ui_chat_tick_interval->setValue(m_config->chat_tick_interval());

  // log
  ui_log_max_lines->setValue(m_config->log_max_lines());

  if (m_config->log_is_topdown_enabled())
  {
    ui_log_orientation_top_down->setChecked(true);
  }
  else
  {
    ui_log_orientation_bottom_up->setChecked(true);
  }

  ui_log_display_timestamp->setChecked(m_config->log_display_timestamp_enabled());
  ui_log_display_self_highlight->setChecked(m_config->log_display_self_highlight_enabled());
  ui_log_format_use_newline->setChecked(m_config->log_format_use_newline_enabled());
  ui_log_display_empty_messages->setChecked(m_config->log_display_empty_messages_enabled());
  ui_log_display_music_switch->setChecked(m_config->log_display_music_switch_enabled());
  ui_log_is_recording->setChecked(m_config->log_is_recording_enabled());

  ui_discord_presence->setChecked(m_config->discord_presence());
  ui_discord_hide_server->setChecked(m_config->discord_hide_server());
  ui_discord_hide_character->setChecked(m_config->discord_hide_character());

  // audio
  update_audio_device_list();
  ui_master->setValue(m_config->master_volume());
  ui_suppress_background_audio->setChecked(m_config->suppress_background_audio());
  ui_system->setValue(m_config->system_volume());
  ui_effect->setValue(m_config->effect_volume());
  ui_effect_ignore_suppression->setChecked(m_config->effect_ignore_suppression());
  ui_music->setValue(m_config->music_volume());
  ui_music_ignore_suppression->setChecked(m_config->music_ignore_suppression());
  ui_blip->setValue(m_config->blip_volume());
  ui_blip_ignore_suppression->setChecked(m_config->blip_ignore_suppression());
  ui_blip_rate->setValue(m_config->blip_rate());
  ui_blank_blips->setChecked(m_config->blank_blips_enabled());

  // Widget enabling connections
  ui_gamemode->setEnabled(m_config->manual_gamemode_enabled());
  ui_timeofday->setEnabled(m_config->manual_timeofday_enabled());
  // The manual gamemode checkbox enables browsing the gamemode combox
  // similarly with time of day
  connect(m_config, SIGNAL(manual_gamemode_changed(bool)), ui_gamemode, SLOT(setEnabled(bool)));
  connect(m_config, SIGNAL(manual_timeofday_changed(bool)), ui_timeofday, SLOT(setEnabled(bool)));
}

void AOConfigPanel::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);

  if (isVisible())
  {
    // refresh theme, gamemode and time of day comboboxes
    refresh_theme_list();
    refresh_gamemode_list();
    refresh_timeofday_list();
  }
}

void AOConfigPanel::refresh_theme_list()
{
  const QString p_prev_text = ui_theme->currentText();

  // block signals
  ui_theme->blockSignals(true);
  ui_theme->clear();

  // themes
  const QString path = DRPather::get_application_path() + "/base/themes";
  for (const QString &i_folder : QDir(ao_app->get_case_sensitive_path(path)).entryList(QDir::Dirs))
  {
    if (i_folder == "." || i_folder == "..")
      continue;
    ui_theme->addItem(i_folder);
  }

  // restore previous selection
  ui_theme->setCurrentText(p_prev_text);

  // unblock
  ui_theme->blockSignals(false);
}

void AOConfigPanel::refresh_gamemode_list()
{
  const QString p_prev_text = ui_gamemode->currentText();

  // block signals
  ui_gamemode->blockSignals(true);
  ui_gamemode->clear();

  // add empty entry indicating no gamemode chosen
  ui_gamemode->addItem("<default>");
  // gamemodes
  QString path = DRPather::get_application_path() + "/base/themes/" + m_config->theme() + "/gamemodes/";
  for (const QString &i_folder : QDir(ao_app->get_case_sensitive_path(path)).entryList(QDir::Dirs))
  {
    if (i_folder == "." || i_folder == "..")
      continue;
    ui_gamemode->addItem(i_folder, i_folder);
  }

  // restore previous selection
  ui_gamemode->setCurrentText(p_prev_text);

  // unblock
  ui_gamemode->blockSignals(false);
}

void AOConfigPanel::refresh_timeofday_list()
{
  const QString p_prev_text = ui_timeofday->currentText();

  // block signals
  ui_timeofday->blockSignals(true);
  ui_timeofday->clear();

  // add empty entry indicating no time of day chosen
  ui_timeofday->addItem("<default>");

  // decide path to look for times of day. This differs whether there is a
  // gamemode chosen or not
  QString path;
  if (m_config->gamemode().isEmpty())
    path = DRPather::get_application_path() + "/base/themes/" + m_config->theme() + "/times/";
  else
    path = DRPather::get_application_path() + "/base/themes/" + m_config->theme() + "/gamemodes/" +
           m_config->gamemode() + "/times/";

  // times of day
  for (const QString &i_folder : QDir(ao_app->get_case_sensitive_path(path)).entryList(QDir::Dirs))
  {
    if (i_folder == "." || i_folder == "..")
      continue;
    ui_timeofday->addItem(i_folder, i_folder);
  }

  // restore previous selection
  ui_timeofday->setCurrentText(p_prev_text);

  // unblock
  ui_timeofday->blockSignals(false);
}

void AOConfigPanel::update_audio_device_list()
{
  const auto current_device = m_engine->get_device();
  const auto favorite_device = m_engine->get_favorite_device();

  QSignalBlocker device_blocker(ui_device);
  ui_device->clear();

  std::optional<int> current_device_index;
  {
    for (DRAudioDevice &i_device : m_engine->get_device_list())
    {
      if (!i_device.is_enabled())
        continue;
      ui_device->addItem(i_device.get_name(), i_device.get_driver());
      int item_index = ui_device->count() - 1;

      if (current_device.has_value() && current_device.value().get_driver() == i_device.get_driver())
        current_device_index = item_index;

      if (favorite_device.has_value() && favorite_device.value().get_driver() == i_device.get_driver())
        ui_device->setItemData(item_index, QColor(Qt::green), Qt::BackgroundRole);
    }

    const bool has_items(ui_device->count());
    if (!has_items)
      ui_device->addItem(tr("<no sound>"));
    ui_device->setEnabled(has_items);
  }

  if (current_device_index.has_value())
    ui_device->setCurrentIndex(current_device_index.value());
}

void AOConfigPanel::on_reload_theme_clicked()
{
  Q_EMIT reload_theme();
}

void AOConfigPanel::on_theme_changed(QString p_name)
{
  refresh_theme_list();
  refresh_gamemode_list();
  refresh_timeofday_list();
  ui_theme->setCurrentText(p_name);
}

void AOConfigPanel::on_gamemode_changed(QString p_name)
{
  refresh_theme_list();
  refresh_gamemode_list();
  refresh_timeofday_list();
  ui_gamemode->setCurrentText(p_name);
}

void AOConfigPanel::on_timeofday_changed(QString p_name)
{
  refresh_theme_list();
  refresh_gamemode_list();
  refresh_timeofday_list();
  ui_timeofday->setCurrentText(p_name);
}

void AOConfigPanel::on_gamemode_index_changed(QString p_text)
{
  Q_UNUSED(p_text);
  m_config->set_gamemode(ui_gamemode->currentData().toString());
}

void AOConfigPanel::on_timeofday_index_changed(QString p_text)
{
  Q_UNUSED(p_text);
  m_config->set_timeofday(ui_timeofday->currentData().toString());
}

void AOConfigPanel::on_showname_placeholder_changed(QString p_text)
{
  const QString l_showname(p_text.trimmed().isEmpty() ? "Showname" : p_text);
  ui_showname->setPlaceholderText(l_showname);
  ui_showname->setToolTip(l_showname);
}

void AOConfigPanel::on_log_is_topdown_changed(bool p_enabled)
{
  ui_log_orientation_top_down->setChecked(p_enabled);
  ui_log_orientation_bottom_up->setChecked(!p_enabled);
}

void AOConfigPanel::on_device_current_index_changed(int p_index)
{
  if (p_index == -1 || p_index >= ui_device->count())
    return;

  const QString target_device_driver = ui_device->itemData(p_index).toString();
  for (DRAudioDevice &i_device : m_engine->get_device_list())
    if (target_device_driver == i_device.get_driver())
    {
      m_engine->set_device(i_device);
      m_engine->set_favorite_device(i_device);
      m_config->set_favorite_device_driver(i_device.get_driver());
      break;
    }
}

void AOConfigPanel::on_audio_device_changed(DRAudioDevice p_device)
{
  Q_UNUSED(p_device)
  update_audio_device_list();
}

void AOConfigPanel::on_favorite_audio_device_changed(DRAudioDevice p_device)
{
  Q_UNUSED(p_device)
  update_audio_device_list();
}

void AOConfigPanel::on_audio_device_list_changed(QList<DRAudioDevice> p_device_list)
{
  Q_UNUSED(p_device_list);
  update_audio_device_list();
}

void AOConfigPanel::on_master_value_changed(int p_num)
{
  ui_master_value->setText(QString::number(p_num) + "%");
}

void AOConfigPanel::on_system_value_changed(int p_num)
{
  ui_system_value->setText(QString::number(p_num) + "%");
}

void AOConfigPanel::on_effect_value_changed(int p_num)
{
  ui_effect_value->setText(QString::number(p_num) + "%");
}

void AOConfigPanel::on_music_value_changed(int p_num)
{
  ui_music_value->setText(QString::number(p_num) + "%");
}

void AOConfigPanel::on_blip_value_changed(int p_num)
{
  ui_blip_value->setText(QString::number(p_num) + "%");
}

void AOConfigPanel::username_editing_finished()
{
  m_config->set_username(ui_username->text());
}

void AOConfigPanel::showname_editing_finished()
{
  m_config->set_showname(ui_showname->text());
}

void AOConfigPanel::callwords_editing_finished()
{
  m_config->set_callwords(ui_callwords->text());
}

void AOConfigPanel::on_config_reload_theme_requested()
{
  refresh_theme_list();
  refresh_gamemode_list();
  refresh_timeofday_list();
}
