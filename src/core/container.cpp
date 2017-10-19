#include "container.h"

namespace CB {

Container::Container() = default;
Container::~Container() = default;

std::any Container::Value() { return {}; }

ContainerPtr ContainerHelper::Open(const std::string &name) {
  auto h = std::find_if(handler_list.begin(), handler_list.end(),
                        [&](const OpenHandler &h) { return h.name == name; });
  if (h == handler_list.end())
    return nullptr;
  return h->handler();
}

std::vector<std::string> ContainerHelper::List() {
  std::vector<std::string> result(handler_list.size());
  std::transform(handler_list.begin(), handler_list.end(), result.begin(),
                 [](const OpenHandler &h) { return h.name; });
  return result;
}

void ContainerHelper::InstallList(const HandlerList &list) {
  handler_list.insert(handler_list.end(), list.begin(), list.end());
}

std::any FileContainer::Value() { return file; }

std::string WithIndex(const std::string &base, std::size_t index) {
  return base + "[" + std::to_string(index) + "]";
}

} // namespace CB