#include "aotimer.h"

#include <QDebug>
#include <QTimer>

AOTimer::AOTimer(QWidget *p_parent) : DRTextEdit(p_parent)
{
  // Adapted from:
  // https://stackoverflow.com/questions/36679708/how-to-make-a-chronometer-in-qt-c
  setStyleSheet("QLabel { color : white; }");
  setFrameStyle(QFrame::NoFrame);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setReadOnly(true);

  firing_timer = new QTimer(this);
  firing_timer->setTimerType(Qt::PreciseTimer);
  firing_timer->setInterval(firing_timer_length);
  connect(firing_timer, SIGNAL(timeout()), this, SLOT(update_time()));

  set_time(start_time);
  old_manual_timer.set_time(start_time);
  set_timestep_length(manual_timer_timestep_length);
  redraw();
}

void AOTimer::update_time()
{
  time_spent_in_timestep = 0;
  manual_timer.perform_timestep();

  // Check for underflow/overflow

  // This one checks overflows, as the updated manual timer would appear to have
  // a past time compared to the manual timer a step ago (represented by
  // old_manual_timer)
  if (manual_timer.get_timestep_length() > 0)
  {
    if (manual_timer.get_time().operator<(old_manual_timer.get_time()))
    {
      set_time(QTime(0, 0));
      firing_timer->stop();
      redraw();
      return;
    }
  }
  // This one checks underflows, as the updated manual timer would appear to
  // have a future time compared to the manual timer a step ago (represented by
  // old_manual_timer)
  else if (manual_timer.get_timestep_length() < 0)
  {
    if (manual_timer.get_time().operator>(old_manual_timer.get_time()))
    {
      set_time(QTime(0, 0));
      firing_timer->stop();
      redraw();
      return;
    }
  }

  // If no overflow/underflow, now update the old manual timer so it matches the
  // current timer Redraw and return
  old_manual_timer.perform_timestep();
  redraw();
  // As the firing timer is not single shot, it will restart automatically once
  // expired. However, it will do so with its firing interval, whatever it was
  // (which could be less than firing_timer_length if, say, the timer was
  // paused midstep).
  // The next firing_timer should take firing_timer_length miliseconds no
  // matter what. However, manually restarting a timer is a very expensive QT
  // operation, while QT's built-in automatic restart is very efficient.
  // Therefore, we only restart firing_timer if its length was not
  // firing_timer_length already (see example above).
  if (firing_timer->interval() != firing_timer_length)
    firing_timer->start(firing_timer_length);
}

void AOTimer::set()
{
  set_time(start_time);
}

void AOTimer::resume()
{
  paused = false;
  firing_timer->start();
}

void AOTimer::pause()
{
  paused = true;
  int remaining = firing_timer->remainingTime();
  firing_timer->stop();
  firing_timer->setInterval(remaining);
}

void AOTimer::redraw()
{
  setText(manual_timer.get_time().toString("mm:ss.zzz"));
}

void AOTimer::set_time(QTime new_time)
{
  manual_timer.set_time(new_time);
  old_manual_timer.set_time(new_time);
  redraw();
}

void AOTimer::set_timestep_length(int new_timestep_length)
{
  manual_timer.set_timestep_length(new_timestep_length);
  old_manual_timer.set_timestep_length(new_timestep_length);
}

void AOTimer::set_firing_interval(int new_firing_interval)
{
  /*
   * Update the firing interval of the steptimer for all future timesteps. If a
   * timestep has started when this function is scheduled to run, the current
   * timestep will be readapted to finish in this new firing interval as
   * follows:
   * - Assume length x of the 'current' timestep with orig has elapsed.
   * - Then, the current timestep will be adapted to have length max(0,
   * new_interval-x)
   */

  // Update time spent so far and new future firing interval
  time_spent_in_timestep += (firing_timer_length - firing_timer->remainingTime());
  firing_timer_length = new_firing_interval;
  // For this timestep however, the firing interval will be shorter than
  // firing_timer_length to account for the fact the timer may have already been
  // running a bit in this timestep
  int this_step_firing_interval;
  if (new_firing_interval < time_spent_in_timestep)
    this_step_firing_interval = 0;
  else
    this_step_firing_interval = new_firing_interval - time_spent_in_timestep;

  firing_timer->setInterval(this_step_firing_interval);

  if (!paused)
    firing_timer->start();
}

void AOTimer::set_concentrate_mode()
{
  set_timestep_length(-12);
  set_firing_interval(120);
}

void AOTimer::set_normal_mode()
{
  set_timestep_length(-12);
  set_firing_interval(12);
}

void AOTimer::set_fast_forward_mode()
{
  set_timestep_length(-24);
  set_firing_interval(12);
}
