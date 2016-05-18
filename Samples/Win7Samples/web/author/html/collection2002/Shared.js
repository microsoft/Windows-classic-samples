//-----------------------------------
//  SHARED.JS
//
//  Shared JScript Library. 1998-2001
//
//  Version 1.01.004
//-----------------------------------

function Ignore()
  {
  window.event.returnValue = false;
  }

function RowNum(CellObject)
  {
  return CellObject.parentElement.rowIndex;
  }

function ColNum(CellObject)
  {
  return CellObject.cellIndex;
  }

function SetCookie(Name)
  {
  value = eval(Name);

  switch (typeof value)
    {
    case "string":
      value = "\'" + value + "\'";	// so that eval() in GetCookie() works OK for strings
      break;

    case "number":
      value = value.toString ();
      break;

    case "boolean":
      break;

    default:
      value = "";
      break;
    }

  x = new Date();
  x.setDate(x.getDate() + 45);	// add 45 days (cookie will expire after 45 days)

  document.cookie = escape(Name) + "=" + value + ";path=/;expires=" + x.toUTCString();
  }

function GetCookie(Name)
  {
  dc = unescape(document.cookie);

  if (dc.length == 0)
    return;

  cName = Name + "=";
  pos0 = dc.indexOf(cName);

  if (pos0 == -1)
    return;

  pos0 += cName.length;
  pos1 = dc.indexOf(";", pos0);
  if (pos1 == -1)
    pos1 = dc.length;

  return eval(cName + dc.substring(pos0, pos1));
  }
