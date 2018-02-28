//////////////////////////////////////////////////////////////////////////
// XMLSerialize.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "XMLSerialize.h"
#include <sstream>

XMLSerialize::XMLSerialize()
{
}

XMLSerialize::~XMLSerialize()
{
}

bool XMLSerialize::LoadFromFile(const std::string& path)
{
    XMLDoc doc(path);
    doc.LoadFile();
    if (doc.HasError())
    {
        return false;
    }
    else
    {
        Serialize(doc.GetRootElement(), true);
        return true;
    }
}

bool XMLSerialize::LoadFromBuffer(const std::string& buffer)
{
    XMLDoc doc;
    doc.Parse(buffer.c_str(), 0, XMLEncodingDefault);
    if (doc.HasError())
    {
        return false;
    }
    else
    {
        Serialize(doc.GetRootElement(), true);
        return true;
    }
}

bool XMLSerialize::SaveToFile(const std::string& path,
        const std::string& rootName)
{
    XMLDoc doc;
    XMLDeclaration* decl = new XMLDeclaration(("1.0"), ("utf-8"), (""));
    doc.LinkEndChild(decl);

    XMLElement* root = new XMLElement(rootName);
    Serialize(root, false);
    doc.LinkEndChild(root);

    return doc.SaveFile(path);
}

int XMLSerialize::SaveToBuffer(std::string& buffer, const std::string& rootName)
{
    XMLDoc doc;
    XMLDeclaration* decl = new XMLDeclaration(("1.0"), ("utf-8"), (""));
    doc.LinkEndChild(decl);

    XMLElement* root = new XMLElement(rootName);
    Serialize(root, false);
    doc.LinkEndChild(root);

    XMLPrinter printer;
    printer.SetStreamPrinting();
    root->Accept(&printer);
    buffer = printer.Str();

    return buffer.size();
}

void XMLSerialize::SerializeAttribute(XMLElement* node,
        const std::string& name, bool& value, bool restore)
{
    if (restore)
    {
        std::string strValue;
        bool result = node->GetAttribute(name, strValue);
        if (result)
        {
            int temp = atoi(strValue.c_str());
            if (temp == 0)
            {
                value = false;
            }
            else
            {
                value = true;
            }
        }
    }
    else
    {
        std::stringstream ss;
        ss << value;
        std::string buf = ss.str();
        node->SetAttribute(name, buf);
    }
}

void XMLSerialize::SerializeAttribute(XMLElement* node,
        const std::string& name, Int8& value, bool restore)
{
    SerializeAttribute(node, name, (Int32&) value, restore);
}

void XMLSerialize::SerializeAttribute(XMLElement* node,
        const std::string& name, Int16& value, bool restore)
{
    SerializeAttribute(node, name, (Int32&) value, restore);
}

void XMLSerialize::SerializeAttribute(XMLElement* node,
        const std::string& name, Int32& value, bool restore)
{
    if (restore)
    {
        node->GetAttribute(name, value);
    }
    else
    {
        std::stringstream ss;
        ss << value;
        std::string buf = ss.str();
        node->SetAttribute(name, buf);
    }
}

void XMLSerialize::SerializeAttribute(XMLElement* node,
        const std::string& name, Int64& value, bool restore)
{
    if (restore)
    {
        std::string strValue;
        bool result = node->GetAttribute(name, strValue);
        if (result)
        {
            //value = atoll(v);
            value = atoll(strValue.c_str());
        }
    }
    else
    {
        std::stringstream ss;
        ss << value;
        std::string buf = ss.str();
        node->SetAttribute(name, buf);
    }
}

void XMLSerialize::SerializeAttribute(XMLElement* node,
        const std::string& name, std::string& value, bool restore)
{
    if (restore)
    {
        node->GetAttribute(name, value);
    }
    else
    {
        node->SetAttribute(name, value);
    }
}

void XMLSerialize::SerializeAttribute(XMLElement* node,
        const std::string& name, XMLEnumerable& value, bool restore)
{
    if (restore)
    {
        std::string strValue;
        bool result = node->GetAttribute(name, strValue);
        if (result)
        {
            value.Serialize(strValue, restore);
        }
    }
    else
    {
        std::string str;
        value.Serialize(str, restore);
        node->SetAttribute(name, str);
    }
}

void XMLSerialize::SerializeComplex(XMLElement* node, const std::string& name,
        XMLSerialize& value, bool restore)
{
    if (restore)
    {
        if (XMLElement* subNode = node->GetFirstChildElement(name))
        {
            value.Serialize(subNode, restore);
        }
    }
    else
    {
        XMLElement* subNode = new XMLElement(name);
        value.Serialize(subNode, restore);
        node->LinkEndChild(subNode);
    }
}

void XMLSerialize::SerializeComplex(XMLElement* node, const std::string& name,
        XMLEnumerable& value, bool restore)
{
    if (restore)
    {
        if (XMLElement* subNode = node->GetFirstChildElement(name))
        {
            if (const char* v = subNode->GetText().c_str())
            {
                std::string str = v;
                value.Serialize(str, restore);
            }
        }
    }
    else
    {
        std::string str;
        value.Serialize(str, restore);
        XMLElement* subNode = new XMLElement(name);
        subNode->LinkEndChild(new XMLText(str));
        node->LinkEndChild(subNode);
    }
}

void XMLSerialize::SerializeComplex(XMLElement* node, const std::string& name,
        std::string& value, bool restore)
{
    if (restore)
    {
        if (XMLElement* subNode = node->GetFirstChildElement(name))
        {
            if (const char* v = subNode->GetText().c_str())
            {
                value = v;
            }
        }
    }
    else
    {
        XMLElement* subNode = new XMLElement(name);
        subNode->LinkEndChild(new XMLText(value));
        node->LinkEndChild(subNode);
    }
}

void XMLSerialize::SerializeComplex(XMLElement* node, const std::string& name,
        std::vector<std::string>& value, bool restore)
{
    if (restore)
    {
        value.clear();
        for (XMLElement* subNode = node->GetFirstChildElement(name); subNode
                != NULL; subNode = subNode->GetNextSiblingElement(name))
        {
            value.push_back(subNode->GetText());
        }
    }
    else
    {
        for (std::vector<std::string>::iterator iter = value.begin(); iter
                != value.end(); ++iter)
        {
            XMLElement* subNode = new XMLElement(name);
            subNode->LinkEndChild(new XMLText((*iter)));
            node->LinkEndChild(subNode);
        }
    }
}

void XMLSerialize::SerializeComplex(XMLElement* node, const std::string& name,
        std::vector<int>& value, bool restore)
{
    if (restore)
    {
        value.clear();
        for (XMLElement* subNode = node->GetFirstChildElement(name); subNode
                != NULL; subNode = subNode->GetNextSiblingElement(name))
        {
            std::string tempStr = subNode->GetText();
            if(tempStr.size() > 0)
            {
                int eachValue = atoi(tempStr.c_str());
                value.push_back(eachValue);
            }
        }
    }
    else
    {
        for (std::vector<int>::iterator iter = value.begin(); iter
                != value.end(); ++iter)
        {
            std::stringstream ss;
            ss << (*iter);
            std::string buf = ss.str();

            XMLElement* subNode = new XMLElement(name);
            subNode->LinkEndChild(new XMLText(buf));
            node->LinkEndChild(subNode);
        }
    }
}

void XMLSerialize::SerializeComplex(XMLElement* node, const std::string& name,
        bool& value, bool restore)
{
    if (restore)
    {
        if (XMLElement* subNode = node->GetFirstChildElement(name))
        {
            int temp = atoi(subNode->GetText().c_str());
            if (temp == 0)
            {
                value = false;
            }
            else
            {
                value = true;
            }
        }
    }
    else
    {
        XMLElement* subNode = new XMLElement(name);
        subNode->LinkEndChild(new XMLText(value ? "1" : "0"));
        node->LinkEndChild(subNode);
    }
}

void XMLSerialize::SerializeComplex(XMLElement* node, const std::string& name,
        Int8& value, bool restore)
{
    SerializeComplex(node, name, (Int32 &)value, restore);
}

void XMLSerialize::SerializeComplex(XMLElement* node, const std::string& name,
        Int16& value, bool restore)
{
    SerializeComplex(node, name, (Int32 &)value, restore);
}

void XMLSerialize::SerializeComplex(XMLElement* node, const std::string& name,
        Int32& value, bool restore)
{
    if (restore)
    {
        if (XMLElement* subNode = node->GetFirstChildElement(name))
        {
            value = atoi(subNode->GetText().c_str());
        }
    }
    else
    {
        std::stringstream ss;
        ss << value;
        std::string buf = ss.str();

        XMLElement* subNode = new XMLElement(name);
        subNode->LinkEndChild(new XMLText(buf));
        node->LinkEndChild(subNode);
    }
}

void XMLSerialize::SerializeComplex(XMLElement* node, const std::string& name,
        Int64& value, bool restore)
{
    if (restore)
    {
        if (XMLElement* subNode = node->GetFirstChildElement(name))
        {
            value = atoll(subNode->GetText().c_str());
        }
    }
    else
    {
        std::stringstream ss;
        ss << value;
        std::string buf = ss.str();

        XMLElement* subNode = new XMLElement(name);
        subNode->LinkEndChild(new XMLText(buf));
        node->LinkEndChild(subNode);
    }
}

void XMLSerialize::SerializeComplex(XMLElement* node, const std::string& name,
        std::map<std::string, std::string>& value, bool restore)
{
    if (restore)
    {
        for (XMLElement* subNode = node->GetFirstChildElement(name); subNode
                != NULL; subNode = subNode->GetNextSiblingElement(name))
        {
            std::string id;
            subNode->GetAttribute("id", id);
            value[id] = subNode->GetText();
        }
    }
    else
    {
        for (std::map<std::string, std::string>::iterator iter = value.begin();
                iter != value.end(); ++iter)
        {
            XMLElement* subNode = new XMLElement(name);
            std::string id = iter->first;
            XMLSerialize::SerializeAttribute(subNode, "id", id, restore);
            subNode->SetValue(iter->second);
            node->LinkEndChild(subNode);
        }
    }
}

