#include <QDebug>

#include "aoapplication.h"

#include "datatypes.h"
#include "networkmanager.h"
#include "lobby.h"

int main(int argc, char *argv[])
{
  AOApplication main_app(argc, argv);
  main_app.construct_lobby();
  main_app.net_manager->connect_to_master();
  AOPacket *f_packet = new AOPacket("ALL#%");
  main_app.net_manager->send_ms_packet(f_packet);
  main_app.w_lobby->show();

  return main_app.exec();
}