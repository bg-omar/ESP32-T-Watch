#ifndef MAIN_DISPLAY
#define MAIN_DISPLAY

  void LaunchSetDate();
  static void event_handler(lv_event_t * e);
  static void event_ok(lv_event_t * e);
  static void event_cancel(lv_event_t * e);
  static void  event_scrolling(lv_event_t * e);

#endif