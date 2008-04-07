<?php

$_HEADER = array();
while($data = fgets(STDIN, 4096)) {
	$data = trim($data);
	if($data == "") {
		/* empty line means from next line we shall get the sms body */
		break;
	}
	$matches = array();
	if(preg_match("/^(.*)\:(.*)/", $data, $matches)) {
		$_HEADER[$matches[1]] = $matches[2];
	}
}
$_REQUEST = "";
while($data = fgets(STDIN, 4096)) {
	$_REQUEST .= $data;
}

/**
 * SO, 
 * $_REQUEST      is the sms data.
 * $_HEADER       is the header array. For example $_HEADER["SMS_FROM"] is the sender number.
 */

unset($data);
unset($matches);

<?php

$_HEADER = array();
while($data = fgets(STDIN, 4096)) {
	$data = trim($data);
	if($data == "") {
		/* empty line means from next line we shall get the sms body */
		break;
	}
	$matches = array();
	if(preg_match("/^(.*)\:(.*)/", $data, $matches)) {
		$_HEADER[$matches[1]] = $matches[2];
	}
}
$_REQUEST = "";
while($data = fgets(STDIN, 4096)) {
	$_REQUEST .= $data;
}

/**
 * SO, 
 * $_REQUEST      is the sms data.
 * $_HEADER       is the header array. For example $_HEADER["SMS_FROM"] is the sender number.
 */

unset($data);
unset($matches);

