<?php
/*-----------------------------------------+
| Gammu phpClass
| Author      : Stieven R. Kalengkian
| Contact     : stieven.kalengkian@gmail.com
| Website     : www.glowciptamedia.com
| Version     : 2.1
| Last Update : April, 26 2007
| Tested OS   : Linux Slackware 11.0 and Windows XP Pro (unlicense demo only)
| PHP ver.    : 5.1
| MySQL ver   : 5.0
| Apache Web Server  : 1.x and 2.0
| Gammu Version : 1.10.00 built 22:52:25 Apr 18 2006 in MS VC .NET 2003 (win32 and linux)
|
| Solution Problem :
| The solution to the "No configuration file found" error is to put a copy of the gammurc
| in the directory whrein your php files are found specifically the one that uses the gammu class.
| Reference >> Classes of Davis Muhajereen D. Dimalen
|
| Description:
| Gammu API with PHP
| (only if you wan't running daemon)
| I recommended you to use Gammu
| --smsd MYSQL see
| http://www.gammu.org/wiki/index.php?title=Gammu:Full_reference_manual
| this is my custom API-SMS.
|
| Gammu Command :
| gammu nothing --identify
| gammu nothing --deleteallsms [INBOX_ID]
| gammu nothing --getsmsfolders
| gammu nothing --getallsms
| gammu nothing --getallmemory ME|SM
| echo [text] | gammu nothing --sendsms TEXT [number]
|
| Change log
| v2.1
| - Add function enable_sudo([int]) 
|   set 1 if using sudo for gammu command exe
------------------------------------------*/

class gammu {

/* Initializing gammu bin/EXE */
var $gammu = "/usr/local/bin/gammu";
var $replacemen = array("\n","\t","\r");
var $dataBuffer;
var $filedb = "smsbox.txt";
var $TextEncrypt = 0;
var $sudo = 1;

//var $configFile = "/etc/gammurc";
/* Locate gammurc files (fullpath) */

   function gammu($ex="") {
           if ($ex!="") { $this->gammu = $ex; }
   }

   function gammu_exe($exe,&$respon) {
           $respon=array();
           if (file_exists($this->gammu)) {
               if ($this->sudo == 1) {
               @exec("sudo ".$exe,$respon);
               } else { @exec($exe,$respon); }
           }
           else { array_push($respon,"Can not found $this->gammu or Gammu is not installed"); }
           //print_r($respon);
   }

   function Send($number=0,$text,$validity=0,$report=0,$reply=0,&$respon) {
           $option="";
          // if ($report == 1) { $option.="-report "; }
          // if ($validity != 0) { $option.="-validity $validity "; }
          // if ($reply == 1) { $option.="-reply "; }
           $text=str_replace($this->replacemen," ",$text);
           if ($number) {
               if ($this->sudo == 1) { exec("echo $text | sudo ".$this->gammu." nothing --sendsms TEXT $number $option -autolen ".strlen($text)."",$rs); }
               else { $this->gammu_exe("echo $text | ".$this->gammu." nothing --sendsms TEXT $number $option -autolen ".strlen($text)."",$rs); }
           }
           for($i=0;$i<count($rs);$i++) {
                   $respon.=$rs[$i]."\r\n";
           }
           $this->Error=$respon;
           if (eregi("OK",$respon)) { return 1; } else { $this->Error=$respon; return 0; }
   }

   function Identify(&$MANUFACTURER,&$MODEL,&$IMEI,&$SIM_IMSI,&$NetworkName,&$NetworkLevel,&$BatteryLevel,&$respon) {
        $x=0;
                $this->gammu_exe($this->gammu." nothing --identify",$rs);
                for($i=0;$i<count($rs);$i++) {
                        $r.=$rs[$i]."\r\n";
                }
                $this->gammu_exe($this->gammu." nothing --monitor 1",$rs);
                for($i=0;$i<count($rs);$i++) {
                        $r.=$rs[$i]."\r\n";
                }
                if(eregi("Error opening device",$r)) {
                        $respon="Not Connected ($r) [Error opening device]";
                        return 0;
                }
               if (eregi("No configuration file found",$r)) {
                        $respon="No configuration file found";
                        return 0;

                }
                if (eregi("Gammu is not installed",$r)) {
                        $respon=$r;
                        return 0;
                }
                if (preg_match("/Manufacturer(.+):(.+)/",$r,$s)) {
                        $MANUFACTURER=trim(str_replace("\n","",$s[2]));
                        $respon="";
                        $x++;
                }
                if (preg_match("/Model(.+):(.+)/",$r,$s)) {
                        $MODEL=trim(str_replace("\n","",$s[2]));
                        $respon="";
                        $x++;
                }
                if (preg_match("/IMEI(.+):(.+)/",$r,$s)) {
                        $IMEI=trim(str_replace("\n","",$s[2]));
                        $respon="";
                        $x++;
                }
                if (preg_match("/SIM IMSI(.+):(.+)/",$r,$s)) {
                        $SIM_IMSI=trim(str_replace("\n","",$s[2]));
                        $respon="";
                        $x++;
                }
                if (preg_match("/Battery level(.+):(.+)/",$r,$s)) {
                        $BatteryLevel=trim(str_replace("\n","",$s[2]));
                        $respon="";
                        $x++;
                }
                if (preg_match("/Network level(.+):(.+)/",$r,$s)) {
                        $NetworkLevel=trim(str_replace("\n","",$s[2]));
                        $respon="";
                        $x++;
                }
                if (preg_match("/Network  (.+):(.+)/",$r,$s)) {
                        $NetworkName=trim(str_replace("\n","",$s[2]));
                        $respon="";
                        $x++;
                }

                if ($x > 0) { return 1; }
   }

   function Del($d,&$respon) {
              $this->gammu_exe($this->gammu." nothing --deleteallsms $d",$rs);
              for($i=0;$i<count($rs);$i++) {
                        $respon.=$rs[$i]."\r\n";
              }

   }

   function Get() {
           //Checking SMS INBOX Folder
            $this->dataBuffer=false;
           //$this->gammu_exe($this->gammu." nothing --getsmsfolders",$rs);
           for($i=0;$i<count($rs);$i++) {
                   if (preg_match("/^(.). \"(.+)Inbox\", phone\s*/",$rs[$i],$s)) {
                           $this->Inbox_PHONE=trim(str_replace("\n","",$s[1]));
                   }
                   else if (preg_match("/^(.). \"(.+)Inbox\", SIM\s*/",$rs[$i],$s)) {
                           $this->Inbox_SIM=trim(str_replace("\n","",$s[1]));
                   }
                   else if (preg_match("/^(.). \"(.+)Outbox\", phone\s*/",$rs[$i],$s)) {
                           $this->Outbox_PHONE=trim(str_replace("\n","",$s[1]));
                   }
                   else if (preg_match("/^(.). \"(.+)Outbox\", SIM\s*/",$rs[$i],$s)) {
                           $this->Outbox_SIM=trim(str_replace("\n","",$s[1]));
                   }
                   else if (eregi("Error opening device",$rs[$i])) {
                           $respon="Error opening device";
                           return $respon;
                   }
           }

           $trimData=array(" ","\n","\r","\t","\"","'");
           $this->gammu_exe($this->gammu." nothing --geteachsms",$rs);
           $x=0;$y=0;
           for ($i=0;$i<count($rs);$i++) {
           //echo $rs[$i];
                   //Initializing Data
                   if (eregi("^SMS message",$rs[$i])) continue;
                   if (preg_match("/Location (.+), folder \"(.+)\"/",$rs[$i],$d)) {
                           $folder=$d[2];
                           if ($folder == "Outbox") {
                                   $fid=$x;
                                   $x++;
                           }
                           if ($folder == "Inbox") {
                                   $fid=$y;
                                   $y++;
                           }
                           //$fid=$x;
                           //$x++;
                           $data[$folder][$fid]=array();
                           $data[$folder][$fid]['location']=trim($d[1]);
                   }

                   //Inbox SMS Center Buffer
                   else if (preg_match("/^SMSC number(.+): \"(.+)\"/",$rs[$i],$d)) {
                           $data[$folder][$fid]['SMSC']=trim($d[2]);
                   }
                   //Outbox Ref Number Buffer
                   else if (preg_match("/^Reference number(.+): (.+)/",$rs[$i],$d)) {
                           $data[$folder][$fid]['REFNUM']=trim($d[2]);
                   }
                   //Inbox SentTime Buffer
                   else if (preg_match("/^Sent (.+): (.+)/",$rs[$i],$d)) {
                           $u=strtotime(trim($d[2]));
                           if (!$u or $u <=0) { $u=date("U"); }
                           $data[$folder][$fid]['SentTime']=date("Y-m-d H:i:s",$u);
                   }
                   //Coding Buffer
                   else if (preg_match("/^Coding (.+): (.+)\s*/",$rs[$i],$d)) {
                           $data[$folder][$fid]['Coding']=trim($d[2]);
                   }
                   //Phone Number Buffer
                   else if (preg_match("/^Remote number(.+): \"(.+)\"\s*/",$rs[$i],$d)) {
                           $number=trim($d[2]);
                           if (substr($number,0,1) == "0") {
                                   $number=substr($number,1,strlen($number));
                                   $number="+62".$number;
                           }
                           $data[$folder][$fid]['Number']=$number;
                   }
                   //Status Buffer
                   else if (preg_match("/^Status (.+): (.+)\s*/",$rs[$i],$d)) {
                           $data[$folder][$fid]['Status']=trim($d[2]);
                   }
                   //Buffer linked
                   else if (preg_match("/(.+)Concatenated \(linked\) message, ID \((.+)\) (.+), part (.+) of (.+)/",$rs[$i],$d)) {
                           $data[$folder][$fid]['link']['coding']=trim($d[2]);
                           $data[$folder][$fid]['link']['id']=trim($d[3]);
                           $data[$folder][$fid]['link']['part']=trim($d[4]);
                   }
                   //Buffer Other Information
                   else if (preg_match("/^(.+) : (.+)\s*/",$rs[$i],$d)) {
                           $data[$folder][$fid][trim(str_replace($trimData,"",$d[1]))]=trim($d[2]);
                   }
                   //Buffer Other Body
                   else {
                           //By Pass last line or info for Gammu UNIX/LINUX version
                           if (preg_match("/(.+) SMS parts in (.+) SMS sequences/",$rs[$i],$xxx)) continue;
                           //Buffer BODY
                           if (trim($rs[$i])) { $data[$folder][$fid]['body'].=htmlspecialchars(trim(addslashes($rs[$i]))); }
                   }
                   $data[$folder][$fid]['ID']=md5($data[$folder][$fid]['body'].$data[$folder][$fid]['Number'].$data[$folder][$fid]['SentTime'].$data[$folder][$fid]['location']);
           }
           //$this->dataBuffer=$data;
           return $data;
           //return 1;
           //asort($this->dataBuffer);
   }

   function setFile($filedb) {
           $this->filedb=$filedb;
   }
   function setEncryptText($t) {
           $this->TextEncrypt=$t;
   }

   function SaveTXT(&$respon) {
           if ($this->dataBuffer) {

           $write="";$writeDump="";
           if (!file_exists($this->filedb)) {
                $fp=@fopen($this->filedb,"w+") or $respon="Can not create file $this->filedb";
           }
           $fp=@fopen($this->filedb,"w+");
           //Save Inbox
           for ($i=0;$i<count($this->dataBuffer['Inbox']);$i++) {
                  $write.="INBOX\t{$this->dataBuffer['Inbox'][$i]['ID']}\t{$this->dataBuffer['Inbox'][$i]['Number']}\t{$this->dataBuffer['Inbox'][$i]['SentTime']}\t{$this->dataBuffer['Inbox'][$i]['SMSC']}\t{$this->dataBuffer['Inbox'][$i]['body']}\r\n";
                  $writeDump.="$this->dataBuffer['Inbox'][$i]['ID']";
           }
           //Save Outbox
           for ($i=0;$i<count($this->dataBuffer['Outbox']);$i++) {
                  $write.="OUTBOX\t{$this->dataBuffer['Outbox'][$i]['ID']}\t{$this->dataBuffer['Outbox'][$i]['Number']}\t{$this->dataBuffer['Outbox'][$i]['body']}\r\n";
                  $writeDump.="$this->dataBuffer['Outbox'][$i]['ID']";
           }
           $dump=strtoupper(md5($write).md5($write).md5($write));
           if ($this->TextEncrypt == 1) {
               fwrite($fp,"e=1".base64_encode(base64_encode($writeDump)).$dump.md5($dump).strrev($dump)."x4nG3l1C4x".strrev(base64_encode($write))."x4nG3l1C4x".$dump.base64_encode($writeDump.strrev($writeDump).strrev($dump)));
           }
           else { fwrite($fp,"e=0\r\n".$write); }
           @fclose($fp);
           $respon="SMS SAVED [".count($this->dataBuffer['Inbox'])." Inbox] [".count($this->dataBuffer['Outbox'])." Outbox]";

           }
           else { $respon="NO DATA SAVED"; }

   }

   function ReadTXT(&$respon) {
           $x=0;$data=array();$dataDump=array();
           $w=0;
           $z=0;
           if (file_exists($this->filedb)) {
                   $fp=@fopen($this->filedb,"r");
                   $ck=fread($fp,3);
                   if ($ck == "e=0") {
                           while($fg=@fgets($fp,10240)) {
                                 $x++;
                                 if ($x==1) continue;
                                 $dataDump[$x]=explode("\t",trim($fg));
                                 if (!$dataDump[$x][3]) continue;
                                 if ($dataDump[$x][0] == "INBOX") {
                                         $data['Inbox'][$w]['ID']=$dataDump[$x][1];
                                         $data['Inbox'][$w]['Number']=$dataDump[$x][2];
                                         $data['Inbox'][$w]['SentTime']=$dataDump[$x][3];
                                         $data['Inbox'][$w]['SMSC']=$dataDump[$x][4];
                                         $data['Inbox'][$w]['body']=$dataDump[$x][5];
                                         $w++;
                                 }
                                 if ($dataDump[$x][0] == "OUTBOX") {
                                         $data['Outbox'][$z]['ID']=$dataDump[$x][1];
                                         $data['Outbox'][$z]['Number']=$dataDump[$x][2];
                                         $data['Outbox'][$z]['body']=$dataDump[$x][3];
                                         $z++;
                                 }
                                 //$data=$dataDump[];
                           }
                   }
                   @fclose($fp);
                   $this->dataBuffer=$data;
                   if (!$this->dataBuffer) { $respon="NO DATA FOUND"; return 0; }
                   else { return 1; }
           }
           else { $respon="Can not found $this->filedb"; return 0; }
   }

   function DeleteTXT($id,$box='Inbox') {
          if ($this->dataBuffer[$box][$id]) {
              $this->dataBuffer[$box][$id]=NULL;
              unset($this->dataBuffer[$box][$id]);
          }
          $this->SaveTXT($respon);
          $this->ReadTXT($respon);
   }

   function SaveSQL(&$respon) {
   }

   function setDate() {
           $this->gammu_exe($this->gammu." --setdatetime",$respon);
           $data=$respon[0];
           $this->gammu_exe($this->gammu." --getdatetime",$respon);
           $data.="\r\n".$respon[0];
           return $data;
   }

   function getContact($me="SM",&$respon) {
         if ($this->Version() < 1.10) {
                 $respon="Invalid Gammu Version, require version 1.10+<br>Your version $this->gammu_version";
             return false;
         }
         $this->gammu_exe($this->gammu." nothing --getallmemory $me",$rs);
         $sx=0;
         for($i=0;$i<count($rs);$i++) {
                 $rs[$i]=trim($rs[$i]);
                 //if (!$rs[$i]) continue;
                 if (preg_match("/^Memory (.+), Location (.+)/",$rs[$i],$d)) {
                         $eml=0;
                         $x=$sx;
                         if (!trim($d[1])) continue;
                         $data[$x]['Location']=$d[2];
                         $data[$x]['MEM']=$d[1];
                         //echo $d[1]." - ".$d[2]."<br>";
                         $sx++;
                 }
                 if (preg_match("/^Email address (.+): \"(.+)\"/",$rs[$i],$d)) {
                         $data[$x]['email'][$eml]=htmlentities(addslashes($d[2]));
                         $eml++;
                 }
                 else if (preg_match("/^Name(.+): \"(.+)\/(.+)\"\"/",$rs[$i],$d)) {
                         $data[$x]['FirstName']=htmlentities(addslashes($d[2]));
                 }
                 else if (preg_match("/^Name(.+): \"(.+)\"\"/",$rs[$i],$d)) {
                         $data[$x]['FirstName']=htmlentities(addslashes($d[2]));
                 }
                 else if (preg_match("/^Name(.+): \"(.+)\"/",$rs[$i],$d)) {
                         $data[$x]['FirstName']=htmlentities(addslashes($d[2]));
                 }
                 else if (preg_match("/^Last name(.+): \"(.+)\"/",$rs[$i],$d)) {
                         $data[$x]['LastName']=htmlentities(addslashes($d[2]));
                 }
                 else if (preg_match("/^First name(.+): \"(.+)\/\"\"/",$rs[$i],$d)) {
                         $data[$x]['FirstName']=htmlentities(addslashes($d[2]));
                 }
                 else if (preg_match("/^First name(.+): \"(.+)\"/",$rs[$i],$d)) {
                         $data[$x]['FirstName']=htmlentities(addslashes($d[2]));
                 }
                 else if (preg_match("/^Job title(.+): \"(.+)\"/",$rs[$i],$d)) {
                         $data[$x]['JobTitle']=htmlentities(addslashes($d[2]));
                 }
                 else if (preg_match("/^Company(.+): \"(.+)\"/",$rs[$i],$d)) {
                         $data[$x]['Company']=htmlentities(addslashes($d[2]));
                 }
                 else if (preg_match("/^Home number(.+): \"(.+)\"/",$rs[$i],$d)) {
                         $data[$x]['HomeNumber']=htmlentities(addslashes($d[2]));
                 }
                 else if (preg_match("/^Work number(.+): \"(.+)\"/",$rs[$i],$d)) {
                         $data[$x]['WorkNumber']=htmlentities(addslashes($d[2]));
                 }
                 else if (preg_match("/^Mobile number(.+): \"(.+)\"/",$rs[$i],$d)) {
                         $data[$x]['MobileNumber']=htmlentities(addslashes($d[2]));
                 }
                 else if (preg_match("/^Fax number(.+): \"(.+)\"/",$rs[$i],$d)) {
                         $data[$x]['FaxNumber']=htmlentities(addslashes($d[2]));
                 }
                 else if (preg_match("/^General number(.+): \"(.+)\"/",$rs[$i],$d)) {
                         $data[$x]['GeneralNumber']=htmlentities(addslashes($d[2]));
                 }
                 else if (preg_match("/^URL address(.+): \"(.+)\"/",$rs[$i],$d)) {
                         $data[$x]['URL']=htmlentities(addslashes($d[2]));
                 }
                 else if (preg_match("/^Street address(.+): \"(.+)\"/",$rs[$i],$d)) {
                         $data[$x]['StreetAddress']=htmlentities(addslashes($d[2]));
                 }
                 else if (preg_match("/^City(.+): \"(.+)\"/",$rs[$i],$d)) {
                         $data[$x]['City']=htmlentities(addslashes($d[2]));
                 }
                 else if (preg_match("/^State(.+): \"(.+)\"/",$rs[$i],$d)) {
                         $data[$x]['State']=htmlentities(addslashes($d[2]));
                 }
                 else if (preg_match("/^Zip code(.+): \"(.+)\"/",$rs[$i],$d)) {
                         $data[$x]['ZipCode']=htmlentities(addslashes($d[2]));
                 }
                 else if (preg_match("/^Text(.+): \"(.+)\"/",$rs[$i],$d)) {
                         $data[$x]['Text']=htmlentities(addslashes($d[2]));
                 }
                 else if (preg_match("/^LUID(.+): \"(.+)\"/",$rs[$i],$d)) {
                         $data[$x]['LUID']=htmlentities(addslashes($d[2]));
                 }
                 else if (preg_match("/^Date and time(.+): (.+)/",$rs[$i],$d)) {
                         $data[$x]['TimeInfo']=htmlentities(addslashes($d[2]));
                 }

                 /*
                 else if (preg_match("/^(.+):(.+)/",$rs[$i],$d)) {
                         $data[$x][trim($d[1])]=$d[2];
                 }
                 else { $data[$x]['Buffer']=$rs[$i]; }
                 */
         }
         return $data;
   }

   function Version() {
            $this->gammu_exe($this->gammu." nothing --version",$respon);
            preg_match("/^\[Gammu version (.+) built (.+)\]/",$respon[0],$gm);
            list($My,$Mi,$Mr)=explode(".",$gm[1]);
            $this->gammu_version = "$My.$Mi";
            return $this->gammu_version;
   }

   function enable_sudo($s=1) {
           $this->sudo=$s;
   }
}


?>