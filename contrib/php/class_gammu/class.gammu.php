<?php
/*-----------------------------------------+
| Gammu phpClass
| Author      : Stieven R. Kalengkian
| Contact     : stieven.kalengkian@gmail.com
| Website     : www.sleki.org - My Blog http://stieven.glowciptamedia.com/
| Version     : 3.0
| Last Update : Dec, 08 2009
|
| Tested on
| OS   			: Linux Slackware 12.0 and Windows XP Pro (unlicense demo only)
| PHP ver.   	: 5.2.11
| MySQL ver		: 5.1
| Apache		: 1.x and 2.0
| Gammu Version : [Gammu version 1.26.90 built 15:57:29 on Oct 12 2009 using GCC 4.3, MinGW 3.11]
|
|
| Description:
| Gammu API with PHP
| (only if you wan't running daemon)
| I recommended you to use Gammu
| Please visit Official Gammu Website for more information about Gammu
| http://wammu.eu/gammu
|
|
| Change log:
|
| v3.0
|	**!!!! Warning... if you use class version 2.x or below 
|   **!!!! it is not compatible with this version (3.x)
|	**!!!! Please check.. this class only work on PHP 5.2+
| - New method for new gammu technology,
| - Fully support for Windows or Linux OS
|
| v2.1
| - Add function enable_sudo([int]) 
|   set 1 if using sudo for gammu command exe
------------------------------------------*/

class gammu {
	/* Initializing gammu bin/EXE */
	var $gammu = "/usr/local/bin/gammu";
	var $datetime_format = 'Y-m-d H:i:s';
	
	function __construct($gammu_bin_location='',$gammu_config_file='',$gammu_config_section='')
	{
		$this->gammu = $gammu_bin_location ? $gammu_bin_location : '/usr/local/bin/gammu';
		if (!file_exists($this->gammu)) {
			$this->error("Can not found <b><u>{$this->gammu}</u></b> or Gammu is not installed\r\n");
		} else {
			$this->gammu = $gammu_config_file != '' ? $this->gammu." -c {$gammu_config_file}" : $this->gammu;
			$this->gammu = $gammu_config_section != '' ? $this->gammu." -s ". (int) $gammu_config_section ."" : $this->gammu;
		}
	}
	
	function gammu_exec($options='--identify',$break=0) {
		$exec=$this->gammu." ".$options;
		exec($exec,$r);
		if ($break == 1) { return $r; }
		else { return $this->unbreak($r); }
	}
	
	function unbreak($r) {
		for($i=0;$i<count($r);$i++) {
			$response.=$r[$i]."\r\n";
		}
		return $response;
	}
	
	function Identify(&$response)
	{
		$r = $this->gammu_exec('--identify',1);
		if (preg_match("#Error opening device|No configuration file found|Gammu is not installed#si", $this->unbreak($r),$s)) {
			$response = $r;
			return 0;
		}  else {
			for($i=0;$i<count($r);$i++) {
				//if (preg_match("#^(Manufacturer|Model|Firmware|IMEI|Product code|SIM IMSI).+:(.+)#",$r[$i],$s)) {
				if (preg_match("#^(.+):(.+)#",$r[$i],$s)) {
					if (trim($s[1]) and trim($s[2])) { $response[str_replace(" ","_",trim($s[1]))]=trim($s[2]); }
				}
			}
			$r = $this->gammu_exec('--monitor 1',1);
			for($i=0;$i<count($r);$i++) {
				if (preg_match("#^(.+):(.+)#",$r[$i],$s)) {
					if (trim($s[1]) and trim($s[2])) { $response[str_replace(" ","_",trim($s[1]))]=trim($s[2]); }
				}
			}
			return 1;
		}
	}
	
	function Get()
	{
		$r = $this->gammu_exec('--getallsms 1',1);
		$data = array();
		$x = 0; $y = 0;
		for($i=0;$i<count($r);$i++) {
			if (preg_match("#^SMS message#",$r[$i])) { continue; }
			if (preg_match("#^Location (.+), folder \"(.+)\"#",$r[$i],$s)) {
				$folder=strtolower(trim($s[2]));
				if ($folder == "outbox") {
					   $fid=$x;
					   $x++;
				}
				if ($folder == "inbox") {
					   $fid=$y;
					   $y++;
				}
				$data[$folder][$fid]=array();
				$data[$folder][$fid]['location']=trim($s[1]);
			}
			else if (preg_match("/(.+)Concatenated \(linked\) message, ID \((.+)\) (.+), part (.+) of (.+)/",$r[$i],$d)) {
                           $data[$folder][$fid]['link']['coding']=trim($d[2]);
                           $data[$folder][$fid]['link']['id']=trim($d[3]);
                           $data[$folder][$fid]['link']['part']=trim($d[4]);
            }
			else if (preg_match("#(.+): (.+)#si",$r[$i],$s)) {
				if (trim($s[1]) == 'Sent') { $s[2]=date($this->datetime_format,strtotime(trim(trim($s[2]),'"'))); }
				if (trim($s[1]) and trim($s[2])) { 
					$data[$folder][$fid][strtolower(str_replace(" ","_",trim($s[1])))]=trim(trim($s[2]),'"');
				}
			}
			else {
				//By Pass last line
				if (preg_match("/(.+) SMS parts in (.+) SMS sequences/",$r[$i],$xxx)) continue;
				//Buffer BODY
				if (trim($r[$i])) { $data[$folder][$fid]['body'].=trim($r[$i]); }
			}
			$data[$folder][$fid]['ID']=md5(serialize($data[$folder][$fid]));
		}
		
		return $data;
	}
	
	function Send($number,$text,&$respon) {
		$respon = $this->gammu_exec("--sendsms TEXT {$number} -len ". strlen($text)." -text \"{$text}\"");
		if (eregi("OK",$respon)) { return 1; } else { return 0; }
	}
	
	function phoneBook($mem = 'ME')
	{
		$r = $this->gammu_exec('--getallmemory '.$mem,1);
		$data = array();
		$x=0; $sx = 0;
		for($i=0;$i<count($r);$i++) {
			if (preg_match("#^Memory (.+), Location (.+)#",$r[$i],$d)) {
				$x=$sx;
				if (!trim($d[1])) continue;
				$data[$x]['Location']=trim($d[2]);
				$data[$x]['MEM']=trim($d[1]);
				$sx++;
			}
			if (preg_match("#(^Email.+): (.+)#si",$r[$i],$s)) {
				$data[$x]['email'][]=trim(trim($s[2]),'"');
			}
			else if (preg_match("#(.+): (.+)#si",$r[$i],$s)) {
				$data[$x][strtolower(str_replace(" ","_",trim($s[1])))]=trim(trim($s[2]),'"');
			}
		}
		return $data;
	}
	
	function error($e,$exit=0) {
		echo $e."\n";
		if ($exit == 1) { exit; }
	}
}

/*-----------------------------------------
|
| :::::::::::: EXAMPLE ::::::::::::::::::
|
-----------------------------------------*/
/** /
$gammu_bin 				= dirname(__FILE__).'/gammu/bin/gammu.exe';
$gammu_config 			= dirname(__FILE__).'/gammurc.txt';
$gammu_config_section	= '1'; // for default section please set "blank" value --> $gammu_config_section = '';

$sms = new gammu($gammu_bin,$gammu_config,$gammu_config_section);

/* Identify Device information * /
$sms->Identify($response);
echo '<pre>';
print_r($response); 
echo '</pre>'; 

/* Get SMS from Device* /
$response = $sms->Get();
echo '<pre>';print_r($response); echo '</pre>'; 

/* Sending SMS * /
$sms->Send('+6281380830000','Test!',$response);
echo '<pre>';
print_r($response); echo '</pre>'; 

/* Get Phone -> ME = Phone Memory; SM = Sim Card;  options list => DC|MC|RC|ON|VM|SM|ME|MT|FD|SL * /
$response = $sms->phoneBook('ME');
echo '<pre>';print_r($response); echo '</pre>'; 
/**/

?>