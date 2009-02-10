<html>
<head>
</head>
<body>
<FORM ACTION=sms.php method="post">
Search SMS-es by first entered chars:
<INPUT NAME=Char SIZE=20>
<INPUT TYPE=SUBMIT VALUE="Search"><br/><br/>
</FORM>

<?//this script was created by Michal Holes (michal@holes.sk)
//under GNU GPL for showing of the received SMS-es by 
//great sms daemon GAMMU saved to standard MySQL database and table
//created by script GAMMU

function mysql_timestamp_to_human($dt) //mysql timestamp to human readable
{
	$yr=strval(substr($dt,0,4)); //year
	$mo=strval(substr($dt,4,2)); //month
	$da=strval(substr($dt,6,2)); //day
	$hr=strval(substr($dt,8,2)); //hour
	$mi=strval(substr($dt,10,2)); //minute
	//$se=strval(substr($dt,12,2)); //sec

        return date("d M Y H:i", mktime ($hr,$mi,0,$mo,$da,$yr)).""; //format of displayed date and time
} 

mysql_connect("localhost", "sms", "sms") or die(mysql_error()); //connect to mysql
mysql_select_db("sms") or die(mysql_error()); //select database
if ($Char!="") //check if char was inserted
    $If = "WHERE TextDecoded LIKE '".AddSlashes($Char)."%'"; //if yes, change variable
else 
    $If = ""; //or do nothing

$result = mysql_query("SELECT * FROM inbox $If ORDER BY ReceivingDateTime"); //select data with variable and order it by time and date of the reception

echo "<table border=1>"; //begin creation of table
echo "<tr> <th>Datum a cas </th><th>Tel. cislo</th> <th>Sprava</th> </tr>"; //header of the table
while($row = mysql_fetch_array( $result )) { //create variable row
	// Print out the contents of each row into a table

	echo "<tr><td>"; //begin of row and cell	
	$d= $row['ReceivingDateTime']; //create variable for function at begin
	echo mysql_timestamp_to_human($d); //paste normal date
	echo "</td><td>";// end and begin of cell
	echo $row['SenderNumber']; //show sender number
	echo "</td><td>"; //end and begin of cell
	echo $row['TextDecoded']; //show text of the message
	echo "</td></tr>"; //end of the cell and row
}
echo "</table>"; //end of table

?>
<br><br><br>
</body>
</html>
