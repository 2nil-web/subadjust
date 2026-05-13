
var activeX=[];
function callActiveX(AXName, add) {
  if (typeof activeX[AXName] === 'undefined') activeX[AXName]=new ActiveXObject(AXName);
  return activeX[AXName];
}

function fso() { return callActiveX("Scripting.FileSystemObject"); }
function wsh() { return callActiveX("WScript.Shell"); }
function sha() { return callActiveX("Shell.Application"); }
function wsn() { return callActiveX('Wscript.Network'); }
function wsn() { return callActiveX('ScriptControl'); }

function addExpTab(pth)
{
  WScript.Sleep(500);
  wsh().SendKeys("^t");
  WScript.Sleep(300);
  wsh().SendKeys("%d");
  WScript.Sleep(300);
  wsh().SendKeys(pth+"{ENTER}");
}

prf=wsh().ExpandEnvironmentStrings("%USERPROFILE%")+"\\";
repo=prf+"Documents\\home\\00-subadjust\\";
wsh().Run("explorer.exe /select,"+repo);
WScript.Sleep(1000);
addExpTab(repo+"src");
addExpTab(repo+"assets");
addExpTab(prf+".subadjust");

