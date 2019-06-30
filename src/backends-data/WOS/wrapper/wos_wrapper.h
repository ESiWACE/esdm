/* This file is part of ESDM.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ESDM.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __WOS_WRAPPER_H
#define __WOS_WRAPPER_H

using namespace wosapi;

extern "C" {

struct t_WosClusterPtr {
  WosClusterPtr wos;
  t_WosClusterPtr(WosClusterPtr w): wos(w) {
  }
  t_WosClusterPtr(): wos() {
  }
};

struct t_WosStatus {
  WosStatus wstatus;
  t_WosStatus(WosStatus ws): wstatus(ws) {
  }
  t_WosStatus(): wstatus() {
  }
};

struct t_WosOID {
  WosOID woid;
  t_WosOID(WosOID wo): woid(wo) {
  }
  t_WosOID(): woid() {
  }
};

struct t_WosPolicy {
  WosPolicy wpolicy;
  t_WosPolicy(WosPolicy wp): wpolicy(wp) {
  }
  t_WosPolicy(): wpolicy() {
  }
};

struct t_WosObjPtr {
  WosObjPtr wobj;
  t_WosObjPtr(WosObjPtr wj): wobj(wj) {
  }
  t_WosObjPtr(): wobj() {
  }
};

struct t_WosPutStreamPtr {
  WosPutStreamPtr ps;
  t_WosPutStreamPtr(WosPutStreamPtr wps): ps(wps) {
  }
  t_WosPutStreamPtr(): ps() {
  }
};

struct t_WosGetStreamPtr {
  WosGetStreamPtr gs;
  t_WosGetStreamPtr(WosGetStreamPtr wgs): gs(wgs) {
  }
  t_WosGetStreamPtr(): gs() {
  }
};

#ifdef __cplusplus
extern "C" {
#endif

typedef struct t_WosClusterPtr t_WosClusterPtr;
typedef struct t_WosStatus t_WosStatus;
typedef struct t_WosOID t_WosOID;
typedef struct t_WosPolicy t_WosPolicy;
typedef struct t_WosObjPtr t_WosObjPtr;
typedef struct t_WosPutStream t_WosPutStream;
typedef struct t_WosGetStream t_WosGetStream;

typedef struct t_WosPutStreamPtr t_WosPutStreamPtr;
typedef struct t_WosGetStreamPtr t_WosGetStreamPtr;
typedef struct t_WosClusterImplPtr t_WosClusterImplPtr;

enum WosStatusType { ok,
  error,
  NoNodeForPolicy = 200,
  NoNodeForObject = 201,
  UnknownPolicyName = 202,
  InternalError = 203,
  ObjectFrozen = 204,
  InvalidObjId = 205,
  NoSpace = 206,
  ObjNotFound = 207,
  ObjCorrupted = 208,
  FsCorrupted = 209,
  PolicyNotSupported = 210,
  IOErr = 211,
  InvalidObjectSize = 212,
  MissingObject = 213,
  TemporarilyNotSupported = 214,
  OutOfMemory = 215,
  ReservationNotFound = 216,
  EmptyObject = 217,
  InvalidMetadataKey = 218,
  UnusedReservation = 219,
  WireCorruption = 220,
  CommandTimeout = 221,
  InvalidGetSpanMode = 222,
  PutStreamAbandoned = 223,
  IncompleteSearchMetadata = 224,
  InvalidSearchMetadataTextLength = 225,
  InvalidIntegerSearchMetadata = 226,
  InvalidRealSearchMetadata = 227,
  ObjectComplianceReject = 228,
  InvalidComplianceDate = 229,
  _max_err_code // insert new errors above this
};

enum BufferMode {
  Buffered,
  Unbuffered
};

enum IntegrityCheck {
  IntegrityCheckEnabled,
  IntegrityCheckDisabled
};

typedef void *Context;

typedef void (*Callback)(t_WosStatus *, t_WosObjPtr *, Context);

t_WosClusterPtr *Conn(char *host);

/**
	// Put
	*/
void Put_b(t_WosStatus *, t_WosOID *, t_WosPolicy *, t_WosObjPtr *, t_WosClusterPtr *);

/**
	// Get
	*/
void Get_b(t_WosStatus *, const t_WosOID *, t_WosObjPtr *, t_WosClusterPtr *);

/**
	// Pre-reserve (lease?) an OID
	*/
void Reserve_b(t_WosStatus *, t_WosOID *, t_WosPolicy *, t_WosClusterPtr *);

/**
	// Attach data to a pre-reserved OID (once-only)
	*/
void PutOID_b(t_WosStatus *, const t_WosOID *, t_WosObjPtr *, t_WosClusterPtr *);

/**
	// Delete an object
	*/
void Delete_b(t_WosStatus *, const t_WosOID *, t_WosClusterPtr *);

/**
	// Query for the existence of an object
	*/
void Exists_b(t_WosStatus *, const t_WosOID *, t_WosClusterPtr *);

/**
	//Put no blocking
	*/
void Put_nb(t_WosObjPtr *, t_WosPolicy *, Callback, Context, t_WosClusterPtr *);

/**
	//Get no blocking
	*/
void Get_nb(const t_WosOID *, Callback, Context, t_WosClusterPtr *);

/**
	//Reserve no blocking
	*/
void Reserve_nb(t_WosPolicy *, Callback, Context, t_WosClusterPtr *);

/**
	//PuoOID no blocking
	*/
void PutOID_nb(t_WosObjPtr *, const t_WosOID *, Callback, Context, t_WosClusterPtr *);

/**
	//Delete no blocking
	*/
void Delete_nb(const t_WosOID *, Callback, Context, t_WosClusterPtr *);

/**
	//Exists no blocking
	*/
void Exists_nb(const t_WosOID *, Callback, Context, t_WosClusterPtr *);

/**
	// Wait for existing async operations to complete
	*/
void Wait(t_WosClusterPtr *);

/**
	// Create a Put Stream for creating a single (large) object:
	*/
t_WosPutStreamPtr *CreatePutStream(t_WosPolicy *, t_WosClusterPtr *);

/**
	// Create a Put Stream for use with a pre-reserved OID for creating a single (large) object:
	*/
t_WosPutStreamPtr *CreatePutOIDStream(t_WosOID *, t_WosClusterPtr *);

/**
	// Create a Get Stream for accessing a single (large) object:
	*/
t_WosGetStreamPtr *CreateGetStream(t_WosOID *woid, t_WosClusterPtr *wos);

/**
	// Metadata
	*/
void SetMetaStream(const char *key, const char *value, t_WosPutStreamPtr *wps);

/**
	// Blocking API
	*/
void PutSpan(t_WosStatus *, const void *, unsigned long int, unsigned long int, t_WosPutStreamPtr *);

void ClosePutStream(t_WosStatus *, t_WosOID *, t_WosPutStreamPtr *);

void PutSpan_nb(const void *, unsigned long int, unsigned long int, Callback, Context, t_WosPutStreamPtr *);

/**
	// Blocking API
	*/
void GetSpan(t_WosStatus *wstatus, t_WosObjPtr *wobj, unsigned long int off, unsigned long int len, t_WosGetStreamPtr *gs);

/**
	// Get a policy from a t_WosClusterPtr object
	*/
t_WosPolicy *GetPolicy(t_WosClusterPtr *, char *);

/**
	// Create functions
	*/
t_WosObjPtr *WosObjCreate();
t_WosOID *CreateWoid();
t_WosStatus *CreateStatus();

/**
	// Delete
	*/
void DeleteWosObj(t_WosObjPtr *);
void DeleteWosWoid(t_WosOID *);
void DeleteWosStatus(t_WosStatus *);
void DeleteWosPolicy(t_WosPolicy *);
void DeleteWosCluster(t_WosClusterPtr *);
void DeletePutStream(t_WosPutStreamPtr *);

/**
	// Hooks
	*/
t_WosObjPtr *WosObjHookCreate();

/**
	// Set and get metadata/data from an object
	*/

void SetMetaObj(const char *, const char *, t_WosObjPtr *);
void SetDataObj(const void *, int, t_WosObjPtr *);
void GetDataObj(const void **, int *, const t_WosObjPtr *);
void GetMetaObj(const char *, char **, const t_WosObjPtr *);

t_WosOID *GetOidObj(t_WosObjPtr *);
const char *OIDtoString(t_WosOID *);
void SetOID(t_WosOID *woid, const char *oid);

/**
	// Get status
	*/
int GetStatus(const t_WosStatus *);

/**
	// catch error
	*/
void CatchError(const t_WosStatus *);

#ifdef __cplusplus
}
#endif
#endif
