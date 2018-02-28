//////////////////////////////////////////////////////////////////////////
// XMLParser.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "XMLParser.h"
#include <ctype.h>
#include <sstream>
#include <iostream>
#include <stddef.h>

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
const char* XMLBase::mErrorString[XML_ERROR_STRING_COUNT] =
{ "OK. ", "Error. ", "Error: Failed to Open File. ",
        "Error: Memory Allocation Failed. ", "Error: Parse Element. ",
        "Error: Failed to Read Element Name. ", "Error: Read Element Value. ",
        "Error: Read Attributes. ", "Error: Empty Tag. ",
        "Error: Read End Tag. ", "Error: Parse Unknown. ",
        "Error: Parse Comment. ", "Error: Parse Declaration. ",
        "Error: Document Empty. ",
        "Error: NULL or Unexpected EOF Found in Input Stream. ",
        "Error: Parse CDATA. ", "Error: XMLDoc Can Only be at the Root. ", };

bool XMLBase::mCondenseWhiteSpace = true;

// Note the "PutString" hard codes the same list. This
// is less flexible than it appears. Changing the entries
// or order will break put string.	
XMLBase::Entity XMLBase::mEntity[NUM_ENTITY] =
{
{ "&amp;", 5, '&' },
{ "&lt;", 4, '<' },
{ "&gt;", 4, '>' },
{ "&quot;", 6, '\"' },
{ "&apos;", 6, '\'' } };

// Bunch of Unicode info at:
//http://www.unicode.org/faq/utf_bom.html
// Including the basic of this table, which determines the #bytes in the
// sequence from the lead byte. 1 placed for invalid sequences --
// although the result will be junk, pass it through as much as possible.
// Beware of the non-characters in UTF-8:	
//				ef bb bf (Microsoft "lead bytes")
//				ef bf be
//				ef bf bf 
const unsigned char XML_UTF_LEAD_0 = 0xefU;
const unsigned char XML_UTF_LEAD_1 = 0xbbU;
const unsigned char XML_UTF_LEAD_2 = 0xbfU;

const int XMLBase::mUTF8ByteTable[256] =
{
//	0	1	2	3	4	5	6	7	8	9	a	b	c	d	e	f
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x00
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x10
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x20
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x30
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x50
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x60
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x70	End of ASCII range
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x80 0x80 to 0xc1 invalid
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x90
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0xa0
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0xb0
        1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 0xc0 0xc2 to 0xdf 2 byte
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 0xd0
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 0xe0 0xe0 to 0xef 3 byte
        4, 4, 4, 4, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 // 0xf0 0xf0 to 0xf4 4 byte, 0xf5 and higher invalid
};
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// XMLBase
int XMLBase::GetRow() const
{
    return mLocation.row + 1;
}

int XMLBase::GetColumn() const
{
    return mLocation.col + 1;
}

// Set a pointer to arbitrary user data.
void XMLBase::SetUserData(void* user)
{
    mUserData = user;
}

// Get a pointer to arbitrary user data.
void* XMLBase::GetUserData()
{
    return mUserData;
}

// Get a pointer to arbitrary user data.
const void* XMLBase::GetUserData() const
{
    return mUserData;
}

bool XMLBase::IsWhiteSpace(char c)
{
    return (isspace((unsigned char) c) || c == '\n' || c == '\r');
}

bool XMLBase::IsWhiteSpace(int c)
{
    if (c < 256)
    {
        return IsWhiteSpace((char) c);
    }
    // Again, only truly correct for English/Latin...but usually works.
    return false;
}

// Get a character, while interpreting entities.
// The length can be from 0 to 4 bytes.
const char* XMLBase::GetChar(const char* p, char* value, int* length,
        XMLEncoding encoding)
{
    assert(p);
    if (encoding == XMLEncodingUTF8)
    {
        *length = mUTF8ByteTable[*((const unsigned char*) p)];
        assert( *length >= 0 && *length < 5 );
    }
    else
    {
        *length = 1;
    }

    if (*length == 1)
    {
        if (*p == '&')
        {
            return GetEntity(p, value, length, encoding);
        }
        *value = *p;
        return p + 1;
    }
    else if (*length)
    {
        for (int i = 0; p[i] && i < *length; ++i)
        {
            value[i] = p[i];
        }
        return p + (*length);
    }
    else
    {
        // Not valid text.
        return 0;
    }
}

int XMLBase::ToLower(int v, XMLEncoding encoding)
{
    if (encoding == XMLEncodingUTF8)
    {
        if (v < 128)
        {
            return tolower(v);
        }
        return v;
    }
    else
    {
        return tolower(v);
    }
}

void XMLBase::ConvertUTF32ToUTF8(unsigned long input, char* output, int* length)
{
    const unsigned long BYTE_MASK = 0xBF;
    const unsigned long BYTE_MARK = 0x80;
    const unsigned long FIRST_BYTE_MARK[7] =
    { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

    if (input < 0x80)
    {
        *length = 1;
    }
    else if (input < 0x800)
    {
        *length = 2;
    }
    else if (input < 0x10000)
    {
        *length = 3;
    }
    else if (input < 0x200000)
    {
        *length = 4;
    }
    else
    {
        *length = 0;
        return;
    } // This code won't covert this correctly anyway.

    output += *length;

    // Scary scary fall through.
    switch (*length)
    {
    case 4:
        --output;
        *output = (char) ((input | BYTE_MARK) & BYTE_MASK);
        input >>= 6;
    case 3:
        --output;
        *output = (char) ((input | BYTE_MARK) & BYTE_MASK);
        input >>= 6;
    case 2:
        --output;
        *output = (char) ((input | BYTE_MARK) & BYTE_MASK);
        input >>= 6;
    case 1:
        --output;
        *output = (char) (input | FIRST_BYTE_MARK[*length]);
    }
}

int XMLBase::IsAlpha(unsigned char anyByte, XMLEncoding /*encoding*/)
{
    // This will only work for low-ASCII, everything else is assumed to be a valid
    // letter. I'm not sure this is the best approach, but it is quite tricky trying
    // to figure out alphabetical vs. not across encoding. So take a very 
    // conservative approach.
    if (anyByte < 127)
    {
        return isalpha(anyByte);
    }
    else
    {
        return 1; // What else to do? The Unicode set is huge...get the English ones right.
    }
}

int XMLBase::IsAlphaNum(unsigned char anyByte, XMLEncoding /*encoding*/)
{
    // This will only work for low-ASCII, everything else is assumed to be a valid
    // letter. I'm not sure this is the best approach, but it is quite tricky trying
    // to figure out alphabetical vs. not across encoding. So take a very 
    // conservative approach.
    if (anyByte < 127)
    {
        return isalnum(anyByte);
    }
    else
    {
        return 1; // What else to do? The Unicode set is huge...get the English ones right.
    }
}

//////////////////////////////////////////////////////////////////////////
// XMLParsingData
class XMLParsingData
{
    friend class XMLDoc;
public:
    void Stamp(const char* now, XMLEncoding encoding);

    const XMLCursor& Cursor()
    {
        return cursor;
    }

private:
    // Only used by the document!
    XMLParsingData(const char* start, int _tabsize, int row, int col)
    {
        assert(start);
        stamp = start;
        tabsize = _tabsize;
        cursor.row = row;
        cursor.col = col;
    }

    XMLCursor cursor;
    const char* stamp;
    int tabsize;
};

void XMLParsingData::Stamp(const char* now, XMLEncoding encoding)
{
    assert(now);

    // Do nothing if the tabsize is 0.
    if (tabsize < 1)
    {
        return;
    }

    // Get the current row, column.
    int row = cursor.row;
    int col = cursor.col;
    const char* p = stamp;
    assert(p);

    while (p < now)
    {
        // Treat p as unsigned, so we have a happy compiler.
        const unsigned char* pU = (const unsigned char*) p;

        // Code contributed by Fletcher Dunn: (modified by lee)
        switch (*pU)
        {
        case 0:
            // We *should* never get here, but in case we do, don't
            // advance past the terminating null character, ever
            return;

        case '\r':
            // bump down to the next line
            ++row;
            col = 0;
            // Eat the character
            ++p;

            // Check for \r\n sequence, and treat this as a single character
            if (*p == '\n')
            {
                ++p;
            }
            break;

        case '\n':
            // bump down to the next line
            ++row;
            col = 0;

            // Eat the character
            ++p;

            // Check for \n\r sequence, and treat this as a single
            // character.  (Yes, this bizarre thing does occur still
            // on some arcane platforms...)
            if (*p == '\r')
            {
                ++p;
            }
            break;

        case '\t':
            // Eat the character
            ++p;

            // Skip to next tab stop
            col = (col / tabsize + 1) * tabsize;
            break;

        case XML_UTF_LEAD_0:
            if (encoding == XMLEncodingUTF8)
            {
                if (*(p + 1) && *(p + 2))
                {
                    // In these cases, don't advance the column. These are
                    // 0-width spaces.
                    if (*(pU + 1) == XML_UTF_LEAD_1 && *(pU + 2)
                            == XML_UTF_LEAD_2)
                    {
                        p += 3;
                    }
                    else if (*(pU + 1) == 0xbfU && *(pU + 2) == 0xbeU)
                    {
                        p += 3;
                    }
                    else if (*(pU + 1) == 0xbfU && *(pU + 2) == 0xbfU)
                    {
                        p += 3;
                    }
                    else
                    {
                        p += 3;
                        ++col;
                    } // A normal character.
                }
            }
            else
            {
                ++p;
                ++col;
            }
            break;

        default:
            if (encoding == XMLEncodingUTF8)
            {
                // Eat the 1 to 4 byte utf8 character.
                int step = XMLBase::mUTF8ByteTable[*((const unsigned char*) p)];
                if (step == 0)
                {
                    // Error case from bad encoding, but handle gracefully.
                    step = 1;
                }
                p += step;

                // Just advance one column, of course.
                ++col;
            }
            else
            {
                ++p;
                ++col;
            }
            break;
        }
    }
    cursor.row = row;
    cursor.col = col;
    assert(cursor.row >= -1);
    assert(cursor.col >= -1);
    stamp = p;
    assert(stamp );
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
const char* XMLBase::SkipWhiteSpace(const char* p, XMLEncoding encoding)
{
    if (!p || !*p)
    {
        return 0;
    }

    if (encoding == XMLEncodingUTF8)
    {
        while (*p)
        {
            const unsigned char* pU = (const unsigned char*) p;

            // Skip the stupid Microsoft UTF-8 Byte order marks
            if (*(pU + 0) == XML_UTF_LEAD_0 && *(pU + 1) == XML_UTF_LEAD_1
                    && *(pU + 2) == XML_UTF_LEAD_2)
            {
                p += 3;
                continue;
            }
            else if (*(pU + 0) == XML_UTF_LEAD_0 && *(pU + 1) == 0xbfU && *(pU
                    + 2) == 0xbeU)
            {
                p += 3;
                continue;
            }
            else if (*(pU + 0) == XML_UTF_LEAD_0 && *(pU + 1) == 0xbfU && *(pU
                    + 2) == 0xbfU)
            {
                p += 3;
                continue;
            }

            if (IsWhiteSpace(*p) || *p == '\n' || *p == '\r')
            {
                // Still using old rules for white space.
                ++p;
            }
            else
            {
                break;
            }
        }
    }
    else
    {
        while (((*p) && IsWhiteSpace(*p)) || (*p == '\n') || (*p == '\r'))
        {
            ++p;
        }
    }

    return p;
}

bool XMLBase::StreamWhiteSpace(std::istream* in, std::string* tag)
{
    for (;;)
    {
        if (!in->good())
        {
            return false;
        }

        int c = in->peek();
        // At this scope, we can't get to a document. So fail silently.
        if (!IsWhiteSpace(c) || c <= 0)
        {
            return true;
        }

        *tag += (char) in->get();
    }

    return true;
}

bool XMLBase::StreamTo(std::istream* in, int character, std::string* tag)
{
    while (in->good())
    {
        int c = in->peek();
        if (c == character)
        {
            return true;
        }
        if (c <= 0)
        {
            // Silent failure: can't get document at this scope
            return false;
        }

        in->get();
        *tag += (char) c;
    }
    return false;
}

// One of XML's more performance demanding functions. Try to keep the memory overhead down. The
// "assign" optimization removes over 10% of the execution time.
const char* XMLBase::ReadName(const char* p, std::string* name,
        XMLEncoding encoding)
{
    // Oddly, not supported on some compilers,
    //name->clear();
    // So use this:
    *name = "";
    assert(p);

    // Names start with letters or underscores.
    // Of course, in Unicode, XML has no idea what a letter *is*. The
    // algorithm is generous.
    // After that, they can be letters, underscores, numbers,
    // hyphens, or colons. (Colons are valid only for namespaces,
    // but XML can't tell namespaces from names.)
    if (p && *p && (IsAlpha((unsigned char) *p, encoding) || *p == '_'))
    {
        const char* start = p;
        while (p && *p && (IsAlphaNum((unsigned char) *p, encoding) || *p
                == '_' || *p == '-' || *p == '.' || *p == ':'))
        {
            //(*name) += *p; // expensive
            ++p;
        }
        if (p - start > 0)
        {
            name->assign(start, p - start);
        }
        return p;
    }
    return 0;
}

const char* XMLBase::GetEntity(const char* p, char* value, int* length,
        XMLEncoding encoding)
{
    // Presume an entity, and pull it out.
    std::string ent;
    int i;
    *length = 0;

    if (*(p + 1) && *(p + 1) == '#' && *(p + 2))
    {
        unsigned long ucs = 0;
        ptrdiff_t delta = 0;
        unsigned mult = 1;

        if (*(p + 2) == 'x')
        {
            // Hexadecimal.
            if (!*(p + 3))
            {
                return 0;
            }

            const char* q = p + 3;
            q = strchr(q, ';');

            if (!q || !*q)
            {
                return 0;
            }

            delta = q - p;
            --q;

            while (*q != 'x')
            {
                if (*q >= '0' && *q <= '9')
                {
                    ucs += mult * (*q - '0');
                }
                else if (*q >= 'a' && *q <= 'f')
                {
                    ucs += mult * (*q - 'a' + 10);
                }
                else if (*q >= 'A' && *q <= 'F')
                {
                    ucs += mult * (*q - 'A' + 10);
                }
                else
                {
                    return 0;
                }

                mult *= 16;
                --q;
            }
        }
        else
        {
            // Decimal.
            if (!*(p + 2))
            {
                return 0;
            }

            const char* q = p + 2;
            q = strchr(q, ';');

            if (!q || !*q)
            {
                return 0;
            }

            delta = q - p;
            --q;

            while (*q != '#')
            {
                if (*q >= '0' && *q <= '9')
                {
                    ucs += mult * (*q - '0');
                }
                else
                {
                    return 0;
                }
                mult *= 10;
                --q;
            }
        }
        if (encoding == XMLEncodingUTF8)
        {
            // convert the UCS to UTF-8
            ConvertUTF32ToUTF8(ucs, value, length);
        }
        else
        {
            *value = (char) ucs;
            *length = 1;
        }
        return p + delta + 1;
    }

    // Now try to match it.
    for (i = 0; i < NUM_ENTITY; ++i)
    {
        if (strncmp(mEntity[i].str, p, mEntity[i].strLength) == 0)
        {
            assert(strlen(mEntity[i].str) == mEntity[i].strLength);
            *value = mEntity[i].chr;
            *length = 1;
            return (p + mEntity[i].strLength);
        }
    }

    // So it wasn't an entity, its unrecognized, or something like that.
    *value = *p; // Don't put back the last one, since we return it!
    //*length = 1;	// Leave unrecognized entities - this doesn't really work.
    // Just writes strange XML.
    return p + 1;
}

bool XMLBase::StringEqual(const char* p, const char* tag, bool ignoreCase,
        XMLEncoding encoding)
{
    assert(p);
    assert(tag);
    if (!p || !*p)
    {
        assert(0);
        return false;
    }

    const char* q = p;

    if (ignoreCase)
    {
        while (*q && *tag && ToLower(*q, encoding) == ToLower(*tag, encoding))
        {
            ++q;
            ++tag;
        }

        if (*tag == 0)
        {
            return true;
        }
    }
    else
    {
        while (*q && *tag && *q == *tag)
        {
            ++q;
            ++tag;
        }

        if (*tag == 0)
        {
            // Have we found the end of the tag, and everything equal?
            return true;
        }
    }
    return false;
}

const char* XMLBase::ReadText(const char* p, std::string* text,
        bool trimWhiteSpace, const char* endTag, bool caseInsensitive,
        XMLEncoding encoding)
{
    *text = "";
    if (!trimWhiteSpace // certain tags always keep whitespace
            || !mCondenseWhiteSpace) // if true, whitespace is always kept
    {
        // Keep all the white space.
        while (p && *p && !StringEqual(p, endTag, caseInsensitive, encoding))
        {
            int len;
            char cArr[4] =
            { 0, 0, 0, 0 };
            p = GetChar(p, cArr, &len, encoding);
            text->append(cArr, len);
        }
    }
    else
    {
        bool whitespace = false;

        // Remove leading white space:
        p = SkipWhiteSpace(p, encoding);
        while (p && *p && !StringEqual(p, endTag, caseInsensitive, encoding))
        {
            if (*p == '\r' || *p == '\n')
            {
                whitespace = true;
                ++p;
            }
            else if (IsWhiteSpace(*p))
            {
                whitespace = true;
                ++p;
            }
            else
            {
                // If we've found whitespace, add it before the
                // new character. Any whitespace just becomes a space.
                if (whitespace)
                {
                    (*text) += ' ';
                    whitespace = false;
                }
                int len;
                char cArr[4] =
                { 0, 0, 0, 0 };
                p = GetChar(p, cArr, &len, encoding);
                if (len == 1)
                {
                    (*text) += cArr[0]; // more efficient
                }
                else
                {
                    text->append(cArr, len);
                }
            }
        }
    }
    if (p)
    {
        p += strlen(endTag);
    }

    return p;
}

void XMLBase::EncodeString(const std::string& str, std::string* outString)
{
    int i = 0;

    while (i < (int) str.length())
    {
        unsigned char c = (unsigned char) str[i];

        if (c == '&' && i < ((int) str.length() - 2) && str[i + 1] == '#'
                && str[i + 2] == 'x')
        {
            // Hexadecimal character reference.
            // Pass through unchanged.
            // &#xA9;	-- copyright symbol, for example.
            // The -1 is a bug fix from Rob Laveaux. It keeps
            // an overflow from happening if there is no ';'.
            // There are actually 2 ways to exit this loop -
            // while fails (error case) and break (semicolon found).
            // However, there is no mechanism (currently) for
            // this function to return an error.
            while (i < (int) str.length() - 1)
            {
                outString->append(str.c_str() + i, 1);
                ++i;
                if (str[i] == ';')
                {
                    break;
                }
            }
        }
        else if (c == '&')
        {
            outString->append(mEntity[0].str, mEntity[0].strLength);
            ++i;
        }
        else if (c == '<')
        {
            outString->append(mEntity[1].str, mEntity[1].strLength);
            ++i;
        }
        else if (c == '>')
        {
            outString->append(mEntity[2].str, mEntity[2].strLength);
            ++i;
        }
        else if (c == '\"')
        {
            outString->append(mEntity[3].str, mEntity[3].strLength);
            ++i;
        }
        else if (c == '\'')
        {
            outString->append(mEntity[4].str, mEntity[4].strLength);
            ++i;
        }
        else if (c < 32)
        {
            // Easy pass at non-alpha/numeric/symbol
            // Below 32 is symbolic.
            char buf[32];

#if defined(XML_SNPRINTF)		
            XML_SNPRINTF(buf, sizeof(buf), "&#x%02X;", (unsigned) (c & 0xff));
#else
            sprintf(buf, "&#x%02X;", (unsigned)(c & 0xff));
#endif		

            outString->append(buf, (int) strlen(buf));
            ++i;
        }
        else
        {
            *outString += (char) c; // somewhat more efficient function call.
            ++i;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// XMLNode
XMLNode::XMLNode(NodeType type) :
    XMLBase()
{
    mParent = 0;
    mType = type;
    mFirstChild = 0;
    mLastChild = 0;
    mPreviousNode = 0;
    mNextNode = 0;
}

XMLNode::~XMLNode()
{
    XMLNode* node = mFirstChild;
    XMLNode* temp = 0;

    while (node)
    {
        temp = node;
        node = node->mNextNode;
        delete temp;
    }
}

// The meaning of 'value' changes for the specific type of XMLNode.
// Document: filename of the xml file
// Element: name of the element
// Comment: the comment text
// Unknown: the tag contents
// Text: the text string
// The subclasses will wrap this function.
std::string XMLNode::GetValue() const
{
    return mValue;
}

// Changes the value of the node. Defined as:
// Document: filename of the xml file
// Element:	name of the element
// Comment:	the comment text
// Unknown:	the tag contents
// Text: the text string
void XMLNode::SetValue(const std::string& value)
{
    mValue = value;
}

// One step up the DOM.
XMLNode* XMLNode::GetParent() const
{
    return mParent;
}

void XMLNode::CopyTo(XMLNode* target) const
{
    target->SetValue(mValue.c_str());
    target->mUserData = mUserData;
}

void XMLNode::Clear()
{
    XMLNode* node = mFirstChild;
    XMLNode* temp = 0;

    while (node)
    {
        temp = node;
        node = node->mNextNode;
        delete temp;
    }

    mFirstChild = 0;
    mLastChild = 0;
}

long XMLNode::GetNumberOfChildNodes() const
{
    long result = 0;

    XMLNode* node;
    for (node = mFirstChild; node; node = node->mNextNode)
    {
        result++;
    }

    return result;
}

XMLNode* XMLNode::Identify(const char* p, XMLEncoding encoding)
{
    XMLNode* returnNode = 0;

    p = SkipWhiteSpace(p, encoding);
    if (!p || !*p || *p != '<')
    {
        return 0;
    }

    XMLDoc* doc = GetDocument();
    p = SkipWhiteSpace(p, encoding);
    if (!p || !*p)
    {
        return 0;
    }

    // What is this thing? 
    // - Elements start with a letter or underscore, but xml is reserved.
    // - Comments: <!--
    // - Deceleration: <?xml
    // - Everything else is unknown to XML.
    const char* xmlHeader =
    { "<?xml" };
    const char* commentHeader =
    { "<!--" };
    const char* dtdHeader =
    { "<!" };
    const char* cdataHeader =
    { "<![CDATA[" };

    if (StringEqual(p, xmlHeader, true, encoding))
    {
        returnNode = new XMLDeclaration();
    }
    else if (StringEqual(p, commentHeader, false, encoding))
    {
        returnNode = new XMLComment();
    }
    else if (StringEqual(p, cdataHeader, false, encoding))
    {
        XMLText* text = new XMLText("");
        text->SetCDATA(true);
        returnNode = text;
    }
    else if (StringEqual(p, dtdHeader, false, encoding))
    {
        returnNode = new XMLUnknown();
    }
    else if (IsAlpha(*(p + 1), encoding) || *(p + 1) == '_')
    {
        returnNode = new XMLElement("");
    }
    else
    {
        returnNode = new XMLUnknown();
    }

    if (returnNode)
    {
        // Set the parent, so it can report errors
        returnNode->mParent = this;
    }
    else
    {
        if (doc)
        {
            doc->SetError(XML_ERROR_OUT_OF_MEMORY, 0, 0, XMLEncodingUnknown);
        }
    }
    return returnNode;
}

XMLNode* XMLNode::LinkEndChild(XMLNode* node)
{
    assert(node->mParent == 0 || node->mParent == this);
    assert(node->GetDocument() == 0 || node->GetDocument() == this->GetDocument());

    if (node->GetType() == XMLNode::DOCUMENT)
    {
        delete node;
        if (GetDocument())
        {
            GetDocument()->SetError(XML_ERROR_DOCUMENT_TOP_ONLY, 0, 0,
                    XMLEncodingUnknown);
        }
        return 0;
    }

    node->mParent = this;

    node->mPreviousNode = mLastChild;
    node->mNextNode = 0;

    if (mLastChild)
    {
        mLastChild->mNextNode = node;
    }
    else
    {
        mFirstChild = node; // it was an empty list.
    }

    mLastChild = node;
    return node;
}

XMLNode* XMLNode::InsertEndChild(const XMLNode& addThis)
{
    if (addThis.GetType() == XMLNode::DOCUMENT)
    {
        if (GetDocument())
        {
            GetDocument()->SetError(XML_ERROR_DOCUMENT_TOP_ONLY, 0, 0,
                    XMLEncodingUnknown);
        }
        return 0;
    }
    XMLNode* node = addThis.Clone();
    if (!node)
    {
        return 0;
    }

    return LinkEndChild(node);
}

XMLNode* XMLNode::InsertBeforeChild(XMLNode* beforeThis, const XMLNode& addThis)
{
    if (!beforeThis || beforeThis->mParent != this)
    {
        return 0;
    }
    if (addThis.GetType() == XMLNode::DOCUMENT)
    {
        if (GetDocument())
        {
            GetDocument()->SetError(XML_ERROR_DOCUMENT_TOP_ONLY, 0, 0,
                    XMLEncodingUnknown);
        }
        return 0;
    }

    XMLNode* node = addThis.Clone();
    if (!node)
    {
        return 0;
    }
    node->mParent = this;

    node->mNextNode = beforeThis;
    node->mPreviousNode = beforeThis->mPreviousNode;
    if (beforeThis->mPreviousNode)
    {
        beforeThis->mPreviousNode->mNextNode = node;
    }
    else
    {
        assert(mFirstChild == beforeThis);
        mFirstChild = node;
    }
    beforeThis->mPreviousNode = node;
    return node;
}

XMLNode* XMLNode::InsertAfterChild(XMLNode* afterThis, const XMLNode& addThis)
{
    if (!afterThis || afterThis->mParent != this)
    {
        return 0;
    }

    if (addThis.GetType() == XMLNode::DOCUMENT)
    {
        if (GetDocument())
        {
            GetDocument()->SetError(XML_ERROR_DOCUMENT_TOP_ONLY, 0, 0,
                    XMLEncodingUnknown);
        }
        return 0;
    }

    XMLNode* node = addThis.Clone();
    if (!node)
    {
        return 0;
    }

    node->mParent = this;

    node->mPreviousNode = afterThis;
    node->mNextNode = afterThis->mNextNode;
    if (afterThis->mNextNode)
    {
        afterThis->mNextNode->mPreviousNode = node;
    }
    else
    {
        assert(mLastChild == afterThis);
        mLastChild = node;
    }
    afterThis->mNextNode = node;
    return node;
}

XMLNode* XMLNode::ReplaceChild(XMLNode* replaceThis, const XMLNode& withThis)
{
    if (replaceThis->mParent != this)
    {
        return 0;
    }

    XMLNode* node = withThis.Clone();
    if (!node)
    {
        return 0;
    }

    node->mNextNode = replaceThis->mNextNode;
    node->mPreviousNode = replaceThis->mPreviousNode;

    if (replaceThis->mNextNode)
    {
        replaceThis->mNextNode->mPreviousNode = node;
    }
    else
    {
        mLastChild = node;
    }

    if (replaceThis->mPreviousNode)
    {
        replaceThis->mPreviousNode->mNextNode = node;
    }
    else
    {
        mFirstChild = node;
    }

    delete replaceThis;
    node->mParent = this;
    return node;
}

bool XMLNode::RemoveChild(XMLNode* removeThis)
{
    if (removeThis->mParent != this)
    {
        assert(0);
        return false;
    }

    if (removeThis->mNextNode)
    {
        removeThis->mNextNode->mPreviousNode = removeThis->mPreviousNode;
    }
    else
    {
        mLastChild = removeThis->mPreviousNode;
    }

    if (removeThis->mPreviousNode)
    {
        removeThis->mPreviousNode->mNextNode = removeThis->mNextNode;
    }
    else
    {
        mFirstChild = removeThis->mNextNode;
    }

    delete removeThis;
    return true;
}

// The first child of this node. Will be null if there are no children.
XMLNode* XMLNode::GetFirstChild() const
{
    return mFirstChild;
}

XMLNode* XMLNode::GetFirstChild(const std::string& value) const
{
    XMLNode* node;
    for (node = mFirstChild; node; node = node->mNextNode)
    {
        if (strcasecmp(value.c_str(), node->GetValue().c_str()) == 0)
        {
            return node;
        }
    }
    return 0;
}

// The last child of this node. Will be null if there are no children.
XMLNode* XMLNode::GetLastChild() const
{
    return mLastChild;
}

XMLNode* XMLNode::GetLastChild(const std::string& value) const
{
    XMLNode* node;
    for (node = mLastChild; node; node = node->mPreviousNode)
    {
        if (strcasecmp(value.c_str(), node->GetValue().c_str()) == 0)
        {
            return node;
        }
    }
    return 0;
}

XMLNode* XMLNode::IterateChildren(const XMLNode* previous) const
{
    if (!previous)
    {
        return GetFirstChild();
    }
    else
    {
        assert(previous->mParent == this);
        return previous->GetNextSibling();
    }
}

XMLNode* XMLNode::IterateChildren(const std::string& value,
        const XMLNode* previous) const
{
    if (!previous)
    {
        return GetFirstChild(value);
    }
    else
    {
        assert(previous->mParent == this);
        return previous->GetNextSibling(value);
    }
}

XMLNode* XMLNode::GetNextSibling(const std::string& value) const
{
    XMLNode* node;
    for (node = mNextNode; node; node = node->mNextNode)
    {
        if (strcasecmp(value.c_str(), node->GetValue().c_str()) == 0)
        {
            return node;
        }
    }
    return 0;
}

// Navigate to a sibling node.
XMLNode* XMLNode::GetNextSibling() const
{
    return mNextNode;
}

XMLNode* XMLNode::GetPreviousSibling(const std::string& value) const
{
    XMLNode* node;
    for (node = mPreviousNode; node; node = node->mPreviousNode)
    {
        if (strcasecmp(value.c_str(), node->GetValue().c_str()) == 0)
        {
            return node;
        }
    }
    return 0;
}

// Navigate to a sibling node.
XMLNode* XMLNode::GetPreviousSibling() const
{
    return mPreviousNode;
}

void XMLElement::RemoveAttribute(const std::string& name)
{
    XMLAttribute* node = mAttributeSet.Find(name);
    if (node)
    {
        mAttributeSet.Remove(node);
        delete node;
    }
}

XMLElement* XMLNode::GetFirstChildElement(const std::string& value) const
{
    XMLNode* node;
    for (node = GetFirstChild(value); node; node = node->GetNextSibling(value))
    {
        if (node->ToElement())
        {
            return node->ToElement();
        }
    }
    return 0;
}

XMLElement* XMLNode::GetFirstChildElement() const
{
    XMLNode* node;
    for (node = GetFirstChild(); node; node = node->GetNextSibling())
    {
        if (node->ToElement())
        {
            return node->ToElement();
        }
    }
    return 0;
}

int XMLNode::GetNumberOfChildElements() const
{
    int result = 0;
    XMLElement* element = GetFirstChildElement();
    for (; element; element = element->GetNextSiblingElement())
    {
        result++;
    }

    return result;
}

XMLElement* XMLNode::GetNextSiblingElement(const std::string& value) const
{
    XMLNode* node;
    for (node = GetNextSibling(value); node; node = node->GetNextSibling(value))
    {
        if (node->ToElement())
        {
            return node->ToElement();
        }
    }
    return 0;
}

XMLElement* XMLNode::GetNextSiblingElement() const
{
    XMLNode* node;
    for (node = GetNextSibling(); node; node = node->GetNextSibling())
    {
        if (node->ToElement())
        {
            return node->ToElement();
        }
    }
    return 0;
}

// Query the type (as an enumerated value, above) of this node.
// The possible types are: DOCUMENT, ELEMENT, COMMENT, UNKNOWN, TEXT, and DECLARATION.
int XMLNode::GetType() const
{
    return mType;
}

XMLDoc* XMLNode::GetDocument() const
{
    const XMLNode* node;
    for (node = this; node; node = node->mParent)
    {
        if (node->ToDocument())
        {
            return node->ToDocument();
        }
    }
    return 0;
}

// Returns true if this node has children.
bool XMLNode::HasChildren() const
{
    return (mFirstChild != NULL);
}

//////////////////////////////////////////////////////////////////////////
// XMLElement
XMLElement::XMLElement(const std::string& value) :
    XMLNode(XMLNode::ELEMENT)
{
    mFirstChild = mLastChild = 0;
    mValue = value;
}

XMLElement::XMLElement(const XMLElement& copy) :
    XMLNode(XMLNode::ELEMENT)
{
    mFirstChild = mLastChild = 0;
    copy.CopyTo(this);
}

void XMLElement::operator=(const XMLElement& base)
{
    ClearThis();
    base.CopyTo(this);
}

XMLElement::~XMLElement()
{
    ClearThis();
}

void XMLElement::ClearThis()
{
    Clear();
    while (mAttributeSet.GetFirst())
    {
        XMLAttribute* node = mAttributeSet.GetFirst();
        mAttributeSet.Remove(node);
        delete node;
    }
}

bool XMLElement::GetAttribute(const std::string& name, std::string& result,
        const std::string& defaultValue) const
{
    result = defaultValue;
    XMLAttribute* node = mAttributeSet.Find(name);
    if (node)
    {
        result = node->GetValue();
        return true;
    }
    else
    {
        return false;
    }
}

bool XMLElement::GetAttribute(const std::string& name, int& result,
        int defaultValue) const
{
    result = defaultValue;
    std::string temp;
    bool ret = GetAttribute(name, temp);
    if (ret)
    {
        result = atoi(temp.c_str());
        return true;
    }
    else
    {
        return false;
    }
}

bool XMLElement::GetAttribute(const std::string& name, double& result,
        double defaultValue) const
{
    result = defaultValue;
    std::string temp;
    bool ret = GetAttribute(name, temp);
    if (ret)
    {
        result = atof(temp.c_str());
        return true;
    }
    else
    {
        return false;
    }
}

int XMLElement::QueryAttribute(const std::string& name, std::string& result,
        const std::string& defaultValue) const
{
    const XMLAttribute* node = mAttributeSet.Find(name);
    if (!node)
    {
        result = defaultValue;
        return XML_NO_ATTRIBUTE;
    }
    return node->QueryValue(result);
}

int XMLElement::QueryAttribute(const std::string& name, int& result,
        int defaultValue) const
{
    const XMLAttribute* node = mAttributeSet.Find(name);
    if (!node)
    {
        result = defaultValue;
        return XML_NO_ATTRIBUTE;
    }
    return node->QueryValue(result);
}

int XMLElement::QueryAttribute(const std::string& name, double& result,
        double defaultValue) const
{
    const XMLAttribute* node = mAttributeSet.Find(name);
    if (!node)
    {
        result = defaultValue;
        return XML_NO_ATTRIBUTE;
    }
    return node->QueryValue(result);
}

void XMLElement::SetAttribute(const std::string& name, int value)
{
    char buf[64];
#if defined(XML_SNPRINTF)		
    XML_SNPRINTF(buf, sizeof(buf), "%d", value);
#else
    sprintf(buf, "%d", value);
#endif
    SetAttribute(name, std::string(buf));
}

void XMLElement::SetAttribute(const std::string& name, double value)
{
    char buf[256];
#if defined(XML_SNPRINTF)		
    XML_SNPRINTF(buf, sizeof(buf), "%f", value);
#else
    sprintf(buf, "%f", value);
#endif
    SetAttribute(name, std::string(buf));
}

void XMLElement::SetAttribute(const std::string& name, const std::string& value)
{
    XMLAttribute* node = mAttributeSet.Find(name);
    if (node)
    {
        node->SetValue(value);
        return;
    }

    XMLAttribute* attrib = new XMLAttribute(name, value);
    if (attrib)
    {
        mAttributeSet.Add(attrib);
    }
    else
    {
        XMLDoc* document = GetDocument();
        if (document)
        {
            document->SetError(XML_ERROR_OUT_OF_MEMORY, 0, 0,
                    XMLEncodingUnknown);
        }
    }
}

void XMLElement::Print(FILE* cfile, int depth) const
{
    int i;
    assert(cfile);
    for (i = 0; i < depth; i++)
    {
        fprintf(cfile, "    ");
    }

    fprintf(cfile, "<%s", mValue.c_str());

    const XMLAttribute* attrib;
    for (attrib = mAttributeSet.GetFirst(); attrib; attrib = attrib->GetNext())
    {
        fprintf(cfile, " ");
        attrib->Print(cfile, depth);
    }

    // There are 3 different formatting approaches:
    // 1) An element without children is printed as a <foo /> node
    // 2) An element with only a text child is printed as <foo> text </foo>
    // 3) An element with children is printed on multiple lines.
    XMLNode* node;
    if (!mFirstChild)
    {
        fprintf(cfile, " />");
    }
    else if (mFirstChild == mLastChild && mFirstChild->ToText())
    {
        fprintf(cfile, ">");
        mFirstChild->Print(cfile, depth + 1);
        fprintf(cfile, "</%s>", mValue.c_str());
    }
    else
    {
        fprintf(cfile, ">");

        for (node = mFirstChild; node; node = node->GetNextSibling())
        {
            if (!node->ToText())
            {
                fprintf(cfile, "\n");
            }
            node->Print(cfile, depth + 1);
        }
        fprintf(cfile, "\n");
        for (i = 0; i < depth; ++i)
        {
            fprintf(cfile, "    ");
        }
        fprintf(cfile, "</%s>", mValue.c_str());
    }
}

void XMLElement::CopyTo(XMLElement* target) const
{
    // superclass:
    XMLNode::CopyTo(target);

    // Element class: 
    // Clone the attributes, then clone the children.
    const XMLAttribute* attribute = 0;
    for (attribute = mAttributeSet.GetFirst(); attribute; attribute
            = attribute->GetNext())
    {
        target->SetAttribute(attribute->GetName(), attribute->GetValue());
    }

    XMLNode* node = 0;
    for (node = mFirstChild; node; node = node->GetNextSibling())
    {
        target->LinkEndChild(node->Clone());
    }
}

bool XMLElement::Accept(XMLVisitor* visitor) const
{
    if (visitor->VisitEnter(*this, mAttributeSet.GetFirst()))
    {
        for (const XMLNode* node = GetFirstChild(); node; node
                = node->GetNextSibling())
        {
            if (!node->Accept(visitor))
            {
                break;
            }
        }
    }
    return visitor->VisitExit(*this);
}

XMLNode* XMLElement::Clone() const
{
    XMLElement* clone = new XMLElement(GetValue());
    if (!clone)
    {
        return 0;
    }

    CopyTo(clone);
    return clone;
}

std::string XMLElement::GetText() const
{
    const XMLNode* child = this->GetFirstChild();
    if (child)
    {
        const XMLText* childText = child->ToText();
        if (childText)
        {
            return childText->GetValue();
        }
    }

    return "";
}

const char* XMLElement::Parse(const char* p, XMLParsingData* data,
        XMLEncoding encoding)
{
    p = SkipWhiteSpace(p, encoding);
    XMLDoc* document = GetDocument();

    if (!p || !*p)
    {
        if (document)
        {
            document->SetError(XML_ERROR_PARSING_ELEMENT, 0, 0, encoding);
        }
        return 0;
    }

    if (data)
    {
        data->Stamp(p, encoding);
        mLocation = data->Cursor();
    }

    if (*p != '<')
    {
        if (document)
        {
            document->SetError(XML_ERROR_PARSING_ELEMENT, p, data, encoding);
        }
        return 0;
    }

    p = SkipWhiteSpace(p + 1, encoding);

    // Read the name.
    const char* pErr = p;

    p = ReadName(p, &mValue, encoding);
    if (!p || !*p)
    {
        if (document)
        {
            document->SetError(XML_ERROR_FAILED_TO_READ_ELEMENT_NAME, pErr,
                    data, encoding);
        }
        return 0;
    }

    std::string endTag("</");
    endTag += mValue;
    endTag += ">";

    // Check for and read attributes. Also look for an empty
    // tag or an end tag.
    while (p && *p)
    {
        pErr = p;
        p = SkipWhiteSpace(p, encoding);
        if (!p || !*p)
        {
            if (document)
            {
                document->SetError(XML_ERROR_READING_ATTRIBUTES, pErr, data,
                        encoding);
            }
            return 0;
        }
        if (*p == '/')
        {
            ++p;
            // Empty tag.
            if (*p != '>')
            {
                if (document)
                {
                    document->SetError(XML_ERROR_PARSING_EMPTY, p, data,
                            encoding);
                }
                return 0;
            }
            return (p + 1);
        }
        else if (*p == '>')
        {
            // Done with attributes (if there were any.)
            // Read the value -- which can include other
            // elements -- read the end tag, and return.
            ++p;
            p = ReadValue(p, data, encoding);// Note this is an Element method, and will set the error if one happens.
            if (!p || !*p)
            {
                // We were looking for the end tag, but found nothing.
                // Fix for [ 1663758 ] Failure to report error on bad XML
                if (document)
                {
                    document->SetError(XML_ERROR_READING_END_TAG, p, data,
                            encoding);
                }
                return 0;
            }

            // We should find the end tag now
            if (StringEqual(p, endTag.c_str(), false, encoding))
            {
                p += endTag.length();
                return p;
            }
            else
            {
                if (document)
                {
                    document->SetError(XML_ERROR_READING_END_TAG, p, data,
                            encoding);
                }
                return 0;
            }
        }
        else
        {
            // Try to read an attribute:
            XMLAttribute* attrib = new XMLAttribute();
            if (!attrib)
            {
                if (document)
                {
                    document->SetError(XML_ERROR_OUT_OF_MEMORY, pErr, data,
                            encoding);
                }
                return 0;
            }

            attrib->SetDocument(document);
            pErr = p;
            p = attrib->Parse(p, data, encoding);

            if (!p || !*p)
            {
                if (document)
                {
                    document->SetError(XML_ERROR_PARSING_ELEMENT, pErr, data,
                            encoding);
                }
                delete attrib;
                return 0;
            }

            // Handle the strange case of double attributes:
            XMLAttribute* node = mAttributeSet.Find(attrib->GetName());
            if (node)
            {
                node->SetValue(attrib->GetValue());
                delete attrib;
                return 0;
            }

            mAttributeSet.Add(attrib);
        }
    }
    return p;
}

const char* XMLElement::ReadValue(const char* p, XMLParsingData* data,
        XMLEncoding encoding)
{
    XMLDoc* document = GetDocument();

    // Read in text and elements in any order.
    const char* pWithWhiteSpace = p;
    p = SkipWhiteSpace(p, encoding);

    while (p && *p)
    {
        if (*p != '<')
        {
            // Take what we have, make a text element.
            XMLText* textNode = new XMLText("");

            if (!textNode)
            {
                if (document)
                {
                    document->SetError(XML_ERROR_OUT_OF_MEMORY, 0, 0, encoding);
                }
                return 0;
            }

            if (XMLBase::IsWhiteSpaceCondensed())
            {
                p = textNode->Parse(p, data, encoding);
            }
            else
            {
                // Special case: we want to keep the white space
                // so that leading spaces aren't removed.
                p = textNode->Parse(pWithWhiteSpace, data, encoding);
            }

            if (!textNode->IsBlank())
            {
                LinkEndChild(textNode);
            }
            else
            {
                delete textNode;
            }
        }
        else
        {
            // We hit a '<'
            // Have we hit a new element or an end tag? This could also be
            // a XMLText in the "CDATA" style.
            if (StringEqual(p, "</", false, encoding))
            {
                return p;
            }
            else
            {
                XMLNode* node = Identify(p, encoding);
                if (node)
                {
                    p = node->Parse(p, data, encoding);
                    LinkEndChild(node);
                }
                else
                {
                    return 0;
                }
            }
        }
        pWithWhiteSpace = p;
        p = SkipWhiteSpace(p, encoding);
    }

    if (!p)
    {
        if (document)
        {
            document->SetError(XML_ERROR_READING_ELEMENT_VALUE, 0, 0, encoding);
        }
    }
    return p;
}

void XMLElement::StreamIn(std::istream* in, std::string* tag)
{
    // We're called with some amount of pre-parsing. That is, some of "this"
    // element is in "tag". Go ahead and stream to the closing ">"
    while (in->good())
    {
        int c = in->get();
        if (c <= 0)
        {
            XMLDoc* document = GetDocument();
            if (document)
            {
                document->SetError(XML_ERROR_EMBEDDED_NULL, 0, 0,
                        XMLEncodingUnknown);
            }
            return;
        }
        (*tag) += (char) c;

        if (c == '>')
        {
            break;
        }
    }

    if (tag->length() < 3)
    {
        return;
    }

    // Okay...if we are a "/>" tag, then we're done. We've read a complete tag.
    // If not, identify and stream.
    if (tag->at(tag->length() - 1) == '>' && tag->at(tag->length() - 2) == '/')
    {
        // All good!
        return;
    }
    else if (tag->at(tag->length() - 1) == '>')
    {
        // There is more. Could be:
        //		text
        //		cdata text (which looks like another node)
        //		closing tag
        //		another node.
        for (;;)
        {
            StreamWhiteSpace(in, tag);

            // Do we have text?
            if (in->good() && in->peek() != '<')
            {
                // Yep, text.
                XMLText text("");
                text.StreamIn(in, tag);

                // What follows text is a closing tag or another node.
                // Go around again and figure it out.
                continue;
            }

            // We now have either a closing tag...or another node.
            // We should be at a "<", regardless.
            if (!in->good())
            {
                return;
            }
            assert(in->peek() == '<');
            int tagIndex = (int) tag->length();

            bool closingTag = false;
            bool firstCharFound = false;

            for (;;)
            {
                if (!in->good())
                {
                    return;
                }

                int c = in->peek();
                if (c <= 0)
                {
                    XMLDoc* document = GetDocument();
                    if (document)
                    {
                        document->SetError(XML_ERROR_EMBEDDED_NULL, 0, 0,
                                XMLEncodingUnknown);
                    }
                    return;
                }

                if (c == '>')
                {
                    break;
                }

                *tag += (char) c;
                in->get();

                // Early out if we find the CDATA id.
                if (c == '[' && tag->size() >= 9)
                {
                    size_t len = tag->size();
                    const char* start = tag->c_str() + len - 9;
                    if (strcmp(start, "<![CDATA[") == 0)
                    {
                        assert(!closingTag);
                        break;
                    }
                }

                if (!firstCharFound && c != '<' && !IsWhiteSpace(c))
                {
                    firstCharFound = true;
                    if (c == '/')
                    {
                        closingTag = true;
                    }
                }
            }
            // If it was a closing tag, then read in the closing '>' to clean up the input stream.
            // If it was not, the streaming will be done by the tag.
            if (closingTag)
            {
                if (!in->good())
                {
                    return;
                }

                int c = in->get();
                if (c <= 0)
                {
                    XMLDoc* document = GetDocument();
                    if (document)
                    {
                        document->SetError(XML_ERROR_EMBEDDED_NULL, 0, 0,
                                XMLEncodingUnknown);
                    }
                    return;
                }
                assert(c == '>');
                *tag += (char) c;

                // We are done, once we've found our closing tag.
                return;
            }
            else
            {
                // If not a closing tag, id it, and stream.
                const char* tagloc = tag->c_str() + tagIndex;
                XMLNode* node = Identify(tagloc, XMLEncodingDefault);
                if (!node)
                {
                    return;
                }
                node->StreamIn(in, tag);
                delete node;
                node = 0;

                // No return: go around from the beginning: text, closing tag, or node.
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// XMLDoc
XMLDoc::XMLDoc() :
    XMLNode(XMLNode::DOCUMENT)
{
    mTabsize = 4;
    mIsUseMicrosoftBOM = false;
    mXMLText = "";
    ClearError();
}

XMLDoc::XMLDoc(const std::string& documentName) :
    XMLNode(XMLNode::DOCUMENT)
{
    mTabsize = 4;
    mIsUseMicrosoftBOM = false;
    mValue = documentName;
    mXMLText = "";
    ClearError();
}

XMLDoc::XMLDoc(const XMLDoc& copy) :
    XMLNode(XMLNode::DOCUMENT)
{
    copy.CopyTo(this);
    this->mErrorID = copy.mErrorID;
    this->mHasError = copy.mHasError;
    this->mTabsize = copy.mTabsize;
    this->mIsUseMicrosoftBOM = copy.mIsUseMicrosoftBOM;
}

void XMLDoc::operator=(const XMLDoc& copy)
{
    Clear();
    copy.CopyTo(this);
}

bool XMLDoc::LoadXMLString(const std::string& data, XMLEncoding encoding)
{
    mXMLText = data;
    Parse(data.c_str(), 0, encoding);
    if (HasError())
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool XMLDoc::LoadFile(XMLEncoding encoding)
{
    return LoadFile(GetValue(), encoding);
}

bool XMLDoc::SaveFile() const
{
    return SaveFile(GetValue());
}

bool XMLDoc::LoadFile(const std::string& fileName, XMLEncoding encoding)
{
    // There was a really terrifying little bug here. The code:
    // value = filename
    // in the STL case, cause the assignment method of the std::string to
    // be called. What is strange, is that the std::string had the same
    // address as it's c_str() method, and so bad things happen. Looks
    // like a bug in the Microsoft STL implementation.
    // Add an extra string to avoid the crash.
    mValue = fileName;

    // reading in binary mode so that XML can normalize the EOL
    FILE* file = OpenXMLFile(mValue.c_str(), "rb");
    if (file)
    {
        bool result = LoadFile(file, encoding);
        fclose(file);
        return result;
    }
    else
    {
        SetError(XML_ERROR_OPENING_FILE, 0, 0, XMLEncodingUnknown);
        return false;
    }
}

bool XMLDoc::LoadFile(FILE* file, XMLEncoding encoding)
{
    if (!file)
    {
        SetError(XML_ERROR_OPENING_FILE, 0, 0, XMLEncodingUnknown);
        return false;
    }

    // Delete the existing data:
    Clear();
    mLocation.Clear();

    // Get the file size, so we can pre-allocate the string. HUGE speed impact.
    long length = 0;
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Strange case, but good to handle up front.
    if (length <= 0)
    {
        SetError(XML_ERROR_DOCUMENT_EMPTY, 0, 0, XMLEncodingUnknown);
        return false;
    }

    // If we have a file, assume it is all one big XML file, and read it in.
    // The document parser may decide the document ends sooner than the entire file, however.
    std::string data;
    data.reserve(length);

    // Subtle bug here. XML did use fgets. But from the XML spec:
    // 2.11 End-of-Line Handling
    // <snip>
    // <quote>
    // ...the XML processor MUST behave as if it normalized all line breaks in external 
    // parsed entities (including the document entity) on input, before parsing, by translating 
    // both the two-character sequence #xD #xA and any #xD that is not followed by #xA to 
    // a single #xA character.
    // </quote>
    //
    // It is not clear fgets does that, and certainly isn't clear it works cross platform. 
    // Generally, you expect fgets to translate from the convention of the OS to the C/Unix
    // convention, and not work generally.

    char* buf = new char[length + 1];
    buf[0] = 0;

    if (fread(buf, length, 1, file) != 1)
    {
        delete[] buf;
        SetError(XML_ERROR_OPENING_FILE, 0, 0, XMLEncodingUnknown);
        return false;
    }

    const char* lastPos = buf;
    const char* p = buf;

    buf[length] = 0;
    while (*p)
    {
        assert(p < (buf+length));
        if (*p == 0xa)
        {
            // Newline character. No special rules for this. Append all the characters
            // since the last string, and include the newline.
            data.append(lastPos, (p - lastPos + 1)); // append, include the newline
            ++p; // move past the newline
            lastPos = p; // and point to the new buffer (may be 0)
            assert(p <= (buf+length));
        }
        else if (*p == 0xd)
        {
            // Carriage return. Append what we have so far, then
            // handle moving forward in the buffer.
            if ((p - lastPos) > 0)
            {
                data.append(lastPos, p - lastPos); // do not add the CR
            }
            data += (char) 0xa; // a proper newline

            if (*(p + 1) == 0xa)
            {
                // Carriage return - new line sequence
                p += 2;
                lastPos = p;
                assert(p <= (buf+length));
            }
            else
            {
                // it was followed by something else...that is presumably characters again.
                ++p;
                lastPos = p;
                assert(p <= (buf+length));
            }
        }
        else
        {
            ++p;
        }
    }
    // Handle any left over characters.
    if (p - lastPos)
    {
        data.append(lastPos, p - lastPos);
    }
    delete[] buf;
    buf = 0;

    return LoadXMLString(data, encoding);
}

bool XMLDoc::SaveFile(const std::string& fileName) const
{
    // The old c stuff lives on...
    FILE* fp = OpenXMLFile(fileName, "w");
    if (fp)
    {
        bool result = SaveFile(fp);
        fclose(fp);
        return result;
    }
    return false;
}

bool XMLDoc::SaveFile(FILE* fp) const
{
    if (mIsUseMicrosoftBOM)
    {
        const unsigned char XML_UTF_LEAD_0 = 0xefU;
        const unsigned char XML_UTF_LEAD_1 = 0xbbU;
        const unsigned char XML_UTF_LEAD_2 = 0xbfU;

        fputc(XML_UTF_LEAD_0, fp);
        fputc(XML_UTF_LEAD_1, fp);
        fputc(XML_UTF_LEAD_2, fp);
    }
    Print(fp, 0);
    return (ferror(fp) == 0);
}

FILE* XMLDoc::OpenXMLFile(const std::string& fileName, const char* mode)
{
    return fopen(fileName.c_str(), mode);
}

void XMLDoc::CopyTo(XMLDoc* target) const
{
    XMLNode::CopyTo(target);

    target->mHasError = mHasError;
    target->mErrorID = mErrorID;
    target->mErrorDesc = mErrorDesc;
    target->mTabsize = mTabsize;
    target->mErrorLocation = mErrorLocation;
    target->mIsUseMicrosoftBOM = mIsUseMicrosoftBOM;
    target->mXMLText = mXMLText;

    XMLNode* node = 0;
    for (node = mFirstChild; node; node = node->GetNextSibling())
    {
        target->LinkEndChild(node->Clone());
    }
}

XMLNode* XMLDoc::Clone() const
{
    XMLDoc* clone = new XMLDoc();
    if (!clone)
    {
        return 0;
    }

    CopyTo(clone);
    return clone;
}

void XMLDoc::StreamIn(std::istream* in, std::string* tag)
{
    // The basic issue with a document is that we don't know what we're
    // streaming. Read something presumed to be a tag (and hope), then
    // identify it, and call the appropriate stream method on the tag.
    // This "pre-streaming" will never read the closing ">" so the
    // sub-tag can orient itself.
    if (!StreamTo(in, '<', tag))
    {
        SetError(XML_ERROR_PARSING_EMPTY, 0, 0, XMLEncodingUnknown);
        return;
    }

    while (in->good())
    {
        int tagIndex = (int) tag->length();
        while (in->good() && in->peek() != '>')
        {
            int c = in->get();
            if (c <= 0)
            {
                SetError(XML_ERROR_EMBEDDED_NULL, 0, 0, XMLEncodingUnknown);
                break;
            }
            (*tag) += (char) c;
        }

        if (in->good())
        {
            // We now have something we presume to be a node of 
            // some sort. Identify it, and call the node to
            // continue streaming.
            XMLNode* node = Identify(tag->c_str() + tagIndex,
                    XMLEncodingDefault);
            if (node)
            {
                node->StreamIn(in, tag);
                bool isElement = node->ToElement() != 0;
                delete node;
                node = 0;

                // If this is the root element, we're done. Parsing will be
                // done by the >> operator.
                if (isElement)
                {
                    return;
                }
            }
            else
            {
                SetError(XML_ERROR, 0, 0, XMLEncodingUnknown);
                return;
            }
        }
    }
    // We should have returned sooner.
    SetError(XML_ERROR, 0, 0, XMLEncodingUnknown);
}

const char* XMLDoc::Parse(const char* p, XMLParsingData* prevData,
        XMLEncoding encoding)
{
    ClearError();

    // Parse away, at the document level. Since a document
    // contains nothing but other tags, most of what happens
    // here is skipping white space.
    if (!p || !*p)
    {
        SetError(XML_ERROR_DOCUMENT_EMPTY, 0, 0, XMLEncodingUnknown);
        return 0;
    }

    // Note that, for a document, this needs to come
    // before the while space skip, so that parsing
    // starts from the pointer we are given.
    mLocation.Clear();
    if (prevData)
    {
        mLocation.row = prevData->cursor.row;
        mLocation.col = prevData->cursor.col;
    }
    else
    {
        mLocation.row = 0;
        mLocation.col = 0;
    }
    XMLParsingData data(p, GetTabSize(), mLocation.row, mLocation.col);
    mLocation = data.Cursor();

    if (encoding == XMLEncodingUnknown)
    {
        // Check for the Microsoft UTF-8 lead bytes.
        const unsigned char* pU = (const unsigned char*) p;
        if (*(pU + 0) && *(pU + 0) == XML_UTF_LEAD_0 && *(pU + 1) && *(pU + 1)
                == XML_UTF_LEAD_1 && *(pU + 2) && *(pU + 2) == XML_UTF_LEAD_2)
        {
            encoding = XMLEncodingUTF8;
            mIsUseMicrosoftBOM = true;
        }
    }

    p = SkipWhiteSpace(p, encoding);
    if (!p)
    {
        SetError(XML_ERROR_DOCUMENT_EMPTY, 0, 0, XMLEncodingUnknown);
        return 0;
    }

    while (p && *p)
    {
        XMLNode* node = Identify(p, encoding);
        if (node)
        {
            p = node->Parse(p, &data, encoding);
            LinkEndChild(node);
        }
        else
        {
            break;
        }

        // Did we get encoding info?
        if (encoding == XMLEncodingUnknown && node->ToDeclaration())
        {
            XMLDeclaration* dec = node->ToDeclaration();
            const char* enc = dec->GetEncoding().c_str();
            assert(enc);

            if (*enc == 0)
            {
                encoding = XMLEncodingUTF8;
            }
            else if (StringEqual(enc, "UTF-8", true, XMLEncodingUnknown))
            {
                encoding = XMLEncodingUTF8;
            }
            else if (StringEqual(enc, "UTF8", true, XMLEncodingUnknown))
            {
                encoding = XMLEncodingUTF8; // incorrect, but be nice
            }
            else
            {
                encoding = XMLEncodingLegacy;
            }
        }

        p = SkipWhiteSpace(p, encoding);
    }

    // Was this empty?
    if (!mFirstChild)
    {
        SetError(XML_ERROR_DOCUMENT_EMPTY, 0, 0, encoding);
        return 0;
    }

    // All is well.
    return p;
}

void XMLDoc::SetError(int err, const char* pError, XMLParsingData* data,
        XMLEncoding encoding)
{
    // The first error in a chain is more accurate - don't set again!
    if (mHasError)
    {
        return;
    }

    assert(err > 0 && err < XML_ERROR_STRING_COUNT);
    mHasError = true;
    mErrorID = err;
    mErrorDesc = mErrorString[mErrorID];

    mErrorLocation.Clear();
    if (pError && data)
    {
        data->Stamp(pError, encoding);
        mErrorLocation = data->Cursor();
    }
}

void XMLDoc::Print(FILE* cfile, int depth) const
{
    assert(cfile);
    for (const XMLNode* node = GetFirstChild(); node; node
            = node->GetNextSibling())
    {
        node->Print(cfile, depth);
        fprintf(cfile, "\n");
    }
}

bool XMLDoc::Accept(XMLVisitor* visitor) const
{
    if (visitor->VisitEnter(*this))
    {
        for (const XMLNode* node = GetFirstChild(); node; node
                = node->GetNextSibling())
        {
            if (!node->Accept(visitor))
            {
                break;
            }
        }
    }
    return visitor->VisitExit(*this);
}

//////////////////////////////////////////////////////////////////////////
// XMLAttribute
// Construct an empty attribute.
XMLAttribute::XMLAttribute() :
    XMLBase()
{
    mDocument = 0;
    mPreviousAttribute = mNextAttribute = 0;
}

// Construct an attribute with a name and value.
XMLAttribute::XMLAttribute(const std::string& name, const std::string& value)
{
    mName = name;
    mValue = value;
    mDocument = 0;
    mPreviousAttribute = mNextAttribute = 0;
}

// Return the value of this attribute.
std::string XMLAttribute::GetValue() const
{
    return mValue;
}

// Return the name of this attribute.
std::string XMLAttribute::GetName() const
{
    return mName;
}

XMLAttribute* XMLAttribute::GetNext() const
{
    // We are using knowledge of the sentinel. The sentinel
    // have a value or name.
    if (mNextAttribute->mValue.empty() && mNextAttribute->mName.empty())
    {
        return 0;
    }
    return mNextAttribute;
}

XMLAttribute* XMLAttribute::GetPrevious() const
{
    // We are using knowledge of the sentinel. The sentinel
    // have a value or name.
    if (mPreviousAttribute->mValue.empty() && mPreviousAttribute->mName.empty())
    {
        return 0;
    }
    return mPreviousAttribute;
}

void XMLAttribute::Print(FILE* cfile, int /*depth*/, std::string* str) const
{
    std::string n, v;

    EncodeString(mName, &n);
    EncodeString(mValue, &v);

    if (mValue.find('\"') == std::string::npos)
    {
        if (cfile)
        {
            fprintf(cfile, "%s=\"%s\"", n.c_str(), v.c_str());
        }
        if (str)
        {
            (*str) += n;
            (*str) += "=\"";
            (*str) += v;
            (*str) += "\"";
        }
    }
    else
    {
        if (cfile)
        {
            fprintf(cfile, "%s='%s'", n.c_str(), v.c_str());
        }
        if (str)
        {
            (*str) += n;
            (*str) += "='";
            (*str) += v;
            (*str) += "'";
        }
    }
}

// Set the document pointer so the attribute can report errors.
void XMLAttribute::SetDocument(XMLDoc* doc)
{
    mDocument = doc;
}

int XMLAttribute::QueryValue(int& result) const
{
    if (sscanf(mValue.c_str(), "%d", &result) == 1)
    {
        return XML_SUCCESS;
    }
    return XML_WRONG_TYPE;
}

int XMLAttribute::QueryValue(double& result) const
{
    if (sscanf(mValue.c_str(), "%lf", &result) == 1)
    {
        return XML_SUCCESS;
    }
    return XML_WRONG_TYPE;
}

int XMLAttribute::QueryValue(std::string& result) const
{
    result = mValue;
    return XML_SUCCESS;
}

void XMLAttribute::SetName(const std::string& name)
{
    mName = name;
}

void XMLAttribute::SetValue(const std::string& value)
{
    mValue = value;
}

void XMLAttribute::SetValue(int value)
{
    char buf[64];
#if defined(XML_SNPRINTF)		
    XML_SNPRINTF(buf, sizeof(buf), "%d", value);
#else
    sprintf (buf, "%d", value);
#endif
    SetValue(buf);
}

void XMLAttribute::SetValue(double value)
{
    char buf[256];
#if defined(XML_SNPRINTF)		
    XML_SNPRINTF(buf, sizeof(buf), "%lf", value);
#else
    sprintf(buf, "%lf", value);
#endif
    SetValue(buf);
}

int XMLAttribute::GetValueInt() const
{
    return atoi(mValue.c_str());
}

double XMLAttribute::GetValueDouble() const
{
    return atof(mValue.c_str());
}

const char* XMLAttribute::Parse(const char* p, XMLParsingData* data,
        XMLEncoding encoding)
{
    p = SkipWhiteSpace(p, encoding);
    if (!p || !*p)
    {
        return 0;
    }

    if (data)
    {
        data->Stamp(p, encoding);
        mLocation = data->Cursor();
    }
    // Read the name, the '=' and the value.
    const char* pErr = p;
    p = ReadName(p, &mName, encoding);
    if (!p || !*p)
    {
        if (mDocument)
        {
            mDocument->SetError(XML_ERROR_READING_ATTRIBUTES, pErr, data,
                    encoding);
        }

        return 0;
    }
    p = SkipWhiteSpace(p, encoding);
    if (!p || !*p || *p != '=')
    {
        if (mDocument)
        {
            mDocument->SetError(XML_ERROR_READING_ATTRIBUTES, p, data, encoding);
        }

        return 0;
    }

    ++p; // skip '='
    p = SkipWhiteSpace(p, encoding);
    if (!p || !*p)
    {
        if (mDocument)
        {
            mDocument->SetError(XML_ERROR_READING_ATTRIBUTES, p, data, encoding);
        }

        return 0;
    }

    const char* end;
    const char SINGLE_QUOTE = '\'';
    const char DOUBLE_QUOTE = '\"';

    if (*p == SINGLE_QUOTE)
    {
        ++p;
        end = "\'"; // single quote in string
        p = ReadText(p, &mValue, false, end, false, encoding);
    }
    else if (*p == DOUBLE_QUOTE)
    {
        ++p;
        end = "\""; // double quote in string
        p = ReadText(p, &mValue, false, end, false, encoding);
    }
    else
    {
        // All attribute values should be in single or double quotes.
        // But this is such a common error that the parser will try
        // its best, even without them.
        mValue = "";
        while (p && *p // existence
                && !IsWhiteSpace(*p) && *p != '\n' && *p != '\r' // whitespace
                && *p != '/' && *p != '>') // tag end
        {
            if (*p == SINGLE_QUOTE || *p == DOUBLE_QUOTE)
            {
                // [1451649] Attribute values with trailing quotes not handled correctly
                // We did not have an opening quote but seem to have a 
                // closing one. Give up and throw an error.
                if (mDocument)
                {
                    mDocument->SetError(XML_ERROR_READING_ATTRIBUTES, p, data,
                            encoding);
                }
                return 0;
            }
            mValue += *p;
            ++p;
        }
    }
    return p;
}

//////////////////////////////////////////////////////////////////////////
// XMLComment
XMLComment::XMLComment(const XMLComment& copy) :
    XMLNode(XMLNode::COMMENT)
{
    copy.CopyTo(this);
}

void XMLComment::operator=(const XMLComment& base)
{
    Clear();
    base.CopyTo(this);
}

void XMLComment::Print(FILE* cfile, int depth) const
{
    assert(cfile);
    for (int i = 0; i < depth; i++)
    {
        fprintf(cfile, "    ");
    }
    fprintf(cfile, "<!--%s-->", mValue.c_str());
}

void XMLComment::CopyTo(XMLComment* target) const
{
    XMLNode::CopyTo(target);
}

bool XMLComment::Accept(XMLVisitor* visitor) const
{
    return visitor->Visit(*this);
}

XMLNode* XMLComment::Clone() const
{
    XMLComment* clone = new XMLComment();

    if (!clone)
    {
        return 0;
    }

    CopyTo(clone);
    return clone;
}

void XMLComment::StreamIn(std::istream* in, std::string* tag)
{
    while (in->good())
    {
        int c = in->get();
        if (c <= 0)
        {
            XMLDoc* document = GetDocument();
            if (document)
            {
                document->SetError(XML_ERROR_EMBEDDED_NULL, 0, 0,
                        XMLEncodingUnknown);
            }
            return;
        }

        (*tag) += (char) c;

        if (c == '>' && tag->at(tag->length() - 2) == '-' && tag->at(
                tag->length() - 3) == '-')
        {
            // All is well.
            return;
        }
    }
}

const char* XMLComment::Parse(const char* p, XMLParsingData* data,
        XMLEncoding encoding)
{
    XMLDoc* document = GetDocument();
    mValue = "";

    p = SkipWhiteSpace(p, encoding);

    if (data)
    {
        data->Stamp(p, encoding);
        mLocation = data->Cursor();
    }
    const char* startTag = "<!--";
    const char* endTag = "-->";

    if (!StringEqual(p, startTag, false, encoding))
    {
        document->SetError(XML_ERROR_PARSING_COMMENT, p, data, encoding);
        return 0;
    }
    p += strlen(startTag);

    // [ 1475201 ] XML parses entities in comments
    // Oops - ReadText doesn't work, because we don't want to parse the entities.
    // p = ReadText( p, &value, false, endTag, false, encoding );
    //
    // from the XML spec:
    /*
     [Definition: Comments may appear anywhere in a document outside other markup; in addition,
     they may appear within the document type declaration at places allowed by the grammar.
     They are not part of the document's character data; an XML processor MAY, but need not,
     make it possible for an application to retrieve the text of comments. For compatibility,
     the string "--" (double-hyphen) MUST NOT occur within comments.] Parameter entity
     references MUST NOT be recognized within comments.
     An example of a comment:
     <!-- declarations for <head> & <body> -->
     */

    mValue = "";
    // Keep all the white space.
    while (p && *p && !StringEqual(p, endTag, false, encoding))
    {
        mValue.append(p, 1);
        ++p;
    }
    if (p)
    {
        p += strlen(endTag);
    }

    return p;
}

//////////////////////////////////////////////////////////////////////////
// XMLText
void XMLText::Print(FILE* cfile, int depth) const
{
    assert(cfile);
    if (mIsCDATA)
    {
        int i;
        fprintf(cfile, "\n");
        for (i = 0; i < depth; i++)
        {
            fprintf(cfile, "    ");
        }
        fprintf(cfile, "<![CDATA[%s]]>\n", mValue.c_str()); // unformatted output
    }
    else
    {
        std::string buffer;
        EncodeString(mValue, &buffer);
        fprintf(cfile, "%s", buffer.c_str());
    }
}

void XMLText::CopyTo(XMLText* target) const
{
    XMLNode::CopyTo(target);
    target->mIsCDATA = mIsCDATA;
}

bool XMLText::Accept(XMLVisitor* visitor) const
{
    return visitor->Visit(*this);
}

XMLNode* XMLText::Clone() const
{
    XMLText* clone = 0;
    clone = new XMLText("");

    if (!clone)
    {
        return 0;
    }

    CopyTo(clone);
    return clone;
}

void XMLText::StreamIn(std::istream* in, std::string* tag)
{
    while (in->good())
    {
        int c = in->peek();
        if (!mIsCDATA && (c == '<'))
        {
            return;
        }
        if (c <= 0)
        {
            XMLDoc* document = GetDocument();
            if (document)
            {
                document->SetError(XML_ERROR_EMBEDDED_NULL, 0, 0,
                        XMLEncodingUnknown);
            }
            return;
        }

        (*tag) += (char) c;
        in->get(); // "commits" the peek made above

        if (mIsCDATA && c == '>' && tag->size() >= 3)
        {
            size_t len = tag->size();
            if ((*tag)[len - 2] == ']' && (*tag)[len - 3] == ']')
            {
                // terminator of cdata.
                return;
            }
        }
    }
}

const char* XMLText::Parse(const char* p, XMLParsingData* data,
        XMLEncoding encoding)
{
    mValue = "";
    XMLDoc* document = GetDocument();

    if (data)
    {
        data->Stamp(p, encoding);
        mLocation = data->Cursor();
    }

    const char* const startTag = "<![CDATA[";
    const char* const endTag = "]]>";

    if (mIsCDATA || StringEqual(p, startTag, false, encoding))
    {
        mIsCDATA = true;

        if (!StringEqual(p, startTag, false, encoding))
        {
            document->SetError(XML_ERROR_PARSING_CDATA, p, data, encoding);
            return 0;
        }
        p += strlen(startTag);

        // Keep all the white space, ignore the encoding, etc.
        while (p && *p && !StringEqual(p, endTag, false, encoding))
        {
            mValue += *p;
            ++p;
        }

        std::string dummy;
        p = ReadText(p, &dummy, false, endTag, false, encoding);
        return p;
    }
    else
    {
        bool ignoreWhite = true;
        const char* end = "<";
        p = ReadText(p, &mValue, ignoreWhite, end, false, encoding);
        if (p)
        {
            return p - 1; // don't truncate the '<'
        }

        return 0;
    }
}

bool XMLText::IsBlank() const
{
    for (unsigned int i = 0; i < mValue.length(); i++)
    {
        if (!IsWhiteSpace(mValue[i]))
        {
            return false;
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
// XMLDeclaration
XMLDeclaration::XMLDeclaration(const std::string& version,
        const std::string& encoding, const std::string& standalone) :
    XMLNode(XMLNode::DECLARATION)
{
    mVersion = version;
    mEncoding = encoding;
    mStandalone = standalone;
}

XMLDeclaration::XMLDeclaration(const XMLDeclaration& copy) :
    XMLNode(XMLNode::DECLARATION)
{
    copy.CopyTo(this);
}

void XMLDeclaration::operator=(const XMLDeclaration& copy)
{
    Clear();
    copy.CopyTo(this);
}

void XMLDeclaration::Print(FILE* cfile, int /*depth*/, std::string* str) const
{
    if (cfile)
    {
        fprintf(cfile, "<?xml ");
    }
    if (str)
    {
        (*str) += "<?xml ";
    }

    if (!mVersion.empty())
    {
        if (cfile)
        {
            fprintf(cfile, "version=\"%s\" ", mVersion.c_str());
        }
        if (str)
        {
            (*str) += "version=\"";
            (*str) += mVersion;
            (*str) += "\" ";
        }
    }
    if (!mEncoding.empty())
    {
        if (cfile)
        {
            fprintf(cfile, "encoding=\"%s\" ", mEncoding.c_str());
        }
        if (str)
        {
            (*str) += "encoding=\"";
            (*str) += mEncoding;
            (*str) += "\" ";
        }
    }
    if (!mStandalone.empty())
    {
        if (cfile)
        {
            fprintf(cfile, "standalone=\"%s\" ", mStandalone.c_str());
        }
        if (str)
        {
            (*str) += "standalone=\"";
            (*str) += mStandalone;
            (*str) += "\" ";
        }
    }
    if (cfile)
    {
        fprintf(cfile, "?>");
    }
    if (str)
    {
        (*str) += "?>";
    }
}

void XMLDeclaration::CopyTo(XMLDeclaration* target) const
{
    XMLNode::CopyTo(target);

    target->mVersion = mVersion;
    target->mEncoding = mEncoding;
    target->mStandalone = mStandalone;
}

bool XMLDeclaration::Accept(XMLVisitor* visitor) const
{
    return visitor->Visit(*this);
}

XMLNode* XMLDeclaration::Clone() const
{
    XMLDeclaration* clone = new XMLDeclaration();
    if (!clone)
    {
        return 0;
    }

    CopyTo(clone);
    return clone;
}

void XMLDeclaration::StreamIn(std::istream* in, std::string* tag)
{
    while (in->good())
    {
        int c = in->get();
        if (c <= 0)
        {
            XMLDoc* document = GetDocument();
            if (document)
            {
                document->SetError(XML_ERROR_EMBEDDED_NULL, 0, 0,
                        XMLEncodingUnknown);
            }
            return;
        }
        (*tag) += (char) c;

        if (c == '>')
        {
            // All is well.
            return;
        }
    }
}

const char* XMLDeclaration::Parse(const char* p, XMLParsingData* data,
        XMLEncoding encoding)
{
    p = SkipWhiteSpace(p, encoding);
    // Find the beginning, find the end, and look for the stuff in-between.
    XMLDoc* document = GetDocument();
    if (!p || !*p || !StringEqual(p, "<?xml", true, encoding))
    {
        if (document)
        {
            document->SetError(XML_ERROR_PARSING_DECLARATION, 0, 0, encoding);
        }
        return 0;
    }
    if (data)
    {
        data->Stamp(p, encoding);
        mLocation = data->Cursor();
    }
    p += 5;

    mVersion = "";
    mEncoding = "";
    mStandalone = "";

    while (p && *p)
    {
        if (*p == '>')
        {
            ++p;
            return p;
        }

        p = SkipWhiteSpace(p, encoding);
        if (StringEqual(p, "version", true, encoding))
        {
            XMLAttribute attrib;
            p = attrib.Parse(p, data, encoding);
            mVersion = attrib.GetValue();
        }
        else if (StringEqual(p, "encoding", true, encoding))
        {
            XMLAttribute attrib;
            p = attrib.Parse(p, data, encoding);
            mEncoding = attrib.GetValue();
        }
        else if (StringEqual(p, "standalone", true, encoding))
        {
            XMLAttribute attrib;
            p = attrib.Parse(p, data, encoding);
            mStandalone = attrib.GetValue();
        }
        else
        {
            // Read over whatever it is.
            while (p && *p && *p != '>' && !IsWhiteSpace(*p))
            {
                ++p;
            }
        }
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
// XMLUnknown
void XMLUnknown::StreamIn(std::istream* in, std::string* tag)
{
    while (in->good())
    {
        int c = in->get();
        if (c <= 0)
        {
            XMLDoc* document = GetDocument();
            if (document)
            {
                document->SetError(XML_ERROR_EMBEDDED_NULL, 0, 0,
                        XMLEncodingUnknown);
            }
            return;
        }
        (*tag) += (char) c;

        if (c == '>')
        {
            // All is well.
            return;
        }
    }
}

const char* XMLUnknown::Parse(const char* p, XMLParsingData* data,
        XMLEncoding encoding)
{
    XMLDoc* document = GetDocument();
    p = SkipWhiteSpace(p, encoding);

    if (data)
    {
        data->Stamp(p, encoding);
        mLocation = data->Cursor();
    }
    if (!p || !*p || *p != '<')
    {
        if (document)
        {
            document->SetError(XML_ERROR_PARSING_UNKNOWN, p, data, encoding);
        }
        return 0;
    }
    ++p;
    mValue = "";

    while (p && *p && *p != '>')
    {
        mValue += *p;
        ++p;
    }

    if (!p)
    {
        if (document)
        {
            document->SetError(XML_ERROR_PARSING_UNKNOWN, 0, 0, encoding);
        }
    }
    if (*p == '>')
    {
        return p + 1;
    }

    return p;
}

void XMLUnknown::Print(FILE* cfile, int depth) const
{
    for (int i = 0; i < depth; i++)
    {
        fprintf(cfile, "    ");
    }
    fprintf(cfile, "<%s>", mValue.c_str());
}

void XMLUnknown::CopyTo(XMLUnknown* target) const
{
    XMLNode::CopyTo(target);
}

bool XMLUnknown::Accept(XMLVisitor* visitor) const
{
    return visitor->Visit(*this);
}

XMLNode* XMLUnknown::Clone() const
{
    XMLUnknown* clone = new XMLUnknown();
    if (!clone)
    {
        return 0;
    }

    CopyTo(clone);
    return clone;
}

//////////////////////////////////////////////////////////////////////////
// XMLAttributeSet
XMLAttributeSet::XMLAttributeSet()
{
    mSentinel.mNextAttribute = &mSentinel;
    mSentinel.mPreviousAttribute = &mSentinel;
}

XMLAttributeSet::~XMLAttributeSet()
{
    assert(mSentinel.mNextAttribute == &mSentinel);
    assert(mSentinel.mPreviousAttribute == &mSentinel);
}

void XMLAttributeSet::Add(XMLAttribute* addMe)
{
    assert(!Find(std::string(addMe->GetName())));// Shouldn't be multiply adding to the set.
    addMe->mNextAttribute = &mSentinel;
    addMe->mPreviousAttribute = mSentinel.mPreviousAttribute;

    mSentinel.mPreviousAttribute->mNextAttribute = addMe;
    mSentinel.mPreviousAttribute = addMe;
}

void XMLAttributeSet::Remove(XMLAttribute* removeMe)
{
    XMLAttribute* node;
    for (node = mSentinel.mNextAttribute; node != &mSentinel; node
            = node->mNextAttribute)
    {
        if (node == removeMe)
        {
            node->mPreviousAttribute->mNextAttribute = node->mNextAttribute;
            node->mNextAttribute->mPreviousAttribute = node->mPreviousAttribute;
            node->mNextAttribute = 0;
            node->mPreviousAttribute = 0;
            return;
        }
    }
    assert(0); // we tried to remove a non-linked attribute.
}

XMLAttribute* XMLAttributeSet::Find(const std::string& name) const
{
    for (XMLAttribute* node = mSentinel.mNextAttribute; node != &mSentinel; node
            = node->mNextAttribute)
    {
        if (strcasecmp(node->mName.c_str(), name.c_str()) == 0)
        {
            return node;
        }
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
std::istream& operator>>(std::istream & in, XMLNode & base)
{
    std::string tag;
    tag.reserve(8 * 1000);
    base.StreamIn(&in, &tag);

    base.Parse(tag.c_str(), 0, XMLEncodingDefault);
    return in;
}

std::ostream& operator<<(std::ostream & out, const XMLNode & base)
{
    XMLPrinter printer;
    printer.SetStreamPrinting();
    base.Accept(&printer);
    out << printer.Str();

    return out;
}

std::string& operator<<(std::string& out, const XMLNode& base)
{
    XMLPrinter printer;
    printer.SetStreamPrinting();
    base.Accept(&printer);
    out.append(printer.Str());

    return out;
}

//////////////////////////////////////////////////////////////////////////
// XMLHandle
XMLHandle XMLHandle::GetFirstChild() const
{
    if (mNode)
    {
        XMLNode* child = mNode->GetFirstChild();
        if (child)
        {
            return XMLHandle(child);
        }
    }
    return XMLHandle(0);
}

XMLHandle XMLHandle::GetFirstChild(const std::string& value) const
{
    if (mNode)
    {
        XMLNode* child = mNode->GetFirstChild(value);
        if (child)
        {
            return XMLHandle(child);
        }
    }
    return XMLHandle(0);
}

XMLHandle XMLHandle::GetFirstChildElement() const
{
    if (mNode)
    {
        XMLElement* child = mNode->GetFirstChildElement();
        if (child)
        {
            return XMLHandle(child);
        }
    }
    return XMLHandle(0);
}

XMLHandle XMLHandle::GetFirstChildElement(const std::string& value) const
{
    if (mNode)
    {
        XMLElement* child = mNode->GetFirstChildElement(value);
        if (child)
        {
            return XMLHandle(child);
        }
    }
    return XMLHandle(0);
}

XMLHandle XMLHandle::GetChild(int count) const
{
    if (mNode)
    {
        int i;
        XMLNode* child = mNode->GetFirstChild();
        for (i = 0; child && i < count; child = child->GetNextSibling(), ++i)
        {
            // nothing
            ;
        }
        if (child)
        {
            return XMLHandle(child);
        }
    }
    return XMLHandle(0);
}

XMLHandle XMLHandle::GetChild(const std::string& value, int count) const
{
    if (mNode)
    {
        int i;
        XMLNode* child = mNode->GetFirstChild(value);
        for (i = 0; child && i < count; child = child->GetNextSibling(value), ++i)
        {
            // nothing
            ;
        }
        if (child)
        {
            return XMLHandle(child);
        }
    }
    return XMLHandle(0);
}

XMLHandle XMLHandle::GetChildElement(int count) const
{
    if (mNode)
    {
        int i;
        XMLElement* child = mNode->GetFirstChildElement();
        for (i = 0; child && i < count; child = child->GetNextSiblingElement(), ++i)
        {
            // nothing
            ;
        }
        if (child)
        {
            return XMLHandle(child);
        }
    }
    return XMLHandle(0);
}

XMLHandle XMLHandle::GetChildElement(const std::string& value, int count) const
{
    if (mNode)
    {
        int i;
        XMLElement* child = mNode->GetFirstChildElement(value);
        for (i = 0; child && i < count; child = child->GetNextSiblingElement(
                value), ++i)
        {
            // nothing
            ;
        }
        if (child)
        {
            return XMLHandle(child);
        }
    }
    return XMLHandle(0);
}

//////////////////////////////////////////////////////////////////////////
// XMLPrinter
bool XMLPrinter::VisitEnter(const XMLDoc&)
{
    return true;
}

bool XMLPrinter::VisitExit(const XMLDoc&)
{
    return true;
}

bool XMLPrinter::VisitEnter(const XMLElement& element,
        const XMLAttribute* firstAttribute)
{
    DoIndent();
    mBuffer += "<";
    mBuffer += element.GetValue();

    for (const XMLAttribute* attrib = firstAttribute; attrib; attrib
            = attrib->GetNext())
    {
        mBuffer += " ";
        attrib->Print(0, 0, &mBuffer);
    }

    if (!element.GetFirstChild())
    {
        mBuffer += " />";
        DoLineBreak();
    }
    else
    {
        mBuffer += ">";
        if (element.GetFirstChild()->ToText() && element.GetLastChild()
                == element.GetFirstChild()
                && element.GetFirstChild()->ToText()->CDATA() == false)
        {
            mSimpleTextPrint = true;
            // no DoLineBreak()!
        }
        else
        {
            DoLineBreak();
        }
    }
    ++mDepth;
    return true;
}

bool XMLPrinter::VisitExit(const XMLElement& element)
{
    --mDepth;
    if (!element.GetFirstChild())
    {
        // nothing.
        ;
    }
    else
    {
        if (mSimpleTextPrint)
        {
            mSimpleTextPrint = false;
        }
        else
        {
            DoIndent();
        }
        mBuffer += "</";
        mBuffer += element.GetValue();
        mBuffer += ">";
        DoLineBreak();
    }
    return true;
}

bool XMLPrinter::Visit(const XMLText& text)
{
    if (text.CDATA())
    {
        DoIndent();
        mBuffer += "<![CDATA[";
        mBuffer += text.GetValue();
        mBuffer += "]]>";
        DoLineBreak();
    }
    else if (mSimpleTextPrint)
    {
        std::string str;
        XMLBase::EncodeString(text.GetValue(), &str);
        mBuffer += str;
    }
    else
    {
        DoIndent();
        std::string str;
        XMLBase::EncodeString(text.GetValue(), &str);
        mBuffer += str;
        DoLineBreak();
    }
    return true;
}

bool XMLPrinter::Visit(const XMLDeclaration& declaration)
{
    DoIndent();
    declaration.Print(0, 0, &mBuffer);
    DoLineBreak();
    return true;
}

bool XMLPrinter::Visit(const XMLComment& comment)
{
    DoIndent();
    mBuffer += "<!--";
    mBuffer += comment.GetValue();
    mBuffer += "-->";
    DoLineBreak();
    return true;
}

bool XMLPrinter::Visit(const XMLUnknown& unknown)
{
    DoIndent();
    mBuffer += "<";
    mBuffer += unknown.GetValue();
    mBuffer += ">";
    DoLineBreak();
    return true;
}
