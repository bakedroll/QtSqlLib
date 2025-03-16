#include "QtSqlLib/API/SchemaTypes.h"

namespace QtSqlLib::API
{

bool TableColumnId::operator<(const TableColumnId& rhs) const
{
  if (tableId == rhs.tableId)
  {
    return columnId < rhs.columnId;
  }
  return tableId < rhs.tableId;
}

bool TableColumnId::operator!=(const TableColumnId& rhs) const
{
  return (tableId != rhs.tableId) || (columnId != rhs.columnId);
}

bool RelationshipTableId::operator<(const RelationshipTableId& rhs) const
{
  if (relationshipId == rhs.relationshipId)
  {
    return tableId < rhs.tableId;
  }
  return relationshipId < rhs.relationshipId;
}

}
