/*
 * comdispatch.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 20234-3-1

Description: Dispatch ASCOM command to COM

**************************************************/

#ifndef ATOM_ASCOM_COMDISPATCH_HPP
#define ATOM_ASCOM_COMDISPATCH_HPP

#include <string>
#include <map>

class _com_error;

std::string ExcepMsg(const EXCEPINFO& excep);
std::string ExcepMsg(const std::string& prefix, const EXCEPINFO& excep);

struct Variant : public VARIANT
{
    Variant() { VariantInit(this); }
};

class ExcepInfo : public EXCEPINFO
{
    ExcepInfo& operator=(const ExcepInfo& rhs) = delete;
public:
    ExcepInfo();
    ~ExcepInfo();
    void Assign(HRESULT hr, const std::string& source);
    void Assign(const _com_error& err, const std::string& source);
};

typedef Variant VariantArg;

class DispatchClass
{
    typedef std::map<std::string, DISPID> idmap_t;
    idmap_t m_idmap;
public:
    DispatchClass() { }
    ~DispatchClass() { }
    static bool dispid(DISPID *ret, IDispatch *idisp, OLECHAR *name, ExcepInfo *excep);
    bool dispid_cached(DISPID *ret, IDispatch *idisp, OLECHAR *name, ExcepInfo *excep);
};

class DispatchObj
{
    DispatchClass *m_class;
    IDispatch *m_idisp;
    ExcepInfo m_excep;
public:
    DispatchObj();
    DispatchObj(DispatchClass *cls);
    DispatchObj(IDispatch *idisp, DispatchClass *cls);
    ~DispatchObj();
    void Attach(IDispatch *idisp, DispatchClass *cls);
    bool Create(OLECHAR *progid);
    bool GetDispatchId(DISPID *ret, OLECHAR *name);
    bool GetProp(Variant *res, DISPID dispid);
    bool GetProp(Variant *res, OLECHAR *name);
    bool GetProp(Variant *res, OLECHAR *name, int arg);
    bool PutProp(OLECHAR *name, OLECHAR *val);
    bool PutProp(DISPID dispid, bool val);
    bool PutProp(DISPID dispid, double val);
    bool PutProp(OLECHAR *name, bool val);
    bool InvokeMethod(Variant *res, OLECHAR *name);
    bool InvokeMethod(Variant *res, OLECHAR *name, OLECHAR *arg);
    bool InvokeMethod(Variant *res, OLECHAR *name, double arg1, double arg2);
    bool InvokeMethod(Variant *res, DISPID dispid, double arg1, double arg2);
    bool InvokeMethod(Variant *res, DISPID dispid);
    const EXCEPINFO& Excep() const { return m_excep; }
    IDispatch *IDisp() const { return m_idisp; }
};

// IGlobalInterfaceTable wrapper
class GITEntry
{
    IGlobalInterfaceTable *m_pIGlobalInterfaceTable;
    DWORD m_dwCookie;
public:
    GITEntry();
    ~GITEntry();
    void Register(IDispatch *idisp);
    void Register(const DispatchObj& obj) { Register(obj.IDisp()); }
    void Unregister();
    bool IsRegistered() const { return m_dwCookie != 0; }
    IDispatch *Get() const
    {
        IDispatch *idisp = 0;
        if (m_dwCookie)
            m_pIGlobalInterfaceTable->GetInterfaceFromGlobal(m_dwCookie, IID_IDispatch, (LPVOID *)&idisp);
        return idisp;
    }
};

struct GITObjRef : public DispatchObj
{
    GITObjRef(const GITEntry& gitentry)
    {
        Attach(gitentry.Get(), 0);
    }
};

#endif
