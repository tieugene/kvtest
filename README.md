# kvtest

Key-Value storage test

## Description

Tools to test popular file-based key-value storages simulating real work - add and read up to 2 billions key-value pairs (20-byte key, 4-byte value).

Test stages:

- `Add` - create given number of random key-value records and store these keys in RAM (!).
- `Get` - try to get random *existing* keys
- `Ask` - try to get random *existing* and *non-existing* keys (50/50)
- `Try` - try to get (if exist, 50%) or add (if not, 50%) key-values

Backends used:

- BerkeleyDB (kvtest_bdb)
- LevelDB (kvtest_ldb)
- RocksDB (kvtest_rdb)
- Kyotocabinet (kvtest_kc)
- Tkrzw (kvtest_tk)
- ~~LMDB~~ (in plans)

*Note: It is possible to define DB type for some backends (BerkeleyDB, Kyotocabinet and Tkrzw). This depends on filename given (type `kvtest_<backend> -f anywrongfilename 1` for details).*

## Usage

`kvtest_<backend> [options] <num>`

Options:

- `-h` - help
- `-f <path>` - file/dir name of DB
- `-t n` - duration of each test, sec. (1..255, default=5)
- `-x c` - exclude step (g=Get/a=Ask/t=Try)
- `-v` - verbose

Mandatory argument `<num>` - power of 2 of records to create (1..31).

To avoid long RTFM - full example:

`kvtest_tk -f test.tkt -t 10 -x a -x t -v 30`

- `kvtest_tk` - Tkrzw engine
- `-f test.tkt` - TreeDBM file database
- `-t 10` - execute each test 10 sec (excepting `Add`)
- `-x a -x t` - exclude `Ask` and `Try` stages
- `-v` - verbose output (show progress)
- `30` - create 2<sup>30</sup> (1G) records (`Add` stage)

*Note: pay attention to `Try` stage - it creates new 2<sup>n-1</sup> records in addition to `Add` stage.*

## License

Tool is published under GPLv3 license.
