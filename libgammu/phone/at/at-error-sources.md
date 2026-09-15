# AT error sources and decoding policy

`at-error.c` keeps CME (mobile equipment) and CMS (SMS) codes separate.
Each entry specifies both a diagnostic description and a Gammu error result.
Numeric modem codes are preserved in SMS callbacks and SMSD status fields.

## Lookup and compatibility

The identified manufacturer's table takes precedence over the standard table for
that error family. An unidentified manufacturer gets only standard mappings.
Unlisted codes retain their numeric value, have no description, and return
`ERR_UNKNOWN`. No manufacturer is inferred from an error number or chipset.

These are **vendor defaults**, not guarantees for every model or firmware. This
is deliberate: a useful documented vendor default is preferable here to requiring
model-specific identification. The reference families below settle conflicts.
Do not merge Wavecom and Sierra Wireless tables because of the acquisition.

Descriptions and classifications come from the same entry. Documented busy and
initialization states return `ERR_BUSY`; network failures return
`ERR_NETWORK_ERROR`; explicit cancellation, invalid input, security, SIM, memory,
and unsupported-operation errors use the corresponding existing Gammu errors.
Ambiguous failures remain `ERR_UNKNOWN`. An `ERROR` response never becomes
`ERR_NONE`, even for SIMCom's descriptions "No Error" and "SMS no error", or
u-blox's acknowledgement/status descriptions.

Existing standard-code classifications are retained. Newer standard codes found
in individual vendor manuals are included in those vendors' tables without
claiming that their definitions apply to all devices. This change does not alter
SMSD retry scheduling or introduce new public error values.

Numeric CMS zero retains Gammu's legacy `ERR_PHONE_INTERNAL` classification,
with the diagnostic label "Unspecified SMS failure". This is a compatibility
mapping, not a new interpretation from a vendor manual. CME zero remains a
separate entry returning `ERR_UNKNOWN`.

Verbose errors use complete case-insensitive matches with surrounding whitespace
ignored. The vendor table is searched before the standard table; within a table,
the lowest numeric code wins when descriptions repeat. For example, Telit uses
"wrong state" for both CME 551 and 601: numeric replies preserve either code,
while the verbose reply resolves to 551. A standard description cannot select a
code overridden by the vendor. Unknown or malformed verbose replies have no
inferred code. Malformed numeric replies, overflow, and trailing garbage are
rejected; the internal validity flag distinguishes these from valid zero and
Samsung's valid -1. The callback fallback for a missing code is -1.

## Reference inventory

Local paths are relative to the sibling `gsm-docs` repository. They are research
references, not build dependencies. Online links point to manufacturer-authored
manuals, sometimes hosted by distributors or archives.

| Manufacturer/default family | Source | Imported scope |
| --- | --- | --- |
| Siemens TC65 | `vendors/siemens/TC65_atc_v02000.pdf`, V02.000, June 2, 2006, section 2.12, tables 2.4–2.8, pp. 50–54 | CME 256–263 and documented extended CME codes; CMS 512–554 |
| Motorola g20 | `vendors/motorola/g20 AT Commands 9808901C68.pdf`, 98-08901C68-O, table 95, pp. 196–198 | Extended CME entries and CMS 512–519 |
| Huawei UMTS M2M | `vendors/huawei/huawei-inetrmax-cmd.pdf`, V100R001 issue 18, June 22, 2009, section 21.2 | CME 257–264 and 65280–65287 |
| Falcom SAMBA75 | `vendors/falcom/Samba75_at_command_set.pdf`, version 1.00, section 1.12.1, tables 1.2–1.5 | Extended CME entries explicitly listed by Falcom |
| iTegno 38XX / iWOW | `vendors/itegno/iTegno 38XX ATC Guide_v1.0.pdf`, 02000C13 v1.0, sections 19.1 and 19.2.2 | CME 512–514; CMS 512–513 |
| Wavecom legacy GSM | [AT commands interface](https://ozeki.hu/attachments/588/wavecom_command.pdf), WCOM/GSM/SW/SII015 version 8.1, September 25, 1999, section 16.3 | CMS 512–515; CME 515–516 |
| Quectel EC25/EC21 | [AT Commands Manual](https://quectel.com/content/uploads/2021/03/Quectel_EC25EC21_AT_Commands_Manual_V1.3.pdf), v1.3, sections 21.5–21.6 | CME 901–904; listed CMS extensions 512–531 |
| SIMCom SIM800 | [AT Command Manual](https://cdn-shop.adafruit.com/datasheets/sim800_series_at_command_manual_v1.01.pdf), v1.01, July 23, 2013, sections 19.1–19.2 | Additional CME and CMS entries from the error summaries |
| Telit ME310G1/ME910G1/ML865G1 | [AT Commands Reference Guide](https://device.report/m/890b48749c7ff99b9494a55e688863b69e7ce18a96b783a086dc08d3daa62570.pdf), 80617ST10991A rev. 9, February 17, 2021, sections 3.2.2.1–3.2.2.2 | Additional CME entries; CMS 510 and 512–515 |
| Sierra Wireless HL78xx | [AT Commands Interface Guide](https://forum.sierrawireless.com/uploads/short-url/6SoafMAgwLaVshhVzM8rq14ky3.pdf), 41111821 rev. 11, April 17, 2020, sections 14.3.1 and 14.3.3 | Additional CME entries; CMS 606 |
| u-blox LENA-R8 | [AT commands manual](https://content.u-blox.com/sites/default/files/documents/LENA-R8_ATCommands_UBX-22016905.pdf), UBX-22016905 R04, appendices A.1–A.2 | Additional CME and CMS entries; excludes A.3 onward, which use other namespaces |
| Samsung | Existing `ATGEN_DispatchMessage` and `ATGEN_HandleCMEError` workaround | CME -1: empty phonebook location; now strictly Samsung-only |
| Ericsson / Sony Ericsson | Existing `ATGEN_HandleCMEError` P1i phonebook workaround | CME 601: unsupported operation; now strictly Ericsson-only |

## Conflicts and audit notes

- Siemens M20 documents CMS 512 as "SIM not ready", whereas TC65, MC35i,
  MC45, and several phone manuals use "User abort". TC65 is the default.
  Siemens 256–263 and the extended GPRS entries were incorrectly present in
  Gammu's shared CMS table; the TC65 manual places them in CME.
- Motorola g20 and C18 agree on CMS 512–517. The old speculative global
  description for 516 ("too high location?") is replaced with the documented
  preferred-memory-storage error, scoped to Motorola.
- iTegno CME 513 means lower-layer SMS failure, while CMS 513 means ACM reset
  needed. Sharing the two error families would introduce a new incorrect mapping.
- Wavecom CMS 512 means MM establishment failure. It does not establish the
  exact network failure cause and must not be used to diagnose Jio compatibility
  automatically. The legacy manual is the Wavecom default, not an assertion about
  all newer Wavecom/Sierra firmware.
- Quectel and SIMCom share several CMS values but disagree on 512. Keep their
  tables independent. SIMCom HTTP/FTP result codes and other command-specific
  notification namespaces are not imported into CME/CMS; entries explicitly in
  its CME/CMS summaries are included.
- u-blox A.2 lists two meanings for CMS 543 and 548. The first printed meanings
  (SMS CONTROL rejection and undefined result) are used. Neither an
  acknowledgement description nor "RP-Error OK" is treated as successful SMS
  submission. Wrapped descriptions are joined before use.
- Falcom's error tables are similar to Siemens but not identical; only the entries
  actually listed by Falcom are included.
- Sony Ericsson `vendors/sony-ericsson/dg_at_2006--09_r14a.pdf`, R14a,
  October 2009, pp. 11–13, lists standard errors and marks CMS 512 as
  manufacturer-specific without defining it. It supplies no additional proprietary
  mapping. CME 601 remains a compatibility workaround, not a claim from this manual.
- Nokia `vendors/nokia/AT_Command_Set_For_Nokia_GSM_And_WCDMA_Products_v1_2_en.pdf`,
  sections 15.1–15.2, and Bosch `vendors/bosch/boschat.pdf`, sections 3.52–3.53,
  provide no additional proprietary CME/CMS mappings used here.
- The local Alcatel material is primarily binary protocol/service documentation;
  no verified additional AT CME/CMS mapping was identified. The Sagem directory
  contains ringtone documentation, not an AT error reference. No meanings are
  invented for these vendors or for other unidentified manufacturers.

## Extending the tables

Add the manufacturer identity and detection only when a documented mapping needs
it. Record the exact manual and error-family scope here. Keep tables sorted and
numeric codes unique; resolve conflicting meanings explicitly. Add response-path
regressions for the new vendor, including a collision with another vendor or
error family. Preserve standard classifications unless a separate, justified
change updates them.

Validation uses synthetic AT replies through the protocol state machine, including
SMS callbacks and manufacturer identification. It does not require a modem and
does not claim hardware coverage for every listed model.
