//////////////////////////////////////////////////////////////////////////
// ErrorCodes.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef ErrorCodes_INCLUDED
#define ErrorCodes_INCLUDED

enum ErrorCode
{
    // No error
    ErrorOK = 0,

    // File related
    ErrorFileIO,
    ErrorFileAccessDenied,
    ErrorFileNotFound,
    ErrorFileNotDirectory,
    ErrorFileNotFile,
    ErrorFileReadOnly,
    ErrorFileExists,
    ErrorFileNoSpaceLeft,
    ErrorFileQuotaExceeded,
    ErrorFileDirectoryNotEmpty,
    ErrorFilePathSyntax,
    ErrorFileTableOverflow,
    ErrorFileTooManyOpening,
    ErrorFile,

    // Network related
    ErrorNetworkSysNotReady,
    ErrorNetworkSysNotInit,
    ErrorNetworkInterrupted,
    ErrorNetworkPermissionDenied,
    ErrorNetworkBadAddress,
    ErrorNetworkTooManyOpening,
    ErrorNetworkOperationBlock,
    ErrorNetworkOperationInProgress,
    ErrorNetworkOperationAlready,
    ErrorNetworkNonSocket,
    ErrorNetworkDestAddrRequired,
    ErrorNetworkMessageTooLong,
    ErrorNetworkWrongProtocol,
    ErrorNetworkProtocolNotAvailable,
    ErrorNetworkProtocolNotSupported,
    ErrorNetworkSocketNotSupported,
    ErrorNetworkOperationNotSupported,
    ErrorNetworkProtocolFamilyNotSupported,
    ErrorNetworkAddrFamilyNotSupported,
    ErrorNetworkAddrInUse,
    ErrorNetworkAddrNotAvailable,
    ErrorNetworkDown,
    ErrorNetworkUnreachable,
    ErrorNetworkReset,
    ErrorNetworkConnectionAborted,
    ErrorNetworkConnectionReset,
    ErrorNetworkNoBufferSpace,
    ErrorNetworkSocketIsConnected,
    ErrorNetworkSocketIsNotConnected,
    ErrorNetworkShutdown,
    ErrorNetworkTimeout,
    ErrorNetworkConnectionRefused,
    ErrorNetworkHostDown,
    ErrorNetworkHostUnreachable,
    ErrorNetworkHostNotFound,
    ErrorNetworkTryAgain,
    ErrorNetworkNoRecoverable,
    ErrorNetworkNoAddrFound,
    ErrorNetworkInterfaceNotFound,
    ErrorNetworkSocketException,
    ErrorNetwork,

    // CFS related
    ErrorDNSServerInvalid,
    ErrorCFSServerInvalid,
    ErrorCFSServerNotConfigured,
    ErrorCFSServerTimeout,
    ErrorBadDNSReply,
    ErrorBadUDPReply,
    ErrorBadUDPSignature,
    ErrorCryptoFailed,
    ErrorUrlUnrated,
    ErrorUrlAllowed,
    ErrorUrlForbidden,
    ErrorKeywordsForbidden,
    ErrorCategoryForbidden,

    //LocalCFSDB DB Service module
    ErrorDBServiceInternalException,
    ErrorDBServiceUserID,
    ErrorDBServiceRequestType,
    ErrorDBServiceDomain,
    ErrorDBServicePort,
    ErrorDBServicePath,

    // Common
    ErrorInvalidArgument,
    ErrorNotImplemented,

    ErrorUnknown,
};

#endif // ErrorCodes_INCLUDED
