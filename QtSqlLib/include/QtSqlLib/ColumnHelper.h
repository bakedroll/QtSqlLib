#pragma once

#include <QtSqlLib/API/IID.h>

#include <optional>
#include <type_traits>
#include <vector>

#include <QMetaType>

namespace QtSqlLib
{

class ConcatenatedColumn;

class ColumnHelper
{
public:
  enum class EOrder
  {
    Ascending,
    Descending
  };

  struct ColumnAlias
  {
    ColumnAlias();
    explicit ColumnAlias(const QString& alias);

    QString alias;
  };

  struct SelectColumn
  {
    SelectColumn();

    template<typename T>
    SelectColumn(const T& id, const QString& alias = "") :
      columnId(castId(id)),
      alias(alias)
    {
    }


    SelectColumn(const std::shared_ptr<ConcatenatedColumn>& concatenatedColumn) :
      concatenatedColumn(concatenatedColumn)
    {
    }

    API::IID::Type columnId = 0;
    //TODO: // or unique_ptr
    std::shared_ptr<ConcatenatedColumn> concatenatedColumn;

    // https://www.sqlitetutorial.net/sqlite-string-functions/sqlite-concat/
    // https://thelinuxcode.com/concatenate-two-columns-sqlite-with-space/

    QString alias;
  };

  struct ColumnData
  {
    ColumnData();

    template<typename TRel, typename TID>
    ColumnData(const TRel& relId, const TID& id) :
      relationshipId(castId(relId)),
      columnId(castId(id))
    {
    }

    template<typename TID>
    ColumnData(const TID& id) :
      columnId(castId(id))
    {
    }

    std::optional<API::IID::Type> relationshipId;
    API::IID::Type columnId = 0;
  };

  struct GroupColumn
  {
    GroupColumn();
    GroupColumn(const ColumnData& data);

    template<typename T>
    GroupColumn(const T& id) :
        data({ castId(id) })
    {
    }

    ColumnData data;
  };

  struct OrderColumn
  {
    OrderColumn();
    OrderColumn(const ColumnData& data);

    template<typename T>
    OrderColumn(const T& id) :
        data({ castId(id) })
    {
    }

    ColumnData data;
    EOrder order = EOrder::Ascending;
  };

  using SelectColumnList = std::vector<SelectColumn>;
  using GroupColumnList = std::vector<GroupColumn>;
  using OrderColumnList = std::vector<OrderColumn>;

  template <typename TElem, typename... Args, typename = std::enable_if_t<
    std::is_same_v<TElem, SelectColumn> ||
    std::is_same_v<TElem, GroupColumn> ||
    std::is_same_v<TElem, OrderColumn>>>
  static std::vector<TElem> make(const Args&... args)
  {
    const auto size = expectedSize(Identity<TElem>(), args...);
    std::vector<TElem> list(size);
    makeIntern(list, 0, args...);
    return list;
  }

private:
  template<typename T, bool isFundamental>
  struct UnderlyingBase
  {
    using Type = typename std::underlying_type<T>::type;
  };

  template <typename T>
  struct UnderlyingBase<T, true>
  {
    using Type = T;
  };

  template <typename T>
  struct Underlying : public UnderlyingBase<T, std::is_fundamental_v<T>>
  {
  };

  template<typename T>
  struct Identity
  {
    using Type = T;
  };

  template <typename T>
  static API::IID::Type castId(const T& value)
  {
    return static_cast<API::IID::Type>(static_cast<typename Underlying<T>::Type>(value));
  }

  template<typename TElem, typename... Args>
  static size_t expectedSize(const Identity<TElem>&, const Args&...)
  {
    return sizeof...(Args);
  }

  template<typename... Args>
  static size_t expectedSize(const Identity<OrderColumn>&, const Args&... args)
  {
    return countOrderColumns(args...);
  }

  template<typename TID, typename... Args>
  static size_t countOrderColumns(const TID&, const Args&... tail)
  {
    return 1 + countOrderColumns(tail...);
  }

  template<typename... Args>
  static size_t countOrderColumns(EOrder, const Args&... tail)
  {
    return countOrderColumns(tail...);
  }

  static size_t countOrderColumns()
  {
    return 0;
  }

  template<typename TElem, typename TID, typename... Args>
  static void makeIntern(std::vector<TElem>& list, size_t currentIndex, const TID& id, const Args&... tail)
  {
    list[currentIndex] = TElem(id);
    makeIntern(list, currentIndex + 1, tail...);
  }

  template<typename TElem, typename TID>
  static void makeIntern(std::vector<TElem>& list, size_t currentIndex, const TID& id)
  {
    list[currentIndex] = TElem(id);
  }

  template<typename TElem, typename... Args>
  static void makeIntern(std::vector<TElem>& list, size_t currentIndex, const SelectColumn& selectColumn, const Args&... tail)
  {
    list[currentIndex] = selectColumn;
    makeIntern(list, currentIndex + 1, tail...);
  }

  template<typename TElem>
  static void makeIntern(std::vector<TElem>& list, size_t currentIndex, const SelectColumn& selectColumn)
  {
    list[currentIndex] = selectColumn;
  }

  template<typename TElem, typename... Args>
  static void makeIntern(std::vector<TElem>& list, size_t currentIndex, EOrder order, const Args&... tail)
  {
    list[currentIndex - 1].order = order;
    makeIntern(list, currentIndex, tail...);
  }

  template<typename TElem>
  static void makeIntern(std::vector<TElem>& list, size_t currentIndex, EOrder order)
  {
    list[currentIndex - 1].order = order;
  }

};

}

Q_DECLARE_METATYPE(QtSqlLib::ColumnHelper::ColumnData);
Q_DECLARE_METATYPE(QtSqlLib::ColumnHelper::ColumnAlias);
