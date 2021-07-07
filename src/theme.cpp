#include "theme.h"

// src
#include "aoapplication.h"
#include "datatypes.h"
#include "drtextedit.h"

// qt
#include <QDebug>
#include <QFontDatabase>

#include <string>

void set_size_and_pos(QWidget *p_widget, QString p_identifier, QString p_ini_file, AOApplication *ao_app)
{
  pos_size_type design_ini_result = ao_app->get_element_dimensions(p_identifier, p_ini_file);

  if (design_ini_result.width < 0 || design_ini_result.height < 0)
  {
    qDebug() << "W: could not find \"" << p_identifier << "\" in " << p_ini_file;
    // Don't hide, as some widgets don't have a built-in way of reappearing again.
    p_widget->move(0, 0);
    p_widget->resize(0, 0);
  }
  else
  {
    p_widget->move(design_ini_result.x, design_ini_result.y);
    p_widget->resize(design_ini_result.width, design_ini_result.height);
  }
}

void set_text_alignment_or_default(QWidget *p_widget, QString p_identifier, QString p_ini_file, AOApplication *ao_app,
                                   std::string p_property, Qt::Alignment p_default_horizontal,
                                   Qt::Alignment p_default_vertical)
{
  const QStringList l_values =
      ao_app->read_theme_ini(p_identifier + "_align", p_ini_file).split(",", DR::SkipEmptyParts);
  if (!p_widget->property(p_property.c_str()).isValid())
    return;
  p_widget->setProperty(p_property.c_str(),
                        QVariant(QHash<QString, Qt::Alignment>{
                                     {"left", Qt::AlignLeft}, {"center", Qt::AlignHCenter}, {"right", Qt::AlignRight}}
                                     .value(l_values.value(0).trimmed().toLower(), p_default_horizontal) |
                                 QHash<QString, Qt::Alignment>{
                                     {"top", Qt::AlignTop}, {"center", Qt::AlignVCenter}, {"bottom", Qt::AlignBottom}}
                                     .value(l_values.value(1).trimmed().toLower(), p_default_vertical)));
}

void set_text_alignment(QWidget *p_widget, QString p_identifier, QString p_ini_file, AOApplication *ao_app)
{
  set_text_alignment_or_default(p_widget, p_identifier, p_ini_file, ao_app, "alignment", Qt::AlignLeft,
                                Qt::AlignVCenter);
}

void set_font(QWidget *p_widget, QString p_identifier, QString ini_file, AOApplication *ao_app)
{
  QString class_name = p_widget->metaObject()->className();

  int f_weight = ao_app->get_font_property(p_identifier, ini_file);

  // Font priority
  // 1. "font_" + p_identifier
  // 2. "font_default"
  // 3. System font
  QFontDatabase font_database;
  QString font_name = ao_app->get_font_name("font_" + p_identifier, ini_file);
  if (!font_database.families().contains(font_name))
  {
    font_name = ao_app->get_font_name("font_default", ini_file);
  }
  p_widget->setFont(QFont(font_name, f_weight));

  const QColor l_font_color = ao_app->get_color(p_identifier + "_color", ini_file);
  int bold = ao_app->get_font_property(p_identifier + "_bold", ini_file);
  QString is_bold = (bold == 1 ? "bold" : "");

  QString style_sheet_string = class_name + " { " + "background-color: rgba(0, 0, 0, 0);\n" +
                               "color: " + l_font_color.name(QColor::HexArgb) + ";\n" + "font: " + is_bold + ";" + " }";
  p_widget->setStyleSheet(style_sheet_string);
}

void set_drtextedit_font(DRTextEdit *p_widget, QString p_identifier, QString ini_file, AOApplication *ao_app)
{
  set_font(p_widget, p_identifier, ini_file, ao_app);

  // Do outlines
  bool outline = (ao_app->get_font_property(p_identifier + "_outline", ini_file) == 1);
  p_widget->set_outline(outline);

  // alignment
  set_text_alignment_or_default(p_widget, p_identifier, ini_file, ao_app, "text_alignment", Qt::AlignLeft,
                                Qt::AlignTop);
}
