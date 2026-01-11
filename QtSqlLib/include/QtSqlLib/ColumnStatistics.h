#pragma once

#include <QtSqlLib/API/IID.h>

#include <QString>

#include <optional>

namespace QtSqlLib
{

class ColumnStatistics
{
public:
  enum class EType : unsigned char
  {
    Min = 0x1,
    Max = 0x2,
    Sum = 0x3,
    Count = 0x4,
    Avg = 0x5,
    GroupConcat = 0x6
  };

  enum class EMethod : unsigned char
  {
    All = 0x0,
    Distict = 0x1
  };

  static ColumnStatistics min(API::IID::Type columnId);
  static ColumnStatistics max(API::IID::Type columnId);
  static ColumnStatistics sum(API::IID::Type columnId, EMethod method = EMethod::All);
  static ColumnStatistics count(API::IID::Type columnId, EMethod method = EMethod::All);
  static ColumnStatistics count(EMethod method = EMethod::All);
  static ColumnStatistics avg(API::IID::Type columnId, EMethod method = EMethod::All);
  static ColumnStatistics groupConcat(API::IID::Type columnId, const std::optional<char>& separator = std::nullopt);

  static bool isColumnStatistics(API::IID::Type id);
  static ColumnStatistics fromId(API::IID::Type id);

  static QString toString(
    EType type, EMethod method, const std::optional<char>& separator, const std::optional<QString>& column = std::nullopt);

  API::IID::Type id() const;

  EType type() const;
  EMethod method() const;
  bool hasColumn() const;
  API::IID::Type columnId() const;
  std::optional<char> separator() const;

private:
  ColumnStatistics(EType type, EMethod method, bool hasColumn, API::IID::Type columnId, const std::optional<char>& separator);

  EType m_type : 3;
  EMethod m_method : 1;
  bool m_hasColumn : 1;
  API::IID::Type m_columnId : sizeof(API::IID::Type) * 8 - 11;
  std::optional<char> m_groupConcatSeparator;

};

}
