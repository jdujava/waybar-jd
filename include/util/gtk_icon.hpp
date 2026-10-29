#pragma once
#include <gtkmm/icontheme.h>

#include <mutex>
#include <string>

class DefaultGtkIconThemeWrapper {
 private:
  static std::mutex default_theme_mutex;

 public:
  static bool has_icon(const std::string&);
  static Gtk::IconInfo lookup_icon(const Glib::RefPtr<Gtk::IconTheme>&, const char*, int,
                                  Gtk::IconLookupFlags, int theme_size = 0);
  static Glib::RefPtr<Gdk::Pixbuf> load_icon(
      const char*, int, Gtk::IconLookupFlags,
      Glib::RefPtr<Gtk::StyleContext> style = Glib::RefPtr<Gtk::StyleContext>(), int theme_size = 0);
};
