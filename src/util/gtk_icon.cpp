#include "util/gtk_icon.hpp"

#include <giomm/icon.h>
#include <glibmm/fileutils.h>

/* We need a global mutex for accessing the object returned by Gtk::IconTheme::get_default()
 * because it always returns the same object across different threads, and concurrent
 * access can cause data corruption and lead to invalid memory access and crashes.
 * Even concurrent calls that seem read only such as has_icon can cause issues because
 * the GTK lib may update the internal icon cache on this calls.
 */

std::mutex DefaultGtkIconThemeWrapper::default_theme_mutex;

bool DefaultGtkIconThemeWrapper::has_icon(const std::string& value) {
  const std::lock_guard<std::mutex> lock(default_theme_mutex);

  return Gtk::IconTheme::get_default()->has_icon(value);
}

Gtk::IconInfo DefaultGtkIconThemeWrapper::lookup_icon(const Glib::RefPtr<Gtk::IconTheme>& theme,
                                                     const char* name, int size,
                                                     Gtk::IconLookupFlags flags, int theme_size) {
  auto info = theme->lookup_icon(name, theme_size > 0 ? theme_size : size, flags);
  if (info && theme_size > 0 && theme_size != size) {
    const auto filename = info.get_filename();
    if (!filename.empty() && Glib::file_test(filename, Glib::FILE_TEST_IS_REGULAR)) {
      // Keep the selected artwork, but let GTK rasterize SVGs at the target size
      // and retain symbolic recoloring instead of enlarging a small bitmap.
      auto resized = theme->lookup_icon(Gio::Icon::create(filename), size, flags);
      if (resized) return resized;
    }
  }
  return info;
}

Glib::RefPtr<Gdk::Pixbuf> DefaultGtkIconThemeWrapper::load_icon(
    const char* name, int tmp_size, Gtk::IconLookupFlags flags,
    Glib::RefPtr<Gtk::StyleContext> style, int theme_size) {
  const std::lock_guard<std::mutex> lock(default_theme_mutex);

  auto default_theme = Gtk::IconTheme::get_default();

  auto icon_info = lookup_icon(default_theme, name, tmp_size, flags, theme_size);

  if (icon_info == nullptr) {
    return default_theme->load_icon(name, tmp_size, flags);
  }

  if (style.get() == nullptr) {
    return icon_info.load_icon();
  }

  bool is_sym = false;
  return icon_info.load_symbolic(style, is_sym);
}
