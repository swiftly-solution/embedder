#ifndef _embedder_src_engine_classes_h
#define _embedder_src_engine_classes_h

#include <map>
#include <string>
#include <any>

class EContext;

class ClassData
{
private:
    std::map<std::string, std::any> m_classData;
    std::string m_className;
    EContext* m_ctx;

public:
    ClassData(std::map<std::string, std::any> data, std::string className, EContext* ctx);
    ~ClassData();

    std::string GetClassname();

    void SetData(std::string key, std::any value);

    template <class T>
    T GetData(std::string key)
    {
        try
        {
            return std::any_cast<T>(m_classData[key]);
        }
        catch (std::bad_any_cast& e)
        {
            return T();
        }
    }

    template<class T>
    T* GetDataPtr(std::string key)
    {
        try
        {
            return std::any_cast<T>(&(m_classData[key]));
        }
        catch (std::bad_any_cast& e)
        {
            return (T*)0;
        }
    }

    template <class T>
    T GetDataOr(std::string key, T value)
    {
        try
        {
            return std::any_cast<T>(m_classData[key]);
        }
        catch (std::bad_any_cast& e)
        {
            return value;
        }
    }

    bool HasData(std::string key);
};

#endif