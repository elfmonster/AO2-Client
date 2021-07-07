#include "aoapplication.h"

#include "aoconfig.h"
#include "aoconfigpanel.h"
#include "courtroom.h"
#include "debug_functions.h"
#include "drdiscord.h"
#include "drpacket.h"
#include "drserversocket.h"
#include "lobby.h"

#include <QDir>
#include <QFontDatabase>
#include <QRegularExpression>

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
#include <QDesktopWidget>
#else
#include <QScreen>
#endif

const QString AOApplication::MASTER_NAME = "Master";
const QString AOApplication::MASTER_HOST = "master.aceattorneyonline.com";
const int AOApplication::MASTER_PORT = 27016;
const int AOApplication::MASTER_RECONNECT_DELAY = 5000;

AOApplication::AOApplication(int &argc, char **argv) : QApplication(argc, argv)
{
  ao_config = new AOConfig(this);
  ao_config_panel = new AOConfigPanel(this);
  dr_discord = new DRDiscord(this);
  m_master_socket = new DRServerSocket(this);
  m_server_socket = new DRServerSocket(this);

  connect(ao_config, SIGNAL(theme_changed(QString)), this, SLOT(handle_theme_modification()));
  connect(ao_config, SIGNAL(gamemode_changed(QString)), this, SLOT(handle_theme_modification()));
  connect(ao_config, SIGNAL(timeofday_changed(QString)), this, SLOT(handle_theme_modification()));
  connect(ao_config_panel, SIGNAL(reload_theme()), this, SLOT(handle_theme_modification()));
  ao_config_panel->hide();

  dr_discord->set_presence(ao_config->discord_presence());
  dr_discord->set_hide_server(ao_config->discord_hide_server());
  dr_discord->set_hide_character(ao_config->discord_hide_character());
  connect(ao_config, SIGNAL(discord_presence_changed(bool)), dr_discord, SLOT(set_presence(bool)));
  connect(ao_config, SIGNAL(discord_hide_server_changed(bool)), dr_discord, SLOT(set_hide_server(bool)));
  connect(ao_config, SIGNAL(discord_hide_character_changed(bool)), dr_discord, SLOT(set_hide_character(bool)));

  connect(m_master_socket, SIGNAL(connected_to_server()), this, SLOT(_p_send_master_handshake()));
  connect(m_master_socket, SIGNAL(socket_error(QString)), this, SLOT(_p_handle_master_error(QString)));
  connect(m_master_socket, SIGNAL(packet_received(DRPacket)), this, SLOT(_p_handle_master_packet(DRPacket)));

  connect(m_server_socket, SIGNAL(disconnected_from_server()), this, SLOT(_p_handle_server_disconnection()));
  connect(m_server_socket, SIGNAL(packet_received(DRPacket)), this, SLOT(_p_handle_server_packet(DRPacket)));

#ifndef QT_DEBUG
  connect_to_master();
#endif
}

AOApplication::~AOApplication()
{
  destruct_lobby();
  destruct_courtroom();
}

int AOApplication::get_client_id() const
{
  return m_client_id;
}

void AOApplication::set_client_id(int id)
{
  m_client_id = id;
}

Lobby *AOApplication::get_lobby() const
{
  return m_lobby;
}

void AOApplication::construct_lobby()
{
  if (is_lobby_constructed)
  {
    qDebug() << "W: lobby was attempted constructed when it already exists";
    return;
  }

  m_lobby = new Lobby(this);
  is_lobby_constructed = true;

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
  QRect screen_geometry = QApplication::desktop()->screenGeometry();
#else
  QScreen *screen = QApplication::screenAt(m_lobby->pos());
  if (screen == nullptr)
    return;
  QRect screen_geometry = screen->geometry();
#endif
  int x = (screen_geometry.width() - m_lobby->width()) / 2;
  int y = (screen_geometry.height() - m_lobby->height()) / 2;
  m_lobby->move(x, y);

  m_lobby->show();

  dr_discord->set_state(DRDiscord::State::Idle);
  dr_discord->clear_server_name();
}

void AOApplication::destruct_lobby()
{
  if (!is_lobby_constructed)
  {
    qDebug() << "W: lobby was attempted destructed when it did not exist";
    return;
  }

  delete m_lobby;
  is_lobby_constructed = false;
}

Courtroom *AOApplication::get_courtroom() const
{
  return m_courtroom;
}

void AOApplication::construct_courtroom()
{
  if (is_courtroom_constructed)
  {
    qDebug() << "W: courtroom was attempted constructed when it already exists";
    return;
  }

  m_courtroom = new Courtroom(this);
  connect(m_courtroom, SIGNAL(closing()), this, SLOT(on_courtroom_closing()));
  connect(m_courtroom, SIGNAL(destroyed()), this, SLOT(on_courtroom_destroyed()));
  is_courtroom_constructed = true;

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
  QRect screen_geometry = QApplication::desktop()->screenGeometry();
#else
  QScreen *screen = QApplication::screenAt(m_courtroom->pos());
  if (screen == nullptr)
    return;
  QRect screen_geometry = screen->geometry();
#endif
  int x = (screen_geometry.width() - m_courtroom->width()) / 2;
  int y = (screen_geometry.height() - m_courtroom->height()) / 2;
  m_courtroom->move(x, y);
}

void AOApplication::destruct_courtroom()
{
  // destruct courtroom
  if (is_courtroom_constructed)
  {
    delete m_courtroom;
    is_courtroom_constructed = false;
  }
  else
  {
    qDebug() << "W: courtroom was attempted destructed when it did not exist";
  }

  // gracefully close our connection to the current server
  m_server_socket->disconnect_from_server();
}

DRDiscord *AOApplication::get_discord() const
{
  return dr_discord;
}

bool AOApplication::has_message_acknowledgement_feature() const
{
  return feature_ackMS;
}

bool AOApplication::has_character_declaration_feature() const
{
  return feature_chrini;
}

bool AOApplication::has_showname_declaration_feature() const
{
  return feature_showname;
}

bool AOApplication::has_chat_speed_feature() const
{
  return feature_chat_speed;
}

void AOApplication::handle_theme_modification()
{
  load_fonts();

  Q_EMIT reload_theme();
}

void AOApplication::set_favorite_list()
{
  m_favorite_server_list = read_serverlist_txt();
}

QVector<server_type> &AOApplication::get_favorite_list()
{
  return m_favorite_server_list;
}

QString AOApplication::get_current_char()
{
  if (!is_courtroom_constructed)
    return nullptr;
  return m_courtroom->get_character_ini();
}

/**
 * @brief Check the path for various known exploits.
 *
 * In order:
 * - Directory traversal (most commonly: "../" jumps)
 * @param p_file The path to check.
 * @return A sanitized path. If any check fails, the path returned is an empty string. The sanitized path does not
 * necessarily exist.
 */
QString AOApplication::sanitize_path(QString p_file)
{
  if (!p_file.contains(".."))
    return p_file;

  QStringList list = p_file.split(QRegularExpression("[\\/]"));
  while (!list.isEmpty())
    if (list.takeFirst().contains(QRegularExpression("\\.{2,}")))
      return nullptr;

  return p_file;
}

void AOApplication::toggle_config_panel()
{
  ao_config_panel->setVisible(!ao_config_panel->isVisible());
  if (ao_config_panel->isVisible())
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    QRect screen_geometry = QApplication::desktop()->screenGeometry();
#else
    QScreen *screen = QApplication::screenAt(ao_config_panel->pos());
    if (screen == nullptr)
      return;
    QRect screen_geometry = screen->geometry();
#endif
    int x = (screen_geometry.width() - ao_config_panel->width()) / 2;
    int y = (screen_geometry.height() - ao_config_panel->height()) / 2;
    ao_config_panel->setFocus();
    ao_config_panel->raise();
    ao_config_panel->move(x, y);
  }
}

bool AOApplication::get_first_person_enabled()
{
  return ao_config->get_bool("first_person", false);
}

void AOApplication::add_favorite_server(int p_server)
{
  if (p_server < 0 || p_server >= m_server_list.size())
    return;

  server_type fav_server = m_server_list.at(p_server);

  QString str_port = QString::number(fav_server.port);

  QString server_line = fav_server.ip + ":" + str_port + ":" + fav_server.name;

  write_to_serverlist_txt(server_line);
}

QVector<server_type> &AOApplication::get_server_list()
{
  return m_server_list;
}

void AOApplication::load_fonts()
{
  QFontDatabase l_database;
  for (const QFileInfo &fileInfo : QDir(get_case_sensitive_path(get_base_path() + "fonts")).entryInfoList())
    l_database.addApplicationFont(fileInfo.absoluteFilePath());
}

void AOApplication::loading_cancelled()
{
  destruct_courtroom();

  m_lobby->hide_loading_overlay();
}

void AOApplication::on_courtroom_closing()
{
  ao_config_panel->hide();
}

void AOApplication::on_courtroom_destroyed()
{
  ao_config_panel->hide();
}
