#!@SH_BIN@

set -x
set -e

rm -rf smsd-test
mkdir smsd-test
cd smsd-test
# Create database
@SQLITE_BIN@ smsd.db < @CMAKE_CURRENT_SOURCE_DIR@/../docs/sql/sqlite.sql
# Dummy backend storage
mkdir gammu-dummy
# Create config file
cat > .smsdrc <<EOT
[gammu]
model = dummy
connection = none
port = @CMAKE_CURRENT_BINARY_DIR@/smsd-test/gammu-dummy
gammuloc = /dev/null

[smsd]
service = dbi
driver = sqlite3
database = smsd.db
dbdir = @CMAKE_CURRENT_BINARY_DIR@/smsd-test/
debuglevel = 255
logfile = stderr
EOT

CONFIG_PATH="@CMAKE_CURRENT_BINARY_DIR@/smsd-test/.smsdrc"

@CMAKE_CURRENT_BINARY_DIR@/gammu-smsd -c "$CONFIG_PATH" &
SMSD_PID=$!

sleep 10

@CMAKE_CURRENT_BINARY_DIR@/gammu-smsd-monitor -c "$CONFIG_PATH" -l 1

sleep 10
kill $SMSD_PID
