var WSHShell = WScript.CreateObject("WScript.Shell");
var strArguments = WScript.Arguments;

if (strArguments.length==1)
{
  var fs=WScript.CreateObject("Scripting.FileSystemObject");

  var readline='';

  var f=fs.GetFile("../../version");
  var fs2=f.OpenAsTextStream(1,0);
  readline=fs2.ReadLine();
  fs2.close();

  fs.CreateTextFile("mversion.h");

  var f=fs.GetFile("mversion.h");
  var fs2=f.OpenAsTextStream(2,0);
  fs2.WriteLine('#ifndef __mversion_h');
  fs2.WriteLine('#define __mversion_h');
  fs2.WriteLine('#define VERSION \"'+readline+'\"');
  fs2.WriteLine('#endif');
  fs2.close();
} else
{
  WScript.Echo("Do not use this file !");
}

