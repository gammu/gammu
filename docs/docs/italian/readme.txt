Gammu README versione 02.agosto.2002
------------------------------------
DESCRIZIONE
Gammu e' un programma che permette di effettuare numerose operazioni con telefoni
cellulari Nokia e di altre marche, collegati al PC tramite cavo o porta a infrarossi:
invio/ricezione suonerie, loghi accensione, loghi operatore, messaggi con disegni ecc;
gestione SMS (backup, modifica, invio tramite computer); attivazione NetMonitor su
telefoni DC3 (vedere sito http://marcin-wiacek.topnet.pl per maggiori informazioni su
NetMonitor); backup e ripristino dei dati sul telefono, ecc.
Il programma e' distribuito con licensa GNU GPL (vedere file "copying").

VERSIONI DISPONIBILI
Gammu e' disponibile per vari sistemi operativi, tra cui anche Windows nelle sue
varie versioni, ed e' disponibile anche in formato sorgente, in modo da poter essere
compilato su qualunque sistema operativo.

L'AUTORE
Il programma e' stato creato inizialmente da Martin Wiacek, basato in parte sul programma
"Gnokii" (www.gnokii.org), ma ha ricevuto molti contribti da molte altre persone, essendo
un programma di tipo "open source", ossia modificabile, dietro autorizzazione dell'autore,
da chiunque sia in grado di apportarvi migliorie di qualunque genere.

CONTRIBUTI
Chiunque puo' fornire il suo contributo al progetto Gammu, volto a supportare il maggior
numero di telefoni e di funzioni possibile, per soddisfare le esigenze di tutti. E'
possibile aiutare in molti modi:
	1. Scrivendo codice che supporti altri modelli, o aggiustando il codice esistente,
	ove ce ne sia bisogno. Il formato e il metodo da usare per le aggiunte e' descritto
	nei documenti riservati agli sviluppatori (developers). A questo proposito, l'autore
	e' in possesso di specifiche per molti modelli (Motorola, Sagen, Siemens xx45), e
	basta implementare poche funzioni per avere il supporto di questi telefoni...
	2. Riferendo di malfunzionamenti del programma (vedere la sezione BUG REPORT di
	questo documento).
	3. Aiuto nella traduzione della documentazione.
	4. Aiuto per scoprire i protocolli dei telefoni.
	5. Suggerimenti per migliorare le cose :-)

DOVE TROVARE L'ULTIMA VERSIONE
L'ultima versione di Gammu puo' essere scaricata da http://marcin-wiacek.topnet.pl .
Ci sono due versioni: stabile e in fase di sviluppo. La seconda e' possibile che venga
aggiornata anche quotidianamente.

IN CASO DI PROBLEMI
In caso di problemi col programma, ci si puo' rivolgere all'autore (marcin-wiacek@topnet.pl),
oppure abbonarsi alla mailing list e cercare aiuto li'...

---------------------

Note tecniche

Domanda. 
   Come compilare il programma?

Risposta. 
   * cambiare i parametri nel file "config.h"
   * compilare:
	* utilizzando MS Visual C++ 6.0 per Windows:
	   portarsi nella directory "/gnokii" e avviare "gnokii.dsw"
	* sotto Unix o Linux:
	   selezionare il compilatore in "makefile.glo" (se necessario) e usare "make"
	* utilizzando Borland C++ 3.1 per MS-DOS:
	  ho provato senza successo :-( ; E' RICHIESTO AIUTO.
	* sotto DJGPP per MS-DOS:
	  usare semplicemente "make" :-)
	  (si e' compilato senza problemi in MS-DOS puro sotto Windows 98 SE; usando una
	  finestra DOS sotto Windows XP, ho riscontrato problemi di memoria).

	NOTA: in queste prime versioni non e' possibile utilizzare Gammu compilato
	 	usando DJGPP - il driver per la porta seriale non e' ancora pronto!
	* sotto CygWin per Windows:
	  Non l'ho provato, ma dovrebbe funzionare.
	* sotto Borland C++ Builder 5 trial:
	  portarsi nella directory "gammu/bcc" e controllare se e' presente la directory
	  "output" (semmai crearla), e poi avviare "gammu.bpr".
	  CONSIGLIO: "gammu.bpr" e' stato creato usando "Tools/Visual C++ Project Conve..",
			 e leggermente modificato.
	* sotto Borland C++ a linea di comando:
	  portarsi nella directory "gammu/bcc" e controllare se e' presente la directory
	  "output" (semmai crearla), poi digitare "make -fgammu.mak" dal prompt dei comandi.
	  CONSIGLIO: "gammu.mak" e' stato creato usando Borland C++ Builder usando l'opzione
	  "Project/Export makefiles" dal menu principale.

-------------------------------------------------------------------------------
D. Dove trovare i compilatori per compilare Gammu?
R.	* MS Visual C++ 6.0:
	   andate in un negozio di computer ;-) . E' un programma commerciale.
	* compilatori per Unix (Linux) come gcc o kgcc (gratis):
	  disponibili su Internet e nelle  distribuzioni Linux che si possono trovare su
	  molte riviste di informatica. L'installazione dipende dalla distribuzione.
	* DJGPP (gratis):
	  vedere http://www.midpec.com/djgpp/ (potete trovarci molte informazioni)
 	    oppure
	  1. Andate direttamente su http://www.simtel.net
	  2. cercate la sezione DJGPP
	  3. scaricate i pacchetti:
              v2/readme.1st       - Istruzioni per l'installazione di DJGPP
              v2/faq230b.zip      - Le Domande Frequenti (FAQ,in inglese) su  DJGPP 
              v2/djdev203.zip     - Il kit di sviluppo  DJGPP 
              v2misc/csdpmi4b.zip - Il server DPMI
              v2gnu/bnu210b.zip   - Le utility binarie GNU
              v2gnu/gcc2952b.zip  - Il compilatore GNU C
              v2gnu/mak279b.zip   - L'utility GNU Make 
            o piu' recenti.
	   4. decomprimere tutti i pacchetti nella stessa directory (ad esempio C:\DJGPP)
	   5. aggiungere all'autoexec.bat queste linee:
              PATH C:\DJGPP\BIN;%PATH%
              SET DJGPP=C:\DJGPP\DJGPP.ENV
	* Borland C++ Builder:
         Vedere www.borland.com (e' un programma commerciale). Versioni dimostrative
	   vengono talvolta allegate a CD in riviste di informatica.
	* Borland C++ compiler (gratis):
         Vedere www.borland.com. Talvolta si trova su CD in edicola.
-------------------------------------------------------------------------------
D. Quali sono i problemi conosciuti?

A  1. Nokia 3310 (3315):

      * i nomi della rubrica risultano vuoti.

	  Per evitarlo, bisogna avere un firmware successivo al 4.06 sul telefono.
	  Per notizie sull'aggiornamento del firmware del telefono, vedi in fondo a
	  questo file.
	* Non si possono salvare gli appuntamenti agenda, ma solo i Promemoria (dopo
	  aver aggiornato il firmware). Le versioni 5.11 e successive non sembrano essere
	  in grado di salare altro che i promemoria (addirittura, nelle vecchie versioni
	  questo fatto non era nemmeno ufficiale).

   2. Nokia 6210 (forse anche 6250/7110):

	* Salvando alcuni SMS come loghi o suonerie sul teelfono e poi rinviandoli dal menu
	  del telefono, essi vengono danneggiati.

	  Non e' un problema di Gammu. Un semplice test lo dimostra: salvate
	  un messaggio di questo tipo sulla SIM inserita in un vecchio Nokia, come un 6150. 
	  Inviatelo tramite il menu del 6150. E' a posto. Ora mettete la SIM in un 6210,
	  e inviate il messaggio del menu: ora e' rovinato!

      * Il telefono non avvisa di tutti gli SMS in arrivo.

	  Purtroppo, conosciamo solo il frame (*) che abilita l'avviso solo di certi SMS.
	  E ci sono problemi anche con quelli. Per questo motivo quest'opzione e' in genere
	  disabilitata in Gammu, occorre abilitarla in fase di compilazione del programma
	  modificando il file "config.h".

	* Salvando SMS sul telefono come non letti, non compare il simbolo di "nuovo messaggio".
	  Spiacente. Chiedete alla Nokia se e' possibile averlo.

	* Dopo aver cancellato il logo operatore, esso resta visibile finche' non si riavvia il
	  telefono. Ringraziate la Nokia per questo.

   3. Nokia 7110: impossibile leggere suonerie in formato binario dal telefono.

	Aggiornate il firmware (*). Il piu' recente e' il 5.01 e funziona.

   4.	Tutti i telefoni Noia con gruppi chiamanti: inviando a un modello di telefono il
	backup fatto su un altro modello, le suonerie non risultano selezionabili dal menu,
	o risultano differenti.

	Ogni modello Nokia puo avere un IDentificatore diverso per le stesse suonerie.
	Gli ID non vengono salvati da Gammu per ogni modello, perche' ora
	(per i telefoni DCT3) e' terribilmente facile cambiarli e una simile lista
	non sarebbe mai aggiornata. Per questo motivo, bisogna fare il backup dei 
	gruppi chiamanti in formato testo, e rimuovere le righe con gli ID. Cosi', dopo
	il ripristino sul telefono sara' possibile selezionare le suonerie dal menu.

   5. Il telefono (o forse Gammu) non da' informazioni sul Cell Broadcasting.

	Per default questa caratteristica e' disabilitata su Gammu (vedere opzioni di
	compilazione nel file "config.h"), poiche' alcune persone hanno riscontrato problemi:
	i loro telefoni non rispondevano subito a questo comando, o rispondevano con molto
	ritardo. Personalmente, io non ho avuto questo problema. Se volete, sperimentate
	il comando (Ma dovrete ricompilare Gammu con le opportune opzioni nel file "config.h").
	Sel funziona, tanto meglio! 

   6. La seconda suoneria del 3210, che non puo' essere impostata .
	
	Ebbene, non so che farci. Usate --nokiacomposer per inserire manualmente la suoneria!
   ---------
   CONSIGLIO: Per saperne di piu' sugli errori dei firmware Nokia, visitate il sito:
 		  http://marcin-wiacek.topnet.pl 
-------------------------------------------------------------------------------
D. Come riferire ai programmatori gli errori del programma?
R. Se il programma non funziona affato o comunque da' problemi, usare il parametro 
	"logfile" nel file "gnokiirc" (un esempio di file "gnokiirc" e' nella cartella 
	/docs/examples), in modo da creare un cosiddetto "file di log".
	Impostare ad esempio tale parametro su "rapporto", e avviare il programma
	con l'opzione che non funziona. Verra' creato un file "rapporto". Esaminatelo.
	Se ci sono delle righe che iniziano con "[ERROR:", leggete qui sotto cosa fare 
	col file.
	
	Se invece non ci sono queste righe, impostare "logfile" a "binary" (esattamente
	questo nome), e avviate di nuovo l'opzione che da' problemi. Verra' creato un log
	"binary". Vedi sotto cosa fare con questo file.

    COSA FARE COI FILE DI LOG:
   
   Occorre inviarli alla mailing list di Gammu, o direttamente all'autore 
  (marcin-wiacek@topnet.pl) (se volete evitare di inviare questi dati a tutti i
   membri della mailing list). Se i file sono troppo grandi, comprimeteli.

   A questi file potrete aggiungere:
	1. (Se usate Linux o Unix): Il nome della distribuzione, la versione del kernel.
	2. Almeno una breve descrizione del problema.

   Non serve accludere il file gnokiirc, le impostazioni vengono salvate nel file di log.

   Grazie per il feedback!

	CONSIGLIO:	impostando "logfile" a "errors" (esattamente questo nome),
			il file di log conterra' solo gli errori. Dopo aver usato
			per un po' Gammu, esaminate questo file per vedere
			se ci sono stati problemi.
-------------------------------------------------------------------------------
D. Chi ha aiutato a scriverlo?
R. Qui ci sono alcune persone che hanno contribuito allo sviluppo del progetto.
   Probabilmente non sono menzionati tutti (specialmente nella versione tradotta
   di questo file...). Se credi che il tuo nome debba comparire nell'elenco, faccelo
   sapere.
	* tutte le persone che hanno sviluppato "gnokii" (alcune idee sono prese da li')
	  come Pawel Kot, Manfred Jonsson,... (vedere i CREDITS di "gnokii").
	* tutti i responsabili del progetto "MyGnokii", come Gabriele Zappi, Ralf Thelen, ...
	  (vedere CREDITS di "MyGnokii").
	* Marcin Wiacek - l'autore di tutta 'sta roba!
	* Markus Plail & Tibor Harsszegi - per l'aiuto sul DCT4
	* Frederick Ros - aiuto per il 5210
	* Jan Ktatochvil - comandi AT
	* Walek - aggiustamenti vari

-------------------------------------------------------------------------------
D. Che modelli sono supportati?
R. 1. 3210|3310|3330|3390|3410|5110|5110i|5130|5190|5210|5510|6110|6130|6150|
      8210|8250|8290|8850|8855|8890:
	Questi dovrebbero funzionare tutti (apparte forse alcuni dettagli), ora
	o in futuro. Ho testato personalmente 3210, 3310, 5110 e 6150.

   2. 3510|6310|6310i|6510|8310|8910:
	Molto ben testati (specie il 6310i). Dovrebbe funzionare tutto, ora o
	prossimamente.
	DA FARE:
		* lettura sveglia
		* supporto completo di messaggi grafici e loghi
		* impostazione suonerie binarie
		* netmon, ALS
		* reset per tutte le connessioni

   3. 6210|6250|7110:

	Molto ben testati (specie il 6210). Dovrebbe funzionare tutto, ora o
	prossimamente.

     DA FARE: lettura nomi dei messaggi


   4. 9110|9210:

     Netmonitor attivato sul 9210, probabilmente funziona anche qualcos'altro.

   5. Telefoni AT:

     Testati vari telefoni Nokia. Molte cose funzionano al 100%, come il supporto SMS 
     (sia in modalita' testo che PDU), rubrica...

	DA FARE: 	Supporto per Siemens.

   6. Altre cose da fare:
			Profili
			Deviazione chiamate
			Conversione di tutti i tipi di suonerie
			Decodifica completa del Cell Broadcasting
			Driver seriale per DJGPP
-------------------------------------------------------------------------------
D. Che formati sono supportati?
R. loghi			: xpm (solo salvataggio), bmp, nlm, nsl, ngg, nol
   suonerie			: rttl, binary format created for Gammu,
                          mid (salvataggio), re (lettura), ott, formato
				  Nokia Communicator trovato su fkn.pl .
   backup/ripristino	: formato testo creato da Gammu,
                          lmb (avvio, rubrica, gruppi chiamanti)
   backup/ripristino SMS: formato testo creato da Gammu.

-------------------------------------------------------------------------------


*	FRAME: E' una sequenza di dati inviati da Gammu al telefono per fargli compiere
	le operazioni desiderate. Gli autori di Gammu hanno ricavato il formato dei FRAME
	utilizzati dai Nokia non da documentazione ufficiale, ma tramite analisi dei dati
	trasmessi attraverso la porta seriale (vedere la cartella "develop" per dettagli).


** 	Che cos'e' il FIRMWARE del telefono?
 	Il FIRMWARE e' il sistema operativo del telefono, ossia il programma che permette al
	telefono di funzionare. Un cellulare, infatti, non e' un altro che un computer
	in grado di gestire la comunicazione a voce tra due (o piu') utenti, e come
	tutti i computer ha bisogno di programmi, per funzionare: il FIRMWARE, appunti ; esso sta a 
	meta' tra l'HARDWARE, che e' l'insieme dei circuiti che compongono un computer,
	e non puo' essere modificato, e il SOFTWARE, ossia i programmi che utilizzano
	l'hardware per funzionare; infatti, il FIRMWARE e' software memorizzato
	"stabilmente" sull'hardware. Tra virgolette, perche' per un utente comune il
	firmware di un cellulare resta quello finche' il cellulare non viene buttao, perso,
	venduto o quant'altro. Gli utenti piu' evoluti, pero', possono riuscire
	a modificare il firmware, mediante un'operazione detta FLASHING, effettuata
	mediante particolari programmi detti FLASHER.
	In teoria, solo le case produttrici possiedono questi programmi, per cui per
	aggiornare il firmware del vostro cellulare dovreste portarlo in un centro
	assistenza autorizzato... da dove verrebbe spedito alla sede centrale della ditta,
	magari a 500 km dalla vostra citta', e lo rivedreste solo dopo 1 o 2 mesi, e
	dopo un esborso di 20-30 euro o piu'.
	IN PRATICA, su internet e' possibile trovare tutti gli ingredienti per "flashare"
	da soli il proprio cellulare; il problema e' che occorre una cosa fondamentale,
	il cavetto adatto per effettuare il flashing, che puo' costare anche molto di piu'
	dell'operazione effettuata secondo i canali ufficiali! Oltre a questo, occorre,
	come detto, il software flasher, e poi ovviamente il firmware aggiornato; questo,
	pero', si da' il caso che sia protetto da copyright da parte della casa che produce
	il telefono, e distribuirlo sulla rete e' illegale...
	Sul sito di Gammu (http://marcin-wiacek.topnet.pl) potete trovare dettagli su
	pregi e difetti delle varie versioni di firmware, ma non il materiale suddetto,
	per trovare il quale dovrete fare delle ricerche su internet usando appunto parole
	come "firmware", "flashing", "mobile phone" eccetera.

