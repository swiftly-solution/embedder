#ifndef _embedder_src_dotnet_invoker_h
#define _embedder_src_dotnet_invoker_h

/**
 * This file is inspired from FiveM's C# resources implementation.
 * All credits go to the CFX.re project located at https://github.com/citizenfx/fivem (https://cfx.re)
 *
 * Copyright (c) 2014 Bas Timmer/NTAuthority et al.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * This file has been modified from its original form for use in this program
 * under GNU Lesser General Public License, version 2.
 */

#include <cstdint>
#include <string>
#include <cstring>
#include <typeindex>
#include <typeinfo>
#include <map>

void* DotnetAllocateContextPointer(int size, int count);

extern std::map<std::type_index, int> typesMap;

enum class CallKind
{
    None,
    Function,
    ClassMember,
    ClassFunction,
    CoreClassFunction
};

struct MapData
{
    void** keys;
    void** values;
    int key_type;
    int value_type;
    int length;
};

struct ArrayData
{
    void** elements;
    int length;
    int type;
};

struct CallData
{
    int args_count;
    int has_return;

    uint64_t args_data[64];
    int args_type[64];

    uint64_t return_value;
    int return_type;

    const char* namespace_str;
    int namespace_len;

    const char* function_str;
    int function_len;

    const char* dbginfo_str;
    int dbginfo_len;

    int call_kind;
};

class CallContext
{
public:
    enum
    {
        ArgsCap = 64,
        ArgsSize = sizeof(uint64_t)
    };

protected:
    int m_args_count;
    int m_has_return;
    void* m_args_data;
    int* m_args_type;
    void* m_return_value;

    int m_return_type;

    const char* m_namespace_str;
    int m_namespace_len;

    const char* m_function_str;
    int m_function_len;

    CallKind m_call_kind;

    CallData* m_cdata;
public:
    CallContext()
    {

    }

    CallContext(CallData& data)
    {
        m_has_return = data.has_return;
        m_args_count = data.args_count;
        m_args_data = (void*)(data.args_data);
        m_args_type = data.args_type;

        m_return_value = &data.return_value;
        m_return_type = data.return_type;

        m_namespace_str = data.namespace_str;
        m_namespace_len = data.namespace_len;
        m_function_str = data.function_str;
        m_function_len = data.function_len;

        m_call_kind = (CallKind)data.call_kind;

        m_cdata = &data;
    }

    inline std::string GetDebugInfo()
    {
        if (!m_cdata->dbginfo_str) return "";
        else return m_cdata->dbginfo_str;
    }

    inline int GetArgumentType(int index)
    {
        return m_args_type[index];
    }

    inline void SetArgumentType(int index, int type)
    {
        m_args_type[index] = type;
    }

    inline int GetReturnType()
    {
        return m_return_type;
    }

    inline void SetReturnType(int value)
    {
        m_return_type = value;
    }

    template <typename T> inline T GetArgument(int index)
    {
        auto functionData = (uint64_t*)m_args_data;

        return *reinterpret_cast<T*>(&functionData[index]);
    }

    inline void* GetArgumentPtr(int index)
    {
        auto functionData = (uint64_t*)m_args_data;
        return (void*)(&functionData[index]);
    }

    template <typename T> inline void SetArgument(int index, T value)
    {
        auto functionData = (uint64_t*)m_args_data;

        if (sizeof(T) < ArgsSize)
        {
            *reinterpret_cast<uint64_t*>(&functionData[index]) = 0;
        }

        *reinterpret_cast<T*>(&functionData[index]) = value;
    }

    inline int GetArgumentCount() { return m_args_count; }

    template <typename T> inline void PushArgument(T value)
    {
        auto functionData = (uint64_t*)m_args_data;

        if constexpr (std::is_same<T, std::string>::value) {
            std::string val = (std::string)value;
            char* string_buf = (char*)DotnetAllocateContextPointer(sizeof(char), val.size() + 1);
            strcpy(string_buf, val.c_str());
            *reinterpret_cast<char**>(&functionData[m_args_count]) = string_buf;
        }
        else {
            if (sizeof(T) < ArgsSize)
            {
                *reinterpret_cast<uint64_t*>(&functionData[m_args_count]) = 0;
            }

            *reinterpret_cast<T*>(&functionData[m_args_count]) = value;
        }
        m_args_count++;
        m_cdata->args_count++;
    }

    template <typename T> inline void SetResult(T value)
    {
        auto functionData = (uint64_t*)m_return_value;
        m_has_return = 1;
        m_cdata->has_return = 1;

        if constexpr (std::is_same<T, std::string>::value) {
            std::string val = (std::string)value;
            char* string_buf = (char*)DotnetAllocateContextPointer(sizeof(char), val.size() + 1);
            strcpy(string_buf, val.c_str());
            *reinterpret_cast<char**>(&functionData[0]) = string_buf;
            return;
        }
        else {
            if (sizeof(T) < ArgsSize)
            {
                *reinterpret_cast<uint64_t*>(&functionData[0]) = 0;
            }

            *reinterpret_cast<T*>(&functionData[0]) = value;
        }
    }

    template <typename T> inline T GetResult()
    {
        auto functionData = (uint64_t*)m_return_value;

        return *reinterpret_cast<T*>(functionData);
    }

    inline void* GetResultPtr()
    {
        auto functionData = (uint64_t*)m_return_value;
        return (void*)(functionData);
    }

    inline int GetNumArgs() { return m_cdata->args_count; }

    inline CallKind GetCallKind() { return m_call_kind; }
    inline std::string GetNamespace() { return std::string(m_namespace_str, m_namespace_len); }
    inline std::string GetFunction() { return std::string(m_function_str, m_function_len); }
};

void Dotnet_InvokeNative(CallData& context);
void Dotnet_ClassDataFinalizer(void* plugin_context, void* instance);

#endif