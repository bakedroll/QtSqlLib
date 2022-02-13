#pragma once

#include <QtSqlLib/Schema.h>

namespace QtSqlLib::API
{

class IQueryElement;
class IQueryVisitor;

class IQuerySequence
{
public:
  IQuerySequence() = default;
  virtual ~IQuerySequence() = default;

  IQuerySequence(const IQuerySequence& other) = delete;
  IQuerySequence& operator= (const IQuerySequence& other) = delete;

  virtual void addQuery(std::unique_ptr<IQueryElement> sequence) = 0;

  virtual void prepare(Schema& schema) = 0;
  virtual void traverse(IQueryVisitor& visitor) = 0;

};

}

