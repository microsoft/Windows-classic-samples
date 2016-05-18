
ADOPrivps.dll: dlldata.obj ADOPriv_p.obj ADOPriv_i.obj
	link /dll /out:ADOPrivps.dll /def:ADOPrivps.def /entry:DllMain dlldata.obj ADOPriv_p.obj ADOPriv_i.obj kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib 

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL $<

clean:
	@del ADOPrivps.dll
	@del ADOPrivps.lib
	@del ADOPrivps.exp
	@del dlldata.obj
	@del ADOPriv_p.obj
	@del ADOPriv_i.obj
