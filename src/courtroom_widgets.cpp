#include "courtroom.h"

#include "aoapplication.h"
#include "aoblipplayer.h"
#include "aobutton.h"
#include "aocharmovie.h"
#include "aoconfig.h"
#include "aoevidencedescription.h"
#include "aoevidencedisplay.h"
#include "aoimagedisplay.h"
#include "aolabel.h"
#include "aolineedit.h"
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
#include "drchatlog.h"
#include "drtextedit.h"
#include "file_functions.h"
#include "theme.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QFile>
#include <QListWidget>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QSignalMapper>
#include <QTimer>
#include <QVBoxLayout>

void Courtroom::create_widgets()
{
  m_keepalive_timer = new QTimer(this);
  m_keepalive_timer->start(60000);

  m_tick_timer = new QTimer(this);
  m_tick_timer->setSingleShot(false);
  m_tick_timer->setTimerType(Qt::PreciseTimer); // Prevents drift

  m_sound_timer = new QTimer(this);
  m_sound_timer->setSingleShot(true);

  m_flash_timer = new QTimer(this);
  m_flash_timer->setSingleShot(true);

  char_button_mapper = new QSignalMapper(this);

  m_system_player = new AOSystemPlayer(ao_app, this);
  m_effects_player = new AOSfxPlayer(ao_app, this);
  m_shouts_player = new AOShoutPlayer(ao_app, this);
  m_music_player = new AOMusicPlayer(ao_app, this);
  m_blips_player = new AOBlipPlayer(ao_app, this);

  ui_background = new AOImageDisplay(this, ao_app);

  ui_viewport = new QWidget(this);
  ui_vp_background = new AOScene(ui_viewport, ao_app);
  ui_vp_player_char = new AOCharMovie(ui_viewport, ao_app);
  ui_vp_desk = new AOScene(ui_viewport, ao_app);

  ui_vp_music_display_a = new AOImageDisplay(this, ao_app);
  ui_vp_music_display_b = new AOImageDisplay(this, ao_app);
  ui_vp_music_area = new QWidget(ui_vp_music_display_a);
  ui_vp_music_name = new DRTextEdit(ui_vp_music_area);
  ui_vp_music_name->setText("DANGANRONPA ONLINE");
  ui_vp_music_name->setFrameStyle(QFrame::NoFrame);
  ui_vp_music_name->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui_vp_music_name->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui_vp_music_name->setReadOnly(true);
  music_anim = new QPropertyAnimation(ui_vp_music_name, "geometry", this);

  ui_vp_clock = new AOMovie(this, ao_app);
  ui_vp_clock->set_play_once(true);

  ui_vp_evidence_display = new AOEvidenceDisplay(this, ao_app);

  ui_vp_chatbox = new AOImageDisplay(this, ao_app);
  ui_vp_showname = new DRTextEdit(ui_vp_chatbox);
  ui_vp_showname->setFrameStyle(QFrame::NoFrame);
  ui_vp_showname->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui_vp_showname->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui_vp_showname->setReadOnly(true);
  ui_vp_message = new DRTextEdit(ui_vp_chatbox);
  ui_vp_message->setFrameStyle(QFrame::NoFrame);
  ui_vp_message->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui_vp_message->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui_vp_message->setReadOnly(true);

  ui_vp_showname_image = new AOImageDisplay(this, ao_app);

  ui_vp_effect = new AOMovie(this, ao_app);
  ui_vp_wtce = new AOMovie(this, ao_app);
  ui_vp_objection = new AOMovie(this, ao_app);

  ui_vp_chat_arrow = new AOMovie(this, ao_app);
  ui_vp_chat_arrow->set_play_once(false);

  ui_iniswap_dropdown = new QComboBox(this);

  ui_ic_chatlog = new DRTextEdit(this);
  ui_ic_chatlog->setReadOnly(true);
  ui_ic_chatlog->set_auto_align(false);

  ui_ooc_chatlog = new DRChatLog(this);
  ui_ooc_chatlog->setReadOnly(true);
  ui_ooc_chatlog->setOpenExternalLinks(true);

  ui_area_list = new QListWidget(this);
  ui_music_list = new QListWidget(this);
  ui_sfx_list = new QListWidget(this);

  ui_ic_chat_showname = new QLineEdit(this);
  ui_ic_chat_showname->setFrame(false);
  ui_ic_chat_showname->setPlaceholderText("Showname");
  ui_ic_chat_showname->setText(ao_config->showname());

  ui_ic_chat_message = new QLineEdit(this);
  ui_ic_chat_message->setFrame(false);

  ui_muted = new AOImageDisplay(ui_ic_chat_message, ao_app);
  ui_muted->hide();

  ui_ooc_chat_message = new QLineEdit(this);
  ui_ooc_chat_message->setFrame(false);

  ui_ooc_chat_name = new QLineEdit(this);
  ui_ooc_chat_name->setFrame(false);
  ui_ooc_chat_name->setPlaceholderText("Name");
  ui_ooc_chat_name->setText(ao_config->username());

  ui_music_search = new QLineEdit(this);
  ui_music_search->setFrame(false);

  ui_sfx_search = new QLineEdit(this);
  ui_sfx_search->setFrame(false);

  ui_note_area = new AONoteArea(this, ao_app);
  ui_note_area->add_button = new AOButton(ui_note_area, ao_app);
  ui_note_area->m_layout = new QVBoxLayout(ui_note_area);

  ui_note_scroll_area = new QScrollArea(this);
  ui_note_scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui_note_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui_note_scroll_area->setWidgetResizable(true);

  ui_set_notes = new AOButton(this, ao_app);

  construct_emotes();

  ui_defense_bar = new AOImageDisplay(this, ao_app);
  ui_prosecution_bar = new AOImageDisplay(this, ao_app);

  load_shouts(); // Readds from theme, deletes old shouts if needed and creates
                 // new ones

  ui_shout_up = new AOButton(this, ao_app);
  ui_shout_up->setProperty("cycle_id", 1);
  ui_shout_down = new AOButton(this, ao_app);
  ui_shout_down->setProperty("cycle_id", 0);

  load_effects();

  ui_effect_down = new AOButton(this, ao_app);
  ui_effect_down->setProperty("cycle_id", 2);
  ui_effect_up = new AOButton(this, ao_app);
  ui_effect_up->setProperty("cycle_id", 3);

  load_wtce();

  ui_wtce_up = new AOButton(this, ao_app);
  ui_wtce_up->setProperty("cycle_id", 5);
  ui_wtce_down = new AOButton(this, ao_app);
  ui_wtce_down->setProperty("cycle_id", 4);

  ui_change_character = new AOButton(this, ao_app);
  ui_call_mod = new AOButton(this, ao_app);
  ui_switch_area_music = new AOButton(this, ao_app);

  ui_config_panel = new AOButton(this, ao_app);
  ui_note_button = new AOButton(this, ao_app);

  ui_label_images.resize(label_images.size());
  for (int i = 0; i < ui_label_images.size(); ++i)
  {
    ui_label_images[i] = new AOImageDisplay(this, ao_app);
  }

  ui_pre = new QCheckBox(this);
  ui_pre->setText("Pre");
  ui_flip = new QCheckBox(this);
  ui_flip->setText("Flip");
  ui_flip->hide();
  ui_hidden = new QCheckBox(this);
  ui_hidden->setText("Hidden");

  // filling vectors with existing label/checkbox pointers
  ui_checks.push_back(ui_pre);
  ui_checks.push_back(ui_flip);
  ui_checks.push_back(ui_hidden);

  ui_defense_plus = new AOButton(this, ao_app);
  ui_defense_minus = new AOButton(this, ao_app);

  ui_prosecution_plus = new AOButton(this, ao_app);
  ui_prosecution_minus = new AOButton(this, ao_app);

  ui_text_color = new QComboBox(this);
  ui_text_color->addItem("White");
  ui_text_color->addItem("Green");
  ui_text_color->addItem("Red");
  ui_text_color->addItem("Orange");
  ui_text_color->addItem("Blue");
  ui_text_color->addItem("Yellow");
  ui_text_color->addItem("Purple");
  ui_text_color->addItem("Pink");

  ui_evidence_button = new AOButton(this, ao_app);

  ui_vp_notepad_image = new AOImageDisplay(this, ao_app);
  ui_vp_notepad = new DRTextEdit(this);
  ui_vp_notepad->setFrameStyle(QFrame::NoFrame);

  ui_timers.resize(1);
  ui_timers[0] = new AOTimer(this);

  construct_evidence();

  load_free_blocks(); // Done last so they are guaranteed to be at bottom

  construct_char_select();
}

void Courtroom::connect_widgets()
{
  connect(m_keepalive_timer, SIGNAL(timeout()), this, SLOT(ping_server()));

  connect(ao_app, SIGNAL(reload_theme()), this, SLOT(on_app_reload_theme_requested()));

  connect(ui_vp_objection, SIGNAL(done()), this, SLOT(objection_done()));
  connect(ui_vp_player_char, SIGNAL(done()), this, SLOT(preanim_done()));

  connect(m_sound_timer, SIGNAL(timeout()), this, SLOT(play_sfx()));

  connect(m_tick_timer, SIGNAL(timeout()), this, SLOT(next_chat_letter()));

  connect(m_flash_timer, SIGNAL(timeout()), this, SLOT(realization_done()));

  connect(ui_emote_left, SIGNAL(clicked()), this, SLOT(on_emote_left_clicked()));
  connect(ui_emote_right, SIGNAL(clicked()), this, SLOT(on_emote_right_clicked()));

  connect(ui_emote_dropdown, SIGNAL(activated(int)), this, SLOT(on_emote_dropdown_changed(int)));
  connect(ui_iniswap_dropdown, SIGNAL(activated(int)), this, SLOT(on_iniswap_dropdown_changed(int)));
  connect(ui_pos_dropdown, SIGNAL(activated(int)), this, SLOT(on_pos_dropdown_changed(int)));

  connect(ao_config, SIGNAL(showname_changed(QString)), this, SLOT(on_showname_changed(QString)));
  connect(ao_config, SIGNAL(showname_placeholder_changed(QString)), this,
          SLOT(on_showname_placeholder_changed(QString)));
  connect(ao_config, SIGNAL(character_ini_changed(QString)), this, SLOT(on_character_ini_changed()));
  connect(ui_ic_chat_showname, SIGNAL(editingFinished()), this, SLOT(on_ic_showname_editing_finished()));
  connect(ui_ic_chat_message, SIGNAL(returnPressed()), this, SLOT(on_ic_message_return_pressed()));

  connect(ao_config, SIGNAL(username_changed(QString)), ui_ooc_chat_name, SLOT(setText(QString)));
  connect(ui_ooc_chat_name, SIGNAL(editingFinished()), this, SLOT(on_ooc_name_editing_finished()));
  connect(ui_ooc_chat_message, SIGNAL(returnPressed()), this, SLOT(on_ooc_return_pressed()));

  connect(ui_music_list, SIGNAL(clicked(QModelIndex)), this, SLOT(on_music_list_clicked()));
  connect(ui_area_list, SIGNAL(clicked(QModelIndex)), this, SLOT(on_area_list_clicked()));
  connect(ui_music_list, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(on_music_list_double_clicked(QModelIndex)));
  connect(ui_area_list, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(on_area_list_double_clicked(QModelIndex)));

  // connect events for shout/effect/wtce buttons happen in load_shouts(),
  // load_effects(), load_wtce()
  connect(ui_shout_up, SIGNAL(clicked(bool)), this, SLOT(on_cycle_clicked()));
  connect(ui_shout_down, SIGNAL(clicked(bool)), this, SLOT(on_cycle_clicked()));

  connect(ui_effect_up, SIGNAL(clicked(bool)), this, SLOT(on_cycle_clicked()));
  connect(ui_effect_down, SIGNAL(clicked(bool)), this, SLOT(on_cycle_clicked()));

  connect(ui_wtce_up, SIGNAL(clicked(bool)), this, SLOT(on_cycle_clicked()));
  connect(ui_wtce_down, SIGNAL(clicked(bool)), this, SLOT(on_cycle_clicked()));

  connect(ui_defense_minus, SIGNAL(clicked()), this, SLOT(on_defense_minus_clicked()));
  connect(ui_defense_plus, SIGNAL(clicked()), this, SLOT(on_defense_plus_clicked()));
  connect(ui_prosecution_minus, SIGNAL(clicked()), this, SLOT(on_prosecution_minus_clicked()));
  connect(ui_prosecution_plus, SIGNAL(clicked()), this, SLOT(on_prosecution_plus_clicked()));

  connect(ui_text_color, SIGNAL(currentIndexChanged(int)), this, SLOT(on_text_color_changed(int)));

  connect(this, SIGNAL(loaded_theme()), this, SLOT(on_chat_config_changed()));
  connect(ao_config, SIGNAL(log_max_lines_changed(int)), this, SLOT(on_chat_config_changed()));
  connect(ao_config, SIGNAL(log_display_timestamp_changed(bool)), this, SLOT(on_chat_config_changed()));
  connect(ao_config, SIGNAL(log_display_self_highlight_changed(bool)), this, SLOT(on_chat_config_changed()));
  connect(ao_config, SIGNAL(log_format_use_newline_changed(bool)), this, SLOT(on_chat_config_changed()));
  connect(ao_config, SIGNAL(log_display_empty_messages_changed(bool)), this, SLOT(on_chat_config_changed()));
  connect(ao_config, SIGNAL(log_display_music_switch_changed(bool)), this, SLOT(on_chat_config_changed()));
  connect(ao_config, SIGNAL(log_is_topdown_changed(bool)), this, SLOT(on_chat_config_changed()));

  connect(ui_music_search, SIGNAL(textChanged(QString)), this, SLOT(on_music_search_edited()));
  connect(ui_sfx_search, SIGNAL(editingFinished()), this, SLOT(filter_sfx_list()));

  connect(ui_change_character, SIGNAL(clicked()), this, SLOT(on_change_character_clicked()));
  connect(ui_call_mod, SIGNAL(clicked()), this, SLOT(on_call_mod_clicked()));

  connect(ui_switch_area_music, SIGNAL(clicked()), this, SLOT(on_switch_area_music_clicked()));

  connect(ui_config_panel, SIGNAL(clicked()), this, SLOT(on_config_panel_clicked()));
  connect(ui_note_button, SIGNAL(clicked()), this, SLOT(on_note_button_clicked()));

  connect(ui_vp_notepad, SIGNAL(textChanged()), this, SLOT(on_note_text_changed()));

  connect(ui_pre, SIGNAL(clicked()), this, SLOT(on_pre_clicked()));
  connect(ui_flip, SIGNAL(clicked()), this, SLOT(on_flip_clicked()));
  connect(ui_hidden, SIGNAL(clicked()), this, SLOT(on_hidden_clicked()));

  connect(ui_sfx_list, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this,
          SLOT(_p_sfxCurrentItemChanged(QListWidgetItem *, QListWidgetItem *)));

  connect(ui_evidence_button, SIGNAL(clicked()), this, SLOT(on_evidence_button_clicked()));

  connect(ui_note_area->add_button, SIGNAL(clicked(bool)), this, SLOT(on_add_button_clicked()));
  connect(ui_set_notes, SIGNAL(clicked(bool)), this, SLOT(on_set_notes_clicked()));
}

void Courtroom::reset_widget_names()
{
  // Assign names to the default widgets
  widget_names = {
      {"courtroom", this},
      {"viewport", ui_viewport},
      {"background", ui_vp_background},   //*
      {"player_char", ui_vp_player_char}, //*
      {"desk", ui_vp_desk},               //*
      {"music_display_a", ui_vp_music_display_a},
      {"music_display_b", ui_vp_music_display_b},
      {"music_area", ui_vp_music_area},
      {"music_name", ui_vp_music_name},
      // music_anim
      {"clock", ui_vp_clock},
      // ui_vp_evidence_display
      {"ao2_chatbox", ui_vp_chatbox},
      {"showname", ui_vp_showname},
      {"message", ui_vp_message},
      {"showname_image", ui_vp_showname_image},
      {"vp_effect", ui_vp_effect},
      {"vp_wtce", ui_vp_wtce},
      {"vp_objection", ui_vp_objection},
      {"chat_arrow", ui_vp_chat_arrow},
      {"ic_chatlog", ui_ic_chatlog},
      {"server_chatlog", ui_ooc_chatlog},
      {"area_list", ui_area_list},
      {"music_list", ui_music_list},
      {"sfx_list", ui_sfx_list},
      {"ic_chat_name", ui_ic_chat_showname},
      {"ao2_ic_chat_message", ui_ic_chat_message},
      // ui_muted
      {"ooc_chat_message", ui_ooc_chat_message},
      {"ooc_chat_name", ui_ooc_chat_name},
      {"music_search", ui_music_search},
      {"sfx_search", ui_sfx_search},
      {"note_scroll_area", ui_note_scroll_area},
      {"note_area", ui_note_area},
      // add_button
      // m_layout
      {"set_notes_button", ui_set_notes},
      {"emotes", ui_emotes},
      {"emote_left", ui_emote_left},
      {"emote_right", ui_emote_right},
      {"emote_dropdown", ui_emote_dropdown},
      {"iniswap_dropdown", ui_iniswap_dropdown},
      {"pos_dropdown", ui_pos_dropdown},
      {"defense_bar", ui_defense_bar},
      {"prosecution_bar", ui_prosecution_bar},
      // Each ui_shouts[i]
      {"shout_up", ui_shout_up},
      {"shout_down", ui_shout_down},
      // Each ui_effects[i]
      {"effect_down", ui_effect_down},
      {"effect_up", ui_effect_up},
      // Each ui_wtce[i]
      {"wtce_up", ui_wtce_up},
      {"wtce_down", ui_wtce_down},
      {"change_character", ui_change_character},
      {"call_mod", ui_call_mod},
      {"switch_area_music", ui_switch_area_music},
      {"config_panel", ui_config_panel},
      {"note_button", ui_note_button},
      // Each ui_label_images[i]
      {"pre", ui_pre},
      {"flip", ui_flip},
      {"hidden", ui_hidden},
      {"defense_plus", ui_defense_plus},
      {"defense_minus", ui_defense_minus},
      {"prosecution_plus", ui_prosecution_plus},
      {"prosecution_minus", ui_prosecution_minus},
      {"text_color", ui_text_color},
      {"evidence_button", ui_evidence_button},
      {"notepad_image", ui_vp_notepad_image},
      {"notepad", ui_vp_notepad},
      // Each ui_timers[i]
      {"evidence_background", ui_evidence},
      {"evidence_buttons", ui_evidence_buttons},
      {"char_select", ui_char_select_background},
      {"back_to_lobby", ui_back_to_lobby},
      {"char_buttons", ui_char_buttons},
      {"char_select_left", ui_chr_select_left},
      {"char_select_right", ui_chr_select_right},
      {"spectator", ui_spectator},
  };
}

void Courtroom::insert_widget_name(QString p_widget_name, QWidget *p_widget)
{
  if (widget_names.contains(p_widget_name))
    qWarning() << QString("[WARNING] Widget <%1> is already defined").arg(p_widget_name);
  widget_names.insert(p_widget_name, p_widget);
  p_widget->setObjectName(p_widget_name);
}

void Courtroom::insert_widget_names(QVector<QString> &p_name_list, QVector<QWidget *> &p_widget_list)
{
  if (p_name_list.length() != p_widget_list.length())
    qFatal("[WARNING] Length of names and widgets differs!");
  for (int i = 0; i < p_widget_list.length(); ++i)
    insert_widget_name(p_name_list[i], p_widget_list[i]);
}

void Courtroom::set_widget_names()
{
  // Assign names to the default widgets
  reset_widget_names();

  // set existing widget names
  for (const QString &widget_name : widget_names.keys())
    widget_names[widget_name]->setObjectName(widget_name);

  // setup table of widgets and names
  insert_widget_names(effect_names, ui_effects);
  insert_widget_names(free_block_names, ui_free_blocks);
  insert_widget_names(shout_names, ui_shouts);
  insert_widget_names(wtce_names, ui_wtce);

  // timers are special children
  QVector<QString> timer_names;
  for (int i = 0; i < ui_timers.length(); ++i)
    timer_names.append("timer_" + QString::number(i));
  insert_widget_names(timer_names, ui_timers);
}

void Courtroom::set_widget_layers()
{
  // File lookup order
  // 1. In the theme folder (gamemode-timeofday/main/default), look for

  QString path = ao_app->find_theme_asset_path(COURTROOM_LAYERS_INI);
  QFile layer_ini(path);
  // needed to avoid cyclic parenting
  QStringList recorded_widgets;

  // Given the purpose of find_theme_asset_path, this step failing would be
  // weird
  if (layer_ini.open(QFile::ReadOnly))
  {
    QTextStream in(&layer_ini);

    // current parent's name
    QString parent_name = "courtroom";
    // the courtroom is ALWAYS going to be recorded
    recorded_widgets.append(parent_name);

    while (!in.atEnd())
    {
      QString line = in.readLine().trimmed();

      // skip if line is empty
      if (line.isEmpty())
        continue;

      // revert to default parent if we encounter an end scope
      if (line.startsWith("[\\"))
      {
        parent_name = "courtroom";
      }
      // is this a parent?
      else if (line.startsWith("["))
      {
        // update the current parent
        parent_name = line.remove(0, 1).chopped(1).trimmed();
      }
      // if this is not a parent, it's a child
      else
      {
        // if the child is already known, skip
        if (recorded_widgets.contains(line))
          continue;
        // make the child known
        recorded_widgets.append(line);

        // attach the children to the parents'
        QWidget *child = widget_names[line];
        // if child is null, then it does not exist
        if (!child)
          continue;

        QWidget *parent = widget_names[parent_name];
        // if parent is null, attach main parent
        if (!parent)
          parent = widget_names["courtroom"];

        // set child to parent
        bool was_visible = child->isVisible();
        child->setParent(parent);
        child->raise();

        // Readjust visibility in case this changed after the widget changed
        // parent I don't know why, I don't want to know why, I shouldn't have
        // to wonder why, but for whatever reason these stupid panels aren't
        // laying out correctly unless we do this terribleness
        if (child->isVisible() != was_visible)
          child->setVisible(was_visible);
      }
    }
  }

  // do special logic if config panel was not found in courtroom_layers. In
  // particular, make it visible and raise it in front of all assets. This can
  // help assist a theme designer who accidentally missed config_panel and would
  // have become unable to reload themes had they closed the config panel
  if (!recorded_widgets.contains("config_panel"))
  {
    ui_config_panel->setParent(this);
    ui_config_panel->raise();
    ui_config_panel->setVisible(true);
  }
}

void Courtroom::set_widgets()
{
  pos_size_type f_courtroom = ao_app->get_element_dimensions("courtroom", COURTROOM_DESIGN_INI);

  if (f_courtroom.width < 0 || f_courtroom.height < 0)
  {
    qDebug() << "W: did not find courtroom width or height in " << COURTROOM_DESIGN_INI;

    resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
  }
  else
  {
    resize(f_courtroom.width, f_courtroom.height);
  }

  ui_background->move(0, 0);
  ui_background->resize(size());
  ui_background->set_image("courtroombackground.png");

  set_size_and_pos(ui_viewport, "viewport", COURTROOM_DESIGN_INI, ao_app);

  ui_vp_background->move(0, 0);
  ui_vp_background->combo_resize(ui_viewport->size());

  ui_vp_player_char->move(0, 0);
  ui_vp_player_char->combo_resize(ui_viewport->size());

  // the AO2 desk element
  ui_vp_desk->move(0, 0);
  ui_vp_desk->combo_resize(ui_viewport->size());

  ui_vp_evidence_display->move(0, 0);
  ui_vp_evidence_display->resize(ui_viewport->width(), ui_viewport->height());

  set_size_and_pos(ui_vp_notepad_image, "notepad_image", COURTROOM_DESIGN_INI, ao_app);
  ui_vp_notepad_image->set_image("notepad_image.png");
  ui_vp_notepad_image->hide();

  set_size_and_pos(ui_vp_notepad, "notepad", COURTROOM_DESIGN_INI, ao_app);
  ui_vp_notepad->hide();

  set_size_and_pos(ui_vp_showname, "showname", COURTROOM_DESIGN_INI, ao_app);

  set_size_and_pos(ui_vp_showname_image, "showname_image", COURTROOM_DESIGN_INI, ao_app);
  ui_vp_showname_image->hide();

  set_size_and_pos(ui_vp_message, "message", COURTROOM_DESIGN_INI, ao_app);
  ui_vp_message->setTextInteractionFlags(Qt::NoTextInteraction);
  ui_vp_message->setStyleSheet("background-color: rgba(0, 0, 0, 0);"
                               "color: white");

  set_size_and_pos(ui_vp_chat_arrow, "chat_arrow", COURTROOM_DESIGN_INI, ao_app);
  if (!ao_app->find_theme_asset_path("chat_arrow", animated_or_static_extensions()).isEmpty())
    ui_vp_chat_arrow->play("chat_arrow");
  ui_vp_chat_arrow->hide();

  ui_vp_effect->move(ui_viewport->x(), ui_viewport->y());
  ui_vp_effect->resize(ui_viewport->width(), ui_viewport->height());
  ui_vp_effect->hide();

  ui_vp_wtce->move(ui_viewport->x(), ui_viewport->y());
  ui_vp_wtce->combo_resize(ui_viewport->width(), ui_viewport->height());

  ui_vp_objection->move(ui_viewport->x(), ui_viewport->y());
  ui_vp_objection->combo_resize(ui_viewport->width(), ui_viewport->height());

  set_size_and_pos(ui_ic_chatlog, "ic_chatlog", COURTROOM_DESIGN_INI, ao_app);

  set_size_and_pos(ui_ooc_chatlog, "server_chatlog", COURTROOM_DESIGN_INI, ao_app);

  set_size_and_pos(ui_music_list, "music_list", COURTROOM_DESIGN_INI, ao_app);
  set_size_and_pos(ui_area_list, "area_list", COURTROOM_DESIGN_INI, ao_app);
  if (ui_music_list->isVisible())
    ui_area_list->hide();
  //  ui_area_list->setStyleSheet("background-color: rgba(0, 0, 0, 0);");

  set_size_and_pos(ui_sfx_list, "sfx_list", COURTROOM_DESIGN_INI, ao_app);

  set_size_and_pos(ui_ic_chat_showname, "ic_chat_name", COURTROOM_DESIGN_INI, ao_app);
  set_text_alignment(ui_ic_chat_showname, "ic_chat_name", COURTROOM_FONTS_INI, ao_app);
  ui_ic_chat_showname->setStyleSheet("background-color: rgba(100, 100, 100, 255);");
  set_size_and_pos(ui_ic_chat_message, "ao2_ic_chat_message", COURTROOM_DESIGN_INI, ao_app);
  set_text_alignment(ui_ic_chat_message, "ao2_ic_chat_message", COURTROOM_FONTS_INI, ao_app);
  ui_ic_chat_message->setStyleSheet("QLineEdit{background-color: rgba(100, 100, 100, 255);}");

  set_size_and_pos(ui_vp_chatbox, "ao2_chatbox", COURTROOM_DESIGN_INI, ao_app);

  set_size_and_pos(ui_vp_music_area, "music_area", COURTROOM_DESIGN_INI, ao_app);
  ui_vp_music_area->show();
  set_size_and_pos(ui_vp_music_name, "music_name", COURTROOM_DESIGN_INI, ao_app);

  set_size_and_pos(ui_vp_music_display_a, "music_display_a", COURTROOM_DESIGN_INI, ao_app);
  ui_vp_music_display_a->set_image("music_display_a.png");
  ui_vp_music_display_a->show();

  set_size_and_pos(ui_vp_music_display_b, "music_display_b", COURTROOM_DESIGN_INI, ao_app);
  ui_vp_music_display_b->set_image("music_display_b.png");
  ui_vp_music_display_b->show();

  set_size_and_pos(ui_vp_clock, "clock", COURTROOM_DESIGN_INI, ao_app);
  ui_vp_clock->hide();

  ui_vp_chatbox->set_image("chatmed.png");
  ui_vp_chatbox->hide();

  ui_muted->resize(ui_ic_chat_message->width(), ui_ic_chat_message->height());
  ui_muted->set_image("muted.png");

  set_size_and_pos(ui_ooc_chat_message, "ooc_chat_message", COURTROOM_DESIGN_INI, ao_app);
  set_text_alignment(ui_ooc_chat_message, "ooc_chat_message", COURTROOM_FONTS_INI, ao_app);
  ui_ooc_chat_message->setStyleSheet("background-color: rgba(100, 100, 100, 255);");

  set_size_and_pos(ui_ooc_chat_name, "ooc_chat_name", COURTROOM_DESIGN_INI, ao_app);
  set_text_alignment(ui_ooc_chat_name, "ooc_chat_name", COURTROOM_FONTS_INI, ao_app);
  ui_ooc_chat_name->setStyleSheet("background-color: rgba(100, 100, 100, 255);");

  set_size_and_pos(ui_music_search, "music_search", COURTROOM_DESIGN_INI, ao_app);
  set_text_alignment(ui_music_search, "music_search", COURTROOM_FONTS_INI, ao_app);

  set_size_and_pos(ui_sfx_search, "sfx_search", COURTROOM_DESIGN_INI, ao_app);
  set_text_alignment(ui_sfx_search, "sfx_search", COURTROOM_FONTS_INI, ao_app);

  // char select
  reconstruct_char_select();

  // emotes
  set_size_and_pos(ui_emotes, "emotes", COURTROOM_DESIGN_INI, ao_app);
  construct_emote_page_layout();

  set_size_and_pos(ui_emote_left, "emote_left", COURTROOM_DESIGN_INI, ao_app);
  ui_emote_left->set_image("arrow_left.png");

  set_size_and_pos(ui_emote_right, "emote_right", COURTROOM_DESIGN_INI, ao_app);
  ui_emote_right->set_image("arrow_right.png");

  { // emote preview
    pos_size_type l_emote_preview_size = ao_app->get_element_dimensions("emote_preview", COURTROOM_DESIGN_INI);
    if (l_emote_preview_size.width <= 0 || l_emote_preview_size.height <= 0)
    {
      l_emote_preview_size.width = 320;
      l_emote_preview_size.height = 192;
    }
    ui_emote_preview->resize(l_emote_preview_size.width, l_emote_preview_size.height);
    ui_emote_preview->set_image("emote_preview.png");
    ui_emote_preview_character->resize(l_emote_preview_size.width, l_emote_preview_size.height);
  }

  set_size_and_pos(ui_emote_dropdown, "emote_dropdown", COURTROOM_DESIGN_INI, ao_app);

  set_size_and_pos(ui_iniswap_dropdown, "iniswap_dropdown", COURTROOM_DESIGN_INI, ao_app);

  set_size_and_pos(ui_pos_dropdown, "pos_dropdown", COURTROOM_DESIGN_INI, ao_app);

  set_size_and_pos(ui_defense_bar, "defense_bar", COURTROOM_DESIGN_INI, ao_app);
  ui_defense_bar->set_image("defensebar" + QString::number(defense_bar_state) + ".png");

  set_size_and_pos(ui_prosecution_bar, "prosecution_bar", COURTROOM_DESIGN_INI, ao_app);
  ui_prosecution_bar->set_image("prosecutionbar" + QString::number(prosecution_bar_state) + ".png");

  for (int i = 0; i < shout_names.size(); ++i)
  {
    set_size_and_pos(ui_shouts[i], shout_names[i], COURTROOM_DESIGN_INI, ao_app);
  }
  reset_shout_buttons();

  set_size_and_pos(ui_shout_up, "shout_up", COURTROOM_DESIGN_INI, ao_app);
  ui_shout_up->set_image("shoutup.png");
  ui_shout_up->hide();
  set_size_and_pos(ui_shout_down, "shout_down", COURTROOM_DESIGN_INI, ao_app);
  ui_shout_down->set_image("shoutdown.png");
  ui_shout_down->hide();

  // courtroom_config.ini necessary + check for crash
  if (ao_app->read_theme_ini_bool("enable_single_shout", COURTROOM_CONFIG_INI) && ui_shouts.size() > 0)
  {
    for (auto &shout : ui_shouts)
      move_widget(shout, "bullet");

    set_shouts();

    ui_shout_up->show();
    ui_shout_down->show();
  }

  for (int i = 0; i < effect_names.size(); ++i)
  {
    set_size_and_pos(ui_effects[i], effect_names[i], COURTROOM_DESIGN_INI, ao_app);
  }
  reset_effect_buttons();

  set_size_and_pos(ui_effect_up, "effect_up", COURTROOM_DESIGN_INI, ao_app);
  ui_effect_up->set_image("effectup.png");
  ui_effect_up->hide();
  set_size_and_pos(ui_effect_down, "effect_down", COURTROOM_DESIGN_INI, ao_app);
  ui_effect_down->set_image("effectdown.png");
  ui_effect_down->hide();

  if (ao_app->read_theme_ini_bool("enable_single_effect", COURTROOM_CONFIG_INI) &&
      ui_effects.size() > 0) // check to prevent crashing
  {
    for (auto &effect : ui_effects)
      move_widget(effect, "effect");

    set_effects();

    ui_effect_up->show();
    ui_effect_down->show();
  }

  set_size_and_pos(ui_wtce_up, "wtce_up", COURTROOM_DESIGN_INI, ao_app);
  ui_wtce_up->set_image("wtceup.png");
  ui_wtce_up->hide();
  set_size_and_pos(ui_wtce_down, "wtce_down", COURTROOM_DESIGN_INI, ao_app);
  ui_wtce_down->set_image("wtcedown.png");
  ui_wtce_down->hide();

  for (int i = 0; i < wtce_names.size(); ++i)
  {
    set_size_and_pos(ui_wtce[i], wtce_names[i], COURTROOM_DESIGN_INI, ao_app);
  }

  if (ao_app->read_theme_ini_bool("enable_single_wtce", COURTROOM_CONFIG_INI)) // courtroom_config.ini necessary
  {
    for (auto &wtce : ui_wtce)
      move_widget(wtce, "wtce");
    qDebug() << "AA: single wtce";
  }
  set_judge_wtce();
  reset_wtce_buttons();

  for (int i = 0; i < free_block_names.size(); ++i)
  {
    set_size_and_pos(ui_free_blocks[i], free_block_names[i], COURTROOM_DESIGN_INI, ao_app);
  }
  set_free_blocks();

  // Set the default values for the buttons, then determine if they should be
  // replaced by images
  set_size_and_pos(ui_change_character, "change_character", COURTROOM_DESIGN_INI, ao_app);
  set_size_and_pos(ui_call_mod, "call_mod", COURTROOM_DESIGN_INI, ao_app);
  set_size_and_pos(ui_note_button, "note_button", COURTROOM_DESIGN_INI, ao_app);
  set_size_and_pos(ui_switch_area_music, "switch_area_music", COURTROOM_DESIGN_INI, ao_app);
  set_size_and_pos(ui_config_panel, "config_panel", COURTROOM_DESIGN_INI, ao_app);

  ui_change_character->setText("");
  ui_call_mod->setText("");
  ui_switch_area_music->setText("");
  ui_config_panel->setText("");
  ui_note_button->setText("");

  ui_change_character->setStyleSheet("");
  ui_call_mod->setStyleSheet("");
  ui_switch_area_music->setStyleSheet("");
  ui_config_panel->setStyleSheet("");
  ui_note_button->setStyleSheet("");

  if (ao_app->read_theme_ini_bool("enable_button_images", COURTROOM_CONFIG_INI))
  {
    // Set files, ask questions later
    // set_image first tries the gamemode-timeofday folder, then the theme
    // folder, then falls back to the default theme
    ui_change_character->set_image("changecharacter.png");
    if (ui_change_character->get_image().isEmpty())
      ui_change_character->setText("Change Character");

    ui_call_mod->set_image("callmod.png");
    if (ui_call_mod->get_image().isEmpty())
      ui_call_mod->setText("Call Mod");

    ui_switch_area_music->set_image("switch_area_music.png");
    if (ui_switch_area_music->get_image().isEmpty())
      ui_switch_area_music->setText("A/M");

    ui_config_panel->set_image("config_panel.png");
    if (ui_config_panel->get_image().isEmpty())
      ui_config_panel->setText("Config");

    ui_note_button->set_image("notebutton.png");
    if (ui_note_button->get_image().isEmpty())
      ui_note_button->setText("Notes");
  }

  // The config panel has a special property. If it is displayed beyond the right or lower limit of the window, it will
  // be moved to 0, 0 A similar behavior will occur if the button is resized to 0, 0 due to 'config_panel' not being
  // found in courtroom_design.ini This is to assist with people who switch to incompatible and/or smaller themes and
  // have the button disappear
  if (ui_config_panel->x() > width() || ui_config_panel->y() > height() || ui_config_panel->width() <= 0 ||
      ui_config_panel->height() <= 0)
  {
    ui_config_panel->setVisible(true);
    ui_config_panel->move(0, 0);
    // Moreover, if the width or height is invalid, change it to some fixed
    // values
    if (ui_config_panel->width() <= 0 || ui_config_panel->height() <= 0)
      ui_config_panel->resize(64, 64);
  }

  set_size_and_pos(ui_pre, "pre", COURTROOM_DESIGN_INI, ao_app);
  ui_pre->setText("Pre");

  set_size_and_pos(ui_flip, "flip", COURTROOM_DESIGN_INI, ao_app);

  set_size_and_pos(ui_hidden, "hidden", COURTROOM_DESIGN_INI, ao_app);

  for (int i = 0; i < ui_label_images.size(); ++i)
  {
    set_size_and_pos(ui_label_images[i], label_images[i].toLower() + "_image", COURTROOM_DESIGN_INI, ao_app);
  }

  if (ao_app->read_theme_ini_bool("enable_label_images", COURTROOM_CONFIG_INI))
  {
    for (int i = 0; i < ui_checks.size(); ++i) // loop through checks
    {
      QString image = label_images[i].toLower() + ".png";
      ui_label_images[i]->set_image(image);

      if (!ui_label_images[i]->get_image().isEmpty())
        ui_checks[i]->setText("");
      else
        ui_checks[i]->setText(label_images[i]);
    }

    for (int i = 0; i < ui_labels.size(); ++i) // now through labels..........
    {
      int j = i + ui_checks.size();
      QString image = label_images[j].toLower() + ".png";
      ui_label_images[j]->set_image(image);

      if (!ui_label_images[j]->get_image().isEmpty())
        ui_labels[i]->setText("");
      else
        ui_labels[i]->setText(label_images[j]);
    }
  }
  else
  {
    for (int i = 0; i < ui_checks.size(); ++i) // same thing
    {
      ui_checks[i]->setText(label_images[i]);
      ui_label_images[i]->set_image("");
    }

    for (int i = 0; i < ui_labels.size(); ++i) // same thing
    {
      int j = i + ui_checks.size();
      ui_labels[i]->setText(label_images[j]);
      ui_label_images[j]->set_image("");
    }
  }

  set_size_and_pos(ui_defense_plus, "defense_plus", COURTROOM_DESIGN_INI, ao_app);
  ui_defense_plus->set_image("defplus.png");

  set_size_and_pos(ui_defense_minus, "defense_minus", COURTROOM_DESIGN_INI, ao_app);
  ui_defense_minus->set_image("defminus.png");

  set_size_and_pos(ui_prosecution_plus, "prosecution_plus", COURTROOM_DESIGN_INI, ao_app);
  ui_prosecution_plus->set_image("proplus.png");

  set_size_and_pos(ui_prosecution_minus, "prosecution_minus", COURTROOM_DESIGN_INI, ao_app);
  ui_prosecution_minus->set_image("prominus.png");

  set_size_and_pos(ui_text_color, "text_color", COURTROOM_DESIGN_INI, ao_app);

  set_size_and_pos(ui_evidence_button, "evidence_button", COURTROOM_DESIGN_INI, ao_app);
  ui_evidence_button->set_image("evidencebutton.png");

  set_size_and_pos(ui_evidence, "evidence_background", COURTROOM_DESIGN_INI, ao_app);
  ui_evidence->set_image("evidencebackground.png");

  set_size_and_pos(ui_evidence_name, "evidence_name", COURTROOM_DESIGN_INI, ao_app);

  set_size_and_pos(ui_evidence_buttons, "evidence_buttons", COURTROOM_DESIGN_INI, ao_app);

  set_size_and_pos(ui_evidence_left, "evidence_left", COURTROOM_DESIGN_INI, ao_app);
  ui_evidence_left->set_image("arrow_left.png");

  set_size_and_pos(ui_evidence_right, "evidence_right", COURTROOM_DESIGN_INI, ao_app);
  ui_evidence_right->set_image("arrow_right.png");

  set_size_and_pos(ui_evidence_present, "evidence_present", COURTROOM_DESIGN_INI, ao_app);
  ui_evidence_present->set_image("present_disabled.png");

  set_size_and_pos(ui_evidence_overlay, "evidence_overlay", COURTROOM_DESIGN_INI, ao_app);
  ui_evidence_overlay->set_image("evidenceoverlay.png");

  set_size_and_pos(ui_evidence_delete, "evidence_delete", COURTROOM_DESIGN_INI, ao_app);
  ui_evidence_delete->set_image("deleteevidence.png");

  set_size_and_pos(ui_evidence_image_name, "evidence_image_name", COURTROOM_DESIGN_INI, ao_app);

  set_size_and_pos(ui_evidence_image_button, "evidence_image_button", COURTROOM_DESIGN_INI, ao_app);

  set_size_and_pos(ui_evidence_x, "evidence_x", COURTROOM_DESIGN_INI, ao_app);
  ui_evidence_x->set_image("evidencex.png");

  set_size_and_pos(ui_evidence_description, "evidence_description", COURTROOM_DESIGN_INI, ao_app);

  ui_char_button_selector->set_image("char_selector.png");
  ui_char_button_selector->hide();

  set_size_and_pos(ui_back_to_lobby, "back_to_lobby", COURTROOM_DESIGN_INI, ao_app);
  ui_back_to_lobby->setText("Back to Lobby");

  set_size_and_pos(ui_char_buttons, "char_buttons", COURTROOM_DESIGN_INI, ao_app);

  set_size_and_pos(ui_chr_select_left, "char_select_left", COURTROOM_DESIGN_INI, ao_app);
  ui_chr_select_left->set_image("arrow_left.png");

  set_size_and_pos(ui_chr_select_right, "char_select_right", COURTROOM_DESIGN_INI, ao_app);
  ui_chr_select_right->set_image("arrow_right.png");

  set_size_and_pos(ui_spectator, "spectator", COURTROOM_DESIGN_INI, ao_app);

  handle_music_anim();

  set_size_and_pos(ui_set_notes, "set_notes_button", COURTROOM_DESIGN_INI, ao_app);
  ui_set_notes->set_image("set_notes.png");
  ui_note_area->m_layout->setSpacing(10);
  set_size_and_pos(ui_note_area, "note_area", COURTROOM_DESIGN_INI, ao_app);
  set_size_and_pos(ui_note_scroll_area, "note_area", COURTROOM_DESIGN_INI, ao_app);
  ui_note_scroll_area->setWidget(ui_note_area);

  ui_note_area->set_image("note_area.png");
  ui_note_area->add_button->set_image("add_button.png");
  ui_note_area->add_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  ui_note_area->setLayout(ui_note_area->m_layout);
  ui_note_area->show();
  ui_note_scroll_area->hide();

  list_note_files();

  if (!contains_add_button)
  {
    ui_note_area->m_layout->addWidget(ui_note_area->add_button);
    contains_add_button = true;
  }

  // This is used to force already existing notepicker elements to reset their image and theme setting
  for (AONotePicker *notepicker : ui_note_area->findChildren<AONotePicker *>())
  {
    for (AOButton *button : notepicker->findChildren<AOButton *>())
    {
      button->refresh_image();
    }
    QLineEdit *f_line = notepicker->findChild<QLineEdit *>();
    set_dropdown(f_line, "[LINE EDIT]");
  }

  adapt_numbered_items(ui_timers, "timer_number", "timer");
  set_dropdowns();
  set_fonts();

  Q_EMIT loaded_theme();
}

void Courtroom::move_widget(QWidget *p_widget, QString p_identifier)
{
  QString filename = COURTROOM_DESIGN_INI;

  pos_size_type design_ini_result = ao_app->get_element_dimensions(p_identifier, filename);

  if (design_ini_result.width < 0 || design_ini_result.height < 0)
  {
    qDebug() << "W: could not find \"" << p_identifier << "\" in " << filename;
    // Don't hide, as some widgets don't have a built-in way of reappearing again.
    p_widget->move(0, 0);
    p_widget->resize(0, 0);
  }
  else
  {
    p_widget->move(design_ini_result.x, design_ini_result.y);
  }
}

template <typename T>
int Courtroom::adapt_numbered_items(QVector<T *> &item_vector, QString config_item_number, QString item_name)
{
  // &item_vector must be a vector of size at least 1!

  // Redraw the new correct number of items.
  int new_item_number = ao_app->read_theme_ini_int(config_item_number, COURTROOM_CONFIG_INI);
  int current_item_number = item_vector.size();
  // Note we use the fact that, if config_item_number is not there,
  // read_theme_ini returns an empty string, which .toInt() would fail to
  // convert and by documentation transform to 0.

  // Decide what to do if the new theme has a different amount of items than the
  // old one
  if (new_item_number < current_item_number)
  {
    // Hide old items if there are any.
    for (int i = new_item_number; i < current_item_number; i++)
    {
      item_vector[i]->hide();
    }
  }
  else if (current_item_number < new_item_number)
  {
    // Create new items
    item_vector.resize(new_item_number);
    for (int i = current_item_number; i < new_item_number; i++)
    {
      item_vector[i] = new T(this);
      item_vector[i]->stackUnder(item_vector[i - 1]);
      // index i-1 exists as i >= current_item_number == item_vector.size() >= 1
    }
  }
  // Note that by design we do *not* destroy items (or similarly, the size of
  // item_vector is non-decreasing. This is because we want to allow for items
  // to, say, run in the background as invisible. With that said, we can now
  // properly format our new items
  for (int i = 0; i < new_item_number; i++)
  {
    item_vector[i]->show();
    set_size_and_pos(item_vector[i], item_name + "_" + QString::number(i), COURTROOM_DESIGN_INI, ao_app);
    // Note that show is deliberately placed before set_size_and_pos
    // That is because we want to retain the functionality set_size_and_pos
    // includes where hides a widget if it is unable to find a position for it,
    // and does not change its visibility otherwise.
  }
  return new_item_number;
}

void Courtroom::check_effects()
{
  // Asset lookup order
  // 1. In the character folder, look for
  // `effect_names.at(i)` + extensions in `exts` in order
  // 2. In the theme folder (gamemode-timeofday/main/default), look for
  // `effect_names.at(i)` + extensions in `exts` in order
  // Only enable buttons where a file was found

  for (int i = 0; i < ui_effects.size(); ++i)
  {
    QString path = ao_app->find_asset_path({ao_app->get_character_path(get_character_ini(), effect_names.at(i))},
                                           animated_extensions());
    if (path.isEmpty())
      path = ao_app->find_theme_asset_path(effect_names.at(i), animated_extensions());
    effects_enabled[i] = (!path.isEmpty());
  }
}

void Courtroom::check_free_blocks()
{
  // Asset lookup order
  // 1. In the character folder, look for
  // `free_block_names.at(i)` + extensions in `exts` in order
  // 2. In the theme folder (gamemode-timeofday/main/default), look for
  // `free_block_names.at(i)` + extensions in `exts` in order
  // Only enable buttons where a file was found

  for (int i = 0; i < ui_free_blocks.size(); ++i)
  {
    QString path = ao_app->find_asset_path({ao_app->get_character_path(get_character_ini(), free_block_names.at(i))},
                                           animated_extensions());
    if (path.isEmpty())
      path = ao_app->find_theme_asset_path(free_block_names.at(i), animated_extensions());
    free_blocks_enabled[i] = (!path.isEmpty());
  }
}

void Courtroom::check_shouts()
{
  // Asset lookup order
  // 1. In the character folder, look for
  // `shout_names.at(i)` + extensions in `exts` in order
  // 2. In the theme folder (gamemode-timeofday/main/default), look for
  // `shout_names.at(i)` + extensions in `exts` in order
  // Only enable buttons where a file was found

  for (int i = 0; i < ui_shouts.size(); ++i)
  {
    QString path = ao_app->find_asset_path({ao_app->get_character_path(get_character_ini(), shout_names.at(i))},
                                           animated_extensions());

    if (path.isEmpty())
      path = ao_app->find_theme_asset_path(shout_names.at(i), animated_extensions());

    shouts_enabled[i] = (!path.isEmpty());
  }
}

void Courtroom::check_wtce()
{
  // Asset lookup order
  // 1. In the character folder, look for
  // `wtce_names.at(i)` + extensions in `exts` in order
  // 2. In the theme folder (gamemode-timeofday/main/default), look for
  // `wtce_names.at(i)` + extensions in `exts` in order
  // Only enable buttons where a file was found

  for (int i = 0; i < ui_wtce.size(); ++i)
  {
    QString path = ao_app->find_asset_path({ao_app->get_character_path(get_character_ini(), wtce_names.at(i))},
                                           animated_extensions());
    if (path.isEmpty())
      path = ao_app->find_theme_asset_path(wtce_names.at(i), animated_extensions());
    wtce_enabled[i] = (!path.isEmpty());
  }
}

void Courtroom::delete_widget(QWidget *p_widget)
{
  // remove the widget from recorded names
  widget_names.remove(p_widget->objectName());

  // transfer the children to our grandparent since our parent is
  // about to commit suicide
  QWidget *grand_parent = p_widget->parentWidget();
  // if we don't have a grand parent, attach ourselves to courtroom
  if (!grand_parent)
    grand_parent = this;

  // set new parent
  for (QWidget *child : p_widget->findChildren<QWidget *>(nullptr, Qt::FindDirectChildrenOnly))
    child->setParent(grand_parent);

  // delete widget
  delete p_widget;
}

void Courtroom::load_effects()
{
  // Close any existing effects to prevent memory leaks
  for (QWidget *widget : qAsConst(ui_effects))
    delete_widget(widget);

  // And create new effects
  int effect_number = ao_app->read_theme_ini_int("effect_number", COURTROOM_CONFIG_INI);
  effects_enabled.resize(effect_number);
  ui_effects.resize(effect_number);

  for (int i = 0; i < ui_effects.size(); ++i)
  {
    AOButton *l_button = new AOButton(this, ao_app);
    ui_effects.replace(i, l_button);
    l_button->setCheckable(true);
    l_button->setProperty("effect_id", i + 1);
    l_button->stackUnder(ui_effect_up);
    l_button->stackUnder(ui_effect_down);

    connect(l_button, SIGNAL(clicked(bool)), this, SLOT(on_effect_button_clicked(bool)));
    connect(l_button, SIGNAL(toggled(bool)), this, SLOT(on_effect_button_toggled(bool)));
  }

  // And add names
  effect_names.clear();
  for (int i = 1; i <= ui_effects.size(); ++i)
  {
    QStringList names = ao_app->get_effect(i);
    if (!names.isEmpty())
    {
      const QString l_name = names.at(0).trimmed();
      effect_names.append(l_name);
      AOButton *l_button = ui_effects.at(i - 1);
      l_button->setProperty("effect_name", l_name);
      Q_EMIT l_button->toggled(l_button->isChecked());
    }
  }
}

void Courtroom::load_free_blocks()
{
  for (QWidget *widget : qAsConst(ui_free_blocks))
    delete_widget(widget);

  // And create new free block buttons
  int free_block_number = ao_app->read_theme_ini_int("free_block_number", COURTROOM_CONFIG_INI);
  free_blocks_enabled.resize(free_block_number);
  ui_free_blocks.resize(free_block_number);

  for (int i = 0; i < ui_free_blocks.size(); ++i)
  {
    ui_free_blocks[i] = new AOMovie(this, ao_app);
    // ui_free_blocks[i]->setProperty("free_block_id", i+1);
    ui_free_blocks[i]->set_play_once(false);
    ui_free_blocks[i]->stackUnder(ui_vp_player_char);
  }

  // And add names
  free_block_names.clear();
  for (int i = 0; i < ui_free_blocks.size(); ++i)
  {
    QString name = "free_block_" + ao_app->get_spbutton("[FREE BLOCKS]", i + 1).trimmed();
    if (!name.isEmpty())
    {
      free_block_names.append(name);
      widget_names[name] = ui_free_blocks[i];
      ui_free_blocks[i]->setObjectName(name);
    }
  }
}

void Courtroom::load_shouts()
{
  for (QWidget *widget : qAsConst(ui_shouts))
    delete_widget(widget);

  // And create new shouts
  int shout_number = ao_app->read_theme_ini_int("shout_number", COURTROOM_CONFIG_INI);
  shouts_enabled.resize(shout_number);
  ui_shouts.resize(shout_number);

  for (int i = 0; i < ui_shouts.size(); ++i)
  {
    AOButton *l_button = new AOButton(this, ao_app);
    ui_shouts.replace(i, l_button);
    l_button->setCheckable(true);
    l_button->setProperty("shout_id", i + 1);
    l_button->stackUnder(ui_shout_up);
    l_button->stackUnder(ui_shout_down);

    connect(l_button, SIGNAL(clicked(bool)), this, SLOT(on_shout_button_clicked(bool)));
    connect(l_button, SIGNAL(toggled(bool)), this, SLOT(on_shout_button_toggled(bool)));
  }

  // And add names
  shout_names.clear();
  for (int i = 1; i <= ui_shouts.size(); ++i)
  {
    QString name = ao_app->get_spbutton("[SHOUTS]", i).trimmed();
    if (!name.isEmpty())
    {
      shout_names.append(name);
      AOButton *l_button = ui_shouts.at(i - 1);
      widget_names.insert(name, l_button);
      l_button->setObjectName(name);
      l_button->setProperty("shout_name", name);
      Q_EMIT l_button->toggled(l_button->isChecked());
    }
  }
}

void Courtroom::load_wtce()
{
  for (QWidget *widget : qAsConst(ui_wtce))
    delete_widget(widget);

  // And create new wtce buttons
  const int l_wtce_count = ao_app->read_theme_ini_int("wtce_number", COURTROOM_CONFIG_INI);
  wtce_enabled.resize(l_wtce_count);

  ui_wtce.clear();
  for (int i = 0; i < l_wtce_count; ++i)
  {
    AOButton *l_button = new AOButton(this, ao_app);
    ui_wtce.append(l_button);
    l_button->setProperty("wtce_id", i + 1);
    l_button->stackUnder(ui_wtce_up);
    l_button->stackUnder(ui_wtce_down);
    connect(l_button, SIGNAL(clicked(bool)), this, SLOT(on_wtce_clicked()));
  }

  // And add names
  wtce_names.clear();
  for (int i = 1; i <= ui_wtce.size(); ++i)
  {
    QString name = ao_app->get_spbutton("[WTCE]", i).trimmed();
    if (!name.isEmpty())
    {
      wtce_names.append(name);
      widget_names[name] = ui_wtce[i - 1];
      ui_wtce[i - 1]->setObjectName(name);
    }
  }
}

/**
 * @brief Show the currently selected shout button, hide the remaining ones.
 * If no shouts exist, this method does nothing.
 */
void Courtroom::set_shouts()
{
  for (auto &shout : ui_shouts)
    shout->hide();
  if (ui_shouts.size() > 0)
    ui_shouts[m_shout_current]->show(); // check to prevent crashing
}

/**
 * @brief Show the currently selected effect button, hide the remaining ones.
 * If no effects exist, this method does nothing.
 */
void Courtroom::set_effects()
{
  for (auto &effect : ui_effects)
    effect->hide();

  // check to prevent crashing
  if (ui_effects.size() > 0)
    ui_effects[m_effect_current]->show();
}

void Courtroom::set_judge_enabled(bool p_enabled)
{
  is_judge = p_enabled;

  // set judge button visibility
  ui_defense_plus->setVisible(is_judge);
  ui_defense_minus->setVisible(is_judge);
  ui_prosecution_plus->setVisible(is_judge);
  ui_prosecution_minus->setVisible(is_judge);

  set_judge_wtce();
}

/**
 * @brief Show the currently selected splash button, hide the remaining ones.
 * If no splashes exist, this method does nothing.
 */
void Courtroom::set_judge_wtce()
{
  // hide all wtce before enabling visibility
  for (auto &wtce : ui_wtce)
    wtce->hide();

  // check if we use a single wtce or multiple
  const bool is_single_wtce = ao_app->read_theme_ini_bool("enable_single_wtce", COURTROOM_CONFIG_INI);

  // update visibility for next/previous
  ui_wtce_up->setVisible(is_judge && is_single_wtce);
  ui_wtce_down->setVisible(is_judge && is_single_wtce);

  // prevent going ahead if we have no wtce
  if (!is_judge || ui_wtce.length() == 0)
    return;

  // set visibility based off parameter
  if (is_single_wtce == true)
  {
    ui_wtce[m_wtce_current]->show();
  }
  else
  {
    for (AOButton *i_wtce : qAsConst(ui_wtce))
      i_wtce->show();
  }
}

/**
 * @brief Show all free blocks and restart their animations.
 */
void Courtroom::set_free_blocks()
{
  for (int i = 0; i < ui_free_blocks.size(); i++)
  {
    AOMovie *free_block = ui_free_blocks[i];
    free_block->play(free_block_names[i]);
  }
}

void Courtroom::set_dropdown(QWidget *widget, QString target_tag)
{
  QString f_file = "courtroom_stylesheets.css";
  QString style_sheet_string = ao_app->get_stylesheet(target_tag, f_file);
  if (style_sheet_string != "")
    widget->setStyleSheet(style_sheet_string);
}

void Courtroom::set_dropdowns()
{
  set_dropdown(ui_text_color, "[TEXT COLOR]");
  set_dropdown(ui_emote_dropdown, "[EMOTE DROPDOWN]");
  set_dropdown(ui_iniswap_dropdown, "[INISWAP DROPDOWN]");
  set_dropdown(ui_pos_dropdown, "[POS DROPDOWN]");
  set_dropdown(ui_ic_chat_message, "[IC LINE]");
  set_dropdown(ui_ooc_chat_message, "[OOC LINE]");
}

void Courtroom::set_fonts()
{
  set_drtextedit_font(ui_vp_showname, "showname", COURTROOM_FONTS_INI, ao_app);
  set_drtextedit_font(ui_vp_message, "message", COURTROOM_FONTS_INI, ao_app);
  set_drtextedit_font(ui_ic_chatlog, "ic_chatlog", COURTROOM_FONTS_INI, ao_app);

  // Chatlog does not support drtextedit because html
  set_font(ui_ooc_chatlog, "server_chatlog", COURTROOM_FONTS_INI, ao_app);
  set_font(ui_music_list, "music_list", COURTROOM_FONTS_INI, ao_app);
  set_font(ui_area_list, "area_list", COURTROOM_FONTS_INI, ao_app);
  set_font(ui_sfx_list, "sfx_list", COURTROOM_FONTS_INI, ao_app);
  set_drtextedit_font(ui_vp_music_name, "music_name", COURTROOM_FONTS_INI, ao_app);
  set_drtextedit_font(ui_vp_notepad, "notepad", COURTROOM_FONTS_INI, ao_app);

  for (int i = 0; i < ui_timers.length(); ++i)
  {
    AOTimer *i_timer = ui_timers.at(i);
    set_drtextedit_font(i_timer, QString("timer_%1").arg(i), COURTROOM_FONTS_INI, ao_app);
  }
}
