<?
# CONFIGURATION
$db_serv="127.0.0.1";
$db_user="root";
$db_pass="";
$db_name="sms";
$dokument="index.php";
$delall = true;

#############################
## DO NOT CHANGE BELOW !!! ##
#############################

$inbox="UpdatedInDB,DateTime,Text,SenderNumber,Coding,UDH,SMSCNumber,Class,TextDecoded,ID,RecipientID";
$outbox="UpdatedInDB,DateTime,Text,DestinationNumber,Coding,UDH,Class,TextDecoded,ID,MultiPart,RelativeValidity,SendingDateTime";
$outbox_multipart="Text,Coding,UDH,Class,TextDecoded,ID,SequencePosition";
$sentitems="UpdatedInDB,DateTime,SendingDateTime,DeliveryDateTime,Text,DestinationNumber,Coding,UDH,SMSCNumber,Class,TextDecoded,ID,SenderID,SequencePosition,Status,StatusError,TPMR,RelativeValidity";

function dispdatetime($dt)
{
	return "$dt[0]$dt[1]$dt[2]$dt[3]-$dt[4]$dt[5]-$dt[6]$dt[7] $dt[8]$dt[9]:$dt[10]$dt[11]:$dt[12]$dt[13]";
}
function dispdate($dt)
{
	return "$dt[0]$dt[1]$dt[2]$dt[3]-$dt[4]$dt[5]-$dt[6]$dt[7]";
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
	}
	if ($coding == "Unicode") {
		if ($text == "") {
			if ($textdecoded == "") {
				echo "Unicode text<br>\n";
			} else {
				echo "Unicode text: $textdecoded<br>\n";
			}
		} else {
			if ($textdecoded == "") {
				echo "<b>Unicode text</b><br>\n";
			} else {
				echo "<b>Unicode text</b>: $textdecoded<br>\n";
			}
		}
	}
	if ($coding == "Default") {
		if ($text == "") {
			if ($textdecoded == "") {
				echo "Text<br>\n";
			} else {
				echo "Text: $textdecoded<br>\n";
			}
		} else {
			if ($textdecoded == "") {
				echo "<b>Text</b><br>\n";
			} else {
				echo "<b>Text</b>: $textdecoded<br>\n";
			}
		}
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

$dbpass = @mysql_connect("$db_serv","$db_user","$db_pass");
if ($dbpass) {
	$dbconnect = mysql_select_db("$db_name");
}

if ($dbpass && $dbconnect && isset($_POST['op']) && isset($_POST['year']) && isset($_POST['month']) && isset($_POST['day']) && isset($_POST['hour']) && isset($_POST['minute']) && isset($_POST['second']) && isset($_POST['number']) && isset($_POST['tresc']) && isset($_POST['validity'])) {
	if ($_POST['op']=="addsms") {
		$year = $_POST['year'];
		$month = $_POST['month'];
		$day = $_POST['day'];
		$hour = $_POST['hour'];
		$minute = $_POST['minute'];
		$second = $_POST['second'];
		$datoom="$year$month$day$hour$minute$second";
		$number=$_POST['number'];
		$tresc=$_POST['tresc'];
		$validity=$_POST['validity'];
		if (isset($_POST['class'])) {
			$class = $_POST['class'];
		} else {
			$class = "-1";
		}
		mysql_query ("insert into outbox(Class,DestinationNumber,TextDecoded,SendingDateTime,RelativeValidity) VALUES('$class','$number','$tresc','$datoom','$validity')");
		$_GET['op']="outbox";
	}
}

if ($dbpass && $dbconnect && isset($_GET['op']) && isset($_GET['dzial'])) {
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

if ($dbpass && $dbconnect && !isset($_GET['op']) && !isset($_POST['op'])) {
	$_GET['op']='inbox';
}

echo "<HTML>\n<HEAD>\n";
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

if (!$dbpass) {
	echo "&nbsp;<p><center>Authorization error with MySQL server</center>\n</body>\n</html>";
	exit;
}
if (!$dbconnect) {
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
}
echo "</script>\n";
echo "<table width = 100% cellspacing=1 cellpadding=5 border=0 bgcolor=silver>\n";
echo "<tr>\n<td colspan=2 bgcolor=red>\n";
echo "<b><font color=white size=2 face=verdana>SMS Gateway example</font></b>";
echo "</td>\n</tr>\n<tr>\n<td bgcolor=whitesmoke valign=top>";

#MENU

echo "<br><a href=$dokument?op=newsms>NEW OUTBOX SMS</a><br><br>\n";

echo "<a href=$dokument?op=inbox>INBOX</a><br>\n";
if (isset($_GET['op']) && $_GET['op']=="inbox") {
	$result = mysql_db_query("$db_name","select substring(DateTime,1,8) from inbox group by substring(DateTime,1,8) order by substring(DateTime,1,8) desc");
	while($rekord = mysql_fetch_row($result)) {
		$d = dispdate($rekord[0]);
		echo " &nbsp &middot <a href=$dokument?op=inbox&day=$rekord[0]>$d</a><br>";
	}
	mysql_free_result($result);
}

echo "<a href=$dokument?op=outbox>OUTBOX</a><br>\n";
if (isset($_GET['op']) && $_GET['op']=="outbox") {
	$result = mysql_db_query("$db_name","select substring(SendingDateTime,1,8) from outbox group by substring(SendingDateTime,1,8) order by substring(SendingDateTime,1,8) desc");
	while($rekord = mysql_fetch_row($result)) {
		$d = dispdate($rekord[0]);
		echo " &nbsp &middot <a href=$dokument?op=outbox&day=$rekord[0]>$d</a><br>";
	}
	mysql_free_result($result);
}

echo "<a href=$dokument?op=sentitems>SENT ITEMS</a><br>\n";
if (isset($_GET['op']) && $_GET['op']=="sentitems") {
	$result = mysql_db_query("$db_name","select $sentitems,substring(SendingDateTime,1,8) from sentitems group by substring(SendingDateTime,1,8) order by substring(SendingDateTime,1,8) desc");
	while($rekord = mysql_fetch_row($result)) {
		$d = dispdate($rekord[18]);
		echo " &nbsp &middot <a href=$dokument?op=sentitems&day=$rekord[18]>$d</a><br>";
	}
	mysql_free_result($result);
}

echo "<br><a href=$dokument?op=stats>STATS</a><br>\n";

# /MENU

echo "</td>\n<td bgcolor=whitesmoke valign=top>\n";

#TRESC

if (isset($_GET['op'])) {
  	if ($_GET['op']=="inbox") {
		echo "<b>INBOX";
		if (isset($_GET['day']) && $_GET['day']!="") {
			$d = $_GET['day'];
			$d2 = dispdate($d);
			echo " $d2";
		}
		echo "</b><br><br>\n";
		echo "<table width=650 cellspacing=1 border=1>";
		echo "<tr bgcolor=gold><td>ID</td>\n";
		echo "<td>FROM</td>\n";
		echo "<td>SMSC</td>\n";
		echo "<td>CLASS</td>\n";
		echo "<td>RECEIVE TIME</td>\n";
		if ($delall) {
			echo "<td><a href=$dokument?op=del&dzial=inbox Title='Click to delete all SMS' OnClick=\"return DelAll('Inbox');\">[X]</td></tr>\n";
		} else {
			echo "<td></td>\n";
		}
		if (!isset($_GET['day']) || $_GET['day']=="") {
	  		$result = mysql_db_query("$db_name","select $inbox from inbox order by DateTime desc");
		} else {
			$d = $_GET['day'];
	  		$result = mysql_db_query("$db_name","select $inbox from inbox where DateTime like '$d%' order by DateTime desc");
		}
		while($rekord = mysql_fetch_row($result)) {
			if (!isset($_GET['day']) || $_GET['day']=="") {
				if (isset($_GET['id']) && $_GET['id']!="" && $_GET['id'] == $rekord[9]) {
					echo "<tr><td>$rekord[9] <a href=$dokument?op=inbox title='Click to hide details'>[<<]</a></td>";
				} else {
					echo "<tr><td>$rekord[9] <a href=$dokument?op=inbox&id=$rekord[9] title='Click to display details'>[>>]</a></td>";
				}
			} else {
				if (isset($_GET['id']) && $_GET['id']!="" && $_GET['id'] == $rekord[9]) {
					echo "<tr><td>$rekord[9] <a href=$dokument?op=inbox&day=$d title='Click to hide details'>[<<]</a></td>";
				} else {
					echo "<tr><td>$rekord[9] <a href=$dokument?op=inbox&day=$d&id=$rekord[9] title='Click to display details'>[>>]</a></td>";
				}
			}
			echo "<td>$rekord[3]</td><td>$rekord[6]</td><td>$rekord[7]</td>";
			$d2 = dispdatetime($rekord[1]);
			echo "<td>$d2</td>";
			if (!isset($_GET['day']) || $_GET['day']=="") {
				echo "<td><a href=$dokument?op=del&dzial=inbox&id=$rekord[9] OnClick=\"return Del($rekord[9]);\" Title='Click to delete'>[X]</a></td></tr>";
			} else {
				echo "<td><a href=$dokument?op=del&dzial=inbox&day=$d&id=$rekord[9] OnClick=\"return Del($rekord[9]);\" Title='Click to delete'>[X]</a></td></tr>";
			}
			if (isset($_GET['id']) && $_GET['id']!="" && $_GET['id'] == $rekord[9]) {
				$op = $_GET['op'];
				$id = $_GET['id'];
				echo "</tr><tr><td colspan=6 bgcolor=white>";
				$d2 = dispdatetime($rekord[0]);
				echo "Last changed in DB: $d2<br>";
				dispsmsinfo($rekord[7],$rekord[5],$rekord[2],$rekord[8],$rekord[4]);
				echo "</td></tr>\n";				
			}
		}
		mysql_free_result($result);
		echo "</table>";
  	}
  	if ($_GET['op']=="outbox") {
		echo "<b>OUTBOX";
		if (isset($_GET['day']) && $_GET['day']!="") {
			$d = $_GET['day'];
			$d2 = dispdate($d);
			echo " $d2";
		}
		echo "</b><br><br>\n";
		echo "<table width=650 cellspacing=1 border=1>";
		echo "<tr bgcolor=gold><td>ID</td>\n";
		echo "<td>TO</td>\n";
		echo "<td>TIME 2BE SENT</td>\n";
		echo "<td>PARTS</td>\n";
		echo "<td>VALIDITY</td>\n";
		if ($delall) {
			echo "<td><a href=$dokument?op=del&dzial=outbox Title='Click to delete all SMS' OnClick=\"return DelAll('Outbox');\">[X]</td></tr>\n";
		} else {
			echo "<td></td>\n";
		}
		if (!isset($_GET['day']) || $_GET['day']=="") {
			$result = mysql_db_query("$db_name","select $outbox from outbox order by SendingDateTime desc");
		} else {
			$d = $_GET['day'];
			$result = mysql_db_query("$db_name","select $outbox from outbox where SendingDateTime like '$d%' order by SendingDateTime desc");
		}
		while($rekord = mysql_fetch_row($result)) {
			if (!isset($_GET['day']) || $_GET['day']=="") {
				if (isset($_GET['id']) && $_GET['id']!="" && $_GET['id'] == $rekord[8]) {
					echo "<tr><td>$rekord[8] <a href=$dokument?op=outbox title='Click to hide details'>[<<]</a></td>";
				} else {
					echo "<tr><td>$rekord[8] <a href=$dokument?op=outbox&id=$rekord[8] title='Click to display details'>[>>]</a></td>";
				}
			} else {
				if (isset($_GET['id']) && $_GET['id']!="" && $_GET['id'] == $rekord[8]) {
					echo "<tr><td>$rekord[8] <a href=$dokument?op=outbox&day=$d title='Click to hide details'>[<<]</a></td>";
				} else {
					echo "<tr><td>$rekord[8] <a href=$dokument?op=outbox&day=$d&id=$rekord[8] title='Click to display details'>[>>]</a></td>";
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
			if (!isset($_GET['day']) || $_GET['day']=="") {
				echo "</td><td><a href=$dokument?op=del&dzial=outbox&id=$rekord[8] Title='Click to delete' OnClick=\"return Del($rekord[8]);\" >[X]</a></td></tr>";
			} else {
				echo "</td><td><a href=$dokument?op=del&dzial=outbox&day=$d&id=$rekord[8] Title='Click to delete' OnClick=\"return Del($rekord[8]);\" >[X]</a></td></tr>";
			}

			if (isset($_GET['id']) && $_GET['id']!="" && $_GET['id'] == $rekord[8]) {
				$op = $_GET['op'];
				$id = $_GET['id'];
				echo "</tr><tr><td colspan=6 bgcolor=white>";
				$result2 = mysql_db_query("$db_name","select $outbox from outbox where ID='$id'");
				while($rekord2 = mysql_fetch_row($result2)) {
					$d2 = dispdatetime($rekord2[0]);
					echo "Last changed in DB: $d2<br>";
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
  	}
  	if ($_GET['op']=="sentitems") {
		echo "<b>SENT ITEMS";
		if (isset($_GET['day']) && $_GET['day']!="") {
			$d = $_GET['day'];
			$d2 = dispdate($d);
			echo " $d2";
		}		
		echo "</b><br><br>\n";
		echo "<table width=650 cellspacing=1 border=1>";
		echo "<tr bgcolor=gold><td>ID</td>\n";
		echo "<td>TO</td>\n";
		echo "<td>SMSC</td>\n";
		echo "<td>CLASS</td>\n";
		echo "<td>SEND TIME</td>\n";
		if ($delall) {
			echo "<td><a href=$dokument?op=del&dzial=sentitems Title='Click to delete all SMS' OnClick=\"return DelAll('Sent Items');\">[X]</td></tr>\n";
		} else {
			echo "<td></td>\n";
		}
		if (!isset($_GET['day']) || $_GET['day']=="") {
			$result = mysql_db_query("$db_name","select $sentitems from sentitems order by SendingDateTime");
		} else {
			$d = $_GET['day'];
			$result = mysql_db_query("$db_name","select $sentitems from sentitems where SendingDateTime like '$d%' group by ID order by ID");
		}
		while($rekord = mysql_fetch_row($result)) {
			if (!isset($_GET['day']) || $_GET['day']=="") {
				if (isset($_GET['id']) && $_GET['id']!="" && $_GET['id'] == $rekord[11]) {
					echo "<tr><td>$rekord[11] <a href=$dokument?op=sentitems title='Click to hide details'>[<<]</a></td>\n";
				} else {
					echo "<tr><td>$rekord[11] <a href=$dokument?op=sentitems&id=$rekord[11] title='Click to display details'>[>>]</a></td>\n";
				}
			} else {
				if (isset($_GET['id']) && $_GET['id']!="" && $_GET['id'] == $rekord[11]) {
					echo "<tr><td>$rekord[11] <a href=$dokument?op=sentitems&day=$d title='Click to hide details'>[<<]</a></td>\n";
				} else {
					echo "<tr><td>$rekord[11] <a href=$dokument?op=sentitems&day=$d&id=$rekord[11] title='Click to display details'>[>>]</a></td>\n";
				}
			}
			echo "<td>$rekord[5]</td>\n";
			echo "<td>$rekord[8]</td>\n";
			echo "<td>$rekord[9]</td>\n";
			$d2 = dispdatetime($rekord[0]);
			echo "<td>$d2</td>\n";
			if (!isset($_GET['day']) || $_GET['day']=="") {
				echo "<td><a href=$dokument?op=del&dzial=sentitems&id=$rekord[11] Title='Click to delete' OnClick=\"return Del($rekord[11]);\" >[X]</a></td></tr>";
			} else {
				echo "<td><a href=$dokument?op=del&dzial=sentitems&day=$d&id=$rekord[11] Title='Click to delete' OnClick=\"return Del($rekord[11]);\" >[X]</a></td></tr>";
			}
			echo "</tr>\n";
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
				echo "<td>POSITION</td>\n";
				echo "<td>SENDING TIME</td>\n";
				echo "<td>ERROR CODE</td>\n";
				echo "<td>STATUS</td></tr>\n";

				$result2 = mysql_db_query("$db_name","select $sentitems from sentitems where ID='$id'");
				while($rekord2 = mysql_fetch_row($result2)) {
					echo "<tr><td>SMS $rekord2[13]</td>\n";
					$d2 = dispdatetime($rekord2[0]);
					echo "<td>$d2</td>\n";
					echo "<td>$rekord2[15]</td>\n";
					echo "<td>$rekord2[14]</td></tr>";
				}
				mysql_free_result($result2);

				echo "</td></tr></table>\n";
			}
		}
		mysql_free_result($result);
		echo "</table>";
  	} 
	if ($_GET['op']=="newsms") {
		$rok=date("Y");
		$miesiac=date("m");
		$dzionek=date("d");
		$godzina=date("H");
		$minuta=date("i");
		$sekunda=date("s");

		echo "<b>NEW OUTBOX SMS</b><p>\n";
		echo "<form method=\"POST\" action=$dokument name=newsms>\n";
		echo "<input type=hidden name=op value=addsms>\n";

		echo "<table><tr><td>Recipient (eg. +48xxxxxxxxx)</td>";
		echo "<td><input name=number maxlength=20></td></tr>\n";

		echo "<tr><td>Sending date (year-month-day)</td>";
		echo "<td><input name=year maxlength=4 value=\"$rok\" size=4>\n";
		echo "<input name=month maxlength=2 value=\"$miesiac\" size=2>\n";
		echo "<input name=day maxlength=2 value=\"$dzionek\" size=2></td></tr>\n";

		echo "<tr><td>Sending time (hour:minute:second)</td><td><input name=hour maxlength=2 value=\"$godzina\" size=2>\n";
		echo "<input name=minute maxlength=2 value=\"$minuta\" size=2>\n";
		echo "<input name=second maxlength=2 value=\"$sekunda\" size=2></td></tr>\n";

		echo "<tr><td>Validity</td><td><select name=validity>\n";
		echo "<option value=-1 select>Default (taken from sending phone)\n";
		echo "<option value=0>5 minutes\n";
		echo "<option value=1>10 minutes\n";
		echo "<option value=255>max. time (depends on SMSC)\n";
		echo "</select></td></tr>\n";

		echo "<td colspan=2><input type=checkbox name=class value=0>Send class 0 SMS</input><br>\n";

		echo "<textarea name=tresc rows=5 cols=32 maxlength=160></textarea><br>\n";

		echo "<input type=submit name=send value=SEND OnClick=\"if (newsms.number.value=='') {alert('Sender number not filled'); return false;} else return true;\"></td></tr>\n";

		echo "</table></form>\n";
		echo "<table width=650 cellspacing=1 border=0>";
		echo "<tr><td>&nbsp;</td></tr></table>\n";
  	}
  	if ($_GET['op']=="stats") {
		echo "<b>STATISTICS</b><br>Not filled yet\n";
		echo "<table width=650 cellspacing=1 border=0>";
		echo "<tr><td>&nbsp;</td></tr></table>\n";
	}
}

echo "</td></tr>\n<tr>\n<td colspan=2 height=15 bgcolor=red align=center>\n";
echo "<b><font color=white size=2 face=verdana>";
echo "Version 0.0.2 (c) 2004 by Michal Kruger & <a href=mailto:marcin@mwiacek.com>Marcin Wiacek</a>. Part of <a href=http://www.mwiacek.com>Gammu</a> project.";
echo "</font></b></a>\n</td>\n</tr>\n</table>";
?>
