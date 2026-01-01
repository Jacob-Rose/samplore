# Third Party Libraries

## SQLite

Download the SQLite amalgamation from https://www.sqlite.org/download.html

You need two files:
- `sqlite3.h`
- `sqlite3.c`

Place them in this directory for the project to build.

Alternatively, run:
```bash
curl -O https://www.sqlite.org/2024/sqlite-amalgamation-3460200.zip
unzip sqlite-amalgamation-3460200.zip
mv sqlite-amalgamation-3460200/sqlite3.* .
rm -rf sqlite-amalgamation-3460200 sqlite-amalgamation-3460200.zip
```
