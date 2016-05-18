// Copyright (c)  Microsoft Corporation
//***************************************************************************
// 
// WMI Sample Script - Embedded object manipulation (JScript)
//
// This script demonstrates the manipulation of embedded object values.
//
//***************************************************************************
try
{
	var Service = GetObject("winmgmts:root/default");

	//*******************************
	//Create an embedded object class
	//*******************************

	/*
	[woobit(24.5)]
	class EmObjInner {
		[key] uint32 pInner = 10;
	};
	*/

	var EmbObjInner = Service.Get();
	EmbObjInner.Path_.Class = "EmbObjInner";
	EmbObjInner.Qualifiers_.Add ("woobit", 24.5);
	var Property = EmbObjInner.Properties_.Add ("pInner", 19);
	Property.Qualifiers_.Add ("key", true);
	Property.Value = 10;
	EmbObjInner.Put_();
	EmbObjInner = Service.Get("EmbObjInner");

	//************************************
	//Create another embedded object class
	//************************************

	/*
	[wazzuck("oxter")]
	class EmbObjOuter {
		uint32 p0 = 25;
		EmbObjInner pOuter = instance of EmbObjInner { pInner = 564; };
		EmbObjInner pOuterArray[] = {
			instance of EmbObjInner { pInner = 0; }, 
			instance of EmbObjInner { pInner = 1; }, 
			instance of EmbObjInner { pInner = 2; }
		};
	};
	*/

	var EmbObjOuter = Service.Get();
	EmbObjOuter.Path_.Class = "EmbObjOuter";
	EmbObjOuter.Qualifiers_.Add ("wazzuck", "oxter");
	EmbObjOuter.Properties_.Add ("p0", 19).Value = 25;
	var Property = EmbObjOuter.Properties_.Add ("pOuter", 13);
	var Instance = EmbObjInner.SpawnInstance_();
	Instance.pInner = 564;
	Property.Value = Instance;

	// Add an array of embedded objects property
	var Property = EmbObjOuter.Properties_.Add ("pOuterArray", 13, true);
	Property.Qualifiers_.Add ("cimtype","object:EmbObjInner");

	var Instance0 = EmbObjInner.SpawnInstance_();
	Instance0.pInner = 0;
	var Instance1 = EmbObjInner.SpawnInstance_();
	Instance1.pInner = 1;
	var Instance2 = EmbObjInner.SpawnInstance_();
	Instance2.pInner = 2;
	Property.Value  = new Array (Instance0, Instance1, Instance2);

	var Instance3 = EmbObjInner.SpawnInstance_();
	Instance3.pInner = 42;
	Property.Value(3) = Instance3

	EmbObjOuter.Put_();
	var EmbObjOuter = Service.Get("EmbObjOuter");

	//Create a final class which wraps both embedded objects

	var Class = Service.Get();
	Class.Path_.Class = "EMBPROPTEST01";
	var Property = Class.Properties_.Add ("p1", 13);
	var Instance = EmbObjOuter.SpawnInstance_();
	Instance.p0 = 2546;
	Property.Value = Instance;
	Class.Put_();

	WScript.Echo ("The initial value of p0 is [2546]", Property.Value.p0);
	WScript.Echo ("The initial value of p0 is [2546]", Class.Properties_("p1").Value.Properties_("p0"));
	WScript.Echo ("The initial value of pInner is [564]", Property.Value.pOuter.pInner);
	WScript.Echo ("The initial value of pInner is [564]",  
		Class.Properties_("p1").Value.Properties_("pOuter").Value.Properties_("pInner"));

	WScript.Echo ("The initial value of EMBPROPTEST01.p1.pOuterArray[0].pInner is", Class.p1.pOuterArray(0).pInner);
	WScript.Echo ("The initial value of EMBPROPTEST01.p1.pOuterArray[1].pInner is", Class.p1.pOuterArray(1).pInner);
	WScript.Echo ("The initial value of EMBPROPTEST01.p1.pOuterArray[2].pInner is", Class.p1.pOuterArray(2).pInner);
	WScript.Echo ("The initial value of EMBPROPTEST01.p1.pOuterArray[3].pInner is", Class.p1.pOuterArray(3).pInner);
	var Class = Service.Get("EMBPROPTEST01");

	//Now do direct assignment to the outer emb obj
	Class.p1.p0 = 23;
	WScript.Echo ("The new value of p0 is [23]", Class.p1.p0);

	var Property = Class.p1;
	Property.p0 = 787;
	WScript.Echo ("The new value of p0 is [787]", Class.p1.p0);

	Class.Properties_("p1").Value.p0 = 56;
	WScript.Echo ("The new value of p0 is [56]", Class.p1.p0);

	//Now do direct assignment to the inner emb obj
	Class.p1.pOuter.pInner = 4;
	WScript.Echo ("The new value of pInner is [4]", Class.p1.pOuter.pInner);

	var Property = Class.p1.pOuter;
	Property.pInner = 12;
	WScript.Echo ("The new value of pInner is [12]", Class.p1.pOuter.pInner);

	//Now do assignment to the inner emb obj array
	Class.p1.pOuterArray(1).pInner = 5675;
	WScript.Echo ("The new value of Class.p1.pOuterArray[1].pInner is [5675]", Class.p1.pOuterArray(1).pInner);

}
catch (e)
{
	WScript.Echo ("Error in script: " + e);
}
