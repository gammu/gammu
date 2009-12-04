-- Poll trigger

-- It check sformat of message, stores vote and injects reply message
-- to smsd database

-- Written by Okta <okta.nc@gmail.com>

delimiter $
create trigger
smsd.sms
AFTER
INSERT
on
smsd.inbox
for each row BEGIN
DECLARE smsx,smsz,smsin varchar(200);
DECLARE format,smsvote,sender_no varchar(200);
DECLARE error,x,totalvote int;

set smsz=new.TextDecoded;
select ltrim(smsz) into smsx;
select rtrim(smsx) into smsin;
set sender_no=new.SenderNumber;
select substring_index(smsin,' ',1) into format;
select substring(smsin,6) into smsvote;
select count(candidate) from polling.polling_data
where candidate=smsvote into x;
select vote from polling.polling_data
where candidate=smsvote into totalvote;
set totalvote=totalvote+1;
set error=0;

if format='vote' and x>0 then
update polling.polling_data set vote=totalvote where candidate=smsvote;
else
set error=1;
end if;

if error=1 then
insert into smsd.outbox (DestinationNumber,TextDecoded,CreatorID,Coding)
VALUES (sender_no,'Sorry the format that you entered was incorrect or the
candidate is not available. Please resend in the following format: [VOTE
CandidateName]','Voter','Default_No_Compression');
end if;

if error=0 then
insert into smsd.outbox (DestinationNumber,TextDecoded,CreatorID,Coding)
VALUES (sender_no,'Thank you. Your vote has been
accepted','Voter','Default_No_Compression');
end if;

END;
$
