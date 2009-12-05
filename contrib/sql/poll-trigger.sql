-- Poll trigger

-- It checks format of message, stores vote and injects reply message
-- to smsd database

-- Written by Okta <okta.nc@gmail.com>
-- Updated by Peter Stuge <peter@stuge.se> to remove a race condition

delimiter $

create trigger smsd.inbox_ai after insert on smsd.inbox for each row
begin
  declare msg,smsvote char(200);
  declare found_candidate tinyint unsigned default 0;
  declare reply char(160) default 'Sorry, the format that you entered was incorrect. Please resend in the following format: [VOTE CandidateName]';

  set msg=trim(new.TextDecoded);
  set msg=trim(trim(leading '[' from trim(trailing ']' from msg)));

  if left(msg,5)='vote ' then
    set smsvote=trim(substring(msg,6));
    select count(candidate)>0 from polling.polling_data where candidate=smsvote into found_candidate;
    if found_candidate=0 then
      set reply='Sorry, the candidate that you entered was not found. Please resend in the following format: [VOTE CandidateName]';
    else
      update polling.polling_data set vote=vote+1 where candidate=smsvote;
      set reply='Thank you. Your vote has been accepted';
    end if;
  end if;

  insert into smsd.outbox (DestinationNumber,TextDecoded,CreatorID,Coding) values (new.SenderNumber,reply,'Voter','Default_No_Compression');
end; $

delimiter ;
