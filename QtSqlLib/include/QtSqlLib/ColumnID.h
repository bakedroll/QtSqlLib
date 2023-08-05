#pragma once

#include <QtSqlLib/API/SchemaTypes.h>

namespace QtSqlLib
{

class ColumnID
{
public:
  ColumnID();
  ColumnID(const API::TableColumnId& tableColumnId);
  ColumnID(const API::IID& columnId);
  ColumnID(const QString& tableAlias, const API::TableColumnId& tableColumnId);
  virtual ~ColumnID();

  const API::TableColumnId& get() const;
  bool isTableIdValid() const;
  QString getTableAlias() const;

private:
  ColumnID(const QString& tableAlias, const API::TableColumnId& tableColumnId, bool bIsTableIdValid);

  QString m_tableAlias;
  API::TableColumnId m_tableColumnId;
  bool m_bIsTableIdValid;

};

}

Q_DECLARE_METATYPE(QtSqlLib::ColumnID);
