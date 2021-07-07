#include "lobby.h"

#include "aoapplication.h"
#include "aobutton.h"
#include "aoconfig.h"
#include "aoimagedisplay.h"
#include "commondefs.h"
#include "debug_functions.h"
#include "drchatlog.h"
#include "drpacket.h"
#include "drpather.h"
#include "drtextedit.h"
#include "theme.h"
#include "version.h"

#include <QDebug>
#include <QFontDatabase>
#include <QImageReader>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QProgressBar>

Lobby::Lobby(AOApplication *p_ao_app) : QMainWindow()
{
  ao_app = p_ao_app;
  ao_config = new AOConfig(this);

  this->setWindowTitle("Danganronpa Online");

  ui_background = new AOImageDisplay(this, ao_app);
  ui_public_servers = new AOButton(this, ao_app);
  ui_favorites = new AOButton(this, ao_app);
  ui_refresh = new AOButton(this, ao_app);
  ui_add_to_fav = new AOButton(this, ao_app);
  ui_connect = new AOButton(this, ao_app);
  ui_version = new DRTextEdit(this);
  ui_version->setFrameStyle(QFrame::NoFrame);
  ui_version->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui_version->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui_version->setReadOnly(true);
  ui_about = new AOButton(this, ao_app);
  ui_server_list = new QListWidget(this);
  ui_player_count = new DRTextEdit(this);
  ui_player_count->setFrameStyle(QFrame::NoFrame);
  ui_player_count->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui_player_count->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui_player_count->setWordWrapMode(QTextOption::NoWrap);
  ui_player_count->setReadOnly(true);
  ui_description = new QTextBrowser(this);
  ui_description->setOpenExternalLinks(true);
  ui_description->setReadOnly(true);
  ui_chatbox = new DRChatLog(this);
  ui_chatbox->setOpenExternalLinks(true);
  ui_chatbox->setReadOnly(true);
  ui_chatname = new QLineEdit(this);
  ui_chatname->setPlaceholderText("Name");
  ui_chatmessage = new QLineEdit(this);
  ui_loading_background = new AOImageDisplay(this, ao_app);
  ui_loading_text = new DRTextEdit(ui_loading_background);
  ui_progress_bar = new QProgressBar(ui_loading_background);
  ui_progress_bar->setMinimum(0);
  ui_progress_bar->setMaximum(100);
  ui_progress_bar->setStyleSheet("QProgressBar{ color: white; }");
  ui_cancel = new AOButton(ui_loading_background, ao_app);

  connect(ui_public_servers, SIGNAL(clicked()), this, SLOT(on_public_servers_clicked()));
  connect(ui_favorites, SIGNAL(clicked()), this, SLOT(on_favorites_clicked()));
  connect(ui_refresh, SIGNAL(pressed()), this, SLOT(on_refresh_pressed()));
  connect(ui_refresh, SIGNAL(released()), this, SLOT(on_refresh_released()));
  connect(ui_add_to_fav, SIGNAL(pressed()), this, SLOT(on_add_to_fav_pressed()));
  connect(ui_add_to_fav, SIGNAL(released()), this, SLOT(on_add_to_fav_released()));
  connect(ui_connect, SIGNAL(pressed()), this, SLOT(on_connect_pressed()));
  connect(ui_connect, SIGNAL(released()), this, SLOT(on_connect_released()));
  connect(ui_about, SIGNAL(clicked()), this, SLOT(on_about_clicked()));
  connect(ui_server_list, SIGNAL(clicked(QModelIndex)), this, SLOT(on_server_list_clicked(QModelIndex)));
  connect(ui_chatmessage, SIGNAL(returnPressed()), this, SLOT(on_chatfield_return_pressed()));
  connect(ui_cancel, SIGNAL(clicked()), ao_app, SLOT(loading_cancelled()));

  set_widgets();
}

bool Lobby::is_public_server() const
{
  return is_public_server_selected;
}

// sets images, position and size
void Lobby::set_widgets()
{
  pos_size_type f_lobby = ao_app->get_element_dimensions("lobby", LOBBY_DESIGN_INI);

  if (f_lobby.width < 0 || f_lobby.height < 0)
  {
    qDebug() << "W: did not find lobby width or height in " << LOBBY_DESIGN_INI;

    // Most common symptom of bad config files, missing assets, or misnamed
    // theme folder
    call_notice("It doesn't look like your client is set up correctly. This can be "
                "due to the following reasons: \n"
                "1. Check you downloaded and extracted the resources correctly from "
                "the DRO Discord including the large 'base' folder.\n"
                "2. If you did, check that the base folder is in the same folder "
                "where you launched Danganronpa Online from: " +
                DRPather::get_application_path() +
                "\n"
                "3. If it is there, check that your current theme folder exists in "
                "base/themes. According to base/config.ini, your current theme is " +
                ao_config->theme());

    this->resize(517, 666);
  }
  else
  {
    this->resize(f_lobby.width, f_lobby.height);
  }

  set_size_and_pos(ui_background, "lobby", LOBBY_DESIGN_INI, ao_app);
  ui_background->set_image("lobbybackground.png");

  set_size_and_pos(ui_public_servers, "public_servers", LOBBY_DESIGN_INI, ao_app);
  ui_public_servers->set_image("publicservers_selected.png");

  set_size_and_pos(ui_favorites, "favorites", LOBBY_DESIGN_INI, ao_app);
  ui_favorites->set_image("favorites.png");

  set_size_and_pos(ui_refresh, "refresh", LOBBY_DESIGN_INI, ao_app);
  ui_refresh->set_image("refresh.png");

  set_size_and_pos(ui_add_to_fav, "add_to_fav", LOBBY_DESIGN_INI, ao_app);
  ui_add_to_fav->set_image("addtofav.png");

  set_size_and_pos(ui_connect, "connect", LOBBY_DESIGN_INI, ao_app);
  ui_connect->set_image("connect.png");

  set_size_and_pos(ui_version, "version", LOBBY_DESIGN_INI, ao_app);
  ui_version->setText("Version: " + get_version_string());

  set_size_and_pos(ui_about, "about", LOBBY_DESIGN_INI, ao_app);
  ui_about->set_image("about.png");

  set_size_and_pos(ui_server_list, "server_list", LOBBY_DESIGN_INI, ao_app);
  ui_server_list->setStyleSheet("background-color: rgba(0, 0, 0, 0);"
                                "font: bold;");

  set_size_and_pos(ui_player_count, "player_count", LOBBY_DESIGN_INI, ao_app);
  ui_player_count->setStyleSheet("font: bold;"
                                 "color: white;"
                                 "qproperty-alignment: AlignCenter;");

  set_size_and_pos(ui_description, "description", LOBBY_DESIGN_INI, ao_app);
  ui_description->setStyleSheet("background-color: rgba(0, 0, 0, 0);"
                                "color: white;");

  set_size_and_pos(ui_chatbox, "chatbox", LOBBY_DESIGN_INI, ao_app);
  ui_chatbox->setReadOnly(true);
  ui_chatbox->setStyleSheet("QTextBrowser{background-color: rgba(0, 0, 0, 0);}");

  set_size_and_pos(ui_chatname, "chatname", LOBBY_DESIGN_INI, ao_app);
  set_text_alignment(ui_chatname, "chatname", LOBBY_FONTS_INI, ao_app);
  ui_chatname->setStyleSheet("background-color: rgba(0, 0, 0, 0);"
                             "selection-background-color: rgba(0, 0, 0, 0);");

  set_size_and_pos(ui_chatmessage, "chatmessage", LOBBY_DESIGN_INI, ao_app);
  set_text_alignment(ui_chatmessage, "chatmessage", LOBBY_FONTS_INI, ao_app);
  ui_chatmessage->setStyleSheet("background-color: rgba(0, 0, 0, 0);"
                                "selection-background-color: rgba(0, 0, 0, 0);");

  ui_loading_background->resize(this->width(), this->height());
  ui_loading_background->set_image("loadingbackground.png");

  set_size_and_pos(ui_loading_text, "loading_label", LOBBY_DESIGN_INI, ao_app);
  ui_loading_text->setFont(QFont("Arial", 20, QFont::Bold));
  ui_loading_text->setReadOnly(true);
  ui_loading_text->setAlignment(Qt::AlignCenter);
  ui_loading_text->setFrameStyle(QFrame::NoFrame);
  ui_loading_text->setStyleSheet("background-color: rgba(0, 0, 0, 0);"
                                 "color: rgba(255, 128, 0, 255);");
  ui_loading_text->append("Loading");

  set_size_and_pos(ui_progress_bar, "progress_bar", LOBBY_DESIGN_INI, ao_app);
  set_size_and_pos(ui_cancel, "cancel", LOBBY_DESIGN_INI, ao_app);
  ui_cancel->setText("Cancel");

  ui_loading_background->hide();

  set_fonts();
  set_stylesheets();
  set_choose_a_server();
}

void Lobby::set_fonts()
{
  set_drtextedit_font(ui_player_count, "player_count", LOBBY_FONTS_INI, ao_app);
  set_font(ui_description, "description", LOBBY_FONTS_INI, ao_app);
  set_font(ui_chatbox, "chatbox", LOBBY_FONTS_INI, ao_app);
  set_font(ui_chatname, "chatname", LOBBY_FONTS_INI, ao_app);
  set_font(ui_chatmessage, "chatmessage", LOBBY_FONTS_INI, ao_app);
  set_drtextedit_font(ui_loading_text, "loading_text", LOBBY_FONTS_INI, ao_app);
  set_font(ui_server_list, "server_list", LOBBY_FONTS_INI, ao_app);
}

void Lobby::set_stylesheet(QWidget *widget, QString target_tag)
{
  QString f_file = "lobby_stylesheets.css";
  QString style_sheet_string = ao_app->get_stylesheet(target_tag, f_file);
  if (style_sheet_string != "")
    widget->setStyleSheet(style_sheet_string);
}

void Lobby::set_stylesheets()
{
  set_stylesheet(ui_player_count, "[PLAYER COUNT]");
  set_stylesheet(ui_description, "[DESCRIPTION]");
  set_stylesheet(ui_chatbox, "[CHAT BOX]");
  set_stylesheet(ui_chatname, "[CHAT NAME]");
  set_stylesheet(ui_chatmessage, "[CHAT MESSAGE]");
  set_stylesheet(ui_loading_text, "[LOADING TEXT]");
  set_stylesheet(ui_server_list, "[SERVER LIST]");
}

void Lobby::show_loading_overlay()
{
  ui_loading_background->show();
}

void Lobby::hide_loading_overlay()
{
  ui_loading_background->hide();
}

void Lobby::set_loading_text(QString p_text)
{
  ui_loading_text->clear();
  ui_loading_text->setAlignment(Qt::AlignCenter);
  ui_loading_text->append(p_text);
}

int Lobby::get_selected_server()
{
  return ui_server_list->currentRow();
}

void Lobby::set_loading_value(int p_value)
{
  ui_progress_bar->setValue(p_value);
}

void Lobby::on_public_servers_clicked()
{
  ui_public_servers->set_image("publicservers_selected.png");
  ui_favorites->set_image("favorites.png");

  list_servers();

  is_public_server_selected = true;
}

void Lobby::on_favorites_clicked()
{
  ui_favorites->set_image("favorites_selected.png");
  ui_public_servers->set_image("publicservers.png");

  ao_app->set_favorite_list();
  // ao_app->favorite_list = read_serverlist_txt();

  list_favorites();

  is_public_server_selected = false;
}

void Lobby::on_refresh_pressed()
{
  ui_refresh->set_image("refresh_pressed.png");
}

void Lobby::on_refresh_released()
{
  ui_refresh->set_image("refresh.png");
  ao_app->request_server_list();
}

void Lobby::on_add_to_fav_pressed()
{
  ui_add_to_fav->set_image("addtofav_pressed.png");
}

void Lobby::on_add_to_fav_released()
{
  ui_add_to_fav->set_image("addtofav.png");

  // you cant add favorites from favorites m8
  if (!is_public_server_selected)
    return;

  ao_app->add_favorite_server(ui_server_list->currentRow());
}

void Lobby::on_connect_pressed()
{
  ui_connect->set_image("connect_pressed.png");
}

void Lobby::on_connect_released()
{
  ui_connect->set_image("connect.png");
  ao_app->send_server_packet(DRPacket("askchaa"));
}

void Lobby::on_about_clicked()
{
  const bool hasApng = QImageReader::supportedImageFormats().contains("apng");

  QString msg = tr("<h2>Danganronpa Online</h2>"
                   "version: %1"
                   "<p><b>Source code:</b> "
                   "<a href='https://github.com/Chrezm/DRO-Client'>"
                   "https://github.com/Chrezm/DRO-Client</a>"
                   "<p><b>Development:</b><br>"
                   "Cerapter, Elf, Iuvee, Tricky Leifa, Ziella"
                   "<p>Based on Attorney Online 2:<br>"
                   "<a href='https://github.com/AttorneyOnline/AO2-Client'>"
                   "https://github.com/AttorneyOnline/AO2-Client</a>"
                   "<p>Running on Qt version %2 with the BASS audio engine.<br>"
                   "APNG plugin loaded: %3"
                   "<p>Built on %4")
                    .arg(get_version_string())
                    .arg(QLatin1String(QT_VERSION_STR))
                    .arg(hasApng ? tr("Yes") : tr("No"))
                    .arg(QLatin1String(__DATE__));

  QMessageBox::about(this, tr("About"), msg);
}

void Lobby::on_server_list_clicked(QModelIndex p_model)
{
  int n_server = p_model.row();

  if (n_server < 0)
    return;

  if (is_public_server_selected)
  {
    QVector<server_type> f_server_list = ao_app->get_server_list();

    if (n_server >= f_server_list.size())
      return;

    m_last_server = f_server_list.at(p_model.row());
  }
  else
  {
    if (n_server >= ao_app->get_favorite_list().size())
      return;

    m_last_server = ao_app->get_favorite_list().at(p_model.row());
  }

  ui_player_count->setText("Connecting...");
  ui_description->setHtml("Connecting to " + m_last_server.name + "...");

  ao_app->connect_to_server(m_last_server);
}

void Lobby::on_chatfield_return_pressed()
{
  // no you can't send empty messages
  if (ui_chatname->text() == "" || ui_chatmessage->text() == "")
    return;

  QString f_header = "CT";
  QStringList f_contents{ui_chatname->text(), ui_chatmessage->text()};

  ao_app->send_master_packet(DRPacket(f_header, f_contents));

  ui_chatmessage->clear();
}

void Lobby::list_servers()
{
  is_public_server_selected = true;
  ui_favorites->set_image("favorites.png");
  ui_public_servers->set_image("publicservers_selected.png");

  ui_server_list->clear();

  for (const server_type &i_server : ao_app->get_server_list())
  {
    ui_server_list->addItem(i_server.name);
  }
}

void Lobby::list_favorites()
{
  ui_server_list->clear();

  for (const server_type &i_server : ao_app->get_favorite_list())
  {
    ui_server_list->addItem(i_server.name);
  }
}

void Lobby::append_chatmessage(QString f_name, QString f_message)
{
  ui_chatbox->append_chatmessage(f_name, f_message);
}

void Lobby::append_error(QString f_message)
{
  ui_chatbox->append_error(f_message);
}

void Lobby::set_choose_a_server()
{
  ui_player_count->setText(nullptr);
  ui_description->setHtml(tr("Choose a server."));
}

void Lobby::set_player_count(int players_online, int max_players)
{
  const QString f_string = "Connected: " + QString::number(players_online) + "/" + QString::number(max_players);
  ui_player_count->setText(f_string);
  ui_player_count->setAlignment(Qt::AlignHCenter);

  QString l_text = m_last_server.desc.toHtmlEscaped();
  const QRegExp l_regex("(https?://[^\\s/$.?#].[^\\s]*)");
  if (l_text.contains(l_regex))
    l_text.replace(l_regex, "<a href=\"\\1\">\\1</a>");
  ui_description->setHtml(l_text.replace("\n", "<br />"));
}
