<?
class gammu {

	
	
	
	function checknew(){
		global $mysql;
		$new = $mysql->count("SELECT ID FROM inbox, proclast WHERE ID>proclast.lastinbox");
		if($new > 0){
			return $new;
		}else{
			return false;
		}
	}
	
	
	
	function enviarsms($sms, $destinatario="", $validade="255", $flash="-1", $deferidodata ="", $phone=""){
		global $mysql, $adminsphone;
		if ($destinatario == ""){
			$destinatario = $adminsphone;
		}
		
		require_once('config.php');
		//$sms = limpar($sms, "soft");
		if(strlen($sms) > 160){
			return false;
		}elseif($deferidodata == ""){
			$datoom = date("YmdHis");	
		}else{
			$datoom = $deferidodata;
		}
		if ($mysql->sql("INSERT INTO outbox(Class,DestinationNumber,TextDecoded,SendingDateTime,RelativeValidity,SenderID,DeliveryReport) VALUES('$flash','$destinatario','$sms','$datoom','$validade','$phone','yes')")){
			return true;
		}else{
			return false;
		}
	}
	
	
	
	
	function processnew(){
		global $mysql, $triggers, $smsc, $adminsphone;
		if($this->checknew()){
			if($mysql->count("SELECT * FROM inbox, proclast WHERE ID>proclast.lastinbox ORDER BY ID ASC LIMIT 1") == 1){
				$res = $mysql->farray("SELECT * FROM inbox, proclast WHERE ID>proclast.lastinbox ORDER BY ID ASC LIMIT 1");
				$res['TextDecoded'] = addslashes(strip_tags($res['TextDecoded']));
				$res['TextDecoded'] = explode(" ",$res['TextDecoded']);
				$res['TextDecoded'][0] = strtolower($res['TextDecoded'][0]);
				if(isset($triggers[$res['TextDecoded'][0]])) {
					include($triggers[$res['TextDecoded'][0]]);
				}
				$mysql->sql("UPDATE proclast SET lastinbox='{$res['ID']}'");
			}
		}
		if($this->checknew()){
			$this->processnew();
		}
	}
	
}
$gammu = new gammu;
##/usr/local/gammu/bin/gammu --dialvoice 1799
?>