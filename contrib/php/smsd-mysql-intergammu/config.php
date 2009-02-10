<?
#############################################################
## API gammu / mysql / php by Hugo Pinto ( hugo@bragatel.pt )
## NAME: InterGammu
## Description: Interact with your server thru gsm
##
## i.e. 1 : send a sms to your gammu connected server's mobile phone,
##          with the text "ADMIN IP",
##          and you'll receive a sms with the server's current IP.
## i.e. 2 : send a sms to your gammu connected server's mobile phone,
##          with the text "ADMIN UPTIME",
##          and you'll receive a sms with the server's uptime detail.
##feel free to add more commands
##
## For any qustions and if you have any sugestions you can mail me at hugo@bragatel.pt
## please feel free to send me by mail any more commands written by you!!!
#############################################################


##Settings for MySql
$mysql_host = "localhost";
$mysql_user = "sms";
$mysql_pass = "gammu";
$mysql_db = "sms";


##Allowed SMSC's
$smsc = array('vodafone' => "+351911616161",		/* Número do centro de mensagens Vodafone (ex.: "+351911616161")*/
			  'tmn' => "+351962100000",				/* Número do centro de mensagens TMN*/
			  'optimus' => "+35193121314" 			/* Número do centro de mensagens Optimus*/
		);

##adminsphone : some procedures will only be allowed if requested by these phone number:
$adminsphone = "+351917520623";
##procedures : this array contains all the triggers and reference to the related script.
## i.e.: If the SMS starts with "Admin", it wil run the "proc/admin.php" script...
## check out the proc/admin.php
$triggers = array(
	"admin"=>"proc/admin.php",
	"sample2"=>"proc/sample2.php"
);

##includes
include('funcoes/func.sql.php');
include('funcoes/func.gammu.php');

/*
for this to work properly, it must be added this table to de SMS database:
###########################################################################
--
-- Table structure for table `proclast`
--

CREATE TABLE `proclast` (
  `lastinbox` int(11) unsigned NOT NULL default '0',
  KEY `lastinbox` (`lastinbox`)
)

TYPE=MyISAM;

--
-- Dumping data for table `proclast`
--

INSERT INTO `proclast` VALUES (1);

##########################################################################
*/


?>
