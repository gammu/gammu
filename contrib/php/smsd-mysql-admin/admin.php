<?php
# CONFIGURATION
//they can be defined or not. if not, interface will give login form
//$db_serv="127.0.0.1";
//$db_user="root";
//$db_pass="";
//$db_name="smsd"

//these must be defined
$dokument="admin.php"; //name of current document
$delall = true; //should be Delete All available for folders ?
$max_limit=1; //maximal number of sms for outgoing sms editor

#############################
## DO NOT CHANGE BELOW !!! ##
#############################

$inbox="UpdatedInDB,ReceivingDateTime,Text,SenderNumber,Coding,UDH,SMSCNumber,Class,TextDecoded,ID,RecipientID";
$outbox="UpdatedInDB,InsertIntoDB,Text,DestinationNumber,Coding,UDH,Class,TextDecoded,ID,MultiPart,RelativeValidity,SendingDateTime,SenderID,SendingTimeOut,DeliveryReport";
$outbox_multipart="Text,Coding,UDH,Class,TextDecoded,ID,SequencePosition";
$sentitems="UpdatedInDB,InsertIntoDB,SendingDateTime,DeliveryDateTime,Text,DestinationNumber,Coding,UDH,SMSCNumber,Class,TextDecoded,ID,SenderID,SequencePosition,Status,StatusError,TPMR,RelativeValidity";
$phones="ID,InsertIntoDB,TimeOut,Send,Receive,IMEI,Client,Battery,Signal";
$daemons="Start,Info";

function dispdatetime($dt)
{
	return "$dt[0]$dt[1]$dt[2]$dt[3]-$dt[5]$dt[6]-$dt[8]$dt[9] $dt[11]$dt[12]:$dt[14]$dt[15]:$dt[17]$dt[18]";
}
function dispdate($dt)
{
	return "$dt[0]$dt[1]$dt[2]$dt[3]-$dt[5]$dt[6]-$dt[8]$dt[9]";
}
function dispsmsinfo($class,$udh,$text,$textdecoded,$coding)
{
	if (!$udh == "") {
		echo "UDH AVAILABLE<br>\n";
	}
	if ($class == "0" || $class == "1" || $class == "2" || $class == "3") {
		echo "Class: $class<br>\n";
	}
	if ($coding == "8bit") {
		echo "BINARY<br>\n";
	} else {
		if (!$text == "") echo "<b>";
		if ($coding == "Unicode_No_Compression" || $coding == "Unicode_Compression") {
			echo "Unicode t";
		} else {
			echo "T";
		}
		if ($textdecoded == "") {
			echo "ext</b><br>\n";
		} else {
			echo "ext</b>: $textdecoded<br>\n";
		}
		if ($text == "") echo "</b>";
		echo "<br>\n";
	}
}
function dispvalidity($validity) {
	if ($validity == -1) {
		echo "default";
	} else if ($validity == 0) {
		echo "5 minutes";
	} else if ($validity == 255) {
		echo "max. time";
	} else {
		echo "$validity";
	}
}

$arg="";
if (!isset($db_serv) && isset($_GET['serv'])) {
	$db_serv = $_GET['serv'];
	if ($arg == "") {$arg="?";} else {$arg=$arg."&";};
	$arg = $arg . "serv=$db_serv";
	$dbservorig=false;
}
if (!isset($db_user) && isset($_GET['user'])) {
	$db_user = $_GET['user'];
	if ($arg == "") {$arg="?";} else {$arg=$arg."&";};
	$arg = $arg . "user=$db_user";
	$dbuserorig=false;
}
if (!isset($db_pass) && isset($_GET['pass'])) {
	$db_pass = $_GET['pass'];
	if ($arg == "") {$arg="?";} else {$arg=$arg."&";};
	$arg = $arg . "pass=$db_pass";
	$dbpassorig=false;
}
if ($arg == "") {$arg="?";} else {$arg=$arg."&";};

if (!isset($db_name) && isset($_GET['db'])) {
	$db_name = $_GET['db'];
}

if (isset($db_pass) && isset($db_user) && isset($db_serv)) {
	$dbpass = @mysql_connect("$db_serv","$db_user","$db_pass");
	if ($dbpass) {
		mysql_query("SET NAMES UTF8;");
		if (isset($db_name)) {
			$dbconnect = mysql_select_db("$db_name");
		}
	}
}

if (isset($dbpass) && isset($dbconnect) && isset($_GET['op']) &&
    isset($_GET['year']) && isset($_GET['month']) && isset($_GET['day']) &&
    isset($_GET['hour']) && isset($_GET['minute']) && isset($_GET['second']) &&
    isset($_GET['number']) && isset($_GET['tresc']) && isset($_GET['validity']) &&
    isset($_GET['report']) && isset($_GET['phone']))
{
	if ($_GET['op']=="addsms") {
		$year = $_GET['year'];
		$month = $_GET['month'];
		$day = $_GET['day'];
		$hour = $_GET['hour'];
		$minute = $_GET['minute'];
		$second = $_GET['second'];
		$datoom="$year$month$day$hour$minute$second";
		$number=$_GET['number'];
		$tresc=$_GET['tresc'];
		$validity=$_GET['validity'];
		if (isset($_GET['class'])) {
			$class = $_GET['class'];
		} else {
			$class = "-1";
		}
		$phone = $_GET['phone'];
		$report = $_GET['report'];
		if (strlen($tresc) > 160) {
//			$result2 = mysql_db_query("$db_name","select ID from outbox order by ID desc limit 1");
//			$rekord2 = mysql_fetch_row($result2);
//			if ($rekord == null) {
//				$newid = 0;
//			} else {
//				$newid = $rekord2[0];
//			}
//			mysql_free_result($result2);
//			$pos = 0;
//			$text = "";
		} else {
			if ($report == "yes") {
				mysql_query ("insert into outbox(UpdatedInDB,InsertIntoDB,Class,DestinationNumber,TextDecoded,SendingDateTime,RelativeValidity,SenderID,DeliveryReport,Coding) VALUES(now(),now(),'$class','$number','$tresc','$datoom','$validity','$phone','yes','Default_No_Compression')");
			}
			if ($report == "no") {
				mysql_query ("insert into outbox(UpdatedInDB,InsertIntoDB,Class,DestinationNumber,TextDecoded,SendingDateTime,RelativeValidity,SenderID,DeliveryReport,Coding) VALUES(now(),now(),'$class','$number','$tresc','$datoom','$validity','$phone','no','Default_No_Compression')");
			}
			if ($report == "default") {
				mysql_query ("insert into outbox(UpdatedInDB,InsertIntoDB,Class,DestinationNumber,TextDecoded,SendingDateTime,RelativeValidity,SenderID,Coding) VALUES(now(),now(),'$class','$number','$tresc','$datoom','$validity','$phone','Default_No_Compression')");
			}
		}
	}
}

if (isset($dbpass) && isset($dbconnect) && isset($_GET['op']) && isset($_GET['dzial'])) {
	if ($_GET['op']=="del") {
		if (isset($_GET['id'])) {
			$dzial = $_GET['dzial'];
			$id = $_GET['id'];
			mysql_query ("delete from $dzial where id='$id'");
			if ($dzial == "outbox") {
				mysql_query ("delete from outbox_multipart where id='$id'");
			}
		} else if ($delall) {
			$dzial = $_GET['dzial'];
			mysql_query ("delete from $dzial");
			if ($dzial == "outbox") {
				mysql_query ("delete from outbox_multipart");
			}
		}
	}
	$_GET['op']=$dzial;
}

echo "<HTML>\n<HEAD>\n";
echo "  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />";

echo "<STYLE TYPE=text/css>\n";
echo "<!--\n";
echo "BODY 			{text-decoration: none; color: #404040; font-family: verdana, arial; font-weight: normal; font-size: 10px; margin-left: 0pt; margin-right: 0pt; margin-top: 0pt; margin-bottom: 0pt}\n";
echo "input,select,option 	{text-decoration: none; color: #404040; font-family: verdana, arial; font-weight: normal; font-size: 10px;}\n";
echo "TD,P,BR   		{text-decoration: none; color: #404040; font-family: verdana, arial; font-weight: normal; font-size: 10px;}\n";
echo "B 			{text-decoration: none; 		font-family: verdana, arial; font-weight: bold;   font-size: 10px;}\n";
echo "LI        		{text-decoration: none; color: #404040; font-family: verdana, arial; font-weight: normal; font-size: 10px;}\n";
echo "A:link    		{text-decoration: none; color: navy;  	font-family: verdana, arial; font-weight: normal; font-variant: small-caps; font-size: 10px;}\n";
echo "A:visited 		{text-decoration: none; color: navy;  	font-family: verdana, arial; font-weight: normal; font-variant: small-caps; font-size: 10px;}\n";
echo "A:active  		{text-decoration: none; color: navy;  	font-family: verdana, arial; font-weight: normal; font-variant: small-caps; font-size: 10px;}\n";
echo "A:hover   		{text-decoration: none; color: red;   	font-family: verdana, arial; font-weight: normal; font-variant: small-caps; font-size: 10px;}\n";
echo "-->\n";
echo "</STYLE>\n";
echo "<TITLE>SMS SERVER based on GAMMU</TITLE>\n<body bgcolor=#ededed>\n";

if (isset($dbpass) && isset($dbconnect) && isset($_GET['op']) && isset($_GET['action'])) {
	if ($_GET['op']=="daemons") {
		$action = urldecode($_GET['action']);
		popen ($action, "r");
	}
}

if (isset($dbpass) && isset($dbconnect) && isset($_GET['op']) && isset($_GET['year']) && isset($_GET['month']) && isset($_GET['day']) && isset($_GET['hour']) && isset($_GET['minute']) && isset($_GET['second']) && isset($_GET['number']) && isset($_GET['tresc']) && isset($_GET['validity'])) {
	if ($_GET['op']=="addsms") {
		echo "<script>";
		echo "document.location.href='$dokument$arg"."db=$db_name&op=outbox';";
		echo "</script>";
	}
}

if (!isset($dbpass)) {
	echo "<b>LOGIN</b><p>\n";
	echo "<form method=\"GET\" action=$dokument name=login>\n<table>\n";
	echo "<tr><td>Server address:port :</td><td><input name=serv maxlength=20></td></tr>\n";
	echo "<tr><td>Name :</td><td><input name=user maxlength=20></td></tr>\n";
	echo "<tr><td>Password :</td><td><input name=pass maxlength=20></td></tr>\n";
	echo "<tr><td colspan=2><input type=submit name=send value=SEND OnClick=\"if (login.db_serv.value=='') {alert('Sender number not filled'); return false;} else return true;\"></td></tr></table></form>\n";
	exit;
}
if (!$dbpass) {
	echo "&nbsp;<p><center>Authorization error with MySQL server\n";

	if (isset($dbservorig) || isset($dbuserorig) || isset($dbpassorig)) {
		echo "<p><a href=ala OnClick=\"history.back(); return false;\">Back to login form</a>";
	}

	echo "</center></body>\n</html>";
	exit;
}
if (isset($dbconnect) && !$dbconnect) {
	echo "&nbsp;<p><center>No database in MySQL server</center>\n</body>\n</html>";
	exit;
}

echo "<script language=JavaScript>\n";
echo "function Del(ID) {\n";
echo "  return confirm(\"Do you want to delete SMS with ID \"+ID+\" ?\");\n";
echo "}\n";
if ($delall) {
	echo "function DelAll(ID) {\n";
	echo "  if (confirm(\"Do you want to delete all SMS from \"+ID+\" ?\")) {\n";
	echo "    if (confirm(\"REALLY ?\")) {\n";
	echo "      return true;\n";
	echo "    }\n";
	echo "  }\n";
	echo "  return false;\n";
	echo "}\n";
	echo "function update() {\n";
	if ($max_limit == 1) {
		echo "  if (document.newsms.tresc.value.length > 160) {\n";
		echo "    document.newsms.tresc.value = document.newsms.tresc.value.substring(0, 160);";
	} else {
		echo "  if (document.newsms.tresc.value.length > ". 153*$max_limit .") {\n";
		echo "    document.newsms.tresc.value = document.newsms.tresc.value.substring(0, ". 153*$max_limit .");";
	}
	echo "  \n  }\nif (document.newsms.tresc.value.length > 160) {\n";
	echo "    document.newsms.smsnum.value = Math.ceil(document.newsms.tresc.value.length/153);\n";
	echo "    document.newsms.left.value = document.newsms.smsnum.value*153 - document.newsms.tresc.value.length;\n";
	echo "  } else {\n";
	echo "    document.newsms.smsnum.value = 1;\n";
	echo "    document.newsms.left.value = 160 - document.newsms.tresc.value.length;\n";
	echo "  }\n";
	echo "}\n";
}
echo "</script>\n";
echo "<table width = 100% cellspacing=1 cellpadding=5 border=0 bgcolor=silver>\n";
echo "<tr>\n<td colspan=2 bgcolor=red>\n";
echo "<b><font color=white size=2 face=verdana>SMS Gateway example, user $db_user</font></b>";
echo "</td>\n</tr>\n<tr>\n<td bgcolor=whitesmoke valign=top>";

#MENU

if (isset($dbservorig) || isset($dbuserorig) || isset($dbpassorig)) {
	echo "<nobr><a href=$dokument>OTHER USER</a></nobr><br>\n";
}

$result0 = mysql_list_dbs($dbpass);
while ($row0 = mysql_fetch_object($result0)) {
	$result = mysql_db_query("$row0->Database","select Version from gammu");
	$rekord = @mysql_fetch_row($result);
	if (!$rekord) continue;
	mysql_free_result($result);
	if ($rekord[0]!='7') continue;

	$result2 = @mysql_list_tables($row0->Database);
	if (!$result2) continue;
	$found = false;
	while ($row2 = mysql_fetch_row($result2)) {
		if ($row2[0] == "inbox")            $found = true;
		if ($row2[0] == "outbox")           $found = true;
		if ($row2[0] == "outbox_multipart") $found = true;
		if ($row2[0] == "sentitems")        $found = true;
	}
	mysql_free_result($result2);
	if (!$found) continue;
	if (!isset($_GET['db']) || $_GET['db']!=$row0->Database) {
		echo "<a href=$dokument$arg"."db=$row0->Database>[>>] $row0->Database</a><br>\n";
		continue;
	}
	echo "<a href=$dokument$arg"."x=x>[<<] $row0->Database</a><br>\n";

	echo "&nbsp <a href=$dokument$arg"."db=$row0->Database&op=daemons>DAEMONS</a><br>\n";
	echo "&nbsp <a href=$dokument$arg"."db=$row0->Database&op=phones>PHONES</a><p>\n";

	echo "<nobr>&nbsp <a href=$dokument$arg"."db=$row0->Database&op=newsms>NEW OUTBOX SMS</a></nobr><br><br>\n";

	echo "&nbsp <a href=$dokument$arg"."db=$row0->Database&op=inbox>INBOX</a><br>\n";
	if (isset($_GET['op']) && $_GET['op']=="inbox") {
		$result = mysql_db_query("$db_name","select substring(ReceivingDateTime,1,10) from inbox group by substring(ReceivingDateTime,1,10) order by substring(ReceivingDateTime,1,10) desc");
		while($rekord = mysql_fetch_row($result)) {
			$d = dispdate($rekord[0]);
			echo " &nbsp &nbsp &middot <a href=$dokument$arg"."db=$row0->Database&op=inbox&date=$rekord[0]>$d</a><br>";
		}
		mysql_free_result($result);
	}

	echo "&nbsp <a href=$dokument$arg"."db=$row0->Database&op=outbox>OUTBOX</a><br>\n";
	if (isset($_GET['op']) && $_GET['op']=="outbox") {
		$result = mysql_db_query("$db_name","select substring(SendingDateTime,1,10) from outbox group by substring(SendingDateTime,1,10) order by substring(SendingDateTime,1,10) desc");
		while($rekord = mysql_fetch_row($result)) {
			$d = dispdate($rekord[0]);
			echo " &nbsp &nbsp &middot <a href=$dokument$arg"."db=$row0->Database&op=outbox&date=$rekord[0]>$d</a><br>";
		}
		mysql_free_result($result);
	}

	echo "&nbsp <a href=$dokument$arg"."db=$row0->Database&op=sentitems>SENT ITEMS</a><br>\n";
	if (isset($_GET['op']) && $_GET['op']=="sentitems") {
		$result = mysql_db_query("$db_name","select $sentitems,substring(SendingDateTime,1,10) from sentitems group by substring(SendingDateTime,1,10) order by substring(SendingDateTime,1,10) desc");
		while($rekord = mysql_fetch_row($result)) {
			$d = dispdate($rekord[18]);
			echo " &nbsp &nbsp &middot <a href=$dokument$arg"."db=$row0->Database&op=sentitems&date=$rekord[18]>$d</a><br>";
		}
		mysql_free_result($result);
	}
}
mysql_free_result($result0);

# /MENU

echo "</td>\n<td bgcolor=whitesmoke valign=top>\n";

#TRESC

if (isset($_GET['op'])) {
  	if ($_GET['op']=="inbox") {
		$innum = 0;
		echo "<b>DATABASE $db_name, INBOX";
		if (isset($_GET['date']) && $_GET['date']!="") {
			$d = $_GET['date'];
			$d2 = dispdate($d);
			echo " $d2";
		}
		echo "</b><br><br>\n";
		echo "<table width=620 cellspacing=1 border=1>";
		echo "<tr bgcolor=gold><td>ID</td>\n";
		echo "<td>FROM</td>\n";
		echo "<td>SMSC</td>\n";
		echo "<td>CLASS</td>\n";
		echo "<td>RECEIVE TIME</td>\n";
		echo "<td>PHONE</td>\n";
		if ($delall) {
			echo "<td><a href=$dokument$arg"."db=$db_name&op=del&dzial=inbox Title='Click to delete all SMS' OnClick=\"return DelAll('Inbox');\">[X]</td></tr>\n";
		} else {
			echo "<td></td></tr>\n";
		}
		if (!isset($_GET['date']) || $_GET['date']=="") {
	  		$result = mysql_db_query("$db_name","select $inbox from inbox order by ReceivingDateTime desc");
		} else {
			$d = $_GET['date'];
	  		$result = mysql_db_query("$db_name","select $inbox from inbox where ReceivingDateTime like '$d%' order by ReceivingDateTime desc");
		}
		while($rekord = mysql_fetch_row($result)) {
			$innum++;
			if (!isset($_GET['date']) || $_GET['date']=="") {
				if (isset($_GET['id']) && $_GET['id']!="" && $_GET['id'] == $rekord[9]) {
					echo "<tr><td>$rekord[9] <a href=$dokument$arg"."db=$db_name&op=inbox title='Click to hide details'>[<<]</a></td>";
				} else {
					echo "<tr><td>$rekord[9] <a href=$dokument$arg"."db=$db_name&op=inbox&id=$rekord[9] title='Click to display details'>[>>]</a></td>";
				}
			} else {
				if (isset($_GET['id']) && $_GET['id']!="" && $_GET['id'] == $rekord[9]) {
					echo "<tr><td>$rekord[9] <a href=$dokument$arg"."db=$db_name&op=inbox&date=$d title='Click to hide details'>[<<]</a></td>";
				} else {
					echo "<tr><td>$rekord[9] <a href=$dokument$arg"."db=$db_name&op=inbox&date=$d&id=$rekord[9] title='Click to display details'>[>>]</a></td>";
				}
			}
			echo "<td>$rekord[3]</td><td>$rekord[6]</td>";
			if ($rekord[7] == "-1") {
				echo "<td>not set</td>";
			} else {
				echo "<td>$rekord[7]</td>";
			}
			$d2 = dispdatetime($rekord[1]);
			echo "<td>$d2</td>";
			echo "<td>$rekord[10]</td>";
			if (!isset($_GET['date']) || $_GET['date']=="") {
				echo "<td><a href=$dokument$arg"."db=$db_name&op=del&dzial=inbox&id=$rekord[9] OnClick=\"return Del($rekord[9]);\" Title='Click to delete'>[X]</a></td></tr>";
			} else {
				echo "<td><a href=$dokument$arg"."db=$db_name&op=del&dzial=inbox&date=$d&id=$rekord[9] OnClick=\"return Del($rekord[9]);\" Title='Click to delete'>[X]</a></td></tr>";
			}
			if (isset($_GET['id']) && $_GET['id']!="" && $_GET['id'] == $rekord[9]) {
				$op = $_GET['op'];
				$id = $_GET['id'];
				echo "</tr><tr><td colspan=7 bgcolor=white>";
				$d2 = dispdatetime($rekord[0]);
				echo "Last changed in DB: $d2<br>";
				$d2 = dispdatetime($rekord[1]);
				echo "Insert into DB: $d2<br>";
				dispsmsinfo($rekord[7],$rekord[5],$rekord[2],$rekord[8],$rekord[4]);
				echo "</td></tr>\n";
			}
		}
		mysql_free_result($result);
		echo "</table>";
		echo "<br>$innum SMS received";
  	}
  	if ($_GET['op']=="outbox") {
		$outduring = 0;
		$outfuture = 0;
		echo "<b>DATABASE $db_name, OUTBOX";
		if (isset($_GET['date']) && $_GET['date']!="") {
			$d = $_GET['date'];
			$d2 = dispdate($d);
			echo " $d2";
		}
		echo "</b><br><br>\n";
		echo "<table width=620 cellspacing=1 border=1>";
		echo "<tr bgcolor=gold><td>ID</td>\n";
		echo "<td>TO</td>\n";
		echo "<td>TIME 2BE SENT</td>\n";
		echo "<td>PARTS</td>\n";
		echo "<td>VALIDITY</td>\n";
		echo "<td>SENDING</td>\n";
		echo "<td>PHONE</td>\n";
		echo "<td>REPORT</td>\n";
		if ($delall) {
			echo "<td><a href=$dokument$arg"."db=$db_name&op=del&dzial=outbox Title='Click to delete all SMS' OnClick=\"return DelAll('Outbox');\">[X]</td></tr>\n";
		} else {
			echo "<td></td></tr>\n";
		}
		if (!isset($_GET['date']) || $_GET['date']=="") {
			$result = mysql_db_query("$db_name","select $outbox from outbox order by SendingDateTime desc");
		} else {
			$d = $_GET['date'];
			$result = mysql_db_query("$db_name","select $outbox from outbox where SendingDateTime like '$d%' order by SendingDateTime desc");
		}
		while($rekord = mysql_fetch_row($result)) {
			if (!isset($_GET['date']) || $_GET['date']=="") {
				if (isset($_GET['id']) && $_GET['id']!="" && $_GET['id'] == $rekord[8]) {
					echo "<tr><td>$rekord[8] <a href=$dokument$arg"."db=$db_name&op=outbox title='Click to hide details'>[<<]</a></td>";
				} else {
					echo "<tr><td>$rekord[8] <a href=$dokument$arg"."db=$db_name&op=outbox&id=$rekord[8] title='Click to display details'>[>>]</a></td>";
				}
			} else {
				if (isset($_GET['id']) && $_GET['id']!="" && $_GET['id'] == $rekord[8]) {
					echo "<tr><td>$rekord[8] <a href=$dokument$arg"."db=$db_name&op=outbox&date=$d title='Click to hide details'>[<<]</a></td>";
				} else {
					echo "<tr><td>$rekord[8] <a href=$dokument$arg"."db=$db_name&op=outbox&date=$d&id=$rekord[8] title='Click to display details'>[>>]</a></td>";
				}
			}
			echo "<td>$rekord[3]</td>";
			$d2 = dispdatetime($rekord[11]);
			echo "<td>$d2</td>";
			$counter = 1;
			if ($rekord[9] == "true") {
				$result2 = mysql_db_query("$db_name","select $outbox_multipart from outbox_multipart where id='$rekord[8]'");
				while($rekord2 = mysql_fetch_row($result2)) $counter++;
				mysql_free_result($result2);
			}
			echo "<td>$counter</td><td>";
			dispvalidity($rekord[10]);
			if ($rekord[13] != "00000000000000") {
				$result2 = mysql_db_query("$db_name","select now()+0;");
				$rekord2 = mysql_fetch_row($result2);
				if ($rekord[13]<$rekord2[0]) {
					echo "</td><td>no (earlier failed)</td><td>";
					$outfuture++;
				} else {
					echo "</td><td>yes (now)</td><td>";
					$outduring++;
				}
				mysql_free_result($result2);
			} else {
				echo "</td><td>no (earlier not tried)</td><td>";
				$outfuture++;
			}
			if ($rekord[12] == "") {
				echo "<< any >>";
			} else {
				echo "$rekord[12]";
			}
			echo "</td><td>$rekord[14]</td><td>";
			if (!isset($_GET['date']) || $_GET['date']=="") {
				echo "<a href=$dokument$arg"."db=$db_name&op=del&dzial=outbox&id=$rekord[8] Title='Click to delete' OnClick=\"return Del($rekord[8]);\" >[X]</a></td></tr>";
			} else {
				echo "<a href=$dokument$arg"."db=$db_name&op=del&dzial=outbox&date=$d&id=$rekord[8] Title='Click to delete' OnClick=\"return Del($rekord[8]);\" >[X]</a></td></tr>";
			}

			if (isset($_GET['id']) && $_GET['id']!="" && $_GET['id'] == $rekord[8]) {
				$op = $_GET['op'];
				$id = $_GET['id'];
				echo "</tr><tr><td colspan=9 bgcolor=white>";
				$result2 = mysql_db_query("$db_name","select $outbox from outbox where ID='$id'");
				while($rekord2 = mysql_fetch_row($result2)) {
					$d2 = dispdatetime($rekord2[0]);
					echo "Last changed in DB: $d2<br>";
					$d2 = dispdatetime($rekord2[1]);
					echo "Insert into DB: $d2<br>";
					if ($rekord2[9] == "true") {
						echo "<hr size=1 color=silver>";
					}
					dispsmsinfo($rekord2[6],$rekord2[5],$rekord2[2],$rekord2[7],$rekord2[4]);
					if ($rekord[9] == "true") {
						$result3 = mysql_db_query("$db_name","select $outbox_multipart from outbox_multipart where id='$rekord[8]'");
						while($rekord3 = mysql_fetch_row($result3)) {
							echo "<hr size=1 color=silver>";
							dispsmsinfo($rekord3[3],$rekord3[2],$rekord3[0],$rekord3[4],$rekord3[1]);
						}
						mysql_free_result($result3);
					}
				}
				mysql_free_result($result2);
				echo "</td></tr>\n";
			}
		}
		mysql_free_result($result);
		echo "</table>";
		echo "<br>$outduring SMS sequences during sending, $outfuture SMS sequences waiting for sending";
  	}
  	if ($_GET['op']=="sentitems") {
		$sentnum = 0;
		echo "<b>DATABASE $db_name, SENT ITEMS";
		if (isset($_GET['date']) && $_GET['date']!="") {
			$d = $_GET['date'];
			$d2 = dispdate($d);
			echo " $d2";
		}
		echo "</b><br><br>\n";
		echo "<table width=620 cellspacing=1 border=1>";
		echo "<tr bgcolor=gold><td>ID</td>\n";
		echo "<td>TO</td>\n";
		echo "<td>SMSC</td>\n";
		echo "<td>UPDATED</td>\n";
		echo "<td>PHONE</td>\n";
		if ($delall) {
			echo "<td><a href=$dokument$arg"."db=$db_name&op=del&dzial=sentitems Title='Click to delete all SMS' OnClick=\"return DelAll('Sent Items');\">[X]</td></tr>\n";
		} else {
			echo "<td></td></tr>\n";
		}
		if (!isset($_GET['date']) || $_GET['date']=="") {
			$result = mysql_db_query("$db_name","select $sentitems from sentitems order by SendingDateTime");
		} else {
			$d = $_GET['date'];
			$result = mysql_db_query("$db_name","select $sentitems from sentitems where SendingDateTime like '$d%' group by ID order by ID");
		}
		while($rekord = mysql_fetch_row($result)) {
			if (!isset($_GET['date']) || $_GET['date']=="") {
				if (isset($_GET['id']) && $_GET['id']!="" && $_GET['id'] == $rekord[11]) {
					echo "<tr><td>$rekord[11] <a href=$dokument$arg"."db=$db_name&op=sentitems title='Click to hide details'>[<<]</a></td>\n";
				} else {
					echo "<tr><td>$rekord[11] <a href=$dokument$arg"."db=$db_name&op=sentitems&id=$rekord[11] title='Click to display details'>[>>]</a></td>\n";
				}
			} else {
				if (isset($_GET['id']) && $_GET['id']!="" && $_GET['id'] == $rekord[11]) {
					echo "<tr><td>$rekord[11] <a href=$dokument$arg"."db=$db_name&op=sentitems&date=$d title='Click to hide details'>[<<]</a></td>\n";
				} else {
					echo "<tr><td>$rekord[11] <a href=$dokument$arg"."db=$db_name&op=sentitems&date=$d&id=$rekord[11] title='Click to display details'>[>>]</a></td>\n";
				}
			}
			echo "<td>$rekord[5]</td>\n";
			echo "<td>$rekord[8]</td>\n";
			$d2 = dispdatetime($rekord[0]);
			echo "<td>$d2</td>\n";
			echo "<td>$rekord[12]</td>\n";
			if (!isset($_GET['date']) || $_GET['date']=="") {
				echo "<td><a href=$dokument$arg"."db=$db_name&op=del&dzial=sentitems&id=$rekord[11] Title='Click to delete' OnClick=\"return Del($rekord[11]);\" >[X]</a></td></tr>";
			} else {
				echo "<td><a href=$dokument$arg"."db=$db_name&op=del&dzial=sentitems&date=$d&id=$rekord[11] Title='Click to delete' OnClick=\"return Del($rekord[11]);\" >[X]</a></td></tr>";
			}
			echo "</tr>\n";
			$sentnum++;
			if (isset($_GET['id']) && $_GET['id']!="" && $_GET['id'] == $rekord[11]) {
				$op = $_GET['op'];
				$id = $_GET['id'];
				echo "</tr><tr><td colspan=6 bgcolor=white>";

				for ($i = 1; $i <= 10; $i++) {
					$result2 = mysql_db_query("$db_name","select $sentitems from sentitems where ID='$id' and SequencePosition='$i' limit 1");
					if ($result2 == null) break;
					while($rekord2 = mysql_fetch_row($result2)) {
						if (!$i == 1) echo "<hr size=1 color=silver>";
						echo "Validity: ";
						dispvalidity($rekord2[17]);
						echo "<br>\n";
						dispsmsinfo($rekord2[9],$rekord2[7],$rekord2[4],$rekord2[10],$rekord2[6]);
					}
					mysql_free_result($result2);
				}

				echo "<table width=100%><tr bgcolor=silver>\n";
				echo "<td>PART</td>\n";
				echo "<td>ERROR CODE</td>\n";
				echo "<td>STATUS</td>\n";
				echo "<td>SENDING TIME</td>\n";
				echo "<td>DELIVERY TIME</td></tr>\n";

				$result2 = mysql_db_query("$db_name","select $sentitems from sentitems where ID='$id'");
				while($rekord2 = mysql_fetch_row($result2)) {
					echo "<tr><td>$rekord2[13]</td>\n";
					echo "<td>$rekord2[15]</td>\n";
					echo "<td>$rekord2[14]</td>";
					$d2 = dispdatetime($rekord2[2]);
					echo "<td>$d2</td>\n";
					if ($rekord2[3] != "00000000000000") {
						$d2 = dispdatetime($rekord2[3]);
						echo "<td>$d2</td></tr>\n";
					} else {
						echo "<td>not set</td></tr>\n";
					}
				}
				mysql_free_result($result2);

				echo "</td></tr></table>\n";
			}
		}
		mysql_free_result($result);
		echo "</table>";
		echo "<br>$sentnum SMS sequences sent";
  	}
	if ($_GET['op']=="newsms") {
		$result2 = mysql_db_query("$db_name","select now()+0;");
		$rekord2 = mysql_fetch_row($result2);
		$fulldt = $rekord2[0];
		mysql_free_result($result2);

		$rok="$fulldt[0]$fulldt[1]$fulldt[2]$fulldt[3]";
		$miesiac="$fulldt[4]$fulldt[5]";
		$dzionek="$fulldt[6]$fulldt[7]";
		$godzina="$fulldt[8]$fulldt[9]";
		$minuta="$fulldt[10]$fulldt[11]";
		$sekunda="$fulldt[12]$fulldt[13]";

		echo "<b>DATABASE $db_name, NEW OUTBOX SMS</b><p>\n";
		echo "<form method=\"GET\" action=$dokument name=newsms>\n";
		echo "<input type=hidden name=op value=addsms>\n";
		echo "<input type=hidden name=serv value=$db_serv>\n";
		echo "<input type=hidden name=user value=$db_user>\n";
		echo "<input type=hidden name=pass value=$db_pass>\n";
		echo "<input type=hidden name=db value=$db_name>\n";

		echo "<table><tr><td>Recipient (eg. +48xxxxxxxxx)</td>";
		echo "<td><input name=number maxlength=20></td></tr>\n";

		echo "<tr><td>Sending date (year-month-day)</td>";
		echo "<td><input name=year maxlength=4 value=\"$rok\" size=4>\n";
		echo " - <input name=month maxlength=2 value=\"$miesiac\" size=2>\n";
		echo " - <input name=day maxlength=2 value=\"$dzionek\" size=2></td></tr>\n";

		echo "<tr><td>Sending time (hour:minute:second)</td><td><input name=hour maxlength=2 value=\"$godzina\" size=2>\n";
		echo " : <input name=minute maxlength=2 value=\"$minuta\" size=2>\n";
		echo " : <input name=second maxlength=2 value=\"$sekunda\" size=2></td></tr>\n";

		echo "<tr><td>Validity</td><td><select name=validity>\n";
		echo "<option value=-1 select>Default (taken from sending phone)\n";
		echo "<option value=0>5 minutes\n";
		echo "<option value=1>10 minutes\n";
		echo "<option value=255>max. time (depends on SMSC)\n";
		echo "</select></td></tr><tr>\n";

		echo "<tr><td>Delivery Report</td><td><select name=report>\n";
		echo "<option value=default select>Default (depends on sending daemon)\n";
		echo "<option value=yes>yes\n";
		echo "<option value=no>no\n";
		echo "</select></td></tr><tr>\n";

		echo "<tr><td>Sending phone</td><td><select name=phone>\n";
		echo "<option value=\"\" select>any\n";
//		echo "<option value=bzzz>ala\n";
		$result = mysql_db_query("$db_name","select $phones from phones where TimeOut>NOW() AND ID<>\"\"");
		while($rekord = mysql_fetch_row($result)) {
			echo "<option value=$rekord[0]>$rekord[5] / $rekord[0]\n";
		}
		echo "</select></td></tr><tr>\n";

		echo "<td colspan=2><input type=checkbox name=class value=0>Send class 0 SMS</input><br>\n";

		echo "<textarea name=tresc cols=70 rows=5 onChange=\"update();\" onFocus=\"update();\" onKeyUp=\"update();\" onKeyDown=\"update();\" onclick=\"update();\"></textarea></td></tr>\n";

		echo "<tr><td colspan=2><input type=submit value=SEND OnClick=\"if (newsms.number.value=='') {alert('Sender number not filled'); return false;} else return true;\"></td></tr>\n";

		echo "<tr><td><b>Chars left in current SMS</b></td><td><input name=left maxlength=3 value=\"160\" size=3 readonly></td></tr>\n";

		echo "<tr><td><b>SMS number</b></td><td><input name=smsnum maxlength=3 value=\"1\" size=3 readonly> / $max_limit</td></tr>\n";

		echo "</table></form>\n";
		echo "<table width=620 cellspacing=1 border=0>";
		echo "<tr><td>&nbsp;</td></tr></table>\n";
  	}
  	if ($_GET['op']=="daemons") {
		echo "<b>DATABASE $db_name, DAEMONS</b><p>\n";

		echo "<table width=620 cellspacing=1 border=1>";
		echo "<tr bgcolor=gold><td>INFO</td>\n";
		echo "<td></td></tr>\n";
		$result = mysql_db_query("$db_name","select $daemons from daemons");
		while($rekord = mysql_fetch_row($result)) {
			echo "<td>$rekord[1]</td>\n";
			$x = urlencode($rekord[0]);
			echo "<td><a href=$dokument$arg"."db=$db_name&action=$x&op=daemons Title='Click to start' OnClick=\"return Del('ala');\" >[X]</a></td></tr>";
		}
		mysql_free_result($result);
		echo "</table>";

		echo "<table width=620 cellspacing=1 border=0>";
		echo "<tr><td>&nbsp;</td></tr></table>\n";
	}
  	if ($_GET['op']=="phones") {
		$counter = 0;
		echo "<b>DATABASE $db_name, PHONES</b><p>\n";
		echo "<table width=1024 cellspacing=1 border=1>";
		echo "<tr bgcolor=gold><td>IMEI</td>\n";
		echo "<td>ID</td>\n";
		echo "<td>SEND SMS</td>\n";
		echo "<td>RECEIVE SMS</td>\n";
		echo "<td>LOGGED</td>\n";
		echo "<td>CLIENT</td>\n";
		echo "<td>BATTERY</td>\n";
		echo "<td>SIGNAL</td></tr>\n";
		$result = mysql_db_query("$db_name","select $phones from phones where TimeOut>NOW()");
		while($rekord = mysql_fetch_row($result)) {
			$counter++;
			echo "<td>$rekord[5]</td>\n";
			echo "<td>$rekord[0]</td>\n";
			echo "<td>$rekord[3]</td>\n";
			echo "<td>$rekord[4]</td>\n";
			$d2 = dispdatetime($rekord[1]);
			echo "<td>$d2</td>\n";
			echo "<td>$rekord[6]</td>\n";
			echo "<td>$rekord[7]</td>\n";
			echo "<td>$rekord[8]</td></tr>\n";
		}
		mysql_free_result($result);
		echo "</table>";
		echo "<br>$counter phones<p>";
		echo "<table width=620 cellspacing=1 border=0>";
		echo "<tr><td>&nbsp;</td></tr></table>\n";
	}
} else {
	echo "<table width=620 cellspacing=1 border=0>";
	echo "<tr><td>&nbsp;</td></tr></table>\n";
}

echo "</td></tr>\n<tr>\n<td colspan=2 height=15 bgcolor=red align=center>\n";
echo "<b><font color=white size=2 face=verdana>\n";
echo "SMS Gateway example version 0.0.3 (c) 2004 by Michal Kruger & <a href=mailto:marcin@mwiacek.com>Marcin Wiacek</a>. Part of <a href=http://www.mwiacek.com>Gammu</a> project<br><hr width=100>\n";
echo "This PC - IP ".$HTTP_SERVER_VARS['REMOTE_ADDR'].":".$HTTP_SERVER_VARS['REMOTE_PORT'];
//." (".gethostbyaddr($HTTP_SERVER_VARS['REMOTE_ADDR']).
echo ", ".$HTTP_SERVER_VARS['HTTP_USER_AGENT']."<br>\n";
echo "WWW server - IP ".$HTTP_SERVER_VARS['HTTP_HOST'].":".$HTTP_SERVER_VARS['SERVER_PORT'];
//echo " (".gethostbyaddr($HTTP_SERVER_VARS['HTTP_HOST'])."), \n";
echo ", ".$HTTP_SERVER_VARS['SERVER_SOFTWARE'].", MySQL client ",mysql_get_client_info(),"<br>\n";
echo "MySQL server - IP $db_serv, ",mysql_get_server_info(),"<br>\n";
echo "</font></b>\n";
echo "</td>\n</tr>\n</table>";


?>

<br />
<?php
#MENU 02

$link = mysql_connect ($db_serv, $db_user, $db_pass);
$result0 = mysql_list_dbs($link);
$arg = '?';

	if (!isset($_GET['db']) || $_GET['db']!=$row0->Database) {
		echo "<a href=$dokument$arg"."db=$row0->Database>[>>] $row0->Database</a><br>\n";

	}
	echo "<a href=$dokument$arg"."x=x>[<<] $row0->Database</a><br>\n";

	echo "&nbsp <a href=$dokument$arg"."db=$row0->Database&op=daemons>DAEMONS</a><br>\n";
	echo "&nbsp <a href=$dokument$arg"."db=$row0->Database&op=phones>PHONES</a><p>\n";

	echo "<nobr>&nbsp <a href=$dokument$arg"."db=$row0->Database&op=newsms>NEW OUTBOX SMS</a></nobr><br><br>\n";

	echo "&nbsp <a href=$dokument$arg"."db=$row0->Database&op=inbox>INBOX</a><br>\n";
	if (isset($_GET['op']) && $_GET['op']=="inbox") {
		$result = mysql_db_query("$db_name","select substring(ReceivingDateTime,1,10) from inbox group by substring(ReceivingDateTime,1,10) order by substring(ReceivingDateTime,1,10) desc");
		while($rekord = mysql_fetch_row($result)) {
			$d = dispdate($rekord[0]);
			echo " &nbsp &nbsp &middot <a href=$dokument$arg"."db=$row0->Database&op=inbox&date=$rekord[0]>$d</a><br>";
		}
		mysql_free_result($result);
	}

	echo "&nbsp <a href=$dokument$arg"."db=$row0->Database&op=outbox>OUTBOX</a><br>\n";
	if (isset($_GET['op']) && $_GET['op']=="outbox") {
		$result = mysql_db_query("$db_name","select substring(SendingDateTime,1,10) from outbox group by substring(SendingDateTime,1,10) order by substring(SendingDateTime,1,10) desc");
		while($rekord = mysql_fetch_row($result)) {
			$d = dispdate($rekord[0]);
			echo " &nbsp &nbsp &middot <a href=$dokument$arg"."db=$row0->Database&op=outbox&date=$rekord[0]>$d</a><br>";
		}
		mysql_free_result($result);
	}

	echo "&nbsp <a href=$dokument$arg"."db=$row0->Database&op=sentitems>SENT ITEMS</a><br>\n";
	if (isset($_GET['op']) && $_GET['op']=="sentitems") {
		$result = mysql_db_query("$db_name","select $sentitems,substring(SendingDateTime,1,10) from sentitems group by substring(SendingDateTime,1,10) order by substring(SendingDateTime,1,10) desc");
		while($rekord = mysql_fetch_row($result)) {
			$d = dispdate($rekord[18]);
			echo " &nbsp &nbsp &middot <a href=$dokument$arg"."db=$row0->Database&op=sentitems&date=$rekord[18]>$d</a><br>";
		}
		mysql_free_result($result);
	}

mysql_free_result($result0);

# /MENU


?>
