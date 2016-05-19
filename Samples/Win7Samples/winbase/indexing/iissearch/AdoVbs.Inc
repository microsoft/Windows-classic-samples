<%
'--------------------------------------------------------------------
' Microsoft ADO
'
' (c) 1996 Microsoft Corporation.  All Rights Reserved.
'
'
'
' ADO constants include file for VBScript
'
'--------------------------------------------------------------------

'---- CursorTypeEnum Values ----
Const adOpenForwardOnly = 0
Const adOpenKeyset = 1
Const adOpenDynamic = 2
Const adOpenStatic = 3

'---- CursorOptionEnum Values ----
Const adHoldRecords = &H00000100
Const adMovePrevious = &H00000200
Const adAddNew = &H01000400
Const adDelete = &H01000800
Const adUpdate = &H01008000
Const adBookmark = &H00002000
Const adApproxPosition = &H00004000
Const adUpdateBatch = &H00010000
Const adResync = &H00020000
Const adNotify = &H00040000

'---- LockTypeEnum Values ----
Const adLockReadOnly = 1
Const adLockPessimistic = 2
Const adLockOptimistic = 3
Const adLockBatchOptimistic = 4

'---- ExecuteOptionEnum Values ----
Const adRunAsync = &H00000010

'---- ObjectStateEnum Values ----
Const adStateClosed = &H00000000
Const adStateOpen = &H00000001
Const adStateConnecting = &H00000002
Const adStateExecuting = &H00000004

'---- CursorLocationEnum Values ----
Const adUseNone = 1
Const adUseServer = 2
Const adUseClient = 3
Const adUseClientBatch = 3

'---- DataTypeEnum Values ----
Const adEmpty = 0
Const adTinyInt = 16
Const adSmallInt = 2
Const adInteger = 3
Const adBigInt = 20
Const adUnsignedTinyInt = 17
Const adUnsignedSmallInt = 18
Const adUnsignedInt = 19
Const adUnsignedBigInt = 21
Const adSingle = 4
Const adDouble = 5
Const adCurrency = 6
Const adDecimal = 14
Const adNumeric = 131
Const adBoolean = 11
Const adError = 10
Const adUserDefined = 132
Const adVariant = 12
Const adIDispatch = 9
Const adIUnknown = 13
Const adGUID = 72
Const adDate = 7
Const adDBDate = 133
Const adDBTime = 134
Const adDBTimeStamp = 135
Const adBSTR = 8
Const adChar = 129
Const adVarChar = 200
Const adLongVarChar = 201
Const adWChar = 130
Const adVarWChar = 202
Const adLongVarWChar = 203
Const adBinary = 128
Const adVarBinary = 204
Const adLongVarBinary = 205

'---- FieldAttributeEnum Values ----
Const adFldMayDefer = &H00000002
Const adFldUpdatable = &H00000004
Const adFldUnknownUpdatable = &H00000008
Const adFldFixed = &H00000010
Const adFldIsNullable = &H00000020
Const adFldMayBeNull = &H00000040
Const adFldLong = &H00000080
Const adFldRowID = &H00000100
Const adFldRowVersion = &H00000200
Const adFldCacheDeferred = &H00001000

'---- EditModeEnum Values ----
Const adEditNone = &H0000
Const adEditInProgress = &H0001
Const adEditAdd = &H0002

'---- RecordStatusEnum Values ----
Const adRecOK = &H0000000
Const adRecNew = &H0000001
Const adRecModified = &H0000002
Const adRecDeleted = &H0000004
Const adRecUnmodified = &H0000008
Const adRecInvalid = &H0000010
Const adRecMultipleChanges = &H0000040
Const adRecPendingChanges = &H0000080
Const adRecCanceled = &H0000100
Const adRecCantRelease = &H0000400
Const adRecConcurrencyViolation = &H0000800
Const adRecIntegrityViolation = &H0001000
Const adRecMaxChangesExceeded = &H0002000
Const adRecObjectOpen = &H0004000
Const adRecOutOfMemory = &H0008000
Const adRecPermissionDenied = &H0010000
Const adRecSchemaViolation = &H0020000
Const adRecDBDeleted = &H0040000

'---- GetRowsOptionEnum Values ----
Const adGetRowsRest = -1

'---- PositionEnum Values ----
Const adPosUnknown = -1
Const adPosBOF = -2
Const adPosEOF = -3

'---- enum Values ----
Const adBookmarkCurrent = 0
Const adBookmarkFirst = 1
Const adBookmarkLast = 2

'---- MarshalOptionsEnum Values ----
Const adMarshalAll = 0
Const adMarshalModifiedOnly = 1

'---- AffectEnum Values ----
Const adAffectCurrent = 1
Const adAffectGroup = 2
Const adAffectAll = 3

'---- FilterGroupEnum Values ----
Const adFilterNone = 0
Const adFilterPendingRecords = 1
Const adFilterAffectedRecords = 2
Const adFilterFetchedRecords = 3
Const adFilterPredicate = 4

'---- SearchDirection Values ----
Const adSearchForward = 0
Const adSearchBackward = 1

'---- ConnectPromptEnum Values ----
Const adPromptAlways = 1
Const adPromptComplete = 2
Const adPromptCompleteRequired = 3
Const adPromptNever = 4

'---- ConnectModeEnum Values ----
Const adModeUnknown = 0
Const adModeRead = 1
Const adModeWrite = 2
Const adModeReadWrite = 3
Const adModeShareDenyRead = 4
Const adModeShareDenyWrite = 8
Const adModeShareExclusive = &Hc
Const adModeShareDenyNone = &H10

'---- IsolationLevelEnum Values ----
Const adXactUnspecified = &Hffffffff
Const adXactChaos = &H00000010
Const adXactReadUncommitted = &H00000100
Const adXactBrowse = &H00000100
Const adXactCursorStability = &H00001000
Const adXactReadCommitted = &H00001000
Const adXactRepeatableRead = &H00010000
Const adXactSerializable = &H00100000
Const adXactIsolated = &H00100000

'---- XactAttributeEnum Values ----
Const adXactCommitRetaining = &H00020000
Const adXactAbortRetaining = &H00040000

'---- PropertyAttributesEnum Values ----
Const adPropNotSupported = &H0000
Const adPropRequired = &H0001
Const adPropOptional = &H0002
Const adPropRead = &H0200
Const adPropWrite = &H0400

'---- ErrorValueEnum Values ----
Const adErrInvalidArgument = &Hbb9
Const adErrNoCurrentRecord = &Hbcd
Const adErrIllegalOperation = &Hc93
Const adErrInTransaction = &Hcae
Const adErrFeatureNotAvailable = &Hcb3
Const adErrItemNotFound = &Hcc1
Const adErrObjectInCollection = &Hd27
Const adErrObjectNotSet = &Hd5c
Const adErrDataConversion = &Hd5d
Const adErrObjectClosed = &He78
Const adErrObjectOpen = &He79
Const adErrProviderNotFound = &He7a
Const adErrBoundToCommand = &He7b
Const adErrInvalidParamInfo = &He7c
Const adErrInvalidConnection = &He7d
Const adErrStillExecuting = &He7f
Const adErrStillConnecting = &He81

'---- ParameterAttributesEnum Values ----
Const adParamSigned = &H0010
Const adParamNullable = &H0040
Const adParamLong = &H0080

'---- ParameterDirectionEnum Values ----
Const adParamUnknown = &H0000
Const adParamInput = &H0001
Const adParamOutput = &H0002
Const adParamInputOutput = &H0003
Const adParamReturnValue = &H0004

'---- CommandTypeEnum Values ----
Const adCmdUnknown = &H0008
Const adCmdText = &H0001
Const adCmdTable = &H0002
Const adCmdStoredProc = &H0004

'---- SchemaEnum Values ----
Const adSchemaProviderSpecific = -1
Const adSchemaAsserts = 0
Const adSchemaCatalogs = 1
Const adSchemaCharacterSets = 2
Const adSchemaCollations = 3
Const adSchemaColumns = 4
Const adSchemaCheckConstraints = 5
Const adSchemaConstraintColumnUsage = 6
Const adSchemaConstraintTableUsage = 7
Const adSchemaKeyColumnUsage = 8
Const adSchemaReferentialContraints = 9
Const adSchemaTableConstraints = 10
Const adSchemaColumnsDomainUsage = 11
Const adSchemaIndexes = 12
Const adSchemaColumnPrivileges = 13
Const adSchemaTablePrivileges = 14
Const adSchemaUsagePrivileges = 15
Const adSchemaProcedures = 16
Const adSchemaSchemata = 17
Const adSchemaSQLLanguages = 18
Const adSchemaStatistics = 19
Const adSchemaTables = 20
Const adSchemaTranslations = 21
Const adSchemaProviderTypes = 22
Const adSchemaViews = 23
Const adSchemaViewColumnUsage = 24
Const adSchemaViewTableUsage = 25
Const adSchemaProcedureParameters = 26
Const adSchemaForeignKeys = 27
Const adSchemaPrimaryKeys = 28
Const adSchemaProcedureColumns = 29
%>
