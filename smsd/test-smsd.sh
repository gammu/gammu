#!@SH_BIN@

set -x
set -e
SMSD_PID=0

SERVICE="$1"

echo "NOTICE: This test is quite tricky about timing, if you run it on really slow platform, it might fail."
echo "NOTICE: Testing service $SERVICE"

cleanup() {
    if [ $SMSD_PID -ne 0 ] ; then
        kill $SMSD_PID
        sleep 1
    fi
}

trap cleanup INT QUIT EXIT

cd @CMAKE_CURRENT_BINARY_DIR@

rm -rf smsd-test-$SERVICE
mkdir smsd-test-$SERVICE
cd smsd-test-$SERVICE

# Dummy backend storage
mkdir gammu-dummy
# Create config file
cat > .smsdrc <<EOT
[gammu]
model = dummy
connection = none
port = @CMAKE_CURRENT_BINARY_DIR@/smsd-test-$SERVICE/gammu-dummy
gammuloc = /dev/null
loglevel = textall

[smsd]
commtimeout = 5
debuglevel = 255
logfile = stderr
runonreceive = @CMAKE_CURRENT_BINARY_DIR@/smsd-test-$SERVICE/received.sh
EOT

# Add driver specific configuration
case $SERVICE in
    dbi-sqlite3)
        cat >> .smsdrc <<EOT
service = dbi
driver = sqlite3
database = smsd.db
dbdir = @CMAKE_CURRENT_BINARY_DIR@/smsd-test-$SERVICE/
EOT
        ;;
    dbi-pgsql)
        cat >> .smsdrc <<EOT
service = dbi
driver = pgsql
pc = @PSQL_HOST@
database = @PSQL_DATABASE@
user = @PSQL_USER@
password = @PSQL_PASSWORD@
EOT
        ;;
    pgsql)
        cat >> .smsdrc <<EOT
service = pgsql
pc = @PSQL_HOST@
database = @PSQL_DATABASE@
user = @PSQL_USER@
password = @PSQL_PASSWORD@
EOT
        ;;
    dbi-mysql)
        cat >> .smsdrc <<EOT
service = dbi
driver = mysql
pc = @MYSQL_HOST@
database = @MYSQL_DATABASE@
user = @MYSQL_USER@
password = @MYSQL_PASSWORD@
EOT
        ;;
    mysql)
        cat >> .smsdrc <<EOT
service = mysql
pc = @MYSQL_HOST@
database = @MYSQL_DATABASE@
user = @MYSQL_USER@
password = @MYSQL_PASSWORD@
EOT
        ;;
    files*)
        INBOXF=`echo $SERVICE | sed 's/.*-//'`
        cat >> .smsdrc <<EOT
service = files
inboxpath = @CMAKE_CURRENT_BINARY_DIR@/smsd-test-$SERVICE/inbox/
outboxpath = @CMAKE_CURRENT_BINARY_DIR@/smsd-test-$SERVICE/outbox/
sentsmspath = @CMAKE_CURRENT_BINARY_DIR@/smsd-test-$SERVICE/sent/
errorsmspath = @CMAKE_CURRENT_BINARY_DIR@/smsd-test-$SERVICE/error/
inboxformat = $INBOXF
transmitformat = auto
EOT
        ;;
esac

# Create database structures
case $SERVICE in
    *sqlite3)
        @SQLITE_BIN@ smsd.db < @CMAKE_CURRENT_SOURCE_DIR@/../docs/sql/sqlite.sql
        ;;
    *pgsql)
        echo "DROP TABLE IF EXISTS daemons, gammu, inbox, outbox, outbox_multipart, pbk, pbk_groups, phones, sentitems;" | PGPASSWORD=@PSQL_PASSWORD@ @PSQL_BIN@ -h @PSQL_HOST@ -U @PSQL_USER@ @PSQL_DATABASE@
        PGPASSWORD=@PSQL_PASSWORD@ @PSQL_BIN@ -h @PSQL_HOST@ -U @PSQL_USER@ @PSQL_DATABASE@ < @CMAKE_CURRENT_SOURCE_DIR@/../docs/sql/pgsql.sql 2>&1 | grep -v 'ERROR.*language "plpgsql" already exists'
        ;;
    *mysql)
        echo "DROP TABLE IF EXISTS  daemons, gammu, inbox, outbox, outbox_multipart, pbk, pbk_groups, phones, sentitems;" | @MYSQL_BIN@ -u@MYSQL_USER@ -h@MYSQL_HOST@ -p@MYSQL_PASSWORD@ @MYSQL_DATABASE@
        @MYSQL_BIN@ -h@MYSQL_HOST@ -u@MYSQL_USER@ -p@MYSQL_PASSWORD@ @MYSQL_DATABASE@ < @CMAKE_CURRENT_SOURCE_DIR@/../docs/sql/mysql.sql
        ;;
    files*)
        mkdir -p @CMAKE_CURRENT_BINARY_DIR@/smsd-test-$SERVICE/inbox/
        mkdir -p @CMAKE_CURRENT_BINARY_DIR@/smsd-test-$SERVICE/outbox/
        mkdir -p @CMAKE_CURRENT_BINARY_DIR@/smsd-test-$SERVICE/sent/
        mkdir -p @CMAKE_CURRENT_BINARY_DIR@/smsd-test-$SERVICE/error/
        ;;
esac

cat > @CMAKE_CURRENT_BINARY_DIR@/smsd-test-$SERVICE/received.sh << EOT
#!@SH_BIN@
echo "\$@" >> @CMAKE_CURRENT_BINARY_DIR@/smsd-test-$SERVICE/received.log
exit 4
EOT
chmod +x @CMAKE_CURRENT_BINARY_DIR@/smsd-test-$SERVICE/received.sh

CONFIG_PATH="@CMAKE_CURRENT_BINARY_DIR@/smsd-test-$SERVICE/.smsdrc"
DUMMY_PATH="@CMAKE_CURRENT_BINARY_DIR@/smsd-test-$SERVICE/gammu-dummy"

mkdir -p $sms $DUMMY_PATH/sms/1
mkdir -p $sms $DUMMY_PATH/sms/2
mkdir -p $sms $DUMMY_PATH/sms/3
mkdir -p $sms $DUMMY_PATH/sms/4
mkdir -p $sms $DUMMY_PATH/sms/5

@CMAKE_CURRENT_BINARY_DIR@/gammu-smsd@GAMMU_TEST_SUFFIX@ -c "$CONFIG_PATH" &
SMSD_PID=$!

sleep 5

for sms in 62 68 74 ; do
    cp @CMAKE_CURRENT_SOURCE_DIR@/../tests/at-sms-encode/$sms.backup $DUMMY_PATH/sms/1/$sms
done

@CMAKE_CURRENT_BINARY_DIR@/gammu-smsd-inject@GAMMU_TEST_SUFFIX@ -c "$CONFIG_PATH" TEXT 123465 -text "Lorem ipsum." &
@CMAKE_CURRENT_BINARY_DIR@/gammu-smsd-inject@GAMMU_TEST_SUFFIX@ -c "$CONFIG_PATH" TEXT 123465 -text "Lorem ipsum."

# Insert message manually
case $SERVICE in
    *sqlite3)
        echo "INSERT INTO outbox(DestinationNumber,TextDecoded,CreatorID,Coding) VALUES('800123465', 'This is a SQL test message', 'T3st', 'Default_No_Compression');" | @SQLITE_BIN@ smsd.db
        ;;
    *pgsql)
        echo "INSERT INTO outbox(DestinationNumber,TextDecoded,CreatorID,Coding) VALUES('800123465', 'This is a SQL test message', 'T3st', 'Default_No_Compression');" | PGPASSWORD=@PSQL_PASSWORD@ @PSQL_BIN@ -h @PSQL_HOST@ -U @PSQL_USER@ @PSQL_DATABASE@
        ;;
    *mysql)
        echo "INSERT INTO outbox(DestinationNumber,TextDecoded,CreatorID,Coding) VALUES('800123465', 'This is a SQL test message', 'T3st', 'Default_No_Compression');" | @MYSQL_BIN@ -u@MYSQL_USER@ -h@MYSQL_HOST@ -p@MYSQL_PASSWORD@ @MYSQL_DATABASE@
        ;;
    files*)
        cp @CMAKE_CURRENT_SOURCE_DIR@/tests/OUT+4201234567890.txt @CMAKE_CURRENT_BINARY_DIR@/smsd-test-$SERVICE/outbox/
        ;;
esac


sleep 5

for sms in 10 16 26 ; do
    cp @CMAKE_CURRENT_SOURCE_DIR@/../tests/at-sms-encode/$sms.backup $DUMMY_PATH/sms/3/$sms
done

TIMEOUT=0
while ! @CMAKE_CURRENT_BINARY_DIR@/gammu-smsd-monitor@GAMMU_TEST_SUFFIX@ -C -c "$CONFIG_PATH" -l 1 -d 0 | grep -q ";999999999999999;3;6;0;100;42" ; do
    @CMAKE_CURRENT_BINARY_DIR@/gammu-smsd-monitor@GAMMU_TEST_SUFFIX@ -C -c "$CONFIG_PATH" -l 1 -d 0
    sleep 1
    TIMEOUT=$(($TIMEOUT + 1))
    if [ $TIMEOUT -gt 60 ] ; then
        echo "ERROR: Wrong timeout!"
        exit 1
    fi
done

sleep 5

@CMAKE_CURRENT_BINARY_DIR@/gammu-smsd-monitor@GAMMU_TEST_SUFFIX@ -C -c "$CONFIG_PATH" -l 1 -d 0

if [ `wc -l < @CMAKE_CURRENT_BINARY_DIR@/smsd-test-$SERVICE/received.log` -ne 6 ] ; then
    echo "ERROR: Wrong number of messages received!"
    exit 1
fi
