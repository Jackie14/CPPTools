//////////////////////////////////////////////////////////////////////////
// JSONParser.h
//////////////////////////////////////////////////////////////////////////

#pragma once


#include <vector>
#include <string>
#include <map>

//////////////////////////////////////////////////////////////////////////
enum JSONType 
{ 
    JSONTypeNull, 
    JSONTypeString,
    JSONTypeBool,
    JSONTypeDouble, 
    JSONTypeArray,
    JSONTypeObject
};

//////////////////////////////////////////////////////////////////////////
class JSONValue;
typedef std::vector<JSONValue*> JSONArray;
typedef std::map<std::string, JSONValue*> JSONObject;

//////////////////////////////////////////////////////////////////////////
class JSONValue
{
    friend class JSONParser; 

public:
    JSONValue();
    // Basic constructor for creating a JSON Value of mType String
    JSONValue(const char* charValue);
    // Basic constructor for creating a JSON Value of mType String
    JSONValue(std::string stringValue);
    // Basic constructor for creating a JSON Value of mType bool
    JSONValue(bool boolValue);
    // Basic constructor for creating a JSON Value of mType double
    JSONValue(double doubleValue);
    // Basic constructor for creating a JSON Value of mType Array
    JSONValue(JSONArray arrayValue);
    // Basic constructor for creating a JSON Value of mType Object
    JSONValue(JSONObject objectValue);
    ~JSONValue();

    // Checks if the value is a NULL
    bool IsNull() const;

    //Checks if the value is a String
    bool IsString() const;

    // Checks if the value is a bool
    bool IsBool() const;

    // Checks if the value is a double
    bool IsDouble() const;

    // Checks if the value is an Array
    bool IsArray() const;

    // Checks if the value is an Object
    bool IsObject() const;

    // Retrieves the String value of this JSONValue
    // Use IsString() before using this method.
    std::string AsString() const;

    // Retrieves the bool value of this JSONValue
    // Use IsBool() before using this method.
    bool AsBool() const;

    // Retrieves the double value of this JSONValue
    // Use IsDouble() before using this method.
    double AsDouble() const;

    // Retrieves the Array value of this JSONValue
    // Use IsArray() before using this method.
    JSONArray AsArray() const;

    // Retrieves the Object value of this JSONValue
    // Use IsObject() before using this method.
    JSONObject AsObject() const;

    // Creates a JSON encoded string for the value with all necessary characters escaped
    // Returns the JSON string
    std::string ToString() const;

protected:
    // Parses a JSON encoded value to a JSONValue object
    // data: Pointer to a wchar_t* that contains the data
    // Returns a pointer to a JSONValue object on success, NULL on error
    static JSONValue* Parse(const char** data);

private:
    // Creates a JSON encoded string with all required fields escaped
    static std::string StringifyString(std::string str);

    JSONType mType;
    std::string mStringValue;
    bool mBoolValue;
    double mDoubleValue;
    JSONArray mArrayValue;
    JSONObject mObjectValue;
};

//////////////////////////////////////////////////////////////////////////
class JSONParser
{
    friend class JSONValue;

public:
    // Parses a complete JSON encoded string
    // data: The JSON text
    // Returns a JSON Value representing the root, or NULL on error
    static JSONValue* Parse(const char* data);

    // Turns the passed in JSONValue into a JSON encode string
    // value: The root value
    // Returns a JSON encoded string representation of the given value
    static std::string ToString(JSONValue* value);

protected:
    // Skips over any whitespace characters (space, tab, \r or \n) defined by the JSON spec
    // data: Pointer to a wchar_t* that contains the JSON text
    // Returns true if there is more data, or false if the end of the text was reached
    static bool SkipWhitespace(const char** data);

    // Extracts a JSON String as defined by the spec
    // Any escaped characters are swapped out for their un-escaped values
    // data: Pointer to a wchar_t* that contains the JSON text
    // str: Reference to a std::wstring to receive the extracted string
    // Returns true on success, false on failure
    static bool ExtractString(const char** data, std::string& str);

    // Parses some text as though it is an integer
    // data: Pointer to a wchar_t* that contains the JSON text
    // Returns the int value of the number found
    static int ParseInt(const char** data);

private:
    JSONParser();
};
