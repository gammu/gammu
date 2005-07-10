<?

$dbpass = @mysql_connect("localhost","root","");
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

echo "<title>Admin page</title>\n";
echo "</head><body leftmargin=0 bottommargin=0 rightmargin=0>";

if (!isset($_POST['op'])) {

	echo "<span style=\"font-family: Tahoma;\">";
	echo "<form method=\"POST\" action=autoreply.php>\n";
	echo "<input type=hidden name=op value=newsequence>\n";
	echo "<input type=submit value='NEW SEQUENCE'>";
	echo "</form><p>\n";

	$result = mysql_db_query("autoreply","select ID from rules group by ID ");
	while($rekord = mysql_fetch_row($result)) {
		echo "<hr><table width=100% border=0><tr><td >";

		echo "<a name=$rekord[0]><font size=+2>Sequence $rekord[0]</font></td>\n<td></td><td>";
		echo "<form method=\"POST\" action=autoreply.php>\n";
		echo "<input type=hidden name=op value=delsequence>\n";
		echo "<input type=hidden name=id value=$rekord[0]>\n";
		echo "<input type=submit value='DEL'>";
		echo "</form></td></tr>\n";
	
		echo "<tr><td colspan=3>&nbsp;<br><b>Rules</b></td></tr>";

		$count = 0;
		$result2 = mysql_db_query("autoreply","select ID,RuleID,DB,SQL,User,Pass,PC from rules where ID=$rekord[0]");
		while($rekord2 = mysql_fetch_row($result2)) {
			echo "<tr><td >";
			echo "<form method=\"POST\" action=autoreply.php>\n";
			echo "<input type=hidden name=op value=editrule>\n";
			echo "<input type=hidden name=ruleid value=$rekord2[1]>\n";

			echo "<nobr>";
			echo "PC<input name=pc value='$rekord2[6]'>";
			echo "User<input name=user value='$rekord2[4]'>";
			echo "Pass<input name=pass value='$rekord2[5]'>";
			echo "DB<input name=db value='$rekord2[2]'></nobr>";
			echo "<br>SQL text<textarea name=sql rows=2 cols=55>$rekord2[3]</textarea></td>";

			echo "<td valign=top><input type=submit value=SET></td></form><td valign=top>\n";
			if ($count != 0) {
				echo "<form method=\"POST\" action=autoreply.php>\n";
				echo "<input type=hidden name=op value=delrule>\n";
				echo "<input type=hidden name=ruleid value=$rekord2[1]>\n";
				echo "<input type=hidden name=id value=$rekord2[0]>\n";
				echo "<input type=submit value=DEL>";
				echo "</form>";
			}
			echo "</td></tr>\n";
			$count++;
		}
	
		echo "<tr><td>";
		echo "<form method=\"POST\" action=autoreply.php>\n";
		echo "<input type=hidden name=op value=addrule>\n";
		echo "<input type=hidden name=id value=$rekord[0]>\n";
		echo "<nobr>";
		echo "pc<input name=pc>";
		echo "user<input name=user >";
		echo "pass<input name=pass >";
		echo "db<input name=db></nobr>";
		echo "<br>SQL text<textarea name=sql rows=2 cols=55></textarea></td>";
		echo "<td></td><td valign=top>\n";
		echo "<input type=submit value=ADD>";
		echo "</form></td></tr>\n";
	
		echo "<tr><td colspan=3>&nbsp;<br><b>Actions</b></td></tr>";

		$count = 0;
		$result2 = mysql_db_query("autoreply","select ActionID, User,User2,Pass,Pass2,DB,DB2,PC,PC2,SQL,User3,Pass3,DB3,PC3 from actions where ID=$rekord[0]");
		while($rekord2 = mysql_fetch_row($result2)) {
			echo "<tr><td><table cellspacing=0 cellpadding=0 border=0><tr><td colspan=3>";
			echo "<form method=\"POST\" action=autoreply.php name='current_$rekord2[0]'>\n";
			echo "<input type=hidden name=op value=editaction>\n";
			echo "<input type=hidden name=actionid value=$rekord2[0]>\n";
			echo "<nobr>PC<input name=pc value='$rekord2[7]'>";
			echo "User<input name=user  value='$rekord2[1]'>Pass<input name=pass  value='$rekord2[3]'>DB<input name=db value='$rekord2[5]'></nobr><br>";
			echo "<nobr>PC<input name=pc2 value='$rekord2[8]'>";
			echo "User<input name=user2 value='$rekord2[2]'>Pass<input name=pass2 value='$rekord2[4]'>DB<input name=db2 value='$rekord2[6]'></nobr><br>";
			echo "<nobr>SQL PC<input name=pc3 value='$rekord2[13]'>";
			echo "SQL user<input name=user3  value='$rekord2[10]'>SQL pass<input name=pass3  value='$rekord2[11]'>SQL DB<input name=db3 value='$rekord2[12]'></nobr></td>";
			echo "</tr>\n";
			echo "<tr><td></td><td valign=top>SQL text</td>";
			echo "<td colspan=2><textarea name=sql rows=2 cols=55>$rekord2[9]</textarea></td></tr></table>";
			echo "</td><td valign=top><input type=submit value=SET></td><td valign=top>";
			echo "</form>";
			if ($count != 0) {
				echo "<form method=\"POST\" action=autoreply.php>\n";
				echo "<input type=hidden name=op value=delaction>\n";
				echo "<input type=hidden name=actionid value=$rekord2[0]>\n";
				echo "<input type=submit value=DEL>";
				echo "</form>";
			}
			echo "</td></tr>";
			$count++;
		}

		echo "<tr><td><table border=0 cellspacing=0 cellpadding=0><tr><td colspan=3>";
		echo "<form method=\"POST\" action=autoreply.php name='new_$rekord[0]'>\n";
		echo "<input type=hidden name=op value=addaction>\n";
		echo "<input type=hidden name=id value=$rekord[0]>\n";
		echo "<nobr>PC<input name=pc>";
		echo "User<input name=user>Pass<input name=pass>DB<input name=db></nobr><br>";
		echo "<nobr>PC<input name=pc2>";
		echo "User<input name=user2>Pass<input name=pass2>DB<input name=db2></nobr><br>";
		echo "<nobr>SQL PC<input name=pc3>";
		echo "SQL user<input name=user3>SQL pass<input name=pass3>SQL DB<input name=db3></nobr></td>";
		echo "</tr>\n";
		echo "<tr><td></td><td valign=top>";
		echo "SQL text</td><td><textarea name=sql rows=2 cols=55></textarea></td></tr></table>";
		echo "</td><td></td><td valign=top>";
		echo "<input type=submit value=ADD></td></tr>";
		echo "</form>";
		
		echo "</table>\n";
	}
	echo "<hr>(c) 2005 by <a href=http://www.mwiacek.com>Marcin Wiacek</a>, written for Michal Kruger";
}

echo "</body></html>";

?>