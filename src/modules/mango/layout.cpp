#include "modules/mango/layout.hpp"

#include <spdlog/spdlog.h>

namespace waybar::modules::mango {

Layout::Layout(const std::string& id, const Bar& bar, const Json::Value& config)
    : ALabel(config, "layout", id, "{}", 0, false), bar_(bar) {
  IPC::getInstance().registerForIPC("monitor", this);
}

Layout::~Layout() { IPC::getInstance().unregisterForIPC(this); }

void Layout::onEvent(const Json::Value& ev) { dp.emit(); }

void Layout::doUpdate() {
  std::lock_guard<std::mutex> lock(mutex_);

  const auto monitor = IPC::getInstance().getMonitor(bar_.output->name);
  std::string symbol = monitor["layout_symbol"].asString();
  // Overview keeps the underlying tag's layout symbol in Mango's IPC state.
  const auto& tags = monitor["active_tags"];
  if (tags.isArray() && tags.size() == 1 && tags[0].asUInt() == 0) symbol = "OVERVIEW";

  if (symbol.empty()) {
    label_.hide();
    last_symbol_.clear();
    return;
  }

  if (symbol != last_symbol_) {
    if (!last_symbol_.empty()) label_.get_style_context()->remove_class(last_symbol_);
    label_.get_style_context()->add_class(symbol);
    last_symbol_ = symbol;
  }

  std::string text;
  std::string format_key = "format-" + symbol;

  if (config_.isMember(format_key)) {
    text = fmt::format(fmt::runtime(config_[format_key].asString()), fmt::arg("symbol", symbol));
  } else {
    text = fmt::format(fmt::runtime(format_), fmt::arg("symbol", symbol));
  }

  if (!text.empty()) {
    label_.show();
    setLabelMarkup(text);
  } else {
    label_.hide();
  }
}

void Layout::update() {
  doUpdate();
  ALabel::update();
}

void Layout::doAction(const std::string& action) {
  Json::Value request;
  if (action == "cycle") {
    request["command"] = "dispatch switch_layout";
  } else if (action == "toggle-floating") {
    request["command"] = "dispatch togglefloating";
  } else {
    return;
  }
  IPC::sendAsync(request);
}

}  // namespace waybar::modules::mango
