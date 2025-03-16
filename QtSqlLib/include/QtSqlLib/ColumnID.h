#pragma once

#include <QtSqlLib/API/SchemaTypes.h>

#include <QString>

namespace QtSqlLib
{

class ColumnID
{
public:
  ColumnID();
  ColumnID(const API::IID& columnId);
  ColumnID(const API::IID& tableId, const API::IID& columnId);
  ColumnID(const QString& tableAlias, const API::IID& tableId, const API::IID& columnId);
  virtual ~ColumnID();

  bool isTableIdValid() const;
  API::IID::Type tableId() const;
  API::IID::Type columnId() const;
  QString tableAlias() const;

private:
  ColumnID(const QString& tableAlias, const std::optional<API::IID::Type>& tableId, API::IID::Type columnId);

  QString m_tableAlias;
  std::optional<API::IID::Type> m_tableId;
  API::IID::Type m_columnId;

};

}

Q_DECLARE_METATYPE(QtSqlLib::ColumnID);
