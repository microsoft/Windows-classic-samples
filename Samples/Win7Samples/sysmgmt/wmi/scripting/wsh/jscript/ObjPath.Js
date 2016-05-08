// Copyright (c)  Microsoft Corporation
//***************************************************************************
// 
// WMI Sample Script - Object Path manipulation (JScript)
//
// This script demonstrates the manipulation of object paths.
//
//***************************************************************************

try
{
	WScript.Echo ("");
	WScript.Echo ("Create an Object Path");
	WScript.Echo ("");

	var d = new ActiveXObject("WbemScripting.SWbemObjectPath");
	d.Path = '\\EREWHON\ROOT\DEFAULT:Foo.Bar=12,Wibble="Hah"';

	DumpPath(d);

	d.RelPath = "Hmm.G=1,H=3";

	WScript.Echo ();
	DumpPath(d);

	WScript.Echo ();
	WScript.Echo ("Extract an Object Path from a class");
	WScript.Echo ();

	var c = GetObject("winmgmts:").Get();
	c.Path_.Class = "PATHTEST00";
	var p = c.Put_();

	DumpPath(p);

	WScript.Echo ();
	WScript.Echo ("Extract an Object Path from a singleton");
	WScript.Echo ();

	var i = GetObject("winmgmts:root/default:__cimomidentification=@");
	var p = i.Path_;
	DumpPath(p);

	WScript.Echo ();
	WScript.Echo ("Extract an Object Path from a keyed instance");
	WScript.Echo ();

	var i = GetObject('winmgmts:win32_logicaldisk="C:"');
	var p = i.Path_;
	DumpPath(p);

	WScript.Echo ();
	WScript.Echo ("Clone keys");
	WScript.Echo ();

	var newKeys = p.Keys.Clone();
	DumpKeys (newKeys);

	WScript.Echo ();
	WScript.Echo ("Change Cloned keys");
	WScript.Echo ();

	//Note that the cloned copy of Keys _should_ be mutable
	newKeys.Add ("fred", 23);
	newKeys.Remove ("DeviceID");
	DumpKeys (newKeys);
}
catch (e)
{
	WScript.Echo ("Error in script: " + e);
}

function DumpPath(p)
{
 WScript.Echo ("Path=", p.Path);
 WScript.Echo ("RelPath=",p.RelPath);
 WScript.Echo ("Class=", p.Class);
 WScript.Echo ("Server=", p.Server);
 WScript.Echo ("Namespace=", p.Namespace);
 WScript.Echo ("DisplayName=", p.DisplayName);
 WScript.Echo ("ParentNamespace=", p.ParentNamespace);
 WScript.Echo ("IsClass=", p.IsClass);
 WScript.Echo ("IsSingleton=", p.IsSingleton);
 DumpKeys (p.Keys);
}

function DumpKeys (keys)
{
 var e = new Enumerator (keys);

 for (;!e.atEnd();e.moveNext ())
 {
	var key = e.item ();
	WScript.Echo ("KeyName:", key.Name, "KeyValue:", key.Value );
 }
}



