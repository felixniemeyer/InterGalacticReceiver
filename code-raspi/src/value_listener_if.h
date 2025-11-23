#ifndef VALUE_LISTENER_H
#define VALUE_LISTENER_H

class IValueListener
{
  public:
    virtual void update(int val) = 0;
    virtual ~IValueListener() = default;
};

#endif
