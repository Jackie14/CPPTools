//////////////////////////////////////////////////////////////////////////
// XMLSerialize.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef XMLSerialize_INCLUDED
#define XMLSerialize_INCLUDED

#include <string>
#include <vector>
#include <map>
#include <stdlib.h>
#include <stdio.h>
#include "XMLParser.h"
#include "Types.h"

#define XML_ATTRIBUTE(var) XMLSerialize::SerializeAttribute(node, #var, var, restore)
#define XML_COMPLEX(var) XMLSerialize::SerializeComplex(node, #var, var, restore)
#define XML_ENUM(var) XMLEnumerable::SerializeEnum(valueName, value, #var, var, restore)

class XMLEnumerable
{
public:
    static void SerializeEnum(std::string& valueName, UInt64& value,
            const std::string& matchName, UInt64 matchValue, bool restore)
    {
        if (restore)
        {
            if (strcasecmp(valueName.c_str(), matchName.c_str()) == 0)
            {
                value = matchValue;
            }
        }
        else
        {
            if (value == matchValue)
            {
                valueName = matchName;
            }
        }
    }

    UInt64 GetValue() const
    {
        return mValue;
    }

    void SetValue(UInt64 value)
    {
        mValue = value;
    }

    bool operator ==(UInt64 value) const
    {
        return mValue == value;
    }

    bool operator !=(UInt64 value) const
    {
        return mValue != value;
    }

    virtual void Serialize(std::string& valueName, bool restore) = 0;
    virtual ~XMLEnumerable()
    {
    }

private:
    UInt64 mValue;
};

class XMLSerialize
{
public:
    XMLSerialize();
    virtual ~XMLSerialize();

    virtual void Serialize(XMLElement* node, bool restore) = 0;

    bool LoadFromFile(const std::string& path);
    bool LoadFromBuffer(const std::string& buffer);
    bool SaveToFile(const std::string& path, const std::string& rootName);
    int SaveToBuffer(std::string& buffer, const std::string& rootName);

    static void SerializeAttribute(XMLElement* node, const std::string& name,
            bool& value, bool restore);
    static void SerializeAttribute(XMLElement* node, const std::string& name,
            Int8& value, bool restore);
    static void SerializeAttribute(XMLElement* node, const std::string& name,
            Int16& value, bool restore);
    static void SerializeAttribute(XMLElement* node, const std::string& name,
            Int32& value, bool restore);
    static void SerializeAttribute(XMLElement* node, const std::string& name,
            Int64& value, bool restore);
    static void SerializeAttribute(XMLElement* node, const std::string& name,
            std::string& value, bool restore);
    static void SerializeAttribute(XMLElement* node, const std::string& name,
            XMLEnumerable& value, bool restore);
    static void SerializeComplex(XMLElement* node, const std::string& name,
            XMLSerialize& value, bool restore);
    static void SerializeComplex(XMLElement* node, const std::string& name,
            XMLEnumerable& value, bool restore);
    static void SerializeComplex(XMLElement* node, const std::string& name,
            std::string& value, bool restore);
    static void SerializeComplex(XMLElement* node, const std::string& name,
            std::vector<std::string>& value, bool restore);
    static void SerializeComplex(XMLElement* node, const std::string& name,
            std::vector<int>& value, bool restore);
    static void SerializeComplex(XMLElement* node, const std::string& name,
            bool& value, bool restore);
    static void SerializeComplex(XMLElement* node, const std::string& name,
            Int8& value, bool restore);
    static void SerializeComplex(XMLElement* node, const std::string& name,
            Int16& value, bool restore);
    static void SerializeComplex(XMLElement* node, const std::string& name,
            Int32& value, bool restore);
    static void SerializeComplex(XMLElement* node, const std::string& name,
            Int64& value, bool restore);
    static void SerializeComplex(XMLElement* node, const std::string& name,
            std::map<std::string, std::string>& value, bool restore);

    template<typename T>
    static void SerializeComplex(XMLElement* node, const std::string& name,
            std::vector<T>& value, bool restore)
    {
        if (restore)
        {
            for (XMLElement* subNode = node->GetFirstChildElement(name); subNode
                    != NULL; subNode = subNode->GetNextSiblingElement(name))
            {
                T tmp;
                tmp.Serialize(subNode, restore);
                value.push_back(tmp);
            }
        }
        else
        {
            for (typename std::vector<T>::iterator iter = value.begin();
            		iter != value.end(); ++iter)
            {
                XMLElement* subNode = new XMLElement(name);
                iter->Serialize(subNode, restore);
                node->LinkEndChild(subNode);
            }
        }
    }

    template<typename T>
    static void SerializeComplex(XMLElement* node, const std::string& name,
            std::map<std::string, T>& value, bool restore)
    {
        if (restore)
        {
            for (XMLElement* subNode = node->GetFirstChildElement(name); subNode
                    != NULL; subNode = subNode->GetNextSiblingElement(name))
            {
                T tmp;
                std::string id;
                subNode->GetAttribute("id", id);
                tmp.Serialize(subNode, restore);
                value[id] = tmp;
            }
        }
        else
        {
            for (typename std::map<std::string, T>::iterator iter = value.begin();
            		iter != value.end(); ++iter)
            {
                XMLElement* subNode = new XMLElement(name);
                std::string id = iter->first;
                XMLSerialize::SerializeAttribute(subNode, "id", id, restore);
                iter->second.Serialize(subNode, restore);
                node->LinkEndChild(subNode);
            }
        }
    }
};

#endif // XMLSerialize_INCLUDED
