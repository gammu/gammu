<?

//script takes SMS from Inbox from MySQL SMSD DB (sms/Inbox), joins linked
//and saves them to new DB (newsms/linked)

$ip = "localhost";
$user = "root";
$pass = "";
$smsd_db = "sms";
$new_db = "newsms";
$new_db_table = "linked";

//internal source

$inbox = "UpdatedInDB,ReceivingDateTime,Text,SenderNumber,Coding,UDH,SMSCNumber,Class,TextDecoded,ID,RecipientID";

//adds SMS to new table and marks processed sms as processed
function copysms($Text,$Sender,$DateTime,$id)
{
	global $new_db, $new_db_table, $smsd_db;

	if (eregi("([0-9]{1,})(#)([a-z]{1,})(#)(.*)", $Text, $regs)) {
		$testo = "".$regs[5]."";
		$category = "".$regs[1]."";
		$off_rich = "".$regs[3]."";
		if ($off_rich=='r' || $off_rich=='R') {
			$offerta_richiesta="a";
		} else {
			$offerta_richiesta="v";
		}
	} else {
		$testo = $Text;
		$category = "45";
		$offerta_richiesta = "v";
	}
	mysql_select_db("$new_db");
	$result = mysql_query("insert into $new_db_table (ID,Text_Offer,Text_Category,PhoneNumber,Text_Other,DateTime) values ('$id','$offerta_richiesta','$category','$Sender','" . mysql_real_escape_string($testo) . "','$DateTime')");

	//mark sms in Gammu smsd as processed
	mysql_select_db("$smsd_db");
	while ($id!="") {
		if (eregi("([0-9]{1,})(,)(.*)", $id, $id2)) {
//			echo $id2[1]."<br>";
			$result = mysql_query("update inbox set Processed='true' WHERE ID='$id2[1]'");
			$id = $id2[3];
		} else {
			$result = mysql_query("update inbox set Processed='true' WHERE ID='$id'");
//			echo $id;
			break;
		}
	}
//	echo "<br><br>";
}

echo "<HTML>\n<HEAD>\n";
echo "<TITLE>SMS</TITLE></HEAD><body>\n";

$dbpass = @mysql_connect("$ip","$user","$pass");
if (!$dbpass) {
	echo "error connecting to db\n";
} else {
	//searching for sms in Inbox table from Gammu SMSD

	$result = mysql_db_query("$smsd_db","select $inbox from inbox where Processed='false' AND Coding NOT IN ('8bit')");
	$counter_complete = 0;
	$counter_incomplete = 0;
	while($rekord = mysql_fetch_row($result)) {
		$udh = $rekord[5];
		$complete = false;

		//we have single sms
		if ($udh == "") {
			copysms($rekord[8],$rekord[3],$rekord[1],$rekord[9]);
			$counter_complete++;
		}
		//we have linked sms with 8 bit ID (short UDH)
		if (strlen($udh) == 12 && $udh[0] == 0 && $udh[1] == 5 && $udh[2] == 0 && $udh[3] == 0 &&
                                          $udh[4] == 0 && $udh[5] == 3 && $udh[10] == 0 && $udh[11] == 1) {
			$complete = true;
			$text = $rekord[8];
			$id = $rekord[9];
			//we search for all sms in sequence
			for ($i = 2; $i < ($udh[8]*10+$udh[9])+1; $i++) {
				$udh2 = "050003$udh[6]$udh[7]$udh[8]$udh[9]";
				if ($i < 10) {
					$udh2 = "$udh2"."0$i";
				} else {
					$udh2 = "$udh2"."$i";
				}
				$result2 = mysql_db_query("sms","select $inbox from inbox where UDH='$udh2' AND SenderNumber='$rekord[3]' && SMSCNumber='$rekord[6]' && Processed='false'");
				$rekord2 = mysql_fetch_row($result2);
				mysql_free_result($result2);
				if ($rekord2 != NULL) {
					$text="$text"."$rekord2[8]";
					$id = "$id,"."$rekord2[9]";
				} else {
					$counter_incomplete++;
					$complete = false;
					break;
				}
			}
		}
		//we have linked sms with 16 bit ID (long UDH)
		if (strlen($udh) == 14 && $udh[0] == 0 && $udh[1] == 6 && $udh[2] == 0 && $udh[3] == 8 &&
                                          $udh[4] == 0 && $udh[5] == 4 && $udh[12] == 0 && $udh[13] == 1) {
			$text = $rekord[8];
			$id = $rekord[9];
			$complete = true;
			//we search for all sms in sequence
			for ($i = 2; $i < ($udh[10]*10+$udh[11])+1; $i++) {
				$udh2 = "060804$udh[6]$udh[7]$udh[8]$udh[9]$udh[10]$udh[11]";
				if ($i < 10) {
					$udh2 = "$udh2"."0$i";
				} else {
					$udh2 = "$udh2"."$i";
				}
				$result2 = mysql_db_query("sms","select $inbox from inbox where UDH='$udh2' AND SenderNumber='$rekord[3]' && SMSCNumber='$rekord[6]' && Processed='false'");
				$rekord2 = mysql_fetch_row($result2);
				mysql_free_result($result2);
				if ($rekord2 != NULL) {
					$text="$text"."$rekord2[8]";
					$id = "$id,"."$rekord2[9]";
				} else {
					$counter_incomplete++;
					$complete = false;
					break;
				}
			}
		}
		//all sms from linked sequence found
		if ($complete == true) {
			copysms($text,$rekord[3],$rekord2[1],$id);
			$counter_complete++;
		}
	}
	mysql_free_result($result);
	echo "$counter_complete SMS sequences processed correctly, ";
	echo "$counter_incomplete SMS sequences incomplete";
}

echo "</body></html>";
?>
