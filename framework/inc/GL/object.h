#pragma once

#include <string>

class Object
{
public:
    void SetName(const std::string& name) { m_name = name; }
    const std::string GetName() const { return m_name; }

protected:
    Object() {}

private:
    std::string m_name = "unnamed_object";
};
