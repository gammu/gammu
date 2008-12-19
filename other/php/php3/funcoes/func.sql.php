<?

/**********
 * Classe MySQL
 *********/

class mysql {

 var $conn;
 

 function mysql() {

 global $mysql_host, $mysql_user, $mysql_pass, $mysql_db;

	$this->conn = @mysql_connect("$mysql_host", "$mysql_user", "$mysql_pass");
	if ($this->conn<0) {
		return -1;
	}
	if (@mysql_select_db("$mysql_db", $this->conn)==false) {
		return -1;
	}
 }

 function fechar() {
	mysql_close($this->conn);
 }
 
 function count($query) {
	$result = mysql_num_rows(mysql_query($query, $this->conn));
	//$this->fechar();
	return $result;
 }


 function farray($query) {
 	$result = mysql_fetch_array(mysql_query($query, $this->conn));
    //$this->fechar();
    return $result;
 }

 function frow($query) {
	$result = mysql_fetch_row(mysql_query($query, $this->conn));
	//$this->fechar();
	return $result;
 }

function fmultiarray($query) {
	$this->mysql();
	$result = mysql_query($query);
	$count = mysql_num_rows($result);
	if($count == 0) {
		//$this->fechar();
		return false;
	}elseif($count == 1) {
		$resultado[0] = $this->farray($query);
		//$this->fechar();
		return $resultado;
	}elseif($count > 1) {
		$i = 0;
		//$this->mysql();
		//$result = mysql_query($query);
		while($fetch = mysql_fetch_array($result)) {
			$resultado[$i] = $fetch;
			$i++;
		}
		//$this->fechar();
		return $resultado;
	}else{
		//$this->fechar();
		return false;
	}
}



 function sql($sql) {
 	mysql_query("COMMIT",$this->conn);
	$result = mysql_query($sql, $this->conn);
	//$this->fechar();
	return $result;
 }


}

$mysql = new mysql;
//register_shutdown_function($mysql->fechar);
?>