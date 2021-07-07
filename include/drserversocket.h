#pragma once

#include "datatypes.h"
#include "drpacket.h"

#include <QAbstractSocket>
#include <QObject>

class QTcpSocket;
class QTimer;

class DRServerSocket : public QObject
{
  Q_OBJECT

public:
  DRServerSocket(QObject *parent = nullptr);

  bool is_connected() const;
  void connect_to_server(server_type server, bool is_reconnectable);
  void disconnect_from_server();

public slots:
  void send_packet(DRPacket packet);

signals:
  void connected_to_server();
  void disconnected_from_server();
  void reconnecting_to_server();
  void packet_received(DRPacket);
  void socket_error(QString);

private:
  static const int RECONNECT_DELAY;

  server_type m_server;
  QTcpSocket *m_socket = nullptr;
  bool m_is_connected = false;
  QTimer *m_reconnect_timer = nullptr;
  bool is_reconnectable = false;
  QString m_buffer;

private slots:
  void _p_update_state(QAbstractSocket::SocketState);
  void _p_reconnect_to_server();
  void _p_check_socket_error();
  void _p_read_socket();
};
