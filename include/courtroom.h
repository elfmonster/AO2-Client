#ifndef COURTROOM_H
#define COURTROOM_H

#include "datatypes.h"

class AOApplication;
class AOBlipPlayer;
class AOButton;
class AOCharButton;
class AOCharMovie;
class AOConfig;
class AOEmoteButton;
class AOEvidenceButton;
class AOEvidenceDescription;
class AOEvidenceDisplay;
class AOImageDisplay;
class AOLabel;
class AOLineEdit;
class AOMovie;
class AOMusicPlayer;
class AONoteArea;
class AONotepad;
class AOScene;
class AOSfxPlayer;
class AOShoutPlayer;
class AOSystemPlayer;
class AOTimer;
class DRChatLog;
class DRTextEdit;

#include <QMainWindow>
#include <QMap>
#include <QModelIndex>
#include <QQueue>
#include <QStack>
#include <QTextCharFormat>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPropertyAnimation;
class QScrollArea;
class QSignalMapper;

#include <optional>

class Courtroom : public QMainWindow
{
  Q_OBJECT

public:
  static const int DEFAULT_WIDTH;
  static const int DEFAULT_HEIGHT;

  explicit Courtroom(AOApplication *p_ao_app);
  ~Courtroom();

  void set_character_list(QVector<char_type> character_list);
  void set_area_list(QStringList area_list);
  void set_music_list(QStringList music_list);

  // sets position of widgets based on theme ini files
  void set_widgets();
  // helper function that calls above function on the relevant widgets
  void set_fonts();

  // sets dropdown menu stylesheet
  void set_dropdown(QWidget *widget, QString target_tag);

  // helper funciton that call above function on the relevant widgets
  void set_dropdowns();

  void set_window_title(QString p_title);

  // sets status as taken on character with cid n_char and places proper shading
  // on charselect
  void set_taken(int n_char, bool p_taken);

  // sets the current background to argument. also does some checks to see if
  // it's a legacy bg
  void set_background(QString p_background);

  void set_tick_rate(const int tick_rate);

  // sets the evidence list member variable to argument
  void set_evidence_list(QVector<evi_type> &p_evi_list);

  // sets the character position
  void set_character_position(QString p_pos);

  void send_ooc_packet(QString ooc_message);

  void ignore_next_showname();
  void send_showname_packet(QString p_showname);

  // called when a DONE#% from the server was received
  void done_received();

  // sets desk and bg based on pos in chatmessage
  void set_scene();

  // sets text color based on text color in chatmessage
  void set_text_color();

  // disables chat if current cid matches second argument
  // enables if p_muted is false
  void set_muted(bool p_muted, int p_cid);

  // send a message that the player is banned and quits the server
  void set_ban(int p_cid);

  // implementations in path_functions.cpp
  QString get_background_path(QString p_file);

public:
  QString get_character();
  QString get_character_ini();
  void update_iniswap_list();
  void update_default_iniswap_item();
  void select_base_character_iniswap();

  // Set the showname of the client
  void set_showname(QString p_showname);

  // sets up widgets
  void setup_courtroom();

  // properly sets up some varibles: resets user state
  void enter_courtroom(int p_cid);

  // helper function that populates ui_music_list with the contents of
  // music_list
  void list_music();
  void list_areas();

  void list_note_files();
  void set_note_files();

  void move_widget(QWidget *p_widget, QString p_identifier);

  void set_shouts();

  void set_effects();

  void set_judge_wtce();

  void set_free_blocks();
  void set_judge_enabled(bool p_enabled);

  // these are for OOC chat
  void append_server_chatmessage(QString p_name, QString p_message);

  // handles resetting the UI after the server acknowledged the client sent an
  // message.
  void handle_acknowledged_ms();
  // these functions handle chatmessages sequentially.
  // The process itself is very convoluted and merits separate documentation
  // But the general idea is objection animation->pre animation->talking->idle
  void handle_chatmessage(QStringList p_contents);
  void handle_chatmessage_2();
  void handle_chatmessage_3();

  struct IcLogTextFormat
  {
    QTextCharFormat base;
    QTextCharFormat name;
    QTextCharFormat selfname;
    QTextCharFormat message;
    QTextCharFormat system;
  };
  IcLogTextFormat m_ic_log_format;
  void load_ic_text_format();

  // adds text to the IC chatlog. p_name first as bold then p_text then a newlin
  // this function keeps the chatlog scrolled to the top unless there's text
  // selected
  // or the user isn't already scrolled to the top
  void update_ic_log(bool p_reset_log);
  void append_ic_text(QString p_name, QString p_line, bool p_system, bool p_music, bool p_self);

  void append_system_text(QString p_showname, QString p_line);

  // prints who played the song to IC chat and plays said song(if found on local
  // filesystem) takes in a list where the first element is the song name and
  // the second is the char id of who played it
  void handle_song(QStringList p_contents);

  // animates music text
  void handle_music_anim();

  // handle server-side clock animation and display
  void handle_clock(QString time);

  void play_preanim();

  // plays a splash animation based on the argument
  void handle_wtce(QString p_wtce);

  // sets the hp bar of defense(p_bar 1) or pro(p_bar 2)
  // state is an number between 0 and 10 inclusive
  void set_hp_bar(int p_bar, int p_state);

  void check_connection_received();

  // checks whether shout/effect/wtce/free block files are found
  void check_shouts();
  void check_effects();
  void check_wtce();
  void check_free_blocks();

  void resume_timer(int timer_id);
  void set_timer_time(int timer_id, int new_time);
  void set_timer_timestep(int timer_id, int timestep_length);
  void set_timer_firing(int timer_id, int firing_interval);
  void pause_timer(int timer_id);

  template <typename T>
  int adapt_numbered_items(QVector<T *> &item_vector, QString config_item_number, QString item_name);

signals:
  void loaded_theme();
  void closing();

private:
  AOApplication *ao_app = nullptr;
  AOConfig *ao_config = nullptr;

  QTimer *m_reload_timer = nullptr;

  QVector<char_type> m_chr_list;
  QVector<evi_type> m_evidence_list;
  QStringList m_area_list;
  QStringList m_music_list;

  QSignalMapper *char_button_mapper = nullptr;

  // triggers ping_server() every 60 seconds
  QTimer *m_keepalive_timer = nullptr;

  // maintains a timer for how fast messages tick onto screen
  QTimer *m_tick_timer = nullptr;
  std::optional<int> m_server_tick_rate;
  int m_tick_speed = 0;
  // which tick position(character in chat message) we are at
  int m_tick_step = 0;
  bool is_ignore_next_letter = false;
  // used to determine how often blips sound
  int m_blip_step = 0;
  int m_rainbow_step = 0;
  bool is_first_showname_sent = false;
  bool is_next_showname_ignored = false;
  bool is_rainbow_enabled = false;
  bool is_note_shown = false;
  bool contains_add_button = false;

  //////////////
  QScrollArea *ui_note_scroll_area = nullptr;

  // delay before sfx plays
  QTimer *m_sound_timer = nullptr;

  // keeps track of how long realization is visible(it's just a white square and
  // should be visible less than a second)
  QTimer *m_flash_timer = nullptr;

  // Generate a File Name based on the time you launched the client
  QString icchatlogsfilename = QDateTime::currentDateTime().toString("'logs/'ddd MMMM dd yyyy hh.mm.ss.z'.txt'");

  static const int MESSAGE_SIZE = 16;
  QString m_chatmessage[MESSAGE_SIZE];
  bool chatmessage_is_empty = false;

  QString previous_ic_message;

  QColor m_message_color;
  QString m_message_color_name;
  QStack<QString> m_message_color_stack;

  bool is_client_muted = false;

  // state of animation, 0 = objecting, 1 = preanim, 2 = talking, 3 = idle
  int anim_state = 3;

  // state of text ticking, 0 = not yet ticking, 1 = ticking in progress, 2 =
  // ticking done
  int text_state = 2;

  // if enabled, disable showing our own sprites when we talk in ic
  bool m_msg_is_first_person = false;

  // Cached values for chat_tick
  bool m_chatbox_message_outline = false;
  bool m_chatbox_message_enable_highlighting = false;
  QVector<QStringList> m_chatbox_message_highlight_colors;

  QString current_file;

  // if true, a reload theme order was delayed to be executed *after* a shout
  // this allows reload theme orders that were received while a shout was
  // playing to be executed only after the shout is done playing
  bool shout_delayed_reload_theme = false;

  int m_shout_state = 0;
  int m_effect_state = 0;
  int m_text_color = 0;
  int m_shout_current = 0;
  int m_effect_current = 0;
  int m_wtce_current = 0;
  bool is_presenting_evidence = false;
  bool is_judge = false;
  bool is_system_speaking = false;

  int defense_bar_state = 0;
  int prosecution_bar_state = 0;

  int m_current_chr_page = 0;
  int char_columns = 10;
  int char_rows = 9;
  int m_page_max_chr_count = 90;

  QVector<DREmote> m_emote_list;
  int m_emote_id = 0;
  int m_current_emote_page = 0;
  int emote_columns = 5;
  int emote_rows = 2;
  int m_page_max_emote_count = 10;
  int m_emote_preview_id = -1;

  //  inmchatlog_changed;

  QVector<evi_type> local_evidence_list;

  int current_evidence_page = 0;
  int current_evidence = 0;
  int evidence_columns = 6;
  int evidence_rows = 3;
  int max_evidence_on_page = 18;

  int current_clock = -1;

  QString current_background = "gs4";

  AOImageDisplay *ui_background = nullptr;

  QWidget *ui_viewport = nullptr;
  AOScene *ui_vp_background = nullptr;
  AOCharMovie *ui_vp_player_char = nullptr;
  AOScene *ui_vp_desk = nullptr;
  AOEvidenceDisplay *ui_vp_evidence_display = nullptr;

  AONoteArea *ui_note_area = nullptr;

  AOImageDisplay *ui_vp_notepad_image = nullptr;
  DRTextEdit *ui_vp_notepad = nullptr;

  AOImageDisplay *ui_vp_chatbox = nullptr;
  DRTextEdit *ui_vp_showname = nullptr;
  DRTextEdit *ui_vp_message = nullptr;
  AOMovie *ui_vp_effect = nullptr;
  AOMovie *ui_vp_wtce = nullptr;
  AOMovie *ui_vp_objection = nullptr;
  AOMovie *ui_vp_chat_arrow = nullptr;

  AOImageDisplay *ui_vp_music_display_a = nullptr;
  AOImageDisplay *ui_vp_music_display_b = nullptr;

  AOImageDisplay *ui_vp_showname_image = nullptr;

  DRTextEdit *ui_vp_music_name = nullptr;
  QPropertyAnimation *music_anim = nullptr;

  QWidget *ui_vp_music_area = nullptr;

  AOMovie *ui_vp_clock = nullptr;
  QVector<AOTimer *> ui_timers;

  DRTextEdit *ui_ic_chatlog = nullptr;
  QList<DRChatRecord> m_ic_record_list;
  QQueue<DRChatRecord> m_ic_record_queue;

  DRChatLog *ui_ooc_chatlog = nullptr;

  QListWidget *ui_area_list = nullptr;
  QListWidget *ui_music_list = nullptr;

  QListWidget *ui_sfx_list = nullptr;
  QVector<DRSfx> m_sfx_list;
  const QString m_sfx_default_file = "__DEFAULT__";
  QColor m_sfx_color_found;
  QColor m_sfx_color_missing;

  QLineEdit *ui_ic_chat_showname = nullptr;
  QLineEdit *ui_ic_chat_message = nullptr;

  QLineEdit *ui_ooc_chat_name = nullptr;
  QLineEdit *ui_ooc_chat_message = nullptr;

  QLineEdit *ui_music_search = nullptr;

  QLineEdit *ui_sfx_search = nullptr;

  QWidget *ui_emotes = nullptr;
  QVector<AOEmoteButton *> ui_emote_list;
  AOButton *ui_emote_left = nullptr;
  AOButton *ui_emote_right = nullptr;
  AOImageDisplay *ui_emote_preview = nullptr;
  AOCharMovie *ui_emote_preview_character = nullptr;

  QComboBox *ui_emote_dropdown = nullptr;
  QComboBox *ui_iniswap_dropdown = nullptr;
  QComboBox *ui_pos_dropdown = nullptr;

  AOImageDisplay *ui_defense_bar = nullptr;
  AOImageDisplay *ui_prosecution_bar = nullptr;

  // buttons to cycle through shouts
  AOButton *ui_shout_up = nullptr;
  AOButton *ui_shout_down = nullptr;
  // buttons to cycle through effects
  AOButton *ui_effect_up = nullptr;
  AOButton *ui_effect_down = nullptr;
  // buttons to cycle through wtce
  AOButton *ui_wtce_up = nullptr;
  AOButton *ui_wtce_down = nullptr;

  // holds all the shout button objects
  QVector<AOButton *> ui_shouts;
  // holds all the effect button objects
  QVector<AOButton *> ui_effects;
  // holds all the shout buttons objects
  QVector<AOButton *> ui_wtce;
  // holds all the free block objects
  QVector<AOMovie *> ui_free_blocks;

  // holds all the names for sound files for the shouts
  QVector<QString> shout_names;

  // holds all the names for sound/anim files for the effects
  QVector<QString> effect_names;

  // holds all the names for sound/anim files for the shouts
  QVector<QString> wtce_names;

  // holds all the names for free blocks
  QVector<QString> free_block_names;

  // holds whether the animation file exists for a determined shout/effect
  QVector<bool> shouts_enabled;
  QVector<bool> effects_enabled;
  QVector<bool> wtce_enabled;
  QVector<bool> free_blocks_enabled;

  AOButton *ui_change_character = nullptr;
  AOButton *ui_call_mod = nullptr;
  AOButton *ui_switch_area_music = nullptr;

  AOButton *ui_config_panel = nullptr;

  AOButton *ui_set_notes = nullptr;

  QCheckBox *ui_pre = nullptr;
  QCheckBox *ui_flip = nullptr;
  QCheckBox *ui_hidden = nullptr;

  QVector<QCheckBox *> ui_checks; // 0 = pre, 1 = flip, 2 = hidden
  QVector<AOLabel *> ui_labels;   // 0 = music, 1 = sfx, 2 = blip
  QVector<AOImageDisplay *> ui_label_images;
  QVector<QString> label_images = {"Pre", "Flip", "Hidden"};

  AOButton *ui_effect_flash = nullptr;
  AOButton *ui_effect_gloom = nullptr;

  AOButton *ui_defense_plus = nullptr;
  AOButton *ui_defense_minus = nullptr;

  AOButton *ui_prosecution_plus = nullptr;
  AOButton *ui_prosecution_minus = nullptr;

  QComboBox *ui_text_color = nullptr;

  AOImageDisplay *ui_muted = nullptr;

  AOButton *ui_note_button = nullptr;

  AOButton *ui_evidence_button = nullptr;
  AOImageDisplay *ui_evidence = nullptr;
  AOLineEdit *ui_evidence_name = nullptr;
  QWidget *ui_evidence_buttons = nullptr;
  QVector<AOEvidenceButton *> ui_evidence_list;
  AOButton *ui_evidence_left = nullptr;
  AOButton *ui_evidence_right = nullptr;
  AOButton *ui_evidence_present = nullptr;
  AOImageDisplay *ui_evidence_overlay = nullptr;
  AOButton *ui_evidence_delete = nullptr;
  AOLineEdit *ui_evidence_image_name = nullptr;
  AOButton *ui_evidence_image_button = nullptr;
  AOButton *ui_evidence_x = nullptr;
  AOEvidenceDescription *ui_evidence_description = nullptr;

  AOImageDisplay *ui_char_select_background = nullptr;

  // abstract widget to hold char buttons
  QWidget *ui_char_buttons = nullptr;
  AOImageDisplay *ui_char_button_selector = nullptr;
  QVector<AOCharButton *> ui_char_button_list;

  AOButton *ui_back_to_lobby = nullptr;

  AOButton *ui_chr_select_left = nullptr;
  AOButton *ui_chr_select_right = nullptr;

  AOButton *ui_spectator = nullptr;

  QHash<QString, QWidget *> widget_names;

  void create_widgets();
  void connect_widgets();
  void set_widget_names();
  void reset_widget_names();
  void insert_widget_name(QString p_widget_name, QWidget *p_widget);
  void insert_widget_names(QVector<QString> &p_widget_names, QVector<QWidget *> &p_widgets);
  template <typename T> void insert_widget_names(QVector<QString> &p_widget_names, QVector<T *> &p_widgets);
  void set_widget_layers();

  void construct_char_select();
  void reconstruct_char_select();
  void reset_char_select();
  void set_char_select();
  void set_char_select_page();

  void construct_emotes();
  void construct_emote_page_layout();
  void reset_emote_page();
  void refresh_emote_page(const bool scroll_to_current_emote = false);
  void fill_emote_dropdown();
  DREmote get_emote(const int id);
  DREmote get_current_emote();

  void construct_evidence();
  void set_evidence_page();

  void load_note();
  void save_note();
  void save_textlog(QString p_text);

  bool is_spectating();

public slots:
  void objection_done();
  void preanim_done();

  void realization_done();

  void mod_called(QString p_ip);

private slots:
  void setup_chat();
  void play_sfx();

  void start_chat_timer();
  void stop_chat_timer();
  void calculate_chat_tick_interval();
  void next_chat_letter();
  void post_chat();

  void on_showname_changed(QString);
  void on_showname_placeholder_changed(QString);
  void on_character_ini_changed();
  void on_ic_showname_editing_finished();
  void on_ic_message_return_pressed();
  void on_chat_config_changed();

  void on_ooc_name_editing_finished();
  void on_ooc_return_pressed();

  void on_music_search_edited();
  void on_music_list_clicked();
  void on_area_list_clicked();
  void on_music_list_double_clicked(QModelIndex p_model);
  void on_area_list_double_clicked(QModelIndex p_model);

  void select_emote(int p_id);

  void on_emote_clicked(int id);
  void show_emote_tooltip(int id, QPoint global_pos);
  void hide_emote_tooltip(int id);

  void on_emote_left_clicked();
  void on_emote_right_clicked();

  void on_emote_dropdown_changed(int p_index);
  void on_iniswap_dropdown_changed(int p_index);
  void on_pos_dropdown_changed(int p_index);

  void on_evidence_name_edited();
  void on_evidence_image_name_edited();
  void on_evidence_image_button_clicked();
  void on_evidence_clicked(int p_id);
  void on_evidence_double_clicked(int p_id);

  void on_evidence_hover(int p_id, bool p_state);

  void on_evidence_left_clicked();
  void on_evidence_right_clicked();
  void on_evidence_present_clicked();

  void on_cycle_clicked();

  void cycle_shout(int p_delta);
  void cycle_effect(int p_delta);
  void cycle_wtce(int p_delta);

  void on_add_button_clicked();
  void on_delete_button_clicked();

  void on_set_file_button_clicked();
  void on_file_selected();

  void delete_widget(QWidget *p_widget);
  void load_shouts();
  void load_effects();
  void load_wtce();
  void load_free_blocks();
  void reset_shout_buttons();

  void on_shout_button_clicked(const bool);
  void on_shout_button_toggled(const bool);

  void reset_effect_buttons();
  void on_effect_button_clicked(const bool);
  void on_effect_button_toggled(const bool);

  void on_defense_minus_clicked();
  void on_defense_plus_clicked();
  void on_prosecution_minus_clicked();
  void on_prosecution_plus_clicked();

  void on_text_color_changed(int p_color);

  void reset_wtce_buttons();
  void on_wtce_clicked();

  void on_change_character_clicked();
  void on_app_reload_theme_requested();
  void on_call_mod_clicked();

  void on_switch_area_music_clicked();

  void on_config_panel_clicked();
  void on_note_button_clicked();

  void on_set_notes_clicked();

  void on_note_text_changed();

  void on_pre_clicked();
  void on_flip_clicked();
  void on_hidden_clicked();

  void on_evidence_button_clicked();

  void on_evidence_delete_clicked();
  void on_evidence_x_clicked();

  void on_back_to_lobby_clicked();

  void on_char_select_left_clicked();
  void on_char_select_right_clicked();
  void update_character_icon(QString character);
  void char_clicked(int n_char);
  void char_mouse_entered(AOCharButton *p_caller);
  void char_mouse_left();

  void on_spectator_clicked();

  void ping_server();

  // character
  // ===========================================================================
public:
  using CharacterId = int;
  enum : CharacterId
  {
    SpectatorId = -1,
  };

  int get_character_id();
public slots:
  void set_character_id(const int);
signals:
  void character_id_changed(int);

private:
  // character id, which index of the char_list the player is
  CharacterId m_chr_id = SpectatorId;

  // sfx
public:
  std::optional<DRSfx> current_sfx();
  QString current_sfx_file();
  void load_current_character_sfx_list();
  void load_sfx_list_theme();
  void select_default_sfx();
  void clear_sfx_selection();
  void update_all_sfx_item_color();

public slots:
  void filter_sfx_list();

private:
  void set_sfx_item_color(QListWidgetItem *item);

private slots:
  void _p_sfxCurrentItemChanged(QListWidgetItem *current_item, QListWidgetItem *previous_item);

  /*!
   * =============================================================================
   * AUDIO SYSTEM
   */

public:
  bool is_audio_suppressed() const;

public slots:
  void suppress_audio(bool p_enabled);
  void stop_all_audio();

private:
  AOSfxPlayer *m_effects_player = nullptr;
  AOShoutPlayer *m_shouts_player = nullptr;
  AOSystemPlayer *m_system_player = nullptr;
  AOMusicPlayer *m_music_player = nullptr;
  AOBlipPlayer *m_blips_player = nullptr;
  bool is_audio_muted = false;

  // QWidget interface
protected:
  void closeEvent(QCloseEvent *event) override;
};

template <typename T> void Courtroom::insert_widget_names(QVector<QString> &p_widget_names, QVector<T *> &p_widgets)
{
  QVector<QWidget *> widgets;

  for (QWidget *widget : p_widgets)
    widgets.append(widget);

  insert_widget_names(p_widget_names, widgets);
}

#endif // COURTROOM_H
