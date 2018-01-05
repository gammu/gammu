/**
 * Null SMSD service.
 *
 * This service does not store anything anywhere.
 */
/* Copyright (c) 2010 - 2018 Michal Cihar <michal@cihar.com>. */

#include "../core.h"

GSM_SMSDService SMSDNull = {
	NONEFUNCTION,		/* Init                 */
	NONEFUNCTION,		/* Free                 */
	NONEFUNCTION,		/* InitAfterConnect     */
	NONEFUNCTION,		/* SaveInboxSMS         */
	EMPTYFUNCTION,		/* FindOutboxSMS        */
	NONEFUNCTION,		/* MoveSMS              */
	NONEFUNCTION,		/* CreateOutboxSMS      */
	NONEFUNCTION,		/* AddSentSMSInfo       */
	NOTIMPLEMENTED,		/* UpdateRetries        */
	NOTIMPLEMENTED,		/* RefreshSendStatus    */
	NOTIMPLEMENTED,		/* RefreshPhoneStatus   */
	NONEFUNCTION		/* ReadConfiguration    */
};

/* How should editor handle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
