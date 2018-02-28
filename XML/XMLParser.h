//////////////////////////////////////////////////////////////////////////
// XMLParser.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef XMLParser_INCLUDED
#define XMLParser_INCLUDED

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <iostream>
#include <sstream>
#include <string>

class XMLDoc;
class XMLElement;
class XMLComment;
class XMLUnknown;
class XMLAttribute;
class XMLText;
class XMLDeclaration;
class XMLParsingData;

//////////////////////////////////////////////////////////////////////////
// Internal structure for tracking location of items in the XML file.
struct XMLCursor
{
    XMLCursor()
    {
        Clear();
    }
    void Clear()
    {
        row = col = -1;
    }

    int row; // 0 based.
    int col; // 0 based.
};

//////////////////////////////////////////////////////////////////////////
// If you call the Accept() method, it requires being passed a XMLVisitor
// class to handle callbacks. For nodes that contain other nodes (Document, Element)
// you will get called with a VisitEnter/VisitExit pair. Nodes that are always leaves
// are simple called with Visit().
// If you return 'true' from a Visit method, recursive parsing will continue. If you return
// false, no children of this node or its siblings will be Visited.
// All flavors of Visit methods have a default implementation that returns 'true' (continue 
// visiting). You need to only override methods that are interesting to you.
// Generally Accept() is called on the XMLDoc, although all nodes support Visiting.
// You should never change the document from a callback.
class XMLVisitor
{
public:
    virtual ~XMLVisitor()
    {
    }

    // Visit a document.
    virtual bool VisitEnter(const XMLDoc& /*doc*/)
    {
        return true;
    }
    // Visit a document.
    virtual bool VisitExit(const XMLDoc& /*doc*/)
    {
        return true;
    }

    // Visit an element.
    virtual bool VisitEnter(const XMLElement& /*element*/, const XMLAttribute* /*firstAttribute*/)
    {
        return true;
    }
    // Visit an element.
    virtual bool VisitExit(const XMLElement& /*element*/)
    {
        return true;
    }

    // Visit a declaration
    virtual bool Visit(const XMLDeclaration& /*declaration*/)
    {
        return true;
    }
    // Visit a text node
    virtual bool Visit(const XMLText& /*text*/)
    {
        return true;
    }
    // Visit a comment node
    virtual bool Visit(const XMLComment& /*comment*/)
    {
        return true;
    }
    // Visit an unknown node
    virtual bool Visit(const XMLUnknown& /*unknown*/)
    {
        return true;
    }
};

//////////////////////////////////////////////////////////////////////////
// Only used by Attribute::Query functions
enum
{
    XML_SUCCESS, XML_NO_ATTRIBUTE, XML_WRONG_TYPE
};

// Used by the parsing routines.
enum XMLEncoding
{
    XMLEncodingUnknown, XMLEncodingUTF8, XMLEncodingLegacy
};

const XMLEncoding XMLEncodingDefault = XMLEncodingUnknown;

//////////////////////////////////////////////////////////////////////////
// XMLBase is a base class for every class.
// It does little except to establish that classes
// can be printed and provide some utility functions.
// In XML, the document and elements can contain
// other elements and other types of nodes.
// A Document can contain: Element (container or leaf)
//						Comment (leaf)
//						Unknown (leaf)
//						Declaration( leaf )
// An Element can contain: Element (container or leaf)
//						Text (leaf)
//						Attributes (not on tree)
//						Comment (leaf)
//						Unknown (leaf)
// A Deceleration contains: Attributes (not on tree)
class XMLBase
{
    friend class XMLNode;
    friend class XMLElement;
    friend class XMLDoc;

public:
    XMLBase() :
        mUserData(0)
    {
    }
    virtual ~XMLBase()
    {
    }

    // All XML classes can print themselves to a file stream
    // Either or both cfile and str can be null.
    // This is a formatted print, and will insert tabs and newlines.
    // (For an unformatted stream, use the << operator.)
    virtual void Print(FILE* cfile, int depth) const = 0;

    // The world does not agree on whether white space should be kept or
    // not. In order to make everyone happy, these global, static functions
    // are provided to set whether or not XML will condense all white space
    // into a single space or not. The default is to condense. Note changing this
    // value is not thread safe.
    static void SetCondenseWhiteSpace(bool condense)
    {
        mCondenseWhiteSpace = condense;
    }

    // Return the current white space setting.
    static bool IsWhiteSpaceCondensed()
    {
        return mCondenseWhiteSpace;
    }

    // Return the position, in the original source file, of this node or attribute.
    // The row and column are 1-based. (That is the first row and first column is
    // 1,1). If the returns values are 0 or less, then the parser does not have
    // a row and column value.
    // Generally, the row and column value will be set when the XMLDoc::Load(),
    // XMLDoc::LoadFile(), or any XMLNode::Parse() is called. It will NOT be set
    // when the DOM was created from operator>>.
    // The values reflect the initial load. Once the DOM is modified programmatical
    // (by adding or changing nodes and attributes) the new values will NOT update to
    // reflect changes in the document.
    // There is a minor performance cost to computing the row and column. Computation
    // can be disabled if XMLDoc::SetTabSize() is called with 0 as the value.
    int GetRow() const;
    int GetColumn() const;

    // Set a pointer to arbitrary user data.
    void SetUserData(void* user);
    // Get a pointer to arbitrary user data.
    void* GetUserData();
    // Get a pointer to arbitrary user data.
    const void* GetUserData() const;

    // Table that returns, for a given lead byte, the total number of bytes
    // in the UTF-8 sequence.
    static const int mUTF8ByteTable[256];

    virtual const char* Parse(const char* p, XMLParsingData* data,
            XMLEncoding encoding /*= XMLEncodingUnknown */) = 0;

    // Expands entities in a string. Note this should not contain the tag's '<', '>', etc, 
    // or they will be transformed into entities! 
    static void EncodeString(const std::string& str, std::string* out);

    enum
    {
        XML_NO_ERROR = 0,
        XML_ERROR,
        XML_ERROR_OPENING_FILE,
        XML_ERROR_OUT_OF_MEMORY,
        XML_ERROR_PARSING_ELEMENT,
        XML_ERROR_FAILED_TO_READ_ELEMENT_NAME,
        XML_ERROR_READING_ELEMENT_VALUE,
        XML_ERROR_READING_ATTRIBUTES,
        XML_ERROR_PARSING_EMPTY,
        XML_ERROR_READING_END_TAG,
        XML_ERROR_PARSING_UNKNOWN,
        XML_ERROR_PARSING_COMMENT,
        XML_ERROR_PARSING_DECLARATION,
        XML_ERROR_DOCUMENT_EMPTY,
        XML_ERROR_EMBEDDED_NULL,
        XML_ERROR_PARSING_CDATA,
        XML_ERROR_DOCUMENT_TOP_ONLY,

        XML_ERROR_STRING_COUNT
    };

protected:
    static const char* SkipWhiteSpace(const char*, XMLEncoding encoding);
    static bool IsWhiteSpace(char c);
    static bool IsWhiteSpace(int c);

    static bool StreamWhiteSpace(std::istream* in, std::string* tag);
    static bool StreamTo(std::istream* in, int character, std::string* tag);

    // Reads an XML name into the string provided. Returns
    // a pointer just past the last character of the name,
    // or 0 if the function has an error. 
    static const char* ReadName(const char* p, std::string* name,
            XMLEncoding encoding);

    // Reads text. Returns a pointer past the given end tag.
    // Wickedly complex options, but it keeps the (sensitive) code in one place.
    static const char* ReadText(const char* in, // where to start
            std::string* text, // the string read
            bool ignoreWhiteSpace, // whether to keep the white space
            const char* endTag, // what ends this text
            bool ignoreCase, // whether to ignore case in the end tag
            XMLEncoding encoding); // the current encoding

    // If an entity has been found, transform it into a character.
    static const char* GetEntity(const char* in, char* value, int* length,
            XMLEncoding encoding);

    // Get a character, while interpreting entities.
    // The length can be from 0 to 4 bytes.
    static const char* GetChar(const char* p, char* value, int* length,
            XMLEncoding encoding);

    // Return true if the next characters in the stream are any of the endTag sequences.
    // Ignore case only works for English, and should only be relied on when comparing
    // to English words: StringEqual( p, "version", true ) is fine.
    static bool StringEqual(const char* p, const char* endTag, bool ignoreCase,
            XMLEncoding encoding);

    static const char* mErrorString[XML_ERROR_STRING_COUNT];

    XMLCursor mLocation;

    // Field containing a generic user pointer
    void* mUserData;

    // None of these methods are reliable for any language except English.
    // Good for approximation, not great for accuracy.
    static int IsAlpha(unsigned char anyByte, XMLEncoding encoding);
    static int IsAlphaNum(unsigned char anyByte, XMLEncoding encoding);
    static int ToLower(int v, XMLEncoding encoding);
    static void ConvertUTF32ToUTF8(unsigned long input, char* output,
            int* length);

private:
    XMLBase(const XMLBase&); // not implemented.
    void operator=(const XMLBase& base); // not allowed.

    struct Entity
    {
        const char* str;
        unsigned int strLength;
        char chr;
    };
    enum
    {
        NUM_ENTITY = 5, MAX_ENTITY_LENGTH = 6

    };
    static Entity mEntity[NUM_ENTITY];
    static bool mCondenseWhiteSpace;
};

//////////////////////////////////////////////////////////////////////////
// The parent class for everything in the Document Object Model.
// (Except for attributes).
// Nodes have siblings, a parent, and children. A node can be
// in a document, or stand on its own. The type of a XMLNode
// can be queried, and it can be cast to its more defined type.
class XMLNode: public XMLBase
{
    friend class XMLDoc;
    friend class XMLElement;

public:
    // An input stream operator, for every class. Tolerant of newlines and
    // formatting, but doesn't expect them.
    friend std::istream& operator >>(std::istream& in, XMLNode& base);

    // An output stream operator, for every class. Note that this outputs
    // without any newlines or formatting, as opposed to Print(), which
    // includes tabs and new lines.
    // The operator<< and operator>> are not completely symmetric. Writing
    // a node to a stream is very well defined. You'll get a nice stream
    // of output, without any extra whitespace or newlines.
    // But reading is not as well defined. (As it always is.) If you create
    // a XMLElement (for example) and read that from an input stream,
    // the text needs to define an element or junk will result. This is
    // true of all input streams, but it's worth keeping in mind.
    // A XMLDoc will read nodes until it reads a root element, and
    // all the children of that root element.
    friend std::ostream& operator<<(std::ostream& out, const XMLNode& base);

    // Appends the XML node or attribute to a std::string.
    friend std::string& operator<<(std::string& out, const XMLNode& base);

    // The types of XML nodes supported by XML. (All the
    // unsupported types are picked up by UNKNOWN.)
    enum NodeType
    {
        DOCUMENT, ELEMENT, COMMENT, UNKNOWN, TEXT, DECLARATION, TYPECOUNT
    };

    virtual ~XMLNode();

    // The meaning of 'value' changes for the specific type of XMLNode.
    // Document: filename of the xml file
    // Element: name of the element
    // Comment: the comment text
    // Unknown: the tag contents
    // Text: the text string
    // The subclasses will wrap this function.
    std::string GetValue() const;

    // Changes the value of the node. Defined as:
    // Document: filename of the xml file
    // Element:	name of the element
    // Comment:	the comment text
    // Unknown:	the tag contents
    // Text: the text string
    void SetValue(const std::string& value);

    // Delete all the children of this node. Does not affect 'this'.
    void Clear();

    // One step up the DOM.
    XMLNode* GetParent() const;

    // Return the number of child nodes.
    long GetNumberOfChildNodes() const;

    // The first child of this node. Will be null if there are no children.
    XMLNode* GetFirstChild() const;
    // The first child of this node with the matching 'value'. Will be null if none found.
    XMLNode* GetFirstChild(const std::string& value) const;
    // The last child of this node. Will be null if there are no children.
    XMLNode* GetLastChild() const;
    // The last child of this node matching 'value'. Will be null if there are no children.
    XMLNode* GetLastChild(const std::string& value) const;

    // An alternate way to walk the children of a node.
    // One way to iterate over nodes is:
    // for(child = parent->FirstChild(); child; child = child->NextSibling())
    // IterateChildren does the same thing with the syntax:
    // child = 0;
    // while(child = parent->IterateChildren(child))
    // IterateChildren takes the previous child as input and finds
    // the next one. If the previous child is null, it returns the
    // first. IterateChildren will return null when done.
    XMLNode* IterateChildren(const XMLNode* previous) const;
    // This flavor of IterateChildren searches for children with a particular 'value'
    XMLNode* IterateChildren(const std::string& value, const XMLNode* previous) const;

    // Add a new node related to this. Adds a child past the LastChild.
    // Returns a pointer to the new object or NULL if an error occurred.
    XMLNode* InsertEndChild(const XMLNode& addThis);

    // Add a new node related to this. Adds a child past the LastChild.
    // NOTE: the node to be added is passed by pointer, and will be
    // henceforth owned (and deleted) by XML. This method is efficient
    // and avoids an extra copy, but should be used with care as it
    // uses a different memory model than the other insert functions.
    XMLNode* LinkEndChild(XMLNode* addThis);

    // Add a new node related to this. Adds a child before the specified child.
    // Returns a pointer to the new object or NULL if an error occurred.
    XMLNode* InsertBeforeChild(XMLNode* beforeThis, const XMLNode& addThis);

    // Add a new node related to this. Adds a child after the specified child.
    // Returns a pointer to the new object or NULL if an error occurred.
    XMLNode* InsertAfterChild(XMLNode* afterThis, const XMLNode& addThis);

    // Replace a child of this node.
    //Returns a pointer to the new object or NULL if an error occurred.
    XMLNode* ReplaceChild(XMLNode* replaceThis, const XMLNode& withThis);

    // Delete a child of this node.
    bool RemoveChild(XMLNode* removeThis);

    // Navigate to a sibling node.
    XMLNode* GetPreviousSibling() const;
    // Navigate to a sibling node.
    XMLNode* GetPreviousSibling(const std::string& value) const;

    // Navigate to a sibling node with the given 'value'.
    XMLNode* GetNextSibling(const std::string& value) const;
    // Navigate to a sibling node.
    XMLNode* GetNextSibling() const;

    int GetNumberOfChildElements() const;

    // Convenience function to get through elements.
    // Calls NextSibling and ToElement. Will skip all non-Element nodes. 
    // Returns 0 if there is not another element.
    XMLElement* GetNextSiblingElement() const;
    // Convenience function to get through elements.
    // Calls NextSibling and ToElement. Will skip all non-Element nodes. 
    // Returns 0 if there is not another element.
    XMLElement* GetNextSiblingElement(const std::string& value) const;

    // Convenience function to get through elements.
    XMLElement* GetFirstChildElement() const;
    // Convenience function to get through elements.
    XMLElement* GetFirstChildElement(const std::string& value) const;

    // Query the type (as an enumerated value, above) of this node.
    // The possible types are: DOCUMENT, ELEMENT, COMMENT, UNKNOWN, TEXT, and DECLARATION. 
    int GetType() const;

    // Return a pointer to the Document this node lives in.
    // Returns null if not in a document.
    XMLDoc* GetDocument() const;

    // Returns true if this node has children.
    bool HasChildren() const;

    // Cast to a more defined type. Will return null if not of the requested type.
    virtual XMLDoc* ToDocument() const
    {
        return 0;
    }
    virtual XMLElement* ToElement() const
    {
        return 0;
    }
    virtual XMLComment* ToComment() const
    {
        return 0;
    }
    virtual XMLUnknown* ToUnknown() const
    {
        return 0;
    }
    virtual XMLText* ToText() const
    {
        return 0;
    }
    virtual XMLDeclaration* ToDeclaration() const
    {
        return 0;
    }

    // Create an exact duplicate of this node and return it. The memory must be deleted by the caller. 
    virtual XMLNode* Clone() const = 0;

    // Accept a hierarchical visit the nodes in the XML DOM. Every node in the 
    // XML tree will be conditionally visited and the host will be called back
    // via the XMLVisitor interface.
    // The interface has been based on ideas from:
    // - http://www.saxproject.org/
    // - http://c2.com/cgi/wiki?HierarchicalVisitorPattern 
    // Which are both good references for "visiting".
    // An example of using Accept():
    // XMLPrinter printer;
    // xmlDoc.Accept(&printer);
    // const char* xmlcstr = printer.CStr();
    virtual bool Accept(XMLVisitor* visitor) const = 0;

protected:
    XMLNode(NodeType type);

    // Copy to the allocated object. Shared functionality between Clone, Copy constructor,
    // and the assignment operator.
    void CopyTo(XMLNode* target) const;

    // The real work of the input operator.
    virtual void StreamIn(std::istream* in, std::string* tag) = 0;

    // Figure out what is at *p, and parse it. Returns null if it is not an xml node.
    XMLNode* Identify(const char* start, XMLEncoding encoding);

    XMLNode* mParent;
    NodeType mType;

    XMLNode* mFirstChild;
    XMLNode* mLastChild;

    std::string mValue;

    XMLNode* mPreviousNode;
    XMLNode* mNextNode;

private:
    XMLNode(const XMLNode&); // not implemented.
    void operator=(const XMLNode& base); // not allowed.
};

//////////////////////////////////////////////////////////////////////////
// An attribute is a name-value pair. Elements have an arbitrary
// number of attributes, each with a unique name.
// The attributes are not XMLNodes, since they are not
// part of the XML document object model. There are other
// suggested ways to look at this problem.
class XMLAttribute: public XMLBase
{
    friend class XMLAttributeSet;

public:
    // Construct an empty attribute.
    XMLAttribute();
    // Construct an attribute with a name and value.
    XMLAttribute(const std::string& name, const std::string& value);

    // Return the value of this attribute.
    std::string GetValue() const;
    // Return the name of this attribute.
    std::string GetName() const;

    // Return the value of this attribute, converted to an integer.
    int GetValueInt() const;
    // Return the value of this attribute, converted to a double. 
    double GetValueDouble() const;

    // QueryIntValue examines the value string. It is an alternative to the
    // IntValue() method with richer error checking.
    // If the value is an integer, it is stored in 'value' and 
    // the call returns XML_SUCCESS. If it is not
    // an integer, it returns XML_WRONG_TYPE.
    // A specialized but useful call. Note that for success it returns 0,
    // which is the opposite of almost all other XML calls.
    int QueryValue(int& result) const;
    int QueryValue(double& result) const;
    int QueryValue(std::string& result) const;

    // Set the name of this attribute.
    void SetName(const std::string& name);

    // Set the value.
    void SetValue(const std::string& value);
    // Set the value from an integer.
    void SetValue(int value);
    // Set the value from a double.
    void SetValue(double value);

    // Get the next sibling attribute in the DOM. Returns null at end.
    XMLAttribute* GetNext() const;

    // Get the previous sibling attribute in the DOM. Returns null at beginning.
    XMLAttribute* GetPrevious() const;

    bool operator==(const XMLAttribute& rhs) const
    {
        return rhs.mName == mName;
    }
    bool operator<(const XMLAttribute& rhs) const
    {
        return mName < rhs.mName;
    }
    bool operator>(const XMLAttribute& rhs) const
    {
        return mName > rhs.mName;
    }

    // Attribute parsing starts: first letter of the name
    // returns: the next char after the value end quote
    virtual const char* Parse(const char* p, XMLParsingData* data,
            XMLEncoding encoding);

    // Prints this Attribute to a FILE stream.
    virtual void Print(FILE* cfile, int depth) const
    {
        Print(cfile, depth, 0);
    }
    void Print(FILE* cfile, int depth, std::string* str) const;

    // Set the document pointer so the attribute can report errors.
    void SetDocument(XMLDoc* doc);

private:
    XMLAttribute(const XMLAttribute&); // not implemented.
    void operator=(const XMLAttribute& base); // not allowed.

    // A pointer back to a document, for error reporting. 
    XMLDoc* mDocument;
    std::string mName;
    std::string mValue;
    XMLAttribute* mPreviousAttribute;
    XMLAttribute* mNextAttribute;
};

//////////////////////////////////////////////////////////////////////////
// A class used to manage a group of attributes.
// It is only used internally, both by the ELEMENT and the DECLARATION.
// The set can be changed transparent to the Element and Declaration
// classes that use it, but NOT transparent to the Attribute
// which has to implement a next() and previous() method. Which makes
// it a bit problematic and prevents the use of STL.
// This version is implemented with circular lists because:
// - I like circular lists
// - it demonstrates some independence from the (typical) doubly linked list.
class XMLAttributeSet
{
public:
    XMLAttributeSet();
    ~XMLAttributeSet();

    void Add(XMLAttribute* attribute);
    void Remove(XMLAttribute* attribute);

    XMLAttribute* GetFirst() const
    {
        return (mSentinel.mNextAttribute == &mSentinel) ? 0
                : mSentinel.mNextAttribute;
    }
    XMLAttribute* GetLast() const
    {
        return (mSentinel.mPreviousAttribute == &mSentinel) ? 0
                : mSentinel.mPreviousAttribute;
    }

    XMLAttribute* Find(const std::string& name) const;

private:
    // Because of hidden/disabled copy-constructor in XMLAttribute (sentinel-element),
    // this class must be also use a hidden/disabled copy-constructor !!!
    XMLAttributeSet(const XMLAttributeSet&); // not allowed
    void operator=(const XMLAttributeSet&); // not allowed (as XMLAttribute)

    XMLAttribute mSentinel;
};

//////////////////////////////////////////////////////////////////////////
// The element is a container class. It has a value, the element name,
// and can contain other elements, text, comments, and unknowns.
// Elements also contain an arbitrary number of attributes.
class XMLElement: public XMLNode
{
public:
    // Construct an element.
    XMLElement(const std::string& value);
    XMLElement(const XMLElement&);

    void operator=(const XMLElement& base);

    virtual ~XMLElement();

    // Given an attribute name, get the value for the attribute of that name
    // return false if none exists.
    bool GetAttribute(const std::string& name, std::string& result,
            const std::string& defaultValue = "") const;
    bool GetAttribute(const std::string& name, int& result, int defaultValue =
            0) const;
    bool GetAttribute(const std::string& name, double& result,
            double defaultValue = 0.0) const;

    // QueryAttribute examines the attribute - it is an alternative to the
    // GetAttribute() method with richer error checking.
    // If the attribute is an integer, it is stored in 'value' and 
    // the call returns XML_SUCCESS. If it is not
    // an integer, it returns XML_WRONG_TYPE. If the attribute
    // does not exist, then XML_NO_ATTRIBUTE is returned.
    int QueryAttribute(const std::string& name, std::string& result,
            const std::string& defaultValue = "") const;
    int QueryAttribute(const std::string& name, int& result, int defaultValue =
            0) const;
    int QueryAttribute(const std::string& name, double& result,
            double defaultValue = 0.0) const;

    // Template form of the attribute query which will try to read the
    // attribute into the specified type. Very easy, very powerful, but
    // be careful to make sure to call this with the correct type.
    // NOTE: This method doesn't work correctly for 'string' types.
    // @return XML_SUCCESS, XML_WRONG_TYPE, or XML_NO_ATTRIBUTE
    template<typename T>
    int QueryAttribute(const std::string& name, T* result, T defaultValue = 0) const
    {
        const XMLAttribute* node = mAttributeSet.Find(name);
        if (!node)
        {
            result = defaultValue;
            return XML_NO_ATTRIBUTE;
        }

        std::stringstream sstream(node->GetValue());
        sstream >> *result;
        if (!sstream.fail())
        {
            return XML_SUCCESS;
        }
        return XML_WRONG_TYPE;
    }

    // Sets an attribute of name to a given value. 
    // The attribute will be created if it does not exist, or changed if it does.
    void SetAttribute(const std::string& name, const std::string& value);
    void SetAttribute(const std::string& name, int value);
    void SetAttribute(const std::string& name, double value);

    // Deletes an attribute with the given name.
    void RemoveAttribute(const std::string& name);

    // Access the first attribute in this element.
    XMLAttribute* FirstAttribute() const
    {
        return mAttributeSet.GetFirst();
    }
    // Access the last attribute in this element.
    XMLAttribute* LastAttribute() const
    {
        return mAttributeSet.GetLast();
    }

    // Convenience function for easy access to the text inside an element. Although easy
    // and concise, GetText() is limited compared to getting the XMLText child
    // and accessing it directly.
    // If the first child of 'this' is a XMLText, the GetText()
    // returns the character string of the Text node, else null is returned.
    // This is a convenient method for getting the text of simple contained text:
    // <foo>This is text</foo>
    // const char* str = fooElement->GetText();
    // 'str' will be a pointer to "This is text". 
    // Note that this function can be misleading. If the element foo was created from this XML:
    // <foo>This is text</foo> 
    // then the value of str would be null. The first child node isn't a text node, it is
    // another element. From this XML:
    // <foo>This is <b>text</b></foo> 
    // GetText() will return "This is ".
    // WARNING: GetText() accesses a child node - don't become confused with the 
    // similarly named XMLHandle::Text() and XMLNode::ToText() which are 
    // safe type casts on the referenced node.
    std::string GetText() const;

    // Creates a new Element and returns it - the returned element is a copy.
    virtual XMLNode* Clone() const;
    // Print the Element to a FILE stream.
    virtual void Print(FILE* cfile, int depth) const;

    // Attribute parsing starts: next char past '<'
    // returns: next char past '>'
    virtual const char* Parse(const char* p, XMLParsingData* data,
            XMLEncoding encoding);

    // Cast to a more defined type. Will return null not of the requested type. 
    virtual XMLElement* ToElement() const
    {
        return const_cast<XMLElement*> (this);
    }

    // Walk the XML tree visiting this node and all of its children. 
    virtual bool Accept(XMLVisitor* visitor) const;

protected:
    void CopyTo(XMLElement* target) const;
    // like clear, but initializes 'this' object as well
    void ClearThis();

    virtual void StreamIn(std::istream* in, std::string* tag);
    // Reads the "value" of the element -- another element, or text.
    // This should terminate with the current end tag.
    const char* ReadValue(const char* in, XMLParsingData* prevData,
            XMLEncoding encoding);

private:
    XMLAttributeSet mAttributeSet;
};

//////////////////////////////////////////////////////////////////////////
// An XML comment. 
class XMLComment: public XMLNode
{
public:
    // Constructs an empty comment.
    XMLComment() :
        XMLNode(XMLNode::COMMENT)
    {
    }
    // Construct a comment from text.
    XMLComment(const std::string& value) :
        XMLNode(XMLNode::COMMENT)
    {
        SetValue(value);
    }
    XMLComment(const XMLComment&);
    void operator=(const XMLComment& base);

    virtual ~XMLComment()
    {
    }

    // Returns a copy of this Comment.
    virtual XMLNode* Clone() const;
    // Write this Comment to a FILE stream.
    virtual void Print(FILE* cfile, int depth) const;

    // Attribute parsing starts: at the ! of the !--
    // returns: next char past '>'
    virtual const char* Parse(const char* p, XMLParsingData* data,
            XMLEncoding encoding);

    // Cast to a more defined type. Will return null not of the requested type.
    virtual XMLComment* ToComment() const
    {
        return const_cast<XMLComment*> (this);
    }

    // Walk the XML tree visiting this node and all of its children. 
    virtual bool Accept(XMLVisitor* visitor) const;

protected:
    void CopyTo(XMLComment* target) const;
    virtual void StreamIn(std::istream* in, std::string* tag);

private:
};

//////////////////////////////////////////////////////////////////////////
// XML text. A text node can have 2 ways to output the next. "normal" output 
// and CDATA. It will default to the mode it was parsed from the XML file and
// you generally want to leave it alone, but you can change the output mode with 
// SetCDATA() and query it with CDATA().
class XMLText: public XMLNode
{
    friend class XMLElement;
public:
    // Constructor for text element. By default, it is treated as 
    // normal, encoded text. If you want it be output as a CDATA text
    // element, set the parameter _cdata to 'true'
    XMLText(const std::string& value) :
        XMLNode(XMLNode::TEXT)
    {
        SetValue(value);
        mIsCDATA = false;
    }
    virtual ~XMLText()
    {
    }

    XMLText(const XMLText& copy) :
        XMLNode(XMLNode::TEXT)
    {
        copy.CopyTo(this);
        this->mIsCDATA = copy.mIsCDATA;
    }
    void operator=(const XMLText& base)
    {
        base.CopyTo(this);
    }

    // Write this text object to a FILE stream.
    virtual void Print(FILE* cfile, int depth) const;

    // Queries whether this represents text using a CDATA section.
    bool CDATA() const
    {
        return mIsCDATA;
    }
    // Turns on or off a CDATA representation of text.
    void SetCDATA(bool _cdata)
    {
        mIsCDATA = _cdata;
    }

    virtual const char* Parse(const char* p, XMLParsingData* data,
            XMLEncoding encoding);

    // Cast to a more defined type. Will return null not of the requested type.
    virtual XMLText* ToText() const
    {
        return const_cast<XMLText*> (this);
    }

    // Walk the XML tree visiting this node and all of its children. 
    virtual bool Accept(XMLVisitor* content) const;

protected:
    virtual XMLNode* Clone() const;
    void CopyTo(XMLText* target) const;

    // returns true if all white space and new lines
    bool IsBlank() const;
    virtual void StreamIn(std::istream* in, std::string* tag);

private:
    // true if this should be input and output as a CDATA style text element
    bool mIsCDATA;
};

//////////////////////////////////////////////////////////////////////////
// In correct XML the declaration is the first entry in the file.
// <?xml version="1.0" standalone="yes"?>
// XML will happily read or write files without a declaration,
// however. There are 3 possible attributes to the declaration:
// version, encoding, and standalone.
// Note: In this version of the code, the attributes are
// handled as special cases, not generic attributes, simply
// because there can only be at most 3 and they are always the same.
class XMLDeclaration: public XMLNode
{
public:
    // Construct an empty declaration.
    XMLDeclaration() :
        XMLNode(XMLNode::DECLARATION)
    {
    }

    // Constructor.
    XMLDeclaration(const std::string& version, const std::string& encoding,
            const std::string& standalone);

    XMLDeclaration(const XMLDeclaration& copy);
    void operator=(const XMLDeclaration& copy);

    virtual ~XMLDeclaration()
    {
    }

    // Version. Will return an empty string if none was found.
    std::string GetVersion() const
    {
        return mVersion;
    }

    // Encoding. Will return an empty string if none was found.
    std::string GetEncoding() const
    {
        return mEncoding;
    }

    // Is this a standalone document?
    std::string GetStandalone() const
    {
        return mStandalone;
    }

    // Creates a copy of this Declaration and returns it.
    virtual XMLNode* Clone() const;
    // Print this declaration to a FILE stream.
    virtual void Print(FILE* cfile, int depth, std::string* str) const;
    virtual void Print(FILE* cfile, int depth) const
    {
        Print(cfile, depth, 0);
    }

    virtual const char* Parse(const char* p, XMLParsingData* data,
            XMLEncoding encoding);

    // Cast to a more defined type. Will return null not of the requested type.
    virtual XMLDeclaration* ToDeclaration() const
    {
        return const_cast<XMLDeclaration*> (this);
    }

    // Walk the XML tree visiting this node and all of its children. 
    virtual bool Accept(XMLVisitor* visitor) const;

protected:
    void CopyTo(XMLDeclaration* target) const;
    // used to be public
    virtual void StreamIn(std::istream* in, std::string* tag);

private:
    std::string mVersion;
    std::string mEncoding;
    std::string mStandalone;
};

//////////////////////////////////////////////////////////////////////////
// Any tag that XML doesn't recognize is saved as an
// unknown. It is a tag of text, but should not be modified.
// It will be written back to the XML, unchanged, when the file is saved.
// DTD tags get thrown into XMLUnknowns.
class XMLUnknown: public XMLNode
{
public:
    XMLUnknown() :
        XMLNode(XMLNode::UNKNOWN)
    {
    }
    virtual ~XMLUnknown()
    {
    }

    XMLUnknown(const XMLUnknown& copy) :
        XMLNode(XMLNode::UNKNOWN)
    {
        copy.CopyTo(this);
    }
    void operator=(const XMLUnknown& copy)
    {
        copy.CopyTo(this);
    }

    /// Creates a copy of this Unknown and returns it.
    virtual XMLNode* Clone() const;
    // Print this Unknown to a FILE stream.
    virtual void Print(FILE* cfile, int depth) const;

    virtual const char* Parse(const char* p, XMLParsingData* data,
            XMLEncoding encoding);

    // Cast to a more defined type. Will return null not of the requested type.
    virtual XMLUnknown* ToUnknown() const
    {
        return const_cast<XMLUnknown*> (this);
    }

    // Walk the XML tree visiting this node and all of its children. 
    virtual bool Accept(XMLVisitor* content) const;

protected:
    void CopyTo(XMLUnknown* target) const;

    virtual void StreamIn(std::istream* in, std::string* tag);

private:
};

//////////////////////////////////////////////////////////////////////////
// Always the top level node. A document binds together all the
// XML pieces. It can be saved, loaded, and printed to the screen.
// The 'value' of a document node is the xml file name.
class XMLDoc: public XMLNode
{
public:
    // Create an empty document, that has no name.
    XMLDoc();
    // Create a document with a name. The name of the document is also the filename of the xml.
    XMLDoc(const std::string& documentName);

    XMLDoc(const XMLDoc& copy);
    void operator=(const XMLDoc& copy);

    virtual ~XMLDoc()
    {
    }

    // Load the document from a string. 
    bool LoadXMLString(const std::string& data, XMLEncoding encoding =
            XMLEncodingDefault);
    // Load a file using the current document value.
    // Returns true if successful. Will delete any existing document data before loading.
    bool LoadFile(XMLEncoding encoding = XMLEncodingDefault);
    // Save a file using the current document value. Returns true if successful.
    bool SaveFile() const;
    // Load a file using the given filename. Returns true if successful.
    bool LoadFile(const std::string& fileName, XMLEncoding encoding =
            XMLEncodingDefault);
    // Save a file using the given filename. Returns true if successful.
    bool SaveFile(const std::string& fileName) const;
    // Load a file using the given FILE*. Returns true if successful. Note that this method
    // doesn't stream - the entire object pointed at by the FILE*
    // will be interpreted as an XML file. XML doesn't stream in XML from the current
    // file location. Streaming may be added in the future.
    bool LoadFile(FILE*, XMLEncoding encoding = XMLEncodingDefault);
    // Save a file using the given FILE*. Returns true if successful.
    bool SaveFile(FILE*) const;

    static FILE* OpenXMLFile(const std::string& fileName, const char* mode);

    // Parse the given null terminated block of xml data. Passing in an encoding to this
    // method (either XMLEncodingLegacy or XMLEncodingUTF8 will force XML
    // to use that encoding, regardless of what XML might otherwise try to detect.
    virtual const char* Parse(const char* p, XMLParsingData* data = 0,
            XMLEncoding encoding = XMLEncodingDefault);

    // Get the root element -- the only top level element -- of the document.
    // In well formed XML, there should only be one. XML is tolerant of
    // multiple elements at the document level.
    XMLElement* GetRootElement() const
    {
        return GetFirstChildElement();
    }

    // If an error occurs, Error will be set to true. Also,
    // - The ErrorId() will contain the integer identifier of the error (not generally useful)
    // - The ErrorDesc() method will return the name of the error. (very useful)
    // - The ErrorRow() and ErrorCol() will return the location of the error (if known)
    bool HasError() const
    {
        return mHasError;
    }

    // Contains a textual (English) description of the error if one occurs.
    std::string GetErrorDesc() const
    {
        return mErrorDesc;
    }

    // Generally, you probably want the error string ( ErrorDesc() ). But if you
    // prefer the ErrorId, this function will fetch it.
    int GetErrorId() const
    {
        return mErrorID;
    }

    // Returns the location (if known) of the error. The first column is column 1, 
    // and the first row is row 1. A value of 0 means the row and column wasn't applicable
    // (memory errors, for example, have no row/column) or the parser lost the error. (An
    // error in the error reporting, in that case.)
    int GetErrorRow() const
    {
        return mErrorLocation.row + 1;
    }

    int GetErrorCol() const
    {
        return mErrorLocation.col + 1;
    }

    // SetTabSize() allows the error reporting functions (ErrorRow() and ErrorCol())
    // to report the correct values for row and column. It does not change the output
    // or input in any way.
    // By calling this method, with a tab size
    // greater than 0, the row and column of each node and attribute is stored
    // when the file is loaded. Very useful for tracking the DOM back in to
    // the source file.
    // The tab size is required for calculating the location of nodes. If not
    // set, the default of 4 is used. The tab size is set per document. Setting
    // the tab size to 0 disables row/column tracking.
    // Note that row and column tracking is not supported when using operator>>.
    // The tab size needs to be enabled before the parse or load. Correct usage:
    // XMLDoc doc;
    // doc.SetTabSize( 8 );
    // doc.Load("myfile.xml");
    void SetTabSize(int tabsize)
    {
        mTabsize = tabsize;
    }

    int GetTabSize() const
    {
        return mTabsize;
    }

    // If you have handled the error, it can be reset with this call. The error
    // state is automatically cleared if you Parse a new XML block.
    void ClearError()
    {
        mHasError = false;
        mErrorID = 0;
        mErrorDesc = "";
        mErrorLocation.row = mErrorLocation.col = 0;
    }

    // Write the document to standard out using formatted printing ("pretty print"). 
    void Print() const
    {
        Print(stdout, 0);
    }

    // Print this Document to a FILE stream.
    virtual void Print(FILE* cfile, int depth = 0) const;
    void SetError(int err, const char* errorLocation, XMLParsingData* prevData,
            XMLEncoding encoding);

    // Cast to a more defined type. Will return null not of the requested type.
    virtual XMLDoc* ToDocument() const
    {
        return const_cast<XMLDoc*> (this);
    }

    // Walk the XML tree visiting this node and all of its children. 
    virtual bool Accept(XMLVisitor* content) const;

    std::string GetXMLText() const
    {
        return mXMLText;
    }

protected:
    virtual XMLNode* Clone() const;
    virtual void StreamIn(std::istream* in, std::string* tag);

private:
    void CopyTo(XMLDoc* target) const;

    bool mHasError;
    int mErrorID;
    std::string mErrorDesc;
    int mTabsize;
    XMLCursor mErrorLocation;
    // the UTF-8 BOM were found when read. Note this, and try to write.
    bool mIsUseMicrosoftBOM;
    std::string mXMLText;
};

//////////////////////////////////////////////////////////////////////////
// A XMLHandle is a class that wraps a node pointer with null checks; this is
// an incredibly useful thing. Note that XMLHandle is not part of the XML
// DOM structure. It is a separate utility class.
// Take an example:
// <Document>
// <Element attributeA = "valueA">
// <Child attributeB = "value1" />
// <Child attributeB = "value2" />
// </Element>
// <Document>
// Assuming you want the value of "attributeB" in the 2nd "Child" element, it's very 
// easy to write a *lot* of code that looks like:
// XMLElement* root = document.FirstChildElement( "Document" );
// if ( root )
// {
// XMLElement* element = root->FirstChildElement( "Element" );
// if ( element )
// {
// XMLElement* child = element->FirstChildElement( "Child" );
// if ( child )
//{
// XMLElement* child2 = child->NextSiblingElement( "Child" );
// if ( child2 )
// {
// Finally do something useful.
//
// And that doesn't even cover "else" cases. XMLHandle addresses the verbosity
// of such code. A XMLHandle checks for null	pointers so it is perfectly safe 
// and correct to use:
// XMLHandle docHandle( &document );
// XMLElement* child2 = docHandle.FirstChild( "Document" ).FirstChild( "Element" ).Child( "Child", 1 ).ToElement();
// if ( child2 )
// {
// do something useful
//
// Which is MUCH more concise and useful.
//
// It is also safe to copy handles - internally they are nothing more than node pointers.
// XMLHandle handleCopy = handle;
//
// What they should not be used for is iteration:
// int i=0; 
// while ( true )
// {
// XMLElement* child = docHandle.FirstChild( "Document" ).FirstChild( "Element" ).Child( "Child", i ).ToElement();
// if ( !child )
// break;
// do something
// ++i;
// }
//
// It seems reasonable, but it is in fact two embedded while loops. The Child method is 
// a linear walk to find the element, so this code would iterate much more than it needs 
// to. Instead, prefer:
// XMLElement* child = docHandle.FirstChild( "Document" ).FirstChild( "Element" ).FirstChild( "Child" ).ToElement();
// for( child; child; child=child->NextSiblingElement() )
// {
// do something
// }
class XMLHandle
{
public:
    // Create a handle from any node (at any depth of the tree.) This can be a null pointer.
    XMLHandle(XMLNode* node)
    {
        this->mNode = node;
    }
    // Copy constructor
    XMLHandle(const XMLHandle& ref)
    {
        this->mNode = ref.mNode;
    }
    XMLHandle operator=(const XMLHandle& ref)
    {
        this->mNode = ref.mNode;
        return *this;
    }

    // Return a handle to the first child node.
    XMLHandle GetFirstChild() const;
    // Return a handle to the first child node with the given name.
    XMLHandle GetFirstChild(const std::string& value) const;
    // Return a handle to the first child element.
    XMLHandle GetFirstChildElement() const;
    // Return a handle to the first child element with the given name.
    XMLHandle GetFirstChildElement(const std::string& value) const;

    // Return a handle to the "index" child with the given name. 
    // The first child is 0, the second 1, etc.
    XMLHandle GetChild(const std::string& value, int index) const;
    // Return a handle to the "index" child. 
    // The first child is 0, the second 1, etc.
    XMLHandle GetChild(int index) const;
    // Return a handle to the "index" child element with the given name. 
    // The first child element is 0, the second 1, etc. Note that only TiXmlElements
    // are indexed: other types are not counted.
    XMLHandle GetChildElement(const std::string& value, int index) const;
    // Return a handle to the "index" child element. 
    // The first child element is 0, the second 1, etc. Note that only TiXmlElements
    // are indexed: other types are not counted.
    XMLHandle GetChildElement(int index) const;

    // Return the handle as a XMLNode. This may return null.
    XMLNode* ToNode() const
    {
        return mNode;
    }
    // Return the handle as a XMLElement. This may return null.
    XMLElement* ToElement() const
    {
        return ((mNode && mNode->ToElement()) ? mNode->ToElement() : 0);
    }
    //	Return the handle as a XMLText. This may return null.
    XMLText* ToText() const
    {
        return ((mNode && mNode->ToText()) ? mNode->ToText() : 0);
    }
    // Return the handle as a XMLUnknown. This may return null.
    XMLUnknown* ToUnknown() const
    {
        return ((mNode && mNode->ToUnknown()) ? mNode->ToUnknown() : 0);
    }

    // deprecated use ToNode. 
    // Return the handle as a XMLNode. This may return null.
    XMLNode* GetNode() const
    {
        return ToNode();
    }
    // deprecated use ToElement. 
    // Return the handle as a XMLElement. This may return null.
    XMLElement* GetElement() const
    {
        return ToElement();
    }
    // deprecated use ToText()
    // Return the handle as a XMLText. This may return null.
    XMLText* GetText() const
    {
        return ToText();
    }
    // deprecated use ToUnknown()
    // Return the handle as a XMLUnknown. This may return null.
    XMLUnknown* GetUnknown() const
    {
        return ToUnknown();
    }

private:
    XMLNode* mNode;
};

//////////////////////////////////////////////////////////////////////////
// Print to memory functionality. The XMLPrinter is useful when you need to:
// -# Print to memory (especially in non-STL mode)
// -# Control formatting (line endings, etc.)
// When constructed, the XMLPrinter is in its default "pretty printing" mode.
// Before calling Accept() you can call methods to control the printing
// of the XML document. After XMLNode::Accept() is called, the printed document can
// be accessed via the CStr(), Str(), and Size() methods.
// XMLPrinter uses the Visitor API.
// XMLPrinter printer;
// printer.SetIndent( "\t" );
// doc.Accept( &printer );
// fprintf( stdout, "%s", printer.CStr() );
class XMLPrinter: public XMLVisitor
{
public:
    XMLPrinter() :
        mDepth(0), mSimpleTextPrint(false), mBuffer(), mIndent("    "),
                mLineBreak("\n")
    {
    }

    virtual bool VisitEnter(const XMLDoc& doc);
    virtual bool VisitExit(const XMLDoc& doc);

    virtual bool VisitEnter(const XMLElement& element,
            const XMLAttribute* firstAttribute);
    virtual bool VisitExit(const XMLElement& element);

    virtual bool Visit(const XMLDeclaration& declaration);
    virtual bool Visit(const XMLText& text);
    virtual bool Visit(const XMLComment& comment);
    virtual bool Visit(const XMLUnknown& unknown);

    // Set the indent characters for printing. By default 4 spaces
    // but tab (\t) is also useful, or null/empty string for no indentation.
    void SetIndent(const std::string& indent)
    {
        mIndent = !indent.empty() ? indent : "";
    }
    // Query the indention string.
    std::string GetIndent() const
    {
        return mIndent;
    }
    // Set the line breaking string. By default set to newline (\n). 
    // Some operating systems prefer other characters, or can be
    // set to the null/empty string for no indentation.
    void SetLineBreak(const std::string& lineBreak)
    {
        mLineBreak = !lineBreak.empty() ? lineBreak : "";
    }
    // Query the current line breaking string.
    std::string GetLineBreak()
    {
        return mLineBreak;
    }

    // Switch over to "stream printing" which is the most dense formatting without 
    // line breaks. Common when the XML is needed for network transmission.
    void SetStreamPrinting()
    {
        mIndent = "";
        mLineBreak = "";
    }

    // Return the length of the result string.
    size_t Size()
    {
        return mBuffer.size();
    }

    // Return the result.
    std::string Str() const
    {
        return mBuffer;
    }

private:
    void DoIndent()
    {
        for (int i = 0; i < mDepth; ++i)
        {
            mBuffer += mIndent;
        }
    }
    void DoLineBreak()
    {
        mBuffer += mLineBreak;
    }

    int mDepth;
    bool mSimpleTextPrint;
    std::string mBuffer;
    std::string mIndent;
    std::string mLineBreak;
};

#endif // XMLParser_INCLUDED
