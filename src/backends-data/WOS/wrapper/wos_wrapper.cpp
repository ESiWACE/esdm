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

#include "wos_wrapper.h"
#include "wos_cluster.hpp"
#include "wos_obj.hpp"

#include <string.h>
#include <stdio.h>

t_WosClusterPtr *Conn(char *host) {
	wosapi::WosClusterPtr wos;
	try {
		wos = WosCluster::Connect(host);
	}
	catch(WosException & e) {
		return NULL;
	}
	return (new t_WosClusterPtr(wos));
}

t_WosPutStreamPtr *CreatePutStream(t_WosPolicy * wpolicy, t_WosClusterPtr * wos) {
	wosapi::WosPutStreamPtr wps;
	try {
		wps = wos->wos->CreatePutStream(wpolicy->wpolicy);
	}
	catch(WosException & e) {
		return NULL;
	}
	return (new t_WosPutStreamPtr(wps));
}

t_WosPutStreamPtr *CreatePutOIDStream(t_WosOID * woid, t_WosClusterPtr * wos) {
	wosapi::WosPutStreamPtr wps;
	try {
		wps = wos->wos->CreatePutOIDStream(woid->woid);
	}
	catch(WosException & e) {
		return NULL;
	}
	return (new t_WosPutStreamPtr(wps));
}

void PutSpan(t_WosStatus * wstatus, const void *data, unsigned long int off, unsigned long int len, t_WosPutStreamPtr * ps) {
	ps->ps->PutSpan(wstatus->wstatus, data, (uint64_t) off, (uint64_t) len);
}

void GetSpan(t_WosStatus * wstatus, t_WosObjPtr * wobj, unsigned long int off, unsigned long int len, t_WosGetStreamPtr * gs) {
	gs->gs->GetSpan(wstatus->wstatus, wobj->wobj, (uint64_t) off, (uint64_t) len);
}

void ClosePutStream(t_WosStatus * wstatus, t_WosOID * woid, t_WosPutStreamPtr * ps) {
	ps->ps->Close(wstatus->wstatus, woid->woid);
}

t_WosGetStreamPtr *CreateGetStream(t_WosOID * woid, t_WosClusterPtr * wos) {
	wosapi::WosGetStreamPtr wgs;
	try {
		wgs = wos->wos->CreateGetStream(woid->woid);
	}
	catch(WosException & e) {
		return NULL;
	}
	return (new t_WosGetStreamPtr(wgs));
}

void Put_b(t_WosStatus * wstatus, t_WosOID * woid, t_WosPolicy * wpolicy, t_WosObjPtr * wobj, t_WosClusterPtr * wos) {
	wos->wos->Put(wstatus->wstatus, woid->woid, wpolicy->wpolicy, wobj->wobj);
	return;
}

void Reserve_b(t_WosStatus * wstatus, t_WosOID * woid, t_WosPolicy * wpolicy, t_WosClusterPtr * wos) {
	wos->wos->Reserve(wstatus->wstatus, woid->woid, wpolicy->wpolicy);
	return;
}

void PutOID_b(t_WosStatus * wstatus, const t_WosOID * woid, t_WosObjPtr * wobj, t_WosClusterPtr * wos) {
	wos->wos->PutOID(wstatus->wstatus, woid->woid, wobj->wobj);
	return;
}

void Get_b(t_WosStatus * wstatus, const t_WosOID * woid, t_WosObjPtr * wobj, t_WosClusterPtr * wos) {
	wos->wos->Get(wstatus->wstatus, woid->woid, wobj->wobj);
	return;
}

void Delete_b(t_WosStatus * wstatus, const t_WosOID * woid, t_WosClusterPtr * wos) {
	wos->wos->Delete(wstatus->wstatus, woid->woid);
	return;
}

void Exists_b(t_WosStatus * wstatus, const t_WosOID * woid, t_WosClusterPtr * wos) {
	wos->wos->Exists(wstatus->wstatus, woid->woid);
	return;
}

t_WosPolicy *GetPolicy(t_WosClusterPtr * wos, char *wpolicy) {
	return new t_WosPolicy(wos->wos->GetPolicy(wpolicy));
}

struct UserMessageContext {
	Context ctx;
	void (*cb) (t_WosStatus *, t_WosObjPtr *, Context);
}

void callback(WosStatus s, WosObjPtr wobj, Context pctx) {
	UserMessageContext *ctx = static_cast < UserMessageContext * >(pctx);
	t_WosStatus *status = new t_WosStatus(s);
	t_WosObjPtr *twobj = new t_WosObjPtr(wobj);
	ctx->cb(status, twobj, ctx->ctx);
	delete status;
	delete twobj;
	delete ctx;
}

void Put_nb(t_WosObjPtr * wobj, t_WosPolicy * policy, Callback cb_put, Context ctx_put, t_WosClusterPtr * wos) {
	struct UserMessageContext *umc = new struct UserMessageContext ();
	umc->cb = cb_put;
	umc->ctx = ctx_put;
	wos->wos->Put(wobj->wobj, policy->wpolicy, callback, umc);
	delete umc;
	return;
}

void PutSpan_nb(const void *data, unsigned long int off, unsigned long int len, Callback cb_putSpan, Context ctx_putSpan, t_WosPutStreamPtr * ps) {
	struct UserMessageContext *umc = new struct UserMessageContext ();
	umc->cb = cb_putSpan;
	umc->ctx = ctx_putSpan;
	ps->ps->PutSpan(data, off, len, umc, callback);
	delete umc;
	return;

}

void Reserve_nb(t_WosPolicy * policy, Callback cb_reserve, Context ctx_reserve, t_WosClusterPtr * wos) {
	struct UserMessageContext *umc = new struct UserMessageContext ();
	umc->cb = cb_reserve;
	umc->ctx = ctx_reserve;
	wos->wos->Reserve(policy->wpolicy, callback, umc);
	delete umc;
	return;
}

void PutOID_nb(t_WosObjPtr * wobj, const t_WosOID * woid, Callback cb_putOID, Context ctx_putOID, t_WosClusterPtr * wos) {
	struct UserMessageContext *umc = new struct UserMessageContext ();
	umc->cb = cb_putOID;
	umc->ctx = ctx_putOID;
	wos->wos->PutOID(wobj->wobj, woid->woid, callback, umc);
	delete umc;
	return;
}

void Get_nb(const t_WosOID * woid, Callback cb_get, Context ctx_get, t_WosClusterPtr * wos) {
	struct UserMessageContext *umc = new struct UserMessageContext ();
	umc->cb = cb_get;
	umc->ctx = ctx_get;
	wos->wos->Get(woid->woid, callback, umc);
	delete umc;
	return;
}

void Delete_nb(const t_WosOID * woid, Callback cb_del, Context ctx_del, t_WosClusterPtr * wos) {
	struct UserMessageContext *umc = new struct UserMessageContext ();
	umc->cb = cb_del;
	umc->ctx = ctx_del;
	wos->wos->Delete(woid->woid, callback, umc);
	delete umc;
	return;
}

void Wait(t_WosClusterPtr * wos) {
	wos->wos->Wait();
	return;
}

t_WosObjPtr *WosObjCreate() {
	return new t_WosObjPtr(WosObj::Create());
}

t_WosOID *CreateWoid() {
	return new t_WosOID();
}

t_WosStatus *CreateStatus() {
	return new t_WosStatus();
}

int GetStatus(const t_WosStatus * wstatus) {
	return wstatus->wstatus;
}

void DeleteWosObj(t_WosObjPtr * data) {
	delete data;
	return;
}

void DeleteWosWoid(t_WosOID * data) {
	delete data;
	return;
}

void DeleteWosStatus(t_WosStatus * data) {
	delete data;
	return;
}

void DeleteWosPolicy(t_WosPolicy * data) {
	delete data;
	return;
}

void DeleteWosCluster(t_WosClusterPtr * data) {
	delete data;
	return;
}

void DeletePutStream(t_WosPutStreamPtr * data) {
	delete data;
	return;
}

void DeleteGetStream(t_WosGetStreamPtr * data) {
	delete data;
	return;
}

void SetMetaObj(const char *key, const char *value, t_WosObjPtr * wobj) {
	wobj->wobj->SetMeta(key, value);
	return;
}

void SetMetaStream(const char *key, const char *value, t_WosPutStreamPtr * ps) {
	ps->ps->SetMeta(key, value);
	return;
}

unsigned long int GetLengthStream(t_WosGetStreamPtr * gs) {
	uint64_t len;
	len = gs->gs->GetLength();
	return (unsigned long int) len;
}

void SetDataObj(const void *value, int len, t_WosObjPtr * wobj) {
	wobj->wobj->SetData(value, (int64_t) len);
	return;
}

void GetDataObj(const void **value, int *len, const t_WosObjPtr * wobj) {
	uint64_t len1;
	wobj->wobj->GetData(*value, len1);
	*len = (int) len1;
	return;
}

void GetMetaObj(const char *key, char **value, const t_WosObjPtr * wobj) {
	std::string temp;
	wobj->wobj->GetMeta(key, temp);
	*value = strndup(temp.c_str(), temp.size());
	return;
}

t_WosOID *GetOidObj(t_WosObjPtr * wobj) {
	return new t_WosOID(wobj->wobj->GetOID());
}

const char *OIDtoString(t_WosOID * woid) {
	return woid->woid.c_str();
}

void SetOID(t_WosOID * woid, const char *oid) {
	woid->woid.assign(oid);
}

t_WosObjPtr *WosObjHookCreate() {
	return new t_WosObjPtr();
}
