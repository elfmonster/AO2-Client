#ifndef AOAPPLICATION_H
#define AOAPPLICATION_H

#include "datatypes.h"
#include "drpacket.h"

class AOConfig;
class AOConfigPanel;
class Courtroom;
class DRDiscord;
class DRServerSocket;
class Lobby;

#include <QApplication>
#include <QVector>

#include <optional>

class AOApplication : public QApplication
{
  Q_OBJECT

public:
  static const QString MASTER_NAME;
  static const QString MASTER_HOST;
  static const int MASTER_PORT;
  static const int MASTER_RECONNECT_DELAY;

  AOApplication(int &argc, char **argv);
  ~AOApplication();

  int get_client_id() const;
  void set_client_id(int id);

  void connect_to_master();
  void send_master_packet(DRPacket packet);
  void request_server_list();
  void connect_to_server(server_type server);
  void send_server_packet(DRPacket packet);

  Lobby *get_lobby() const;
  void construct_lobby();
  void destruct_lobby();

  Courtroom *get_courtroom() const;
  void construct_courtroom();
  void destruct_courtroom();

  DRDiscord *get_discord() const;

  bool has_message_acknowledgement_feature() const;
  bool has_character_declaration_feature() const;
  bool has_showname_declaration_feature() const;
  bool has_chat_speed_feature() const;

  ///////////////////////////////////////////

  void set_favorite_list();
  QVector<server_type> &get_favorite_list();
  void add_favorite_server(int p_server);

  QVector<server_type> &get_server_list();

  // Returns the character the player has currently selected
  QString get_current_char();

  // implementation in path_functions.cpp
  QString get_base_path();
  QString get_character_folder_path(QString character);
  QString get_character_path(QString p_character, QString p_file);
  // QString get_demothings_path();
  QString get_sounds_path(QString p_file);
  QString get_music_path(QString p_song);
  QString get_background_path(QString p_file);
  QString get_default_background_path(QString p_file);
  QString get_evidence_path(QString p_file);

  QString sanitize_path(QString p_file);

  QString find_asset_path(QStringList file_list, QStringList extension_list);
  QString find_asset_path(QStringList file_list);
  QString find_theme_asset_path(QString file, QStringList extension_list);
  QString find_theme_asset_path(QString file);

  QString get_case_sensitive_path(QString p_file);

  ////// Functions for accessing the config panel //////
  void toggle_config_panel();

  ////// Functions for reading and writing files //////
  // Implementations file_functions.cpp

  // Returns text from note file
  QString read_note(QString filename);

  // returns a list of call words
  QStringList get_callwords();

  // returns whatever the client should simulate first person dialog
  bool get_first_person_enabled();

  // TODO document what this does
  QStringList get_sfx_list();

  // Appends the argument string to serverlist.txt
  void write_to_serverlist_txt(QString p_line);

  // Writes to note file
  void write_note(QString p_text, QString filename);

  // appends to note file
  void append_note(QString p_line, QString filename);

  // Returns the contents of serverlist.txt
  QVector<server_type> read_serverlist_txt();

  QString read_ini(QString p_identifier, QString p_path);

  QString read_theme_ini(QString p_identifier, QString p_file);

  bool read_theme_ini_bool(QString p_identifier, QString p_file);

  int read_theme_ini_int(QString p_identifier, QString p_file);

  // Returns the coordinates of widget with p_identifier from p_file
  QPoint get_button_spacing(QString p_identifier, QString p_file);

  // Returns the dimensions of widget with specified identifier from p_file
  pos_size_type get_element_dimensions(QString p_identifier, QString p_file);

  // Returns the value of font property p_identifier from p_file
  int get_font_property(QString p_identifier, QString p_file);

  // Returns the name of the font with p_identifier from p_file
  QString get_font_name(QString p_identifier, QString p_file);

  // Returns the color with p_identifier from p_file
  std::optional<QColor> maybe_color(QString p_identifier, QString p_file);

  // Returns the color with p_identifier from p_file
  QColor get_color(QString p_identifier, QString p_file);

  // Returns the sfx with p_identifier from sounds.ini in the current theme path
  QString get_sfx(QString p_identifier);

  // Returns the value of p_search_line within target_tag and terminator_tag

  QVariant read_char_ini(QString character, QString group, QString key, QVariant default_value);
  QVariant read_char_ini(QString character, QString group, QString key);

  // Returns the text between target_tag and terminator_tag in p_file
  QString get_stylesheet(QString target_tag, QString p_file);

  // Returns string list (color name to color HEX)
  QMap<DR::Color, DR::ColorInfo> get_chatmessage_colors();

  // Returns string list (highlight character to color name)
  QVector<QStringList> get_highlight_colors();

  // Returns special button on cc_config according to index
  QString get_spbutton(QString p_tag, int index);

  // Returns effect on cc_config according to index
  QStringList get_effect(int index);

  // Returns wtce on cc_config according to index
  QStringList get_wtce(int index);

  // Returns the side of the p_char character from that characters ini file
  QString get_char_side(QString p_char);

  // Returns the showname from the ini of p_char
  QString get_showname(QString p_char);

  // Returns showname from showname.ini
  QString read_showname(QString p_char);

  // Returns the value of chat from the specific p_char's ini file
  QString get_chat(QString p_char);

  // Returns the name of p_char
  QString get_char_name(QString p_char);

  QStringList get_char_include(QString character);

  QStringList get_char_include_tree(QString character);

  // Returns p_char's gender
  QString get_gender(QString p_char);

  QVector<DREmote> get_emote_list(QString p_chr);

  // Returns x,y offset for effect p_effect
  QStringList get_effect_offset(QString p_char, int p_effect);

  // Returns overlay at p_effect in char_path/overlay
  QStringList get_overlay(QString p_char, int p_effect);

  ////// Functions for fonts handling //////
  void load_fonts();

public slots:
  void loading_cancelled();

signals:
  void reload_theme();

private:
  AOConfig *ao_config = nullptr;
  AOConfigPanel *ao_config_panel = nullptr;
  DRDiscord *dr_discord = nullptr;

  DRServerSocket *m_master_socket = nullptr;
  DRServerSocket *m_server_socket = nullptr;

  Lobby *m_lobby = nullptr;
  bool is_lobby_constructed = false;

  Courtroom *m_courtroom = nullptr;
  bool is_courtroom_constructed = false;
  bool is_courtroom_loaded = false;

  ///////////////server metadata////////////////
#ifdef DRO_ACKMS // TODO WARNING remove entire block on 1.0.0 release
  bool feature_ackMS = false;
#endif
  bool feature_showname = false;
  bool feature_chrini = false;
  bool feature_chat_speed = false;

  ///////////////loading info///////////////////
  // player number, it's hardly used but might be needed for some old servers
  int m_client_id = 0;

  QString m_server_software;

  int m_character_count = 0;
  int m_loaded_characters = 0;
  int m_evidence_count = 0;
  int m_loaded_evidence = 0;
  int m_music_count = 0;
  int m_loaded_music = 0;

  QVector<server_type> m_server_list;
  QVector<server_type> m_favorite_server_list;

private slots:
  void _p_send_master_handshake();
  void _p_handle_master_error(QString);
  void _p_handle_master_packet(DRPacket);
  void _p_handle_server_disconnection();
  void _p_handle_server_packet(DRPacket);
  void on_courtroom_closing();
  void on_courtroom_destroyed();
  void handle_theme_modification();
};

#endif // AOAPPLICATION_H
