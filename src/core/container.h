#pragma once

#include "core/file.h"
#include "core/sub_file.h"
#include <algorithm>
#include <any>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace CB {

class Container;
using ContainerPtr = std::shared_ptr<Container>;

class Container {
public:
  Container();
  Container(const Container &) = delete;

  Container &operator=(const Container &) = delete;

  virtual ~Container();
  virtual ContainerPtr Open(const std::string &name) = 0;
  virtual std::vector<std::string> List() = 0;
  virtual std::any Value();

  template <typename T> T ValueT() { return std::any_cast<T>(Value()); }
};

class ContainerHelper : public Container {
public:
  ContainerPtr Open(const std::string &name) override;
  std::vector<std::string> List() override;

protected:
  struct OpenHandler {
    std::string name;
    std::function<ContainerPtr()> handler;
  };

  using HandlerList = std::vector<OpenHandler>;

  void InstallList(const HandlerList &list);

private:
  HandlerList handler_list;
};

// a label for big endian field
template <typename T> struct be_ {
public:
  be_() = delete;
};

template <typename T> class SimpleField : public ContainerHelper {
  static_assert(std::is_trivially_copyable<T>::value,
                "T must be trivially copyable!");

public:
  SimpleField(FB::FilePtr file) : file(std::move(file)) {}
  SimpleField(FB::FilePtr file, std::size_t offset)
      : file(std::make_shared<FB::SubFile>(std::move(file), offset,
                                           sizeof(T))) {}

  std::any Value() override { return file->Read<T>(0); }

private:
  FB::FilePtr file;
};

template <typename T> class SimpleField<be_<T>> : public ContainerHelper {
public:
  SimpleField(FB::FilePtr file) : file(std::move(file)) {}
  SimpleField(FB::FilePtr file, std::size_t offset)
      : file(std::make_shared<FB::SubFile>(std::move(file), offset,
                                           sizeof(T))) {}

  std::any Value() override { return swap(file->Read<T>(0)); }

private:
  FB::FilePtr file;
};

class FileContainer : public ContainerHelper {
public:
  FileContainer(FB::FilePtr file) : file(std::move(file)) {}

  std::any Value() override;

protected:
  template <typename T>
  OpenHandler Field(const std::string name, std::size_t offset) {
    return {name, [this, offset]() {
              return std::make_shared<SimpleField<T>>(file, offset);
            }};
  }

  FB::FilePtr file;
};

class ConstContainer : public ContainerHelper {
public:
  ConstContainer(std::any value) : value(std::move(value)) {}
  std::any Value() override { return value; }

private:
  std::any value;
};

std::string WithIndex(const std::string &base, std::size_t index);

} // namespace CB