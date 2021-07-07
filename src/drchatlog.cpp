#include "drchatlog.h"

#include "aoconfig.h"

#include <QRegExp>
#include <QScrollBar>

DRChatLog::DRChatLog(QWidget *parent) : QTextBrowser(parent), dr_config(new AOConfig(this))
{
  connect(this, SIGNAL(message_queued()), this, SLOT(_p_write_message_queue()));
  connect(dr_config, SIGNAL(loaded_theme()), this, SLOT(_p_reset_log()));
}

void DRChatLog::append_chatmessage(QString p_name, QString p_text)
{
  queue_message(p_name.trimmed().isEmpty() ? "Anonymous" : p_name, p_text);
}

void DRChatLog::append_error(QString p_text)
{
  queue_message(nullptr, p_text);
}

void DRChatLog::queue_message(QString p_name, QString p_text)
{
  Message l_message;
  l_message.timestamp = QDateTime::currentDateTime();
  l_message.name = p_name;
  l_message.text = p_text;
  m_message_queue.append(std::move(l_message));
  Q_EMIT message_queued();
}

void DRChatLog::_p_write_message_queue()
{
  QScrollBar *l_scrollbar = verticalScrollBar();
  const bool l_is_end_scroll_pos = l_scrollbar->value() == l_scrollbar->maximum();

  QTextCursor l_cursor = textCursor();
  l_cursor.movePosition(QTextCursor::End);

  while (!m_message_queue.isEmpty())
  {
    if (!m_message_list.isEmpty())
      l_cursor.insertHtml(QString("<br />"));
    const Message l_message = m_message_queue.dequeue();
    m_message_list.append(l_message);

    l_cursor.insertText(QString(QString("[%1] ").arg(l_message.timestamp.toString("hh:mm"))));

    if (!l_message.name.isEmpty())
      l_cursor.insertHtml(QString("<b>%1</b>: ").arg(l_message.name.toHtmlEscaped()));

    QString l_text = l_message.text.toHtmlEscaped();
    const QRegExp l_regex("(https?://[^\\s/$.?#].[^\\s]*)");
    if (l_text.contains(l_regex))
      l_text.replace(l_regex, "<a href=\"\\1\">\\1</a>");
    l_cursor.insertHtml(l_text.replace("\n", "<br />"));
  }

  if (l_is_end_scroll_pos)
    l_scrollbar->setValue(l_scrollbar->maximum());
}

void DRChatLog::_p_reset_log()
{
  clear();
  QQueue<Message> l_new_message_queue;
  dynamic_cast<QList<Message> &>(l_new_message_queue) = std::move(m_message_list);
  l_new_message_queue.append(std::move(m_message_queue));
  m_message_queue = std::move(l_new_message_queue);
  _p_write_message_queue();
}
