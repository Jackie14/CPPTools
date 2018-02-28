//////////////////////////////////////////////////////////////////////////
// DNSHeaders.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef DNS_INCLUDED
#define DNS_INCLUDED


struct DNSHeader
{
	unsigned short id;
	unsigned short flags;
	unsigned short qdCount;
	unsigned short anCount;
	unsigned short nsCount;
	unsigned short arCount;
};

struct DNSQuery
{
	// Query string should be here
	unsigned short type;
	unsigned short classes;
};

#pragma pack(2)
struct DNSMXRR
{
	unsigned short cp __attribute__((packed));
	unsigned short type __attribute__((packed));
	unsigned short classType __attribute__((packed));
	unsigned int ttl __attribute__((packed));
	unsigned short rdLen __attribute__((packed));
	unsigned short prefs __attribute__((packed));
	char rd[0];
};
#pragma pack()

// These don't have to be in the same order as in the packet flags word,
// and they can even overlap in some cases, but they will need to be kept
// in synch with ns_parse.c:ns_flagdata[].
enum NSFlag
{
	NSFlagQuestionResponse, // Question/Response.
	NSFlagOpCode, // Operation code.
	NSFlagAuthAnswer, // Authoritative Answer.
	NSFlagTruncationOccur, // Truncation occurred.
	NSFlagRecursionDesired, // Recursion Desired.
	NSFlagRecursionAvailable, // Recursion Available.
	NSFlagMBZ,	// MBZ.
	NSFlagAuthenticData, // Authentic Data (DNSSEC).
	NSFlagCheckingDisabled, // Checking Disabled (DNSSEC).
	NSFlagResponseCode, // Response code.
	NSFlagMax
};

// Currently defined opcodes.
enum NSOpCode
{
	NSOpCodeQuery = 0,	// Standard query.
	NSOpCodeInverseQuery = 1, // Inverse query (depreciated/unsupported).
	NSOpCodeStatus = 2, // Name server status query (unsupported).
	// Opcode 3 is undefined/reserved.
	NSOpCodeNotify = 4, // Zone change notification.
	NSOpCodeUpdate = 5, // Zone update message.
	NSOpCodeMax = 6
};

// Currently defined response codes.
enum NSResponseCode
{
	NSResponseNoError = 0, // No error occurred.
	NSResponseFormatError = 1, // Format error.
	NSResponseServerFailure = 2, // Server failure.
	NSResponseDomainError = 3, // Name error.
	NSResponseNotImpl = 4, // Unimplemented.
	NSResponseRefused = 5, // Operation refused.
	// these are for BIND_UPDATE
	NSResponseNameExists = 6, // Name exists
	NSResponseRRsetExists = 7, // RRset exists
	NSResponseRRsetNotExists = 8, // RRset does not exist
	NSResponseNotAuth = 9, // Not authoritative for zone
	NSResponseNotZone = 10, // Zone of record different from zone section
	NSResponseMax = 11
};

// Currently defined type values for resources and queries.
enum NSType
{
	NSTypeHostAddr = 1, // Host address.
	NSTypeAuthServer = 2, // Authoritative server.
	NSTypeMailDest = 3, // Mail destination.
	NSTypeMailForwarder = 4, // Mail forwarder.
	NSTypeCanonicalName = 5, // Canonical name.
	NSTypeStartAuthZone = 6, // Start of authority zone.
	NSTypeMailbox = 7, // Mailbox domain name.
	NSTypeMailGroup = 8, // Mail group member.
	NSTypeMailRename = 9, // Mail rename name.
	NSTypeNull = 10, // Null resource record.
	NSTypeWellKnownService = 11, // Well known service.
	NSTypeDomainPtr = 12, // Domain name pointer.
	NSTypeHostInfo = 13, // Host information.
	NSTypeMailInfo = 14, // Mailbox information.
	NSTypeMX = 15, // Mail routing information.
	NSTypeText = 16, // Text strings.
	NSTypeResponsiblePerson = 17, // Responsible person.
	NSTypeAFSDB = 18, // AFS cell database.
	NSTypeX25 = 19, // X_25 calling address.
	NSTypeISDN = 20, // ISDN calling address.
	NSTypeRouter = 21, // Router.
	NSTypeNSAPAddr = 22, // NSAP address.
	NSTypeNSAPPtr = 23, // Reverse NSAP lookup (depreciated).
	NSTypeSecuritySig = 24, // Security signature.
	NSTypeSecurityKey = 25, // Security key.
	NSTypeX400Mail = 26, // X.400 mail mapping.
	NSTypeGeoPosition = 27, // Geographical position (withdrawn).
	NSTypeIP6Addr = 28, // Ip6 Address.
	NSTypeLocation = 29, // Location Information.
	NSTypeNextDomain = 30, // Next domain (security).
	NSTypeEndpointID = 31, // Endpoint identifier.
	NSTypeNimrodLocator = 32, // Nimrod Locator.
	NSTypeServerSelection = 33, // Server Selection.
	NSTypeATMAddr = 34, // ATM Address
	NSTypeNamingAuth = 35, // Naming Authority PoinTeR
	// Query type values which do not appear in resource records.
	NSTypeIncreZoneTransfer = 251, // Incremental zone transfer.
	NSTypeTransferZoneAuth = 252, // Transfer zone of authority.
	NSTypeTransferMailRecords = 253, // Transfer mailbox records.
	NSTypeTransferMailAgentRecords = 254, // Transfer mail agent records.
	NSTypeAny = 255, // Wildcard match.
	NSTypeMax = 65536
};

// Values for classType field
enum NSClassType
{
	NSClassInternet = 1, // Internet.
	// Class 2 unallocated/unsupported.
	NSClassChaos = 3, // MIT Chaos-net.
	NSClassHesiod = 4, // MIT Hesiod.
	// Query class values which do not appear in resource records
	NSClassNone = 254, // for prereq. sections in update requests
	NSClassAny = 255, // Wildcard match.
	NSClassMax = 65536
};

#endif // DNS_INCLUDED

