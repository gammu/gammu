<? 
require_once "class.gammu_nix_win32_v2.php"; 
//$sms = new gammu("[LOCATE_OF_GAMMUEXE]");
$sms = new gammu("gammu.exe");
$sms->setFile("smsbox.txt");
if ($_POST['action'] == "1") {
	if ($sms->Send($_POST['number'],$_POST['text'],$respon)) {
		echo "<script>alert('SMS SENDING');location.replace('?cmd=write')</script>";
	} else { echo "<script>alert('".str_replace("\n","",trim($respon))."');</script>"; }
}
if ($_GET['cmd'] == "delete") {
	$sms->ReadTXT($respon);
	$sms->DeleteTXT($_GET['id'],$_GET['box']);
	echo "<script>location.replace('?cmd=inbox')</script>";
	die();
}
if ($_GET['cmd'] == "download") {
	$sms->Get(0,$respon);
	//echo $respon."<br>";
	$sms->SaveTXT($respon);
	//echo $respon;
	echo "<script>alert('$respon');location.replace('?cmd=inbox')</script>";
	die();
}
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<title>class.gammu_nix_win32.php EXAMPLE</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">

<style type="text/css">
<!--
body,td,th {
	font-family: Geneva, Arial, Helvetica, sans-serif;
	font-size: 11px;
}
a:link {
	color: #0000FF;
}
a:visited {
	color: #0000FF;
}
a:hover {
	color: #0000FF;
}
a:active {
	color: #0000FF;
}
-->
</style>
</head>

<body>
<table width="100%"  border="0" cellspacing="1" cellpadding="2">
  <tr>
    <td><table width="100%"  border="0" cellpadding="2" cellspacing="1" bgcolor="#0000FF">
      <tr align="center" bgcolor="#FFFF00">
        <td width="25%"><strong><a href="?cmd=inbox">INBOX</a></strong></td>
        <td width="25%"><strong><a href="?cmd=outbox">OUTBOX</a></strong></td>
        <td width="25%"><a href="?cmd=write"><strong>WRITE SMS </strong></a></td>
        <td width="25%"><a href="?cmd=download"><strong>DOWNLOAD SMS</strong></a></td>
      </tr>
    </table></td>
  </tr>
  <? if (!$_GET['cmd'] or $_GET['cmd'] == 'inbox') { ?>
  <tr>
    <td align="center">
	<?
	$sms->ReadTXT($respon);
	
	?>
	<form name="form1" method="post" action="">
	  <strong>Click to delete</strong> 
      <table width="100%"  border="0" cellpadding="2" cellspacing="1" bgcolor="#0000FF">
        <tr align="center" bgcolor="#00FF99">
          <td width="100"><strong>Date</strong></td>
          <td width="150"><strong>From</strong></td>
          <td><strong>Body</strong></td>
          <td width="100"><strong>SMS Center</strong></td>
        </tr>
		<?
		if ($sms->dataBuffer['Inbox']) {
		foreach ($sms->dataBuffer['Inbox'] as $msg) {
		list($k,$v)=each($sms->dataBuffer['Inbox']);
		?>
        <tr bgcolor="#FFFFFF" onClick="if (confirm('Are you sure to delete')) { location.replace('?cmd=delete&id=<?=$k;?>&box=Inbox'); }" onMouseOver="this.style.background='yellow';this.style.cursor='pointer'" onMouseOut="this.style.background='white'">
          <td><?=date("M, d y H:i",strtotime($msg['SentTime']));?></td>
          <td><?=$msg['Number'];?></td>
          <td><?=$msg['body'];?></td>
          <td><?=$msg['SMSC'];?></td>
        </tr>
		<? } } ?>
      </table>
    </form></td>
  </tr>
  <? } else if ($_GET['cmd'] == 'outbox') { ?>
  <tr>
    <td>
	<?
	$sms->ReadTXT($respon);
	?>
	<form name="form2" method="post" action="">
      <table width="100%"  border="0" cellpadding="2" cellspacing="1" bgcolor="#0000FF">
        <tr align="center" bgcolor="#00FF99">
          <td width="150"><strong>To</strong></td>
          <td><strong>Body</strong></td>
          </tr>
        <?
		foreach ($sms->dataBuffer['Outbox'] as $msg) { 
		?>
        <tr bgcolor="#FFFFFF">
          <td><?=$msg['Number'];?></td>
          <td><?=$msg['body'];?></td>
          </tr>
        <? } ?>
      </table>
    </form></td>
  </tr>
  <? } else if ($_GET['cmd'] == 'write') { ?>
  <tr>
    <td><form name="form3" method="post" action="">
      <table width="100%"  border="0" cellspacing="1" cellpadding="2">
        <tr>
          <td width="150">Number To</td>
          <td><input name="number" type="text" id="number"></td>
        </tr>
        <tr>
          <td>&nbsp;</td>
          <td><textarea name="text" cols="50" rows="5" id="text"></textarea></td>
        </tr>
        <tr>
          <td>&nbsp;</td>
          <td><input type="button" name="Button" value="SEND" onClick="this.form.action.value=1;this.value='CONNECTING... PLEASE WAIT!!!!';this.disabled='true';this.form.submit();">
		  <input name="action" value="0" type="hidden">		  </td>
        </tr>
      </table>
    </form></td>
  </tr>
<? } ?>
  <tr>
    <td>&nbsp;</td>
  </tr>
</table>

</body>
</html>
