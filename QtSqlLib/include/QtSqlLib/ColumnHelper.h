#pragma once

#include <QtSqlLib/API/IID.h>

#include <optional>
#include <type_traits>
#include <vector>

namespace QtSqlLib
{

class ColumnHelper
{
public:
  enum class EOrder
  {
    Ascending,
    Descending
  };

  struct GroupColumn
  {
    GroupColumn(const GroupColumn& rhs);
    GroupColumn(GroupColumn&& rhs);
    void operator=(const GroupColumn& rhs);

    template<typename T>
    GroupColumn(const T& id) :
        columnId(castId(id))
    {
    }

    std::optional<API::IID::Type> relationshipId;
    API::IID::Type columnId;
  };

  struct OrderColumn
  {
    OrderColumn(const OrderColumn& rhs);
    OrderColumn(OrderColumn&& rhs);
    void operator=(const OrderColumn& rhs);

    template<typename T>
    OrderColumn(const T& id) :
        columnId(castId(id))
    {
    }

    std::optional<API::IID::Type> relationshipId;
    API::IID::Type columnId;
    EOrder order = EOrder::Ascending;
  };

  using GroupColumnList = std::vector<GroupColumn>;
  using OrderColumnList = std::vector<OrderColumn>;

private:
  template<typename T, bool is_fundamental>
  struct underlying_base
  {
    using type = std::underlying_type<T>::type;
  };

  template <typename T>
  struct underlying_base<T, true>
  {
    using type = T;
  };

  template <typename T>
  struct underlying : public underlying_base<T, std::is_fundamental_v<T>>
  {
  };

  template <typename T>
  static API::IID::Type castId(const T& value)
  {
    return static_cast<API::IID::Type>(static_cast<typename underlying<T>::type>(value));
  }

};

};
