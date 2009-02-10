<?

$dbpass = @mysql_connect("localhost","root","root");
if ($dbpass) $dbconnect = mysql_select_db("autoreply");

echo "<html><head>\n";
echo "<link rel=\"stylesheet\" href=\"styl.css\" type=\"text/css\">\n";

if (isset($_POST['op']) && $_POST['op']=="newsequence") {
	$value = '0';

	$result2 = mysql_db_query("autoreply","select ID from rules order by ID desc");
	$rekord2 = mysql_fetch_row($result2);
	if ($rekord2 != '') {
		$value = $rekord2[0] + 1;
	}
        mysql_query ("insert into rules (ID,DB) VALUES('$value','$rekord2[0]')");
	echo "<meta http-equiv=\"refresh\" content=\"0;url=autoreply.php?azz=z\">\n";
}
if (isset($_POST['op']) && $_POST['op']=="delsequence") {
        mysql_query ("delete from rules where ID='".$_POST['id']."'");
        mysql_query ("delete from actions where ID='".$_POST['id']."'");
	echo "<meta http-equiv=\"refresh\" content=\"0;url=autoreply.php\">\n";
}

if (isset($_POST['op']) && $_POST['op']=="delrule") {
        mysql_query ("delete from rules where RuleID='".$_POST['ruleid']."'");
	echo "<meta http-equiv=\"refresh\" content=\"0;url=autoreply.php\">\n";
}
if (isset($_POST['op']) && $_POST['op']=="editrule") {
        mysql_query ("update rules set User='".$_POST['user']."',Pass='".$_POST['pass']."',PC='".$_POST['pc']."',SQL='".$_POST['sql']."',DB='".$_POST['db']."' WHERE RuleID='".$_POST['ruleid']."';");
	echo "<meta http-equiv=\"refresh\" content=\"0;url=autoreply.php?azz=z\">\n";
}
if (isset($_POST['op']) && $_POST['op']=="addrule") {
        mysql_query ("insert into rules (User,Pass,PC,ID,DB,SQL) VALUES('".$_POST['user']."','".$_POST['pass']."','".$_POST['pc']."','".$_POST['id']."','".$_POST['db']."','".$_POST['sql']."')");
	echo "<meta http-equiv=\"refresh\" content=\"0;url=autoreply.php?azz=z\">\n";
}

if (isset($_POST['op']) && $_POST['op']=="delaction") {
        mysql_query ("delete from actions where ActionID='".$_POST['actionid']."'");
	echo "<meta http-equiv=\"refresh\" content=\"0;url=autoreply.php\">\n";
}
if (isset($_POST['op']) && $_POST['op']=="editaction") {
        mysql_query ("update actions set User='".$_POST['user']."',User2='".$_POST['user2']."',User3='".$_POST['user3']."',Pass='".$_POST['pass']."',Pass2='".$_POST['pass2']."',Pass3='".$_POST['pass3']."',DB='".$_POST['db']."',DB2='".$_POST['db2']."',DB3='".$_POST['db3']."',PC='".$_POST['pc']."',PC2='".$_POST['pc2']."',PC3='".$_POST['pc3']."',SQL='".$_POST['sql']."' WHERE ActionID='".$_POST['actionid']."';");
	echo "<meta http-equiv=\"refresh\" content=\"0;url=autoreply.php?azz=z\">\n";
}
if (isset($_POST['op']) && $_POST['op']=="addaction") {
        mysql_query ("insert into actions (ID,User,User2,User3,Pass,Pass2,Pass3,DB,DB2,DB3,PC,PC2,PC3,SQL) VALUES('".$_POST['id']."','".$_POST['user']."','".$_POST['user2']."','".$_POST['user3']."','".$_POST['pass']."','".$_POST['pass2']."','".$_POST['pass3']."','".$_POST['db']."','".$_POST['db2']."','".$_POST['db3']."','".$_POST['pc']."','".$_POST['pc2']."','".$_POST['pc3']."','".$_POST['sql']."')");
	echo "<meta http-equiv=\"refresh\" content=\"0;url=autoreply.php?azz=z\">\n";
}

echo "<STYLE TYPE=\"text/css\">Body {background-color: gainsboro;} table.Body2 {font-weight: normal; font-size: 10pt; color: black;font-family: 'Tahoma' ;}</style>";

echo "<title>Admin page</title>\n";
echo "</head><body leftmargin=0 bottommargin=0 rightmargin=0 class=body2>";

if (!isset($_POST['op'])) {

	echo "<span style=\"font-family: Tahoma;\">";
	echo "<form method=\"POST\" action=autoreply.php>\n";
	echo "<input type=hidden name=op value=newsequence>\n";
	echo "<input type=submit value='NEW SEQUENCE'>";
	echo "</form><p>\n";

	$result = mysql_db_query("autoreply","select ID from rules group by ID ");
	while($rekord = mysql_fetch_row($result)) {
		echo "<hr>";

		echo "<form method=\"POST\" action=autoreply.php  onSubmit=\"return confirm('Do you want to DELETE this sequence ?');\">\n";
		echo "<a name=$rekord[0]><font size=+1>Sequence $rekord[0]</font>";
		echo "<input type=hidden name=op value=delsequence>\n";
		echo "<input type=hidden name=id value=$rekord[0]>\n";
		echo "<input type=submit value='DEL'>";
		echo "</form>\n";
	
		$count = 0;
		$result2 = mysql_db_query("autoreply","select ID,RuleID,DB,SQL,User,Pass,PC from rules where ID=$rekord[0]");
		while($rekord2 = mysql_fetch_row($result2)) {
			echo "<table width=100% class=body2><tr><td colspan=2><b>Existing rule</b></td></tr><tr><td valign=top>";
			echo "<form method=\"POST\" action=autoreply.php  onSubmit=\"return confirm('Do you want to set this rule ?');\">\n";
			echo "<input type=hidden name=op value=editrule>\n";
			echo "<input type=hidden name=ruleid value=$rekord2[1]>\n";

			echo "MySQL PC address<br><input name=pc value='$rekord2[6]'><br>";
			echo "MySQL user name<br><input name=user value='$rekord2[4]'><br>";
			echo "MySQL user password<br><input name=pass value='$rekord2[5]'><br>";
			echo "MySQL DB<br><input name=db value='$rekord2[2]'></td><td>";
			echo "MySQL SQL text<br><textarea name=sql rows=7 cols=96>$rekord2[3]</textarea></td>";
			echo "<td valign=top>Actions<br><input type=submit value=SET></form>";

			if ($count != 0) {
				echo "<br><form method=\"POST\" action=autoreply.php onSubmit=\"return confirm('Do you want to delete this rule ?');\">\n";
				echo "<input type=hidden name=op value=delrule>\n";
				echo "<input type=hidden name=ruleid value=$rekord2[1]>\n";
				echo "<input type=hidden name=id value=$rekord2[0]>\n";
				echo "<input type=submit value=DEL >";
				echo "</form>";
			}

			echo "</td></tr></table>\n";
			$count++;
		}
	
		echo "<table width=100% class=body2><tr><td colspan=2><b>New rule</b></td></tr><tr><td valign=top>";
		echo "<form method=\"POST\" action=autoreply.php  onSubmit=\"return confirm('Do you want to add this rule ?');\">\n";
		echo "<input type=hidden name=op value=addrule>\n";
		echo "<input type=hidden name=id value=$rekord[0]>\n";
		echo "MySQL PC address<br><input name=pc><br>";
		echo "MySQL user name<br><input name=user ><br>";
		echo "MySQL user password<br><input name=pass ><br>";
		echo "MySQL DB name<br><input name=db></td><td>";
		echo "MySQL SQL text<br><textarea name=sql rows=7 cols=96></textarea></td>";
		echo "<td valign=top>Actions<br><input type=submit value=ADD></form></td></tr></table>";
	
		$count = 0;
		$result2 = mysql_db_query("autoreply","select ActionID, User,User2,Pass,Pass2,DB,DB2,PC,PC2,SQL,User3,Pass3,DB3,PC3 from actions where ID=$rekord[0]");
		while($rekord2 = mysql_fetch_row($result2)) {
			echo "<table width=100% class=body2><tr><td colspan=2><b>Existing action</b><td></tr><tr><td>";
			echo "<form method=\"POST\" action=autoreply.php name='current_$rekord2[0]'  onSubmit=\"return confirm('Do you want to set this action ?');\">\n";
			echo "<input type=hidden name=op value=editaction>\n";
			echo "<input type=hidden name=actionid value=$rekord2[0]>\n";
			echo "MySQL PC address<br><input name=pc value='$rekord2[7]'><br><input name=pc2 value='$rekord2[8]'><br><input name=pc3 value='$rekord2[13]'><br>";
			echo "MySQL User name<br><input name=user  value='$rekord2[1]'><br><input name=user2 value='$rekord2[2]'><br><input name=user3  value='$rekord2[10]'><br>";
			echo "MySQL User password<br><input name=pass  value='$rekord2[3]'><br><input name=pass2 value='$rekord2[4]'><br><input name=pass3  value='$rekord2[11]'><br>";
			echo "MySQL DB name<br><input name=db value='$rekord2[5]'><br><input name=db2 value='$rekord2[6]'><br><input name=db3 value='$rekord2[12]'></td><td>MySQL SQL text<br>";
			echo "<textarea name=sql rows=18 cols=96>$rekord2[9]</textarea></td><td valign=top>";
			echo "Actions<br><input type=submit value=SET></form><br>";
			if ($count != 0) {
				echo "<form method=\"POST\" action=autoreply.php onSubmit=\"return confirm('Do you want to delete this action ?');\">\n";
				echo "<input type=hidden name=op value=delaction>\n";
				echo "<input type=hidden name=actionid value=$rekord2[0]>\n";
				echo "<input type=submit value=DEL>";
				echo "</form>";
			}
			echo "</td></tr></table>";
			$count++;
		}

		echo "<table width=100% class=body2><tr><td colspan=2><b>New action</b></td></tr><tr><td valign=top>";
		echo "<form method=\"POST\" action=autoreply.php name='new_$rekord[0]' onSubmit=\"return confirm('Do you want to add new action ?');\">\n";
		echo "<input type=hidden name=op value=addaction>\n";
		echo "<input type=hidden name=id value=$rekord[0]>\n";
		echo "MySQL PC address<br><input name=pc><br><input name=pc2><br><input name=pc3><br>";
		echo "MySQL User name<br><input name=user><br><input name=user2><br><input name=user3><br>";
		echo "MySQL User password<br><input name=pass><br><input name=pass2><br><input name=pass3><br>";
		echo "MySQL DB name<br><input name=db><br><input name=db2><br><input name=db3></td>";
		echo "<td>MySQL SQL text<br><textarea name=sql rows=18 cols=96></textarea></td>";
		echo "<td valign=top>Actions<br><input type=submit value=ADD></form></td></tr></table>";
	}
	echo "<hr>Version from 19.09.2005, created by <a href=http://www.mwiacek.com>Marcin Wiacek</a>, written for Michal Kruger";
}

echo "</body></html>";

?>