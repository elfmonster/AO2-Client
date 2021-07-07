#include "courtroom.h"

#include "aoapplication.h"
#include "aoblipplayer.h"
#include "aobutton.h"
#include "aocharmovie.h"
#include "aoconfig.h"
#include "aoevidencedisplay.h"
#include "aoimagedisplay.h"
#include "aomovie.h"
#include "aomusicplayer.h"
#include "aonotearea.h"
#include "aonotepicker.h"
#include "aoscene.h"
#include "aosfxplayer.h"
#include "aoshoutplayer.h"
#include "aosystemplayer.h"
#include "aotimer.h"
#include "commondefs.h"
#include "debug_functions.h"
#include "drchatlog.h"
#include "drdiscord.h"
#include "drpacket.h"
#include "file_functions.h"
#include "hardware_functions.h"
#include "lobby.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDir>
#include <QInputDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QVBoxLayout>

const int Courtroom::DEFAULT_WIDTH = 714;
const int Courtroom::DEFAULT_HEIGHT = 668;

Courtroom::Courtroom(AOApplication *p_ao_app) : QMainWindow()
{
  ao_app = p_ao_app;
  ao_config = new AOConfig(this);

  m_reload_timer = new QTimer(this);
  m_reload_timer->setInterval(200);
  m_reload_timer->setSingleShot(true);
  connect(m_reload_timer, SIGNAL(timeout()), this, SLOT(on_app_reload_theme_requested()));
  connect(ao_app, SIGNAL(reload_theme()), m_reload_timer, SLOT(start()));

  create_widgets();
  connect_widgets();

  setup_courtroom();
  set_char_select();
}

Courtroom::~Courtroom()
{
  stop_all_audio();
}

void Courtroom::set_character_list(QVector<char_type> p_chr_list)
{
  m_chr_list = p_chr_list;
}

void Courtroom::set_area_list(QStringList p_area_list)
{
  m_area_list = p_area_list;
  list_areas();
}

void Courtroom::set_music_list(QStringList p_music_list)
{
  m_music_list = p_music_list;
  list_music();
}

void Courtroom::setup_courtroom()
{
  load_shouts();
  load_effects();
  load_wtce();
  load_free_blocks();
  load_sfx_list_theme();

  current_evidence_page = 0;
  current_evidence = 0;
  set_evidence_page();

  // Update widgets first, then check if everything is valid
  // This will also handle showing the correct shouts, effects and wtce buttons,
  // and cycling through them if the buttons that are supposed to be displayed
  // do not exist It will also take care of free blocks

  const bool l_chr_select_visible = ui_char_select_background->isVisible();
  set_widgets();
  ui_char_select_background->setVisible(l_chr_select_visible);

  update_iniswap_list();

  m_shout_state = 0;
  m_shout_current = 0;
  check_shouts();
  if (m_shout_current < shouts_enabled.length() && !shouts_enabled[m_shout_current])
    cycle_shout(1);

  m_effect_state = 0;
  m_effect_current = 0;
  check_effects();
  if (m_effect_current < effects_enabled.length() && !effects_enabled[m_effect_current])
    cycle_effect(1);

  m_wtce_current = 0;
  reset_wtce_buttons();
  check_wtce();
  if (is_judge && (m_wtce_current < wtce_enabled.length() && !wtce_enabled[m_wtce_current]))
    cycle_wtce(1);

  check_free_blocks();

  ui_flip->show();

  list_music();
  list_areas();

  set_widget_names();
  set_widget_layers();

  for (AOTimer *i_timer : qAsConst(ui_timers))
    i_timer->redraw();
}

void Courtroom::enter_courtroom(int p_cid)
{
  qDebug() << "enter_courtroom";

  // unmute audio
  suppress_audio(false);

  const QString l_prev_chr_name = get_character_ini();

  // character =================================================================
  const bool l_changed_chr_id = (m_chr_id != p_cid);
  m_chr_id = p_cid;

  if (l_changed_chr_id)
    update_default_iniswap_item();

  QLineEdit *l_current_field = ui_ic_chat_message;
  if (ui_ooc_chat_message->hasFocus())
    l_current_field = ui_ooc_chat_message;
  const int l_current_cursor_pos = l_current_field->cursorPosition();

  const QString l_chr_name = get_character_ini();
  if (is_spectating())
  {
    ao_app->get_discord()->clear_character_name();
    ao_config->clear_showname_placeholder();
  }
  else
  {
    const QString l_showname = ao_app->get_showname(l_chr_name);
    const QString l_final_showname = l_showname.trimmed().isEmpty() ? l_chr_name : l_showname;
    ao_app->get_discord()->set_character_name(l_final_showname);
    ao_config->set_showname_placeholder(l_final_showname);

    if (ao_app->has_character_declaration_feature())
    {
      QStringList l_content{l_chr_name, l_final_showname};
      ao_app->send_server_packet(DRPacket("chrini", l_content));
    }
  }
  const bool l_changed_chr = l_chr_name != l_prev_chr_name;
  if (l_changed_chr)
    set_character_position(ao_app->get_char_side(l_chr_name));
  select_base_character_iniswap();

  const int l_prev_emote_count = m_emote_list.count();
  m_emote_list = ao_app->get_emote_list(l_chr_name);

  const QString l_prev_emote = ui_emote_dropdown->currentText();
  fill_emote_dropdown();

  if (l_changed_chr || l_prev_emote_count != m_emote_list.count())
  {
    m_emote_id = 0;
    m_current_emote_page = 0;
    ui_pre->setChecked(ui_pre->isChecked() || ao_config->always_pre_enabled());
  }
  else
  {
    ui_emote_dropdown->setCurrentText(l_prev_emote);
  }
  refresh_emote_page();

  load_current_character_sfx_list();
  select_default_sfx();

  ui_emotes->setHidden(is_spectating());
  ui_emote_dropdown->setDisabled(is_spectating());
  ui_iniswap_dropdown->setDisabled(is_spectating());
  ui_ic_chat_message->setDisabled(is_spectating());

  // restore line field focus
  l_current_field->setFocus();
  l_current_field->setCursorPosition(l_current_cursor_pos);

  const QString l_showname = ao_config->showname();
  if (!l_showname.isEmpty() && !is_first_showname_sent)
    send_showname_packet(l_showname);

  ui_char_select_background->hide();
}

void Courtroom::done_received()
{
  m_chr_id = SpectatorId;

  suppress_audio(true);

  set_char_select_page();

  set_char_select();

  show();

  ui_spectator->show();
}

void Courtroom::set_window_title(QString p_title)
{
  this->setWindowTitle(p_title);
}

void Courtroom::set_scene()
{
  // witness is default if pos is invalid
  QString f_background = "witnessempty";
  QString f_desk_image = "stand";
  QString f_desk_mod = m_chatmessage[CMDeskModifier];
  QString f_side = m_chatmessage[CMPosition];
  QString ini_path = ao_app->get_background_path(BACKGROUND_BACKGROUNDS_INI);

  if (file_exists(ini_path))
  {
    f_background = ao_app->read_ini(f_side, ini_path);
    f_desk_image = ao_app->read_ini(f_side + "_desk", ini_path);

    if (f_desk_mod == "0") // keeping a bit of the functionality for now
    {
      ui_vp_desk->hide();
    }
  }
  else
  {
    if (f_side == "def")
    {
      f_background = "defenseempty";
      f_desk_image = "defensedesk";
    }
    else if (f_side == "pro")
    {
      f_background = "prosecutorempty";
      f_desk_image = "prosecutiondesk";
    }
    else if (f_side == "jud")
    {
      f_background = "judgestand";
      f_desk_image = "judgedesk";
    }
    else if (f_side == "hld")
    {
      f_background = "helperstand";
      f_desk_image = "helperdesk";
    }
    else if (f_side == "hlp")
    {
      f_background = "prohelperstand";
      f_desk_image = "prohelperdesk";
    }
    else
    {
      f_desk_image = "stand";
    }

    bool has_all_desks = true;
    QStringList alldesks{"defensedesk", "prosecutiondesk", "stand"};
    for (const QString &desk : alldesks)
    {
      QString full_path = ao_app->find_asset_path({get_background_path(desk)}, animated_or_static_extensions());
      if (full_path.isEmpty())
      {
        has_all_desks = false;
        break;
      }
    }

    if (f_desk_mod == "0" || (f_desk_mod != "1" && (f_side == "jud" || f_side == "hld" || f_side == "hlp")))
      ui_vp_desk->hide();
    else if (!has_all_desks)
      ui_vp_desk->hide();
    else
      ui_vp_desk->show();
  }

  ui_vp_background->set_image(f_background);
  ui_vp_desk->set_image(f_desk_image);
}

void Courtroom::set_taken(int n_char, bool p_taken)
{
  if (n_char >= m_chr_list.size())
  {
    qDebug() << "W: set_taken attempted to set an index bigger than char_list size";
    return;
  }

  char_type f_char;
  f_char.name = m_chr_list.at(n_char).name;
  f_char.taken = p_taken;

  m_chr_list.replace(n_char, f_char);
}

void Courtroom::set_background(QString p_background)
{
  current_background = p_background;
}

void Courtroom::set_tick_rate(const int p_tick_rate)
{
  if (p_tick_rate < 0)
  {
    m_server_tick_rate.reset();
    return;
  }
  m_server_tick_rate = p_tick_rate;
}

void Courtroom::handle_music_anim()
{
  pos_size_type res_a = ao_app->get_element_dimensions("music_name", COURTROOM_DESIGN_INI);
  pos_size_type res_b = ao_app->get_element_dimensions("music_area", COURTROOM_DESIGN_INI);
  float speed = static_cast<float>(ao_app->get_font_property("music_name_speed", COURTROOM_FONTS_INI));

  QFont f_font = ui_vp_music_name->font();
  QFontMetrics fm(f_font);
  int dist;
  if (ao_app->read_theme_ini_bool("enable_const_music_speed", COURTROOM_CONFIG_INI))
    dist = res_b.width;
  else
    dist = fm.horizontalAdvance(ui_vp_music_name->toPlainText());
  int time = static_cast<int>(1000000 * dist / speed);
  music_anim->setLoopCount(-1);
  music_anim->setDuration(time);
  music_anim->setStartValue(QRect(res_b.width + res_b.x, res_a.y, res_a.width, res_a.height));
  music_anim->setEndValue(QRect(-dist + res_a.x, res_a.y, res_a.width, res_a.height));
  music_anim->start();
}

void Courtroom::handle_clock(QString time)
{
  current_clock = time.toInt();
  if (current_clock < 0 || current_clock > 23)
    current_clock = -1;
  qInfo() << QString("Clock time changed to %1").arg(current_clock);

  ui_vp_clock->hide();

  if (current_clock == -1)
  {
    qInfo() << "Unknown time; no asset to be used.";
    return;
  }

  qDebug() << "Displaying clock asset...";
  QString clock_filename = "hours/" + QString::number(current_clock);
  const QString asset_path = ao_app->find_theme_asset_path(clock_filename, animated_or_static_extensions());
  if (asset_path.isEmpty())
  {
    qDebug() << "Asset not found; aborting.";
    return;
  }
  ui_vp_clock->play(clock_filename);
  ui_vp_clock->show();
}

void Courtroom::list_music()
{
  ui_music_list->clear();

  const QString l_item_filter = ui_music_search->text();
  const QBrush l_song_brush(ao_app->get_color("found_song_color", COURTROOM_DESIGN_INI));
  const QBrush l_missing_song_brush(ao_app->get_color("missing_song_color", COURTROOM_DESIGN_INI));
  for (const QString &i_item_name : qAsConst(m_music_list))
  {
    if (!i_item_name.contains(l_item_filter, Qt::CaseInsensitive))
      continue;
    QListWidgetItem *l_item = new QListWidgetItem(i_item_name, ui_music_list);
    const QString l_song_path = ao_app->find_asset_path({ao_app->get_music_path(i_item_name)}, audio_extensions());
    l_item->setBackground(l_song_path.isEmpty() ? l_missing_song_brush : l_song_brush);
  }
}

void Courtroom::list_areas()
{
  ui_area_list->clear();

  const QString l_item_filter = ui_music_search->text();
  const QBrush l_area_brush(ao_app->get_color("area_free_color", COURTROOM_DESIGN_INI));
  for (const QString &i_item_name : qAsConst(m_area_list))
  {
    if (!i_item_name.contains(l_item_filter, Qt::CaseInsensitive))
      continue;
    QListWidgetItem *l_item = new QListWidgetItem(i_item_name, ui_area_list);
    l_item->setBackground(l_area_brush);
  }
}

void Courtroom::list_note_files()
{
  QString f_config = ao_app->get_base_path() + CONFIG_FILESABSTRACT_INI;
  QFile f_file(f_config);
  if (!f_file.open(QIODevice::ReadOnly))
  {
    qDebug() << "Couldn't open" << f_config;
    return;
  }

  QString f_filestring = "";
  QString f_filename = "";

  QTextStream in(&f_file);

  QVBoxLayout *f_layout = ui_note_area->m_layout;

  while (!in.atEnd())
  {
    QString line = in.readLine().trimmed();

    QStringList f_contents = line.split("=");
    if (f_contents.size() < 2)
      continue;

    int f_index = f_contents.at(0).toInt();
    f_filestring = f_filename = f_contents.at(1).trimmed();

    if (f_contents.size() > 2)
      f_filename = f_contents.at(2).trimmed();

    while (f_index >= f_layout->count())
      on_add_button_clicked();

    AONotePicker *f_notepicker = static_cast<AONotePicker *>(f_layout->itemAt(f_index)->widget());
    f_notepicker->ui_line->setText(f_filename);
    f_notepicker->m_file = f_filestring;
  }
}

void Courtroom::load_note()
{
  // Do not attempt to load anything if no file was chosen. This makes it so
  // that notepad text is kept in client if the user has decided not to choose a
  // file to save to. Of course, this is ephimeral storage, it will be erased
  // after the client closes or when the user decides to load a file.
  if (current_file.isEmpty())
    return;
  QString f_text = ao_app->read_note(current_file);
  ui_vp_notepad->setText(f_text);
}

void Courtroom::save_note()
{

  QString f_text = ui_vp_notepad->toPlainText();

  ao_app->write_note(f_text, current_file);
}

void Courtroom::save_textlog(QString p_text)
{
  QString f_file = ao_app->get_base_path() + icchatlogsfilename;

  ao_app->append_note("[" + QTime::currentTime().toString() + "]" + p_text, f_file);
}

void Courtroom::append_server_chatmessage(QString p_name, QString p_message)
{
  ui_ooc_chatlog->append_chatmessage(p_name, p_message);
  if (ao_config->log_is_recording_enabled())
    save_textlog("(OOC)" + p_name + ": " + p_message);
}

void Courtroom::ignore_next_showname()
{
  is_next_showname_ignored = true;
}

/**
 * @brief Send a packet to set the showname of the user to
 * the server.
 * @param p_showname The showname.
 */
void Courtroom::send_showname_packet(QString p_showname)
{
  if (is_next_showname_ignored)
  {
    is_next_showname_ignored = false;
    return;
  }

  is_first_showname_sent = true;

  if (ao_app->has_showname_declaration_feature())
  {
    ao_app->send_server_packet(DRPacket("SN", {p_showname}));
  }
  else
  {
    send_ooc_packet(QString("/showname %1").arg(p_showname));
  }
}

void Courtroom::on_showname_changed(QString p_showname)
{
  ui_ic_chat_showname->setText(p_showname);
  send_showname_packet(p_showname);
}

bool Courtroom::is_spectating()
{
  return m_chr_id == SpectatorId;
}

void Courtroom::on_showname_placeholder_changed(QString p_showname_placeholder)
{
  const QString l_showname(p_showname_placeholder.trimmed().isEmpty() ? "Showname" : p_showname_placeholder);
  ui_ic_chat_showname->setPlaceholderText(l_showname);
  ui_ic_chat_showname->setToolTip(l_showname);
}

void Courtroom::on_character_ini_changed()
{
  enter_courtroom(m_chr_id);
}

void Courtroom::on_ic_message_return_pressed()
{
  if (ui_ic_chat_message->text() == "" || is_client_muted)
    return;

  if ((anim_state < 3 || text_state < 2) && m_shout_state == 0)
    return;

  //  qDebug() << "prev_emote = " << prev_emote << "current_emote = " <<
  //  current_emote;

  //  qDebug() << "same_emote = " << same_emote;
  // MS#
  // deskmod#
  // pre-emote#
  // character#
  // emote#
  // message#
  // side#
  // sfx-name#
  // emote_modifier#
  // char_id#
  // sfx_delay#
  // objection_modifier#
  // evidence#
  // placeholder#
  // realization#
  // text_color#%

  QStringList packet_contents;

  const DREmote &l_emote = get_emote(m_emote_id);

  const QString l_desk_modifier =
      l_emote.desk_modifier == -1 ? QString("chat") : QString::number(l_emote.desk_modifier);
  packet_contents.append(l_desk_modifier);

  packet_contents.append(l_emote.anim);

  packet_contents.append(get_character_ini());

  if (ui_hidden->isChecked())
    packet_contents.append("../../misc/blank");
  else
    packet_contents.append(l_emote.dialog);

  packet_contents.append(ui_ic_chat_message->text());

  const QString l_side = ao_app->get_char_side(get_character_ini());
  packet_contents.append(l_side);

  // sfx file
  const QString l_sound_file = current_sfx_file();
  packet_contents.append(l_sound_file.isEmpty() ? "0" : l_sound_file);
  // TODO remove empty string workaround for pre-DRO 1.0.0

  int l_emote_modifier = l_emote.modifier;
  // needed or else legacy won't understand what we're saying
  if (m_shout_state > 0)
  {
    if (l_emote_modifier == 5)
      l_emote_modifier = 6;
    else
      l_emote_modifier = 2;
  }
  else if (ui_pre->isChecked())
  {
    if (l_emote_modifier == 0)
      l_emote_modifier = 1;
  }
  else
  {
    if (l_emote_modifier == 1)
      l_emote_modifier = 0;
    else if (l_emote_modifier == 4)
      l_emote_modifier = 5;
  }

  packet_contents.append(QString::number(l_emote_modifier));
  packet_contents.append(QString::number(m_chr_id));

  if (l_emote.sound_file == current_sfx_file())
    packet_contents.append(QString::number(l_emote.sound_delay));
  else
    packet_contents.append("0");

  QString f_obj_state;

  if (m_shout_state < 0)
    f_obj_state = "0";
  else
    f_obj_state = QString::number(m_shout_state);

  packet_contents.append(f_obj_state);

  if (is_presenting_evidence)
    // the evidence index is shifted by 1 because 0 is no evidence per legacy
    // standards besides, older clients crash if we pass -1
    packet_contents.append(QString::number(current_evidence + 1));
  else
    packet_contents.append("0");

  QString f_flip = ui_flip->isChecked() ? "1" : "0";
  packet_contents.append(f_flip);

  packet_contents.append(QString::number(m_effect_state));

  QString f_text_color;

  if (m_text_color < 0)
    f_text_color = "0";
  else if (m_text_color > ui_text_color->count())
    f_text_color = "0";
  else
    f_text_color = QString::number(m_text_color);

  packet_contents.append(f_text_color);
  ao_app->send_server_packet(DRPacket("MS", packet_contents));
}

void Courtroom::handle_acknowledged_ms()
{
  ui_ic_chat_message->clear();

  // reset states
  ui_pre->setChecked(ao_config->always_pre_enabled());

  reset_shout_buttons();
  reset_effect_buttons();
  reset_wtce_buttons();
  clear_sfx_selection();

  is_presenting_evidence = false;
  ui_evidence_present->set_image("present_disabled.png");
}

void Courtroom::handle_chatmessage(QStringList p_contents)
{
  if (p_contents.size() < 15)
    return;
  else if (p_contents.size() == 15)
    p_contents.append(nullptr);

  for (int i = 0; i < MESSAGE_SIZE; ++i)
    m_chatmessage[i] = p_contents[i];

  int f_char_id = m_chatmessage[CMChrId].toInt();

  if (f_char_id == SpectatorId)
  {
    is_system_speaking = true;
    m_chatmessage[CMChrId] = "0";
    f_char_id = 0;
  }
  else
    is_system_speaking = false;

  if (f_char_id < 0 || f_char_id >= m_chr_list.size())
    return;

  const QString l_message = QString(m_chatmessage[CMMessage])
                                .remove(QRegularExpression("(?<!\\\\)(\\{|\\})"))
                                .replace(QRegularExpression("\\\\(\\{|\\})"), "\\1");
  chatmessage_is_empty = l_message.trimmed().isEmpty();
  m_msg_is_first_person = false;

  // reset our ui state if client just spoke
  if (m_chr_id == f_char_id && is_system_speaking == false)
  {
#ifdef DRO_ACKMS // TODO WARNING remove entire block on 1.0.0 release
    // If the server does not have the feature of acknowledging our MS
    // messages, assume in this if that the message is proof the server
    // acknowledged our message. It is not quite the same, as it is
    // possible the server crafted a message with the same char_id
    // as the client, but the client did not send that message, but it is
    // the best we can do.
    if (!ao_app->has_message_acknowledgement_feature())
    {
      handle_acknowledged_ms();
    }

    // If the server does have the feature of acknowledging our MS messages,
    // it will have sent an ackMS packet prior to the MS one, so chat would
    // have been cleared and thus we need not do anything else.
#endif

    // update first person mode status
    m_msg_is_first_person = ao_app->get_first_person_enabled();
  }

  QString f_showname;
  qDebug() << "handle_chatmessage";

  // We actually DO wanna fail here if the showname is empty but the system is
  // speaking.
  // Having an empty showname for system is actually what we expect.
  if (m_chatmessage[CMShowName].isEmpty() && !is_system_speaking)
  {
    f_showname = ao_app->get_showname(m_chr_list.at(f_char_id).name);
  }
  else
  {
    f_showname = m_chatmessage[CMShowName];
  }

  ui_vp_chat_arrow->hide();
  m_effects_player->stop_all();

  text_state = 0;
  anim_state = 0;
  stop_chat_timer();
  ui_vp_objection->stop();
  ui_vp_evidence_display->reset();

  // reset effect
  ui_vp_effect->stop();

  if (is_system_speaking)
    append_system_text(f_showname, l_message);
  else
    append_ic_text(f_showname, l_message, false, false, f_char_id == m_chr_id);

  if (ao_config->log_is_recording_enabled() && (!chatmessage_is_empty || !is_system_speaking))
  {
    save_textlog(f_showname + ": " + l_message);
  }

  int objection_mod = m_chatmessage[CMShoutModifier].toInt();
  QString f_char = m_chatmessage[CMChrName];

  // if an objection is used
  if (objection_mod > 0)
  {
    int emote_mod = m_chatmessage[CMEmoteModifier].toInt();
    if (emote_mod == 0)
      m_chatmessage[CMEmoteModifier] = 1;

    // handles cases 1-8 (5-8 are DRO only)
    if (objection_mod >= 1 && objection_mod <= ui_shouts.size() && ui_shouts.size() > 0) // check to prevent crashing
    {
      ui_vp_objection->play_interjection(f_char, shout_names.at(objection_mod - 1));
      m_shouts_player->play(f_char, shout_names.at(objection_mod - 1));
    }
    else
      qDebug() << "W: Shout identifier unknown" << objection_mod;
  }
  else
    handle_chatmessage_2();
}

void Courtroom::objection_done()
{
  handle_chatmessage_2();
}

void Courtroom::handle_chatmessage_2() // handles IC
{
  qDebug() << "handle_chatmessage_2";
  ui_vp_player_char->stop();

  if (shout_delayed_reload_theme)
  {
    shout_delayed_reload_theme = false;
    on_app_reload_theme_requested();
  }

  QString real_name = m_chr_list.at(m_chatmessage[CMChrId].toInt()).name;

  QString f_showname;

  if (m_chatmessage[CMShowName].isEmpty())
  {
    f_showname = ao_app->get_showname(real_name);
  }
  else
  {
    f_showname = m_chatmessage[CMShowName];
  }

  ui_vp_showname->setText(f_showname);

  ui_vp_message->clear();
  ui_vp_chatbox->hide();
  ui_vp_showname_image->hide();

  QString l_chatbox_name = ao_app->get_chat(m_chatmessage[CMChrName]);

  if (l_chatbox_name.isEmpty())
  {
    l_chatbox_name = "chatmed.png";

    if (ao_config->log_display_self_highlight_enabled() && m_chatmessage[CMChrId].toInt() == m_chr_id)
    {
      const QString l_chatbox_self_name = "chatmed_self.png";
      if (file_exists(ao_app->find_theme_asset_path(l_chatbox_self_name)))
        l_chatbox_name = l_chatbox_self_name;
    }

    ui_vp_chatbox->set_image(l_chatbox_name);
  }
  else
  {
    QString chatbox_path = ao_app->get_base_path() + "misc/" + l_chatbox_name + ".png";
    ui_vp_chatbox->set_image_from_path(chatbox_path);
  }

  if (m_msg_is_first_person == false)
  {
    set_scene();
  }

  int emote_mod = m_chatmessage[CMEmoteModifier].toInt();

  if (m_chatmessage[CMFlipState].toInt() == 1)
    ui_vp_player_char->set_mirrored(true);
  else
    ui_vp_player_char->set_mirrored(false);

  switch (emote_mod)
  {
  case 1:
  case 2:
  case 6:
    play_preanim();
    break;
  default:
    qDebug() << "W: invalid emote mod: " << QString::number(emote_mod);
    // intentional fallthru
  case 0:
  case 5:
    handle_chatmessage_3();
  }
}

void Courtroom::handle_chatmessage_3()
{
  qDebug() << "handle_chatmessage_3";

  setup_chat();
  int f_evi_id = m_chatmessage[CMEvidenceId].toInt();
  QString f_side = m_chatmessage[CMPosition];

  if (f_evi_id > 0 && f_evi_id <= local_evidence_list.size())
  {
    // shifted by 1 because 0 is no evidence per legacy standards
    QString f_image = local_evidence_list.at(f_evi_id - 1).image;
    // def jud and hlp should display the evidence icon on the RIGHT side
    bool is_left_side = !(f_side == "def" || f_side == "hlp" || f_side == "jud");
    ui_vp_evidence_display->show_evidence(f_image, is_left_side);
  }

  int f_anim_state = 0;
  // BLUE is from an enum in datatypes.h
  bool text_is_blue = m_chatmessage[CMTextColor].toInt() == DR::CBlue;

  if (!text_is_blue && text_state == 1)
    // talking
    f_anim_state = 2;
  else
    // idle
    f_anim_state = 3;

  if (f_anim_state <= anim_state)
    return;

  ui_vp_player_char->stop();
  const QString f_char = m_chatmessage[CMChrName];
  const QString f_emote = m_chatmessage[CMEmote];
  const bool l_hide_emote = (f_emote == "../../misc/blank");

  QString path;
  if (!chatmessage_is_empty && ao_app->read_theme_ini_bool("enable_showname_image", COURTROOM_CONFIG_INI))
  {
    // Asset lookup order
    // 1. In the theme folder (gamemode-timeofday/main/default), in the character
    // folder, look for "showname" + extensions in `exts` in order
    // 2. In the character folder, look for
    // "showname" + extensions in `exts` in order

    path = ao_app->find_theme_asset_path("characters/" + f_char + "/showname", {".png"});
    if (path.isEmpty())
      path = ao_app->find_asset_path({ao_app->get_character_path(f_char, "showname")}, {".png"});
  }

  // Path may be empty if
  // 1. Chat message was empty
  // 2. Enable showname images was false
  // 3. No valid showname image was found
  if (!path.isEmpty())
  {
    ui_vp_showname->hide();
    ui_vp_showname_image->set_image_from_path(path);
    ui_vp_showname_image->show();
  }
  else
  {
    ui_vp_showname->show();
    ui_vp_showname_image->hide();
  }

  ui_vp_player_char->setHidden(l_hide_emote);
  switch (f_anim_state)
  {
  case 2:
    if (m_msg_is_first_person == false)
    {
      ui_vp_player_char->play_talk(f_char, f_emote);
    }
    anim_state = 2;
    break;
  default:
    qDebug() << "W: invalid anim_state: " << f_anim_state;
    [[fallthrough]];
  case 3:
    if (m_msg_is_first_person == false)
    {
      ui_vp_player_char->play_idle(f_char, f_emote);
    }
    anim_state = 3;
    break;
  }

  int effect = m_chatmessage[CMEffectState].toInt();
  QStringList offset = ao_app->get_effect_offset(f_char, effect);
  ui_vp_effect->move(ui_viewport->x() + offset.at(0).toInt(), ui_viewport->y() + offset.at(1).toInt());

  QStringList overlay = ao_app->get_overlay(f_char, effect);
  QString overlay_name = overlay.at(0);
  QString overlay_sfx = overlay.at(1);

  if (effect > 0 && effect <= ui_effects.size() && effect_names.size() > 0) // check to prevent crashing
  {
    QString s_eff = effect_names.at(effect - 1);
    QStringList f_eff = ao_app->get_effect(effect);

    //    QString s_eff = f_eff.at(0).trimmed();
    bool once = f_eff.at(1).trimmed().toInt();

    if (overlay_sfx == "")
      overlay_sfx = ao_app->get_sfx(s_eff);
    //    qDebug() << overlay_sfx << ao_app->get_sfx(s_eff);
    m_effects_player->play_effect(overlay_sfx);
    ui_vp_effect->set_play_once(once);
    if (overlay_name == "")
      overlay_name = s_eff;
    ui_vp_effect->play(overlay_name, f_char);
  }

  QString f_message = m_chatmessage[CMMessage];
  QStringList callwords = ao_app->get_callwords();

  for (const QString &word : callwords)
  {
    if (f_message.contains(word, Qt::CaseInsensitive))
    {
      m_system_player->play(ao_app->get_sfx("word_call"));
      ao_app->alert(this);
      const QString name = "CLIENT";
      const QString message =
          ui_vp_showname->toPlainText() + " has called you via your callword \"" + word + "\": \"" + f_message + "\"";
      ui_ooc_chatlog->append_chatmessage(name, message);
      if (ao_config->log_is_recording_enabled())
        save_textlog("(OOC)" + name + ": " + message);
      break;
    }
  }

  calculate_chat_tick_interval();
  start_chat_timer();
}

void Courtroom::on_chat_config_changed()
{
  update_ic_log(true);
}

void Courtroom::load_ic_text_format()
{
  ui_ic_chatlog->ensurePolished();
  m_ic_log_format.base = QTextCharFormat();
  m_ic_log_format.base.setFont(ui_ic_chatlog->font());
  m_ic_log_format.base.setForeground(ui_ic_chatlog->palette().color(ui_ic_chatlog->foregroundRole()));

  auto set_format_color = [this](const QString &f_identifier, QTextCharFormat &f_format) {
    if (const std::optional<QColor> l_color =
            ao_app->maybe_color(QString("ic_chatlog_%1_color").arg(f_identifier), COURTROOM_FONTS_INI);
        l_color.has_value())
      f_format.setForeground(l_color.value());

    if (ao_app->get_font_property(QString("ic_chatlog_%1_bold").arg(f_identifier), COURTROOM_FONTS_INI))
      f_format.setFontWeight(QFont::Bold);
  };

  m_ic_log_format.name = m_ic_log_format.base;
  set_format_color("showname", m_ic_log_format.name);

  m_ic_log_format.selfname = m_ic_log_format.name;
  if (ao_config->log_display_self_highlight_enabled())
    set_format_color("selfname", m_ic_log_format.selfname);

  m_ic_log_format.message = m_ic_log_format.base;
  set_format_color("message", m_ic_log_format.message);

  m_ic_log_format.system = m_ic_log_format.base;
  set_format_color("system", m_ic_log_format.system);
}

void Courtroom::update_ic_log(bool p_reset_log)
{
  if (const int l_record_count = m_ic_record_list.length() + m_ic_record_queue.length();
      l_record_count > ao_config->log_max_lines())
    m_ic_record_list = m_ic_record_list.mid(l_record_count - ao_config->log_max_lines());

  if (p_reset_log || ao_config->log_max_lines() == 1)
  {
    if (p_reset_log)
    {
      load_ic_text_format();
      QQueue<DRChatRecord> l_new_queue;
      dynamic_cast<QList<DRChatRecord> &>(l_new_queue) = std::move(m_ic_record_list);
      l_new_queue.append(std::move(m_ic_record_queue));
      m_ic_record_queue = std::move(l_new_queue);
    }

    ui_ic_chatlog->clear();
    ui_ic_chatlog->setAlignment(ui_ic_chatlog->get_text_alignment());
  }
  bool l_log_is_empty = m_ic_record_list.length() == 0;

  const bool l_topdown_orientation = ao_config->log_is_topdown_enabled();
  const bool l_use_newline = ao_config->log_format_use_newline_enabled();
  QTextCursor l_cursor = ui_ic_chatlog->textCursor();
  const QTextCursor::MoveOperation move_type = l_topdown_orientation ? QTextCursor::End : QTextCursor::Start;

  const QTextCharFormat &l_name_format = m_ic_log_format.name;
  const QTextCharFormat &l_selfname_format = m_ic_log_format.selfname;
  const QTextCharFormat &l_message_format = m_ic_log_format.message;
  const QTextCharFormat &l_system_format = m_ic_log_format.system;

  QScrollBar *l_scrollbar = ui_ic_chatlog->verticalScrollBar();
  const int l_scroll_pos = l_scrollbar->value();
  const bool l_is_end_scroll_pos = p_reset_log || (l_topdown_orientation ? l_scroll_pos == l_scrollbar->maximum()
                                                                         : l_scroll_pos == l_scrollbar->minimum());

  while (!m_ic_record_queue.isEmpty())
  {
    const DRChatRecord l_record = m_ic_record_queue.takeFirst();
    m_ic_record_list.append(l_record);

    if (!ao_config->log_display_empty_messages_enabled() && l_record.get_message().trimmed().isEmpty())
      continue;

    if (!ao_config->log_display_music_switch_enabled() && l_record.is_music())
      continue;

    l_cursor.movePosition(move_type);

    const QString l_linefeed(QChar::LineFeed);
    if (!l_log_is_empty)
      l_cursor.insertText(l_linefeed + QString(l_use_newline ? l_linefeed : nullptr), l_message_format);
    l_log_is_empty = false;

    if (!l_topdown_orientation)
      l_cursor.movePosition(move_type);

    // self-highlight check
    const QTextCharFormat &l_target_name_format =
        (l_record.is_self() && ao_config->log_display_self_highlight_enabled()) ? l_selfname_format : l_name_format;

    if (ao_config->log_display_timestamp_enabled())
      l_cursor.insertText(QString("[%1] ").arg(l_record.get_timestamp().toString("hh:mm")), l_target_name_format);

    if (l_record.is_system())
    {
      l_cursor.insertText(l_record.get_message(), l_system_format);
    }
    else
    {
      QString l_separator;
      if (l_use_newline)
        l_separator = QString(QChar::LineFeed);
      else if (!l_record.is_music())
        l_separator = ": ";
      else
        l_separator = " ";
      l_cursor.insertText(l_record.get_name() + l_separator, l_target_name_format);
      l_cursor.insertText(l_record.get_message(), l_message_format);
    }
  }

  { // remove unneeded blocks
    const int l_max_block_count = m_ic_record_list.length() * (1 + l_use_newline) +
                                  (l_use_newline * (m_ic_record_list.length() - 1)) + !l_topdown_orientation;
    const QTextCursor::MoveOperation l_orientation = l_topdown_orientation ? QTextCursor::Start : QTextCursor::End;
    const QTextCursor::MoveOperation l_block_orientation =
        l_topdown_orientation ? QTextCursor::NextBlock : QTextCursor::PreviousBlock;

    const int l_remove_block_count = ui_ic_chatlog->document()->blockCount() - l_max_block_count;
    if (l_remove_block_count > 0)
    {
      l_cursor.movePosition(l_orientation);
      for (int i = 0; i < l_remove_block_count; ++i)
        l_cursor.movePosition(l_block_orientation, QTextCursor::KeepAnchor);
      l_cursor.removeSelectedText();
    }
  }

  if (l_is_end_scroll_pos)
    l_scrollbar->setValue(l_topdown_orientation ? l_scrollbar->maximum() : l_scrollbar->minimum());
}

void Courtroom::append_ic_text(QString p_name, QString p_line, bool p_system, bool p_music, bool p_self)
{
  if (p_name.trimmed().isEmpty())
    p_name = "Anonymous";

  if (p_line.trimmed().isEmpty())
    p_line = p_line.trimmed();

  DRChatRecord new_record(p_name, p_line);
  new_record.set_music(p_music);
  new_record.set_system(p_system);
  new_record.set_self(p_self);
  m_ic_record_queue.append(new_record);
  update_ic_log(false);
}

/**
 * @brief Appends a message arriving from system to the IC chatlog.
 *
 * @param p_showname The showname used by the system. Can be an empty string.
 * @param p_line The message that the system is sending.
 */
void Courtroom::append_system_text(QString p_showname, QString p_line)
{
  if (chatmessage_is_empty)
    return;

  append_ic_text(p_showname, p_line, true, false, false);
}

void Courtroom::play_preanim()
{
  // all time values in char.inis are multiplied by a constant(time_mod) to get
  // the actual time
  int sfx_delay = m_chatmessage[CMSoundDelay].toInt() * 60;
  m_sound_timer->start(sfx_delay);

  QString f_preanim = m_chatmessage[CMPreAnim];

  if (f_preanim.trimmed() == "-")
  {
    // no animation, continue
    preanim_done();
    return;
  }

  QString f_char = m_chatmessage[CMChrName];
  // set state
  anim_state = 1;

  if (m_msg_is_first_person == false)
  {
    QString f_anim_path = ao_app->get_character_path(f_char, f_preanim);
    if (ui_vp_player_char->play_pre(f_char, f_preanim))
    {
      qDebug() << "Playing" << f_anim_path;

      // finished
      return;
    }
    else
    {
      qDebug() << "could not find " + f_anim_path;
    }
  }

  // no animation, continue
  preanim_done();
}

void Courtroom::preanim_done()
{
  handle_chatmessage_3();
}

void Courtroom::realization_done()
{
  ui_vp_effect->stop();
}

void Courtroom::setup_chat()
{
  ui_vp_message->clear();

  set_text_color();
  m_rainbow_step = 0;
  // we need to ensure that the text isn't already ticking because this function
  // can be called by two logic paths
  if (text_state != 0)
    return;

  if (chatmessage_is_empty)
  {
    // since the message is empty, it's technically done ticking
    text_state = 2;
    return;
  }

  ui_vp_chatbox->show();

  m_tick_speed = 0;
  m_tick_step = 0;
  is_ignore_next_letter = false;
  m_blip_step = 0;

  // Cache these so chat_tick performs better
  m_chatbox_message_outline = (ao_app->get_font_property("message_outline", COURTROOM_FONTS_INI) == 1);
  m_chatbox_message_enable_highlighting = (ao_app->read_theme_ini_bool("enable_highlighting", COURTROOM_CONFIG_INI));
  m_chatbox_message_highlight_colors = ao_app->get_highlight_colors();

  QString f_gender = ao_app->get_gender(m_chatmessage[CMChrName]);

  //  m_blip_player->set_file(f_gender);
  m_blips_player->set_blips("sfx-blip" + f_gender + ".wav");

  // means text is currently ticking
  text_state = 1;
}

void Courtroom::start_chat_timer()
{
  if (m_tick_timer->isActive())
    return;
  m_tick_timer->start();
}

void Courtroom::stop_chat_timer()
{
  if (!m_tick_timer->isActive())
    return;
  m_tick_timer->stop();
}

void Courtroom::calculate_chat_tick_interval()
{
  double l_tick_rate = ao_config->chat_tick_interval();
  if (m_server_tick_rate.has_value())
    l_tick_rate = qMax(m_server_tick_rate.value(), 0);
  l_tick_rate = qBound(0.0, l_tick_rate * (1.0 - qBound(-1.0, 0.4 * m_tick_speed, 1.0)), l_tick_rate * 2.0);
  m_tick_timer->setInterval(l_tick_rate);
}

void Courtroom::next_chat_letter()
{
  const QString &f_message = m_chatmessage[CMMessage];
  if (m_tick_step >= f_message.length())
  {
    post_chat();
    return;
  }

  // note: this is called fairly often(every 60 ms when char is talking)
  // do not perform heavy operations here
  QTextCharFormat vp_message_format = ui_vp_message->currentCharFormat();
  if (m_chatbox_message_outline)
    vp_message_format.setTextOutline(QPen(Qt::black, 1));
  else
    vp_message_format.setTextOutline(Qt::NoPen);

  const QChar f_character = f_message.at(m_tick_step);
  if (!is_ignore_next_letter && f_character == Qt::Key_Backslash)
  {
    ++m_tick_step;
    is_ignore_next_letter = true;
    next_chat_letter();
    return;
  }
  else if (!is_ignore_next_letter && (f_character == Qt::Key_BraceLeft || f_character == Qt::Key_BraceRight)) // { or }
  {
    ++m_tick_step;
    const bool is_positive = f_character == Qt::Key_BraceRight;
    m_tick_speed = qBound(-3, m_tick_speed + (is_positive ? 1 : -1), 3);
    calculate_chat_tick_interval();
    next_chat_letter();
    start_chat_timer();
    return;
  }
  else if (f_character == Qt::Key_Space)
  {
    ui_vp_message->insertPlainText(f_character);
  }
  else if (m_chatmessage[CMTextColor].toInt() == DR::CRainbow)
  {
    QString html_color;

    switch (m_rainbow_step)
    {
    case 0:
      html_color = "#BA1518";
      break;
    case 1:
      html_color = "#D55900";
      break;
    case 2:
      html_color = "#E7CE4E";
      break;
    case 3:
      html_color = "#65C856";
      break;
    default:
      html_color = "#1596C8";
      m_rainbow_step = -1;
    }

    ++m_rainbow_step;
    // Apply color to the next character
    QColor text_color;
    text_color.setNamedColor(html_color);
    vp_message_format.setForeground(text_color);

    ui_vp_message->textCursor().insertText(f_character, vp_message_format);
  }
  else if (m_chatbox_message_enable_highlighting)
  {
    bool highlight_found = false;
    bool render_character = true;
    // render_character should only be false if the character is a highlight
    // character specifically marked as a character that should not be
    // rendered.
    if (m_message_color_stack.isEmpty())
      m_message_color_stack.push("");

    for (const auto &col : qAsConst(m_chatbox_message_highlight_colors))
    {
      if (f_character == col[0][0] && m_message_color_name != col[1])
      {
        m_message_color_stack.push(col[1]);
        m_message_color_name = m_message_color_stack.top();
        highlight_found = true;
        render_character = (col[2] != "0");
        break;
      }
    }

    // Apply color to the next character
    if (m_message_color_name.isEmpty())
      vp_message_format.setForeground(m_message_color);
    else
    {
      QColor textColor;
      textColor.setNamedColor(m_message_color_name);
      vp_message_format.setForeground(textColor);
    }

    QString m_future_string_color = m_message_color_name;

    for (const auto &col : qAsConst(m_chatbox_message_highlight_colors))
    {
      if (f_character == col[0][1] && !highlight_found)
      {
        if (m_message_color_stack.size() > 1)
          m_message_color_stack.pop();
        m_future_string_color = m_message_color_stack.top();
        render_character = (col[2] != "0");
        break;
      }
    }

    if (render_character)
      ui_vp_message->textCursor().insertText(f_character, vp_message_format);

    m_message_color_name = m_future_string_color;
  }
  else
  {
    ui_vp_message->textCursor().insertText(f_character, vp_message_format);
  }

  QScrollBar *scroll = ui_vp_message->verticalScrollBar();
  scroll->setValue(scroll->maximum());

  if ((f_message.at(m_tick_step) != ' ' || ao_config->blank_blips_enabled()))
  {

    if (m_blip_step % ao_config->blip_rate() == 0)
    {
      m_blip_step = 0;

      // play blip
      m_blips_player->blip_tick();
    }

    ++m_blip_step;
  }

  ui_vp_message->repaint();

  ++m_tick_step;
  is_ignore_next_letter = false;
}

void Courtroom::post_chat()
{
  text_state = 2;
  anim_state = 3;
  stop_chat_timer();

  if (m_msg_is_first_person == false)
  {
    ui_vp_player_char->play_idle(m_chatmessage[CMChrName], m_chatmessage[CMEmote]);
  }

  m_message_color_name = "";
  m_message_color_stack.clear();
  if (!chatmessage_is_empty)
    ui_vp_chat_arrow->restart();
  ui_vp_chat_arrow->setHidden(chatmessage_is_empty);
}

void Courtroom::play_sfx()
{
  const QString l_effect = m_chatmessage[CMSoundName];
  if (l_effect.isEmpty() || l_effect == "0" || l_effect == "1")
    return;
  const QString l_chr = m_chatmessage[CMChrName];
  m_effects_player->play_character_effect(l_chr, l_effect);
}

void Courtroom::set_text_color()
{
  const QMap<DR::Color, DR::ColorInfo> color_map = ao_app->get_chatmessage_colors();
  const DR::Color color = DR::Color(m_chatmessage[CMTextColor].toInt());
  const QString color_code = color_map[color_map.contains(color) ? color : DR::CDefault].code;
  ui_vp_message->setStyleSheet("background-color: rgba(0, 0, 0, 0)");
  m_message_color.setNamedColor(color_code);
}

void Courtroom::set_muted(bool p_muted, int p_cid)
{
  if (p_cid != m_chr_id && p_cid != SpectatorId)
    return;

  if (p_muted)
    ui_muted->show();
  else
  {
    ui_muted->hide();
    ui_ic_chat_message->setFocus();
  }

  ui_muted->resize(ui_ic_chat_message->width(), ui_ic_chat_message->height());
  ui_muted->set_image("muted.png");

  is_client_muted = p_muted;
  ui_ic_chat_message->setEnabled(!p_muted);
}

void Courtroom::set_ban(int p_cid)
{
  if (p_cid != m_chr_id && p_cid != SpectatorId)
    return;

  call_notice("You have been banned.");

  ao_app->construct_lobby();
  ao_app->destruct_courtroom();
}

void Courtroom::handle_song(QStringList p_contents)
{
  if (p_contents.size() < 2)
    return;

  QString f_song = p_contents.at(0);
  int l_chr_id = p_contents.at(1).toInt();

  for (auto &ext : audio_extensions())
  {
    QString r_song = f_song + ext;
    QString song_path = ao_app->get_music_path(r_song);
    if (file_exists(song_path))
    {
      f_song = r_song;
      break;
    }
  }

  if (l_chr_id < 0 || l_chr_id >= m_chr_list.size())
  {
    m_music_player->play(f_song);
  }
  else
  {
    // This 2th argument corresponds to the showname to use when displaying the
    // music change message in IC
    // Backwards compatibility is explicitly kept for older versions of
    // tsuserver that do not send such an argument by assuming an empty showname
    // If there is an empty showname, the client will use instead the default
    // showname of the character.
    QString f_showname;
    if (p_contents.size() == 3)
    {
      f_showname = p_contents.at(2);
    }
    else
    {
      f_showname = "";
    }

    QString str_char;
    if (f_showname.isEmpty())
    {
      str_char = ao_app->get_showname(m_chr_list.at(l_chr_id).name);
    }
    else
    {
      str_char = f_showname;
    }

    append_ic_text(str_char, "has played a song: " + f_song, false, true, l_chr_id == m_chr_id);
    if (ao_config->log_is_recording_enabled())
      save_textlog(str_char + " has played a song: " + f_song);
    m_music_player->play(f_song);
  }

  int pos = f_song.lastIndexOf(QChar('.'));
  QString r_song = f_song.left(pos);

  ui_vp_music_name->setText(r_song);

  handle_music_anim();
}

void Courtroom::handle_wtce(QString p_wtce)
{
  int index = p_wtce.at(p_wtce.size() - 1).digitValue();
  if (index > 0 && index < wtce_names.size() + 1 && wtce_names.size() > 0) // check to prevent crash
  {
    p_wtce.chop(1); // looking for the 'testimony' part
    if (p_wtce == "testimony")
    {
      m_effects_player->play_effect(ao_app->get_sfx(wtce_names[index - 1]));
      ui_vp_wtce->play(wtce_names[index - 1]);
    }
  }
}

void Courtroom::set_hp_bar(int p_bar, int p_state)
{
  if (p_state < 0 || p_state > 10)
    return;

  if (p_bar == 1)
  {
    ui_defense_bar->set_image("defensebar" + QString::number(p_state) + ".png");
    defense_bar_state = p_state;
  }
  else if (p_bar == 2)
  {
    ui_prosecution_bar->set_image("prosecutionbar" + QString::number(p_state) + ".png");
    prosecution_bar_state = p_state;
  }
}

void Courtroom::set_character_position(QString p_pos)
{
  int index = ui_pos_dropdown->findData(p_pos);
  if (index != -1)
    ui_pos_dropdown->setCurrentIndex(index);

  // enable judge mechanics if appropriate
  set_judge_enabled(p_pos == "jud");
}

/**
 * @brief Send a OOC packet (CT) out to the server.
 * @param ooc_name The username.
 * @param ooc_message The message.
 */
void Courtroom::send_ooc_packet(QString ooc_message)
{
  while (ao_config->username().isEmpty())
  {
    ao_config->set_username(QInputDialog::getText(this, "Enter a name", "You must have a username to talk in OOC chat.",
                                                  QLineEdit::Normal, nullptr));
  }
  if (ooc_message.trimmed().isEmpty())
  {
    append_server_chatmessage("CLIENT", "You cannot send empty messages.");
    return;
  }
  QStringList l_content{ao_config->username(), ooc_message};
  ao_app->send_server_packet(DRPacket("CT", l_content));
}

void Courtroom::mod_called(QString p_ip)
{
  ui_ooc_chatlog->append(p_ip);
  if (ao_config->server_alerts_enabled())
  {
    m_system_player->play(ao_app->get_sfx("mod_call"));
    ao_app->alert(this);
    if (ao_config->log_is_recording_enabled())
      save_textlog("(OOC)(MOD CALL)" + p_ip);
  }
}

void Courtroom::on_ic_showname_editing_finished()
{
  const QString l_text = ui_ic_chat_showname->text().simplified();
  ui_ic_chat_showname->setText(l_text);
  set_showname(l_text);
}

void Courtroom::set_showname(QString p_showname)
{
  ao_config->set_showname(p_showname);
}

void Courtroom::on_ooc_name_editing_finished()
{
  const QString l_text = ui_ooc_chat_name->text().simplified();
  ui_ooc_chat_name->setText(l_text);
  ao_config->set_username(l_text);
}

void Courtroom::on_ooc_return_pressed()
{
  const QString l_message = ui_ooc_chat_message->text();

  if (l_message.startsWith("/rainbow") && !is_rainbow_enabled)
  {
    ui_text_color->addItem("Rainbow");
    ui_ooc_chat_message->clear();
    is_rainbow_enabled = true;
    return;
  }
  else if (l_message.startsWith("/switch_am"))
  {
    on_switch_area_music_clicked();
    ui_ooc_chat_message->clear();
    return;
  }
  else if (l_message.startsWith("/rollp"))
  {
    m_effects_player->play_effect(ao_app->get_sfx("dice"));
  }
  else if (l_message.startsWith("/roll"))
  {
    m_effects_player->play_effect(ao_app->get_sfx("dice"));
  }
  else if (l_message.startsWith("/coinflip"))
  {
    m_effects_player->play_effect(ao_app->get_sfx("coinflip"));
  }
  else if (l_message.startsWith("/tr "))
  {
    // Timer resume
    int space_location = l_message.indexOf(" ");

    int timer_id;
    if (space_location == -1)
      timer_id = 0;
    else
      timer_id = l_message.mid(space_location + 1).toInt();
    resume_timer(timer_id);
  }
  else if (l_message.startsWith("/ts "))
  {
    // Timer set
    QStringList arguments = l_message.split(" ");
    int size = arguments.size();

    // Note arguments[0] == "/ts", so every index (and thus length) is off by
    // one.
    if (size > 5)
      return;

    int timer_id = (size > 1 ? arguments[1].toInt() : 0);
    int new_time = (size > 2 ? arguments[2].toInt() : 300) * 1000;
    int timestep_length = (size > 3 ? arguments[3].toDouble() : -.016) * 1000;
    int firing_interval = (size > 4 ? arguments[4].toDouble() : .016) * 1000;
    set_timer_time(timer_id, new_time);
    set_timer_timestep(timer_id, timestep_length);
    set_timer_firing(timer_id, firing_interval);
  }
  else if (l_message.startsWith("/tp "))
  {
    // Timer pause
    int space_location = l_message.indexOf(" ");

    int timer_id;
    if (space_location == -1)
      timer_id = 0;
    else
      timer_id = l_message.mid(space_location + 1).toInt();
    pause_timer(timer_id);
  }

  send_ooc_packet(l_message);
  ui_ooc_chat_message->clear();

  ui_ooc_chat_message->setFocus();
}

void Courtroom::on_music_search_edited()
{
  list_music();
  list_areas();
}

void Courtroom::on_pos_dropdown_changed(int p_index)
{
  ui_ic_chat_message->setFocus();

  if (p_index < 0 || p_index > 5)
    return;

  QString f_pos;

  switch (p_index)
  {
  case 0:
    f_pos = "wit";
    break;
  case 1:
    f_pos = "def";
    break;
  case 2:
    f_pos = "pro";
    break;
  case 3:
    f_pos = "jud";
    break;
  case 4:
    f_pos = "hld";
    break;
  case 5:
    f_pos = "hlp";
    break;
  default:
    f_pos = "";
  }

  if (f_pos == "" || ao_config->username() == "")
    return;

  set_judge_enabled(f_pos == "jud");

  send_ooc_packet("/pos " + f_pos);
  // Uncomment later and remove above
  // Will only work in TSDR 4.3+ servers
  // ao_app->send_server_packet(DRPacket("SP", {f_pos}));
}

void Courtroom::on_music_list_clicked()
{
  ui_ic_chat_message->setFocus();
}

void Courtroom::on_area_list_clicked()
{
  ui_ic_chat_message->setFocus();
}

void Courtroom::on_music_list_double_clicked(QModelIndex p_model)
{
  if (is_client_muted)
    return;
  QString p_song = ui_music_list->item(p_model.row())->text();
  ao_app->send_server_packet(DRPacket("MC", {p_song, QString::number(m_chr_id)}));
  ui_ic_chat_message->setFocus();
}

void Courtroom::on_area_list_double_clicked(QModelIndex p_model)
{
  QString p_area = ui_area_list->item(p_model.row())->text();
  ao_app->send_server_packet(DRPacket("MC", {p_area, QString::number(m_chr_id)}));
  ui_ic_chat_message->setFocus();
}

/**
 * @brief Set the sprites of the shout buttons, and mark the currently
 * selected shout as such.
 *
 * @details If a sprite cannot be found for a shout button, a regular
 * push button is displayed for it with its shout name instead.
 */
void Courtroom::reset_shout_buttons()
{
  for (AOButton *i_button : qAsConst(ui_shouts))
    i_button->setChecked(false);
  m_shout_state = 0;
}

void Courtroom::on_shout_button_clicked(const bool p_checked)
{
  AOButton *l_button = dynamic_cast<AOButton *>(sender());
  if (l_button == nullptr)
    return;

  bool l_ok = false;
  const int l_id = l_button->property("shout_id").toInt(&l_ok);
  if (!l_ok)
    return;

  // disable all other buttons
  for (AOButton *i_button : qAsConst(ui_shouts))
  {
    if (i_button == l_button)
      continue;
    i_button->setChecked(false);
  }
  m_shout_state = p_checked ? l_id : 0;

  ui_ic_chat_message->setFocus();
}

void Courtroom::on_shout_button_toggled(const bool p_checked)
{
  AOButton *l_button = dynamic_cast<AOButton *>(sender());
  if (l_button == nullptr)
    return;

  const QString l_name = l_button->property("shout_name").toString();
  if (l_name.isEmpty())
    return;

  const QString l_image_name(QString("%1%2.png").arg(l_name, QString(p_checked ? "_selected" : nullptr)));
  l_button->set_image(l_image_name);
  if (!l_button->has_image())
    l_button->setText(l_name);
}

void Courtroom::on_cycle_clicked()
{
  AOButton *f_cycle_button = static_cast<AOButton *>(sender());
  int f_cycle_id = f_cycle_button->property("cycle_id").toInt();

  switch (f_cycle_id)
  {
  case 5:
    cycle_wtce(-1);
    break;
  case 4:
    cycle_wtce(1);
    break;
  case 3:
    cycle_effect(-1);
    break;
  case 2:
    cycle_effect(1);
    break;
  case 1:
    cycle_shout(-1);
    break;
  case 0:
    cycle_shout(1);
    break;
  default:
    break;
  }

  if (ao_app->read_theme_ini_bool("enable_cycle_ding", COURTROOM_CONFIG_INI))
    m_system_player->play(ao_app->get_sfx("cycle"));

  set_shouts();
  ui_ic_chat_message->setFocus();
}

/**
 * @brief Selects the shout p_delta slots ahead of the current one, wrapping
 * around if needed. If p_delta is negative, look -p_delta slots behind.
 * @param Shout slots to advance. May be negative.
 */
void Courtroom::cycle_shout(int p_delta)
{
  int n = ui_shouts.size();
  m_shout_current = (m_shout_current - p_delta + n) % n;
  set_shouts();
}

/**
 * @brief Selects the effect p_delta slots ahead of the current one, wrapping
 * around if needed. If p_delta is negative, look -p_delta slots behind.
 * @param Shout slots to advance. May be negative.
 */
void Courtroom::cycle_effect(int p_delta)
{
  int n = ui_effects.size();
  m_effect_current = (m_effect_current - p_delta + n) % n;
  set_effects();
}

/**
 * @brief Selects the splash p_delta slots ahead of the current one, wrapping
 * around if needed. If p_delta is negative, look -p_delta slots behind.
 * @param Shout slots to advance. May be negative.
 */
void Courtroom::cycle_wtce(int p_delta)
{
  int n = ui_wtce.size();
  m_wtce_current = (m_wtce_current - p_delta + n) % n;
  set_judge_wtce();
}

/**
 * @brief Set the sprites of the effect buttons, and mark the currently
 * selected effect as such.
 *
 * @details If a sprite cannot be found for an effect button, a regular
 * push button is displayed for it with its effect name instead.
 */
void Courtroom::reset_effect_buttons()
{
  for (AOButton *i_button : qAsConst(ui_effects))
    i_button->setChecked(false);
  m_effect_state = 0;
}

void Courtroom::on_effect_button_clicked(const bool p_checked)
{
  AOButton *l_button = dynamic_cast<AOButton *>(sender());
  if (l_button == nullptr)
    return;

  bool l_ok = false;
  const int l_id = l_button->property("effect_id").toInt(&l_ok);
  if (!l_ok)
    return;

  // disable all other buttons
  for (AOButton *i_button : qAsConst(ui_effects))
  {
    if (i_button == l_button)
      continue;
    i_button->setChecked(false);
  }

  m_effect_state = p_checked ? l_id : 0;
  ui_ic_chat_message->setFocus();
}

void Courtroom::on_effect_button_toggled(const bool p_checked)
{
  AOButton *l_button = dynamic_cast<AOButton *>(sender());
  if (l_button == nullptr)
    return;

  const QString l_name = l_button->property("effect_name").toString();
  if (l_name.isEmpty())
    return;

  const QString l_image_name(QString("%1%2.png").arg(l_name, QString(p_checked ? "_pressed" : nullptr)));
  l_button->set_image(l_image_name);
  if (!l_button->has_image())
    l_button->setText(l_name);
}

void Courtroom::on_defense_minus_clicked()
{
  int f_state = defense_bar_state - 1;

  if (f_state >= 0)
    ao_app->send_server_packet(DRPacket("HP", {QString::number(1), QString::number(f_state)}));
}

void Courtroom::on_defense_plus_clicked()
{
  int f_state = defense_bar_state + 1;

  if (f_state <= 10)
    ao_app->send_server_packet(DRPacket("HP", {QString::number(1), QString::number(f_state)}));
}

void Courtroom::on_prosecution_minus_clicked()
{
  int f_state = prosecution_bar_state - 1;

  if (f_state >= 0)
    ao_app->send_server_packet(DRPacket("HP", {QString::number(2), QString::number(f_state)}));
}

void Courtroom::on_prosecution_plus_clicked()
{
  int f_state = prosecution_bar_state + 1;

  if (f_state <= 10)
    ao_app->send_server_packet(DRPacket("HP", {QString::number(2), QString::number(f_state)}));
}

void Courtroom::on_text_color_changed(int p_color)
{
  m_text_color = p_color;
  ui_ic_chat_message->setFocus();
}

/**
 * @brief Set the sprites of the splash buttons.
 *
 * @details If a sprite cannot be found for a shout button, a regular
 * push button is displayed for it with its shout name instead.
 */
void Courtroom::reset_wtce_buttons()
{
  for (int i = 0; i < wtce_names.size(); ++i)
  {
    const QString l_name = wtce_names.at(i);
    const QString l_file = l_name + ".png";
    AOButton *l_button = ui_wtce.at(i);
    l_button->set_image(l_file);
    l_button->setText(!l_button->has_image() ? l_name : nullptr);
  }

  m_wtce_current = 0;

  // Unlike the other reset functions, the judge buttons are of immediate
  // action and thus are immediately unpressed after being pressed.
  // Therefore, we do not need to handle displaying a "_selected.png"
  // when appropriate, because there is no appropriate situation
}

void Courtroom::on_wtce_clicked()
{
  //  qDebug() << "AA: wtce clicked!";
  if (is_client_muted)
    return;

  AOButton *f_sig = static_cast<AOButton *>(sender());
  QString id = f_sig->property("wtce_id").toString();

  ao_app->send_server_packet(DRPacket("RT", {QString("testimony%1").arg(id)}));

  ui_ic_chat_message->setFocus();
}

void Courtroom::on_change_character_clicked()
{
  suppress_audio(true);

  set_char_select();

  ui_char_select_background->show();
  ui_spectator->show();
}

void Courtroom::on_app_reload_theme_requested()
{
  // If an objection is playing, delay reload theme order to be executed
  // after objection is done
  if (ui_vp_objection->state() == QMovie::MovieState::Running)
  {
    shout_delayed_reload_theme = true;
    return;
  }

  setup_courtroom();

  // to update status on the background
  set_background(current_background);
}

void Courtroom::on_back_to_lobby_clicked()
{
  // hide so we don't get the 'disconnected from server' prompt
  hide();

  ao_app->construct_lobby();
  ao_app->get_lobby()->list_servers();
  ao_app->get_lobby()->set_choose_a_server();
  ao_app->destruct_courtroom();
}

void Courtroom::on_spectator_clicked()
{
  ao_app->send_server_packet(DRPacket("CC", {QString::number(ao_app->get_client_id()), "-1", get_hdid()}));
}

void Courtroom::on_call_mod_clicked()
{
  QMessageBox::StandardButton reply;
  QString warning = "Are you sure you want to call a mod?\n"
                    "\n"
                    "Be prepared to explain what is happening and why they are needed when they answer.";
  reply = QMessageBox::warning(this, "Warning", warning, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (reply == QMessageBox::Yes)
  {
    ao_app->send_server_packet(DRPacket("ZZ"));
    qDebug() << "Called mod";
  }
  else
    qDebug() << "Did not call mod";

  ui_ic_chat_message->setFocus();
}

void Courtroom::on_switch_area_music_clicked()
{
  if (ui_area_list->isHidden())
  {
    ui_area_list->show();
    ui_music_list->hide();
  }
  else
  {
    ui_area_list->hide();
    ui_music_list->show();
  }
}

void Courtroom::on_pre_clicked()
{
  ui_ic_chat_message->setFocus();
}

void Courtroom::on_flip_clicked()
{
  ui_ic_chat_message->setFocus();
}

void Courtroom::on_hidden_clicked()
{
  ui_ic_chat_message->setFocus();
}

void Courtroom::on_evidence_button_clicked()
{
  if (ui_evidence->isHidden())
  {
    ui_evidence->show();
    ui_evidence_overlay->hide();
  }
  else
  {
    ui_evidence->hide();
  }
}

void Courtroom::on_config_panel_clicked()
{
  ao_app->toggle_config_panel();
  ui_ic_chat_message->setFocus();
}

void Courtroom::on_note_button_clicked()
{
  if (!is_note_shown)
  {
    load_note();
    ui_vp_notepad_image->show();
    ui_vp_notepad->show();
    ui_vp_notepad->setFocus();
    is_note_shown = true;
  }
  else
  {
    save_note();
    ui_vp_notepad_image->hide();
    ui_vp_notepad->hide();
    ui_ic_chat_message->setFocus();
    is_note_shown = false;
  }
}

void Courtroom::on_note_text_changed()
{
  ao_app->write_note(ui_vp_notepad->toPlainText(), current_file);
}

void Courtroom::ping_server()
{
  ao_app->send_server_packet(DRPacket("CH", {QString::number(m_chr_id)}));
}

void Courtroom::closeEvent(QCloseEvent *event)
{
  Q_EMIT closing();
  QMainWindow::closeEvent(event);
}

void Courtroom::on_set_notes_clicked()
{
  if (ui_note_scroll_area->isHidden())
    ui_note_scroll_area->show();
  else
    ui_note_scroll_area->hide();
}

void Courtroom::resume_timer(int p_id)
{
  if (p_id < 0 || p_id >= ui_timers.length())
    return;
  AOTimer *l_timer = ui_timers.at(p_id);
  l_timer->resume();
}

void Courtroom::set_timer_time(int p_id, int new_time)
{
  if (p_id < 0 || p_id >= ui_timers.length())
    return;
  AOTimer *l_timer = ui_timers.at(p_id);
  l_timer->set_time(QTime(0, 0).addMSecs(new_time));
}

void Courtroom::set_timer_timestep(int p_id, int timestep_length)
{
  if (p_id < 0 || p_id >= ui_timers.length())
    return;
  AOTimer *l_timer = ui_timers.at(p_id);
  l_timer->set_timestep_length(timestep_length);
}

void Courtroom::set_timer_firing(int p_id, int firing_interval)
{
  if (p_id < 0 || p_id >= ui_timers.length())
    return;
  AOTimer *l_timer = ui_timers.at(p_id);
  l_timer->set_firing_interval(firing_interval);
}

void Courtroom::pause_timer(int p_id)
{
  if (p_id < 0 || p_id >= ui_timers.length())
    return;
  AOTimer *l_timer = ui_timers.at(p_id);
  l_timer->pause();
}
