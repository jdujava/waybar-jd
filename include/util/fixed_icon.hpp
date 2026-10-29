#pragma once

#include <pango/pangocairo.h>

#include <algorithm>

#include "util/fit_label.hpp"

namespace waybar::util {

// Opt-in fixed slot for the first glyph. Pango reserves ordinary text geometry;
// the glyph is painted separately, so its font size cannot move nearby text.
class FixedIconLabel : public FitLabel {
 public:
  FixedIconLabel() {
    style_connection_ = signal_style_updated().connect(sigc::mem_fun(*this, &FixedIconLabel::updateIcon));
  }

  ~FixedIconLabel() override {
    style_connection_.disconnect();
    if (icon_) pango_cairo_context_set_shape_renderer(icon_->get_context()->gobj(), nullptr, nullptr, nullptr);
  }

  void configureIcon(int width, double offset_y = 0) {
    width_ = std::max(0, width);
    offset_y_ = offset_y;
    updateIcon();
  }

  void set_markup(const Glib::ustring& markup) {
    if (width_ > 0) clearIconAttributes();
    Gtk::Label::set_markup(markup);
    updateIcon();
  }

  void set_text(const Glib::ustring& text) {
    if (width_ > 0) clearIconAttributes();
    Gtk::Label::set_text(text);
    updateIcon();
  }

 private:
  int width_ = 0;
  double offset_y_ = 0;
  Glib::RefPtr<Pango::Layout> icon_;
  sigc::connection style_connection_;

  void clearIconAttributes() {
    gtk_label_set_attributes(gobj(), nullptr);
    resetFitScale();
  }

  void updateIcon() {
    if (width_ <= 0) return;
    clearIconAttributes();
    const auto text = get_text();
    if (text.empty()) return;
    const auto glyph = text.substr(0, 1);
    const auto end = glyph.bytes();
    auto attributes = get_layout()->get_attributes();
    auto glyph_attributes = attributes
                                ? Pango::AttrList(pango_attr_list_copy(attributes.gobj()), false)
                                : Pango::AttrList();
    // Vertical positioning is relative to the visible glyph, using offset_y.
    auto rise = Pango::Attribute::create_attr_rise(0);
    rise.set_start_index(0);
    rise.set_end_index(end);
    glyph_attributes.change(rise);
    auto context = get_pango_context();
    if (!icon_ || icon_->get_context()->gobj() != context->gobj()) {
      if (icon_) pango_cairo_context_set_shape_renderer(icon_->get_context()->gobj(), nullptr, nullptr, nullptr);
      icon_ = Pango::Layout::create(context);
    }
    if (icon_->get_text().raw() != glyph.raw()) icon_->set_text(glyph);
    icon_->set_font_description(get_style_context()->get_font(get_state_flags()));
    icon_->set_attributes(glyph_attributes);
    pango_cairo_context_set_shape_renderer(context->gobj(), renderIcon, this, nullptr);

    Pango::Rectangle ink;
    Pango::Rectangle logical;
    logical.set_y(-12 * PANGO_SCALE);
    logical.set_width(width_ * PANGO_SCALE);
    logical.set_height(16 * PANGO_SCALE);
    auto shape = Pango::Attribute::create_attr_shape(ink, logical);
    shape.set_start_index(0);
    shape.set_end_index(end);
    Pango::AttrList slot_attributes(pango_attr_list_copy(glyph_attributes.gobj()), false);
    slot_attributes.change(shape);
    Gtk::Label::set_attributes(slot_attributes);
  }

  static void renderIcon(cairo_t* cr, PangoAttrShape*, gboolean path, gpointer data) {
    auto& self = *static_cast<FixedIconLabel*>(data);
    if (!self.icon_) return;
    double x, y;
    cairo_get_current_point(cr, &x, &y);
    Pango::Rectangle ink, logical;
    self.icon_->get_pixel_extents(ink, logical);
    int layout_x, layout_y;
    self.get_layout_offsets(layout_x, layout_y);
    const auto allocation = self.get_allocation();
    // The shape callback starts at the text baseline. Convert the label's
    // allocation into those coordinates instead of assuming fixed font metrics.
    const double baseline = self.get_layout()->get_baseline() / double(PANGO_SCALE);
    const double top = y - baseline - (layout_y - allocation.get_y());
    const int height = allocation.get_height();
    const auto color = self.get_style_context()->get_color(self.get_state_flags());
    cairo_save(cr);
    // Clip oversized/shifted icons to their slot and the actual label height.
    cairo_rectangle(cr, x, top, self.width_, height);
    cairo_clip(cr);
    cairo_set_source_rgba(cr, color.get_red(), color.get_green(), color.get_blue(), color.get_alpha());
    cairo_move_to(cr, x + (self.width_ - ink.get_width()) / 2.0 - ink.get_x(),
                  top + (height - ink.get_height()) / 2.0 + self.offset_y_ - ink.get_y());
    if (path)
      pango_cairo_layout_path(cr, self.icon_->gobj());
    else
      pango_cairo_show_layout(cr, self.icon_->gobj());
    cairo_restore(cr);
  }
};

}  // namespace waybar::util
