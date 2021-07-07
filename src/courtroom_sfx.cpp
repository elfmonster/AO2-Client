#include "courtroom.h"

#include "aoapplication.h"
#include "commondefs.h"
#include "file_functions.h"

#include <QCheckBox>
#include <QColor>
#include <QLineEdit>
#include <QListWidget>

#include <optional>

std::optional<DRSfx> Courtroom::current_sfx()
{
  QListWidgetItem *l_item = ui_sfx_list->currentItem();
  if (l_item == nullptr)
    return std::nullopt;
  return m_sfx_list.at(l_item->data(Qt::UserRole).toInt());
}

QString Courtroom::current_sfx_file()
{
  const QString l_current_emote_file = get_current_emote().sound_file;
  const std::optional<DRSfx> l_optional_sfx = current_sfx();
  if (!l_optional_sfx.has_value())
    return l_current_emote_file;
  const QString l_file = l_optional_sfx.value().file;
  return l_file == m_sfx_default_file ? l_current_emote_file : l_file;
}

void Courtroom::load_sfx_list_theme()
{
  m_sfx_color_found = ao_app->get_color("found_song_color", COURTROOM_DESIGN_INI);
  m_sfx_color_missing = ao_app->get_color("missing_song_color", COURTROOM_DESIGN_INI);
  update_all_sfx_item_color();
}

void Courtroom::load_current_character_sfx_list()
{
  QSignalBlocker l_blocker(ui_sfx_list);
  ui_sfx_list->clear();

  // items
  m_sfx_list.clear();
  m_sfx_list.append(DRSfx("Default", m_sfx_default_file));
  m_sfx_list.append(DRSfx("Silence", nullptr));

  const QStringList l_sfx_list = ao_app->get_sfx_list();
  for (const QString &i_sfx_line : l_sfx_list)
  {
    const QStringList l_sfx_entry = i_sfx_line.split("=", DR::SkipEmptyParts);

    const QString l_name = l_sfx_entry.at(l_sfx_entry.size() - 1).trimmed();
    const QString l_file = QString(l_sfx_entry.size() >= 2 ? l_sfx_entry.at(0) : nullptr).trimmed();
    const bool l_is_found = !ao_app->find_asset_path({ao_app->get_sounds_path(l_file)}, audio_extensions()).isEmpty();
    m_sfx_list.append(DRSfx(l_name, l_file, l_is_found));
  }

  // create items
  QList<QListWidgetItem *> l_item_list;
  for (int i = 0; i < m_sfx_list.length(); ++i)
  {
    const DRSfx &i_sfx = m_sfx_list.at(i);
    QListWidgetItem *l_item = new QListWidgetItem(ui_sfx_list);
    l_item_list.append(l_item);
    l_item->setText(i_sfx.name);
    l_item->setData(Qt::UserRole, i);
    set_sfx_item_color(l_item);
  }

  filter_sfx_list();
}

void Courtroom::filter_sfx_list()
{
  const QString l_filter = ui_sfx_search->text();
  for (int i = 0; i < ui_sfx_list->count(); ++i)
  {
    QListWidgetItem *i_item = ui_sfx_list->item(i);
    i_item->setHidden(!i_item->text().contains(l_filter, Qt::CaseInsensitive));
  }
}

void Courtroom::select_default_sfx()
{
  if (ui_sfx_list->count() == 0)
    return;
  ui_sfx_list->setCurrentRow(0);
}

void Courtroom::clear_sfx_selection()
{
  ui_sfx_list->setCurrentRow(-1);
}

void Courtroom::set_sfx_item_color(QListWidgetItem *p_item)
{
  if (p_item == nullptr)
    return;
  const bool l_is_found = m_sfx_list.at(p_item->data(Qt::UserRole).toInt()).is_found;
  QColor l_color = l_is_found ? m_sfx_color_found : m_sfx_color_missing;

  if (p_item == ui_sfx_list->currentItem())
  {
    // Calculate the amount of lightness it would take to light up the row. We
    // also limit it to 1.0, as giving lightness values above 1.0 to QColor does
    // nothing. +0.4 is just an arbitrarily chosen number.
    const double l_final_lightness = qMin(1.0, l_color.lightnessF() + 0.4);

    // This is just the reverse of the above, basically. We set the colour, and we
    // set the brush to have that colour.
    l_color.setHslF(l_color.hueF(), l_color.saturationF(), l_final_lightness);
  }

  p_item->setBackground(l_color);
}

void Courtroom::update_all_sfx_item_color()
{
  for (int i = 0; i < m_sfx_list.length(); ++i)
    set_sfx_item_color(ui_sfx_list->item(i));
}

void Courtroom::_p_sfxCurrentItemChanged(QListWidgetItem *p_current_item, QListWidgetItem *p_previous_item)
{
  set_sfx_item_color(p_current_item);
  set_sfx_item_color(p_previous_item);
  ui_pre->setChecked(ui_pre->isChecked() || current_sfx().has_value());
  ui_ic_chat_message->setFocus();
}
