#include "courtroom.h"

#include "aoapplication.h"
#include "aoconfig.h"
#include "commondefs.h"
#include "file_functions.h"

#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QPixmap>

int Courtroom::get_character_id()
{
  return m_chr_id;
}

void Courtroom::set_character_id(const int p_chr_id)
{
  if (m_chr_id == p_chr_id)
    return;
  m_chr_id = p_chr_id;
  Q_EMIT character_id_changed(m_chr_id);
}

QString Courtroom::get_character()
{
  return is_spectating() ? nullptr : m_chr_list.at(m_chr_id).name;
}

QString Courtroom::get_character_ini()
{
  return ao_config->character_ini(get_character());
}

namespace
{
void drSetItemIcon(QComboBox *p_widget, const int p_index, const QString &p_chr_name, AOApplication *ao_app)
{
  static const QIcon s_blank_icon = []() -> QIcon {
    QPixmap l_blank_texture(64, 64);
    l_blank_texture.fill(Qt::transparent);
    return QIcon(l_blank_texture);
  }();

  const QString l_icon_file = ao_app->get_character_path(p_chr_name, "char_icon.png");
  p_widget->setItemIcon(p_index, file_exists(l_icon_file) ? QIcon(l_icon_file) : s_blank_icon);
}
} // namespace

void Courtroom::update_iniswap_list()
{
  QSignalBlocker b_ini_list(ui_iniswap_dropdown);
  ui_iniswap_dropdown->clear();

  QStringList l_name_list{"Default"};
  const QString l_path = ao_app->get_base_path() + "/characters";
  const QFileInfoList l_info_list = QDir(l_path).entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
  for (const QFileInfo &i_info : l_info_list)
  {
    const QString l_name = i_info.fileName();
    if (!file_exists(ao_app->get_character_path(l_name, CHARACTER_CHAR_INI)))
      continue;
    l_name_list.append(l_name);
  }

  for (int i = 0; i < l_name_list.length(); ++i)
  {
    const QString &i_name = l_name_list.at(i);
    ui_iniswap_dropdown->addItem(i_name);
    if (i == 0)
      continue;
    drSetItemIcon(ui_iniswap_dropdown, i, i_name, ao_app);
  }
  update_default_iniswap_item();

  select_base_character_iniswap();
}

void Courtroom::update_default_iniswap_item()
{
  drSetItemIcon(ui_iniswap_dropdown, 0, get_character(), ao_app);
}

void Courtroom::select_base_character_iniswap()
{
  const QString l_current_chr = get_character_ini();
  if (get_character() == l_current_chr)
  {
    ui_iniswap_dropdown->setCurrentIndex(0);
    return;
  }
  ui_iniswap_dropdown->setCurrentText(l_current_chr);
}

void Courtroom::on_iniswap_dropdown_changed(int p_index)
{
  ao_config->set_character_ini(get_character(),
                               p_index == 0 ? get_character() : ui_iniswap_dropdown->itemText(p_index));
}
