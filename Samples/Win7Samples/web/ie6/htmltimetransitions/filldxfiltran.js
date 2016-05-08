var lastTab = null;
//activated when pressing on the tab
function tabToggle(tabID){


	if ((lastTab != tabID) && (lastTab != null)){
		lastTab.style.zIndex = "1";
		lastTab.style.top = "3";
		lastTab.style.backgroundImage='url(UI_graytab.gif)';
		lastTab.style.fontWeight='normal';
		lastTab.style.paddingTop='2px';
    	}
	if (tabID.style.zIndex == "3")
		{
      	tabID.style.zIndex = "3";
	  	tabID.style.top = "0";
		tabID.style.backgroundImage='url(UI_bluetab.gif)';
		tabID.style.fontWeight='bold';
		tabID.style.paddingTop='4px';
		}
	else 
		{
		tabID.style.zIndex = "3"; 
		tabID.style.top = "0";
		tabID.style.backgroundImage='url(UI_bluetab.gif)';
		tabID.style.fontWeight='bold';
		tabID.style.paddingTop='4px';
   		lastTab = tabID;
    	}
}


var lastDisplay = null;

function displayToggle(displayID){

	if ((lastDisplay != displayID) && (lastDisplay != null)){
	  lastDisplay.style.display="none";
    }
	if (displayID.style.display=="block"){
      displayID.style.display="block";
	}
	else {displayID.style.display="block"; 
   	  lastDisplay = displayID;
    }
}

panState=0
function panelExp()
{
	if(panState=='0')
	{
	oPanel.style.width='670';
	panState=1;
	}
	else
	{
	oPanel.style.width='400';
	panState=0;
	}
}


var bTranState = 0;
function fnToggle() {
//can I Apply to the div?
   		oTransContainer.filters[0].Apply();
    if (bTranState=='0') { 
		bTranState = 1;
        oDIV2.style.visibility="visible"; 
		oDIV1.style.visibility="hidden";   
		}
    else {  
		bTranState = 0;
        oDIV2.style.visibility="hidden"; 
		oDIV1.style.visibility="visible";  
		}
		
    	oTransContainer.filters[0].Play();
}
function copy2Clipboard()
{
  textRange = document.body.createTextRange();
  textRange.moveToElementText(oCodeCopy);
  textRange.execCommand("Copy");
}

function maskOn(){

	if(	addMaskBT.innerText == 'Add Mask Filter')
	{
		addMaskBT.innerText='Remove Filter';
		filterDIV.style.filter='progid:DXImageTransform.Microsoft.maskfilter()';
		mColorHide.style.visibility='visible';
		codeFilter.style.display='inline';	
	}
	else{
		addMaskBT.innerText='Add Mask Filter';
		filterDIV.style.filter='';
		mColorHide.style.visibility='hidden';
		codeFilter.style.display='none';	
	}
}

var alphaOne ='FF';
var alphaTwo ='FF';

