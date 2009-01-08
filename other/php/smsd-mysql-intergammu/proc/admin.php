<?
if(!is_array($res)){
	echo("erro");
}else{
	if($res['SenderNumber'] == $adminsphone){
		if($res['SMSCNumber'] == $smsc['vodafone']){
			$res['TextDecoded'][1] = strtolower($res['TextDecoded'][1]);
			switch ($res['TextDecoded'][1]){

				case "ip":
					unset ($linha);
					exec("ifconfig eth0", $linha);
					$linha[1] = trim($linha[1]);
					$linha[1] = explode(" ",$linha[1]);
					if($linha[1][0] != "inet"){
						$sms = "Nenhum Ip definido."; // translation from portugues: no ip defined
					}else{
						$sms = "IP do server: ".$linha[1][1];
					}
					$this->enviarsms($sms);
					break;
					
				case "uptime":
					unset($linha);
					exec("uptime", $linha);
					$sms = "Uptime: ".$linha[0];
					$this->enviarsms($sms);
					break;
					
				##feel free to add some new commands here
			}
		}else{
			echo "log acesso invalido 2";
		}
	}else{
		echo "log acesso invalido";
	}
}
?>